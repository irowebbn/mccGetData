#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <io.h>
#include <iostream>
#include <fstream>

#include "cbw.h"

#define MAX_DEV_COUNT  100
#define MAX_STR_LENGTH 64
#define MAX_SCAN_OPTIONS_LENGTH 256

using namespace std;

int getGain(int vRange) {
	int gain;
	switch (vRange) {
	case(1): {
		gain = BIP1VOLTS;
		return gain;
		break;
	}
	case(2): {
		gain = BIP2VOLTS;
		return gain;
		break;
	}
	case(5): {
		gain = BIP5VOLTS;
		return gain;
		break;
	}
	case(10): {
		gain = BIP10VOLTS;
		return gain;
		break;
	}
	default: {
		gain = BIP5VOLTS;
		return gain;
		break;
	}
	}
}

int getvRange(int gain) {
	int vRange;
	switch (gain) {
	case(BIP1VOLTS): {
		vRange = 1;
		return vRange;
		break;
	}
	case(BIP2VOLTS): {
		vRange = 2;
		return vRange;
		break;
	}
	case(BIP5VOLTS): {
		vRange = 5;
		return vRange;
		break;
	}
	case(BIP10VOLTS): {
		vRange = 10;
		return vRange;
		break;
	}
	default: {
		vRange = 5;
		return vRange;
		break;
	}
	}
}


