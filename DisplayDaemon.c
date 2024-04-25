#include <stdio.h>
#include "Display.h"
#include "Temperature.h"
#include "ip.h"
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <syslog.h>
#include <mosquitto.h>
#include <gpiod.h>
#include <pthread.h>

char buf[16];
int val;
struct gpiod_chip *chip;
struct gpiod_chip *chip2;
struct gpiod_line *button;
struct gpiod_line *buzzer;

double temperature;
char str[40];
time_t tid;

//Her starter min alert som får buzzeren til at larme når det bliver for varmt
void *alert(void * data){
    chip = gpiod_chip_open_by_name("gpiochip1"); //åbner forbindelse til chipen hvor ind-/udgangene er

    buzzer = gpiod_chip_get_line(chip, 19); //referer til buzzeren
    button = gpiod_chip_get_line(chip, 18); //referer til knappen
    
    gpiod_line_request_output(buzzer, "buzzer", 0); //sætter buzzer til output og sætter den i 0
    gpiod_line_request_input(button, "Button"); //sætter knappen til input

    while(1){
        val = gpiod_line_get_value(button); //tjekker på om knappen bliver trykket på og nulstiller efter
        if (val == 0){
            display_setColor(255,255,255);
            gpiod_line_set_value(buzzer, 0);
            gpiod_line_release(button);
            button = gpiod_chip_get_line(chip, 18);
            gpiod_line_request_output(button, "Button", 1);
            gpiod_line_release(button);
            button = gpiod_chip_get_line(chip, 18);
            gpiod_line_request_input(button, "Button");
        }
        sleep(1);
    }
}

//Her sender jeg en besked ud med temperatur og tid hvert 30. sekund
void *mosquitto(void * data){
    struct mosquitto *mosq1;
    mosquitto_lib_init();

    mosq1 = mosquitto_new(NULL, true, NULL);    
    mosquitto_connect(mosq1, "localhost", 1883, 60); //opretter forbindelse til sig selv som broker
    mosquitto_loop_start(mosq1);

    while(1){
        sleep(30);
        sprintf(str, "Tid: %d, Temperatur: %s", tid, buf);
        mosquitto_publish(mosq1, NULL, "Marcus/celcius", strlen(str), str, 0, 1); //sender beskeden med tid og temperatur
    }
}

double setTemperature(int temperaturefile){
    double temperature = getTemperature(temperaturefile); //henter temperaturen
    sprintf(buf, "%0.4f %s", temperature, "C");

    display_write(2,0,buf);
    if (temperature > 26){ //tjekker på temperaturen for at starte buzzeren hvis den er over 26 grader
        display_setColor(255,0,0);
        gpiod_line_set_value(buzzer, 1);
    }

    syslog (LOG_DEBUG, "Temperaturen er: %0.4f", temperature); //gemmer temperaturen i loggen
    
    return(temperature);
}

int main(void){ 
    int displayfile = display_init("/dev/i2c-2", 0x3e);
    int temperaturefile = temperature_init("/dev/i2c-2", 0x18);
    int rgbdisplayfile = displayRGB_init("/dev/i2c-2", 0x62);

    pthread_t pt[3];

    openlog ("Temperature", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL0); //åbner adgang til at kunne skrive til loggen

    display_setColor(255,255,255);

    pthread_create(&pt[0], NULL, alert, NULL); //starter thread til alarmen
    pthread_create(&pt[1], NULL, mosquitto, NULL); //starter thread til mosquitto

    char *ip = get_ip(); //henter ip adressen

    struct tm *tm;
    int hour;
    int min;
    char *clock;

    while(1){
        display_write(1,0,ip);
        for (int i = 0; i < 10; i++){
            tid = time(0); //finder tiden i unix timestamp
            tm = localtime(&tid); //giver tiden til en struct så det kan blive læst som timer og minutter
            hour = tm->tm_hour+2;
            min = tm->tm_min;
            sprintf(buf, "%2d %2d", hour, min);
            display_write(2,0,buf);
            usleep(500000);
            sprintf(buf, "%2d:%2d", hour, min);
            display_write(2,0,buf);
            usleep(500000);
        }        
        setTemperature(temperaturefile);
        sleep(2);
        display_Clear();
    }
}