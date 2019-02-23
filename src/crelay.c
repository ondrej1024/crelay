/******************************************************************************
 * 
 * Relay card control utility: Main module
 * 
 * Description:
 *   This software is used to controls different type of relays cards.
 *   There are 3 ways to control the relays:
 *    1. via command line
 *    2. via web interface using a browser
 *    3. via HTTP API using a client application
 * 
 * Author:
 *   Ondrej Wisniewski (ondrej.wisniewski *at* gmail.com)
 *
 * Build instructions:
 *   make
 *   sudo make install
 * 
 * Last modified:
 *   14/01/2019
 *
 * Copyright 2015-2019, Ondrej Wisniewski 
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
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>
#include <syslog.h>
#include <arpa/inet.h>

#include "data_types.h"
#include "config.h"
#include "http.h"
#include "mqtt.h"
#include "relay_drv.h"

#define VERSION "0.15"
#define DATE "2019"

#define CONFIG_FILE "/etc/crelay.conf"

/* Global variables */
config_t config;

char rlabels[MAX_NUM_RELAYS][32] = {"My appliance 1", "My appliance 2", "My appliance 3", "My appliance 4",
                                           "My appliance 5", "My appliance 6", "My appliance 7", "My appliance 8"};                                       

/**********************************************************
 * Function: config_cb()
 * 
 * Description:
 *           Callback function for handling the name=value
 *           pairs returned by the conf_parse() function
 * 
 * Returns:  0 on success, <0 otherwise
 *********************************************************/
static int config_cb(void* user, const char* section, const char* name, const char* value)
{
   config_t* pconfig = (config_t*)user;
   
   #define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
   if (MATCH("HTTP server", "server_iface")) 
   {
      pconfig->server_iface = strdup(value);
   } 
   else if (MATCH("HTTP server", "server_port")) 
   {
      pconfig->server_port = atoi(value);
   } 
   else if (MATCH("HTTP server", "relay1_label")) 
   {
      pconfig->relay1_label = strdup(value);
   } 
   else if (MATCH("HTTP server", "relay2_label")) 
   {
      pconfig->relay2_label = strdup(value);
   } 
   else if (MATCH("HTTP server", "relay3_label")) 
   {
      pconfig->relay3_label = strdup(value);
   } 
   else if (MATCH("HTTP server", "relay4_label")) 
   {
      pconfig->relay4_label = strdup(value);
   } 
   else if (MATCH("HTTP server", "relay5_label")) 
   {
      pconfig->relay5_label = strdup(value);
   } 
   else if (MATCH("HTTP server", "relay6_label")) 
   {
      pconfig->relay6_label = strdup(value);
   } 
   else if (MATCH("HTTP server", "relay7_label")) 
   {
      pconfig->relay7_label = strdup(value);
   } 
   else if (MATCH("HTTP server", "relay8_label")) 
   {
      pconfig->relay8_label = strdup(value);
   } 
   else if (MATCH("HTTP server", "pulse_duration")) 
   {
      pconfig->pulse_duration = atoi(value);
   }
   else if (MATCH("GPIO drv", "num_relays")) 
   {
      pconfig->gpio_num_relays = atoi(value);
   } 
   else if (MATCH("GPIO drv", "active_value")) 
   {
      pconfig->gpio_active_value = atoi(value);
   } 
   else if (MATCH("GPIO drv", "relay1_gpio_pin")) 
   {
      pconfig->relay1_gpio_pin = atoi(value);
   } 
   else if (MATCH("GPIO drv", "relay2_gpio_pin")) 
   {
      pconfig->relay2_gpio_pin = atoi(value);
   } 
   else if (MATCH("GPIO drv", "relay3_gpio_pin")) 
   {
      pconfig->relay3_gpio_pin = atoi(value);
   } 
   else if (MATCH("GPIO drv", "relay4_gpio_pin")) 
   {
      pconfig->relay4_gpio_pin = atoi(value);
   } 
   else if (MATCH("GPIO drv", "relay5_gpio_pin")) 
   {
      pconfig->relay5_gpio_pin = atoi(value);
   } 
   else if (MATCH("GPIO drv", "relay6_gpio_pin")) 
   {
      pconfig->relay6_gpio_pin = atoi(value);
   } 
   else if (MATCH("GPIO drv", "relay7_gpio_pin")) 
   {
      pconfig->relay7_gpio_pin = atoi(value);
   } 
   else if (MATCH("GPIO drv", "relay8_gpio_pin")) 
   {
      pconfig->relay8_gpio_pin = atoi(value);
   } 
   else if (MATCH("Sainsmart drv", "num_relays")) 
   {
      pconfig->sainsmart_num_relays = atoi(value);
   } 
   else 
   {
      syslog(LOG_DAEMON | LOG_WARNING, "unknown config parameter %s/%s\n", section, name);
      return -1;  /* unknown section/name, error */
   }
   return 0;
}


