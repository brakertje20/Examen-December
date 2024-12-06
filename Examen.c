#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <MQTTClient.h>
#include <unistd.h>


#define MAX_LINE_LEN	2048

#define ADDRESS     "tcp://192.168.0.103:1883"  // Local RP MQTT broker address
#define CLIENTID    "CFileReaderClient"
#define TOPIC       "P1/MD10"            // Replace with your topic
#define QOS         1
#define TIMEOUT     1000L
volatile MQTTClient_deliveryToken deliveredtoken;
int tarief;
float actueel_sp, actueel_sv;
float totaal_dagv, totaal_nachtv, totaal_dago, totaal_nachto, totaal_v = 0, totaal_o = 0, totaal_g = 0, totaal_gas;
char time[50], timeExtra[30];
int i = 0, day, month, year, hh, mm, ss;


void delivered(void *context, MQTTClient_deliveryToken dt) {
    #ifdef DEBUG
        printf("Message with token value %d delivery confirmed\n", dt);
        printf( "-----------------------------------------------\n" );
    #endif    
    deliveredtoken = dt;
}
void connlost(void *context, char *cause) {
    printf("\nConnection lost\n");
    printf("cause: %s\n", cause);
}

void calculations(float totaal_dagv,float totaal_dago,float totaal_nachtv,float totaal_nachto,float totaal_gas){
    totaal_v += totaal_dagv + totaal_nachtv;
    totaal_o += totaal_dago + totaal_nachto;
    totaal_g += totaal_gas;
}



// This function is called upon when an incoming message from mqtt is arrived
int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message) {
    char *messageIn = message->payload;
    printf( "Msg in : <%s>\n", messageIn );
    sscanf("%d.%d.%d-%d:%d:%d;%f,%f;%f;%f,%f;%f;%s,%f", &day, &month, &year, &hh, &mm, &ss, &tarief, &actueel_sv, &actueel_sp, &totaal_dagv, &totaal_nachtv, &totaal_dago, &totaal_nachto, &timeExtra, &totaal_gas);
    if (i == 0){
        printf("STARTWAARDEN\n\n");
        printf("DATUM-TIJD: %s\nDAG\tTotaal verbruik\t = %f\nDAG\tTotaal opbrengst\t = %f\nNACHT\tTotaal verbruik\t = %f\nNACHT\tTotaal opbrengst\t = %f\nGAS\tTotaal verbruik\t = %f\n", time, totaal_dagv, totaal_dago,totaal_nachtv,totaal_nachto,totaal_gas);
        printf("-----------------------------------------------------------\nTOTALEN:\n-----------------------------------------------------------\n");
    }

    calculations(totaal_dagv,totaal_dago,totaal_nachtv,totaal_nachto,totaal_gas);
    printf("calculations: %f, %f, %f", totaal_v, totaal_o, totaal_g);
    


    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    i++;
    return 1;
}












int main() {
   // Open MQTT client for listening
    
    MQTTClient client;
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    int rc;

    MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;

    MQTTClient_setCallbacks(client, client, connlost, msgarrvd, delivered);
    printf("connecting to %s\n", ADDRESS);
    if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS) {
        printf("Failed to connect, return code %d\n", rc);
        exit(EXIT_FAILURE);
    }

    MQTTClient_subscribe(client, TOPIC, QOS);

    // Keep the program running to continue receiving and publishing messages
    for(;;) {
        sleep(1);
    }

    MQTTClient_disconnect(client, 10000);
    MQTTClient_destroy(&client);
    return rc;
}