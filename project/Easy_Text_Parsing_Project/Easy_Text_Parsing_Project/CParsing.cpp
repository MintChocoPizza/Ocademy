
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
		// ������ ������ ����� false
		if (*ucppBuffer >= _pLastAddressBuffer)
			return false;

		// "//" �ּ��� ��� 0x0d�� ���ö� ���� �Ѿ��.
		if (isLineComment == true)
		{
			if (**ucppBuffer == 0x0d || **ucppBuffer == 0x0a)
			{
				isLineComment = false;
			}
			++(*ucppBuffer);
			continue;
		}

		// "/* */" �ּ��� ���
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
	// �ּ��� �����̽�, �� ����� ���� �ǳ� �ڴ�.
	// 
	// ������ ���� �����ϸ� false
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
		_tprintf_s(_T("%s ���� ���⿡ �����߽��ϴ�. \n"), fileName);
		throw;
	}

	fseek(_pFile, 0, SEEK_END);
	lFileSize = ftell(_pFile);
	fseek(_pFile, 0, SEEK_SET);

	_readBuffer = (unsigned char*)malloc(lFileSize);
	if (_readBuffer == NULL)
	{
		_tprintf_s(_T("%s ���� Ȯ���� �����߽��ϴ�. \n"), fileName);
		throw;
	}

	//-----------------------------------------------------------------------
	// ������ ������ �ּ� + 1�� ����Ű�� �Ѵ�. 
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
	
	// ã���� �ϴ� �ܾ ���ö����� ��� ã�� ���̹Ƿ� while ������ �˻�
	while (CParsing::GetNextWord(&tcpBuff, &iLength))
	{
		// Word ���ۿ� ã�� �ܾ �����Ѵ�.
		memset(tcWord, 0, 256);
		memcpy_s(tcWord, 256, tcpBuff, iLength);
		
		// tcWord ���ڿ��� 0���� �˻��Ѵ�.
		if (tcWord[0] == _T('\0'))
			return false;
		
		// ���ڷ� �Է� ���� �ܾ�� ������ �˻��Ѵ�.
		if (0 == _tcscmp(key, tcWord))
		{
			// �´ٸ� �ٷ� �ڿ� = �� ã�´�. 
			if (CParsing::GetNextWord(&tcpBuff, &iLength))
			{
				memset(tcWord, 0, 256);
				memcpy_s(tcWord, 256, tcpBuff, iLength);
				if (0 == _tcscmp(tcWord, _T("=")))
				{
					
					// = ������ ������ �κ��� ��´�. 
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

