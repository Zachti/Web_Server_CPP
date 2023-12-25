#include "ServerFuncs.h"
#include <sstream>
#include <unordered_map>
#include <string>
#include <filesystem>

void rcvMessage(int index, SocketState* sockets, int& socketsCount)
{
	SOCKET msgSocket = sockets[index].id;

	int len = sockets[index].socketDataLen;
	int bytesRecv = recv(msgSocket, &sockets[index].buffer[len], sizeof(sockets[index].buffer) - len, 0);

	if (SOCKET_ERROR == bytesRecv)
	{
		cout << "HTTP Server: Error at recv(): " << WSAGetLastError() << endl;
		closesocket(msgSocket);
		removeSocket(index, sockets, socketsCount);
		return;
	}
	if (bytesRecv == 0)
	{
		closesocket(msgSocket);
		removeSocket(index, sockets, socketsCount);
		return;
	}
	else
	{
		sockets[index].buffer[len + bytesRecv] = '\0';
		cout << "HTTP Server: Recieved: " << bytesRecv << " bytes of \"" << &sockets[index].buffer[len] << " \message.\n\n";
		sockets[index].socketDataLen += bytesRecv;


		if (sockets[index].socketDataLen > 0)
		{

			sockets[index].send = SEND;

			if (strncmp(sockets[index].buffer, "GET", 3) == 0)
			{
				sockets[index].httpReq = GET;
				strcpy(sockets[index].buffer, &sockets[index].buffer[5]);
				sockets[index].socketDataLen = strlen(sockets[index].buffer);
				sockets[index].buffer[sockets[index].socketDataLen] = NULL;
				return;
			}
			else if (strncmp(sockets[index].buffer, "HEAD", 4) == 0)
			{
				sockets[index].httpReq = HEAD;
				strcpy(sockets[index].buffer, &sockets[index].buffer[6]);
				sockets[index].socketDataLen = strlen(sockets[index].buffer);
				sockets[index].buffer[sockets[index].socketDataLen] = NULL;
				return;
			}
			else if (strncmp(sockets[index].buffer, "PUT", 3) == 0)
			{
				sockets[index].httpReq = PUT;
				return;
			}
			else if (strncmp(sockets[index].buffer, "DELETE", 6) == 0)
			{
				sockets[index].httpReq = DELETE1;
				return;
			}
			else if (strncmp(sockets[index].buffer, "TRACE", 5) == 0)
			{
				sockets[index].httpReq = TRACE;
				strcpy(sockets[index].buffer, &sockets[index].buffer[5]);
				sockets[index].socketDataLen = strlen(sockets[index].buffer);
				sockets[index].buffer[sockets[index].socketDataLen] = NULL;
				return;
			}
			else if (strncmp(sockets[index].buffer, "OPTIONS", 7) == 0)
			{
				sockets[index].httpReq = OPTIONS;
				strcpy(sockets[index].buffer, &sockets[index].buffer[9]);
				sockets[index].socketDataLen = strlen(sockets[index].buffer);
				sockets[index].buffer[sockets[index].socketDataLen] = NULL;
				return;
			}
			else if (strncmp(sockets[index].buffer, "POST", 4) == 0)
			{
				sockets[index].httpReq = POST;
				strcpy(sockets[index].buffer, &sockets[index].buffer[6]);
				sockets[index].socketDataLen = strlen(sockets[index].buffer);
				sockets[index].buffer[sockets[index].socketDataLen] = NULL;
				return;
			}
			else
			{
				sockets[index].httpReq = NOT_ALLOWED_REQ;
				return;
			}
		}
	}
}



void acceptConnection(int index, SocketState* sockets, int& socketsCount)
{
	SOCKET id = sockets[index].id;
	sockets[index].prevActivity = time(0);
	struct sockaddr_in from;
	int fromLen = sizeof(from);

	SOCKET msgSocket = accept(id, (struct sockaddr*)&from, &fromLen);
	if (INVALID_SOCKET == msgSocket){
		cout << "HTTP Server: Error at accept(): " << WSAGetLastError() << endl;
		return;
	}
	cout << "HTTP Server: Client " << inet_ntoa(from.sin_addr) << ":" << ntohs(from.sin_port) << " is connected." << endl;
	unsigned long flag = 1;
	if (ioctlsocket(msgSocket, FIONBIO, &flag) != 0){
		cout << "HTTP Server: Error at ioctlsocket(): " << WSAGetLastError() << endl;
	}
	if (addSocket(msgSocket, RECEIVE, sockets, socketsCount) == false){
		cout << "\t\tToo many connections, dropped!\n";
		closesocket(id);
	}
}
bool addSocket(SOCKET id, enum eSocketStatus what, SocketState* sockets, int& socketsCount)
{
	for (int i = 0; i < MAX_SOCKETS; i++) {
		if (sockets[i].recv == EMPTY) {
			sockets[i].id = id;
			sockets[i].recv = what;
			sockets[i].send = IDLE;
			sockets[i].prevActivity = time(0);
			sockets[i].socketDataLen = 0;
			socketsCount++;
			return true;
		}
	}
	return false;
}
void removeSocket(int index, SocketState* sockets, int& socketsCount)
{
	sockets[index].recv = EMPTY;
	sockets[index].send = EMPTY;
	sockets[index].prevActivity = 0;
	socketsCount--;
	cout << "The socket number " << index << " has been removed\n" << endl;
}