/**********************************************************
 * Function: exit_handler()
 * 
 * Description:
 *           Handles the cleanup at reception of the 
 *           TERM signal.
 * 
 * Returns:  -
 *********************************************************/
static void exit_handler(int signum)
{
   syslog(LOG_DAEMON | LOG_NOTICE, "Exit crelay daemon\n");
   exit(EXIT_SUCCESS);
}

                                           
/**********************************************************
 * Function print_usage()
 * 
 * Description:
 * 
 * Parameters:
 * 
 *********************************************************/
void print_usage()
{
   relay_type_t rtype;
   char cname[60];
   
   printf("crelay, version %s\n\n", VERSION);
   printf("This utility provides a unified way of controlling different types of relay cards.\n");
   printf("Supported relay cards:\n");
   for(rtype=NO_RELAY_TYPE+1; rtype<LAST_RELAY_TYPE; rtype++)
   {
      crelay_get_relay_card_name(rtype, cname);
      printf("  - %s\n", cname);
   }
   printf("\n");
   printf("The program can be run in interactive (command line) mode or in daemon mode with\n");
   printf("built-in web server.\n\n");
   printf("Interactive mode:\n");
   printf("    crelay -i | [-s <serial number>] <relay number> [ON|OFF]\n\n");
   printf("       -i print relay information\n\n");
   printf("       The state of any relay can be read or it can be changed to a new state.\n");
   printf("       If only the relay number is provided then the current state is returned,\n");
   printf("       otherwise the relays state is set to the new value provided as second parameter.\n");
   printf("       The USB communication port is auto detected. The first compatible device\n");
   printf("       found will be used, unless -s switch and a serial number is passed.\n\n");
   printf("Daemon mode:\n");
   printf("    crelay -d|-D [<relay1_label> [<relay2_label> [<relay3_label> [<relay4_label>]]]] \n\n");
   printf("       -d use daemon mode, run in foreground\n");
   printf("       -D use daemon mode, run in background\n\n");
   printf("       In daemon mode the built-in web server will be started and the relays\n");
   printf("       can be completely controlled via a Web browser GUI or HTTP API.\n");
   printf("       The config file %s will be used, if present.\n", CONFIG_FILE);
   printf("       Optionally a personal label for each relay can be supplied as command\n");
   printf("       line parameter which will be displayed next to the relay name on the\n");
   printf("       web page.\n\n");
   printf("       To access the web interface point your Web browser to the following address:\n");
   printf("       http://<my-ip-address>:<port>\n\n");
   printf("       To use the HTTP API send a POST or GET request from the client to this URL:\n");
   printf("       http://<my-ip-address>:<port>/gpio\n\n");
}


/**********************************************************
 * Function main()
 * 
 * Description:
 * 
 * Parameters:
 * 
 *********************************************************/
