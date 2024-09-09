#pragma once

#include <Windows.h>

struct st_SETTINGS
{

	//---------------------------------------------------
	// ���ε��� Listen socket �ּ�
	// 0 == ADDR_ANY
	char OpenIP[32] = { 0, };

	// ���ε��� ���� ���� �ּ�
	// ���� 0�̶��, ADDR_ANY
	// Setting File Key Name : listenAddress
	WCHAR	listenAddress[32] = { 0 };

	// ȣ��Ʈ ����Ʈ ������ ���� ��Ʈ
	// Setting File Key Name : listenPort
	USHORT	listenPort = 6000;

	// NODELAY ������
	// ���� 1�̶��, NoDelay�� Ŵ���μ� Nagle�� ����
	// Setting File Key Name : nodelay
	int		nodelay = false;

	// SNDBUF ������
	// Setting File Key Name : sendBufferSize 355,381 360,681
	int		sendBufferSize = 0;

	// ��α� ť ������ : accept ��� ���� ������
	// Setting File Key Name : backlogSize
	INT32	backlogSize = SOMAXCONN;

	// IOCP Total WorkerThread Count
	// ���� 0�̶��, CPU Count * 2
	// Setting File Key Name : workerThreadTotal
	INT32	workerThreadTotal = 0;

	// IOCP Running WorkerThread Count
	// ���� 0�̶��, CPU Count * 1
	// Setting File Key Name : workerThreadRunning
	INT32	workerThreadRunning = 0;

	// �������� ������ �ִ� ���� ����
	// Setting File Key Name : sessionCountMax
	INT32	sessionCountMax = 10000;

	// ���� Ÿ�Ӿƿ� ������ (ms)
	// Setting File Key Name : sessionTimeout
	INT32	sessionTimeout = 30000;
};
