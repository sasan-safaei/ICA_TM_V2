#pragma once
#ifndef myNet_H
#define myNet_H

#include <unistd.h>
#include <stdio.h>
#include <cstring>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <pthread.h>	
#include <dirent.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <iostream>
#include <fstream>
#include <fcntl.h>
#include <vector>

#define APP_MOD_LINUX
#define DEBUG_main true
class myNet
{
	#ifdef APP_MOD_LINUX
		pthread_t upThread1, upThread2;
		pthread_attr_t pattr1, pattr2;			
		//TCP ***********************************************************
		struct sockaddr_in servaddr = { 0 };//TCP Server
		int tcp_listening_P = socket(AF_INET, SOCK_STREAM, 0);
		int clientSocket;//TCP Listen port
		sockaddr_in client;//TCP
		socklen_t clientSize;//TCP
		//UDP ***********************************************************
		int sockfd = socket(AF_INET, SOCK_DGRAM, 0);//UDP receive
		int socksd = socket(AF_INET, SOCK_DGRAM, 0);//UDP Send	
	#else	
		HANDLE upThread1, upThread2;
		//TCP ***********************************************************
		SOCKET winTCPListenSocket = INVALID_SOCKET;
		SOCKET winTCPClientSocket = INVALID_SOCKET;
		//UDP ***********************************************************
		SOCKET winUDPListenSocket = INVALID_SOCKET;
		SOCKET winUDPSendSocket = INVALID_SOCKET;
	#endif

	int udpPort =	0;
	int tcpPort =	0;
	int Rxcounter=0; //TCP Rx Counter
	
	struct sockaddr_in sendaddr = { 0 };//UDP 
	sockaddr_in service;

