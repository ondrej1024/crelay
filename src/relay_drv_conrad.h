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
 *   19/02/2014
 *
 *****************************************************************************/ 

#ifndef relay_drv_conrad_h
#define relay_drv_conrad_h

/**********************************************************
 * Function detect_com_port_conrad_4chan()
 * 
 * Description:
 * 
 * Parameters:
 * 
 *********************************************************/
int detect_com_port_conrad_4chan(char* portname);

/**********************************************************
 * Function get_relay_conrad_4chan()
 * 
 * Description:
 * 
 * Parameters:
 * 
 *********************************************************/
int get_relay_conrad_4chan(char* portname, uint8 relay, relay_state_t* relay_state);

/**********************************************************
 * Function set_relay_conrad_4chan()
 * 
 * Description:
 * 
 * Parameters:
 * 
 *********************************************************/
int set_relay_conrad_4chan(char* portname, int relay, relay_state_t relay_state);

#endif
