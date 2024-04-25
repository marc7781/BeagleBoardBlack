#include <stdio.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

//Den returnere ip'en som er på eth0
//hvis den bliver kørt ved bootup så kan det godt lige taget et par sekunder
char * get_ip(){
    int n;
    struct ifreq ifr;
    char *buf = malloc(15);
    char array[] = "eth0";
 
    ifr.ifr_addr.sa_family = AF_INET; //sætter den til typen IPv4 adresse
    n = socket(AF_INET, SOCK_DGRAM, 0);
    strncpy(ifr.ifr_name , array , IFNAMSIZ - 1);
    while(1){
        ioctl(n, SIOCGIFADDR, &ifr);
        sprintf(buf, "%s", inet_ntoa(( (struct sockaddr_in *)&ifr.ifr_addr )->sin_addr));//laver ip'en om til en læselig ip adresse
        if(strcmp(buf,"0.0.0.0") != 0 && buf != NULL){//tjekker på om den har fået en ip
            break;
        }
        else{
            sleep(1);
        }
    }
    return buf;
}