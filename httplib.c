/* **************************************************************************																					
 *                                OpenPicus                 www.openpicus.com
 *                                                            italian concept
 * 
 *            openSource wireless Platform for sensors and Internet of Things	
 * **************************************************************************
 *  FileName:        httplib.c
 *  Module:          FlyPort WI-FI - FlyPort ETH
 *  Compiler:        Microchip C30 v3.12 or higher
 *
 *  Author               Rev.    Date              Comment
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *  Diego Luca Candido   0.1     20/10/2012	First prototype
 *  
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 *  Software License Agreement
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *  This is free software; you can redistribute it and/or modify it under
 *  the terms of the GNU General Public License (version 2) as published by 
 *  the Free Software Foundation AND MODIFIED BY OpenPicus team.
 *  
 *  ***NOTE*** The exception to the GPL is included to allow you to distribute
 *  a combined work that includes OpenPicus code without being obliged to 
 *  provide the source code for proprietary components outside of the OpenPicus
 *  code. 
 *  OpenPicus software is distributed in the hope that it will be useful, but 
 *  WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 *  more details. 
 * 
 * 
 * Warranty
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * THE SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" WITHOUT
 * WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT
 * LIMITATION, ANY WARRANTY OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * WE ARE LIABLE FOR ANY INCIDENTAL, SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF
 * PROCUREMENT OF SUBSTITUTE GOODS, TECHNOLOGY OR SERVICES, ANY CLAIMS
 * BY THIRD PARTIES (INCLUDING BUT NOT LIMITED TO ANY DEFENSE
 * THEREOF), ANY CLAIMS FOR INDEMNITY OR CONTRIBUTION, OR OTHER
 * SIMILAR COSTS, WHETHER ASSERTED ON THE BASIS OF CONTRACT, TORT
 * (INCLUDING NEGLIGENCE), BREACH OF WARRANTY, OR OTHERWISE.
 *
 **************************************************************************/

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
		if(strcmp(req->method,"GET") == 0){
			int i;
			for(i = 0; i < req->parameters_size;++i){
				param_size += strlen(req->parameters[0]) + strlen(req->parameters[1])+2;
			}
		
			parameters = malloc(sizeof(param_size)*sizeof(char));
			memset(parameters,'\0',param_size);
			for(i = 0; i < req->parameters_size;++i){
				strcat(parameters,req->parameters[i][0]);
				strcat(parameters,"=");
				strcat(parameters,req->parameters[i][1]);
				strcat(parameters,"&");
			}	
		}
		else{
		// TO IMPLEMENT	
		}
	
	}
	char* str_req = (char*)malloc((sizeof(req->method)+sizeof(req->resource)+sizeof(req->version)+sizeof(req->host)+sizeof(param_size)+22)*sizeof(char));
	
	if(param_size == 0){
		sprintf(str_req,"%s %s %s\r\nHost: %s\r\n\r\n",req->method, req->resource,req->version,req->host);
	}
	else{
		sprintf(str_req,"%s %s?%s %s\r\nHost: %s\r\n\r\n",req->method, req->resource,parameters,req->version,req->host);
	}
	return str_req;
}



TCP_SOCKET* create_http_socket(char* host){
#ifdef DEBUG	
		UARTWrite(1,"### create_http_socket() ###\n");
#endif
	
	TCP_SOCKET *socket = (TCP_SOCKET*)malloc(sizeof(TCP_SOCKET));
	*socket = INVALID_SOCKET;
#ifndef PORT
	*socket = TCPClientOpen(host,PORT);
#else
	*socket = TCPClientOpen(host,"80");
#endif
	vTaskDelay(50);
	while(*socket==INVALID_SOCKET){
#ifdef DEBUG
		UARTWrite(1,"INVALID\n");
#endif
	}
	while(!TCPisConn(*socket)){
	vTaskDelay(50);
#ifdef DEBUG
	UARTWrite(1,"IS CONNECTING\n");	
#endif
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
		UARTWrite(1,"\n");
	}
#endif
	TCPWrite(*socket,request,strlen(request));
	if(TCPisConn(*socket)){
#ifdef DEBUG
		UARTWrite(1,"Socket is connected\n");
#endif
		;
	}
	else
		return -1;
	return 0;
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
	
	int rxlen;
	do{
	rxlen = TCPRxLen(*socket);
	vTaskDelay(2);
#ifdef DEBUG
	char mess[20];
	sprintf(mess,"TCP RX LEN: %d\n",rxlen);
	UARTWrite(1,mess);
#endif	
	}while(rxlen == 0);
	
	
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
	
	TCPClientClose(socket);
	free(socket);
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
	while(strcmp((buftmp = strtok(NULL,"\n")),"\r") != 0){
#ifdef DEBUG
		UARTWrite(1,"current strok: #");
		UARTWrite(1,buftmp);
		UARTWrite(1,"#\n");
		vTaskDelay(5);
#endif
		
	}
	char* rest_html = strtok(NULL,"");
	header_response->response = rest_html;
	
	
	char* first_match = strtok(response,"\n");
	
	char* response_first_version = strtok(first_match," ");
	header_response->version = response_first_version;
	
	char* response_first_code = strtok(NULL," ");
	header_response->code = response_first_code;
	
	char* response_first_status = strtok(NULL,"");
	header_response->status = response_first_status;
	
	
	
	
#ifdef DEBUG
		
	UARTWrite(1,first_match);
	UARTWrite(1,"\n");
	UARTWrite(1,response_first_version);
	UARTWrite(1,"\n");
UARTWrite(1,response_first_code);
	UARTWrite(1,"\n");
	UARTWrite(1,response_first_status);
	UARTWrite(1,"\n");
#endif
	
//	char* body;
	
/*	while(strncmp((body = strtok(response,"\n")),"\n",1)!=0){
#ifdef DEBUG
		UARTWrite(1,body);
		UARTWrite(1,"different from zero");
#endif
	}*/
	
	return header_response;
}