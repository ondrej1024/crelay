/******************************************************************************
 * 
 * Relay card control utility: Driver for XYZ relay card
 *   http:...
 * 
 * Description:
 * 
 * Author:
 *
 * Build instructions:
 *   gcc -c relay_drv_sample.c
 * 
 * Last modified:
 *   
 *
 *****************************************************************************/ 

#ifndef relay_drv_conrad_h
#define relay_drv_conrad_h

/**********************************************************
 * Function detect_com_port_sample()
 * 
 * Description: Detect the port used for communicating 
 *              with the sample relay card
 * 
 * Parameters: portname (out) - pointer to a string where
 *                              the detected com port will
 *                              be stored
 * 
 * Return:  0 - success
 *         -1 - fail, no relay card found
 *********************************************************/
int detect_com_port_sample(char* portname);


/**********************************************************
 * Function get_relay_sample()
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
int get_relay_sample(char* portname, uint8 relay, relay_state_t* relay_state);


/**********************************************************
 * Function set_relay_sample()
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
int set_relay_sample(char* portname, uint8 relay, relay_state_t relay_state);

#endif
