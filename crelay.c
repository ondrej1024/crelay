/******************************************************************************
 * 
 * Conrad USB 4-relay card utility
 * 
 * Description:
 *   Controls the relays on the USB 4-relay card from Conrad.
 *   http://www.conrad.de/ce/de/product/393905
 *   There are 3 ways to control the relais:
 *    1. via command line
 *    2. via web interface using a browser
 *    3. via HTTP API using a client application
 * 
 * Note:
 *   We need the cp210x driver for the Silabs CP2104 chip with GPIO support.
 *   The official in-kernel cp210x driver does currently not yet support
 *   GPIO operations. Therefore the Silabs driver needs to be used:
 *   http://www.silabs.com/products/mcu/pages/usbtouartbridgevcpdrivers.aspx
 * 
 * Author:
 *   Ondrej Wisniewski (ondrej.wisniewski *at* gmail.com)
 *
 * Build instructions:
 *   gcc -o crelay crelay.c
 * 
 * Last modified:
 *   21/01/2014
 *
 *****************************************************************************/ 

/******************************************************************************
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 * 
 *****************************************************************************/ 
 
/******************************************************************************
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
 *****************************************************************************/ 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <sys/socket.h>

#define VERSION "0.3"

#define SERVER "crelay/"VERSION
#define PROTOCOL "HTTP/1.0"
#define RFC1123FMT "%a, %d %b %Y %H:%M:%S GMT"
#define SERVER_PORT 8000
#define API_URL "gpio"

#define RELAY_TAG "pin"
#define STATE_TAG "status"

#define IOCTL_GPIOGET 0x8000
#define IOCTL_GPIOSET 0x8001

#define FIRST_RELAY 1
#define LAST_RELAY  4
#define NUM_RELAYS  (LAST_RELAY-FIRST_RELAY+1)
#define MAX_COM_PORTS 10

/* USB serial device created by cp210x driver */
#define SERIAL_DEV_BASE "/dev/ttyUSB"

typedef unsigned char  uint8;
typedef unsigned short uint16;
typedef unsigned long  uint32;

typedef enum {
   OFF=0,
   ON,
   PULSE
}
relay_state_t;

static char rlabels[NUM_RELAYS][32] = {"My appliance 1", "My appliance 2", "My appliance 3", "My appliance 4"};


/**********************************************************
 * Function detect_com_port()
 * 
 * Description:
 * 
 * Parameters:
 * 
 *********************************************************/
