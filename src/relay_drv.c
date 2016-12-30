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
 *   30/12/2016
 *
 * Copyright 2015-2016, Ondrej Wisniewski 
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
#include <stdint.h>

#include "relay_drv.h"

/* Card driver specific include files */
#include "relay_drv_conrad.h"
#include "relay_drv_sainsmart.h"
#include "relay_drv_hidapi.h"
#include "relay_drv_sainsmart16.h"
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
      NULL, NULL, NULL, ""
   },
#ifdef DRV_CONRAD
   {  // CONRAD_4CHANNEL_USB_RELAY_TYPE
      detect_relay_card_conrad_4chan,
      get_relay_conrad_4chan,
      set_relay_conrad_4chan,
      CONRAD_4CHANNEL_USB_NAME
   },
#endif
#ifdef DRV_SAINSMART
   {  // SAINSMART_USB_RELAY_TYPE
      detect_relay_card_sainsmart_4_8chan,
      get_relay_sainsmart_4_8chan,
      set_relay_sainsmart_4_8chan,
      SAINSMART_USB_NAME
   },
#endif
#ifdef DRV_HIDAPI
   {  // HID_API_RELAY_TYPE
      detect_relay_card_hidapi,
      get_relay_hidapi,
      set_relay_hidapi,
      HID_API_RELAY_NAME
   },
#endif
#ifdef DRV_SAINSMART16
   {  // SAINSMART16_USB_RELAY_TYPE
      detect_relay_card_sainsmart_16chan,
      get_relay_sainsmart_16chan,
      set_relay_sainsmart_16chan,
      SAINSMART16_USB_NAME
   },
#endif
#ifndef BUILD_LIB
   {  // GENERIC_GPIO_RELAY_TYPE
      detect_relay_card_generic_gpio,
      get_relay_generic_gpio,
      set_relay_generic_gpio,
      GENERIC_GPIO_NAME
   }
#endif
};


/**********************************************************
 * Function crelay_detect_all_relay_cards()
 * 
 * Description: Detect all relay cards
 * 
 * Parameters: relay_info(out)- pointer to list of 
 *                              relays info struct
 * 
 * Return:  0 - success
 *         -1 - fail, no relay card found
 *********************************************************/
int crelay_detect_all_relay_cards(relay_info_t** relay_info)
{
   int i;
   relay_info_t* my_relay_info;
   
   /* Create first list element */
   my_relay_info = malloc(sizeof(relay_info_t));
   my_relay_info->next = NULL;

   /* Return pointer to first element to caller */
   *relay_info = my_relay_info;
   
   for (i=1; i<LAST_RELAY_TYPE; i++)
   {
      /* Create new list element with related info for each detected card */
      (*relay_data[i].detect_relay_card_fun)(NULL, NULL, NULL, &my_relay_info);
   }
   
   if ((*relay_info)->next == NULL)
      return -1;
   else
      return 0;
}


/**********************************************************
 * Function crelay_detect_relay_card()
 * 
 * Description: Detect the relay card
 * 
 * Parameters: portname (out) - pointer to a string where
 *                              the detected com port will
 *                              be stored
 *             num_relays(out)- pointer to number of relays
 * 
 * Return:  0 - success
 *         -1 - fail, no relay card found
 *********************************************************/
int crelay_detect_relay_card(char* portname, uint8_t* num_relays, char* serial, relay_info_t** my_relay_info)
{
   int i;
   
   for (i=1; i<LAST_RELAY_TYPE; i++)
   {
      if ((*relay_data[i].detect_relay_card_fun)(portname, num_relays, serial, NULL) == 0)
      {
         relay_type=i;
         return 0;
      }
   }
   
   relay_type = NO_RELAY_TYPE;
   return -1;   
}


/**********************************************************
 * Function crelay_get_relay()
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
int crelay_get_relay(char* portname, uint8_t relay, relay_state_t* relay_state, char* serial)
{
   if (relay_type != NO_RELAY_TYPE)
   {
      return (*relay_data[relay_type].get_relay_fun)(portname, relay, relay_state, serial);
   }
   else
   {   
      return -1;
   }
}


/**********************************************************
 * Function crelay_set_relay()
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
int crelay_set_relay(char* portname, uint8_t relay, relay_state_t relay_state, char* serial)
{
   if (relay_type != NO_RELAY_TYPE)
   {
      return (*relay_data[relay_type].set_relay_fun)(portname, relay, relay_state, serial);
   }
   else
   {   
      return -1;
   }
}


/**********************************************************
 * Function crelay_get_relay_card_type()
 * 
 * Description: Get the detected relay type
 * 
 * Parameters: none
 * 
 * Return: relay type
 *********************************************************/
relay_type_t crelay_get_relay_card_type()
{
   return relay_type;
}


/**********************************************************
 * Function crelay_get_relay_card_name()
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
int crelay_get_relay_card_name(relay_type_t rtype, char* card_name)
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
