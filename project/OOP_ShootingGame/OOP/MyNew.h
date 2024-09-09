#ifndef __MYNEW_H__
#define __MYNEW_H__


/////////////////////////////////////////////////////////////////////////////////////
// 
// LEAK: ���α׷��� ���� �� ���� �������� ���� �޸�
// 
// ARRAY: �迭�� �Ҵ��� �޸𸮸� delete[]�� ���� �ʰ� �׳� delete�� �� �޸� 
// 
// NOALLOC: �̻��� �߸��� �ּҸ� �����Ϸ� �Ѵ�.
// 
// Ŭ���� �迭 �Ҵ� -> �Ϲ� Ŭ���� ����
// Ŭ���� �迭�� ��� ARRAY�� �ȳ�Ÿ���� 
// NOALLOC�� LEAK�� ���ÿ� ��Ÿ����. 
// �����Ҵ� �޸� �ּ� �տ� Ŭ���� �迭�� ũ�⸦ �����ϰ�
// �����δ� �� �޸𸮸� �Ҵ� ����Ѵ�.
// => �׷��Ƿ� �߸��� �޸𸮸� �����Ϸ� ����, ���� �޸𸮴� �������� ���Ͽ� ���ÿ� ����
// 
// �Ϲ� Ŭ���� �Ҵ� -> �迭 Ŭ���� ����
// �Ҹ��ڿ��� ���� ������ ���� ��� 
// Ŭ���� �迭�� �ּҸ� �����ϴ� �ּ� �� ������ Ȯ���ϰ� 
// ������ ��ġ�� ��� ������ ��ġ�� ã�� �� ��� ��� ���� ������ �𸥴�. 
// 
// 
// 
/////////////////////////////////////////////////////////////////////////////////////


struct stALLOCINFO
{
	void* ptr;
	size_t size;
	char fileName[128];
	int line;
	bool arr;
	stALLOCINFO* pPreviousNode;
	stALLOCINFO* pNextNode;
};



class cMYNEW
{
private:
	//stALLOCINFO* pHead;
	//stALLOCINFO* pTail;
	stALLOCINFO pHead;
	stALLOCINFO pTail;

	///////////////////////////////////////////////////////////////////////////////////////
	// ���� ��� �α�
	// 
	///////////////////////////////////////////////////////////////////////////////////////
public: char log[256];
public:	bool setting;

public:
	cMYNEW();
	~cMYNEW();

	///////////////////////////////////////////////////////////////////////////////////////
	// ���Ͽ� ������ ���� �Լ�
	// 
	///////////////////////////////////////////////////////////////////////////////////////
	void saveFile(const char* log, void* ptr, stALLOCINFO* stData);



	///////////////////////////////////////////////////////////////////////////////////////
	// ������ ���� �Լ���
	// 
	///////////////////////////////////////////////////////////////////////////////////////
	bool setAllocInfo(void* ptr, size_t size, const char* file, int line, bool arr);
	stALLOCINFO* findAndAllocInfo(void* ptr);
	void deleteAllocInfo(stALLOCINFO* ptr);


};


void setLog(const char* file, int line);
char* getLog(void);





//------------------------------------------------------------------------
// new ������ �����ε� �Լ�
// 
// 	2��° ���� file�� const�� �ƴ϶�� ������ �߻���
//------------------------------------------------------------------------
void* operator new(size_t size, const char* file, int line);
void operator delete(void* ptr);
void operator delete(void* p, const char* File, int lIne);


void* operator new[](size_t size, const char* file, int line);
void operator delete[](void* ptr);
void operator delete[](void* p, const char* File, int lIne);




//------------------------------------------------------------------------
// delete ������ �����ε� �Լ�
// 
//------------------------------------------------------------------------


#define  new    new(__FILE__, __LINE__)


#endif // !__MYNEW_H__
