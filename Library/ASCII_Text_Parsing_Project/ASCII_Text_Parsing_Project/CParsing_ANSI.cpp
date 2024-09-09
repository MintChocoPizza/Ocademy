
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
		// ������ ������ ����� false;
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
	// �ּ��� �����̽�, �� ����� ���� �ǳ� �ڴ�.
	// 
	// ������ ���� �����ϸ� false
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
		printf_s("%s ���� ���⿡ �����߽��ϴ�. \n", cchpFileName);
		throw;
	}

	fseek(pFile, 0, SEEK_END);
	lFileSize = ftell(pFile) + 1;
	fseek(pFile, 0, SEEK_SET);

	_readBuffer = (unsigned char*)malloc(lFileSize);
	if (_readBuffer == NULL)
	{
		printf_s("%s ���� Ȯ���� �����߽��ϴ�. \n", cchpFileName);
		throw;
	}

	//-----------------------------------------------------------------------
	// ������ �ּҸ� _tempBuffer�� �����Ѵ�.
	// ������ ������ �ּ� + 1�� ����Ű�� �Ѵ�. 
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
	// ������ ������ ó������ Ž���Ѵ�. 
	_tempBuffer = _readBuffer;
	char chWord[256];
	int iLength;

	// ã���� �ϴ� �ܾ ���ö����� ��� ã�� ���̹Ƿ� while ������ �˻�.
	while (GetNextWord(&_tempBuffer, &iLength))
	{
		// chWord ���ۿ� ã�� �ܾ �����Ѵ�. 
		memset(chWord, 0, 256);
		memcpy_s(chWord, 256, _tempBuffer, iLength);
		_tempBuffer += iLength;

		if (chWord[0] == '\0')
			return false;

		// ���ڷ� �Է� ���� �ܾ�� ������ �˻��Ѵ�. 
		if (0 == strcmp(key, chWord))
		{
			// �´ٸ� �ٷ� �ڿ� = �� ã��.
			if (GetNextWord(&_tempBuffer, &iLength))
			{
				memset(chWord, 0, 256);
				memcpy_s(chWord, 256, _tempBuffer, iLength);
				_tempBuffer += iLength;
				if (0 == strcmp(chWord, "="))
				{
					// = ������ ������ �κ��� ����.
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
	// ������ ������ ó������ Ž���Ѵ�. 
	_tempBuffer = _readBuffer;
	char chWord[256];
	int iLength;

	// ã���� �ϴ� �ܾ ���ö����� ��� ã�� ���̹Ƿ� while ������ �˻�.
	while (GetNextWord(&_tempBuffer, &iLength))
	{
		// chWord ���ۿ� ã�� �ܾ �����Ѵ�. 
		memset(chWord, 0, 256);
		memcpy_s(chWord, 256, _tempBuffer, iLength);
		_tempBuffer += iLength;

		if (chWord[0] == '\0')
			return false;

		// ���ڷ� �Է� ���� �ܾ�� ������ �˻��Ѵ�. 
		if (0 == strcmp(key, chWord))
		{
			// �´ٸ� �ٷ� �ڿ� = �� ã��.
			if (GetNextWord(&_tempBuffer, &iLength))
			{
				memset(chWord, 0, 256);
				memcpy_s(chWord, 256, _tempBuffer, iLength);
				_tempBuffer += iLength;
				if (0 == strcmp(chWord, "="))
				{
					// = ������ ������ �κ��� ����.
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
	// ������ ������ ó������ Ž���Ѵ�. 
	_tempBuffer = _readBuffer;
	char chWord[256];
	int iLength;

	// ã���� �ϴ� �ܾ ���ö����� ��� ã�� ���̹Ƿ� while ������ �˻�.
	while (GetNextWord(&_tempBuffer, &iLength))
	{
		// chWord ���ۿ� ã�� �ܾ �����Ѵ�. 
		memset(chWord, 0, 256);
		memcpy_s(chWord, 256, _tempBuffer, iLength);
		_tempBuffer += iLength;

		if (chWord[0] == '\0')
			return false;

		// ���ڷ� �Է� ���� �ܾ�� ������ �˻��Ѵ�. 
		if (0 == strcmp(key, chWord))
		{
			// �´ٸ� �ٷ� �ڿ� = �� ã��.
			if (GetNextWord(&_tempBuffer, &iLength))
			{
				memset(chWord, 0, 256);
				memcpy_s(chWord, 256, _tempBuffer, iLength);
				_tempBuffer += iLength;
				if (0 == strcmp(chWord, "="))
				{
					// = ������ ������ �κ��� ����.
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
	// ������ ������ ó������ Ž���Ѵ�. 
	_tempBuffer = _readBuffer;
	char chWord[256];
	int iLength;

	// ã���� �ϴ� �ܾ ���ö����� ��� ã�� ���̹Ƿ� while ������ �˻�.
	while (GetNextWord(&_tempBuffer, &iLength))
	{
		// chWord ���ۿ� ã�� �ܾ �����Ѵ�. 
		memset(chWord, 0, 256);
		memcpy_s(chWord, 256, _tempBuffer, iLength);
		_tempBuffer += iLength;

		if (chWord[0] == '\0')
			return false;

		// ���ڷ� �Է� ���� �ܾ�� ������ �˻��Ѵ�. 
		if (0 == strcmp(key, chWord))
		{
			// �´ٸ� �ٷ� �ڿ� = �� ã��.
			if (GetNextWord(&_tempBuffer, &iLength))
			{
				memset(chWord, 0, 256);
				memcpy_s(chWord, 256, _tempBuffer, iLength);
				_tempBuffer += iLength;
				if (0 == strcmp(chWord, "="))
				{
					// = ������ ������ �κ��� ����.
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
