#pragma comment(lib, "Ws2_32.lib")
#include "ServerFuncs.h"

void main()
{
	struct SocketState sockets[MAX_SOCKETS] = { 0 };
	int socketsCount = 0;
	time_t currentTime;
	struct timeval timeOut;
	timeOut.tv_sec = 120;
	timeOut.tv_usec = 0;
	WSAData wsaData;
	if (NO_ERROR != WSAStartup(MAKEWORD(2, 2), &wsaData))
	{
		cout << "HTTP Server: Error at WSAStartup()\n";
		return;
	}
	SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == listenSocket)
	{
		cout << "HTTP Server: Error at socket(): " << WSAGetLastError() << endl;
		WSACleanup();
		return;
	}
	sockaddr_in serverService;
	serverService.sin_family = AF_INET;
	serverService.sin_addr.s_addr = INADDR_ANY;
	serverService.sin_port = htons(PORT);
	if (SOCKET_ERROR == bind(listenSocket, (SOCKADDR*)&serverService, sizeof(serverService)))
	{
		cout << "HTTP Server: Error at bind(): " << WSAGetLastError() << endl;
		closesocket(listenSocket);
		WSACleanup();
		return;
	}
	if (SOCKET_ERROR == listen(listenSocket, 50)){
		cout << "HTTP Server: Error at listen(): " << WSAGetLastError() << endl;
		closesocket(listenSocket);
		WSACleanup();
		return;
	}
	addSocket(listenSocket, LISTEN, sockets, socketsCount);
	cout << "Waiting for client connections..." << endl;
	while (true)
	{
		for (int i = 1; i < MAX_SOCKETS; i++)
		{
			currentTime = time(0);
			if ((currentTime - sockets[i].prevActivity > 120) && (sockets[i].prevActivity != 0))
			{
				removeSocket(i, sockets, socketsCount);
			}
		}

		fd_set waitRecv;
		FD_ZERO(&waitRecv);
		for (int i = 0; i < MAX_SOCKETS; i++)
		{
			if ((sockets[i].recv == LISTEN) || (sockets[i].recv == RECEIVE))
				FD_SET(sockets[i].id, &waitRecv);
		}
		fd_set waitSend;
		FD_ZERO(&waitSend);
		for (int i = 0; i < MAX_SOCKETS; i++)
		{
			if (sockets[i].send == SEND)
				FD_SET(sockets[i].id, &waitSend);
		}
		int nfd;
		nfd = select(0, &waitRecv, &waitSend, NULL, &timeOut);
		if (nfd == SOCKET_ERROR)
		{
			cout << "HTTP Server: Error at select(): " << WSAGetLastError() << endl;
			WSACleanup();
			return;
		}

		for (int socket_index = 0; socket_index < MAX_SOCKETS && nfd > 0; socket_index++)
		{
			if (FD_ISSET(sockets[socket_index].id, &waitRecv))
			{
				nfd--;
				switch (sockets[socket_index].recv)
				{
				case LISTEN:
					acceptConnection(socket_index, sockets, socketsCount);
					break;

				case RECEIVE:
					rcvMessage(socket_index, sockets, socketsCount);
					break;
				}
			}
		}

		for (int i = 0; i < MAX_SOCKETS && nfd > 0; i++)
		{
			if (FD_ISSET(sockets[i].id, &waitSend))
			{
				nfd--;
				if (sockets[i].send == SEND)
				{
					if (!sendMessage(i, sockets))
						continue;
				}
			}
		}
	}
	cout << "HTTP Server: Closing Connection.\n";
	closesocket(listenSocket);
	WSACleanup();
}