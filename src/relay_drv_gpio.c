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
 *   25/02/2014
 *
 *****************************************************************************/ 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#include "data_types.h"
#include "relay_drv.h"

/* GPIO sysfs file definitions */
#define GPIO_BASE_DIR  "/sys/class/gpio/"
#define EXPORT_FILE    GPIO_BASE_DIR"export"
#define UNEXPORT_FILE  GPIO_BASE_DIR"unexport"
#define GPIO_BASE_FILE GPIO_BASE_DIR"gpio"

/* Relay number to GPIO pin number associaten */
/*** Change this if you want to use other pins  ***/
#define RELAY1_GPIO_PIN 17 // GPIO 0
#define RELAY2_GPIO_PIN 18 // GPIO 1
#define RELAY3_GPIO_PIN 27 // GPIO 2 (RPi rev.2)
#define RELAY4_GPIO_PIN 22 // GPIO 3
#define RELAY5_GPIO_PIN 23 // GPIO 4
#define RELAY6_GPIO_PIN 24 // GPIO 5
#define RELAY7_GPIO_PIN 25 // GPIO 6
#define RELAY8_GPIO_PIN  4 // GPIO 7


static uint8 pins[] =
{
  0, // dummy
  RELAY1_GPIO_PIN,
  RELAY2_GPIO_PIN,
  RELAY3_GPIO_PIN,
  RELAY4_GPIO_PIN,  
  RELAY5_GPIO_PIN,  
  RELAY6_GPIO_PIN,  
  RELAY7_GPIO_PIN,  
  RELAY8_GPIO_PIN  
};


/**********************************************************
 * Internal function do_export()
 * 
 * Description: Export GPIO pin to sysfs and define the
 *              direction if not already set to out
 * 
 * Parameters: pin - GPIO pin number
 * 
 * Return:  0 - success
 *         -1 - fail
 *********************************************************/
static int do_export(uint8 pin)
{
   int fd;
   char b[64];
   char d[4];
   
   /* Export pin to user space */
   fd = open(EXPORT_FILE, O_WRONLY);
   if (fd < 0) 
   {
      perror(EXPORT_FILE);
      return -1;
   }  
   snprintf(b, sizeof(b), "%d", pin);
   if (pwrite(fd, b, strlen(b), 0) < 0) {
      fprintf(stderr, "Unable to export pin=%d (already in use?): %s\n",
                       pin, strerror(errno));
      return -1;
   }
   close(fd);

   /* We set the direction only if not already set to "out".
    * Setting the direction seems to reset the value of the 
    * pin to 0 but we want to preseve the GPIO pin state.
    */
   
   /* Read the direction */    
   snprintf(b, sizeof(b), "%s%d/direction", GPIO_BASE_FILE, pin);
   fd = open(b, O_RDWR);
   if (fd < 0) 
   {
      fprintf(stderr, "Open %s: %s\n", b, strerror(errno));
      return -1;
   }
   if (pread(fd, d, 2, 0) != 2)
   {
      fprintf(stderr, "Unable to pread from gpio direction for pin %d: %s\n",
                       pin, strerror(errno));
      close(fd);
      return -1;
   }
   
   /* Check of direction is "in" */
   if(!strncmp(d, "in", 2))
   {
      /* Set gpio direction to "out" */
      if (pwrite(fd, "out", 4, 0) < 0) 
      {
         fprintf(stderr, "Unable to pwrite to gpio direction for pin %d: %s\n",
                 pin, strerror(errno));
         close(fd);
         return -1;
      }
   }
   close(fd);
   
   return 0;
}


/**********************************************************
 * Internal function do_unexport()
 * 
 * Description: Unexport GPIO pin from sysfs
 * 
 * Parameters: pin - GPIO pin number
 * 
 * Return:  0 - success
 *         -1 - fail
 *********************************************************/
static int do_unexport(uint8 pin)
{
   int fd;
   char b[64];
   
   /* Free GPIO pin */
   fd = open(UNEXPORT_FILE, O_WRONLY);
   if (fd < 0) 
   {
     perror(UNEXPORT_FILE);
     return -1;
   } 
   snprintf(b, sizeof(b), "%d", pin);
   if (pwrite(fd, b, strlen(b), 0) < 0) 
   {
      fprintf(stderr, "Unable to unexport pin=%d: %s\n", 
                       pin, strerror(errno));
      return -1;
   }  
   close(fd);

   return 0;
}


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
int detect_com_port_generic_gpio(char* portname)
{
   int fd;

   /* Check if GPIO sysfs is available */  
   fd = open(EXPORT_FILE, O_WRONLY);
   if (fd < 0) 
   {
      return -1;
   }
   
   strcpy(portname, GPIO_BASE_DIR);
   close(fd);
   
   return 0;
}


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
int get_relay_generic_gpio(char* portname, uint8 relay, relay_state_t* relay_state)
{
   int fd;
   char b[64];
   char d[1];
   uint8 pin;

   if (relay<FIRST_RELAY || relay>(FIRST_RELAY+GENERIC_GPIO_NUM_RELAYS-1))
   {
      fprintf(stderr, "ERROR: Relay number out of range\n");
      return -1;
   }
 
   /* Get pin number */
   pin=pins[relay];
   
   /* Export pin to user space */
   if (do_export(pin) != 0) 
      return -1;

   /* Get current gpio value */
   snprintf(b, sizeof(b), "%s%d/value", GPIO_BASE_FILE, pin);
   fd = open(b, O_RDWR);
   if (fd < 0) 
   {
      fprintf(stderr, "ERROR: Open %s: %s\n", b, strerror(errno));
      return -1;
   }
   if (pread(fd, d, 1, 0) != 1) 
   {
      fprintf(stderr, "ERROR: Unable to pread gpio value: %s\n",
                       strerror(errno));
      return -1;
   }
   close(fd);
   
   /* Free GPIO pin */
   if (do_unexport(pin) != 0) 
      return -1;

   /* Return current relay state */
   *relay_state = (d[0] == '0') ? OFF : ON;
   
   return 0;
}


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
int set_relay_generic_gpio(char* portname, uint8 relay, relay_state_t relay_state)
{
   int fd;
   char b[64];
   char d[1];
   uint8 pin;
   
   if (relay<FIRST_RELAY || relay>(FIRST_RELAY+GENERIC_GPIO_NUM_RELAYS-1))
   {
      fprintf(stderr, "ERROR: Relay number out of range\n");
      return -1;
   }
 
   /* Get pin number */
   pin=pins[relay];
   
   /* Export pin to user space */
   if (do_export(pin) != 0)
      return -1;

   /* Set new gpio value */
   snprintf(b, sizeof(b), "%s%d/value", GPIO_BASE_FILE, pin);
   fd = open(b, O_RDWR);
   if (fd < 0) 
   {
      fprintf(stderr, "ERROR: Open %s: %s\n", b, strerror(errno));
      return -1;
   }
   d[0] = (relay_state == OFF ? '0' : '1');
   if (pwrite(fd, d, 1, 0) != 1) 
   {
      fprintf(stderr, "ERROR: Unable to pwrite %c to gpio value: %s\n",
                       d[0], strerror(errno));
      return -1;
   }
   close(fd);
   
   /* Free GPIO pin */
   if (do_unexport(pin) != 0) 
      return -1;

   return 0;
}

 
