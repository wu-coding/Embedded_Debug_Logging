#include "Logger.h"

const int PORT=8000;

const char IP_ADDR[]="172.23.22.179";

bool logger_running = true;

LOG_LEVEL filterLog;
pthread_t recThread;
pthread_mutex_t lock_client;

void *recv_func(void *arg);
int loggerfd;

struct sockaddr_in servaddr;
struct sockaddr_in clientaddr; 

int  InitializeLog(){

	if ((loggerfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("cannot create socket");
		return 0;
	}


	memset((char *)&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(PORT);
	inet_pton(AF_INET, IP_ADDR, &servaddr.sin_addr);
	socklen_t addrlen = sizeof(servaddr);


	pthread_mutex_init(&lock_client, NULL);
	int rc;
	filterLog = CRITICAL;
	rc = pthread_create(&recThread, NULL, recv_func, &loggerfd);
	if(rc!=0) {
		cout<<strerror(errno)<<endl;
		return -1;
	}
}
void SetLogLevel(LOG_LEVEL level){

	filterLog = static_cast<LOG_LEVEL>(level);
}
void Log(LOG_LEVEL level, const char *prog, const char *func, int line, const char *message){

	char buf[512];
	static_cast<LOG_LEVEL>(filterLog);
	if(level >= filterLog )
	{
        cout << "Greater than or equal" << endl;
		time_t now = time(0);
		char *dt = ctime(&now);
		memset(buf, 0, 512);
		char levelStr[][16]={"DEBUG", "WARNING", "ERROR", "CRITICAL"};
		int len;
		len = sprintf(buf, "%s %s %s:%s:%d %s\n", dt, levelStr[level], prog, func, line, message)+1;
		buf[len-1]='\0';
		socklen_t addrlen = sizeof(servaddr);

		sendto(loggerfd, buf, len, 0, (struct sockaddr *)&servaddr, addrlen);
	}else if(level < filterLog){
		cout << "Less than" << endl;
	}
}
void ExitLog(){
	logger_running = false;
	close(loggerfd);
}

void *recv_func(void *arg)
{
	socklen_t addrlen = sizeof(servaddr);
	int loggerfd = *(int *)arg;
	char buf[512];
	while(logger_running) {
		pthread_mutex_lock(&lock_client);
		memset(buf,0,512);
		int len = recvfrom(loggerfd, buf, 512, 0, (struct sockaddr *)&servaddr, &addrlen);
		pthread_mutex_unlock(&lock_client);
		if(len > 0) {
			cout<<"received message from server: "<<buf<<endl;
			if(strncmp("Q", buf, 14) == 0){
            exit(1);
			}else{ 				
				SetLogLevel(LOG_LEVEL(buf[0]));
			};
		}
		sleep(1);
	}
	pthread_exit(NULL);
}

