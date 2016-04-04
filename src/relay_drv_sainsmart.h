/******************************************************************************
 * 
 * Relay card control utility: Driver for Sainsmart USB 4-relay card
 * 
 * Description:
 *   This software is used to control the Sainsmart USB 4-relay card.
 *   This file contains the declaration of the specific functions.
 * 
 * Author:
 *   Ondrej Wisniewski (ondrej.wisniewski *at* gmail.com)
 *
 * Last modified:
 *   19/08/2015
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

#ifndef relay_drv_sainsmart_h
#define relay_drv_sainsmart_h

/**********************************************************
 * Function detect_relay_card_sainsmart_4_8chan()
 * 
 * Description: Detect the Sainsmart USB relay card
 * 
 * Parameters: portname (out) - pointer to a string where
 *                              the detected com port will
 *                              be stored
 *             num_relays(out)- pointer to number of relays
 * 
 * Return:  0 - success
 *         -1 - fail, no relay card found
 *********************************************************/
int detect_relay_card_sainsmart_4_8chan(char* portname, uint8* num_relays, char* serial, relay_info_t** relay_info);

/**********************************************************
 * Function get_relay_sainsmart_4_8chan()
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
int get_relay_sainsmart_4_8chan(char* portname, uint8 relay, relay_state_t* relay_state, char* serial);

/**********************************************************
 * Function set_relay_sainsmart_4_8chan()
 * 
 * Description: Set new relay state
 * 
 * Parameters: portname (in)     - communication port
 *             relay (in)        - relay number
 *             relay_state (in)  - current relay state
 * 
 * Return:   0 - success
 *          -1 - fail
 *********************************************************/
int set_relay_sainsmart_4_8chan(char* portname, uint8 relay, relay_state_t relay_state, char* serial);

#endif
