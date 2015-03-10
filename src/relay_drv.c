/******************************************************************************
 * 
 * Relay card control utility: Relay driver generic file
 * 
 * Description:
 *   This software is used to controls different type of relays cards.
 *   This file contains the implementation of the generic functions.
 * 
 * Author:
 *   Ondrej Wisniewski (ondrej.wisniewski *at* gmail.com)
 *
 * Last modified:
 *   07/03/2015
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "data_types.h"
#include "relay_drv.h"

/* Card driver specific include files */
#include "relay_drv_conrad.h"
#include "relay_drv_sainsmart.h"
#include "relay_drv_hidapi.h"
#include "relay_drv_gpio.h"


static relay_type_t relay_type=NO_RELAY_TYPE;

/*
 *  Table which holds the specific relay card data:
 *    - function to detect the communication port
 *    - function to get the current relay state
 *    - function to set the new relay state
 *    - card name string
 *    - number of relays on the card
 * 
 *  The index into the table is the relay card type.
 */
static relay_data_t relay_data[LAST_RELAY_TYPE] =
{ 
   {  // NO_RELAY_TYPE (dummy entry)
      NULL, NULL, NULL, "", 0
   },
#ifdef DRV_CONRAD
   {  // CONRAD_4CHANNEL_USB_RELAY_TYPE
      detect_com_port_conrad_4chan,
      get_relay_conrad_4chan,
      set_relay_conrad_4chan,
      CONRAD_4CHANNEL_USB_NAME,
      CONRAD_4CHANNEL_USB_NUM_RELAYS
   },
#endif
#ifdef DRV_SAINSMART
   {  // SAINSMART_4CHANNEL_USB_RELAY_TYPE
      detect_com_port_sainsmart_4chan,
      get_relay_sainsmart_4chan,
      set_relay_sainsmart_4chan,
      SAINSMART_4CHANNEL_USB_NAME,
      SAINSMART_4CHANNEL_USB_NUM_RELAYS
   },
#endif
#ifdef DRV_HIDAPI
   {  // HID_API_RELAY_TYPE
      detect_com_port_hidapi,
      get_relay_hidapi,
      set_relay_hidapi,
      HID_API_RELAY_NAME,
      HID_API_NUM_RELAYS
   },
#endif
   {  // GENERIC_GPIO_RELAY_TYPE
      detect_com_port_generic_gpio,
      get_relay_generic_gpio,
      set_relay_generic_gpio,
      GENERIC_GPIO_NAME,
      GENERIC_GPIO_NUM_RELAYS
   }
};


/**********************************************************
 * Function detect_com_port()
 * 
 * Description: Detect the port used for communicating 
 *              with the relay card
 * 
 * Parameters: portname (out) - pointer to a string where
 *                              the detected com port will
 *                              be stored
 * 
 * Return:  0 - success
 *         -1 - fail, no relay card found
 *********************************************************/
int detect_com_port(char* portname)
{
   int i;
   
   for (i=1; i<LAST_RELAY_TYPE; i++)
   {
      if ((*relay_data[i].detect_com_port_fun)(portname) == 0)
      {
         relay_type=i;
         return 0;
      }
   }
   
   relay_type = NO_RELAY_TYPE;
   return -1;   
}


/**********************************************************
 * Function get_relay()
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
int get_relay(char* portname, uint8 relay, relay_state_t* relay_state)
{
   if (relay_type != NO_RELAY_TYPE)
   {
      return (*relay_data[relay_type].get_relay_fun)(portname, relay, relay_state);
   }
   else
   {   
      return -1;
   }
}


/**********************************************************
 * Function set_relay()
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
int set_relay(char* portname, uint8 relay, relay_state_t relay_state)
{
   if (relay_type != NO_RELAY_TYPE)
   {
      return (*relay_data[relay_type].set_relay_fun)(portname, relay, relay_state);
   }
   else
   {   
      return -1;
   }
}


/**********************************************************
 * Function get_relay_card_type()
 * 
 * Description: Get the detected relay type
 * 
 * Parameters: none
 * 
 * Return: relay type
 *********************************************************/
relay_type_t get_relay_card_type()
{
   return relay_type;
}


/**********************************************************
 * Function get_relay_card_name()
 * 
 * Description: Get the relay card name for a relay type
 * 
 * Parameters: rtype           - relay type
 *             card_name (out) - pointer to a string where
 *                               the detected com port will
 *                               be stored
 * 
 * Return:  0 - success
 *         -1 - fail, no relay card found
 *********************************************************/
int get_relay_card_name(relay_type_t rtype, char* card_name)
{
   if (rtype != NO_RELAY_TYPE)
   {
      strcpy(card_name, relay_data[rtype].card_name);
      return 0;
   }
   else
   {   
      return -1;
   }  
}


/**********************************************************
 * Function get_last_relay_num()
 * 
 * Description: Get the number of the last relay for
 *              the detected card
 * 
 * Parameters: none
 * 
 * Return: last relay number
 *********************************************************/
int get_last_relay_num()
{
   if (relay_type != NO_RELAY_TYPE)
   {
      return FIRST_RELAY+relay_data[relay_type].num_relays-1;
   }
   else
   {
      return FIRST_RELAY;
   }
}
