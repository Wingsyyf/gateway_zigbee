#ifndef _MYUART_H_
#define _MYUART_H_

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <stdlib.h>


#define UARTPORT "/dev/ttymxc2"
#define FALSE  -1
#define TRUE   0

#define DEFAULTSET   01
#define USERSET   02

void set_speed(int fd, int speed);
int set_Parity(int fd,int databits,int stopbits,int parity);
int open_port(void);

#endif
