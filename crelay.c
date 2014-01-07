/******************************************************************************
 * 
 * Conrad USB 4-relay card utility
 * 
 * Description:
 *   Controls the relays on the USB 4-relay card from Conrad.
 *   http://www.conrad.de/ce/de/product/393905
 * 
 * Note:
 *   We need the cp210x driver for the Silabs CP2104 chip with GPIO support.
 *   The official in-kernel cp210x driver does currently not yet support
 *   GPIO operations. Therefore the Silabs driver needs to be used:
 *   http://www.silabs.com/products/mcu/pages/usbtouartbridgevcpdrivers.aspx
 * 
 * Author:
 *   Ondrej Wisniewski
 *
 * Build instructions:
 *   gcc -o crelay crelay.c
 * 
 * Last modified:
 *   23/12/2013
 *
 *****************************************************************************/ 

/*
 * Communication protocol description
 * ==================================
 * 
 * The Silabs CP2104 USB to UART Bridge Controller is used in GPIO mode.
 * 
 * Get relay status:
 * -----------------
 *  15 14 13 12   11 10 9  8   7  6  5  4    3  2  1  0   bit no
 *  X  X  X  X    X  X  X  X   X  X  X  X   R4 R3 R2 R1   relay state
 * 
 * 
 * Set relay status:
 * -----------------
 *  31 30 29 28   27 26 25 24  23 22 21 20  19 18 17 16   bit no
 *  X  X  X  X    X  X  X  X   X  X  X  X   R4 R3 R2 R1   relay state to set
 * 
 *  15 14 13 12   11 10 9  8   7  6  5  4    3  2  1  0   bit no
 *  X  X  X  X    X  X  X  X   X  X  X  X   R4 R3 R2 R1   relay bit mask
 * 
 * Relay names:
 *  R1: relay 1
 *  R2: relay 2
 *  R3: relay 3
 *  R4: relay 4
 * 
 * Meaning of bit values:
 *  0: NO contact open, NC contact closed, led is on
 *  1: NO contact closed, NC contact open, led is off
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#define IOCTL_GPIOGET 0x8000
#define IOCTL_GPIOSET 0x8001

#define FIRST_RELAY 1
#define LAST_RELAY  4
#define MAX_PORTS 10

/* USB serial device created by cp210x driver */
#define SERIAL_DEV_BASE "/dev/ttyUSB"

typedef unsigned char  uint8;
typedef unsigned short uint16;
typedef unsigned long  uint32;

typedef enum {
   OFF,
   ON
}
relay_state_t;



int detect_port(char* portname)
{
   uint8 found=0;
   uint32 gpio=0;
   int fd;
   int i;
   char pname[16];
   
   for (i=0; i<MAX_PORTS; i++)
   { 
      /* Try to open USB serial device */
      sprintf(pname, "%s%d", SERIAL_DEV_BASE, i);
      //printf("DBG: Trying serial device %s\n", pname);
      fd = open(pname, O_RDONLY | O_NOCTTY | O_NDELAY);
      if (fd != -1) 
      {
         /* Now try to read the GPIO pins, this will work only
          * for a CP210x device with GPIO support in the driver 
          */
         if (ioctl(fd, IOCTL_GPIOGET, &gpio) != -1)
         {
            /* It works, we found a compatible device */
            found=1;
            break;
         }
      }
   }
   
   if (found) 
   {
      strcpy(portname, pname);
      close(fd);
      return 0;
   }
   
   return -1;
}


int get_relay(char* portname, uint8 relay, relay_state_t* relay_state)
{
   uint32 gpio=0;
   int fd;
   int rc;
   
   if (relay<FIRST_RELAY || relay>LAST_RELAY)
   {  
      fprintf(stderr, "ERROR: Relay number out of range\n");
      return -1;      
   }

   /* Open serial device */
   fd = open(portname, O_RDWR | O_NOCTTY | O_NDELAY);
   if (fd == -1) 
   {
      fprintf(stderr, "ERROR: Failed to open %s: %s\n", portname, strerror(errno));
      return -2;      
   }
  
   /* Get relay state from the card via IOCTL */ 
   rc = ioctl(fd, IOCTL_GPIOGET, &gpio);
   if (rc == -1)
   {
      fprintf(stderr, "IOCTL_GPIOGET failed: %s\n", strerror(errno));
      return -3;
   }

   //printf("DBG: Read GPIO bits %08lX\n", gpio);
   
   relay = relay-1;
   *relay_state = (gpio & (0x0001<<relay)) ? OFF : ON;
      
   close(fd);
   return 0;
}


int set_relay(char* portname, int relay, relay_state_t relay_state)
{
   uint32 gpio=0;
   int fd;
   int rc;
   
   //printf("DBG: Relay number is %d\n", relay);
   
   if (relay<FIRST_RELAY || relay>LAST_RELAY)
   {  
      fprintf(stderr, "ERROR: Relay number out of range\n");
      return -1;      
   }
   
   /* Open serial device */
   fd = open(portname, O_RDWR | O_NOCTTY | O_NDELAY);
   if (fd == -1) 
   {
      fprintf(stderr, "ERROR: Failed to open %s: %s\n", portname, strerror(errno));
      return -2;      
   }

   /* Set the relay state bit */
   relay = relay-1;
   if (relay_state == OFF) gpio = 0x0001<<(relay+16);
   
   /* Set the relay bit mask */
   gpio = gpio | (0x0001<<relay);

   //printf("DBG: Writing GPIO bits %08lX\n", gpio);
   
   /* Set relay on the card via IOCTL */
   rc = ioctl(fd, IOCTL_GPIOSET, &gpio);
   if (rc == -1)
   {
      fprintf(stderr, "IOCTL_GPIOSET failed: %s\n", strerror(errno));
      return -3;
   }
   
   return 0;
}


void print_usage()
{
   printf("This is a utility to control the Conrad USB 4-relay card.\n");
   printf("The state of a relay can be read or it can be changed to a new state.\n\n");
   printf("Usage: crelay [<relay number>] [ON|OFF]\n\n");
   printf("       If only the relay number is provided then the current state is returned,otherwise\n");
   printf("       the relays state is set to the new value provided as second parameter.\n");
   printf("       The USB communication port is auto detected. The first compatible device found will be used.\n");
}


int main(int argc, char* argv[])
{
   relay_state_t rstate;
   char serial_port[16];
   
   if (detect_port(serial_port) == -1)
   {
      printf("No compatible device found.\n");
      return -1;
   }
   
   //printf("DBG: Found compatible device on %s\n\n", serial_port);

   switch (argc)
   {
      case 2:
         /* GET current relay state */
         get_relay(serial_port, atoi(argv[1]), &rstate);
         printf("Relay %d is %s\n", atoi(argv[1]), (rstate==ON)?"on":"off");
         break;
         
      case 3:
         /* SET new relay state */
         if (!strcmp(argv[2],"on") || !strcmp(argv[2],"ON")) 
            set_relay(serial_port, atoi(argv[1]), ON);
         else if (!strcmp(argv[2],"off") || !strcmp(argv[2],"OFF")) 
            set_relay(serial_port, atoi(argv[1]), OFF);
         else
            print_usage();
         break;
         
      default:
         print_usage();
   }
   
   return 0;
}
