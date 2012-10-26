// Diego Luca Candido



#include "httplib.h"

#ifndef TASK_FLYPORT_H
#include "taskFlyport.h"

#endif

char* get_http_request(struct HTTP_HEADER_REQUEST* req){
	
	// for now GET method only
#ifdef DEBUG
	UARTWrite(1,"### get_http_request() ###\n");
	
#endif
	int param_size = 0;
	char* parameters;
	if(req->parameters_size > 0){
			int i;
			for(i = 0; i < req->parameters_size;i+=2){
				param_size += strlen(req->parameters[i]) + strlen(req->parameters[i+1])+2;
			}
		
			parameters = malloc(param_size*sizeof(char));
			memset(parameters,'\0',param_size);
			for(i = 0; i < req->parameters_size;i+=2){
				strcat(parameters,req->parameters[i]);
				strcat(parameters,"=");
				strcat(parameters,req->parameters[i+1]);
				if((i+2) != req->parameters_size)
					strcat(parameters,"&");
			}	
	}
	
	
	char* str_req ;
	
	
	if(strcmp(req->method, "GET") == 0){
	str_req= (char*)malloc((strlen(req->method)+strlen(req->resource)+strlen(req->version)+strlen(req->host)+param_size+strlen(parameters)+38)*sizeof(char));
	}
	else if(strcmp(req->method,"POST") == 0){
	str_req = (char*)malloc((strlen(req->method)+strlen(req->resource)+strlen(req->version)+strlen(req->content_type)+strlen(req->host)+param_size+strlen(parameters)+38)*sizeof(char));
		
	}
	else{
	str_req = (char*)malloc((strlen(req->method)+strlen(req->resource)+strlen(req->version)+strlen(req->host)+param_size+38)*sizeof(char));
	
	}
	
	if(param_size == 0){
		if(strcmp(req->method,"GET") == 0){
			sprintf(str_req,"%s %s %s\r\nUser-Agent: HTTPicus 1.0\r\nHost: %s\r\n\r\n",req->method, req->resource,req->version,req->host);
		}
		else if(strcmp(req->method,"POST") == 0){
			sprintf(str_req,"%s %s %s\r\nUser-Agent: HTTPicus 1.0\r\nHost: %s\r\nContent-type: %s\r\nContent-Length: %d\r\n\r\n",req->method, req->resource,req->version,req->host,req->content_type,param_size);
		
		}
	
	}
	else{
		if(strcmp(req->method,"GET") == 0){
		sprintf(str_req,"%s %s?%s %s\r\nUser-Agent: HTTPicus 1.0\r\nHost: %s\r\n\r\n",req->method, req->resource,parameters,req->version,req->host);
		}
		else if(strcmp(req->method,"POST") == 0){
		sprintf(str_req,"%s %s %s\r\nUser-Agent: HTTPicus 1.0\r\nHost: %s\r\nContent-type: %s\r\nContent-Length: %d\r\n\r\n%s\r\n",req->method, req->resource,req->version,req->host,req->content_type,param_size,parameters);
		
		}
	}
	
	free(parameters);
	
	return str_req;
}



TCP_SOCKET* create_http_socket(char* host){
#ifdef DEBUG	
		UARTWrite(1,"### create_http_socket() ###\n");
#endif
	
	TCP_SOCKET *socket = (TCP_SOCKET*)malloc(sizeof(TCP_SOCKET));
	*socket = INVALID_SOCKET;
#ifdef PORT
	*socket = TCPClientOpen(host,PORT);
#else
	*socket = TCPClientOpen(host,"80");
#endif
	vTaskDelay(50);
	int i;
	
	while(*socket==INVALID_SOCKET){
		i+=1;
#ifdef DEBUG
		UARTWrite(1,"INVALID\n");
#endif
		vTaskDelay(10/portTICK_RATE_MS);
		if(i==50)
			break;
	}
	if(i == 50){
		UARTWrite(1,"Exit with error");
		return NULL;
	}
	
	while(!TCPisConn(*socket)){

		i+=1;
#ifdef DEBUG
		UARTWrite(1,"connecting\n");
#endif
		vTaskDelay(10/portTICK_RATE_MS);
		if(i == 400)
			break;
	}
	
	if(i == 400){
		UARTWrite(1,"Exit with error");
		return NULL;
	}
#ifdef DEBUG
	UARTWrite(1,"CONNECTED\n");
#endif
	return socket;
}

