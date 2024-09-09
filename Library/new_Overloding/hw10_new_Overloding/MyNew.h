#ifndef __MYNEW_H__
#define __MYNEW_H__


/////////////////////////////////////////////////////////////////////////////////////
// 
// LEAK: 프로그램이 끝날 때 까지 해지하지 않은 메모리
// 
// ARRAY: 배열로 할당한 메모리를 delete[]를 하지 않고 그냥 delete를 한 메모리 
// 
// NOALLOC: 이상한 잘못된 주소를 해지하려 한다.
// 
// 클래스 배열 할당 -> 일반 클래스 해제
// 클래스 배열의 경우 ARRAY가 안나타나고 
// NOALLOC과 LEAK가 동시에 나타난다. 
// 동적할당 메모리 주소 앞에 클래스 배열의 크기를 저장하고
// 실제로는 그 메모리를 할당 취소한다.
// => 그러므로 잘못된 메모리를 해제하러 가고, 실제 메모리는 해제하지 못하여 동시에 생김
// 
// 일반 클래스 할당 -> 배열 클래스 해제
// 소멸자에서 값의 변경이 있을 경우 
// 클래스 배열의 주소를 저장하는 주소 앞 공간을 확인하고 
// 변수의 위치를 잡기 때문에 위치를 찾을 수 없어서 어떠한 일이 생길지 모른다. 
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
	// 강제 기록 로그
	// 
	///////////////////////////////////////////////////////////////////////////////////////
public: char log[256];
public:	bool setting;

public:
	cMYNEW();
	~cMYNEW();

	///////////////////////////////////////////////////////////////////////////////////////
	// 파일에 데이터 저장 함수
	// 
	///////////////////////////////////////////////////////////////////////////////////////
	void saveFile(const char* log, void* ptr, stALLOCINFO* stData);



	///////////////////////////////////////////////////////////////////////////////////////
	// 데이터 관리 함수들
	// 
	///////////////////////////////////////////////////////////////////////////////////////
	bool setAllocInfo(void* ptr, size_t size, const char* file, int line, bool arr);
	stALLOCINFO* findAndAllocInfo(void* ptr);
	void deleteAllocInfo(stALLOCINFO* ptr);


};


void setLog(const char* file, int line);
char* getLog(void);





//------------------------------------------------------------------------
// new 연산자 오버로딩 함수
// 
// 	2번째 인자 file이 const가 아니라면 오류가 발생함
//------------------------------------------------------------------------
void* operator new(size_t size, const char* file, int line);
void operator delete(void* ptr);
void operator delete(void* p, const char* File, int lIne);


void* operator new[](size_t size, const char* file, int line);
void operator delete[](void* ptr);
void operator delete[](void* p, const char* File, int lIne);




//------------------------------------------------------------------------
// delete 연산자 오버로딩 함수
// 
//------------------------------------------------------------------------


#define  new    new(__FILE__, __LINE__)


#endif // !__MYNEW_H__
