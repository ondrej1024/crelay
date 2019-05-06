/************************************************************************
 *
 * Relay card control utility: MQTT client program
 *
 * Description:
 *   This MQTT client subscribes to the specified topic on the specified
 *   broker and waits for control messages from a client.
 *   When a control message arrives the requested relay switch is performed
 *   and a status update is published to the specified status topic.
 *
 * Author:
 *    Ondrej Wisniewski
 *
 * Last modified:
 *   04/05/2019
 *
 * Build command:
 *    gcc mqttc.c -o mqttc -lmosquitto
 *
 * Uses libmosquitto:
 *    https://mosquitto.org/man/libmosquitto-3.html
 *
 * Send messages to this client with mosquitto_pub:
 *    mosquitto_pub -h test.mosquitto.org -m "my message" -t crelay/ctrl
 *
 * Copyright 2019, Ondrej Wisniewski
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
 ***********************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <syslog.h>
#include <uuid/uuid.h>
#include <mosquitto.h>
#include "relay_drv.h"

/* MQTT topic definitions */
#define CLIENT_ID "crelay"
#define TOPIC_SUB "ctrl"
#define TOPIC_PUB "status"

/* request tag definitions */
#define RELAY_TAG  "pin"
#define STATE_TAG  "status"
#define SERIAL_TAG "serial"

#define UUID_LEN   36

/* global variables */
char    *g_host;
uint16_t g_port;
char    *g_topic_sub;
char    *g_topic_pub;


/**********************************************************
 * Function uuidgen()
 *
 * Description: Generate a uuid string
 *
 * Parameters:
 *
 *********************************************************/
char* uuidgen(void)
{
    uuid_t binuuid;
    char *uuid = malloc(UUID_LEN+1);

    uuid_generate(binuuid);
    uuid_unparse(binuuid, uuid);

    return uuid;
}


/**********************************************************
 * Function relay_ctrl()
 *
 * Description: Relay control function
 *
 * Parameters:
 *
 *********************************************************/
static int relay_ctrl(char *message, char *status)
{
   char com_port[MAX_COM_PORT_NAME_LEN];
   char* serial=NULL;
   uint8_t last_relay=FIRST_RELAY;
   relay_state_t rstate;
   relay_state_t nstate=INVALID;
   int  relay=0;
   int  i;
   char *datastr;

   /* Check if a relay card is present */
   if (crelay_detect_relay_card(com_port, &last_relay, serial, NULL) == -1)
   {
      sprintf(status, "No relay card detected");
      return -1;
   }

   /* Check if request is not empty */
   if ((message != NULL) && (strlen(message) > 0))
   {
      /* Get values from message */
      int found=0;
      //printf("DBG: Found data: ");
      datastr = strstr(message, RELAY_TAG);
      if (datastr) {
         relay = atoi(datastr+strlen(RELAY_TAG)+1);
         printf("relay=%d", relay);
         found++;
      }
      datastr = strstr(message, STATE_TAG);
      if (datastr) {
         nstate = atoi(datastr+strlen(STATE_TAG)+1);
         printf("%sstate=%d", found ? ", " : "", nstate);
      }
      datastr = strstr(message, SERIAL_TAG);
      if (datastr) {
         serial = datastr+strlen(SERIAL_TAG)+1;
         datastr = strstr(serial, "&");
         if (datastr)
            *datastr = 0;
         //printf("%sserial=%s", found ? ", " : "", serial);
      }
      printf("\n");

      /* Perform action (if any) */
      if ((relay != 0) && (nstate != INVALID))
      {
         crelay_set_relay(com_port, relay, nstate, serial);
      }
   }

   /* Read current state for all relays */
   datastr = status;
   for (i=FIRST_RELAY; i<=last_relay; i++)
   {
      crelay_get_relay(com_port, i, &rstate, serial);
      sprintf(datastr, "Relay %d:%d<br>", i, rstate);
      datastr = datastr + strlen(datastr);
   }
   datastr = 0;

   return 0;
}


/**********************************************************
 * Function on_message()
 *
 * Description: MQTT client "message" event handler
 *
 * Parameters:
 *
 *********************************************************/
static void on_message(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *msg)
{
   char status[100];

   printf("%s: %s (%d bytes)\n", msg->topic, (const char *)msg->payload, msg->payloadlen);

   // Perform requested action and read status
   relay_ctrl(msg->payload, status);

   // Publish status response
   if (mosquitto_publish(mosq, NULL, g_topic_pub, strlen(status), status, 0 , 0) != MOSQ_ERR_SUCCESS) {
      printf("Publish failed\n");
   }
}


/**********************************************************
 * Function on_connect()
 *
 * Description: MQTT client "connect" event handler
 *
 * Parameters:
 *
 *********************************************************/
static void on_connect(struct mosquitto *mosq, void *userdata, int result)
{
   if(!result){
      printf("Connected to %s\n", g_host);
      syslog(LOG_DAEMON | LOG_NOTICE, "MQTT client connected to broker %s:%d\n", g_host, g_port);

      if (mosquitto_subscribe(mosq, NULL, g_topic_sub, 0) == MOSQ_ERR_SUCCESS){
         printf("Subscribed to topic %s\n", g_topic_sub);
         syslog(LOG_DAEMON | LOG_NOTICE, "Subscribed to topic %s\n", g_topic_sub);
      }
   }
   else{
      printf("Connection to %s failed\n", g_host);
      mosquitto_disconnect(mosq);
   }
}


/**********************************************************
 * Function on_disconnect()
 *
 * Description: MQTT client "disconnect" event handler
 *
 * Parameters:
 *
 *********************************************************/
static void on_disconnect(struct mosquitto *mosq, void *userdata, int result)
{
   printf("Disconnected from %s\n", g_host);
   syslog(LOG_DAEMON | LOG_ERR, "MQTT client disconnected from broker\n");
}


/**********************************************************
 * Function init_mqtt()
 *
 * Description: MQTT client initialization
 *
 * Parameters:
 *
 *********************************************************/
int init_mqtt(const char *host, uint16_t port, const char *custom_id)
{
   int rc;
   char *id;
   struct  mosquitto *mosq = NULL;

   mosquitto_lib_init();

   id = malloc(strlen(CLIENT_ID)+UUID_LEN+2);
   sprintf(id, "%s-%s", CLIENT_ID, uuidgen());
   mosq = mosquitto_new(id, 1, NULL);
   if(!mosq){
      mosquitto_lib_cleanup();
      return 1;
   }

   g_host = strdup(host);
   g_port = port;

   g_topic_sub = malloc(strlen(CLIENT_ID)+strlen(custom_id)+strlen(TOPIC_SUB)+3);
   sprintf(g_topic_sub, "%s/%s/%s", CLIENT_ID, custom_id, TOPIC_SUB);

   g_topic_pub = malloc(strlen(CLIENT_ID)+strlen(custom_id)+strlen(TOPIC_PUB)+3);
   sprintf(g_topic_pub, "%s/%s/%s", CLIENT_ID, custom_id, TOPIC_PUB);

   mosquitto_connect_callback_set(mosq, on_connect);
   mosquitto_message_callback_set(mosq, on_message);
   mosquitto_disconnect_callback_set(mosq, on_disconnect);

   printf("Connecting to MQTT broker %s:%d with client id %s\n", g_host, g_port, id);
   rc = mosquitto_connect_async(mosq, g_host, g_port, 60);
   rc = mosquitto_loop_start(mosq);

   return rc;
}
