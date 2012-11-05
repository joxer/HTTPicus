// Diego Luca Candido
#ifndef TASK_FLYPORT_H
#define TASK_FLYPORT_H
#include "taskFlyport.h"
#endif
struct HTTP_HEADER_REQUEST{

  char* method;
  char* resource;
  char* version;
  char* host;
  char** parameters;
  int parameters_size;
  char* content_type;
  
};

struct HTTP_HEADER_RESPONSE{
	
	char* version;
	int code;
	char* status;
	char* response_body;
};

char* get_http_request(struct HTTP_HEADER_REQUEST*);
TCP_SOCKET* create_http_socket(char*);
int do_http_request(TCP_SOCKET*,char*);
int do_http_request_header(TCP_SOCKET*, struct HTTP_HEADER_REQUEST*);
char* http_get_response(TCP_SOCKET* );
char* do_http_request_and_get_response(TCP_SOCKET* , char* );
void closeSocket(TCP_SOCKET*);
struct HTTP_HEADER_RESPONSE* get_header_from_response(char* response);
