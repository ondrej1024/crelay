/******************************************************************************
 * 
 * Relay card control utility: Relay driver generic file
 * 
 * Description:
 *   This software is used to controls different type of relays cards.
 *   This file contains the declaration of the generic functions.
 * 
 * Author:
 *   Ondrej Wisniewski (ondrej.wisniewski *at* gmail.com)
 *
 * Last modified:
 *   19/02/2014
 *
 *****************************************************************************/ 
 
#ifndef relay_drv_h
#define relay_drv_h

typedef enum
{
   NO_RELAY_TYPE=0,
   
   CONRAD_4CHANNEL_USB_RELAY_TYPE, /* Conrad usb 4-channel relay card */
   GENERIC_GPIO_RELAY_TYPE,        /* Relays connected directly via GPIO pins */
   
   /* Add other relay types here */
   
   LAST_RELAY_TYPE
} relay_type_t;

typedef enum {
   OFF=0,
   ON,
   PULSE,
   INVALID
}
relay_state_t;

#define CONRAD_4CHANNEL_USB_NAME       "Conrad USB 4-channel relay card"
#define CONRAD_4CHANNEL_USB_NUM_RELAYS 4

#define GENERIC_GPIO_NAME              "Generic GPIO relays"
#define GENERIC_GPIO_NUM_RELAYS        8

#define FIRST_RELAY    1
#define MAX_NUM_RELAYS 8


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
int detect_com_port(char* portname);

/**********************************************************
 * Function get_relay()
 * 
 * Description: Get the current relay state
 * 
 * Parameters: portname (in)     - communication port
 *             relay (in)        - relay number
 *             relay_state (out) - current relay state
 * 
 * Return:   o - success
 *          -1 - fail
 *********************************************************/
int get_relay(char* portname, uint8 relay, relay_state_t* relay_state);

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
int set_relay(char* portname, int relay, relay_state_t relay_state);

/**********************************************************
 * Function get_relay_card_type()
 * 
 * Description: Get the detected relay type
 * 
 * Parameters: none
 * 
 * Return: relay type
 *********************************************************/
relay_type_t get_relay_card_type();

/**********************************************************
 * Function get_relay_card_name()
 * 
 * Description: Get the detected relay card name
 * 
 * Parameters: none
 * 
 * Return: relay card name
 *********************************************************/
int get_relay_card_name(relay_type_t rtype, char* card_name);

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
int get_last_relay_num();

#endif

