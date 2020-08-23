# MSFS2020 ForeFlight Support

This application was made to send sim data from MSFS2020 to ForeFlight. It sends attitude and position data at a constient rate to enable smooth tracking and live data as your fly in MSFS2020.

# Download and Run

For those wishing to just run 

- Download the repo to your machine
- Locate the **MSFS_Foreflight/MSFS_Release** folder
- Open a GitBash/PowerShell/CMD console
-  **Optional** Find broadcast ip
	- Run **ipconfg** 
	- Find broadcast ip [Helpful calculator here](https://remotemonitoringsystems.ca/broadcast.php)
- Run **ForeFlightSupport.exe <ip_of_foreflight_device or broadcast_ip>**
- Example Direct IP: **ForeFlightSupport.exe 192.168.1.5**
- Example Broadcat IP: **ForeFlightSupport.exe 192.168.1.255**
- Close by exiting bash window/console 

# Build and Run
### Requirments 
- [Windows Development Kit 10 ](https://developer.microsoft.com/en-us/windows/downloads/windows-10-sdk/)
- [Winsock2](https://docs.microsoft.com/en-us/windows/win32/winsock/getting-started-with-winsock)
- [MSFS2020 SDK](https://fs2020.surclaro.com/msfs2020-sdk-is-here-start-developing-fs2020-add-ons/)
- SimConnect DLL/LIB (Include with MSFS2020 SDK)
- [Visual Studio](https://docs.microsoft.com/en-us/visualstudio/install/install-visual-studio?view=vs-2019)

For those wishing to build and add on features 
- Download the repo
- Open  **ForeFlightSupport.sln** with Visual Studio 
- Build and run 