int do_http_request(TCP_SOCKET* socket,char* request){
#ifdef DEBUG	
		UARTWrite(1,"### do_http_request() ###\n");
#endif
	
#ifdef DEBUG
	if(TCPisConn(*socket)){
		UARTWrite(1,"connected\n");
		UARTWrite(1,request);
	}
#endif
	TCPWrite(*socket,request,strlen(request));
	if(TCPisConn(*socket)){
#ifdef DEBUG
		UARTWrite(1,"Socket is connected\n");
#endif
		return 0;
	}
	else
		return -1;
	
}

int do_http_request_header(TCP_SOCKET* socket, struct HTTP_HEADER_REQUEST* request){

#ifdef DEBUG
	UARTWrite(1,"### do_http_request_header() ###\n");
#endif
	
	char* sreq = get_http_request(request);
	
	TCPWrite(socket,sreq,strlen(sreq)*sizeof(char));
	
	if(TCPisConn(socket)){
#ifdef DEBUG
		UARTWrite(1,"Socket is connected\n");
#endif
		;
	}
	else
		return -1;
	return 0;
}

char* http_get_response(TCP_SOCKET* socket){
	
#ifdef DEBUG
		UARTWrite(1,"### http_get_response() ###\n");
#endif
	
	int rxlen,i=0;
	
		
	do{
	rxlen = TCPRxLen(*socket);
	vTaskDelay(2);
#ifdef DEBUG
	char mess[20];
	sprintf(mess,"TCP RX LEN: %d\n",rxlen);
	UARTWrite(1,mess);
	
#endif	
	vTaskDelay(10/portTICK_RATE_MS);

	}while(rxlen == 0 && i++ != 400);
	if(i == 401)
		return NULL;
	char *buffer = (char*)malloc(sizeof(char)*rxlen);


	memset(buffer,'\0',rxlen);
#ifdef DEBUG
	if(TCPisConn(*socket)){
		UARTWrite(1,"connected to read");
	}
#endif

	TCPRead(*socket,buffer,rxlen);
	return buffer;
	
}

char* do_http_request_and_get_response(TCP_SOCKET* socket, char* req){

#ifdef DEBUG
		UARTWrite(1,"### do_http_and_get_response() ###\n");
#endif
	
	do_http_request(socket,req);
	return http_get_response(socket);
}

void closeSocket(TCP_SOCKET* socket){

#ifdef DEBUG
		UARTWrite(1,"### close socket() ###\n");
#endif
	
	TCPClientClose(*socket);
	vTaskDelay(25);
	free(socket);
	vTaskDelay(5);
}

struct HTTP_HEADER_RESPONSE* get_header_from_response(char* response){
	
#ifdef DEBUG
	UARTWrite(1,"### get_header_from_response() ###\n");
	UARTWrite(1,"total body: ");
	UARTWrite(1,response);
	UARTWrite(1,"\n");
	vTaskDelay(5);
#endif
	
	
	
	struct HTTP_HEADER_RESPONSE *header_response = (struct HTTP_HEADER_RESPONSE*)malloc(sizeof(struct HTTP_HEADER_RESPONSE));
	
	//strip off first line of header
	
	strtok(response,"\r\n");
		
	char *buftmp;
	
	int i = 0;
#ifdef DEBUG
		UARTWrite(1,"i'm here\n");
#endif
	while(strcmp((buftmp = strtok(NULL,"\n")),"\r") != 0 && i++ != 10){
#ifdef DEBUG
		UARTWrite(1,"current strok: #");
		UARTWrite(1,buftmp);
		UARTWrite(1,"#\n");
		vTaskDelay(5);
#endif
		free(buftmp);
	}
	char* rest_html = strtok(NULL,"");
	header_response->response_body = rest_html;
	
	
	char* first_match = strtok(response,"\n");
	
	char* response_first_version = strtok(first_match," ");
	header_response->version = response_first_version;
	
	char* response_first_code = strtok(NULL," ");
	header_response->code = response_first_code;
	
	char* response_first_status = strtok(NULL,"");
	header_response->status = response_first_status;
	
	
	
	

	
//	char* body;
	
/*	while(strncmp((body = strtok(response,"\n")),"\n",1)!=0){
#ifdef DEBUG
		UARTWrite(1,body);
		UARTWrite(1,"different from zero");
#endif
	}*/
	
	return header_response;
}
