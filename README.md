# crelay
#### Controlling different relay cards for home automation with a Linux software

### About
This software is intended to run on Linux systems to control different relay cards in a unified way. It provides several interfaces for controlling the relays locally or remotely. The relays can be controlled by a human being via a device like smartphone or web browser, or directly by an intelligent device as used in the Internet of Things.  
The software was designed with the following requirements in mind:  

 - simple, intuitive usage and interface
 - as little dependencies as possible (libraries, external programs)
 - runs on different Linux distributions, different hardware platforms
 - lightweight, can run on simple devices
 - easily expandable (adding relay card types and user interfaces)

New relay cards support can be added by providing the cards driver code for detecting the card, reading and setting the relays.
Currently the following relay cards are supported:  

- Conrad USB 4-channel relay card (http://www.conrad.de/ce/de/product/393905), see <i>Note 1</i> below
- Sainsmart USB 4-channel relay card (http://www.sainsmart.com/sainsmart-4-channel-5v-usb-relay-board-module-controller-for-automation-robotics.html), see <i>Note 2</i> below
- HID API compatible relay cards (1/2/4/8 channel)
- Generic GPIO controlled relays, see <i>Note 3</i> below  
<br>

The following picture shows a high level view on the modular software architecture.  
![Software architechture](https://raw.github.com/ondrej1024/crelay/master/screenshots/sw-architecture.png)
<br><br>

### Features
- Command line mode and daemon mode with Web GUI
- Automatic detection of communication port
- Reading of current relay states
- Setting of new relay states
- Single pulse generation on relay contact
- HTTP API for external clients (e.g. Smartphone/tablet apps)
- Multiple relay card type support  
<br>

### Coming soon (to do)
- ThingSpeak Talkback App (https://thingspeak.com/docs/talkback)
- Support for configuration file for custom parameters  
<br>

### Nice to have (wishlist)
- Multiple cards support
- Access control for Web GUI and HTTP API
- Programmable timers for relay actions  
<br>

### Screenshots

#### Web GUI
![Screenshot](https://raw.github.com/ondrej1024/crelay/master/screenshots/crelay-screenshot1.png)
<br><br>
![Screenshot](https://raw.github.com/ondrej1024/crelay/master/screenshots/crelay-screenshot2.png)
<br><br>

#### Command line interface
    $ crelay 
    crelay, version 0.7
    
    This utility provides a unified way of controlling different types of relay cards.
    Currently supported relay cards:
      - Conrad USB 4-channel relay card
      - Sainsmart USB 4-channel relay card
      - Generic GPIO relays
      - HID API compatible relay card
    The card which is detected first will be used. 
    
    The program can be run in interactive (command line) mode or in daemon mode with
    built-in web server.

    Interactive mode:
        crelay -i | [<relay number>] [ON|OFF]

           The state of any relay can be read or it can be changed to a new state.
           If only the relay number is provided then the current state is returned,
           otherwise the relays state is set to the new value provided as second parameter.
           The USB communication port is auto detected. The first compatible device
           found will be used.

    Daemon mode:
        crelay -d [<relay1_label> [<relay2_label> [<relay3_label> [<relay4_label>]]]] 
    
           In daemon mode the built-in web server will be started and the relays
           can be completely controlled via a Web browser GUI or HTTP API.
           Optionally a personal label for each relay can be supplied which will
           be displayed next to the relay name on the web page.
    
           To access the web interface point your Web browser to the following address:
           http://<my-ip-address>:8000
    
           To use the HTTP API send a POST or GET request from the client to this URL:
           http://<my-ip-address>:8000/gpio
<br>  

### HTTP API
An HTTP API is provided to access the server from external clients. This API is compatible with the PiRelay Android app. Therefore this app can be used on your Android phone to control <i>crelay</i> remotely.

- API url:  
<pre><i>ip_address[:port]</i>/gpio</pre>  

- Method:  
<pre>POST or GET</pre>  

- Reading relay states  
Required Parameter: none  

- Setting relay state  
Required Parameter: <pre>pin=[1|2|3|4], status=[0|1|2] where 0=off 1=on 2=pulse</pre>  

- Response from server:  
<pre>
Relay 1:[0|1]
Relay 2:[0|1]
Relay 3:[0|1]
Relay 4:[0|1]
</pre>  
<br>

### Installation
The installation procedure is usually perfomed directly on the target system. Therefore a C compiler and friends should already be installed. Otherwise a cross compilation environment needs to be setup on a PC (this is not described here).  

* Install dependencies:  
<pre>
    apt-get install libftdi1 libftdi-dev libhidapi-libusb0 libhidapi-dev
</pre>

* Clone git repository :  
<pre>
    git clone https://github.com/ondrej1024/crelay
    cd crelay
</pre>

* Alternatively get latest source code version :  
<pre>
    wget https://github.com/ondrej1024/crelay/archive/master.zip
    unzip master.zip
    cd crelay-master
</pre>

* Build and install :  
<pre>
    cd src
    make
    sudo make install
</pre>
<br>

### Adding new relay card drivers
TODO  
<br>

##### <i>Note 1 (Conrad USB 4-channel relay card)</i>:
The relay card software provided by Conrad is Windows only and uses a binary runtime DLL which implements the communication protocol between the host computer and the card. Thanks to a raspberrypi.org forum member, the communication protocol was discovered and made public. This made it possible to develop an open source driver for the Conrad card which can run on any Linux distribution with the cp210x kernel driver installed.

It needs the cp210x kernel driver for the Silabs CP2104 chip with GPIO support. The official in-kernel cp210x driver does currently not yet support GPIO operations. Therefore the Silabs driver from their home page needs to be used:
http://www.silabs.com/products/mcu/pages/usbtouartbridgevcpdrivers.aspx

Unfortunately the kernel internal interfaces are continuously changing and the Silabs drivers don't built just like that for any given kernel version. Therefore, for your convenience, the cp210x directory contains the patched driver sources and pre-built binary drivers for selected distros and kernel versions (currently only Raspberry Pi binaries are provided, contributions are welcome).  
<br>

##### <i>Note 2 (Sainsmart USB 4-channel relay card)</i>:
The Sainsmart card uses the FTDI FT245RL chip. This chip is controlled directly through the open source libFTDI library. No Kernel driver is needed. However on most Linux distributions, the *ftdi_sio* serial driver is automatically loaded when the FT245RL chip is detected. In order to grant the <i>crelay</i> software access to the card, the default driver needs to be unloaded:  

    rmmod ftdi_sio

To prevent automatic loading of the driver, add the following line to /etc/modprobe.d/blacklist.conf:  

    blacklist ftdi_sio
    
<br>  

##### <i>Note 3 (GPIO controlled relays)</i>:
The following GPIO pins are defined as factory default in relay_drv_gpio.c. Change these if you want to control different pins.
<pre>
 #define RELAY1_GPIO_PIN 17 // GPIO 0
 #define RELAY2_GPIO_PIN 18 // GPIO 1
 #define RELAY3_GPIO_PIN 27 // GPIO 2 (RPi rev.2)
 #define RELAY4_GPIO_PIN 22 // GPIO 3
 #define RELAY5_GPIO_PIN 23 // GPIO 4
 #define RELAY6_GPIO_PIN 24 // GPIO 5
 #define RELAY7_GPIO_PIN 25 // GPIO 6
 #define RELAY8_GPIO_PIN  4 // GPIO 7
</pre>

In order to be able to access the GPIO pins you need to run as superuser, therefore crelay needs to be executed with the <i>sudo</i> command on a multiuser system.
