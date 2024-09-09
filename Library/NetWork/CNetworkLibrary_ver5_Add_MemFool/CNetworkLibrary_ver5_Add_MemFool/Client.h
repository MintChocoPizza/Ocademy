#pragma once

#include "C_Ring_Buffer.h"
#include "Packet.h"

struct st_CLIENT
{
	DWORD64 sessionID;

	DWORD IP;

	USHORT PORT;

	WCHAR clientAddressString[SESSION_ADDRESS_WCHAR_LENGTH];

	//SerializeBuffer packet;

	CRITICAL_SECTION cs_Client;
};