bool sendMessage(int index, SocketState* sockets)
{
	int bytesSent = 0, buffLen = 0, fileSize = 0;
	char sendBuff[BUFFSIZE];
	char* subBuff;
	char tempBuff[BUFFSIZE], readBuff[BUFFSIZE];
	string message, fileSizeString, fileAddress;
	ifstream File;
	time_t currentTime;
	time(&currentTime); 
	SOCKET msgSocket = sockets[index].id;
	sockets[index].prevActivity = time(0); 
	switch (sockets[index].httpReq)
	{
	case HEAD:
	{
		subBuff = strtok(sockets[index].buffer, " ");
		fileAddress = "C:\\temp\\indexen.html"; 
		File.open(fileAddress);
		if (!File){
			message = "HTTP/1.1 " + to_string(NOT_FOUND) + " Not Found ";
			fileSize = 0;
		}
		else{
			message = "HTTP/1.1 " + to_string(OK) + " OK ";
			File.seekg(0, ios::end);
			fileSize = File.tellg();
		}
		time_t timtTType = parse_to_time_t(filesystem::last_write_time(filesystem::path{ fileAddress }));
		tm* gmt = gmtime(&timtTType);
		stringstream buffer;
		buffer << put_time(gmt, "%A, %d %B %Y %H:%M");
		string formattedFileModifiedTime = buffer.str();

		message += "\r\nContent-type: text/html";
		message += "\r\nDate:";
		message += formattedFileModifiedTime;
		message += "\r\nContent-length: ";
		fileSizeString = to_string(fileSize);
		message += fileSizeString;
		message += "\r\n\r\n";
		buffLen = message.size();
		strcpy(sendBuff, message.c_str());
		File.close();
		break;
	}
	case GET:
	{
		string FileContent = "";
		subBuff = strtok(sockets[index].buffer, " ");
		fileAddress = "C:\\temp\\index"; 
		string langValue = GetQuery(subBuff, "lang");
		if (langValue.empty())
		{
			fileAddress += "en";
		}
		else
		{
			fileAddress += langValue;
		}
		fileAddress.append(".html");
		File.open(fileAddress);
		if (!File)
		{
			message = "HTTP/1.1 " + to_string(NOT_FOUND) + " Not Found ";
			File.open("C:\\temp\\error.html");
		}
		else
		{
			message = "HTTP/1.1 " + to_string(OK) + " OK ";
		}

		if (File)
		{
			while (File.getline(readBuff, BUFFSIZE))
			{
				FileContent += readBuff;
				fileSize += strlen(readBuff);
			}
		}

		message += "\r\nContent-type: text/html";
		message += "\r\nDate:";
		message += ctime(&currentTime);
		message += "Content-length: ";
		fileSizeString = to_string(fileSize);
		message += fileSizeString;
		message += "\r\n\r\n";
		message += FileContent; 
		buffLen = message.size();
		strcpy(sendBuff, message.c_str());
		File.close();
		break;
	}

	case PUT:
	{
		char fileName[BUFFSIZE];
		int res = put(index, fileName, sockets);
		switch (res)
		{
		case FAILED: {
			cout << "PUT " << fileName << "Failed";
			message = "HTTP/1.1 " + to_string(FAILED) + " Precondition failed \r\nDate: ";
			break;
		}
		case OK: {
			message = "HTTP/1.1 " + to_string(OK) + " OK \r\nDate: ";
			break;
		}

		case CREATED: {
			message = "HTTP/1.1 " + to_string(CREATED) + " CREATED \r\nDate: ";
			break;
		}
		}

		message += ctime(&currentTime);
		message += "Content-length: ";
		fileSizeString = to_string(fileSize);
		message += fileSizeString;
		message += "\r\n\r\n";
		buffLen = message.size();
		strcpy(sendBuff, message.c_str());
		break;
	}
	case DELETE1:
	{
		string fileName = GetQuery(sockets[index].buffer, "fileName");

		fileName = string{ "C:\\temp\\" } + fileName;
		fileName += string{ ".txt" };
		if (remove(fileName.c_str()) != 0)
		{
			message = "HTTP/1.1 " + to_string(NO_CONTENT) + " File not found \r\nDate: ";
		}
		else
		{
			message = "HTTP/1.1 " + to_string(OK) + " OK DELETED \r\nDate: ";
		}

		message += ctime(&currentTime);
		message += "Content-length: ";
		fileSizeString = to_string(fileSize);
		message += fileSizeString;
		message += "\r\n\r\n";
		buffLen = message.size();
		strcpy(sendBuff, message.c_str());
		break;
	}
	case TRACE:
	{
		fileSize = strlen("TRACE");
		fileSize += strlen(sockets[index].buffer);
		message = "HTTP/1.1 " + to_string(OK) + " OK \r\nContent-type: message/http\r\nDate: ";
		message += ctime(&currentTime);
		message += "Content-length: ";
		fileSizeString = to_string(fileSize);
		message += fileSizeString;
		message += "\r\n\r\n";
		message += "TRACE";
		message += sockets[index].buffer;
		buffLen = message.size();
		strcpy(sendBuff, message.c_str());
		break;
	}

	case OPTIONS:{
		message = "HTTP/1.1 " + to_string(NO_CONTENT) + " No Content\r\nOptions: HEAD, GET, POST, PUT, TRACE, DELETE, OPTIONS\r\n";
		message += "Content-length: 0\r\n\r\n";
		buffLen = message.size();
		strcpy(sendBuff, message.c_str());
		break;
	}

	case POST:
	{
		message = "HTTP/1.1 " + to_string(OK) + " OK \r\nDate:";
		message += ctime(&currentTime);
		message += "Content-length: 0\r\n\r\n";
		string bodyMessage = get_field_value(string{ sockets[index].buffer }, string{ "body" });
		cout << "Message received: "<< bodyMessage<<"\n";
		buffLen = message.size();
		strcpy(sendBuff, message.c_str());
		break;
	}
	case NOT_ALLOWED_REQ:
		message = "HTTP/1.1 " + to_string(NOT_OK) + " BAD REQUEST \r\nDate:";
		message += ctime(&currentTime);
		message += "Content-length: 0\r\n\r\n";
		buffLen = message.size();
		strcpy(sendBuff, message.c_str());
	}

	bytesSent = send(msgSocket, sendBuff, buffLen, 0);
	memset(sockets[index].buffer, 0, BUFFSIZE);
	sockets[index].socketDataLen = 0;
	if (SOCKET_ERROR == bytesSent)
	{
		cout << "HTTP Server: Error at send(): " << WSAGetLastError() << endl;
		return false;
	}
	sockets[index].send = IDLE;
	return true;
}

