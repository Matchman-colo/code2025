#include "serial.h"
  
int HiSerfd;

void Hi_sigsegv(int dummy)

{

	if(HiSerfd > 0)

		HI_Serial_Close(HiSerfd);

	fprintf(stderr, "Hi Serial Caught SIGSEGV, Abort!\n");

	fclose(stderr);

	abort();

}

 

void Hi_sigterm(int dummy)

{

	if(HiSerfd > 0)

		HI_Serial_Close(HiSerfd);

	fprintf(stderr, "Hi Serial Caught SIGTERM, Abort!\n");

	fclose(stderr);

	exit(0);

}

 

void Hi_init_signals(void)

{

	struct sigaction sa;

	sa.sa_flags = 0;

	

	sigemptyset(&sa.sa_mask);

	sigaddset(&sa.sa_mask, SIGSEGV);

	sigaddset(&sa.sa_mask, SIGTERM);

	sigaddset(&sa.sa_mask, SIGPIPE);

	

	sa.sa_handler = Hi_sigsegv;

	sigaction(SIGSEGV, &sa, NULL);

 

	sa.sa_handler = Hi_sigterm;

	sigaction(SIGTERM, &sa, NULL);

 

	sa.sa_handler = SIG_IGN;

	sigaction(SIGPIPE, &sa, NULL);

}

 

int HI_Serial_Usage(void)

{

    printf("Usage:\n");

    printf("\tmyhicom [-d] <HiSerialDevice> [-s] get netdeviece info [-rw] read or wite select\n");

    printf("\tmyhicom [-h] for more usage\n");

    printf("\tmyhicom [-v] the verson of the sofware\n");

	printf("\tExample:\n\t ./3521d_232 -d /dev/ttyAMA1 -s 115200 -w HiSerial:HelloWorld\n");
	//printf("\tExample:\n\t ./serial -d /dev/ttyAMA1 -s 115200 -w HiSerial:HelloWorld\n");
	//printf("\tExample:\n\t ./485 -d /dev/ttyAMA4 -s 115200 -w HiSerial:HelloWorld\n");

}
/*

*Function: HI_Serial_Open(int fd,char* ComDevice)  

*Param: fd:file descirbe handle	 Serial Device: /dev/ttyAMA1 /dev/ttyAMA2

*Output: Ok or Fail

*/
int HI_Serial_Open(char* HiSerDevice)  

{  

    int fd;

		

	fd = open(HiSerDevice, O_RDWR|O_NOCTTY|O_NDELAY);  

	if (HI_FALSE == fd)  

	{  

		perror("HiSerial Can't Open Serial HiSerDevice");  

		return(HI_FALSE);  

	}  

	//恢复串口为阻塞状态                                 

	if(fcntl(fd, F_SETFL, 0) < 0)  

	{  

		debugpri("fcntl failed!\n");  

		return(HI_FALSE);  

	}       

	else  

	{  

		debugpri("fcntl=%d\n",fcntl(fd, F_SETFL,0));  

	}  

	//测试是否为终端设备      

	if(0 == isatty(STDIN_FILENO))  

	{  

		debugpri("standard input is not a terminal device\n");  

		return(HI_FALSE);  

	}  

	else  

	{  

		debugpri("isatty success!\n");  

	}                

	//printf("fd->open=%d\n",fd);  

	return fd;  

}  
/*

*Function: HI_Serial_Close(int fd) 

*Param: fd:file descirbe handle	 

*Output: Null

*/
void HI_Serial_Close(int fd)  

{  

	if(fd > 0)

		close(fd); 

	return;	

}  

 

/*

*Function: HI_Serial_Set(int fd,int speed,int flow_ctrl,int databits,int stopbits,int parity) 

*Param1: fd: file descirbe handle	 

*Param2: speed: select the Serial speed.115200,19200,9600...

*Param3: flow_ctrl: if use flow control

*Param4: databits: data bit select

*Param5: stopbits: stopbits select	

*Param5: parity: partiy select	

*Output: Ok or Fail

*/

int HI_Serial_Set(int fd,int speed,int flow_ctrl,int databits,int stopbits,int parity)  

