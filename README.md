# crelay
#### Controlling the Conrad USB 4-channel relay card with a Linux software

### About
This software controls the relays on the USB 4-channel relay card (USB 4fach-Relaiskarte) from Conrad. See here for product description: http://www.conrad.de/ce/de/product/393905
The relay card software provided by Conrad is Windows only and uses a binary runtime DLL which implements the communication protocol between the host computer and the card. Thanks to a raspberrypi.org forum member, the communication protocol was discovered and made public. This made it possible to develop an open source software which can run on any Linux distribution with the cp210x kernel driver installed (see note below).

<i>Note</i>
We need the cp210x kernel driver for the Silabs CP2104 chip with GPIO support. The official in-kernel cp210x driver does currently not yet support GPIO operations. Therefore the Silabs driver from their home page needs to be used:
http://www.silabs.com/products/mcu/pages/usbtouartbridgevcpdrivers.aspx
 
### Features:
- Command line mode and daemon mode with Web GUI
- Automatic detection of USB communication port
- Reading of current relay states
- Setting of new relay states
- Single pulse generation on relay contact

### Not yet supported (to do):
- multiple card support
- access control for Web GUI
- programmable timers for relay actions
- smartphone client app
- other useful things
