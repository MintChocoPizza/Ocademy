
#include <tchar.h>
#include <cstring>
#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>

#include "CParsing.h"

bool CParsing::SkipNoneCommand(unsigned char** ucppBuffer)
{
	bool isLineComment = false;
	bool isMultiLineComments = false;




	while (1)
	{
		// 버퍼의 범위를 벗어나면 false
		if (*ucppBuffer >= _pLastAddressBuffer)
			return false;

		// "//" 주석인 경우 0x0d가 나올때 까지 넘어간다.
		if (isLineComment == true)
		{
			if (**ucppBuffer == 0x0d || **ucppBuffer == 0x0a)
			{
				isLineComment = false;
			}
			++(*ucppBuffer);
			continue;
		}

		// "/* */" 주석인 경우
		if (isMultiLineComments == true)
		{
			if (**ucppBuffer == '*' && *((*ucppBuffer)+1 ) == '/')
			{
				isMultiLineComments = false;
				*ucppBuffer += 2;
			}
			else
			{
				++(*ucppBuffer);
			}

			continue;
		}


		if (**ucppBuffer == ',' || **ucppBuffer == '.' || **ucppBuffer == '"' ||
			**ucppBuffer == 0x20 || **ucppBuffer == 0x08 || **ucppBuffer == 0x09 ||
			**ucppBuffer == 0x0a || **ucppBuffer == 0x0d)
		{
			++(*ucppBuffer);
			continue;
		}
		else if(**ucppBuffer == '/')
		{
			if (*((*ucppBuffer) + 1) == '/')
			{
				isLineComment = true;
				*ucppBuffer += 2;
				continue;
			}
			else if (*((*ucppBuffer) + 1) == '*')
			{
				isMultiLineComments = true;
				*ucppBuffer += 2;
				continue;
			}
		}

		break;
	}

	return true;
}

bool CParsing::GetNextWord(unsigned char** ucppBuffer, int* ipLength)
{
	int iLength = 0;
	//---------------------------------------------------------------
	// 주석과 스페이스, 탭 등등을 전부 건너 뛴다.
	// 
	// 파일의 끝에 도달하면 false
	// 
	//---------------------------------------------------------------
	if (CParsing::SkipNoneCommand(ucppBuffer)) 
	{
		unsigned char* start = *ucppBuffer;
		while (*start != '\0' && *start != ',' && *start != '.' &&
			*start != '"' && *start != ' ' && *start != '\t' &&
			*start != '\r' && *start != '\n')
		{
			++iLength;
			++start;
		}
	}
	else
	{
		return false;
	}

	*ipLength = iLength;


	if (iLength > 0)
	{
		return true;
	}


	return false;
}

CParsing::CParsing() : _pFile(nullptr), _readBuffer(nullptr), _pLastAddressBuffer(nullptr)
{
	
}

CParsing::~CParsing()
{
}

void CParsing::LoadFile(const TCHAR* fileName)
{
	errno_t err;
	long lFileSize;


	err = _tfopen_s(&_pFile, fileName, _T("r, ccs=UTF-8"));
	if (err != NULL || _pFile == NULL)
	{
		_tprintf_s(_T("%s 파일 열기에 실패했습니다. \n"), fileName);
		throw;
	}

	fseek(_pFile, 0, SEEK_END);
	lFileSize = ftell(_pFile);
	fseek(_pFile, 0, SEEK_SET);

	_readBuffer = (unsigned char*)malloc(lFileSize);
	if (_readBuffer == NULL)
	{
		_tprintf_s(_T("%s 버퍼 확보에 실패했습니다. \n"), fileName);
		throw;
	}

	//-----------------------------------------------------------------------
	// 버퍼의 마지막 주소 + 1을 가르키게 한다. 
	// 
	//-----------------------------------------------------------------------
	_pLastAddressBuffer = _readBuffer + lFileSize;


	fread_s(_readBuffer, lFileSize, 1, lFileSize, _pFile);



	fclose(_pFile);
}

bool CParsing::GetValue(const TCHAR* key, int *ipValue)
{
	unsigned char* tcpBuff = _readBuffer;
	TCHAR tcWord[256];
	int iLength;
	
	// 찾고자 하는 단어가 나올때까지 계속 찾을 것이므로 while 문으로 검사
	while (CParsing::GetNextWord(&tcpBuff, &iLength))
	{
		// Word 버퍼에 찾은 단어를 저장한다.
		memset(tcWord, 0, 256);
		memcpy_s(tcWord, 256, tcpBuff, iLength);
		
		// tcWord 문자열이 0인지 검사한다.
		if (tcWord[0] == _T('\0'))
			return false;
		
		// 인자로 입력 받은 단어와 같은지 검사한다.
		if (0 == _tcscmp(key, tcWord))
		{
			// 맞다면 바로 뒤에 = 을 찾는다. 
			if (CParsing::GetNextWord(&tcpBuff, &iLength))
			{
				memset(tcWord, 0, 256);
				memcpy_s(tcWord, 256, tcpBuff, iLength);
				if (0 == _tcscmp(tcWord, _T("=")))
				{
					
					// = 다음의 데이터 부분을 얻는다. 
					if (GetNextWord(&tcpBuff, &iLength))
					{
						memset(tcWord, 0, 256);
						memcpy_s(tcWord, 256, tcpBuff, iLength);

						*ipValue = _ttoi(tcWord);
						return true;
					}
					return false;
				}
				return false;
			}
			return false;

		}
	}

	return false;
}

bool CParsing::GetValue(const TCHAR* key, float* fValue)
{
	return false;
}

bool CParsing::GetValue(const TCHAR* key, TCHAR* cValue)
{
	return false;
}

void CParsing::MemFree()
{
	free(_readBuffer);
}