int main(int argc, const char *argv[]) {
	cbIgnoreInstaCal();
	const char* fileName = "config.txt";
	SYSTEMTIME st;

	DaqDeviceDescriptor devDescriptors[MAX_DEV_COUNT];
	DaqDeviceInterface interfaceType = ANY_IFC;
	DaqDeviceDescriptor DeviceDescriptor;
	int boardNum = 1;
	int numDevs = MAX_DEV_COUNT;
	int detectError = 0;

	// Read config file
	int fileDesc = _open(fileName, _O_RDONLY);
	//Error Message if not possible
	if (fileDesc < 0) {
		fprintf(stderr, "Cannot open file %s\n", fileName);
		system("pause");
		return -1;
	}
	// Copy file to string buf
	char buf[100];
	const int bytesToRd = 100;
	int numRead = _read(fileDesc, buf, bytesToRd);
	if (numRead < 0) {
		fprintf(stderr, "Cannot read file\n");
		system("pause");
		return -1;
	}
	//Close file
	_close(fileDesc);
	// Format of config.txt: Rate (Hz), Number of Channels, Voltage Range (1, 2,5, or 10 volts), Duration (Minutes)
	// Each entry separated by tabs
	char* rateStr = strtok(buf, "\t");
	if (rateStr == NULL) {
		fprintf(stderr, "Cannot read format of %s\n", fileName);
		system("pause");
		return -1;
	}
	char* numChanStr = strtok(NULL, "\t");
	if (numChanStr == NULL) {
		fprintf(stderr, "Cannot read format of %s\n", fileName);
		system("pause");
		return -1;
	}
	char* vRangeStr = strtok(NULL, "\t");
	if (vRangeStr == NULL) {
		fprintf(stderr, "Cannot read format of %s\n", fileName);
		system("pause");
		return -1;
	}
	char* durationStr = strtok(NULL, "\t");
	if (durationStr == NULL) {
		fprintf(stderr, "Cannot read format of %s\n", fileName);
		system("pause");
		return -1;
	}
	int rate = atoi(rateStr);
	const int numChan = atoi(numChanStr);
	int vRange = atoi(vRangeStr);
	int duration = atoi(durationStr);

	printf("Rate: %i\n", rate);
	printf("Number of Channels: %i\n", numChan);
	printf("Range: %i\n", vRange);
	printf("Duration: %i\n", duration);

	// Verify values are acceptable
	if (rate > 100000) {
		printf("Individual channel sample rate cannot exceed 100kHz, acquisition will fail\n");
		system("pause");
		return -1;
	}

	if (rate*numChan > 400000) {
		printf("Aggregate sample rate exceeds 400kHz, acquisition will fail\n");
		system("pause");
		return -1;
	}
	if ((vRange != 1) && (vRange != 2) && (vRange != 5) && (vRange != 10)) {
		printf("Invalid range for voltage gain. Must be 1, 2, 5 or 10.\n");
		system("pause");
		return -1;
	}
	// Write config values back to data.txt
	ofstream dataFile;
	char dataFileName[] = "CONFIGDATA00.txt";
	for (int i = 0; i < 100; i++) {
		dataFileName[10] = i / 10 + '0';
		dataFileName[11] = i % 10 + '0';
		if (GetFileAttributes(dataFileName) == INVALID_FILE_ATTRIBUTES) {//File doesn't already exist
			break;
		}
	}
	dataFile.open(dataFileName);
	dataFile << rate << "\n" << numChan << "\n" << vRange << "\n" << duration << endl;
	

	// Acquire device(s)
	detectError = cbGetDaqDeviceInventory(interfaceType, devDescriptors, &numDevs);

	if (detectError != 0) {
		printf("Cannot acquire device inventory\n");
		char ErrMsg[ERRSTRLEN];
		cbGetErrMsg(detectError, ErrMsg);
		printf(ErrMsg);
		system("pause");
		return -1;
	}

	// verify at least one DAQ device is detected
	if (numDevs == 0) {
		printf("No DAQ device is detected\n");
		system("pause");
		return -1;
	}

	printf("Found %d DAQ device(s)\n", numDevs);
	// for (int i = 0; i < (int) numDevs; i++)
	printf("  %s: (%s)\n", devDescriptors[0].ProductName, devDescriptors[0].UniqueID);
	DeviceDescriptor = devDescriptors[0];
	detectError = cbGetBoardNumber(DeviceDescriptor);
	if (detectError == -1) {
		printf("Board number not yet assigned.\n");
	}
	else {
		printf("Board number is %i.\n", detectError);
	}
	// get a handle to the DAQ device associated with the first descriptor
	detectError = cbCreateDaqDevice(boardNum, DeviceDescriptor);
	if (detectError != 0) {
		printf("Can't create device or assign number.\n");
		char ErrMsg[ERRSTRLEN];
		cbGetErrMsg(detectError, ErrMsg);
		printf(ErrMsg);
		system("pause");
		return -1;
	}
	printf("Board number is %i.\n", boardNum);
	/*
	detectError = cbAInputMode(boardNum, SINGLE_ENDED);
	if (detectError != 0) {
	printf("Can't set input mode\n");
	char ErrMsg[ERRSTRLEN];
	cbGetErrMsg(detectError, ErrMsg);
	printf(ErrMsg);
	system("pause");
	return -1;
	}
	*/

	short LowChan = 0;
	short HighChan = numChan - 1;
	long ratel = (long)rate;
	long count = numChan * rate * duration * 60;

	char outfileStr[] = "DATA00.DAQ";
	outfileStr[4] = dataFileName[10];
	outfileStr[5] = dataFileName[11];

	int gain = getGain(vRange);

	printf("Beginning Sampling for next %i minutes.\n", duration);

	GetSystemTime(&st);
	dataFile << st.wHour << ":" << st.wMinute << ":" << st.wSecond << "." << st.wMilliseconds << endl;
	dataFile.close();
	detectError = cbFileAInScan(boardNum, LowChan, HighChan, count, &ratel, gain, outfileStr, 0);
	//detectError = cdFileAInScan(int boardnNum, int LowChan, int HighChan, long Count, long *Rate, int Range, char *FileName, int Options)
	//Calculate the parameters for above using the config file inputs
	// Add Channels
	//Configure input range and Rate
	//Set up trigger
	//Set up logging mode to save to outfile
	if (detectError != 0) {
		printf("Couldn't start scan\n");
		char ErrMsg[ERRSTRLEN];
		cbGetErrMsg(detectError, ErrMsg);
		printf(ErrMsg);
		system("pause");
		return -1;
	}
	printf("Sampling completed.\n");
	// Write values that were actually recorded to actual.txt
	//rate has been updated by cdFileAInScan()
	long PreTrigCount = 0;
	detectError = cbFileGetInfo(outfileStr, &LowChan, &HighChan, &PreTrigCount, &count, &ratel, &gain);
	if (detectError != 0) {
		printf("Couldn't read file");
		char ErrMsg[ERRSTRLEN];
		cbGetErrMsg(detectError, ErrMsg);
		printf(ErrMsg);
		system("pause");
		return -1;
	}
	char confirmFileName[] = "CONFIRMDATA00.txt";
	confirmFileName[11] = outfileStr[4];
	confirmFileName[12] = outfileStr[5];

	/*
	_itoa(ratel, rateStr, 10);
	_itoa((HighChan - LowChan) + 1, numChanStr, 10);
	_itoa(gain, vRangeStr, 10);
	_itoa(count / (rate*60), durationStr, 10);
	*/
	duration = (count / (rate * 60));
	vRange = getvRange(gain);
	ofstream confirmFile;
	confirmFile.open(confirmFileName);
	confirmFile << ratel << "\n" << ((HighChan - LowChan) + 1) << "\n" << vRange << "\n" << duration;
	confirmFile.close();
	system("pause");
}