/******************************************************************************
 * 
 * Relay card control utility: Driver for Sainsmart 16-channel USB-HID relay
 * control module:
 * 
 * Note:
 *   We need libhidapi, a multi-platform library which allows an application 
 *   to interface with USB and Bluetooth HID-Class devices:
 *   http://www.signal11.us/oss/hidapi
 *   To install the library and development files on a Debian based system:
 *   apt-get install libhidapi-libusb0 libhidapi-dev
 * 
 * Description:
 *   This 16-channel module is used for USB control of the Sainsmart 16-channel
 *   relays.
 *   http://www.sainsmart.com/sainsmart-16-channel-controller-usb-hid-programmable-control-relay-module.html
 * 
 * Author:
 *   Kevin Hilman (khilman *at* kernel.org)
 *
 *   Based on the Node.js relay project by Michael Vines:
 *   * https://github.com/mvines/relay
 * 
 * Build instructions:
 *   gcc -c relay_drv_hidapi_sain.c
 * 
 * Last modified:
 *   28/03/2016
 *
 * Copyright 2016, Kevin Hilman
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <hidapi/hidapi.h>

#include "data_types.h"
#include "relay_drv.h"

#define VENDOR_ID 0x0416
#define DEVICE_ID 0x5020

static uint8 g_num_relays=HID_API_SAIN_NUM_RELAYS;

static unsigned char read_cmd[] = {
  0xD2, 0x0E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x48, 0x49, 0x44,
  0x43, 0x80, 0x02,
};

static unsigned char write_cmd[] = {
  0xC3, 0x0E, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x48, 0x49, 0x44,
  0x43, 0xEE, 0x01, 
};

static unsigned short
mask_map[] = {128, 256, 64, 512, 32, 1024, 16, 2048, 8, 4096, 4, 8192, 2, 16384, 1, 32768};

static unsigned short get_mask(hid_device *handle) {
  int i, res;
  unsigned short mask, read_mask;
  unsigned char buf[16];
  unsigned short *buf16 = (unsigned short *)buf;
  
  res = hid_write(handle, read_cmd, sizeof(read_cmd));
  if (res < 0)
    printf("Unable to read()\n");
  usleep(1000);
  
  res = hid_read(handle, buf, sizeof(buf));
  if (res < 0)
    printf("Unable to read()\n");
     
  mask = 0;
  read_mask = buf16[1];
  
  for ( i = 0 ; i < g_num_relays; i ++) {
    if (read_mask & mask_map[i]) {
      mask |= 1 << i;
    }
  }

  /* printf("DBG: get_mask = 0x%04x\n", mask); */
  return mask;
}

static void set_mask(hid_device *handle, unsigned short mask) {
  int i, res;
  unsigned int size;
  unsigned short checksum = 0, *write_cmd16 = (unsigned short *)write_cmd;
  
  /* printf("DBG: set_mask = 0x%04x\n", mask); */
  
  write_cmd16[1] = mask;
  size = write_cmd[1];
  for (i = 0; i < size; i++)
    checksum += write_cmd[i];
  write_cmd16[size/2] = checksum;
  
  res = hid_write(handle, write_cmd, sizeof(write_cmd));
  if (res < 0)
    printf("Unable to write()\n");
}

/**********************************************************
  * Function detect_relay_card_hidapi_sain()
 * 
 * Description: Detect the HID API compatible relay card
 * 
 * Parameters: portname (out) - pointer to a string where
 *                              the detected com port will
 *                              be stored
 *             num_relays(out)- pointer to number of relays
 * 
 * Return:  0 - success
 *         -1 - fail, no relay card found
 *********************************************************/
int detect_relay_card_hidapi_sain(char* portname, uint8* num_relays)
{
   struct hid_device_info *devs;
   
   if ((devs = hid_enumerate(VENDOR_ID, DEVICE_ID)) == NULL)
   {
      return -1;  
   }

   if (devs->product_string == NULL ||
       devs->path == NULL)
   {
      return -1;
   }
   
   /* Return parameters */
   if (num_relays!=NULL) *num_relays = g_num_relays;
   sprintf(portname, "%s", devs->path);
  
   /* printf("DBG: card %ls found, portname=%s\n", devs->product_string, portname); */
   
   hid_free_enumeration(devs);   
   return 0;
}


/**********************************************************
 * Function get_relay_hidapi_sain()
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
int get_relay_hidapi_sain(char* portname, uint8 relay, relay_state_t* relay_state)
{
   hid_device *hid_dev;
   unsigned short mask, bit;
   
   if (relay<FIRST_RELAY || relay>(FIRST_RELAY+g_num_relays-1))
   {  
      fprintf(stderr, "ERROR: Relay number out of range\n");
      return -1;      
   }

   /* Open HID API device */
   if ((hid_dev = hid_open_path(portname)) == NULL)
   {
      fprintf(stderr, "unable to open HID API device %s\n", portname);
      return -2;
   }

   mask = get_mask(hid_dev);
   bit = 1 << relay;
   if (mask & bit)
     *relay_state = ON;
   else
     *relay_state = OFF;

   /* printf("DBG: get: portname=%s, relay=%d, state=%d\n", portname, relay, (int)*relay_state); */
   hid_close(hid_dev);
   return 0;
}


/**********************************************************
 * Function set_relay_hidapi_sain()
 * 
 * Description: Set new relay state
 * 
 * Parameters: portname (in)     - communication port
 *             relay (in)        - relay number
 *             relay_state (in)  - current relay state
 * 
 * Return:   0 - success
 *          -1 - fail
 *********************************************************/
int set_relay_hidapi_sain(char* portname, uint8 relay, relay_state_t relay_state)
{ 
   hid_device *hid_dev;
   unsigned int bit;
   unsigned short mask;
   
   if (relay<FIRST_RELAY || relay>(FIRST_RELAY+g_num_relays-1))
   {  
      fprintf(stderr, "ERROR: Relay number out of range\n");
      return -1;      
   }

   /* Open HID API device */
   if ((hid_dev = hid_open_path(portname)) == NULL)
   {
      fprintf(stderr, "unable to open HID API device %s\n", portname);
      return -2;
   }

   printf("DBG: Sain USB-HID: set: portname=%s, relay=%d, state=%d\n", portname, relay, (int)relay_state);

   mask = get_mask(hid_dev);
   bit = 1 << (relay - 1);
   if (relay_state)
     mask |= bit;
   else
     mask &= ~bit;
  
   set_mask(hid_dev, mask);
   
   hid_close(hid_dev);
   return 0;
}
