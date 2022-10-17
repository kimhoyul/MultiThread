#include "common.h"
#include "program.h"
#define PORT_NUM	10200		
#define MAX_MSG_LEN	256				
#define SERVER_IP	"192.168.224.1"	

int main()
{
	WSADATA wsadata;
	WSAStartup(MAKEWORD(2, 2), &wsadata);

	SOCKET sock;
	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);	// 소켓 생성
	if (sock == -1) { return -1; };
	
	SOCKADDR_IN servAddr = { 0 };
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = inet_addr(SERVER_IP);
	servAddr.sin_port = htons(PORT_NUM);

	int nResult = 0;
	nResult = connect(sock, (struct sockaddr*)&servAddr, sizeof(servAddr)); // 서버 연결시도
	if (nResult == -1) { return -1; };

	_beginthread(RecvThreadPoint, 0, (void*)sock);

	char msg[MAX_MSG_LEN] = "";
	while (true)
	{
		gets_s(msg, MAX_MSG_LEN);
		send(sock, msg, sizeof(msg), 0);
		if (strcmp(msg, "exit") == 0)
		{
			break;
		}
	}
	closesocket(sock);
	WSACleanup();
	return 0;
}

void RecvThreadPoint(void* param)
{
	SOCKET sock = (SOCKET)param;
	char msg[MAX_MSG_LEN];

	while (recv(sock,msg,MAX_MSG_LEN,0))
	{
		printf("%s\n", msg);
	}
	closesocket(sock);
}
