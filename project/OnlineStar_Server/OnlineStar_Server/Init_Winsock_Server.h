
#ifndef __Init_Winsock_Server_H__
#define __Init_Winsock_Server_H__

//--------------------------------------------------------------------
// 소켓의 포인터를 전달한다. 
// 
// 내부에서 SOCKET로 형 변환하여 사용한다.
//--------------------------------------------------------------------
bool init_Listen_Socket(void* Listen_Socket, void* wsa_Data);



#endif // !__Init_Winsock_Server_H__
