// DomainToIp.cpp : �� ���Ͽ��� 'main' �Լ��� ���Ե˴ϴ�. �ű⼭ ���α׷� ������ ���۵ǰ� ����˴ϴ�.
//

#include <iostream>

#include "DomainToIp.h"

int main()
{
    int err;
    WORD w_Version_Requested;
    WSADATA wsa_Data;

    w_Version_Requested = MAKEWORD(2, 2);
    err = WSAStartup(w_Version_Requested, &wsa_Data);
    if (err != 0)
    {
        printf_s("WSAStartup failed with error: %d \n", err);
        return 1;
    }

    IN_ADDR* p_Addr = (IN_ADDR*)malloc(sizeof(IN_ADDR));
    BOOL b_Err = DomainToIp(L"naver.com", p_Addr);

    printf_s("ip: %d : %d : %d : %d \n",
        p_Addr->S_un.S_un_b.s_b1,
        p_Addr->S_un.S_un_b.s_b2,
        p_Addr->S_un.S_un_b.s_b3,
        p_Addr->S_un.S_un_b.s_b4
    );

    std::cout << "Hello World!\n";
}

// ���α׷� ����: <Ctrl+F5> �Ǵ� [�����] > [��������� �ʰ� ����] �޴�
// ���α׷� �����: <F5> Ű �Ǵ� [�����] > [����� ����] �޴�

// ������ ���� ��: 
//   1. [�ַ�� Ž����] â�� ����Ͽ� ������ �߰�/�����մϴ�.
//   2. [�� Ž����] â�� ����Ͽ� �ҽ� ��� �����մϴ�.
//   3. [���] â�� ����Ͽ� ���� ��� �� ��Ÿ �޽����� Ȯ���մϴ�.
//   4. [���� ���] â�� ����Ͽ� ������ ���ϴ�.
//   5. [������Ʈ] > [�� �׸� �߰�]�� �̵��Ͽ� �� �ڵ� ������ ����ų�, [������Ʈ] > [���� �׸� �߰�]�� �̵��Ͽ� ���� �ڵ� ������ ������Ʈ�� �߰��մϴ�.
//   6. ���߿� �� ������Ʈ�� �ٽ� ������ [����] > [����] > [������Ʈ]�� �̵��ϰ� .sln ������ �����մϴ�.