{  

     

	int   i;  

	int   status;  

	int   speed_arr[] = { B115200, B19200, B9600, B4800, B2400, B1200, B300};  

	int   name_arr[] = {115200,  19200,  9600,  4800,  2400,  1200,  300};  

           

	struct termios options;  

     

 

	if( tcgetattr( fd,&options)  !=  0)  

	{  

		perror("SetupSerial 1");      

		return(HI_FALSE);   

	}  

    

    //set buater rate 

	for ( i= 0;  i < sizeof(speed_arr) / sizeof(int);  i++)  

	{  

		if  (speed == name_arr[i])  

		{               

			cfsetispeed(&options, speed_arr[i]);   

			cfsetospeed(&options, speed_arr[i]);    

		}  

	}       

     

    //set control model 

    options.c_cflag |= CLOCAL;  

    options.c_cflag |= CREAD;  
 

    //set flow control

    switch(flow_ctrl)  

    {  

        

		case 0 ://none  

              options.c_cflag &= ~CRTSCTS;  

              break;     

        

		case 1 ://use hard ware 

              options.c_cflag |= CRTSCTS;  

              break;  

		case 2 ://use sofware

              options.c_cflag |= IXON | IXOFF | IXANY;  

              break;  

    }  

  

    //select data bit   

    options.c_cflag &= ~CSIZE;  

    switch (databits)  

    {    

		case 5    :  

                     options.c_cflag |= CS5;  

                     break;  

		case 6    :  

                     options.c_cflag |= CS6;  

                     break;  

		case 7    :      

                 options.c_cflag |= CS7;  

                 break;  

		case 8:      

                 options.c_cflag |= CS8;  

                 break;    

		default:     

                 fprintf(stderr,"Unsupported data size\n");  

                 return (HI_FALSE);   

    }  

    //select parity bit 

    switch (parity)  

    {    

		case 'n':  

		case 'N':  //无奇偶校验位  

                 options.c_cflag &= ~PARENB;   

                 options.c_iflag &= ~INPCK;      

                 break;   

		case 'o':    

		case 'O':    

                 options.c_cflag |= (PARODD | PARENB);   

                 options.c_iflag |= INPCK;               

                 break;   

		case 'e':   

		case 'E':  

                 options.c_cflag |= PARENB;         

                 options.c_cflag &= ~PARODD;         

                 options.c_iflag |= INPCK;        

                 break;  

		case 's':  

		case 'S':    

                 options.c_cflag &= ~PARENB;  

                 options.c_cflag &= ~CSTOPB;  

                 break;   

        default:    

                 fprintf(stderr,"Unsupported parity\n");      

                 return (HI_FALSE);   

    }   

    // set stopbit  

    switch (stopbits)  

    {    

		case 1:     

                 options.c_cflag &= ~CSTOPB; break;   

		case 2:     

                 options.c_cflag |= CSTOPB; break;  

		default:     

                       fprintf(stderr,"Unsupported stop bits\n");   

                       return (HI_FALSE);  

    }  

     

	//set raw data output 

	options.c_oflag &= ~OPOST;  

    

	options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);  

	//options.c_lflag &= ~(ISIG | ICANON);  

     

    //set wait time  

    options.c_cc[VTIME] = 1;    

    options.c_cc[VMIN] = 1; 

     

  

    tcflush(fd,TCIFLUSH);  

     

    //set the attribute to HiSerial device 

    if (tcsetattr(fd,TCSANOW,&options) != 0)    

	{  

		perror("com set error!\n");    

		return (HI_FALSE);   

	}  

    return (HI_TRUE);   

}  

 

/*

*Function: HI_Serial_Init(int fd, int speed,int flow_ctrl,int databits,int stopbits,int parity)  

*Param: ...

*Output: HI_TRUE or HI_FALSE

*/

int HI_Serial_Init(int fd, int speed,int flow_ctrl,int databits,int stopbits,int parity)  

{  

    int err;  

    //设置串口数据帧格式  

    if (HI_Serial_Set(fd,speed,flow_ctrl,databits,stopbits,parity) == HI_FALSE)  

	{                                                           

		return HI_FALSE;  

	}  

    else  

	{  

		return  HI_TRUE;  

	}  

}  
int HI_Serial_Send(int fd, char *send_buf,int data_len)  