int put(int index, char* filename, SocketState* sockets)
{
	int buffLen = 0;
	int CODE = OK, content_len = 0;
	string str_buffer = { sockets[index].buffer };
	string value, file_name;
	value = get_field_value(str_buffer, "Content-Length");
	content_len = stoi(value);
	file_name = GetQuery(str_buffer, "fileName");
	value = get_field_value(str_buffer, "body");
	strcpy(filename, file_name.c_str());
	file_name = string{ filename };
	file_name = string{ "C:\\temp\\" } + file_name + string{ ".txt" };
	fstream outPutFile;
	if (file_name.find("error") == string::npos){
		try{
			outPutFile.open(file_name);
			if (!outPutFile.good())	{
				outPutFile.open(file_name.c_str(), ios::out);
				CODE = CREATED;
			}
			if (!outPutFile.good()){
				cout << "HTTP Server: Error writing file to local storage: " << WSAGetLastError() << endl;
				CODE = FAILED;
			}
			if (value.empty()){
				CODE = NO_CONTENT;
			}
			else{
				outPutFile << value;
			}
		}
		catch (const exception&){
			outPutFile.close();
		}
	}
	else{
		cout << "HTTP Server: Error writing file to local storage: 'Error' name is not allowed" << endl;
		CODE = FAILED;
	}
	return CODE;
}

template <typename TP>
time_t parse_to_time_t(TP tp)
{
	using namespace chrono;
	auto sctp = time_point_cast<system_clock::duration>(tp - TP::clock::now()
		+ system_clock::now());
	return system_clock::to_time_t(sctp);
}

string get_field_value(const string& request, const string& field) {
	unordered_map<string, string> fields;
	string line, prev_line;
	stringstream request_stream(request);
	while (getline(request_stream, line)) {
		size_t sep = line.find(':');
		if (sep != string::npos) {
			string key = line.substr(0, sep);
			string value = line.substr(sep + 1);
			key = key.substr(key.find_first_not_of(" \t"));
			key = key.substr(0, key.find_last_not_of(" \t") + 1);
			value = value.substr(value.find_first_not_of(" \t"));
			value = value.substr(0, value.find_last_not_of(" \t") + 1);
			value.pop_back();
			fields[key] = value;
		}
		prev_line = line;
	}
	unordered_map<string, string>::iterator it = fields.find(field);
	if (it != fields.end()) {
		return it->second;
	}
	else {
		if (field.find("body") != string::npos)
		{
			string body = "";
			size_t body_start = request.find("\r\n\r\n");
			if (body_start != string::npos)
				body = request.substr(body_start + 4);
			return body;
		}
		return "";
	}
}
string GetQuery(const string& request, const string& param)
{
	string line, value = { "" };
	size_t ValueIndex, endIndex, paramIndex;
	stringstream request_stream(request);
	while (getline(request_stream, line)) {
		paramIndex = line.find(param);
		if (paramIndex != string::npos) {
			ValueIndex = line.find("=", paramIndex) + 1;
			endIndex = line.find(" ", ValueIndex);
			if (endIndex == string::npos) {
				endIndex = line.length();
			}
			value = line.substr(ValueIndex, endIndex - ValueIndex);
			break;
		}
	}
	return value;
}
