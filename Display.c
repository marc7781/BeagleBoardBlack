#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <linux/i2c-dev.h>
#include <i2c/smbus.h>      
#include <byteswap.h>

int32_t reg32;
uint16_t * const reg16poi = (uint16_t *) &reg32; //Address reg32 wordwise
uint8_t  * const reg8poi  = (uint8_t *)  &reg32; //Address reg32 bytewise
int RGBfile;
int file;

//Skal køres inden der kan skrives til displayet
int display_init(char *bus, unsigned int address) {

    file = open(bus, O_RDWR);
    if (file < 0) { // If error
            fprintf(stderr, "ERROR: opening %s - %s\n", bus, strerror(errno));
            exit(1);
    }

    if (ioctl(file, I2C_SLAVE, address) == -1 ) { // If error
            fprintf(stderr, "ERROR: setting  address %d on i2c bus %s with ioctl() - %s", address, bus, strerror(errno) );
            exit(1);
    }

    i2c_smbus_write_byte_data(file, 0x00, 0x28); //sætter displayet til at kører med linjer med 5*8 pixels
    i2c_smbus_write_byte_data(file, 0x00, 0x0E); //sætter blinker til off 0x0D = on
    i2c_smbus_write_byte_data(file, 0x00, 0x01); //Nulstiller Displayet
    i2c_smbus_write_byte_data(file, 0x00, 0x06); //sætter displayet til editor mode
    i2c_smbus_write_byte_data(file, 0x00, 0x02); //ikke helt sikker på hvad den gør

    return(file);
}
//Skal køres inden rgb kan bruges
int displayRGB_init(char *bus, unsigned int address){
    
    RGBfile = open(bus, O_RDWR);
    if (RGBfile < 0) { // If error
            fprintf(stderr, "ERROR: opening %s - %s\n", bus, strerror(errno));
            exit(1);
    }

    if (ioctl(RGBfile, I2C_SLAVE, address) == -1 ) { // If error
            fprintf(stderr, "ERROR: setting  address %d on i2c bus %s with ioctl() - %s", address, bus, strerror(errno) );
            exit(1);
    }

    return(RGBfile);
}

//Skriver til displayet
//X er hvilken af de 2 linjer som der skal skrives til
//Y er hvilken plads på linjen som den skal starte på
//txt er indholdet du vil skrive
void display_write(int x, int y, char *txt) {
    int len;
    char buf[16];

    if (x == 1){
        reg32 = y + 0x80; //for display til at starte på linje 1 + y for antal felter til højre
        i2c_smbus_write_byte_data(file, 0x00, reg8poi[0]); 
    }
    else if (x == 2){
        reg32 = y + 0xC0; //for display til at starte på linje 2 + y for antal felter til højre
        i2c_smbus_write_byte_data(file, 0x00, reg8poi[0]);
    }

    sprintf(buf, "%s", txt);

    len = strlen(buf); //finder længene af den string som man har sendt med
    for(int i = 0; i<len; i++){
        reg32 = buf[i];
        i2c_smbus_write_byte_data(file, 0x40, reg8poi[0]); //skriver hver char ud på displayet
    }
}

//Sletter alt på displayet så det står tomt
void display_Clear(){
    i2c_smbus_write_byte_data(file, 0x00, 0x01); //Nulstiller Displayet
    i2c_smbus_write_byte_data(file, 0x00, 0x80); //sætter displayet til første linje
    i2c_smbus_write_byte_data(file, 0x00, 0x06); //sætter displayet til editor
}

//Sætter baggrundslyset på displayet
//red er hvor meget rød fra 0-255
//green er hvor meget grøn fra 0-255
//blue er hvor meget blå fra 0-255
void display_setColor(int red, int green, int blue){
    reg8poi[0] = red;
    reg8poi[1] = green;
    reg8poi[2] = blue;
    
    
    i2c_smbus_write_byte_data(RGBfile, 0x00, 0x00); //nulstiller mode 1
    
    i2c_smbus_write_byte_data(RGBfile, 0x01, 0x00); //nulstiller mode 2

    i2c_smbus_write_byte_data(RGBfile, 0x08, 0x2A); //sætter led register til 10 på de første 3 PWM

    i2c_smbus_write_byte_data(RGBfile, 0x04, reg8poi[0]); //Red
    i2c_smbus_write_byte_data(RGBfile, 0x03, reg8poi[1]); //Green
    i2c_smbus_write_byte_data(RGBfile, 0x02, reg8poi[2]); //Blue

}