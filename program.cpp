#include "program.h"
#define PORT_NUM	10200	// ��Ʈ��ȣ
#define BLOG_SIZE	5		// ��α� ������
#define MAX_MSG_LEN	256		// �ִ� �޽��� ����

int main()
{
	WSADATA wsadata;						
	WSAStartup(MAKEWORD(2, 2), &wsadata);	// ���� �ʱ�ȭ

	SOCKET sock = SetTCPServer(PORT_NUM, BLOG_SIZE);
	if (sock == -1)
	{
		perror("[ERROR] socket ���� ����\n");
	}
	else
	{
		EventLoop(sock);
	}
	WSACleanup();	// ���� ����
	return 0;
}

SOCKET SetTCPServer(short pnum, int blog)
{
	SOCKET sock; 
	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);	// ���� ����
	if (sock == -1) { return -1; };

	SOCKADDR_IN serverAddr = { 0 };				// ���� ����
	serverAddr.sin_family = AF_INET;			// ..
	serverAddr.sin_addr = GetDefaultMyIP();		// ..
	serverAddr.sin_port = htons(pnum);			// ..

	int nResult = 0;
	nResult = bind(sock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)); // ���� ���ε�
	if (nResult == -1) { return -1; }
	

	nResult = listen(sock, blog);			// ���� ���
	if (nResult == -1) { return -1; };

	return sock;
}

SOCKET sock_base[FD_SETSIZE];	// ���� ����ϴ� �迭 
HANDLE hev_base[FD_SETSIZE];	// �̺�Ʈ �ڵ鷯 �迭
int cnt;						// ����� Ŭ���̾�Ʈ ������ ����ϱ� ���� ����

HANDLE AddNetworkEvent(SOCKET sock, long network_event)
{
	HANDLE hev = WSACreateEvent();

	sock_base[cnt] = sock;
	hev_base[cnt]= hev;
	cnt++;

	WSAEventSelect(sock, hev, network_event); // �ش� ���Ͽ�(sock), �̷� ��Ʈ��ũ �̺�Ʈ(network_event)�� �߻��ϸ�, �� �̺�Ʈ(hev)�� "��ȣ ����"�� �ٲٶ�� �Լ�
	return hev;
}

void EventLoop(SOCKET sock)
{
	AddNetworkEvent(sock, FD_ACCEPT);	// ���� ����(sock_base[0]) ������ֱ� ���� ���
	while (true)
	{
		int index = WSAWaitForMultipleEvents(cnt, hev_base, false, INFINITE, false);
		WSANETWORKEVENTS network_events;
		WSAEnumNetworkEvents(sock_base[index], hev_base[index], &network_events);
		switch (network_events.lNetworkEvents)
		{
		case FD_ACCEPT: AcceptProc(index); break;
		case FD_READ: ReadProc(index); break;
		case FD_CLOSE: CloseProc(index); break;
		}
	}
	closesocket(sock);
}

void AcceptProc(int index)
{
	SOCKADDR_IN clientAddr = { 0 };
	int len = sizeof(clientAddr);

	SOCKET doSock = accept(sock_base[0], (SOCKADDR*)&clientAddr, &len);

	if (cnt == FD_SETSIZE)
	{
		printf("ä�� ���� �� ���� %s:%d ���� �������� ���Ͽ����ϴ�.\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
		closesocket(doSock);
		return;
	}
	AddNetworkEvent(doSock, FD_READ | FD_CLOSE);
	printf("[%s:%d] �����.\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
}

void ReadProc(int index)
{
	char msg[MAX_MSG_LEN];
	recv(sock_base[index], msg, MAX_MSG_LEN, 0);

	SOCKADDR_IN clientAddr = { 0 }; // �޽����� ���� Ŭ���̾�Ʈ�� Ȯ���ϱ� ���� ���� ����
	int len = sizeof(clientAddr);

	getpeername(sock_base[index], (SOCKADDR*)&clientAddr, &len);

	char smsg[MAX_MSG_LEN];
	sprintf(smsg, "[%s:%d]:%s", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port), msg);
	for (int i = 1; i < cnt; i++)
	{
		send(sock_base[i], smsg, MAX_MSG_LEN, 0);
	}
}

void CloseProc(int index)
{
	SOCKADDR_IN clientAddr = { 0 }; // ä�ù濡�� ���� Ŭ���̾�Ʈ�� Ȯ���ϱ� ���� ���� ����
	int len = sizeof(clientAddr);

	getpeername(sock_base[index], (SOCKADDR*)&clientAddr, &len);
	printf("[%s:%d] ���� �����Ͽ����ϴ�.\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
	
	closesocket(sock_base[index]);		// ä�ù��� ���� ���� �ݱ�
	WSACloseEvent(hev_base[index]);		// ä�ù��� ���� ������ �̺�Ʈ�ڵ鷯 �ݱ�
	cnt--;								// ����� Ŭ���̾�Ʈ ���� ���� ó��
	sock_base[index] = sock_base[cnt];	// �������� ����� Ŭ���̾�Ʈ�� ��� ������ Ŭ���̾�Ʈ ���Ͽ� �־��ֱ�
	hev_base[index] = hev_base[cnt];	// �������� ����� Ŭ���̾�Ʈ�� ��� ������ Ŭ���̾�Ʈ �̺�Ʈ�ڵ鷯�� �־��ֱ�
	
	char smsg[MAX_MSG_LEN];
	sprintf(smsg, "[%s:%d] ���� �����Ͽ����ϴ�.\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
	for (int i = 1; i < cnt; i++)
	{
		send(sock_base[i], smsg, MAX_MSG_LEN, 0);
	}
}
