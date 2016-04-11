/******************************************************************************
 * 
 * Relay card control utility: Main module
 * 
 * Description:
 *   This software is used to controls different type of relays cards.
 *   There are 3 ways to control the relais:
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
 *   11/04/2016
 *
 * Copyright 2016, Ondrej Wisniewski 
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
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>
#include <syslog.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "data_types.h"
#include "config.h"
#include "relay_drv.h"

#define VERSION "0.10"
#define DATE "2016"

/* HTTP server defines */
#define SERVER "crelay/"VERSION
#define PROTOCOL "HTTP/1.0"
#define RFC1123FMT "%a, %d %b %Y %H:%M:%S GMT"
#define API_URL "gpio"
#define DEFAULT_SERVER_PORT 8000

/* HTML tag definitions */
#define RELAY_TAG "pin"
#define STATE_TAG "status"

#define CONFIG_FILE "/etc/crelay.conf"

/* Global variables */
config_t config;

static char rlabels[MAX_NUM_RELAYS][32] = {"My appliance 1", "My appliance 2", "My appliance 3", "My appliance 4",
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
   else if (MATCH("GPIO drv", "num_relays")) 
   {
      pconfig->gpio_num_relays = atoi(value);
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
 * Function send_headers()
 * 
 * Description:
 * 
 * Parameters:
 * 
 *********************************************************/
void send_headers(FILE *f, int status, char *title, char *extra, char *mime, 
                  int length, time_t date)
{
   time_t now;
   char timebuf[128];
   
   fprintf(f, "%s %d %s\r\n", PROTOCOL, status, title);
   fprintf(f, "Server: %s\r\n", SERVER);
   now = time(NULL);
   strftime(timebuf, sizeof(timebuf), RFC1123FMT, gmtime(&now));
   fprintf(f, "Date: %s\r\n", timebuf);
   if (extra) fprintf(f, "%s\r\n", extra);
   if (mime) fprintf(f, "Content-Type: %s\r\n", mime);
   if (length >= 0) fprintf(f, "Content-Length: %d\r\n", length);
   if (date != -1)
   {
      strftime(timebuf, sizeof(timebuf), RFC1123FMT, gmtime(&date));
      fprintf(f, "Last-Modified: %s\r\n", timebuf);
   }
   fprintf(f, "Connection: close\r\n");
   fprintf(f, "\r\n");
}


/**********************************************************
 * Function web_page_header()
 * 
 * Description:
 * 
 * Parameters:
 * 
 *********************************************************/
void web_page_header(FILE *f)
{
   /* Send http header */
   send_headers(f, 200, "OK", NULL, "text/html", -1, -1);
   
   /* Display web page heading */
   fprintf(f, "<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01//EN\" \"http://www.w3.org/TR/html4/strict.dtd\">\r\n");
   fprintf(f, "<html><head><title>Relay Card Control interface</title></head><body>\r\n");
   fprintf(f, "<table style=\"text-align: left; width: 600px; background-color: rgb(0, 0, 153); font-family: Helvetica,Arial,sans-serif; font-weight: bold; color: white;\" border=\"0\" cellpadding=\"2\" cellspacing=\"2\">\r\n");
   fprintf(f, "<tbody><tr><td>\r\n");
   fprintf(f, "<span style=\"vertical-align: top; font-size: 48px;\">Relay Card Control</span><br>\r\n");
   fprintf(f, "<span style=\"font-size: 16px; color: rgb(204, 255, 255);\">Remote relay card controlling <span style=\"font-style: italic; color: white;\">made easy</span></span>\r\n");
   fprintf(f, "</td></tr></tbody></table>\r\n");  
}


/**********************************************************
 * Function web_page_footer()
 * 
 * Description:
 * 
 * Parameters:
 * 
 *********************************************************/
void web_page_footer(FILE *f)
{
   /* Display web page footer */
   fprintf(f, "<table style=\"text-align: left; width: 600px; background-color: rgb(0, 0, 153);\" border=\"0\" cellpadding=\"2\" cellspacing=\"2\"><tbody>\r\n");
   fprintf(f, "<tr><td style=\"vertical-align: top; text-align: center;\"><span style=\"font-family: Helvetica,Arial,sans-serif; color: white;\">crelay | version %s | %s</span></td></tr>\r\n",
           VERSION, DATE);
   fprintf(f, "</tbody></table></body></html>\r\n");    
}   


/**********************************************************
 * Function web_page_error()
 * 
 * Description:
 * 
 * Parameters:
 * 
 *********************************************************/
void web_page_error(FILE *f)
{    
   /* No relay card detected, display error message on web page */
   fprintf(f, "<br><table style=\"text-align: left; width: 600px; background-color: yellow; font-family: Helvetica,Arial,sans-serif; font-weight: bold; color: black;\" border=\"0\" cellpadding=\"2\" cellspacing=\"2\">\r\n");
   fprintf(f, "<tbody><tr style=\"font-size: 20px; font-weight: bold;\">\r\n");
   fprintf(f, "<td>No compatible relay card detected !<br>\r\n");
   fprintf(f, "<span style=\"font-size: 14px; color: grey;  font-weight: normal;\">This can be due to the following reasons:\r\n");
   fprintf(f, "<div>- No supported relay card is connected via USB cable</div>\r\n");
   fprintf(f, "<div>- The relay card is connected but it is broken</div>\r\n");
   fprintf(f, "<div>- There is no GPIO sysfs support available or GPIO pins not defined in %s\r\n", CONFIG_FILE);
   fprintf(f, "<div>- You are running on a multiuser OS and don't have root permissions\r\n");
   fprintf(f, "</span></td></tbody></table><br>\r\n");
}   


/**********************************************************
 * Function read_httppost_data()
 * 
 * Description:
 * 
 * Parameters:
 * 
 *********************************************************/
int read_httppost_data(FILE* f, char* data)
{
   char buf[256];
   int data_len=0;
   
   /* POST request: data is provided after the page header.
    * Therefore we skip the header and then read the form
    * data into the buffer.
    */
   while (fgets(buf, sizeof(buf), f) != NULL)
   {
      //printf("%s", buf);
      /* Find length of form data */
      if (strstr(buf, "Content-Length:"))
      {
         data_len=atoi(buf+strlen("Content-Length:"));
         //printf("DBG: data length is %d\n", data_len);
      }
      
      /* Find end of header (empty line) */
      if (!strcmp(buf, "\r\n")) break;
   }
   
   /* Get form data string */
   if (!fgets(data, data_len+1, f)) return -1;
   *(data+data_len) = 0;
   
   return data_len;
}


/**********************************************************
 * Function read_httpget_data()
 * 
 * Description:
 * 
 * Parameters:
 * 
 *********************************************************/
int read_httpget_data(char* buf, char* data)
{
   char *datastr;
         
   /* GET request: data (if any) is provided in the first line
    * of the page header. Therefore we first check if there is
    * any data. If so we read the data into a buffer.
    */
   *data = 0;
   if ((datastr=strchr(buf, '?')) != NULL)
   {
      strcpy(data, datastr+1);
   }
   
   return strlen(data);
}


/**********************************************************
 * Function process_http_request()
 * 
 * Description:
 * 
 * Parameters:
 * 
 *********************************************************/
int process_http_request(FILE *f)
{
   char buf[256];
   char *method;
   char *url;
   char formdata[64];
   char *datastr;
   int  relay=0;
   int  i;
   char com_port[MAX_COM_PORT_NAME_LEN];
   uint8 last_relay=FIRST_RELAY;
   relay_state_t rstate[MAX_NUM_RELAYS];
   relay_state_t nstate=INVALID;
   
   formdata[0]=0;  
   
   /* Read  first line of request header which contains 
    * the request method and url seperated by a space
    */
   if (!fgets(buf, sizeof(buf), f)) return -1;
   //printf("********** Raw data ***********\n");
   //printf("%s", buf);
   
   method = strtok(buf, " ");
   if (!method) return -1;
   //printf("method: %s\n", method);
   
   url = strtok(NULL, " ");
   if (!url) return -2;
   //printf("url: %s\n", url);
   
   fseek(f, 0, SEEK_CUR); // Force change of stream direction
   
   /* Check the request method we are dealing with */
   if (strcasecmp(method, "POST") == 0)
   {
      read_httppost_data(f, formdata);
   }
   else if (strcasecmp(method, "GET") == 0)
   {
      read_httpget_data(url, formdata);         
   }
   else
      return -3;
   
   //printf("DBG: form data: %s\n", formdata);
   
   /* Check if a relay card is present */
   if (detect_relay_card(com_port, &last_relay) == -1)
   {
      if (strstr(url, API_URL))
      {
         /* HTTP API request */
         fprintf(f, "ERROR: No compatible device detected !\r\n");
      }
      else
      {  
         /* Web page request */
         web_page_header(f);
         web_page_error(f);
         web_page_footer(f);
      }     
      return -1;
   }
   
   /* Process form data */
   if (strlen(formdata)>0)
   {
      /* Get values from form data */
      datastr = strstr(formdata, RELAY_TAG);
      if (datastr)
         relay = atoi(datastr+strlen(RELAY_TAG)+1);
      datastr = strstr(formdata, STATE_TAG);
      if (datastr)
         nstate = atoi(datastr+strlen(STATE_TAG)+1);
      
      //printf("DBG: Found data: relay=%d, state=%d\n\n", relay, nstate);
      
      if ((relay != 0) && (nstate != INVALID))
      {
         /* Perform the requested action here */
         if (nstate==PULSE)
         {
            /* Generate pulse on relay switch */
            get_relay(com_port, relay, &rstate[relay-1]);
            if (rstate[relay-1] == ON)
            {
               set_relay(com_port, relay, OFF);
               sleep(1);
               set_relay(com_port, relay, ON);
            }
            else
            {
               set_relay(com_port, relay, ON);
               sleep(1);
               set_relay(com_port, relay, OFF);
            }
         }
         else
         {
            /* Switch relay on/off */
            set_relay(com_port, relay, nstate);
         }
      }
   }
      
   /* Read current state for all relays */
   for (i=FIRST_RELAY; i<=last_relay; i++)
   {
      get_relay(com_port, i, &rstate[i-1]);
   }
   
   /* Send response to client */
   if (strstr(url, API_URL))
   {
      /* HTTP API request */
      for (i=FIRST_RELAY; i<=last_relay; i++)
      {
         fprintf(f, "Relay %d:%d\r\n", i, rstate[i-1]);
      }
   }
   else
   {
      /* Web request */
      char cname[MAX_RELAY_CARD_NAME_LEN];
      get_relay_card_name(get_relay_card_type(), cname);

      web_page_header(f);

      /* Display relay status and controls on web page */
      fprintf(f, "<img style=\"width: 250px; height: 250px;\" alt=\"Card image\" src=\"http://www.conrad.de/medias/global/ce/1000_1999/1900/1920/1928/192846_LB_00_FB.EPS_1000.jpg\">\r\n");
      fprintf(f, "<table style=\"text-align: left; width: 600px; background-color: white; font-family: Helvetica,Arial,sans-serif; font-weight: bold; font-size: 20px;\" border=\"0\" cellpadding=\"2\" cellspacing=\"3\"><tbody>\r\n");
      fprintf(f, "<tr style=\"font-size: 14px; background-color: lightgrey\">\r\n");
      fprintf(f, "<td style=\"width: 200px;\">%s<br><span style=\"font-style: italic; font-size: 12px; color: grey; font-weight: normal;\">on %s</span></td>\r\n", 
              cname, com_port);
      fprintf(f, "<td style=\"background-color: white;\"></td><td style=\"background-color: white;\"></td></tr>\r\n");
      for (i=FIRST_RELAY; i<=last_relay; i++)
      {
         fprintf(f, "<tr style=\"vertical-align: top; background-color: rgb(230, 230, 255);\">\r\n");
         fprintf(f, "<td style=\"width: 300px;\">Relay %d<br><span style=\"font-style: italic; font-size: 16px; color: grey;\">%s</span></td>\r\n", 
                 i, rlabels[i-1]);
         fprintf(f, "<td style=\"text-align: center; vertical-align: middle; width: 100px; background-color: %s;\">%s</td>\r\n", 
                 rstate[i-1]==ON?"red":"lightgrey", rstate[i-1]==ON?"ON":"OFF");
         fprintf(f, "<td style=\"text-align: center; vertical-align: middle; width: 100px;\"><form action='/' method='post'><input type='hidden' name='%s' value='%d' /><input type='hidden' name='%s' value='%d' /><input type='submit' title='Toggle relay' value='%s'></form></td>\r\n", 
                 RELAY_TAG, i, STATE_TAG, rstate[i-1]==ON?0:1, rstate[i-1]==ON?"off":"on");
         fprintf(f, "<td style=\"text-align: center; vertical-align: middle; width: 100px;\"><form action='/' method='post'><input type='hidden' name='%s' value='%d' /><input type='hidden' name='%s' value='2' /><input type='submit' title='Generate pulse' value='pulse'></form></td></tr>\r\n", 
                 RELAY_TAG, i, STATE_TAG);
      }
      fprintf(f, "</tbody></table><br>\r\n");
      
      web_page_footer(f);
   }
   return 0;
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
   char cname[30];
   printf("crelay, version %s\n\n", VERSION);
   printf("This utility provides a unified way of controlling different types of relay cards.\n");
   printf("Currently supported relay cards:\n");
   for(rtype=NO_RELAY_TYPE+1; rtype<LAST_RELAY_TYPE; rtype++)
   {
      get_relay_card_name(rtype, cname);
      printf("  - %s\n", cname);
   }
   printf("The card which is detected first will be used.\n");
   printf("\n");
   printf("The program can be run in interactive (command line) mode or in daemon mode with\n");
   printf("built-in web server.\n\n");
   printf("Interactive mode:\n");
   printf("    crelay -i | [<relay number>] [ON|OFF]\n\n");
   printf("       The state of any relay can be read or it can be changed to a new state.\n");
   printf("       If only the relay number is provided then the current state is returned,\n");
   printf("       otherwise the relays state is set to the new value provided as second parameter.\n");
   printf("       The USB communication port is auto detected. The first compatible device\n");
   printf("       found will be used.\n\n");
   printf("Daemon mode:\n");
   printf("    crelay -d [<relay1_label> [<relay2_label> [<relay3_label> [<relay4_label>]]]] \n\n");
   printf("       In daemon mode the built-in web server will be started and the relays\n");
   printf("       can be completely controlled via a Web browser GUI or HTTP API.\n");
   printf("       Optionally a personal label for each relay can be supplied which will\n");
   printf("       be displayed next to the relay name on the web page.\n\n");
   printf("       To access the web interface point your Web browser to the following address:\n");
   printf("       http://<my-ip-address>:%d\n\n", DEFAULT_SERVER_PORT);
   printf("       To use the HTTP API send a POST or GET request from the client to this URL:\n");
   printf("       http://<my-ip-address>:%d/%s\n\n", DEFAULT_SERVER_PORT, API_URL);
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
   if (argc==1)
   {
      print_usage();
      exit(EXIT_SUCCESS);
   }
   
   if (!strcmp(argv[1],"-d"))
   {
      /*****  Daemon mode *****/
      
      struct sockaddr_in sin;
      struct in_addr iface;
      int port=DEFAULT_SERVER_PORT;
      int sock;
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
         if (config.gpio_num_relays != 0) syslog(LOG_DAEMON | LOG_NOTICE, "gpio_num_relays: %u\n", config.gpio_num_relays);
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
      
      /* Parse command line for relay labels (overrides config file)*/
      for (i=0; i<argc-2 && i<MAX_NUM_RELAYS; i++)
      {
         strcpy(rlabels[i], argv[i+2]);
      }         
      
      /* Start build-in web server */
      sock = socket(AF_INET, SOCK_STREAM, 0);
      sin.sin_family = AF_INET;
      sin.sin_addr.s_addr = iface.s_addr;
      sin.sin_port = htons(port);
      bind(sock, (struct sockaddr *) &sin, sizeof(sin));      
      listen(sock, 5);
      syslog(LOG_DAEMON | LOG_NOTICE, "HTTP server listening on %s:%d\n", inet_ntoa(iface), port);      

      if (daemon(0, 0) == -1) {
	      syslog(LOG_DAEMON | LOG_ERR, "Failed to daemonize: %s", strerror(errno));
	      exit(EXIT_FAILURE);
      }

      while (1)
      {
         int s;
         FILE *f;

         /* Wait for request from web client */
         s = accept(sock, NULL, NULL);
         if (s < 0) break;
         
         f = fdopen(s, "a+");
         process_http_request(f);
         fclose(f);
         close(s);
      }
      
      close(sock);
   }
   else
   {
      /*****  Command line mode *****/
      
      relay_state_t rstate;
      char com_port[MAX_COM_PORT_NAME_LEN];
      char cname[MAX_RELAY_CARD_NAME_LEN];
      uint8 num_relays=FIRST_RELAY;
      int err;

      if (detect_relay_card(com_port, &num_relays) == -1)
      {
         printf("No compatible device detected.\n");
         
         if(geteuid() != 0)
         {
            printf("\nWarning: this program is currently not running with root priviledges !\n");
            printf("Therefore it might not be able to access your relay cards communication port.\n");
            printf("Consider invoking the program from the root account or use \"sudo ...\"\n");
         }
         
         exit(EXIT_FAILURE);
      }

      if (!strcmp(argv[1],"-i"))
      {
         if (get_relay_card_name(get_relay_card_type(), cname) == 0)
            printf("Detected relay card type is %s (on %s, %d channels)\n", cname, com_port, num_relays);
         exit(EXIT_SUCCESS);
      }
   
      switch (argc)
      {
         case 2:
            /* GET current relay state */
            if (get_relay(com_port, atoi(argv[1]), &rstate) == 0)
               printf("Relay %d is %s\n", atoi(argv[1]), (rstate==ON)?"on":"off");
	    else
	      exit(EXIT_FAILURE);
            break;
            
         case 3:
            /* SET new relay state */
            if (!strcmp(argv[2],"on") || !strcmp(argv[2],"ON")) 
               err = set_relay(com_port, atoi(argv[1]), ON);
            else if (!strcmp(argv[2],"off") || !strcmp(argv[2],"OFF")) 
               err = set_relay(com_port, atoi(argv[1]), OFF);
            else {
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
