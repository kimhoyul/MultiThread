#include "program.h"
#define PORT_NUM	10200	// 포트번호
#define BLOG_SIZE	5		// 백로그 사이즈
#define MAX_MSG_LEN	256		// 최대 메시지 길이

int main()
{
	WSADATA wsadata;						
	WSAStartup(MAKEWORD(2, 2), &wsadata);	// 윈속 초기화

	SOCKET sock = SetTCPServer(PORT_NUM, BLOG_SIZE);
	if (sock == -1)
	{
		perror("[ERROR] socket 생성 실패\n");
	}
	else
	{
		EventLoop(sock);
	}
	WSACleanup();	// 윈속 해제
	return 0;
}

SOCKET SetTCPServer(short pnum, int blog)
{
	SOCKET sock; 
	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);	// 소켓 생성
	if (sock == -1) { return -1; };

	SOCKADDR_IN serverAddr = { 0 };				// 소켓 설정
	serverAddr.sin_family = AF_INET;			// ..
	serverAddr.sin_addr = GetDefaultMyIP();		// ..
	serverAddr.sin_port = htons(pnum);			// ..

	int nResult = 0;
	nResult = bind(sock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)); // 소켓 바인드
	if (nResult == -1) { return -1; }
	

	nResult = listen(sock, blog);			// 연결 대기
	if (nResult == -1) { return -1; };

	return sock;
}

SOCKET sock_base[FD_SETSIZE];	// 소켓 기억하는 배열 
HANDLE hev_base[FD_SETSIZE];	// 이벤트 핸들러 배열
int cnt;						// 연결된 클라이언트 개수를 기억하기 위한 변수

HANDLE AddNetworkEvent(SOCKET sock, long network_event)
{
	HANDLE hev = WSACreateEvent();

	sock_base[cnt] = sock;
	hev_base[cnt]= hev;
	cnt++;

	WSAEventSelect(sock, hev, network_event); // 해당 소켓에(sock), 이런 네트워크 이벤트(network_event)가 발생하면, 이 이벤트(hev)를 "신호 상태"로 바꾸라는 함수
	return hev;
}

void EventLoop(SOCKET sock)
{
	AddNetworkEvent(sock, FD_ACCEPT);	// 리슨 소켓(sock_base[0]) 만들어주기 위해 사용
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
		printf("채팅 방이 꽉 차서 %s:%d 님이 입장하지 못하였습니다.\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
		closesocket(doSock);
		return;
	}
	AddNetworkEvent(doSock, FD_READ | FD_CLOSE);
	printf("[%s:%d] 연결됨.\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
}

void ReadProc(int index)
{
	char msg[MAX_MSG_LEN];
	recv(sock_base[index], msg, MAX_MSG_LEN, 0);

	SOCKADDR_IN clientAddr = { 0 }; // 메시지를 보낸 클라이언트를 확인하기 위한 변수 생성
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
	SOCKADDR_IN clientAddr = { 0 }; // 채팅방에서 나간 클라이언트를 확인하기 위한 변수 생성
	int len = sizeof(clientAddr);

	getpeername(sock_base[index], (SOCKADDR*)&clientAddr, &len);
	printf("[%s:%d] 님이 퇴장하였습니다.\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
	
	closesocket(sock_base[index]);		// 채팅방을 나간 소켓 닫기
	WSACloseEvent(hev_base[index]);		// 채팅방을 나간 소켓의 이벤트핸들러 닫기
	cnt--;								// 연결된 클라이언트 개수 감소 처리
	sock_base[index] = sock_base[cnt];	// 마지막에 연결된 클라이언트를 방금 삭제한 클라이언트 소켓에 넣어주기
	hev_base[index] = hev_base[cnt];	// 마지막에 연결된 클라이언트를 방금 삭제한 클라이언트 이벤트핸들러에 넣어주기
	
	char smsg[MAX_MSG_LEN];
	sprintf(smsg, "[%s:%d] 님이 퇴장하였습니다.\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
	for (int i = 1; i < cnt; i++)
	{
		send(sock_base[i], smsg, MAX_MSG_LEN, 0);
	}
}
