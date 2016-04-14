-----------------------------------
Windows noVNC Websockify Service
----------------------------------- 

The "noVNC Websocket Service.exe" file is a windows service wrapper created with Visual Studio 2010 to create a windows service to start stop the noVNC Websocket Server. All files used to create the wrapper can be found in 'noVNC Websocket Service Project' folder.

To download the precompiled executables please grab the zip in the downloads section of websockify project:
https://github.com/kanaka/websockify

---------------------------
Installation
---------------------------

1. This service requires websockify.exe be in the same directory. Instructions on how to compile websockify python script as a windows executable can be found here:
https://github.com/kanaka/websockify/wiki/Compiling-Websockify-as-Windows-Executable

2.To add this service to a Windows PC you need to run the commandline as administrator and then run this line:

sc create "noVNC Websocket Server" binPath= "PATH TO noVNC eg C:\noVNC\utils\Windows\Websocket Service.exe" DisplayName= "noVNC Websocket Server"

3 .Once this is run you will be able to access the service via Control Panel > Admin Tools > Services. In here you can specify whether you want the service to run automatically and start at stop the service.

---------------------------
Configuration
---------------------------
The file noVNCConfig.ini must be in the same directory as "noVNC Websocket Service.exe".

This file contains a single line which is the websockify.exe statup arguements. An example is:
192.168.0.1:5901 192.168.0.1:5900

All websockify supported arguements will work if added here.

---------------------------
Deletion
---------------------------

You can delete the service at any time by running the commandline as admin and using this command:
sc delete "noVNC Websocket Server".

