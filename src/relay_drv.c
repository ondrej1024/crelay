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
 *   19/02/2014
 *
 *****************************************************************************/ 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "data_types.h"
#include "relay_drv.h"

/* Card driver specific include files */
#include "relay_drv_conrad.h"
#include "relay_drv_gpio.h"


static relay_type_t relay_type=NO_RELAY_TYPE;


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
   if (detect_com_port_conrad_4chan(portname) == 0)
   {
     relay_type=CONRAD_4CHANNEL_USB_RELAY_TYPE;
     return 0;
   }
   
   if (detect_com_port_generic_gpio(portname) == 0)
   {
     relay_type=GENERIC_GPIO_RELAY_TYPE;
     return 0;
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
   int rc;
   
   switch (relay_type)
   {
      case CONRAD_4CHANNEL_USB_RELAY_TYPE:
         rc = get_relay_conrad_4chan(portname, relay, relay_state);
      break;
      
      case GENERIC_GPIO_RELAY_TYPE:
         rc = get_relay_generic_gpio(portname, relay, relay_state);       
      break;

      default:
         rc = -1;
   }
   return rc;
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
int set_relay(char* portname, int relay, relay_state_t relay_state)
{
   int rc;
   
   switch (relay_type)
   {
      case CONRAD_4CHANNEL_USB_RELAY_TYPE:
         rc = set_relay_conrad_4chan(portname, relay, relay_state);
      break;
      
      case GENERIC_GPIO_RELAY_TYPE:
         rc = set_relay_generic_gpio(portname, relay, relay_state);      
      break;

      default:
         rc = -1;
   }
   return rc;
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
 * Description: Get the detected relay card name
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
   int rc=0;
   
   switch (rtype)
   {
      case CONRAD_4CHANNEL_USB_RELAY_TYPE:
         strcpy(card_name, CONRAD_4CHANNEL_USB_NAME);
      break;
      
      case GENERIC_GPIO_RELAY_TYPE:
         strcpy(card_name, GENERIC_GPIO_NAME);
      break;

      default:
         rc = -1;
   }
   return rc;    
}
