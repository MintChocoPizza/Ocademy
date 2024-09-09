//////////////////////////////////////////////////////////////////
// 
// ����: 
// 
// ���α׷� ���۽� 	ProfileReset();
// 
// ���ϴ� ������ �� ���� ��ܿ� cMYPROFILE cTest(_T("�������ϸ� �̸�"));
// 
// ���α׷� ���� �� 	ProfileDataOutText(_T("������ ���� �̸�.txt"));
// 
//////////////////////////////////////////////////////////////////
#ifndef __MYPROFILE_H__
#define __MYPROFILE_H__

#include <Windows.h>
#include <time.h>
#include <tchar.h>

#include <stdio.h>

#define PROFILE

#ifdef PROFILE
	#define PRO_BEGIN(TagName)		ProfileBegin(TagName)
	#define PRO_END(TagName)		ProfileEnd(TagName)
#else
	#define PRO_BEGIN(TagName)
	#define PRO_END(TagName)
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// �ϳ��� �Լ� Profiling ����, �� �Լ�
// 
// Parameters: (char *)Profiling �̸�
// Return: ����
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ProfileBegin(const TCHAR* szName);
void ProfileEnd(const TCHAR* szName);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Profiling �� �����͸� Text ���Ϸ� ����Ѵ�. 
// 
// Parameters: (char *)��µ� ���� �̸�
// Return: ����
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ProfileDataOutText(const TCHAR* szFileName);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Profiling �� �����͸� ��� �ʱ�ȭ �Ѵ�. 
// 
// Parameters: ����
// Return: ����
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ProfileReset(void);


struct stPROFILE_SAMPLE
{
	 unsigned long	lFlag;						// ���������� ��� ����
	TCHAR			szName[64];					// �������� ���� �̸� 
	LARGE_INTEGER	lStartTime;					// �������� ���� ���� �ð�
	__int64			iTotalTime;					// ��ü ���ð� ī���� Time. (��½� ȣ��ȸ���� ������ ��� ����)
	__int64			iMin[2];					// �ּ� ���ð� ī���� Time. (�ʴ����� ����Ͽ� ����/ [0] ���� �ּ� [1] ���� �ּ� [2])
	__int64			iMax[2];					// �ִ� ���ð� ī���� Time. (�ʴ����� ����Ͽ� ����/ [0] ���� �ִ� [1] ���� �ִ� [2])
	__int64			iCall;;						// ���� ȣ�� Ƚ��
};

class cMYPROFILE
{
	const TCHAR* _tag;

public:
	cMYPROFILE(const TCHAR* tag)
	{
		PRO_BEGIN(tag);
		_tag = tag;
	}
	~cMYPROFILE()
	{
		PRO_END(_tag);
	}
};




#endif // !__MYPROFILE_H__
