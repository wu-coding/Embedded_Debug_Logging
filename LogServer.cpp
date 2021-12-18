#include <iostream>
#include<stdio.h>
#include <netinet/in.h>
#include<signal.h>
#include<unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <cstring>
#include <arpa/inet.h>
#include "Logger.h"
#include <fcntl.h>
#include <thread>        
#include <chrono> 

using namespace std;

static void sigHandler(int sig);
void *recv_func(void *arg);

bool server_running;
int sockfd;
int filefdWrite;
int filefdRead;
bool loop;

struct sockaddr_in recvaddr;
const int PORT=8000;

const char IP_ADDR[]="172.23.22.179";

pthread_t recThread;
pthread_mutex_t lock_server;

int main(void){

	server_running = true;
	struct sockaddr_in myaddr;
	char buf[512];
	pthread_mutex_init(&lock_server, NULL);	

	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("cannot create socket");
		return 0;
	}


	 memset((char *)&myaddr, 0, sizeof(myaddr));
	 myaddr.sin_family = AF_INET;

	 inet_pton(AF_INET, IP_ADDR, &myaddr.sin_addr);
	 myaddr.sin_port = htons(PORT);


	 if (bind(sockfd, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
		perror("Log Server: bind failed");
		return 0;
	}

    

	int rc;
	rc = pthread_create(&recThread, NULL, recv_func, &sockfd);
	if(rc!=0) {
		perror("Log Server: Thread create failed");
		return -1;
	}
	//signal 
	if( signal(SIGINT, sigHandler) == SIG_ERR){
		perror("Server: Signal Creation Error");
	}


	int enterChoice;
	loop = true;
    while(loop){
    cout << "Select from 3 options:" << endl;
	cout <<  "1. Set the log level" << endl;
	cout <<  "2. Dump the log file here" << endl;
	cout <<  "3. Shut down" << endl;
	cin >> enterChoice;
	switch(enterChoice){
		case 1: {
				int enterSeverity;
				LOG_LEVEL temp = DEBUG;

				cout << "Enter Log Severity: " << endl;
				cout <<	"Log Severity – 0 DEBUG, 1 WARNING, 2 ERROR, 3 CRITICAL" << endl;
				cin >> enterSeverity;

				memset(buf, 0, 512);
				int len;
				len=sprintf(buf, "%d", enterSeverity)+1;

	
				socklen_t addrlen = sizeof(recvaddr);
				sendto(sockfd, buf, len, 0, (struct sockaddr *)&recvaddr, addrlen);

				break;
			}
		case 2: {
				char buff[512];
				memset(buf, 0, 512);
				filefdRead =  open("logFile",O_RDONLY | O_NONBLOCK, 0666);
				while( read(filefdRead, buff, sizeof(buff)) != 0){
					cout << buff; 
				}
				break;
			}
		case 3: {
				loop = false;	
				server_running = false;
				pthread_join(recThread, NULL);
				close(filefdRead);
				close(filefdWrite);
				break;
			}
		default:
            cout << "Wrong Input" << endl;	
			break;
	}
	}
			socklen_t addrlen = sizeof(recvaddr);
			char quit[1];
			quit[0] = 'Q';
			sendto(sockfd, quit , sizeof(quit), 0, (struct sockaddr *)&recvaddr, addrlen);
}

void *recv_func(void *arg)
{
	filefdWrite = open("logFile", O_WRONLY | O_CREAT |O_NONBLOCK | O_TRUNC, 0666);
	int sockfd = *(int *)arg;
	int len;
	socklen_t addrlen = sizeof(recvaddr);
	char buf[512];

    cout << "server: recieve running" << endl;
	while(server_running) {
		pthread_mutex_lock(&lock_server);
		memset(buf,0,512);
		len = recvfrom(sockfd, buf, 512, 0, (struct sockaddr *)&recvaddr, &addrlen);
		pthread_mutex_unlock(&lock_server);
		if(len > 0) {	 
			write(filefdWrite, buf, 512);  
		}
		sleep(1);
	}
	pthread_exit(NULL);

}

static void sigHandler(int signum)
{
	switch(signum) {
		case SIGINT:
			cout<<"signalHandler("<<signum<<"): SIGINT"<<endl;
			loop = false;
			server_running=false;
			break;
		default:
			cout<<"signalHandler("<<signum<<"): unknown"<<endl;
	}
}
