/******************************************************************************
 * 
 * Relay card control utility: Driver for HID API compatible relay cards
 * 
 * Note:
 *   We need libhidapi, a multi-platform library which allows an application 
 *   to interface with USB and Bluetooth HID-Class devices:
 *   http://www.signal11.us/oss/hidapi
 *   To install the library and development files on a Debian based system:
 *   apt-get install libhidapi-libusb0 libhidapi-dev
 * 
 * Description:
 *   This software is used to control the HID API compatible relay cards.
 *   This file contains the implementation of the specific functions.
 * 
 * Author:
 *   Ondrej Wisniewski (ondrej.wisniewski *at* gmail.com)
 *
 *   Based on the work of Darryl Bond and David Buechi
 *   https://github.com/darrylb123/usbrelay
 *   https://github.com/davidbuechi/UsbRelay
 * 
 * Build instructions:
 *   gcc -c relay_drv_hidapi.c
 * 
 * Last modified:
 *   21/02/2016
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
 * Relay Status Request
 * -----------------------------
 * 
 * Send the following buffer using
 * hid_get_feature_report():
 * 
 * Byte  1 2 3 4 5 6 7 8 9
 * Data  1 - - - - - - - -
 * 
 * 
 * The following buffer is received
 * as response:
 * 
 * Byte  1 2 3 4 5 6 7 8 9
 * Data  C C C C C 0 ? S ?
 * 
 * C: 5 characters which form the relay cards Identity string
 * S: status byte where each bit indicates the state of the corresponding relay
 * 
 * 
 * Relay State Setting
 * -----------------------------
 * 
 * Send the following output report using
 * hid_write()
 * 
 * Byte  1 2 3 4 5 6 7 8 9
 * Data  0 S R 0 0 0 0 0 0
 * 
 * S: Relay state (0xff=on, 0xfe=all_on, 0xfd=off, 0xfc=all_off)
 * R: Relay number (integer)
 * 
 *****************************************************************************/ 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <hidapi/hidapi.h>

#include "data_types.h"
#include "relay_drv.h"

#define VENDOR_ID 0x16c0
#define DEVICE_ID 0x05df

#define PRODUCT_STR_BASE "USBRelay"

#define REPORT_LEN          9
#define REPORT_RDDAT_OFFSET 7
#define REPORT_WRCMD_OFFSET 1
#define REPORT_WRREL_OFFSET 2

#define CMD_ON      0xff
#define CMD_ALL_ON  0xfe
#define CMD_OFF     0xfd
#define CMD_ALL_OFF 0xfc

static uint8 g_num_relays=HID_API_NUM_RELAYS;


/**********************************************************
 * Function detect_relay_card_hidapi()
 * 
 * Description: Detect the HID API compatible relay card
 * 
 * Parameters: portname (out) - pointer to a string where
 *                              the detected com port will
 *                              be stored
 *             num_relays(out)- pointer to number of relays
 *             serial(in)     - pointer to a string containing
 *                              serial number [optional]
 *
 * Return:  0 - success
 *         -1 - fail, no relay card found
 *********************************************************/
