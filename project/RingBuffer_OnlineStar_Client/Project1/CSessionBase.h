#pragma once

class C_SessionBase
{
public:
	C_SessionBase();
	virtual ~C_SessionBase();

	virtual bool Update(void) = 0;

public:
	SOCKET Connect_Socket;
};
