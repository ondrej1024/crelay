# crelay
#### Controlling the Conrad USB 4-channel relay card with a Linux software

### About
This software controls the relays on the USB 4-channel relay card (USB 4fach-Relaiskarte) from Conrad. See here for product description: http://www.conrad.de/ce/de/product/393905

The relay card software provided by Conrad is Windows only and uses a binary runtime DLL which implements the communication protocol between the host computer and the card. Thanks to a raspberrypi.org forum member, the communication protocol was discovered and made public. This made it possible to develop an open source software which can run on any Linux distribution with the cp210x kernel driver installed (see note below).

### Features:
- Command line mode and daemon mode with Web GUI
- Automatic detection of USB communication port
- Reading of current relay states
- Setting of new relay states
- Single pulse generation on relay contact
- HTTP API for external clients (e.g. Smartphone/tablet apps)

### Not yet supported (to do):
- multiple card support
- support for relays connected via GPIO pins
- access control for Web GUI and HTTP API
- programmable timers for relay actions
- other useful things


### HTTP API:
An HTTP API is provided to access the server from external clients. This API is compatible with the PiRelay Android app. Therefore this app can be used on your Android phone to control <i>crelay</i> remotely.

- API url  <pre><i>ip_address:[port]</i>/gpio</pre>
- Method: <pre>POST or GET</pre>
- Reading relay states<br>
    Required Parameter: none
- Setting relay state<br> 
    Required Parameter: <pre>pin=[1|2|3|4], status=[0|1|2] where 0=off 1=on 2=pulse</pre>
- Response from server:
<pre>
Relay 1:[0|1]
Relay 2:[0|1]
Relay 3:[0|1]
Relay 4:[0|1]</pre>
<br>

#### Note:
For the crelay software to run, it needs the cp210x kernel driver for the Silabs CP2104 chip with GPIO support. The official in-kernel cp210x driver does currently not yet support GPIO operations. Therefore the Silabs driver from their home page needs to be used:
http://www.silabs.com/products/mcu/pages/usbtouartbridgevcpdrivers.aspx

Unfortunately the kernel internal interfaces are continuously changing and the Silabs drivers don't built just like that for any given kernel version. Therefore, for your convenience, the cp210x directory contains the patched driver sources and pre-built binary drivers for selected distros and kernel versions (currently only Raspberry Pi binaries are provided, contributions are welcome).
