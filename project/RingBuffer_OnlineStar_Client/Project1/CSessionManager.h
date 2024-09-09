#pragma once

class C_SessionManager
{
private:
	static C_SessionManager _C_SessionManager;

private:
	C_SessionManager();
	~C_SessionManager();

public:
	static C_SessionManager* GetInstance(void);
	void SessionUpdate(void);
	
private:
	WSADATA wsa_Data;
};
