/************************************************************************
 * 
 * MQTT client program
 * 
 * This MQTT client subscribes to the specified topic on the specified
 * broker and waits for messages.
 * 
 * Author: 
 *    Ondrej Wisniewski
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
 ***********************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <mosquitto.h>
#include "relay_drv.h"

#define ID        "crelay"
#define TOPIC_SUB "crelay/ctrl"
#define TOPIC_PUB "crelay/status"
#define HOST      "test.mosquitto.org"
#define PORT      1883

/* request tag definitions */
#define RELAY_TAG "pin"
#define STATE_TAG "status"
#define SERIAL_TAG "serial"


int relay_ctrl(char *message, char *status)
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

void on_message(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *msg)
{
    char status[100];
   
    printf("%s: %s (%d bytes)\n", msg->topic, (const char *)msg->payload, msg->payloadlen);
    
    // Perform requested action and read status
    relay_ctrl(msg->payload, status);
    
    // TODO: Request status
    //strcpy(status, "Relay 1:0, Relay 2:0");
    
    // Publish status response
    if (mosquitto_publish(mosq, NULL,TOPIC_PUB, strlen(status), status, 0 , 0) != MOSQ_ERR_SUCCESS) {
       printf("Publish failed\n");
    }
}

void on_connect(struct mosquitto *mosq, void *userdata, int result)
{
	if(!result){
        printf("Connected to %s\n", HOST);
		mosquitto_subscribe(mosq, NULL, TOPIC_SUB, 0);
	}else{
        printf("Connection to %s failed\n", HOST);
		mosquitto_disconnect(mosq);
	}
}

void on_disconnect(struct mosquitto *mosq, void *userdata, int result)
{
     printf("Disconnected from %s\n", HOST);
}

int init_mqtt(void)
{
	int rc;
    struct mosquitto *mosq = NULL;

	mosquitto_lib_init();

	mosq = mosquitto_new(ID, 1, NULL);
	if(!mosq){
		mosquitto_lib_cleanup();
		return 1;
	}
	//if(client_opts_set(mosq, &cfg)){
	//	return 1;
	//}
	//mosquitto_log_callback_set(mosq, my_log_callback);
	//mosquitto_subscribe_callback_set(mosq, my_subscribe_callback);
	mosquitto_connect_callback_set(mosq, on_connect);
	mosquitto_message_callback_set(mosq, on_message);
    mosquitto_disconnect_callback_set(mosq, on_disconnect);

    printf("Connecting to MQTT broker...\n");
	rc = mosquitto_connect_async(mosq, HOST, PORT, 60);
    rc = mosquitto_loop_start(mosq);    

//     while (1) sleep(1);
//     
// 	mosquitto_destroy(mosq);
// 	mosquitto_lib_cleanup();

	return rc;
}