{  

    int len = 0;  

     

    len = write(fd,send_buf,data_len);  

    if (len == data_len )  

	{  

		debugpri("send data is %s\n",send_buf);

		return len;  

	}       

    else     

	{                   

		tcflush(fd,TCOFLUSH);  

		return HI_FALSE;  

	}  

     

} 

 

/*

*Function:  HI_Serial_Recv(int fd, char *rcv_buf,int data_len) 

*Param1: fd:file descirbe handle	

*Param2: rcv_buf:receive Data 

*Param2: data_len:receive Data  len

*Output: Receive Data len or HI_FALSE

*/

int HI_Serial_Recv(int fd, char *rcv_buf,int data_len)  

{  

	int len,fs_sel;  

    fd_set fs_read;  

     

    struct timeval time;  

     

    FD_ZERO(&fs_read);  

    FD_SET(fd,&fs_read);  

     

    time.tv_sec = 30;  

    time.tv_usec = 0;  

     

    //select fdset

    fs_sel = select(fd+1,&fs_read,NULL,NULL,&time);  

    if(fs_sel)  

	{  

		len = read(fd,rcv_buf,data_len);  

		debugpri("HiSeral Receive Data = %s len = %d fs_sel = %d\n",rcv_buf,len,fs_sel);  

		return len;  

	}  

    else  

	{  

		debugpri("Hiserial haven't data receive!");  

		return HI_FALSE;  

	}       

} 


const char* handle_received_data(const char* received) {
    if (strstr(received, "location")) {
		if(whitePixelCount>27000)
		{
			return "Dirty";
		}
		else
		{
			return "Clean";	
		}
    }
		else if(strstr(received, "exit")) 
	{
		exit(0);
	}
    return NULL;
}

/*

*Function: HI_Serial_Send(int fd, char *send_buf,int data_len)

*Param1: fd:file descirbe handle	

*Param2: send_buf:Data to be send

*Param2: data_len:Data len

*Output: Data send len or HI_FALSE

*/



 

/*

*Function:  HI_Serial_Recv(int fd, char *rcv_buf,int data_len) 

*Param1: fd:file descirbe handle	

*Param2: rcv_buf:receive Data 

*Param2: data_len:receive Data  len

*Output: Receive Data len or HI_FALSE

*/


void *creat_trd_recv(void *param) 
{
    int ret = 0;
    char recvbuf[100] = {0};
	char sendbuf[100] = {0};
    int HiSerfd = *(int *)param;

    while (1)
    {
        ret = HI_Serial_Recv(HiSerfd, recvbuf, sizeof(recvbuf));  
        if (ret > 0)
        {
            printf("Recv Data >> %s\n", recvbuf); 
        }
    }
    return NULL;
}



void serial_task(void)
	{
		char HiSerialDev[32] = "/dev/ttyAMA1"; // 默认设备路径
		int SerialSpeed = 115200; // 默认波特率
		char recvbuf[100] = {0};
		char sendbuf[100] = {0};
		int len;
		debugpri("myHicom read\n");
		HiSerfd = HI_Serial_Open(HiSerialDev);
		HI_Serial_Init(HiSerfd, SerialSpeed, 0, 8, 1, 'N');
				len = HI_Serial_Recv(HiSerfd, recvbuf, sizeof(recvbuf));
			if (len > 0) {
				recvbuf[len] = '\0'; // Null-terminate the received data
				const char* response = handle_received_data(recvbuf);
				if (response) {
					snprintf(sendbuf, sizeof(sendbuf), "%s", response);
					//printf("%d",whitePixelCount);
					//snprintf(sendbuf, sizeof(sendbuf), "%s", response);
					 HI_Serial_Send(HiSerfd, sendbuf, strlen(sendbuf));
				} 	
				memset(recvbuf, 0, sizeof(recvbuf));
				memset(sendbuf, 0, sizeof(sendbuf));
			} 
		HI_Serial_Close(HiSerfd);
	}

