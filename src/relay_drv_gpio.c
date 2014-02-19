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
 * Build instructions:
 *   gcc -c relay_drv_gpio.c
 * 
 * Last modified:
 *   19/02/2014
 *
 *****************************************************************************/ 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "data_types.h"
#include "relay_drv.h"


/**********************************************************
 * Function detect_com_port_generic_gpio()
 * 
 * Description:
 * 
 * Parameters:
 * 
 *********************************************************/
int detect_com_port_generic_gpio(char* portname)
{
   return -1;
}


/**********************************************************
 * Function get_relay_generic_gpio()
 * 
 * Description:
 * 
 * Parameters:
 * 
 *********************************************************/
int get_relay_generic_gpio(char* portname, uint8 relay, relay_state_t* relay_state)
{
   return 0;
}


/**********************************************************
 * Function set_relay_generic_gpio()
 * 
 * Description:
 * 
 * Parameters:
 * 
 *********************************************************/
int set_relay_generic_gpio(char* portname, int relay, relay_state_t relay_state)
{
   return 0;
}

 
