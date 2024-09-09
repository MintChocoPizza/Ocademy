#pragma once



// Key: SessionID, Value: bool
// ��ǻ� ������ ��, �ٽ��ѹ� �˻��ؾ� �ؼ� �׳� ���� �ϴ����� �Ǵ��ϸ� �ȴ�. 
// �ٵ� ������ �� ��������� ������ ��ü�� ������ �� �ִ�. 
// �̰� ��� �ذ��ؾ� �ϴ°�?
// 
// 1. end()�� ���� �� ���� begin()�� �̴´�. 
// 2. list �ڷᱸ���� �ϳ� �� ���� ��� �ִ´�. 
// 3. �׳� �ȿ� �ִ°Ÿ� �����ϰ� �߰��� ���ͼ� ������ �ȵȰ��� ���� �����ӿ� �����Ѵ�.
//
// IO -> Disconnect -> Update ������ �ϸ� ���� ���� ���� �ֵ鿡 ���� ����ó���� �� �ʿ䰡 ��������.

bool DisconnectCheck(DWORD dwSessionID);
void enqueueForDeletion(DWORD dwSessionID);
void Disconnect(void);
void mpDelete(SerializeBuffer* pPacket, DWORD dwSessionID);
void InitDisconnectSessionIds(void);

