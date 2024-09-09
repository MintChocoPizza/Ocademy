
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <Windows.h>

#include "CParsing_ANSI.h"



CParsing_ANSI::CParsing_ANSI() : _readBuffer(nullptr), _pLastAddressBuffer(nullptr), _tempBuffer(nullptr)
{
}

CParsing_ANSI::~CParsing_ANSI()
{
	free(_readBuffer);
}


bool CParsing_ANSI::SkipNoneCommand(unsigned char** ucppBuffer)
{
	bool isLineComment = false;
	bool isMultLineComments = false;

	while (1)
	{
		// 버퍼의 범위를 벗어나면 false;
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
		if (isMultLineComments == true)
		{
			if (**ucppBuffer == '*' && *((*ucppBuffer) + 1) == '/')
			{
				isMultLineComments = false;
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
		else if (**ucppBuffer == '/')
		{
			if (*((*ucppBuffer) + 1) == '/')
			{
				isLineComment = true;
				*ucppBuffer += 2;
				continue;
			}
			else if (*((*ucppBuffer) + 1) == '*')
			{
				isMultLineComments = true;
				*ucppBuffer += 2;
				continue;
			}
		}

		break;
	}

	return true;
}

bool CParsing_ANSI::GetNextWord(unsigned char** ucppBuffer, int* ipLength)
{
	int iLength = 0;
	//---------------------------------------------------------------
	// 주석과 스페이스, 탭 등등을 전부 건너 뛴다.
	// 
	// 파일의 끝에 도달하면 false
	// 
	//---------------------------------------------------------------

	if (SkipNoneCommand(ucppBuffer))
	{
		unsigned char* start = *ucppBuffer;

		while ((start < _pLastAddressBuffer) && *start != '\0' && *start != ' ' && *start != '\t' &&
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

	if (iLength > 0)
	{

		*ipLength = iLength;
		return true;
	}

	return false;
}

bool CParsing_ANSI::GetNextValue(unsigned char** ucppBuffer, int* ipLength)
{
	int iLength = 0;

	if (SkipNoneCommand(ucppBuffer))
	{
		unsigned char* start = *ucppBuffer;
		while ((start < _pLastAddressBuffer) && *start != ';')
		{
			++iLength;
			++start;
		}
	}
	else
	{
		return false;
	}

	if (iLength > 0)
	{
		*ipLength = iLength;
		return true;
	}

	return false;
}

void CParsing_ANSI::LoadFile(const char* cchpFileName)
{
	errno_t err;
	FILE* pFile;
	long lFileSize;

	err = fopen_s(&pFile, cchpFileName, "r, ccs=UNICODE");
	if (err != NULL || pFile == NULL)
	{
		printf_s("%s 파일 열기에 실패했습니다. \n", cchpFileName);
		throw;
	}

	fseek(pFile, 0, SEEK_END);
	lFileSize = ftell(pFile) + 1;
	fseek(pFile, 0, SEEK_SET);

	_readBuffer = (unsigned char*)malloc(lFileSize);
	if (_readBuffer == NULL)
	{
		printf_s("%s 버퍼 확보에 실패했습니다. \n", cchpFileName);
		throw;
	}

	//-----------------------------------------------------------------------
	// 버퍼의 주소를 _tempBuffer에 복사한다.
	// 버퍼의 마지막 주소 + 1을 가르키게 한다. 
	// 
	//-----------------------------------------------------------------------
	_tempBuffer = _readBuffer;
	_pLastAddressBuffer = _readBuffer + lFileSize;
	// _pLastAddressBuffer = _readBuffer + (lFileSize - 1);

	fread_s(_readBuffer, lFileSize, 1, lFileSize, pFile);
	_readBuffer[lFileSize - 1] = '\0';

	fclose(pFile);
}

bool CParsing_ANSI::GetValue(const char* key, int* ipValue)
{
	// 무조건 버퍼의 처음부터 탐색한다. 
	_tempBuffer = _readBuffer;
	char chWord[256];
	int iLength;

	// 찾고자 하는 단어가 나올때까지 계속 찾을 것이므로 while 문으로 검사.
	while (GetNextWord(&_tempBuffer, &iLength))
	{
		// chWord 버퍼에 찾은 단어를 저장한다. 
		memset(chWord, 0, 256);
		memcpy_s(chWord, 256, _tempBuffer, iLength);
		_tempBuffer += iLength;

		if (chWord[0] == '\0')
			return false;

		// 인자로 입력 받은 단어와 같은지 검사한다. 
		if (0 == strcmp(key, chWord))
		{
			// 맞다면 바로 뒤에 = 을 찾자.
			if (GetNextWord(&_tempBuffer, &iLength))
			{
				memset(chWord, 0, 256);
				memcpy_s(chWord, 256, _tempBuffer, iLength);
				_tempBuffer += iLength;
				if (0 == strcmp(chWord, "="))
				{
					// = 다음의 데이터 부분을 얻자.
					if (GetNextValue(&_tempBuffer, &iLength))
					{
						memset(chWord, 0, 256);
						memcpy_s(chWord, 256, _tempBuffer, iLength);
						// _tempBuffer += iLength;
						*ipValue = atoi(chWord);
						return true;
					}
				}
			}
			return false;
		}

	}
	return false;
}

bool CParsing_ANSI::GetValue(const char* key, double* fpValue)
{
	// 무조건 버퍼의 처음부터 탐색한다. 
	_tempBuffer = _readBuffer;
	char chWord[256];
	int iLength;

	// 찾고자 하는 단어가 나올때까지 계속 찾을 것이므로 while 문으로 검사.
	while (GetNextWord(&_tempBuffer, &iLength))
	{
		// chWord 버퍼에 찾은 단어를 저장한다. 
		memset(chWord, 0, 256);
		memcpy_s(chWord, 256, _tempBuffer, iLength);
		_tempBuffer += iLength;

		if (chWord[0] == '\0')
			return false;

		// 인자로 입력 받은 단어와 같은지 검사한다. 
		if (0 == strcmp(key, chWord))
		{
			// 맞다면 바로 뒤에 = 을 찾자.
			if (GetNextWord(&_tempBuffer, &iLength))
			{
				memset(chWord, 0, 256);
				memcpy_s(chWord, 256, _tempBuffer, iLength);
				_tempBuffer += iLength;
				if (0 == strcmp(chWord, "="))
				{
					// = 다음의 데이터 부분을 얻자.
					if (GetNextValue(&_tempBuffer, &iLength))
					{
						memset(chWord, 0, 256);
						memcpy_s(chWord, 256, _tempBuffer, iLength);
						// _tempBuffer += iLength;
						*fpValue = atof(chWord);
						return true;
					}
				}
			}
			return false;
		}

	}
	return false;
}

bool CParsing_ANSI::GetValue(const char* key, char* cpValue)
{
	// 무조건 버퍼의 처음부터 탐색한다. 
	_tempBuffer = _readBuffer;
	char chWord[256];
	int iLength;

	// 찾고자 하는 단어가 나올때까지 계속 찾을 것이므로 while 문으로 검사.
	while (GetNextWord(&_tempBuffer, &iLength))
	{
		// chWord 버퍼에 찾은 단어를 저장한다. 
		memset(chWord, 0, 256);
		memcpy_s(chWord, 256, _tempBuffer, iLength);
		_tempBuffer += iLength;

		if (chWord[0] == '\0')
			return false;

		// 인자로 입력 받은 단어와 같은지 검사한다. 
		if (0 == strcmp(key, chWord))
		{
			// 맞다면 바로 뒤에 = 을 찾자.
			if (GetNextWord(&_tempBuffer, &iLength))
			{
				memset(chWord, 0, 256);
				memcpy_s(chWord, 256, _tempBuffer, iLength);
				_tempBuffer += iLength;
				if (0 == strcmp(chWord, "="))
				{
					// = 다음의 데이터 부분을 얻자.
					if (GetNextValue(&_tempBuffer, &iLength))
					{
					
						if (iLength == 1)
						{
							memset(chWord, 0, 256);
							memcpy_s(chWord, 256, _tempBuffer, iLength);
							// _tempBuffer += iLength;

							*cpValue = chWord[0];

							return true;
						}
						else
						{
							return false;
						}
					}
				}
			}
			else
			{
				return false;
			}
		}

	}
	return false;
}

bool CParsing_ANSI::GetValue(const char* key, char cpValue[], int iSize)
{
	// 무조건 버퍼의 처음부터 탐색한다. 
	_tempBuffer = _readBuffer;
	char chWord[256];
	int iLength;

	// 찾고자 하는 단어가 나올때까지 계속 찾을 것이므로 while 문으로 검사.
	while (GetNextWord(&_tempBuffer, &iLength))
	{
		// chWord 버퍼에 찾은 단어를 저장한다. 
		memset(chWord, 0, 256);
		memcpy_s(chWord, 256, _tempBuffer, iLength);
		_tempBuffer += iLength;

		if (chWord[0] == '\0')
			return false;

		// 인자로 입력 받은 단어와 같은지 검사한다. 
		if (0 == strcmp(key, chWord))
		{
			// 맞다면 바로 뒤에 = 을 찾자.
			if (GetNextWord(&_tempBuffer, &iLength))
			{
				memset(chWord, 0, 256);
				memcpy_s(chWord, 256, _tempBuffer, iLength);
				_tempBuffer += iLength;
				if (0 == strcmp(chWord, "="))
				{
					// = 다음의 데이터 부분을 얻자.
					if (GetNextValue(&_tempBuffer, &iLength))
					{
						memset(chWord, 0, 256);
						memcpy_s(chWord, 256, _tempBuffer, iLength);
						// _tempBuffer += iLength;
						strcpy_s(cpValue, iSize, chWord);
						return true;
					}
				}
			}
			else
			{
				return false;
			}
		}

	}
	return false;
}