	//TCP ***********************************************************
	void TcpListenPort() {
		static bool status = false;
		struct sockaddr_in hint = { 0 };
		#ifdef APP_MOD_LINUX
			char host[NI_MAXHOST];
			char svc[NI_MAXSERV];
		#endif		
		int byteRecv;
		if (status == false)
		{
		#ifdef APP_MOD_LINUX
			hint.sin_family = AF_INET;
			hint.sin_port = htons(tcpPort);
			inet_pton(AF_INET, "0,0,0,0", &hint.sin_addr);
			if (bind(tcp_listening_P, (sockaddr*)&hint, sizeof(hint)) == -1) {
				perror("(TCP)failed to bind to IP port");
				sleep(1);
				return;
			}
			if (listen(tcp_listening_P, SOMAXCONN) == -1) {
				perror("(TCP) can not listen");
				sleep(1);
				return;
			}
		#else
			WSADATA wsaData;
			int iResult;	
			struct addrinfo *result = NULL;
			struct addrinfo hints = {0};			
			
			
			// Initialize Winsock
			iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
			if (iResult != 0) { printf("WSAStartup failed with error: %d\n", iResult); return; 
			}
			ZeroMemory(&hints, sizeof(hints));
			hints.ai_family = AF_INET;
			hints.ai_socktype = SOCK_STREAM;
			hints.ai_protocol = IPPROTO_TCP;
			hints.ai_flags = AI_PASSIVE;
			//std::cout << "tcp port: " << std::to_string(tcpPort).c_str() << "\n";
			// Resolve the server address and port
			iResult = getaddrinfo(NULL, std::to_string(tcpPort).c_str(), &hints, &result);
			if (iResult != 0) { printf("getaddrinfo failed with error: %d\n", iResult); WSACleanup(); return; }
			// Create a SOCKET for the server to listen for client connections.
			winTCPListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
			if (winTCPListenSocket == INVALID_SOCKET) {
				printf("socket failed with error: %ld\n", WSAGetLastError());
				freeaddrinfo(result); WSACleanup(); return; }
			// Setup the TCP listening socket
			iResult = bind(winTCPListenSocket, result->ai_addr, (int)result->ai_addrlen);
			if (iResult == SOCKET_ERROR) {
				printf("bind failed with error: %d\n", WSAGetLastError());
				freeaddrinfo(result); closesocket(winTCPListenSocket); WSACleanup(); return; }
			freeaddrinfo(result);
			iResult = listen(winTCPListenSocket, SOMAXCONN);
			if (iResult == SOCKET_ERROR) {
				printf("listen failed with error: %d\n", WSAGetLastError());
				closesocket(winTCPListenSocket); WSACleanup(); return; }
		#endif
			status = true;
		}
		printf("TCP port, wait for new Connection on Port %i ...\n", tcpPort);

		#ifdef APP_MOD_LINUX
			clientSize = sizeof(client);
			clientSocket = accept(tcp_listening_P, (sockaddr*)&client, &clientSize);
			if (clientSocket == -1)
			{
				perror("Problem with client connection");
				printf("-4\n");
				return; // -4;
			}
			//close(tcp_listening_P);
			memset(host, 0, NI_MAXHOST);
			memset(svc, 0, NI_MAXSERV);
			int result = getnameinfo((sockaddr*)&client, sizeof(client), host, NI_MAXHOST, svc, NI_MAXSERV, 0);
			if (result) {
				std::cout << host << " Connected on " << svc << std::endl;
			}
			else {
				inet_ntop(AF_INET, &client.sin_addr, host, NI_MAXHOST);
				std::cout << host << " connected on " << ntohs(client.sin_port) << std::endl;
			}		
		#else
			// Accept a client socket
			winTCPClientSocket = accept(winTCPListenSocket, NULL,NULL);
			if (winTCPClientSocket == INVALID_SOCKET) {
				printf("accept failed with error: %d\n", WSAGetLastError());
				closesocket(winTCPListenSocket);
				WSACleanup(); 
				return; 
			}		
			printf("Connected\n");
		#endif
		

		memset(tcp_Buff, 0, sizeof(tcp_Buff));
		while (true)
		{
			
			#ifdef APP_MOD_LINUX
				byteRecv = recv(clientSocket, tcp_Buff, sizeof(tcp_Buff), 0);
			#else
				byteRecv = recv(winTCPClientSocket, tcp_Buff, sizeof(tcp_Buff), 0);			
			#endif		
			
			if (byteRecv == -1)
			{
				perror("There was a connection issue");			
				break;
			}			
			if (byteRecv == 0) {
				std::cout << "The client disconnected\n" << std::endl;
				break;
			}
			if (DEBUG_main) std::cout << Rxcounter++ << "\nReceived: " << std::string(tcp_Buff, 0, byteRecv) << std::endl;
			getNewTCP = true;
			int cnt = 0;
			while (getNewTCP == true)
			{
				usleep(10000);
				cnt++;				
			}
			if(DEBUG_main) printf(" wait to response: %dmsec \n", cnt*10);
			memset(tcp_Buff, 0, sizeof(tcp_Buff));
			
		}

		// No longer need server socket
		#ifdef APP_MOD_LINUX
			close(clientSocket);
		#else
			//closesocket(winTCPListenSocket);
			//WSACleanup();
			//status = false;
		#endif
		return;
	}
	static void* thread_tcpPort(void* thisPtr) {
		while (1)
		{
			((myNet*)thisPtr)->TcpListenPort();				
		}
		return NULL;
	}	
	//UDP ***********************************************************
	void UdpPortLoop() {
		getNewUDP = false;
		int n = -1;
		printf("UDP Listening on Port %i ...\n", udpPort);
		#ifdef APP_MOD_LINUX			
			while (true) {
				//socklen_t len = 0;
				socklen_t len = sizeof(struct sockaddr_in); // Length of the address structure
				n = recvfrom(sockfd, (char *)udp_Buff, sizeof(udp_Buff), MSG_WAITALL, (struct sockaddr *) &sendaddr, &len);
				if (DEBUG_main){
					std::cout << Rxcounter++ << "\nUDP Packet\n";
				    // Print sender's IP address
					char sender_ip[INET_ADDRSTRLEN]; // Buffer to hold the IP address
					inet_ntop(AF_INET, &sendaddr.sin_addr, sender_ip, sizeof(sender_ip)); // Convert IP address to string format
					std::cout << "Received UDP packet from: " << sender_ip << ":" << ntohs(sendaddr.sin_port) << std::endl;
				} 

				getNewUDP = true;
				udp_Buff[n - 1] = 0;
				while (getNewUDP == true)
					usleep(100000);
			}
		#else
			sockaddr_in SenderAddr;		
			int SenderAddrSize = sizeof(SenderAddr);			
			while (true) {
				n = recvfrom(winUDPListenSocket, (char *)udp_Buff, sizeof(udp_Buff), 0, (SOCKADDR *)& SenderAddr, &SenderAddrSize);
				if (n >= 0)
				{
					if (DEBUG_main) std::cout << Rxcounter++ << "\nUDP Packet " << n << "\n";
					getNewUDP = true;
					udp_Buff[n - 1] = 0;
					while (getNewUDP == true)
						msecSleep(100);
				}
			}
		#endif
	}
	static void* thread_udpPort(void* thisPtr) {
		((myNet*)thisPtr)->UdpPortLoop();
		return NULL;
	}

public:// ***********************************************************
	bool getNewUDP=false;
	bool getNewTCP=false;
	char  udp_Buff[1024]={0};
	char  tcp_Buff[2048]={0};
	
	void RemoveEnter(char* _txTxt){
	int cnt=0;
	char __lastChr;
	while(cnt++<256 & *_txTxt!=0)
	{
		if(*_txTxt==0x0a)
			if(__lastChr==0x0d)
			{
				_txTxt--;
				*_txTxt=0;
				if(DEBUG_main) std::cout<<"Findd Enter Char!!!!!!!!!!!!!!!!!!!!!1\n";
				return;
			}
			__lastChr=*_txTxt++;			
	}
}

