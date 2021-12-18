#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include<stdio.h>
#include<signal.h>
#include<unistd.h>
#include <cstring>
#include <netinet/in.h>
#include <arpa/inet.h>

using namespace std;
// 0 1 2 3
enum LOG_LEVEL{ DEBUG = 0, WARNING = 1, ERROR =2 , CRITICAL = 3};	
	int  InitializeLog();
	void SetLogLevel(LOG_LEVEL level);
	void Log(LOG_LEVEL level, const char *prog, const char *func, int line, const char *message);
	void ExitLog();
	
