
# KNELL: Keen Nameless Electrical Loss Lookout
## 705-power-loss-detection

A cellular Internet of Things (IOT) device that can detect power loss from main AC. Designed as a system as a whole to send out notifications to facility managers/ Staff of interest so that memebers can perform graceful shutdown of key Information Technology infrastructure and services withon alloted Uninterrupted Power Supply (UPS) alloted time. 

## System description

The entire system is composed of a device layer, a cloud layer, a service layer, and a user layer.
See the following diagram for a visual model:
Markup : ![picture alt](https://github.com/skorm11x/705-power-loss-detection/blob/master/pics/KNELLv0.1.drawio.png/)

### Account credentials
Currently, the system operates off of a single particle account (controlled by the primary developer) that is used for device monitoring and control of the cloud events and a single user account through the Pushover.net service: (https://pushover.net/)

There are no future plans to expand the number of accounts directly interfacing with Particle. If the future dictates that end users want more access to the particle cloud a seperate client (under the authentication tab) will be created for them to interact with the data. 

Account login details are stored in the 705 CTS government only drive under Facility management/KNELL_Facility_Monitoring_System

### Push notification service: Pushover
Markup : ![picture alt](https://github.com/skorm11x/705-power-loss-detection/blob/master/pics/pushover.png/)

Why pushover? It is dead simple and cheap! 

Users install an application onto their smartphone and receive push notifications with alert sounds.
Google play: https://play.google.com/store/apps/details?id=net.superblock.pushover
Apple I store: https://apps.apple.com/us/app/pushover-notifications/id506088175

Ultimately, the government sponsoring this development was very risk adverse and unwilling to spend money so we compromised on reoccuring service charges and use a singular, paid, pushover account to deliver push notifications. Thus, when registering or setting up new users, you simply give them the account credentials mentioned above for the facility team and let them configure other settings in the app as it suits them.

Example process:
1. New user requests access to pushover push notifications.
2. User meets with facility management team who instructs them to install application and provides or enters login information into the pushover application.
3. The new user receives the push notifications!

## Developer information and documentation

### Particle software orginization

#### ```/src``` folder:  
This is the source folder that contains the firmware files for your project. It should *not* be renamed. 
Anything that is in this folder when you compile your project will be sent to our compile service and compiled into a firmware binary for the Particle device that you have targeted.

If your application contains multiple files, they should all be included in the `src` folder. If your firmware depends on Particle libraries, those dependencies are specified in the `project.properties` file referenced below.

#### ```.ino``` file:
This file is the firmware that will run as the primary application on your Particle device. It contains a `setup()` and `loop()` function, and can be written in Wiring or C/C++. For more information about using the Particle firmware API to create firmware for your Particle device, refer to the [Firmware Reference](https://docs.particle.io/reference/firmware/) section of the Particle documentation.

#### ```project.properties``` file:  
This is the file that specifies the name and version number of the libraries that your project depends on. Dependencies are added automatically to your `project.properties` file when you add a library to a project using the `particle library add` command in the CLI or add a library in the Desktop IDE.

#### ```models``` folder:
This folder contains all the 3D CAD, OBJ, STL files used in the outdoor enclosure.

### Current libraries in use
 DiagnosticsHelperRK : a library to access lower level things inside of the Device OS that particle boards run. Specifically, we use it to read the system power source. 
 Credit to rickkas7: https://github.com/rickkas7/DiagnosticsHelperRK
 MIT license type so can be utilized for proprietery/ commercial use. 

#### Projects with external libraries
TODO: we need to restructure the simple library use in this repo to match the below: 

If your project includes a library that has not been registered in the Particle libraries system, you should create a new folder named `/lib/<libraryname>/src` under `/<project dir>` and add the `.h`, `.cpp` & `library.properties` files for your library there. Read the [Firmware Libraries guide](https://docs.particle.io/guide/tools-and-features/libraries/) for more details on how to develop libraries. Note that all contents of the `/lib` folder and subfolders will also be sent to the Cloud for compilation.

### Device setup/ Hardware

Hardware required:

BORON 404x LTE (USA)
Markup : ![picture alt](https://github.com/skorm11x/705-power-loss-detection/blob/master/pics/Boron-LTE-Cat-M1.png/)

STOCK ANTENNA:
Markup : ![picture alt](https://github.com/skorm11x/705-power-loss-detection/blob/master/pics/Taoglas-Cellular-Flex-Antenna.png/)

Battery (any 3.7 LIPO):
Markup : ![picture alt](https://github.com/skorm11x/705-power-loss-detection/blob/master/pics/3.7LIPO.jpg/)


Follow the Boron 404x quickstart: https://tools.particle.io/setup
Specifically, there may be some issues with the default firmware/ version of device OS that came installed on the device. Please visit the device doctor for diagnosing any problems encountered with the device not connecting to the cloud properly:
https://docs.particle.io/tools/doctor/

Once firmware is flashed properly you should have success for connection to the particle cloud. It is recommended you download the mobile app onto your device and log onto the particle cloud console https://console.particle.io/ to see your device connected.

From here you can do any of the LED flash setups/ tests to ensure you are understanding the Particle object etc. part of this code **currently** maintains some test code and explanations on how to use the Particle objects/ cloud.

***Future updates to hardware will include reference schematics for easy build out.***

### Particle Console setup and webhooks

This sections explains the particle console and how to setup basic webhooks.

Login with the developer credentials to https://console.particle.io/devices
Ensure that the device is registered properly and is active. This should have also been done in the Device setup/ Hardware section.

Navigate to the *Integrations* section.


