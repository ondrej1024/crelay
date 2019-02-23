/******************************************************************************
 * 
 * Relay card control utility: HTTP server for Web GUI and REST API
 * 
 * Description:
 * 
 * Author:
 *   Ondrej Wisniewski (ondrej.wisniewski *at* gmail.com)
 *
 * Last modified:
 *   22/02/2019
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
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <syslog.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include "relay_drv.h"

/* HTTP server defines */
#define SERVER "crelay/"
#define PROTOCOL "HTTP/1.1"
#define RFC1123FMT "%a, %d %b %Y %H:%M:%S GMT"
#define API_URL "gpio"
#define DEFAULT_SERVER_PORT 8000

/* HTML tag definitions */
#define RELAY_TAG "pin"
#define STATE_TAG "status"
#define SERIAL_TAG "serial"

extern char rlabels[MAX_NUM_RELAYS][32];


/**********************************************************
 * Function send_headers()
 * 
 * Description:
 * 
 * Parameters:
 * 
 *********************************************************/
static void send_headers(FILE *f, int status, char *title, char *extra, char *mime, 
                  int length, time_t date)
{
   time_t now;
   char timebuf[128];
   
   fprintf(f, "%s %d %s\r\n", PROTOCOL, status, title);
   fprintf(f, "Server: %s\r\n", SERVER);
   //fprintf(f, "Access-Control-Allow-Origin: *\r\n"); // TEST For test only
   now = time(NULL);
   strftime(timebuf, sizeof(timebuf), RFC1123FMT, gmtime(&now));
   fprintf(f, "Date: %s\r\n", timebuf);
   if (extra) fprintf(f, "%s\r\n", extra);
   if (mime) fprintf(f, "Content-Type: %s; charset=utf-8\r\n", mime);
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
 * Function java_script_src()
 * 
 * Description:
 * 
 * Parameters:
 * 
 *********************************************************/
static void java_script_src(FILE *f)
{
   fprintf(f, "<script type='text/javascript'>\r\n");
   fprintf(f, "function switch_relay(checkboxElem){\r\n");
   fprintf(f, "   var status = checkboxElem.checked ? 1 : 0;\r\n");
   fprintf(f, "   var pin = checkboxElem.id;\r\n");
   fprintf(f, "   var url = '/gpio?pin='+pin+'&status='+status;\r\n");
   fprintf(f, "   var xmlHttp = new XMLHttpRequest();\r\n");
   fprintf(f, "   xmlHttp.onreadystatechange = function () {\r\n");
   fprintf(f, "      if (this.readyState < 4)\r\n");
   fprintf(f, "         document.getElementById('status').innerHTML = '';\r\n");
   fprintf(f, "      else if (this.readyState == 4) {\r\n"); 
   fprintf(f, "         if (this.status == 0) {\r\n");
   fprintf(f, "            document.getElementById('status').innerHTML = \"Network error\";\r\n");
   fprintf(f, "            checkboxElem.checked = (status==0);\r\n");
   fprintf(f, "         }\r\n");
   fprintf(f, "         else if (this.status != 200) {\r\n");
   fprintf(f, "            document.getElementById('status').innerHTML = this.statusText;\r\n");
   fprintf(f, "            checkboxElem.checked = (status==0);\r\n");
   fprintf(f, "         }\r\n");
   // TODO: add update of all relays status here (according to API response in xmlHttp.responseText) 
   fprintf(f, "      }\r\n");
   fprintf(f, "   }\r\n");
   fprintf(f, "   xmlHttp.open( 'GET', url, true );\r\n");
   fprintf(f, "   xmlHttp.send( null );\r\n");
   fprintf(f, "}\r\n");
   fprintf(f, "</script>\r\n");
}


/**********************************************************
 * Function style_sheet()
 * 
 * Description:
 * 
 * Parameters:
 * 
 *********************************************************/
static void style_sheet(FILE *f)
{
   fprintf(f, "<style>\r\n");
   fprintf(f, ".switch {\r\n");
   fprintf(f, "  position: relative;\r\n");
   fprintf(f, "  display: inline-block;\r\n");
   fprintf(f, "  width: 60px;\r\n");
   fprintf(f, "  height: 34px;\r\n");
   fprintf(f, "}\r\n");
   fprintf(f, ".switch input {\r\n"); 
   fprintf(f, "  opacity: 0;\r\n");
   fprintf(f, "  width: 0;\r\n");
   fprintf(f, "  height: 0;\r\n");
   fprintf(f, "}\r\n");
   fprintf(f, ".slider {\r\n");
   fprintf(f, "  position: absolute;\r\n");
   fprintf(f, "  cursor: pointer;\r\n");
   fprintf(f, "  top: 0;\r\n");
   fprintf(f, "  left: 0;\r\n");
   fprintf(f, "  right: 0;\r\n");
   fprintf(f, "  bottom: 0;\r\n");
   fprintf(f, "  background-color: #ccc;\r\n");
   fprintf(f, "  -webkit-transition: .4s;\r\n");
   fprintf(f, "  transition: .4s;\r\n");
   fprintf(f, "}\r\n");
   fprintf(f, ".slider:before {\r\n");
   fprintf(f, "  position: absolute;\r\n");
   fprintf(f, "  content: \"\";\r\n");
   fprintf(f, "  height: 26px;\r\n");
   fprintf(f, "  width: 26px;\r\n");
   fprintf(f, "  left: 4px;\r\n");
   fprintf(f, "  bottom: 4px;\r\n");
   fprintf(f, "  background-color: white;\r\n");
   fprintf(f, "  -webkit-transition: .4s;\r\n");
   fprintf(f, "  transition: .4s;\r\n");
   fprintf(f, "}\r\n");
   fprintf(f, "input:checked + .slider {\r\n");
   fprintf(f, "  background-color: #2196F3;\r\n");
   fprintf(f, "}\r\n");
   fprintf(f, "input:focus + .slider {\r\n");
   fprintf(f, "  box-shadow: 0 0 1px #2196F3;\r\n");
   fprintf(f, "}\r\n");
   fprintf(f, "input:checked + .slider:before {\r\n");
   fprintf(f, "  -webkit-transform: translateX(26px);\r\n");
   fprintf(f, "  -ms-transform: translateX(26px);\r\n");
   fprintf(f, "  transform: translateX(26px);\r\n");
   fprintf(f, "}\r\n");
   fprintf(f, "</style>\r\n");   
}


/**********************************************************
 * Function web_page_header()
 * 
 * Description:
 * 
 * Parameters:
 * 
 *********************************************************/
static void web_page_header(FILE *f)
{
   /* Send http header */
   send_headers(f, 200, "OK", NULL, "text/html", -1, -1);   
   fprintf(f, "<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01//EN\" \"http://www.w3.org/TR/html4/strict.dtd\">\r\n");
   fprintf(f, "<html><head><title>Relay Card Control</title>\r\n");
   style_sheet(f);
   java_script_src(f);
   fprintf(f, "</head>\r\n");
   
   /* Display web page heading */
   fprintf(f, "<body><table style=\"text-align: left; width: 460px; background-color: #2196F3; font-family: Helvetica,Arial,sans-serif; font-weight: bold; color: white;\" border=\"0\" cellpadding=\"2\" cellspacing=\"2\">\r\n");
   fprintf(f, "<tbody><tr><td>\r\n");
   fprintf(f, "<span style=\"vertical-align: top; font-size: 48px;\">Relay Card Control</span><br>\r\n");
   fprintf(f, "<span style=\"font-size: 16px; color: rgb(204, 255, 255);\">Remote relay card control <span style=\"font-style: italic; color: white;\">made easy</span></span>\r\n");
   fprintf(f, "</td></tr></tbody></table><br>\r\n");  
}


/**********************************************************
 * Function web_page_footer()
 * 
 * Description:
 * 
 * Parameters:
 * 
 *********************************************************/
static void web_page_footer(FILE *f)
{
   /* Display web page footer */
   fprintf(f, "<table style=\"text-align: left; width: 460px; background-color: #2196F3;\" border=\"0\" cellpadding=\"2\" cellspacing=\"2\"><tbody>\r\n");
   fprintf(f, "<tr><td style=\"vertical-align: top; text-align: center;\"><span style=\"font-family: Helvetica,Arial,sans-serif; color: white;\"><a style=\"text-decoration:none; color: white;\" href=http://ondrej1024.github.io/crelay>crelay</a></span></td></tr>\r\n");
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
static void web_page_error(FILE *f)
{    
   /* No relay card detected, display error message on web page */
   fprintf(f, "<br><table style=\"text-align: left; width: 460px; background-color: yellow; font-family: Helvetica,Arial,sans-serif; font-weight: bold; color: black;\" border=\"0\" cellpadding=\"2\" cellspacing=\"2\">\r\n");
   fprintf(f, "<tbody><tr style=\"font-size: 20px; font-weight: bold;\">\r\n");
   fprintf(f, "<td>No compatible relay card detected !<br>\r\n");
   fprintf(f, "<span style=\"font-size: 14px; color: grey;  font-weight: normal;\">This can be due to the following reasons:\r\n");
   fprintf(f, "<div>- No supported relay card is connected via USB cable</div>\r\n");
   fprintf(f, "<div>- The relay card is connected but it is broken</div>\r\n");
   fprintf(f, "<div>- There is no GPIO sysfs support available or GPIO pins not defined in the config file\r\n");
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
static int read_httppost_data(FILE* f, char* data, size_t datalen)
{
   char buf[256];
   int data_len=0;
   
   /* POST request: data is provided after the page header.
    * Therefore we skip the header and then read the form
    * data into the buffer.
    */
   *data = 0;
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

   /* Make sure we're not trying to overwrite the buffer */
   if (data_len >= datalen)
     return -1;

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
static int read_httpget_data(char* buf, char* data, size_t datalen)
{
   char *datastr;
         
   /* GET request: data (if any) is provided in the first line
    * of the page header. Therefore we first check if there is
    * any data. If so we read the data into a buffer.
    *
    * Note that we may truncate the input if it's too long.
    */
   *data = 0;
   if ((datastr=strchr(buf, '?')) != NULL)
   {
       strncpy(data, datastr+1, datalen);
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
int process_http_request(int sock)
{
   FILE *fin, *fout;
   char buf[256];
   char *method;
   char *url;
   char formdata[64];
   char *datastr;
   int  relay=0;
   int  i;
   int  formdatalen;
   char com_port[MAX_COM_PORT_NAME_LEN];
   char* serial=NULL;
   uint8_t last_relay=FIRST_RELAY;
   relay_state_t rstate[MAX_NUM_RELAYS];
   relay_state_t nstate=INVALID;
   
   formdata[0]=0;  

   /* Open file for input */
   fin = fdopen(sock, "r");
   
   /* Read  first line of request header which contains 
    * the request method and url seperated by a space
    */
   if (!fgets(buf, sizeof(buf), fin)) 
   {
      fclose(fin);
      return -1;
   }
   //printf("********** Raw data ***********\n");
   //printf("%s", buf);
   
   method = strtok(buf, " ");
   if (!method) 
   {
      fclose(fin);
      return -1;
   }
   //printf("method: %s\n", method);
   
   url = strtok(NULL, " ");
   if (!url)
   {
      fclose(fin);
      return -2;
   }
   //printf("url: %s\n", url);
   
   /* Check the request method we are dealing with */
   if (strcasecmp(method, "POST") == 0)
   {
      formdatalen = read_httppost_data(fin, formdata, sizeof(formdata));
   }
   else if (strcasecmp(method, "GET") == 0)
   {
      formdatalen = read_httpget_data(url, formdata, sizeof(formdata));
   }
   else
   {
      fclose(fin);
      return -3;
   }
   
   //printf("DBG: form data: %s\n", formdata);
   
   /* Open file for output */
   fout = fdopen(sock, "w");

   /* Send an error if we failed to read the form data properly */
   if (formdatalen < 0) {
     send_headers(fout, 500, "Internal Error", NULL, "text/html", -1, -1);
     fprintf(fout, "ERROR: Invalid Input. \r\n");
     goto done;
   }

   /* Get values from form data */
   if (formdatalen > 0) 
   {
      int found=0;
      //printf("DBG: Found data: ");
      datastr = strstr(formdata, RELAY_TAG);
      if (datastr) {
         relay = atoi(datastr+strlen(RELAY_TAG)+1);
         //printf("relay=%d", relay);
         found++;
      }
      datastr = strstr(formdata, STATE_TAG);
      if (datastr) {
         nstate = atoi(datastr+strlen(STATE_TAG)+1);
         //printf("%sstate=%d", found ? ", " : "", nstate);
      }
      datastr = strstr(formdata, SERIAL_TAG);
      if (datastr) {
         serial = datastr+strlen(SERIAL_TAG)+1;
         datastr = strstr(serial, "&");
         if (datastr)
            *datastr = 0;
         //printf("%sserial=%s", found ? ", " : "", serial);
      }
      //printf("\n\n");
   }
   
   /* Check if a relay card is present */
   if (crelay_detect_relay_card(com_port, &last_relay, serial, NULL) == -1)
   {
      if (strstr(url, API_URL))
      {
         /* HTTP API request, send response */
         send_headers(fout, 503, "No compatible device detected", NULL, "text/plain", -1, -1);
         fprintf(fout, "ERROR: No compatible device detected");
      }
      else
      {  
         /* Web page request */
         web_page_header(fout);
         web_page_error(fout);
         web_page_footer(fout);
      }     
   }
   else
   {  
      /* Process form data */
      if (formdatalen > 0)
      {
         if ((relay != 0) && (nstate != INVALID))
         {
            /* Perform the requested action here */
            if (nstate==PULSE)
            {
               /* Generate pulse on relay switch */
               crelay_get_relay(com_port, relay, &rstate[relay-1], serial);
               if (rstate[relay-1] == ON)
               {
                  crelay_set_relay(com_port, relay, OFF, serial);
                  // FIXME sleep(config.pulse_duration);
                  crelay_set_relay(com_port, relay, ON, serial);
               }
               else
               {
                  crelay_set_relay(com_port, relay, ON, serial);
                  // FIXME sleep(config.pulse_duration);
                  crelay_set_relay(com_port, relay, OFF, serial);
               }
            }
            else
            {
               /* Switch relay on/off */
               crelay_set_relay(com_port, relay, nstate, serial);
            }
         }
      }
      
      /* Read current state for all relays */
      for (i=FIRST_RELAY; i<=last_relay; i++)
      {
         crelay_get_relay(com_port, i, &rstate[i-1], serial);
      }
      
      /* Send response to client */
      if (strstr(url, API_URL))
      {
         /* HTTP API request, send response */
         send_headers(fout, 200, "OK", NULL, "text/plain", -1, -1);
         for (i=FIRST_RELAY; i<=last_relay; i++)
         {
            fprintf(fout, "Relay %d:%d<br>", i, rstate[i-1]);
         }
      }
      else
      {
         /* Web request */
         char cname[MAX_RELAY_CARD_NAME_LEN];
         crelay_get_relay_card_name(crelay_get_relay_card_type(), cname);
         
         web_page_header(fout);
         
         /* Display relay status and controls on web page */
         fprintf(fout, "<table style=\"text-align: left; width: 460px; background-color: white; font-family: Helvetica,Arial,sans-serif; font-weight: bold; font-size: 20px;\" border=\"0\" cellpadding=\"2\" cellspacing=\"3\"><tbody>\r\n");
         fprintf(fout, "<tr style=\"font-size: 14px; background-color: lightgrey\">\r\n");
         fprintf(fout, "<td style=\"width: 200px;\">%s<br><span style=\"font-style: italic; font-size: 12px; color: grey; font-weight: normal;\">on %s</span></td>\r\n", 
                 cname, com_port);
         fprintf(fout, "<td style=\"background-color: white;\"></td><td style=\"background-color: white;\"></td></tr>\r\n");
         for (i=FIRST_RELAY; i<=last_relay; i++)
         {
            fprintf(fout, "<tr style=\"vertical-align: top; background-color: rgb(230, 230, 255);\">\r\n");
            fprintf(fout, "<td style=\"width: 300px;\">Relay %d<br><span style=\"font-style: italic; font-size: 16px; color: grey;\">%s</span></td>\r\n", 
                    i, rlabels[i-1]);
            fprintf(fout, "<td style=\"text-align: center; vertical-align: middle; width: 100px; background-color: white;\"><label class=\"switch\"><input type=\"checkbox\" %s id=%d onchange=\"switch_relay(this)\"><span class=\"slider\"></span></label></td>\r\n", 
                    rstate[i-1]==ON?"checked":"",i);
         }
         fprintf(fout, "</tbody></table><br>\r\n");
         fprintf(fout, "<span id=\"status\" style=\"font-size: 16px; color: red; font-family: Helvetica,Arial,sans-serif;\"></span><br><br>\r\n");
         
         web_page_footer(fout);
      }
   }

 done:
   fclose(fout);
   fclose(fin);

   return 0;
}


static int http_loop(int sock)
{
   while (1)
   {
      int s;
      
      /* Wait for request from web client */
      s = accept(sock, NULL, NULL);
      if (s < 0) break;
      
      /* Process request */
      process_http_request(s);
      close(s);
   }
   
   close(sock);
   return 0;
}


int init_http(struct in_addr iface, int p)
{
   int sock;
   int port = (p==0)?DEFAULT_SERVER_PORT:p;
   struct sockaddr_in sin;
   pid_t pid;

   /* Start build-in web server */
   sock = socket(AF_INET, SOCK_STREAM, 0);
   sin.sin_family = AF_INET;
   sin.sin_addr.s_addr = iface.s_addr;
   sin.sin_port = htons(port);
   if (bind(sock, (struct sockaddr *) &sin, sizeof(sin)) != 0)
   {
      syslog(LOG_DAEMON | LOG_ERR, "Failed to bind socket to port %d : %s", port, strerror(errno));
      //exit(EXIT_FAILURE);
      return -1;
   }
   if (listen(sock, 5) != 0)
   {
      syslog(LOG_DAEMON | LOG_ERR, "Failed to listen to port %d : %s", port, strerror(errno));
      //exit(EXIT_FAILURE);         
      return -2;
   }
   
   syslog(LOG_DAEMON | LOG_NOTICE, "HTTP server listening on %s:%d\n", inet_ntoa(iface), port);      
   
   /* Create child */
   pid=fork();
   
   if (pid == -1)
   {
      return -3;
   }
   
   if (pid == 0)
   {
      /* This is the child process */
      http_loop(sock);
   }
   
   return 0;
}
