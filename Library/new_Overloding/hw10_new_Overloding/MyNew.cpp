#include <malloc.h>
#include <stdio.h>
#include <time.h>

#include "MyNew.h"

#undef new
#undef setLog

cMYNEW cMyNew;

/////////////////////////////////////////////////////////////////////////////////////////////
// 클래스
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
	// 강제 기록 로그
	// 
	///////////////////////////////////////////////////////////////////////////////////////
	setting = false;
	log[0] = NULL;
}

cMYNEW::~cMYNEW()
{
	//------------------------------------------------------------------------
	// 파일에 저장한다. 
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
// 파일에 데이터 저장 함수
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
		// 로그 [주소] [데이터크기] 경로 라인
		fprintf_s(pFile, "%s [%p] [%10zd] %s : %d \n", log, ptr, stData->size, stData->fileName, stData->line);
	}

	fclose(pFile);
}







///////////////////////////////////////////////////////////////////////////////////////
// 데이터 관리 함수들
// 
///////////////////////////////////////////////////////////////////////////////////////
bool cMYNEW::setAllocInfo(void* ptr, size_t size, const char* file, int line, bool arr)
{
	int iCnt = 0;
	stALLOCINFO* stNewData = (stALLOCINFO*)malloc(sizeof(stALLOCINFO));

	if (stNewData == NULL)
		return false;

	// 할당된 메모리 정보 저장, 초기화
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
	// linked list pTail의 앞에 newData를 연결한다.
	// 
	//------------------------------------------------------------------------
	// 1. newData와 Tail의 앞 노드를 연결한다.
	stNewData->pPreviousNode = pTail.pPreviousNode;
	// 2. Tail의 앞 노드와 newData를 연결한다. 
	pTail.pPreviousNode->pNextNode = stNewData;
	// 3. newData와 Tail을 연결한다. 
	stNewData->pNextNode = &pTail;
	// 4. Tail을 newData와 연결한다. 
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
// 전역함수 
// 
// 연산자 오버로딩
// 
/////////////////////////////////////////////////////////////////////////////////////////////

//------------------------------------------------------------------------
// new delete 연산자 오버로딩 함수
// 
// 	2번째 인자 file이 const가 아니라면 오류가 발생함
//------------------------------------------------------------------------
void* operator new(size_t size, const char* file, int line)
{
	// 기본 new 연산자로 메모리 할당
	void* ptr = malloc(size);
	//void* ptr = cMyNew.operator new(size);

	// 동적 할당 정보를 저장한다.
	bool check = cMyNew.setAllocInfo(ptr, size, file, line, false);

	// 메모리가 제대로 할당 되었는지 예외 체크
	if (ptr == NULL || check == NULL)
		return NULL;

	// 할당된 메모리 주소 반환
	return ptr;
}
void operator delete(void* ptr)
{
	// list에서 해당 주소가 있는지 찾아본다.
	stALLOCINFO* dataPtr = cMyNew.findAndAllocInfo(ptr);


	// 배열 delete를 했어야 하는 경우
	if (dataPtr != NULL && dataPtr->arr == true)
	{
		cMyNew.saveFile("ARRAY", ptr, dataPtr);
		return;
	}

	if (dataPtr != NULL)
	{
		// list에서 주소의 데이터를 삭제한다.
		cMyNew.deleteAllocInfo(dataPtr);

		// 기본 delete 연산자로 메모리 해제
		free(ptr);
		return;
	}

	// 만약 dataPtr이 NULL이면 잘못된 주소를 해지하려 한다면 파일에 NoAlloc를 작성한다.
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
	// list에서 해당 주소가 있는지 찾아본다.
	stALLOCINFO* dataPtr = cMyNew.findAndAllocInfo((stALLOCINFO*)ptr);

	// 배열 delete를 했어야 하는 경우
	if (dataPtr != NULL && dataPtr->arr == false)
	{
		cMyNew.saveFile("ARRAY", ptr, dataPtr);
		return;
	}

	if (dataPtr != NULL)
	{
		// list에서 주소의 데이터를 삭제한다.
		cMyNew.deleteAllocInfo(dataPtr);

		// 기본 delete 연산자로 메모리 해제
		free(ptr);
		return;
	}

	// 만약 dataPtr이 NULL이면 잘못된 주소를 해지하려 한다면 파일에 NoAlloc를 작성한다.
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
// new[] delete[] 연산자 오버로딩 함수
// 
// 	2번째 인자 file이 const가 아니라면 오류가 발생함
//------------------------------------------------------------------------
void* operator new[](size_t size, const char* file, int line)
	{
		// 기본 new 연산자로 메모리 할당
		void* ptr = malloc(size);
		// void* ptr = cMyNew.operator new(size);

		// 동적 할당 정보를 저장한다.
		bool check = cMyNew.setAllocInfo(ptr, size, file, line, true);

		// 메모리가 제대로 할당 되었는지 예외 체크
		if (ptr == NULL || check == NULL)
			return NULL;

		// 할당된 메모리 주소 반환
		return ptr;
	}
	//void operator delete[](void* ptr)
	//	{
	//		// list에서 해당 주소가 있는지 찾아본다.
	//		stALLOCINFO* dataPtr = cMyNew.findAndAllocInfo((stALLOCINFO*)ptr);


	//		// 배열 delete를 했어야 하는 경우
	//		if (dataPtr != NULL && dataPtr->arr == false)
	//		{
	//			cMyNew.saveFile("ARRAY", ptr, dataPtr);
	//			return;
	//		}

	//		if (dataPtr != NULL)
	//		{
	//			// list에서 주소의 데이터를 삭제한다.
	//			cMyNew.deleteAllocInfo(dataPtr);

	//			// 기본 delete 연산자로 메모리 해제
	//			free(ptr);
	//			return;
	//		}

	//		// 만약 dataPtr이 NULL이면 잘못된 주소를 해지하려 한다면 파일에 NoAlloc를 작성한다.
	//		if (dataPtr == NULL)
	//		{
	//			cMyNew.saveFile("NOALLOC", ptr, NULL);
	//			return;
	//		}
	//	}































///////////////////////////////////////////////////////////////////////////////////////
// 강제 기록
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
