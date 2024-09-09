#ifndef __CPARSING_H__
#define __CPARSING_H__

#include <Windows.h>

#define CR_MAX_BUFFER 128



////////////////////////////////////////////////////////////////////////////////////////////////////
// 메모리를 언제 해제해야 하는가?
//  -> 직접 함수를 호출하여 해제한다.
//	-> 만약 소멸자에 메모리 해제를 넣는다면, 다른 파일을 불러올 때 마다 소멸자를 호출해야 한다. 
// 
////////////////////////////////////////////////////////////////////////////////////////////////////
class CParsing
{
private:
	FILE* _pFile;
	unsigned char* _readBuffer;					// 버퍼의 시작 주소
	unsigned char* _pLastAddressBuffer;			// 버퍼의 마지막 주소 +1

private:
	//---------------------------------------------------------------------------------------------------
	//  "//", "/* */" 두개를 주석으로 구분한다.
	// 
	//---------------------------------------------------------------------------------------------------
	bool SkipNoneCommand(unsigned char** ucppBuffer);

	//---------------------------------------------------------------------------------------------------
	// 다음 단어 얻기.
	// 
	// Parameters: (char **) 다음 단어의 시작 위치(받음), (int *) 다음 단어의 길이(받음)
	// Return: (BOOL)TRUE, FALSE,
	// 
	//---------------------------------------------------------------------------------------------------
	bool GetNextWord(unsigned char** ucppBuffer, int* ipLength);

public:
	CParsing();
	~CParsing();

	//---------------------------------------------------------------------------------------------------
	// malloc을 사용하여 메모리를 할당한다. 
	// 
	//---------------------------------------------------------------------------------------------------
	void LoadFile(const TCHAR* tcpFileName);
	bool GetValue(const TCHAR* key, int* ipValue);
	bool GetValue(const TCHAR* key, float* fpValue);
	bool GetValue(const TCHAR* key, TCHAR* cpValue);

	void MemFree();
};

#endif // !__CTEXT_PARSING_H__

