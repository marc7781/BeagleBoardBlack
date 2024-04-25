#include <stdio.h>
#include "Display.h"
#include "Temperature.h"

int main(void){
    int displayFile = display_init("/dev/i2c-2", 0x3e);
    int temperatureFile = temperature_init("/dev/i2c-2", 0x18);
    int displayRGBFile = displayRGB_init("/dev/i2c-2", 0x62);
    double temperature = getTemperature(temperatureFile);
    char buf[16];
    sprintf(buf, "%0.4f %s", temperature, "Grader");
    display_write(1,0,"Temperaturen er:", displayFile);
    display_write(2,2,buf, displayFile);
    display_setColor(30,150,255, displayRGBFile);
}