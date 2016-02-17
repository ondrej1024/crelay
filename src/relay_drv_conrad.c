/******************************************************************************
 * 
 * Relay card control utility: Driver for Conrad USB 4-relay card
 *   http://www.conrad.de/ce/de/product/393905
 * 
 * Description:
 *   This software is used to control the Conrad USB 4-relay card.
 *   This file contains the implementation of the specific functions.
 * 
 * Author:
 *   Ondrej Wisniewski (ondrej.wisniewski *at* gmail.com)
 *
 * Build instructions:
 *   gcc -c relay_drv_conrad.c -lusb-1.0
 * 
 * Last modified:
 *   19/08/2015
 *
 * Copyright 2015, Ondrej Wisniewski 
 * 
 * This file is part of crelay.
 * 
 * crelay is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with crelay.  If not, see <http://www.gnu.org/licenses/>.
 * 
 *****************************************************************************/ 

/******************************************************************************
 * Communication protocol description
 * ==================================
 * 
 * The Silabs CP2104 USB to UART Bridge Controller is used in GPIO mode.
 * These bit assignments are valid for the driver Silabs provides for the
 * Kernel version 3.13 and higher.
 * Communication with the controller chip is implemented through libusb 
 * control messages and does not need the cp210x driver. However, the
 * structure of the control messages used here is derived from the 
 * implementation of the GPIO handling in the driver, so it does essentially
 * the same thing, but from a user space program.
 * 
 * Get relay status:
 * -----------------
 *  7  6  5  4    3  2  1  0   bit no
 *  X  X  X  X   R4 R3 R2 R1   relay state
 * 
 * 
 * Set relay status:
 * -----------------
 *  15 14 13 12   11 10 9  8   bit no
 *  X  X  X  X   R4 R3 R2 R1   relay state to set
 * 
 *  7  6  5  4    3  2  1  0   bit no
 *  X  X  X  X   R4 R3 R2 R1   relay bit mask
 * 
 * Relay names:
 *  R1: relay 1
 *  R2: relay 2
 *  R3: relay 3
 *  R4: relay 4
 * 
 * Meaning of bit values:
 *  0: NO contact closed, NC contact open, led is on
 *  1: NO contact open, NC contact closed, led is off
 * 
 *****************************************************************************/ 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <libusb-1.0/libusb.h>

#include "data_types.h"
#include "relay_drv.h"

/* USB IDs */
#define VENDOR_ID 0x10C4
#define DEVICE_ID 0xEA60

/* Config request types */
#define REQTYPE_HOST_TO_DEVICE  0x40
#define REQTYPE_DEVICE_TO_HOST  0xc0

/* Config request codes */
#define CP210X_VENDOR_SPECIFIC  0xFF

/* CP210X_VENDOR_SPECIFIC */
#define CP210X_WRITE_LATCH      0x37E1
#define CP210X_READ_LATCH       0x00C2

#define RSTATES_BITOFFSET 8


static libusb_device *device;


/**********************************************************
 * Function detect_relay_card_conrad_4chan()
 * 
 * Description: Detect the Conrad USB relay card
 * 
 * Parameters: portname (out) - pointer to a string where
 *                              the detected com port will
 *                              be stored
 *             num_relays(out)- pointer to number of relays
 * 
 * Return:  0 - success
 *         -1 - fail, no relay card found
 *********************************************************/
int detect_relay_card_conrad_4chan(char* portname, uint8* num_relays, char* serial)
{
   struct libusb_device_handle *dev = NULL; 
   struct libusb_device_descriptor devdesc;
   int r;
   unsigned char sernum[64];
   
   libusb_init(NULL);
   
   /* Try to open Conrad CP2104 USB device */
   dev = libusb_open_device_with_vid_pid(NULL, VENDOR_ID, DEVICE_ID);
   if (dev == NULL)
   {
      libusb_exit(NULL);
      return -1;
   }
   
   /* Get device reference (can be used later for libusb_open() calls) */
   device = libusb_get_device(dev);

   /* Get device descripter */
   r=libusb_get_device_descriptor (libusb_get_device(dev), &devdesc);
   if (r < 0)
   {
      fprintf(stderr, "unable to get device descripter (%s)\n", libusb_error_name(r));
      libusb_exit(NULL);
      return -1;
   }

   /* Get serial number */
   r=libusb_get_string_descriptor_ascii (dev, devdesc.iSerialNumber, sernum, 64);
   if (r < 0)
   {
      fprintf(stderr, "unable to get device descripter (%s)\n", libusb_error_name(r));
      libusb_exit(NULL);
      return -1;
   }
   
   /* Return parameters */
   if (num_relays!=NULL) *num_relays = CONRAD_4CHANNEL_USB_NUM_RELAYS;
   sprintf(portname, "Serial number %s", sernum);
   libusb_close(dev);
   libusb_exit(NULL);
   
   return 0;
}


