# Tutorial: IFTTT

This tutorial explains how crelay can be controlled via the IFTTT cloud service, using crelay's HTTP API. Since crelay does currently not implement any authorization method, it is a security hazard to expose the crelay API to the public Internet. Please read the disclaimer below.

## How to use crelay with IFTTT

1. Setup your crelay, and make sure it's working correctly.

###### Once again, this can be a major security hazard, if you do not secure your connection properly. Please do *not* use this method unless you know what you are doing.

2. Forward the crelay port (By default it's 8000, though i recommend changing it, if you have shared your IP with anyone.) through your router. ([This guide can be helpful](https://www.noip.com/support/knowledgebase/general-port-forwarding-guide/))

3. Find your public IP, eventually by using [WhatIsMyIPAddress](https://whatismyipaddress.com/), and looking under IPv4. 

4. Go to [IFTTT](https://ifttt.com/), and create an account.

5. Create a new applet, by going under "[My Applets](https://ifttt.com/my_applets)", and then pressing the "[New Applet](https://ifttt.com/create)" button.

6. Press the "+this", and choose a service, which in this case, will be Google Assistant.

7. Then choose a trigger, (I chose the one called "Say a simple phrase".) and set it up to your likings.

8. When you have set up your trigger, press the "+that", and choose the service called "Webhooks", then the action called "Make a web request".

9. Under "URL" you have to put in "https://**_YourPublicIP_**:**_YourcrelayPort_**/gpio?pin=**_RelayNumber_**&status=**_Status_**", with **_YourPublicIP_** being the IP you got from [WhatIsMyIPAddress](https://whatismyipaddress.com/), and **_YourcrelayPort_** being the port you use for crelay, and **_RelayNumber_** being the relay that you wan't to turn on, and **_Status_** being "1" for "On" and "0" for "Off".

10. The method should be "GET", and the Content Type should be "text/plain", and the Body should be left empty.

11. Now press "Create action"

12. Now press "Finish" (And disable notifications)

13. Now repeat the steps from 5 to 12, only with the "1" in "&status=1" being a "0"

## Disclaimer
### The risk of forwarding your crelay port.

By forwarding the crelay API port from the public Internet you agree with the following terms:

You alone are responsible for securing your connection, at least by changing the default port.

Neither the author of crelay [ondrej1024](https://github.com/ondrej1024/) nor the author of this tutorial [Luna](https://github.com/ImLunaUwU) are responsible for any security hazards which may occur with opening your crelay port to the public Internet.

Opening the port for your crelay can be a major security hazard if you don't know how to protect your things correctly.