	//TCP ***********************************************************
		 
	void tcpReplay(char* data) {
		#ifdef APP_MOD_LINUX			
			send(clientSocket, (char*)data, strlen(data), 0);
		#else		
			send(winTCPClientSocket, (char*)data, strlen(data), 0);
		#endif
	}
	//UDP ***********************************************************
	void UdpReplay(char* data) {
		
		#ifdef APP_MOD_LINUX
			if (DEBUG_main){
				char sender_ip[INET_ADDRSTRLEN]; // Buffer to hold the IP address
				inet_ntop(AF_INET, &sendaddr.sin_addr, sender_ip, sizeof(sender_ip)); // Convert IP address to string format
				std::cout << "UDP packet send to: " << sender_ip << ":" << ntohs(sendaddr.sin_port) << std::endl;
				std::cout<< "UDP reply Packet: "<<data<<" end.";
			}
			if (sendto(socksd, data, strlen(data), 0, (struct sockaddr *)&sendaddr, sizeof(sendaddr)) < 0)
				perror("failed to UDP packet send\n");
			
		#else		
			if (sendto(winUDPSendSocket, data, strlen(data), 0, (struct sockaddr *)&sendaddr, sizeof(sendaddr)) < 0)
				perror("failed to UDP packet send!\n");			
		#endif
	}
	// TCP-UDP ******************************************************
	int myNetConfig(int _udpPort, int _tcpPort) {
		udpPort = _udpPort;
		tcpPort = _tcpPort;
		#ifdef APP_MOD_LINUX
			// Open fdp server port to get commands		
			if (sockfd == -1) {
				perror("failed to create socket!\n");
				return false;
			}
			servaddr.sin_family = AF_INET;
			servaddr.sin_port = htons(udpPort);
			servaddr.sin_addr.s_addr = INADDR_ANY;
			int rc = bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr));
			if (rc == -1) {
				perror("failed to bind\n");
				close(sockfd);
				return false;
			}
			sendaddr.sin_family = AF_INET;
			sendaddr.sin_port = htons(udpPort + 1);
			sendaddr.sin_addr.s_addr = INADDR_ANY;
		#else
			sendaddr.sin_family = AF_INET;
			sendaddr.sin_port = htons(udpPort + 1);
			sendaddr.sin_addr.s_addr = inet_addr("127.0.0.1"); //INADDR_ANY;
			// Declare some variables
			WSADATA wsaData;
			int iResult = 0;            // used to return function results
			// The socket address to be passed to bind
			// Initialize Winsock
			iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
			if (iResult != NO_ERROR) { wprintf(L"Error at WSAStartup()\n"); return false; }		
			// Create a SOCKET for listening for 
			// incoming connection requests
			winUDPListenSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
			if (winUDPListenSocket == INVALID_SOCKET) {
				wprintf(L"socket function failed with error: %u\n", WSAGetLastError());
				WSACleanup(); return false; }
			// Create a SOCKET for listening for 
			// incoming connection requests
			winUDPSendSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
			if (winUDPSendSocket == INVALID_SOCKET) {
				wprintf(L"socket2 function failed with error: %u\n", WSAGetLastError());
				WSACleanup(); return false; }
			// The sockaddr_in structure specifies the address family,
			// IP address, and port for the socket that is being bound.
			service.sin_family = AF_INET;
			service.sin_addr.s_addr = INADDR_ANY;// inet_addr("127.0.0.1");
			service.sin_port = htons(_udpPort);
			// Bind the socket.
			iResult = bind(winUDPListenSocket, (SOCKADDR *)&service, sizeof(service));
			if (iResult == SOCKET_ERROR) {
				wprintf(L"bind failed with error %u\n", WSAGetLastError());
				closesocket(winUDPListenSocket); WSACleanup(); return false; }
		#endif
		return true;
	}
	void netRun() {
		#ifdef APP_MOD_LINUX
			pthread_attr_init(&pattr1);
			pthread_create(&upThread1, &pattr1, &myNet::thread_udpPort, this);
			pthread_attr_init(&pattr2);
			pthread_create(&upThread2, &pattr2, &myNet::thread_tcpPort, this);
		#else
			upThread1 = (HANDLE)_beginthread((_beginthread_proc_type)thread_udpPort, 0, this);		
			upThread2 = (HANDLE)_beginthread((_beginthread_proc_type)thread_tcpPort, 0, this);
		#endif		
	}
	void close_ports(){
		try{close(sockfd);}catch (char const* e) { std::cout << "Net.Error: " << e << "\n";}
		try{close(socksd);}catch (char const* e) { std::cout << "Net.Error: " << e << "\n";}
		try{
			std::cout<<"Close TCP Socket\n";
			close(clientSocket);close(tcp_listening_P);
			std::cout<<"Close TCP Socket End\n";
			}catch (char const* e) { std::cout << "Net.Error: " << e << "\n";}
		
		
	}
};
#endif
