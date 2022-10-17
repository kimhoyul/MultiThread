#pragma once
#include "common.h"

SOCKET SetTCPServer(short pnum, int blog);	//대기 소켓 설정

void EventLoop(SOCKET sock);				//이벤트 처리

HANDLE AddNetworkEvent(SOCKET sock, long net_event); //

void AcceptProc(int index);

void ReadProc(int index);

void CloseProc(int index);