int detect_relay_card_hidapi(char* portname, uint8* num_relays, char* serial, relay_info_t** relay_info)
{
   struct hid_device_info *devs, *nextdev;
   hid_device *hid_dev;
   unsigned char buf[REPORT_LEN];  
   uint8 found=0;
   uint8 num;
   relay_info_t* rinfo;

   
   if ((devs = hid_enumerate(VENDOR_ID, DEVICE_ID)) == NULL)
   {
      return -1;  
   }

   if (devs->product_string == NULL ||
       devs->path == NULL)
   {
      return -2;
   }

   /* Find controller with matching serial number if it's specified */
   buf[0]=0;
   nextdev = devs;
   while (nextdev)
   {
      /* Open HID API device */
      if ((hid_dev = hid_open_path(nextdev->path)) == NULL)
      {
         fprintf(stderr, "unable to open HID API device %s\n", portname);
         return -3;
      }
      
      /* Read relay Id requesting a feature report with Id 0x01 */
      buf[0] = 0x01;
      if (hid_get_feature_report(hid_dev, buf, sizeof(buf)) != REPORT_LEN)
      {
         fprintf(stderr, "unable to read feature report from device %s (%ls)\n", portname, hid_error(hid_dev));
         return -4;
      }
      //printf("DBG: Relay ID: %s\n", buf);
      
      hid_close(hid_dev);
      
      if (relay_info != NULL)
      {
         // Save serial number and type in current relay info struct
         (*relay_info)->relay_type = HID_API_RELAY_TYPE;
         strcpy((*relay_info)->serial, (char *)buf);
         // Allocate new struct
         rinfo = malloc(sizeof(relay_info_t));
         rinfo->next = NULL;
         // Link current to new struct
         (*relay_info)->next = rinfo;
         // Move pointer to new struct
         *relay_info = rinfo;
      } 
      else if (serial == NULL || !strcmp(serial, (char *)buf))
      {
         found = 1;
         break;
      }
      nextdev = nextdev->next;
   }
   
   if (found == 0)
   {
      return -5;
   }
   
   //printf("DBG: card %ls found\n", devs->product_string);
   
   /* Get number of relays from product description */
   num = atoi((const char *)(nextdev->product_string+strlen(PRODUCT_STR_BASE)));
   if (num>0)
   {
      g_num_relays = num;
   }

   /* Return parameters */
   if (num_relays!=NULL) *num_relays = g_num_relays;
   sprintf(portname, "%s", nextdev->path);
  
   hid_free_enumeration(devs);   
   return 0;
}


/**********************************************************
 * Function get_relay_hidapi()
 * 
 * Description: Get the current relay state
 * 
 * Parameters: portname (in)     - communication port
 *             relay (in)        - relay number
 *             relay_state (out) - current relay state
 *             serial (in)       - serial number [not used]
 *
 * Return:   0 - success
 *          -1 - fail
 *********************************************************/
int get_relay_hidapi(char* portname, uint8 relay, relay_state_t* relay_state, char* serial)
{
   hid_device *hid_dev;
   unsigned char buf[REPORT_LEN];  

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

   /* Read relay states requesting a feature report with Id 0x01 */
   buf[0] = 0x01;
   if (hid_get_feature_report(hid_dev, buf, sizeof(buf)) != REPORT_LEN)
   {
      fprintf(stderr, "unable to read feature report from device %s (%ls)\n", portname, hid_error(hid_dev));
      return -3;
   }
   //printf("DBG: Relay ID: %s\n", buf);
   //printf("DBG: Read relay bits %02X\n", buf[REPORT_RDDAT_OFFSET]);
   relay = relay-1;
   *relay_state = (buf[REPORT_RDDAT_OFFSET] & (0x01<<relay)) ? ON : OFF;
   
   hid_close(hid_dev);
   return 0;
}


/**********************************************************
 * Function set_relay_hidapi()
 * 
 * Description: Set new relay state
 * 
 * Parameters: portname (in)     - communication port
 *             relay (in)        - relay number
 *             relay_state (in)  - current relay state
 *             serial (in)       - serial number [not used]
 *
 * Return:   o - success
 *          -1 - fail
 *********************************************************/
int set_relay_hidapi(char* portname, uint8 relay, relay_state_t relay_state, char* serial)
{ 
   hid_device *hid_dev;
   unsigned char buf[REPORT_LEN];  

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

   /* Write relay state by sending an output report to the device */
   memset(buf, 0, sizeof(buf));
   buf[REPORT_WRCMD_OFFSET] = (relay_state==ON) ? CMD_ON : CMD_OFF;
   buf[REPORT_WRREL_OFFSET] = relay;
   //printf("DBG: Write relay data %02X %02X\n", buf[REPORT_WRCMD_OFFSET], buf[REPORT_WRREL_OFFSET]);
   if (hid_write(hid_dev, buf, sizeof(buf)) < 0)
   {
      fprintf(stderr, "unable to write output report to device %s (%ls)\n", portname, hid_error(hid_dev));
      return -3;
   }
   
   hid_close(hid_dev);
   return 0;
}
