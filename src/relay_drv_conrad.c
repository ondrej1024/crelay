/******************************************************************************
 * 
 * Relay card control utility: Driver for Conrad USB 4-relay card
 *   http://www.conrad.de/ce/de/product/393905
 * 
 * Note:
 *   We need the cp210x driver for the Silabs CP2104 chip with GPIO support.
 *   The official in-kernel cp210x driver does currently not yet support
 *   GPIO operations. Therefore the Silabs driver needs to be used:
 *   http://www.silabs.com/products/mcu/pages/usbtouartbridgevcpdrivers.aspx
 * 
 * Description:
 *   This software is used to control the Conrad USB 4-relay card.
 *   This file contains the implementation of the specific functions.
 * 
 * Author:
 *   Ondrej Wisniewski (ondrej.wisniewski *at* gmail.com)
 *
 * Build instructions:
 *   gcc -c relay_drv_conrad.c
 * 
 * Last modified:
 *   31/10/2014
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

#include "data_types.h"
#include "relay_drv.h"

#define IOCTL_GPIOGET 0x8000
#define IOCTL_GPIOSET 0x8001

#define RSTATES_BITOFFSET 8

/* USB serial device created by cp210x driver */
#define SERIAL_DEV_BASE "/dev/ttyUSB"
#define MAX_COM_PORTS 10


/**********************************************************
 * Function detect_com_port_conrad_4chan()
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
int detect_com_port_conrad_4chan(char* portname)
{
   uint8 found=0;
   uint32 gpio=0;
   int fd;
   int i;
   char pname[16];
   
   for (i=0; i<MAX_COM_PORTS; i++)
   { 
      /* Try to open USB serial device */
      sprintf(pname, "%s%d", SERIAL_DEV_BASE, i);
      //printf("DBG: Trying serial device %s\n", pname);
      fd = open(pname, O_RDONLY | O_NOCTTY | O_NDELAY);
      if (fd != -1) 
      {
         /* Now try to read the GPIO pins, this will work only
          * for a CP210x device with GPIO support in the driver 
          */
         if (ioctl(fd, IOCTL_GPIOGET, &gpio) != -1)
         {
            /* It works, we found a compatible device */
            found=1;
            break;
         }
      }
   }
   
   if (found) 
   {
      strcpy(portname, pname);
      close(fd);
      return 0;
   }
   
   return -1;
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
int get_relay_conrad_4chan(char* portname, uint8 relay, relay_state_t* relay_state)
{
   uint32 gpio=0;
   int fd;
   int rc;
   
   if (relay<FIRST_RELAY || relay>(FIRST_RELAY+CONRAD_4CHANNEL_USB_NUM_RELAYS-1))
   {  
      fprintf(stderr, "ERROR: Relay number out of range\n");
      return -1;      
   }

   /* Open serial device */
   fd = open(portname, O_RDWR | O_NOCTTY | O_NDELAY);
   if (fd == -1) 
   {
      fprintf(stderr, "ERROR: Failed to open %s: %s\n", portname, strerror(errno));
      return -2;      
   }
  
   /* Get relay state from the card via IOCTL */ 
   rc = ioctl(fd, IOCTL_GPIOGET, &gpio);
   if (rc == -1)
   {
      fprintf(stderr, "IOCTL_GPIOGET failed: %s\n", strerror(errno));
      return -3;
   }

   //printf("DBG: Read GPIO bits %08lX\n", gpio);   
   relay = relay-1;
   *relay_state = (gpio & (0x0001<<relay)) ? OFF : ON;
      
   close(fd);
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
int set_relay_conrad_4chan(char* portname, uint8 relay, relay_state_t relay_state)
{
   uint32 gpio=0;
   int fd;
   int rc;
   
   if (relay<FIRST_RELAY || relay>(FIRST_RELAY+CONRAD_4CHANNEL_USB_NUM_RELAYS-1))
   {  
      fprintf(stderr, "ERROR: Relay number out of range\n");
      return -1;      
   }
   
   /* Open serial device */
   fd = open(portname, O_RDWR | O_NOCTTY | O_NDELAY);
   if (fd == -1) 
   {
      fprintf(stderr, "ERROR: Failed to open %s: %s\n", portname, strerror(errno));
      return -2;      
   }

   /* Set the relay state bit */
   relay = relay-1;
   if (relay_state == OFF) gpio = 0x0001<<(relay+RSTATES_BITOFFSET);
   
   /* Set the relay bit mask */
   gpio = gpio | (0x0001<<relay);

   //printf("DBG: Writing GPIO bits %08lX\n", gpio);
   
   /* Set relay on the card via IOCTL */
   rc = ioctl(fd, IOCTL_GPIOSET, &gpio);
   if (rc == -1)
   {
      fprintf(stderr, "IOCTL_GPIOSET failed: %s\n", strerror(errno));
      return -3;
   }
   
   return 0;
}
 
