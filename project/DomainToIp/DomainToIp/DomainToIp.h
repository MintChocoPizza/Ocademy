#pragma comment(lib, "Ws2_32.lib")

#include <WS2tcpip.h>
#include <WinSock2.h>
#include <Windows.h>

#include <stdio.h>



#define RET_IP_RAND

#ifdef RET_IP_RAND
#include <time.h>
#include <stdlib.h>
#endif // RET_IP_RAND





inline BOOL DomainToIp(const WCHAR *sz_Domain, IN_ADDR *p_Addr)
{
	ADDRINFOW		*p_Addr_Info;
	ADDRINFOW		*p_Addr_Info_Temp;
	SOCKADDR_IN		*p_Sock_Addr;

	int i_Count = 0;

	if (GetAddrInfo(sz_Domain, L"0", NULL, &p_Addr_Info) != 0)
	{
		DWORD err = GetLastError();

		printf_s("func: DomainToIp    err: %d \n", err);
		return FALSE;
	}

	// 도메인에 연결된 IP 전부 출력
	for (p_Addr_Info_Temp = p_Addr_Info; p_Addr_Info_Temp != NULL; p_Addr_Info_Temp = p_Addr_Info_Temp->ai_next)
	{
		p_Sock_Addr = (SOCKADDR_IN *)p_Addr_Info_Temp->ai_addr;

		printf_s("ip: %d : %d : %d : %d \n", 
			p_Sock_Addr->sin_addr.S_un.S_un_b.s_b1,
			p_Sock_Addr->sin_addr.S_un.S_un_b.s_b2,
			p_Sock_Addr->sin_addr.S_un.S_un_b.s_b3,
			p_Sock_Addr->sin_addr.S_un.S_un_b.s_b4
		);

		// IP 갯수 확인
		++i_Count;
	}


#ifdef RET_IP_RAND
	srand(time(NULL));
	int i_Rand = rand() % i_Count;

	printf_s("i_Rand: %d \n", i_Rand);
	
	i_Count = 0;
	for (p_Addr_Info_Temp = p_Addr_Info; p_Addr_Info_Temp != NULL; p_Addr_Info_Temp = p_Addr_Info_Temp->ai_next)
	{
		if (i_Count == i_Rand)
		{
			*p_Addr = ((SOCKADDR_IN*)p_Addr_Info_Temp->ai_addr)->sin_addr;
			break;
		}
		++i_Count;
	}

#else
	*p_Addr = ((SOCKADDR_IN*)p_Addr_Info->ai_addr)->sin_addr;
	//p_Sock_Addr = (SOCKADDR_IN*)p_Addr_Info->ai_addr;
	//*p_Addr = p_Sock_Addr->sin_addr;
#endif // RET_IP_RAND



	FreeAddrInfo(p_Addr_Info);

	return TRUE;
}