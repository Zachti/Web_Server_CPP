#pragma once
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <string.h>
#include <time.h>
#include <string>
#include <fstream>
#include <iostream>
#include <cstring>

using namespace std;
#define OK 200
#define CREATED 201
#define NO_CONTENT 204
#define NOT_OK 400
#define NOT_FOUND 404
#define FAILED 412

constexpr int PORT = 8080;
constexpr int MAX_SOCKETS = 60;
constexpr int BUFFSIZE = 1024;

struct SocketState{
	SOCKET					id;
	enum eSocketStatus		recv;
	enum eSocketStatus		send;
	enum eRequestType		httpReq;
	char					buffer[BUFFSIZE];
	time_t					prevActivity;
	int						socketDataLen;
};

/*-----------------------------------------ENUM--=-----------------------------------------------*/
/*-----------------------------------------------------------------------------------------------*/
enum eSocketStatus {EMPTY,LISTEN,RECEIVE,IDLE,SEND};
enum eRequestType { GET = 1, HEAD, PUT, POST, DELETE1, TRACE, OPTIONS, NOT_ALLOWED_REQ };
/*-----------------------------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------------------------*/





/*------------------------------------FUNCTIONS-------------------------------------------------*/\
/*----------------------------------------------------------------------------------------------*/
bool addSocket(SOCKET id, enum eSocketStatus what, SocketState* sockets, int& socketsCount);
void removeSocket(int index, SocketState* sockets, int& socketsCount);
void acceptConnection(int index, SocketState* sockets, int& socketsCount);
void rcvMessage(int index, SocketState* sockets, int& socketsCount);
bool sendMessage(int index, SocketState* sockets);
int put(int index, char* filename, SocketState* sockets);
string get_field_value(const string& request, const string& field);
string GetQuery(const string& request, const string& param);
template <typename TP>
time_t parse_to_time_t(TP tp);
/*----------------------------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------------------------*/
