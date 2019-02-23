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
 *    mosquitto_pub -h test.mosquitto.org -m "my message" -t crelay-ctrl
 * 
 ***********************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <mosquitto.h>

#define ID    "crelay"
#define TOPIC "crelay-ctrl"
#define HOST  "test.mosquitto.org"
#define PORT  1883

void on_message(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *msg)
{
	printf("%s: %s (%d bytes)\n", msg->topic, (const char *)msg->payload, msg->payloadlen);
}

void on_connect(struct mosquitto *mosq, void *userdata, int result)
{
	if(!result){
        printf("Connected to %s\n", HOST);
		mosquitto_subscribe(mosq, NULL, TOPIC, 0);
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
