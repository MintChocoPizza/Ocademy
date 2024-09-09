#include <malloc.h>
#include <stdio.h>
#include <time.h>

#include "MyNew.h"

#undef new
#undef setLog

cMYNEW cMyNew;

/////////////////////////////////////////////////////////////////////////////////////////////
// Ŭ����
// 
/////////////////////////////////////////////////////////////////////////////////////////////
cMYNEW::cMYNEW()
{
	//pHead = (stALLOCINFO*)malloc(sizeof(stALLOCINFO));
	//pTail = (stALLOCINFO*)malloc(sizeof(stALLOCINFO));

	//pHead->pNextNode = pTail;
	//pTail->pPreviousNode = pHead;

	pHead.pPreviousNode = NULL;
	pHead.pNextNode = &pTail;

	pTail.pPreviousNode = &pHead;
	pTail.pNextNode = NULL;

	///////////////////////////////////////////////////////////////////////////////////////
	// ���� ��� �α�
	// 
	///////////////////////////////////////////////////////////////////////////////////////
	setting = false;
	log[0] = NULL;
}

cMYNEW::~cMYNEW()
{
	//------------------------------------------------------------------------
	// ���Ͽ� �����Ѵ�. 
	// 
	//------------------------------------------------------------------------
	//stALLOCINFO* pCur;

	//for (pCur = pHead->pNextNode; pCur != pTail; pCur = pCur->pNextNode)
	//{
	//	saveFile("LEAK", pCur->ptr, pCur);
	//}

	stALLOCINFO *pCur;
	for (pCur = pHead.pNextNode; pCur != &pTail; pCur = pCur->pNextNode)
	{
		saveFile("LEAK", pCur->ptr, pCur);
	}
}




///////////////////////////////////////////////////////////////////////////////////////
// ���Ͽ� ������ ���� �Լ�
// 
///////////////////////////////////////////////////////////////////////////////////////
void cMYNEW::saveFile(const char* log, void* ptr, stALLOCINFO* stData)
{
	FILE* pFile;
	errno_t err;
	struct tm newtime;
	__time64_t long_time;
	char timebuf[26];


	_time64(&long_time);
	err = _localtime64_s(&newtime, &long_time);
	if (err)
	{
		printf("Invalid argument to _localtime64_s.");
		return;
	}
	sprintf_s(timebuf, "Alloc_%d%d%d_%d%d%d.txt", newtime.tm_year, newtime.tm_mon, newtime.tm_mday, newtime.tm_hour, newtime.tm_min, newtime.tm_sec);


	err = fopen_s(&pFile, timebuf, "a");
	if (err != NULL)
	{
		printf("Invalid argument to fopen_s. ");
		return;
	}


	if (stData == NULL)
	{
		fprintf_s(pFile, "%s [%p] \n", log, ptr);
	}
	else
	{
		// �α� [�ּ�] [������ũ��] ��� ����
		fprintf_s(pFile, "%s [%p] [%10zd] %s : %d \n", log, ptr, stData->size, stData->fileName, stData->line);
	}

	fclose(pFile);
}







///////////////////////////////////////////////////////////////////////////////////////
// ������ ���� �Լ���
// 
///////////////////////////////////////////////////////////////////////////////////////
bool cMYNEW::setAllocInfo(void* ptr, size_t size, const char* file, int line, bool arr)
{
	int iCnt = 0;
	stALLOCINFO* stNewData = (stALLOCINFO*)malloc(sizeof(stALLOCINFO));

	if (stNewData == NULL)
		return false;

	// �Ҵ�� �޸� ���� ����, �ʱ�ȭ
	(*stNewData).ptr = ptr;
	stNewData->size = size;
	while (*file != '\0')
	{
		stNewData->fileName[iCnt] = *file;
		++file;
		++iCnt;
	}
	stNewData->fileName[iCnt] = '\0';
	stNewData->line = line;
	stNewData->pNextNode = NULL;
	stNewData->pPreviousNode = NULL;
	stNewData->arr = arr;

	//------------------------------------------------------------------------
	// linked list pTail�� �տ� newData�� �����Ѵ�.
	// 
	//------------------------------------------------------------------------
	// 1. newData�� Tail�� �� ��带 �����Ѵ�.
	stNewData->pPreviousNode = pTail.pPreviousNode;
	// 2. Tail�� �� ���� newData�� �����Ѵ�. 
	pTail.pPreviousNode->pNextNode = stNewData;
	// 3. newData�� Tail�� �����Ѵ�. 
	stNewData->pNextNode = &pTail;
	// 4. Tail�� newData�� �����Ѵ�. 
	pTail.pPreviousNode = stNewData;

	return true;
}

stALLOCINFO* cMYNEW::findAndAllocInfo(void* ptr)
{
	stALLOCINFO* stCurrentNode;

	for (stCurrentNode = pHead.pNextNode; stCurrentNode != &pTail; stCurrentNode = stCurrentNode->pNextNode)
	{
		if (stCurrentNode->ptr == ptr)
			return stCurrentNode;
	}


	return NULL;
}

void cMYNEW::deleteAllocInfo(stALLOCINFO* ptr)
{
	stALLOCINFO* pNextNode = ptr->pNextNode;
	stALLOCINFO* pPreviousNode = ptr->pPreviousNode;


	pNextNode->pPreviousNode = pPreviousNode;
	pPreviousNode->pNextNode = pNextNode;


	free(ptr);
}












/////////////////////////////////////////////////////////////////////////////////////////////
// �����Լ� 
// 
// ������ �����ε�
// 
/////////////////////////////////////////////////////////////////////////////////////////////

