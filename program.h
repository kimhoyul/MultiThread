#pragma once
#include "common.h"

SOCKET SetTCPServer(short pnum, int blog);	//��� ���� ����

void EventLoop(SOCKET sock);				//�̺�Ʈ ó��

HANDLE AddNetworkEvent(SOCKET sock, long net_event); //

void AcceptProc(int index);

void ReadProc(int index);

void CloseProc(int index);