int detect_com_port(char* portname)
{
   uint8 found=0;
   uint32 gpio=0;
   int fd;
   int i;
   char pname[16];
   
   for (i=0; i<MAX_COM_PORTS; i++)
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


/**********************************************************
 * Function get_relay()
 * 
 * Description:
 * 
 * Parameters:
 * 
 *********************************************************/
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


/**********************************************************
 * Function set_relay()
 * 
 * Description:
 * 
 * Parameters:
 * 
 *********************************************************/
int set_relay(char* portname, int relay, relay_state_t relay_state)
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
   fprintf(f, "<html><head><title>Conrad 4 Channel Relay Card</title></head><body>\r\n");
   fprintf(f, "<table style=\"text-align: left; width: 600px; background-color: rgb(0, 0, 153); font-family: Helvetica,Arial,sans-serif; font-weight: bold; color: white;\" border=\"0\" cellpadding=\"2\" cellspacing=\"2\">\r\n");
   fprintf(f, "<tbody><tr><td>\r\n");
   fprintf(f, "<span style=\"vertical-align: top; font-size: 48px;\">Relay Card Control</span><br>\r\n");
   fprintf(f, "<span style=\"font-size: 16px; color: rgb(204, 255, 255);\">Conrad USB 4-channel Relay card controlling <span style=\"font-style: italic; color: white;\">made easy</span></span>\r\n");
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
   fprintf(f, "<tr><td style=\"vertical-align: top; text-align: center;\"><span style=\"font-family: Helvetica,Arial,sans-serif; color: white;\">crelay | version %s | 2014</span></td></tr>\r\n",
           VERSION);
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
   fprintf(f, "<td>No compatible device detected !<br>\r\n");
   fprintf(f, "<span style=\"font-size: 14px; color: grey;  font-weight: normal;\">This can be due to the following reasons:\r\n");
   fprintf(f, "<div>- No relay card is connected via USB cable</div>\r\n");
   fprintf(f, "<div>- The cp210x driver (with GPIO support) is not installed</div>\r\n");
   fprintf(f, "<div>- The relay card is broken</div>\r\n");
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
   int relay;  
   int i;
   char serial_port[16];
   relay_state_t rstate[NUM_RELAYS];
   relay_state_t nstate;
   
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
   if (detect_com_port(serial_port) == -1)
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
      
      /* Perform the requested action here */
      if (nstate==PULSE)
      {
         /* Generate pulse on relay switch */
         get_relay(serial_port, relay, &rstate[relay-1]);
         if (rstate[relay-1] == ON)
         {
            set_relay(serial_port, relay, OFF);
            sleep(1);
            set_relay(serial_port, relay, ON);
         }
         else
         {
            set_relay(serial_port, relay, ON);
            sleep(1);
            set_relay(serial_port, relay, OFF);
         }
      }
      else
      {
         /* Switch relay on/off */
         set_relay(serial_port, relay, nstate);
      }
   }
      
   /* Read current state for all relays */
   for (i=FIRST_RELAY; i<=LAST_RELAY; i++)
   {
      get_relay(serial_port, i, &rstate[i-1]);
   }
   
   /* Send response to client */
   if (strstr(url, API_URL))
   {
      /* HTTP API request */
      for (i=FIRST_RELAY; i<=LAST_RELAY; i++)
      {
         fprintf(f, "Relay %d:%d\r\n", i, rstate[i-1]);
      }
   }
   else
   {
      /* Web request */

      web_page_header(f);

      /* Display relay status and controls on web page */
      fprintf(f, "<img style=\"width: 250px; height: 250px;\" alt=\"Card image\" src=\"http://www.conrad.de/medias/global/ce/1000_1999/1900/1920/1928/192846_LB_00_FB.EPS_1000.jpg\">\r\n");
      fprintf(f, "<table style=\"text-align: left; width: 600px; background-color: white; font-family: Helvetica,Arial,sans-serif; font-weight: bold; font-size: 20px;\" border=\"0\" cellpadding=\"2\" cellspacing=\"3\"><tbody>\r\n");
      fprintf(f, "<tr style=\"font-size: 14px; background-color: lightgrey\">\r\n");
      fprintf(f, "<td style=\"width: 200px;\">Card 1<span style=\"font-style: italic; font-size: 12px; color: grey; font-weight: normal;\"> on %s</span></td>\r\n", serial_port);
      fprintf(f, "<td style=\"background-color: white;\"></td><td style=\"background-color: white;\"></td></tr>\r\n");
      for (i=FIRST_RELAY; i<=LAST_RELAY; i++)
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
#if 0
         fprintf(f, "<td style=\"text-align: center; vertical-align: middle; width: 100px;\"><form action='/' method='post'><input type='hidden' name='%s' value='%d' /><input name='%s' type='submit' title='Toggle relay' value='%s'></form></td>\r\n", 
                 RELAY_TAG, i, STATE_TAG, rstate[i-1]==ON?"off":"on");
         fprintf(f, "<td style=\"text-align: center; vertical-align: middle; width: 100px;\"><form action='/' method='post'><input type='hidden' name='%s' value='%d' /><input name='%s' type='submit' title='Generate pulse' value='pulse'></form></td></tr>\r\n", 
                 RELAY_TAG, i, STATE_TAG);
#endif
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
   printf("This is a utility to control the Conrad USB 4-relay card.\n");
   printf("The program can be run in interactive (command line) mode or in daemon mode with\n");
   printf("built-in web server.\n\n");
   printf("Interactive mode:\n");
   printf("    crelay [<relay number>] [ON|OFF]\n\n");
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
   printf("       http://<my-ip-address>:%d\n\n", SERVER_PORT);
   printf("       To use the HTTP API send a POST or GET request from the client to this URL:\n");
   printf("       http://<my-ip-address>:%d/%s\n\n", SERVER_PORT, API_URL);
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
      return 0;
   }
   
   if (!strcmp(argv[1],"-d"))
   {
      /*****  Daemon mode *****/
      
      struct sockaddr_in sin;
      int sock;
      int i;
      
      /* Parse command line */
      for (i=0; i<argc-2 && i<NUM_RELAYS; i++)
      {  
         strcpy(rlabels[i], argv[i+2]);
      }

      sock = socket(AF_INET, SOCK_STREAM, 0);
      sin.sin_family = AF_INET;
      sin.sin_addr.s_addr = INADDR_ANY;
      sin.sin_port = htons(SERVER_PORT);
      bind(sock, (struct sockaddr *) &sin, sizeof(sin));
      
      /* Start build-in web server */
      listen(sock, 5);
      printf("HTTP server listening on port %d\n", SERVER_PORT);
      
      
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
      char serial_port[16];
      
      if (detect_com_port(serial_port) == -1)
      {
         printf("No compatible device detected.\n");
         return -1;
      }
      
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
   }
   return 0;
}
