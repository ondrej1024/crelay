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
 *   25/02/2014
 *
 *****************************************************************************/ 

#ifndef relay_drv_gpio_h
#define relay_drv_gpio_h

/**********************************************************
 * Function detect_com_port_generic_gpio()
 * 
 * Description: Detect if GPIO sysfs support is available
 * 
 * Parameters: portname (out) - pointer to a string where
 *                              the detected com port will
 *                              be stored
 * 
 * Return:  0 - success
 *         -1 - fail, no relay card found
 *********************************************************/
int detect_com_port_generic_gpio(char* portname);

/**********************************************************
 * Function get_relay_generic_gpio()
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
int get_relay_generic_gpio(char* portname, uint8 relay, relay_state_t* relay_state);

/**********************************************************
 * Function set_relay_generic_gpio()
 * 
 * Description: Set the new relay state
 * 
 * Parameters: portname (in)     - communication port
 *             relay (in)        - relay number
 *             relay_state (in)  - new relay state
 * 
 * Return:   0 - success
 *          -1 - fail
 *********************************************************/
int set_relay_generic_gpio(char* portname, int relay, relay_state_t relay_state);

#endif
