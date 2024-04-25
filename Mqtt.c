#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <mosquitto.h>

//Author: heth@mercantec.dk
//Date..: 19. nov 2020
//Rev...: 11. april 2024
//Ver...: 0.1 beta
//
//
//INFORMATION
//Mosquitto docementation:
//   - https://mosquitto.org/documentation/
//   - https://github.com/eclipse/mosquitto (Se under exanples)
//Compile with : gcc mqtt_ex1.c -o mqtt_ex1 -lmosquitto

int number;

void on_connect1(struct mosquitto *mosq, void *obj, int result)
{
    int rc = MOSQ_ERR_SUCCESS;

    if(!result){
        mosquitto_subscribe(mosq, NULL, "Marcus/celcius", 0);
        //mosquitto_subscribe(mosq, NULL, "Marcus/cputemp/celcius", 0);
    }else{
        fprintf(stderr, "%s\n", mosquitto_connack_string(result));
    }
}

void on_message1(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message)
{
    struct mosquitto *mosq2 = (struct mosquitto *)obj;
        char *txtpoi, *txtpoi2;
        printf("struct mosquitto_message contains:\n");
        printf("  mid (int).............: %i\n", message->mid );
        printf("  topic (* char)........: %s\n", message->topic );
        printf("  payload (* void)......: %p", message->payload );
        txtpoi = malloc(message->payloadlen + 1);
        if ( txtpoi == 0 ) {
                fprintf( stderr, "Malloc error\n");
        } else {
                strncpy(txtpoi, message->payload, message->payloadlen);
                txtpoi[message->payloadlen] = 0;
                printf("  Message: [%s]\n", txtpoi);
        }
        printf("  payloadlen (int)......: %i\n", message->payloadlen );
        printf("  qos (int).............: %i\n", message->qos );
        printf("  retain (int)..........: %i\n", message->retain );

        //sprintf(txtpoi2, "#%i:%s", number, txtpoi);

        double celcius = (*txtpoi - 320) / 1.8;
        char* str = "henrik/cputemp/fahrenheitq";
        int result;

        //sprintf(str, "%s", celcius);

        //mosquitto_topic_matches_sub(message->topic,message->topic, result);

        printf("Resultet er: %d\n", result);
        if(strcmp(str, message->topic) == 0){
            mosquitto_publish(mosq2, NULL, "Marcus/cputemp/celcius", strlen(str), str, message->qos, message->retain);
        }

        free(txtpoi);
}

int main(int argc, char *argv[])
{
    struct mosquitto *mosq1, *mosq2;
    int version[3];
    number = 1; // Init mesage number

    mosquitto_lib_init();
    mosquitto_lib_version(&version[0],&version[1],&version[2]);
    printf("Mosquitto library version. %i.%i.%i\n", version[0], version[1], version[2]);

    mosq2 = mosquitto_new(NULL, true, NULL);
    mosq1 = mosquitto_new(NULL, true, mosq2);

    mosquitto_connect_callback_set(mosq1, on_connect1);
    mosquitto_message_callback_set(mosq1, on_message1);

    mosquitto_connect(mosq2, "localhost", 1883, 60);  // Replace localhost with IP address of broker
    mosquitto_connect(mosq1, "localhost", 1883, 60);  // Replace localhost with IP address of broker

    mosquitto_loop_start(mosq2);

    mosquitto_loop_forever(mosq1, -1, 1);

    mosquitto_destroy(mosq1);
    mosquitto_destroy(mosq2);

    mosquitto_lib_cleanup();

    return 0;
}