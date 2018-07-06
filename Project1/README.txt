#Usage

- Place Project1.exe in the same directory as a config.txt file. Double-click the executable to begin.
- config.txt must contain the following fields in this order, seperated by tabs:
	- Sampling Rate (integer, in hertz. Must be less than 100kHz)
	- Number of Channels to Sample (Aggregate sample rate cannot exceed 400kHz)
	- Voltage range (1,2,5, or 10)
	- Duration of sampling period (in minutes)
- A copy of the config data for the session is saved at configdataXX.txt, where XX is some number between 00 and 99
- Raw data will be ouput as dataXX.daq , where XX is same number from configdata
- At the end of the session, the actual configuration that was run is saved at confirmdataXX.txt. This should match configdataXX.txt
- Raw data will be saved even if the session is interrupted, but confirmdataXX.txt is only created at the end of a session.



