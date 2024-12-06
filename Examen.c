#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <MQTTClient.h>
#include <unistd.h>


#define MAX_LINE_LEN	2048
//#define DEBUG // code works a bit when using manual input, error with the send code
#define ADDRESS     "tcp://192.168.0.103:1883"  // Local RP MQTT broker address
#define CLIENTID    "CFileReaderClient"
#define TOPIC       "P1/MD10"            // Replace with your topic
#define QOS         1
#define TIMEOUT     1000L
volatile MQTTClient_deliveryToken deliveredtoken;
int tarief;
char actueel_sp, actueel_sv;
float totaal_dagv = 0, totaal_nachtv = 0, totaal_dago = 0, totaal_nachto = 0, totaal_v = 0, totaal_o = 0, totaal_g = 0, totaal_gas = 0;
char time[50], timeExtra[30];
int i = 0;
//int day, month, year, hh, mm, ss;


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

void calculations(totaal_dagv, totaal_dago, totaal_nachtv, totaal_nachto, totaal_gas){

    


    totaal_v += (totaal_dagv + totaal_nachtv);
    totaal_o += (totaal_dago + totaal_nachto);
    totaal_g += (totaal_gas * 11.55);
}



// This function is called upon when an incoming message from mqtt is arrived
int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message) {
    ///sscanf(payload ,"%d.%d.%d-%d:%d:%d;%f,%f;%f;%f,%f;%f;%s;%f", &day, &month, &year, &hh, &mm, &ss, &tarief, &actueel_sv, &actueel_sp, &totaal_dagv, &totaal_nachtv, &totaal_dago, &totaal_nachto, &timeExtra, &totaal_gas);
    //sscanf(payload ,"%s;%d,%f;%f;%f,%f;%f;%f;%s;%f", time, &tarief, &actueel_sv, &actueel_sp, &totaal_dagv, &totaal_nachtv, &totaal_dago, &totaal_nachto, timeExtra, &totaal_gas);
    char *payload = message->payload;
    printf( "Msg in : <%s>\n", payload );

    char *token_str;

    char time_buffer[20];

    long double totaal_gasv;
    logToFile(payload);

    token_str = strtok(payload, ";");
    char *time = token_str;
    token_str = strtok(NULL, ";");
    char *tarief = token_str;
    token_str = strtok(NULL, ";");
    char *actueel_sv = token_str;
    token_str = strtok(NULL, ";");
    char *actueel_sp = token_str;
    token_str = strtok(NULL, ";");
    char *totaal_dagve = token_str;
    token_str = strtok(NULL, ";");
    char *totaal_nachtve = token_str;
    token_str = strtok(NULL, ";");
    char *totaal_dagop = token_str;
    token_str = strtok(NULL, ";");
    char *totaal_nachtop = token_str;
    token_str = strtok(NULL, ";");
    char *timeExtra = token_str;
    token_str = strtok(NULL, ";");
    totaal_gasv = strtold(token_str, NULL);
    
    
    if (i == 0){
        printf("STARTWAARDEN\n\n");
        printf("DATUM-TIJD: %s\nDAG\tTotaal verbruik\t = %s\nDAG\tTotaal opbrengst\t = %s\nNACHT\tTotaal verbruik\t = %s\nNACHT\tTotaal opbrengst\t = %s\nGAS\tTotaal verbruik\t = %ld\n", time, totaal_dagve, totaal_dagop,totaal_nachtve,totaal_nachtop,totaal_gasv);
        printf("-----------------------------------------------------------\nTOTALEN:\n-----------------------------------------------------------\n");
    }
    totaal_dagv = atof(totaal_dagve);
    totaal_dago = atof(totaal_dagop);
    totaal_nachtv = atof(totaal_nachtve);
    totaal_nachto = atof(totaal_nachtop);
    totaal_gas = totaal_gasv;

    calculations(totaal_dagv,totaal_dago,totaal_nachtv,totaal_nachto,totaal_gas);
    #ifdef DEBUG
    printf("calculations: %f, %f, %ld\n", totaal_v, totaal_o, totaal_g);
    #endif
    
    printf("Datum: %s\n--------------\nSTROOM:\n\tTotaal verbruik\t\t=\t%f kWh\n\tTotaal opbrengst\t=\t%f kWh\nGAS:\n\tTotaal verbruik\t\t=\t%ldkWh\n",time,totaal_v,totaal_o,totaal_g);

    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    i++;
    return 1;
}

void logToFile(const char *in) {
    FILE *file = fopen("logFile.log", "a");
    if (file == NULL) {
        perror("Error: cannot open file");
        return;
    }
    fprintf(file, "%s\n", in);
    fclose(file);
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