//------------------------------------------------------------------------
// new delete ������ �����ε� �Լ�
// 
// 	2��° ���� file�� const�� �ƴ϶�� ������ �߻���
//------------------------------------------------------------------------
void* operator new(size_t size, const char* file, int line)
{
	// �⺻ new �����ڷ� �޸� �Ҵ�
	void* ptr = malloc(size);
	//void* ptr = cMyNew.operator new(size);

	// ���� �Ҵ� ������ �����Ѵ�.
	bool check = cMyNew.setAllocInfo(ptr, size, file, line, false);

	// �޸𸮰� ����� �Ҵ� �Ǿ����� ���� üũ
	if (ptr == NULL || check == NULL)
		return NULL;

	// �Ҵ�� �޸� �ּ� ��ȯ
	return ptr;
}
void operator delete(void* ptr)
{
	// list���� �ش� �ּҰ� �ִ��� ã�ƺ���.
	stALLOCINFO* dataPtr = cMyNew.findAndAllocInfo(ptr);


	// �迭 delete�� �߾�� �ϴ� ���
	if (dataPtr != NULL && dataPtr->arr == true)
	{
		cMyNew.saveFile("ARRAY", ptr, dataPtr);
		return;
	}

	if (dataPtr != NULL)
	{
		// list���� �ּ��� �����͸� �����Ѵ�.
		cMyNew.deleteAllocInfo(dataPtr);

		// �⺻ delete �����ڷ� �޸� ����
		free(ptr);
		return;
	}

	// ���� dataPtr�� NULL�̸� �߸��� �ּҸ� �����Ϸ� �Ѵٸ� ���Ͽ� NoAlloc�� �ۼ��Ѵ�.
	if (dataPtr == NULL)
	{
		if (cMyNew.setting)
		{
			cMyNew.saveFile(getLog(), ptr, NULL);
		} 
		cMyNew.saveFile("NOALLOC", ptr, NULL);
		return;
	}
}

void operator delete(void* p, const char* File, int lIne)
{

}





























void operator delete[](void* ptr)
{
	// list���� �ش� �ּҰ� �ִ��� ã�ƺ���.
	stALLOCINFO* dataPtr = cMyNew.findAndAllocInfo((stALLOCINFO*)ptr);

	// �迭 delete�� �߾�� �ϴ� ���
	if (dataPtr != NULL && dataPtr->arr == false)
	{
		cMyNew.saveFile("ARRAY", ptr, dataPtr);
		return;
	}

	if (dataPtr != NULL)
	{
		// list���� �ּ��� �����͸� �����Ѵ�.
		cMyNew.deleteAllocInfo(dataPtr);

		// �⺻ delete �����ڷ� �޸� ����
		free(ptr);
		return;
	}

	// ���� dataPtr�� NULL�̸� �߸��� �ּҸ� �����Ϸ� �Ѵٸ� ���Ͽ� NoAlloc�� �ۼ��Ѵ�.
	if (dataPtr == NULL)
	{
		if (cMyNew.setting)
		{
			cMyNew.saveFile(getLog(), ptr, NULL);
		}

		cMyNew.saveFile("NOALLOC", ptr, NULL);
		return;
	}
}


void operator delete[](void* p, const char* File, int lIne)
{

}

//------------------------------------------------------------------------
// new[] delete[] ������ �����ε� �Լ�
// 
// 	2��° ���� file�� const�� �ƴ϶�� ������ �߻���
//------------------------------------------------------------------------
void* operator new[](size_t size, const char* file, int line)
	{
		// �⺻ new �����ڷ� �޸� �Ҵ�
		void* ptr = malloc(size);
		// void* ptr = cMyNew.operator new(size);

		// ���� �Ҵ� ������ �����Ѵ�.
		bool check = cMyNew.setAllocInfo(ptr, size, file, line, true);

		// �޸𸮰� ����� �Ҵ� �Ǿ����� ���� üũ
		if (ptr == NULL || check == NULL)
			return NULL;

		// �Ҵ�� �޸� �ּ� ��ȯ
		return ptr;
	}
	//void operator delete[](void* ptr)
	//	{
	//		// list���� �ش� �ּҰ� �ִ��� ã�ƺ���.
	//		stALLOCINFO* dataPtr = cMyNew.findAndAllocInfo((stALLOCINFO*)ptr);


	//		// �迭 delete�� �߾�� �ϴ� ���
	//		if (dataPtr != NULL && dataPtr->arr == false)
	//		{
	//			cMyNew.saveFile("ARRAY", ptr, dataPtr);
	//			return;
	//		}

	//		if (dataPtr != NULL)
	//		{
	//			// list���� �ּ��� �����͸� �����Ѵ�.
	//			cMyNew.deleteAllocInfo(dataPtr);

	//			// �⺻ delete �����ڷ� �޸� ����
	//			free(ptr);
	//			return;
	//		}

	//		// ���� dataPtr�� NULL�̸� �߸��� �ּҸ� �����Ϸ� �Ѵٸ� ���Ͽ� NoAlloc�� �ۼ��Ѵ�.
	//		if (dataPtr == NULL)
	//		{
	//			cMyNew.saveFile("NOALLOC", ptr, NULL);
	//			return;
	//		}
	//	}































///////////////////////////////////////////////////////////////////////////////////////
// ���� ���
// 
///////////////////////////////////////////////////////////////////////////////////////
void setLog(const char* file, int line)
	{
		sprintf_s(cMyNew.log, "%s %d", file, line);
		cMyNew.setting = true;
	}
	char* getLog(void)
	{
		cMyNew.setting = false;
		return cMyNew.log;
	}
