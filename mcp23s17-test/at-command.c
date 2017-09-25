/***************************************************************************
 *
 * $Revision: 2.0 $
 * $Date: Saturday, August 7, 2017 10:08:54 UTC
 * $Author: L.M.Zuccarelli
 *
 ****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> 
#include <fcntl.h> 
#include <termios.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <time.h>

#define blocksize 255
#define DEBUG 1int sfd;

char buf[blocksize];int initWiFi();

int ATWiFi();
int getBlock();
int getBlocks(int num, char target[]);
int getVersionWiFi();
int resetWiFi();
int setUARTWiFi();
int modeWiFi(int mode);
int scanWiFi();
int connectWiFi(char ssid[], char pass[]);
int getIPWiFi();
int startServerWiFi();
int getWebPageWiFi(char URL[], char page[]);/*

                                               int main(int argc, char** argv) {
                                               initWiFi();ATWiFi();
                                               connectWiFi("ssid", "password");
                                               getIPWiFi();
                                               startServerWiFi();
                                               return (EXIT_SUCCESS);
                                               }

                                               int initWiFi() {
                                               system("sudo systemctl stop
                                               serial-getty@ttyAMA0.service";);
                                               sfd = open("/dev/serial0", O_RDWR | O_NOCTTY);
                                               if (sfd == -1) {
                                               printf("Error no is : %d\n", errno);
                                               printf("Error description is : %s\n", strerror(errno));
                                               return -1;
                                               }
                                               struct termios options;
                                               tcgetattr(sfd, &options);
                                               cfsetspeed(&options, B115200);
                                               cfmakeraw(&options);
                                               options.c_cflag &= ~CSTOPB;
                                               options.c_cflag |= CLOCAL;
                                               options.c_cflag |= CREAD;
                                               options.c_cc[VTIME] = 1;
                                               options.c_cc[VMIN] = blocksize;
                                               tcsetattr(sfd, TCSANOW, &options);
                                               };

                                               int ATWiFi() {dprintf(sfd, "AT\r\n");
                                               return getBlock();
                                               }

                                               int getVersionWiFi() {
                                               dprintf(sfd, "AT+GMR\r\n");
                                               return getBlock();
                                               }

                                               int resetWiFi() {
                                               dprintf(sfd, "AT+RST\r\n");
                                               return getBlock();
                                               }

                                               int setUARTWiFi() {
                                               dprintf(sfd, "AT+UART_CUR=115200,8,1,0,0\r\n");
                                               return getBlock();
                                               }

                                               int modeWiFi(int mode) {
                                               dprintf(sfd, "AT+CWMODE_CUR=%d\r\n", mode);
                                               return getBlock();
                                               }

                                               int scanWiFi() {
                                               dprintf(sfd, "AT+CWLAP\r\n");
                                               return getBlocks(20, "OK");
                                               }

                                               int connectWiFi(char ssid[], char pass[]) {
                                               dprintf(sfd, "AT+CWJAP_CUR=\"%s\",\"%s\"\r\n",
                                               ssid, pass);
                                               return getBlocks(20, "OK");
                                               }

                                               int getIPWiFi() {
                                               dprintf(sfd, "AT+CIFSR\r\n");
                                               return getBlocks(10, "OK");
                                               }

int getWebPageWiFi(char URL[], char page[]) {
  dprintf(sfd, "AT+CIPSTART=\"TCP\",\"%s\",80\r\n", URL);
  if (getBlocks(10, "OK") < 0) return -1;
  char http[150];
  sprintf(http, "GET %s HTTP/1.1\r\nHost:%s\r\n\r\n",
      page, URL);
  dprintf(sfd, "AT+CIPSEND=%d\r\n", strlen(http));
  if (getBlocks(10, ">") < 0) return -1;
  dprintf(sfd, "%s", http);
  return getBlocks(10, "</html>");
}

int startServerWiFi() {
  char temp[blocksize];
  char id[10];
  dprintf(sfd, "AT+CIPMUX=1\r\n");
  if (getBlocks(10, "OK") < 0) return -1;
  dprintf(sfd, "AT+CIPSERVER=1,80\r\n");
  if (getBlocks(10, "OK") < 0) return -1;
  char data[] = "HTTP/1.0 200 OK\r\n
    Server:Pi\r\n
    Content-type: text/html\r\n\r\n
    <html><head><title>Temperature</title></head>
    <body><p>
    {\"humidity\":81%,\"airtemperature\":23.5C}
  </p></body></html>\r\n";
  for (;;) {
    if (getBlocks(1, "+IPD") < 0)continue;
    char *b = strstr(buf, "+IPD");
    b += 5;
    strncpy(temp, b, sizeof (temp));
    char *e = strstr(temp, ",");
    int d = e - temp;
    memset(id, '\0', sizeof (id));
    strncpy(id, temp, d);
    dprintf(sfd, "AT+CIPSEND=%s,%d\r\n",
        id, strlen(data));
    if (getBlocks(10, ">") < 0) return -1;
    dprintf(sfd, "%s", data);
    if (getBlocks(10, "OK") < 0) return -1;
    dprintf(sfd, "AT+CIPCLOSE=%s\r\n", id);
    if (getBlocks(10, "OK") < 0) return -1;
  }
}

int getBlock() {
  int bytes;
  struct timespec pause;
  pause.tv_sec = 0;
  pause.tv_nsec = 100000000;
  nanosleep(&pause, NULL);
  memset(buf, '\0', sizeof (buf));
  ioctl(sfd, FIONREAD, &bytes);
  if (bytes == 0)return 0;
  int count = read(sfd, buf, blocksize - 1);
  buf[count] = 0;
  if (DEBUG) {
    printf("%s", buf);
    fflush(stdout);
  }
  return count;
}

int getBlocks(int num, char target[]) {
  int i;
  struct timespec pause;
  pause.tv_sec = 1;
  pause.tv_nsec = 0;
  for (i = 0; i < num; i++) {
    nanosleep(&pause, NULL);
    getBlock();
    if (strstr(buf, target))return i;
  }
  return -1;
}