/**********************************************************
 * Function get_relay_conrad_4chan()
 * 
 * Description: Get the current relay state
 * 
 * Parameters: portname (in)     - communication port
 *             relay (in)        - relay number
 *             relay_state (out) - current relay state
 * 
 * Return:   0 - success
 *          -1 - fail
 *********************************************************/
int get_relay_conrad_4chan(char* portname, uint8 relay, relay_state_t* relay_state, char* serial)
{
   struct libusb_device_handle *dev = NULL; 
   int r;  
   uint8 gpio=0;
   
   if (relay<FIRST_RELAY || relay>(FIRST_RELAY+CONRAD_4CHANNEL_USB_NUM_RELAYS-1))
   {  
      fprintf(stderr, "ERROR: Relay number out of range\n");
      return -1;      
   }

   libusb_init(NULL);
   
   /* Open USB device */
   //r = libusb_open(device, &dev);
   //if (r < 0)
   dev = libusb_open_device_with_vid_pid(NULL, VENDOR_ID, DEVICE_ID);
   if (dev == NULL)
   {
      fprintf(stderr, "unable to open CP2104 device\n");
      libusb_exit(NULL);
      return -2;
   }
   
   /* Get relay state from the card */ 
   r = libusb_control_transfer (
                dev,                    // libusb_device_handle *  dev_handle,
                REQTYPE_DEVICE_TO_HOST, // uint8_t         bmRequestType,
                CP210X_VENDOR_SPECIFIC, // uint8_t         bRequest,
                CP210X_READ_LATCH,      // uint16_t        wValue,
                0,                      // uint16_t        wIndex,
                &gpio,                  // unsigned char * data,
                1,                      // uint16_t        wLength,
                0);                     // unsigned int    timeout

   if (r < 0) 
   {
      fprintf(stderr, "libusb_control_transfer error (%s)\n", libusb_error_name(r));
      libusb_close(dev);
      libusb_exit(NULL);
      return -3;
   }

   relay = relay-1;
   *relay_state = (gpio & (0x0001<<relay)) ? OFF : ON;
      
   libusb_close(dev);
   libusb_exit(NULL);
   return 0;
}


/**********************************************************
 * Function set_relay_conrad_4chan()
 * 
 * Description: Set new relay state
 * 
 * Parameters: portname (in)     - communication port
 *             relay (in)        - relay number
 *             relay_state (in)  - current relay state
 * 
 * Return:   o - success
 *          -1 - fail
 *********************************************************/
int set_relay_conrad_4chan(char* portname, uint8 relay, relay_state_t relay_state, char* serial)
{
   struct libusb_device_handle *dev = NULL; 
   int r;  
   uint16 gpio=0;
   
   if (relay<FIRST_RELAY || relay>(FIRST_RELAY+CONRAD_4CHANNEL_USB_NUM_RELAYS-1))
   {  
      fprintf(stderr, "ERROR: Relay number out of range\n");
      return -1;      
   }
   
   libusb_init(NULL);
   
   /* Open USB device */
   //r = libusb_open(device, &dev);
   //if (r < 0)
   dev = libusb_open_device_with_vid_pid(NULL, VENDOR_ID, DEVICE_ID);
   if (dev == NULL)
   {
      fprintf(stderr, "unable to open CP2104 device\n");
      libusb_exit(NULL);
      return -2;
   }
   
   /* Set the relay state bit */
   relay = relay-1;
   if (relay_state == OFF) gpio = 0x0001<<(relay+RSTATES_BITOFFSET);
   
   /* Set the relay bit mask */
   gpio = gpio | (0x0001<<relay);

   /* Set relay state on the card */ 
   r = libusb_control_transfer (
                dev,                    // libusb_device_handle *  dev_handle,
                REQTYPE_HOST_TO_DEVICE, // uint8_t         bmRequestType,
                CP210X_VENDOR_SPECIFIC, // uint8_t         bRequest,
                CP210X_WRITE_LATCH,     // uint16_t        wValue,
                gpio,                   // uint16_t        wIndex,
                NULL,                   // unsigned char * data,
                0,                      // uint16_t        wLength,
                0);                     // unsigned int    timeout
   
   if (r < 0) 
   {
      fprintf(stderr, "libusb_control_transfer error (%s)\n", libusb_error_name(r));
      libusb_close(dev);
      libusb_exit(NULL);
      return -3;
   }

   libusb_close(dev);
   libusb_exit(NULL);
   return 0;
}
