crelay
======

Controlling the Conrad USB 4-relay card with a Linux software.

Description:
   Controls the relays on the USB 4-relay card from Conrad.
   http://www.conrad.de/ce/de/product/393905
   The relay card software provided by Conrad is Windows only and uses a binary 
   runtime DLL. Instead this software can run on any Linux distribution with 
   the cp210x driver installed (see note below) and is completely open source.

Note:
   We need the cp210x driver for the Silabs CP2104 chip with GPIO support.
   The official in-kernel cp210x driver does currently not yet support
   GPIO operations. Therefore the Silabs driver needs to be used:
   http://www.silabs.com/products/mcu/pages/usbtouartbridgevcpdrivers.aspx
 
Features:
   - Command line mode and daemon mode with Web GUI
   - Automatic detection of USB communication port
   - Reading of current relay states
   - Setting of new relay states
   - Single pulse generation on relay contact

Not yet supported (to do):
   - multiple card support
   - access control for Web GUI
   - programmable timers for relay actions
   - other useful things
