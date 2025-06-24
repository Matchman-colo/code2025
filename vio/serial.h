#ifndef SERIAL_H
#define SERIAL_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>


#define HI_FALSE  -1  

#define HI_TRUE    0

#ifdef debugprintf
    #define debugpri(mesg, args...) fprintf(stderr, "[HI Serial print:%s:%d:]  "mesg"   \n", __FILE__, __LINE__, ##args) 
#else
    #define debugpri(mesg, args...)
#endif
// Global variable declarations
extern int HiSerfd; 
extern int whitePixelCount;
// Signal handling function

void HI_Serial_Close(int fd);

void Hi_sigsegv(int dummy);

// Initialize signal handling
void Hi_init_signals(void);

// Display usage information
int HI_Serial_Usage(void);

// Open the serial device
int HI_Serial_Open(char* HiSerDevice);

// Close the serial device
void HI_Serial_Close(int fd);

// Set serial port parameters
int HI_Serial_Set(int fd, int speed, int flow_ctrl, int databits, int stopbits, int parity);

// Initialize the serial port
int HI_Serial_Init(int fd, int speed, int flow_ctrl, int databits, int stopbits, int parity);

// Send data over the serial port
int HI_Serial_Send(int fd, char *send_buf, int data_len);

// Receive data from the serial port
int HI_Serial_Recv(int fd, char *rcv_buf, int data_len);

// Create a thread for receiving data
void *creat_trd_recv(void *param);


void serial_task(void);

const char* handle_received_data(const char* received);



#endif // SERIAL_H