int main(int argc, char *argv[])
{
      relay_state_t rstate;
      char com_port[MAX_COM_PORT_NAME_LEN];
      char cname[MAX_RELAY_CARD_NAME_LEN];
      char* serial=NULL;
      uint8_t num_relays=FIRST_RELAY;
      relay_info_t *relay_info;
      int argn = 1;
      int err;
      int i = 1;

   if (argc==1)
   {
      print_usage();
      exit(EXIT_SUCCESS);
   }
   
   if (!strcmp(argv[1],"-d") || !strcmp(argv[1],"-D"))
   {
      /*****  Daemon mode *****/
      
      struct in_addr iface;
      int port=0;
      int i;
      
      iface.s_addr = INADDR_ANY;

      
      openlog("crelay", LOG_PID|LOG_CONS, LOG_USER);
      syslog(LOG_DAEMON | LOG_NOTICE, "Starting crelay daemon (version %s)\n", VERSION);
   
      /* Setup signal handlers */
      signal(SIGINT, exit_handler);   /* Ctrl-C */
      signal(SIGTERM, exit_handler);  /* "regular" kill */
   
      /* Load configuration from .conf file */
      memset((void*)&config, 0, sizeof(config_t));
      if (conf_parse(CONFIG_FILE, config_cb, &config) >= 0) 
      {
         syslog(LOG_DAEMON | LOG_NOTICE, "Config parameters read from %s:\n", CONFIG_FILE);
         syslog(LOG_DAEMON | LOG_NOTICE, "***************************\n");
         if (config.server_iface != NULL) syslog(LOG_DAEMON | LOG_NOTICE, "server_iface: %s\n", config.server_iface);
         if (config.server_port != 0)     syslog(LOG_DAEMON | LOG_NOTICE, "server_port: %u\n", config.server_port);
         if (config.relay1_label != NULL) syslog(LOG_DAEMON | LOG_NOTICE, "relay1_label: %s\n", config.relay1_label);
         if (config.relay2_label != NULL) syslog(LOG_DAEMON | LOG_NOTICE, "relay2_label: %s\n", config.relay2_label);
         if (config.relay3_label != NULL) syslog(LOG_DAEMON | LOG_NOTICE, "relay3_label: %s\n", config.relay3_label);
         if (config.relay4_label != NULL) syslog(LOG_DAEMON | LOG_NOTICE, "relay4_label: %s\n", config.relay4_label);
         if (config.relay5_label != NULL) syslog(LOG_DAEMON | LOG_NOTICE, "relay5_label: %s\n", config.relay5_label);
         if (config.relay6_label != NULL) syslog(LOG_DAEMON | LOG_NOTICE, "relay6_label: %s\n", config.relay6_label);
         if (config.relay7_label != NULL) syslog(LOG_DAEMON | LOG_NOTICE, "relay7_label: %s\n", config.relay7_label);
         if (config.relay8_label != NULL) syslog(LOG_DAEMON | LOG_NOTICE, "relay8_label: %s\n", config.relay8_label);
         if (config.pulse_duration != 0)  syslog(LOG_DAEMON | LOG_NOTICE, "pulse_duration: %u\n", config.pulse_duration);
         if (config.gpio_num_relays != 0) syslog(LOG_DAEMON | LOG_NOTICE, "gpio_num_relays: %u\n", config.gpio_num_relays);
         if (config.gpio_active_value >= 0) syslog(LOG_DAEMON | LOG_NOTICE, "gpio_active_value: %u\n", config.gpio_active_value);
         if (config.relay1_gpio_pin != 0) syslog(LOG_DAEMON | LOG_NOTICE, "relay1_gpio_pin: %u\n", config.relay1_gpio_pin);
         if (config.relay2_gpio_pin != 0) syslog(LOG_DAEMON | LOG_NOTICE, "relay2_gpio_pin: %u\n", config.relay2_gpio_pin);
         if (config.relay3_gpio_pin != 0) syslog(LOG_DAEMON | LOG_NOTICE, "relay3_gpio_pin: %u\n", config.relay3_gpio_pin);
         if (config.relay4_gpio_pin != 0) syslog(LOG_DAEMON | LOG_NOTICE, "relay4_gpio_pin: %u\n", config.relay4_gpio_pin);
         if (config.relay5_gpio_pin != 0) syslog(LOG_DAEMON | LOG_NOTICE, "relay5_gpio_pin: %u\n", config.relay5_gpio_pin);
         if (config.relay6_gpio_pin != 0) syslog(LOG_DAEMON | LOG_NOTICE, "relay6_gpio_pin: %u\n", config.relay6_gpio_pin);
         if (config.relay7_gpio_pin != 0) syslog(LOG_DAEMON | LOG_NOTICE, "relay7_gpio_pin: %u\n", config.relay7_gpio_pin);
         if (config.relay8_gpio_pin != 0) syslog(LOG_DAEMON | LOG_NOTICE, "relay8_gpio_pin: %u\n", config.relay8_gpio_pin);
         if (config.sainsmart_num_relays != 0) syslog(LOG_DAEMON | LOG_NOTICE, "sainsmart_num_relays: %u\n", config.sainsmart_num_relays);
         syslog(LOG_DAEMON | LOG_NOTICE, "***************************\n");
         
         /* Get relay labels from config file */
         if (config.relay1_label != NULL) strcpy(rlabels[0], config.relay1_label);
         if (config.relay2_label != NULL) strcpy(rlabels[1], config.relay2_label);
         if (config.relay3_label != NULL) strcpy(rlabels[2], config.relay3_label);
         if (config.relay4_label != NULL) strcpy(rlabels[3], config.relay4_label);
         if (config.relay5_label != NULL) strcpy(rlabels[4], config.relay5_label);
         if (config.relay6_label != NULL) strcpy(rlabels[5], config.relay6_label);
         if (config.relay7_label != NULL) strcpy(rlabels[6], config.relay7_label);
         if (config.relay8_label != NULL) strcpy(rlabels[7], config.relay8_label);
         
         /* Get listen interface from config file */
         if (config.server_iface != NULL)
         {
            if (inet_aton(config.server_iface, &iface) == 0)
            {
               syslog(LOG_DAEMON | LOG_NOTICE, "Invalid iface address in config file, using default value");
            }
         }
         
         /* Get listen port from config file */
         if (config.server_port > 0)
         {
            port = config.server_port;
         }

      }
      else
      {
         syslog(LOG_DAEMON | LOG_NOTICE, "Can't load %s, using default parameters\n", CONFIG_FILE);
      }

      /* Ensure pulse duration is valid **/
      if (config.pulse_duration == 0)
      {
         config.pulse_duration = 1;
      }
      
      /* Parse command line for relay labels (overrides config file)*/
      for (i=0; i<argc-2 && i<MAX_NUM_RELAYS; i++)
      {
         strcpy(rlabels[i], argv[i+2]);
      }         
      
      /* Init GPIO pins in case they have been configured */
      crelay_detect_relay_card(com_port, &num_relays, NULL, NULL);
      
      /* Init communication protocols */
      if (init_http(iface, port) != 0)
         exit(EXIT_FAILURE);
      if (init_mqtt() != 0)
         exit(EXIT_FAILURE);

      if (!strcmp(argv[1],"-D"))
      {
         /* Daemonise program (send to background) */
         if (daemon(0, 0) == -1) 
         {
            syslog(LOG_DAEMON | LOG_ERR, "Failed to daemonize: %s", strerror(errno));
            exit(EXIT_FAILURE);
         }
         syslog(LOG_DAEMON | LOG_NOTICE, "Program is now running as system daemon");
      }

      /* Endless loop (wait for requests) */
      while (1) sleep(1);
   }
   else
   {
      /*****  Command line mode *****/
      
      if (!strcmp(argv[argn],"-i"))
      {
         /* Detect all cards connected to the system */
         if (crelay_detect_all_relay_cards(&relay_info) == -1)
         {
            printf("No compatible device detected.\n");
            return -1;
         }
         printf("\nDetected relay cards:\n");
         while (relay_info->next != NULL)
         {
            crelay_get_relay_card_name(relay_info->relay_type, cname);
            printf("  #%d\t%s (serial %s)\n", i++ ,cname, relay_info->serial);
            relay_info = relay_info->next;
            // TODO: free relay_info list memory
         }
         
         exit(EXIT_SUCCESS);
      }
   
      if (!strcmp(argv[argn], "-s"))
      {
         if (argc > (argn+1))
         {
            serial = malloc(sizeof(char)*strlen(argv[argn+1]));
            strcpy(serial, argv[argn+1]);
            argn += 2;
         }
         else
         {
            print_usage();
            exit(EXIT_FAILURE);            
         }
      }

      if (crelay_detect_relay_card(com_port, &num_relays, serial, NULL) == -1)
      {
         printf("No compatible device detected.\n");
         
         if(geteuid() != 0)
         {
            printf("\nWarning: this program is currently not running with root privileges !\n");
            printf("Therefore it might not be able to access your relay card communication port.\n");
            printf("Consider invoking the program from the root account or use \"sudo ...\"\n");
         }
         
         exit(EXIT_FAILURE);
      }

      switch (argc)
      {
         case 2:
         case 4:
            /* GET current relay state */
            if (crelay_get_relay(com_port, atoi(argv[argn]), &rstate, serial) == 0)
               printf("Relay %d is %s\n", atoi(argv[argn]), (rstate==ON)?"on":"off");
            else
               exit(EXIT_FAILURE);
            break;
            
         case 3:
         case 5:
            /* SET new relay state */
            if (!strcmp(argv[argn+1],"on") || !strcmp(argv[argn+1],"ON"))
               err = crelay_set_relay(com_port, atoi(argv[argn]), ON, serial);
            else if (!strcmp(argv[argn+1],"off") || !strcmp(argv[argn+1],"OFF"))
               err = crelay_set_relay(com_port, atoi(argv[argn]), OFF, serial);
            else 
            {
               print_usage();
               exit(EXIT_FAILURE);
            }
            if (err)
               exit(EXIT_FAILURE);
            break;
            
         default:
            print_usage();
      }
   }
   
   exit(EXIT_SUCCESS);
}
