#pragma once



// Key: SessionID, Value: bool
// 사실상 삭제할 때, 다시한번 검색해야 해서 그냥 존재 하는지만 판단하면 된다. 
// 근데 삭제할 때 재귀적으로 삭제할 객체가 생성될 수 있다. 
// 이걸 어떻게 해결해야 하는가?
// 
// 1. end()가 나올 때 까지 begin()만 뽑는다. 
// 2. list 자료구조를 하나 더 만들어서 계속 넣는다. 
// 3. 그냥 안에 있는거만 삭제하고 추가로 들어와서 삭제가 안된것은 다음 프레임에 삭제한다.
//
// IO -> Disconnect -> Update 순으로 하면 굳이 연결 끊길 애들에 대한 예외처리를 할 필요가 없어진다.

bool DisconnectCheck(DWORD dwSessionID);
void enqueueForDeletion(DWORD dwSessionID);
void Disconnect(void);
void mpDelete(SerializeBuffer* pPacket, DWORD dwSessionID);
void InitDisconnectSessionIds(void);

