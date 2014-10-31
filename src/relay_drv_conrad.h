/******************************************************************************
 * 
 * Relay card control utility: Driver for Conrad USB 4-relay card
 * 
 * Description:
 *   This software is used to control the Conrad USB 4-relay card.
 *   This file contains the declaration of the specific functions.
 * 
 * Author:
 *   Ondrej Wisniewski (ondrej.wisniewski *at* gmail.com)
 *
 * Last modified:
 *   25/02/2014
 *
 *****************************************************************************/ 

#ifndef relay_drv_conrad_h
#define relay_drv_conrad_h

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
int detect_com_port_conrad_4chan(char* portname);

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
int get_relay_conrad_4chan(char* portname, uint8 relay, relay_state_t* relay_state);

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
int set_relay_conrad_4chan(char* portname, uint8 relay, relay_state_t relay_state);

#endif
