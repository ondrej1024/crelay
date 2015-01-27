/******************************************************************************
 * 
 * Relay card control utility: Driver for Sainsmart USB 4-channel relay card
 *   http://www.sainsmart.com/sainsmart-4-channel-5v-usb-relay-board-module-controller-for-automation-robotics.html
 * 
 * Note:
 *   We need libFTDI, the open source FTDI USB driver library with bitbang mode.
 *   http://www.intra2net.com/en/developer/libftdi/
 *   To install the library and development files on a Debian based system:
 *   apt-get install libftdi1 libftdi-dev
 * 
 * Description:
 *   This software is used to control the Sainsmart USB 4-channel relay card.
 *   This file contains the implementation of the specific functions.
 * 
 * Author:
 *   Ondrej Wisniewski (ondrej.wisniewski *at* gmail.com)
 *
 * Build instructions:
 *   gcc -c relay_drv_sainsmart.c
 * 
 * Last modified:
 *   26/01/2015
 *
 *****************************************************************************/ 

/******************************************************************************
 * Communication protocol description
 * ==================================
 * 
 * These are the bit assignments. All bits are read and written at the same time.
 * 
 * Relay status/control:
 * ---------------------
 *  7  6  5  4    3  2  1  0   bit no
 *  X  X  X  X   R4 R3 R2 R1   relay state
 * 
 * 
 * Relay names:
 *  R1: relay 1
 *  R2: relay 2
 *  R3: relay 3
 *  R4: relay 4
 * 
 * Meaning of bit values:
 *  0: NO contact open, NC contact closed, led is off
 *  1: NO contact closed, NC contact open, led is on
 * 
 *****************************************************************************/ 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <ftdi.h>

#include "data_types.h"
#include "relay_drv.h"

#define VENDOR_ID 0x0403
#define DEVICE_ID 0x6001

static struct ftdi_context *ftdi;


/**********************************************************
 * Function detect_com_port_sainsmart_4chan()
 * 
 * Description: Detect the port used for communicating 
 *              with the Conrad USB relay card
 * 
 * Parameters: portname (out) - pointer to a string where
 *                              the detected com port will
 *                              be stored
 * 
 * Return:  0 - success
 *         -1 - fail, no relay card found
 *********************************************************/
int detect_com_port_sainsmart_4chan(char* portname)
{
   unsigned int chipid;
   
   if ((ftdi = ftdi_new()) == 0)
   {
      fprintf(stderr, "ftdi_new failed\n");
      return -1;
   }

   /* Try to open FTDI USB device */
   if ((ftdi_usb_open(ftdi, VENDOR_ID, DEVICE_ID)) < 0)
   {
      ftdi_free(ftdi);
      return -1;
   }
   
   /* Set FTDI chip to bitbang mode */
   if (ftdi_set_bitmode(ftdi, 0xFF, BITMODE_BITBANG) < 0)
   {
      fprintf(stderr, "unable to set bitbang mode: (%s)\n", ftdi_get_error_string(ftdi));
      ftdi_free(ftdi);
      return -1;
   }

   /* Check if this is an R type chip */
   if (ftdi->type != TYPE_R)
   {
      fprintf(stderr, "unable to continue, not an R-type chip\n");
      ftdi_free(ftdi);
      return -1;
   }
   
   /* Read out FTDI Chip-ID of R type chips */
   ftdi_read_chipid(ftdi, &chipid);
   sprintf(portname, "FTDI chipid %X", chipid);
   //printf("DBG: portname %s\n", portname);
   
   ftdi_usb_close(ftdi);   
   return 0;
}


/**********************************************************
 * Function get_relay_sainsmart_4chan()
 * 
 * Description: Get the current relay state
 * 
 * Parameters: portname (in)     - communication port
 *             relay (in)        - relay number
 *             relay_state (out) - current relay state
 * 
 * Return:    0 - success
 *          < 0 - fail
 *********************************************************/
int get_relay_sainsmart_4chan(char* portname, uint8 relay, relay_state_t* relay_state)
{
   unsigned char buf[1];
   
   if (relay<FIRST_RELAY || relay>(FIRST_RELAY+SAINSMART_4CHANNEL_USB_NUM_RELAYS-1))
   {  
      fprintf(stderr, "ERROR: Relay number out of range\n");
      return -1;      
   }

   /* Open FTDI USB device */
   if ((ftdi_usb_open(ftdi, VENDOR_ID, DEVICE_ID)) < 0)
   {
      fprintf(stderr, "unable to open ftdi device: (%s)\n", ftdi_get_error_string(ftdi));
      ftdi_free(ftdi);
      return -2;
   }
   
   /* Get relay state from the card */
   if (ftdi_read_pins(ftdi, &buf[0]) < 0)
   {
      fprintf(stderr,"read failed for 0x%x, error %s\n",buf[0], ftdi_get_error_string(ftdi));
      return -3;
   }
   //printf("DBG: Read GPIO bits %02X\n", buf[0]);
   relay = relay-1;
   *relay_state = (buf[0] & (0x01<<relay)) ? ON : OFF;

   ftdi_usb_close(ftdi);
   return 0;
}


/**********************************************************
 * Function set_relay_sainsmart_4chan()
 * 
 * Description: Set new relay state
 * 
 * Parameters: portname (in)     - communication port
 *             relay (in)        - relay number
 *             relay_state (in)  - current relay state
 * 
 * Return:    0 - success
 *          < 0 - fail
 *********************************************************/
int set_relay_sainsmart_4chan(char* portname, uint8 relay, relay_state_t relay_state)
{
   unsigned char buf[1];
   
   if (relay<FIRST_RELAY || relay>(FIRST_RELAY+SAINSMART_4CHANNEL_USB_NUM_RELAYS-1))
   {  
      fprintf(stderr, "ERROR: Relay number out of range\n");
      return -1;      
   }
   
   /* Open FTDI USB device */
   if ((ftdi_usb_open(ftdi, VENDOR_ID, DEVICE_ID)) < 0)
   {
      fprintf(stderr, "unable to open ftdi device: (%s)\n", ftdi_get_error_string(ftdi));
      ftdi_free(ftdi);
      return -2;
   }

   /* Get relay state from the card */
   if (ftdi_read_pins(ftdi, buf) < 0)
   {
      fprintf(stderr,"read failed for 0x%x, error %s\n",buf[0], ftdi_get_error_string(ftdi));
      return -3;
   }
   
   /* Set the new relay state bit */
   relay = relay-1;
   if (relay_state == OFF) 
   {
      /* Clear the relay bit in mask */
      buf[0] = buf[0] & ~(0x01<<relay);
   }
   else
   {
      /* Set the relay bit in mask */
      buf[0] = buf[0] | (0x01<<relay);
   }
   
   //printf("DBG: Writing GPIO bits %02X\n", buf[0]);
   
   /* Set relay on the card */
   if (ftdi_write_data(ftdi, buf, 1) < 0)
   {
      fprintf(stderr,"read failed for 0x%x, error %s\n",buf[0], ftdi_get_error_string(ftdi));
      return -4;
   }
   
   ftdi_usb_close(ftdi);
   return 0;
}
