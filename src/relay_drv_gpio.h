/******************************************************************************
 * 
 * Relay card control utility: Driver for generic GPIO relay card
 * 
 * Description:
 *   This software is used to control the relays connected via GPIO pins.
 *   This file contains the implementation of the specific functions.
 * 
 * Author:
 *   Ondrej Wisniewski (ondrej.wisniewski *at* gmail.com)
 *
 * Last modified:
 *   19/02/2014
 *
 *****************************************************************************/ 

#ifndef relay_drv_gpio_h
#define relay_drv_gpio_h

/**********************************************************
 * Function detect_com_port_generic_gpio()
 * 
 * Description:
 * 
 * Parameters:
 * 
 *********************************************************/
int detect_com_port_generic_gpio(char* portname);

/**********************************************************
 * Function get_relay_generic_gpio()
 * 
 * Description:
 * 
 * Parameters:
 * 
 *********************************************************/
int get_relay_generic_gpio(char* portname, uint8 relay, relay_state_t* relay_state);

/**********************************************************
 * Function set_relay_generic_gpio()
 * 
 * Description:
 * 
 * Parameters:
 * 
 *********************************************************/
int set_relay_generic_gpio(char* portname, int relay, relay_state_t relay_state);

#endif
