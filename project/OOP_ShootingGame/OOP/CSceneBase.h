#ifndef __CSCENEBASE_H__
#define __CSCENEBASE_H__


class CSceneBase
{


public:
	CSceneBase();
	virtual ~CSceneBase();


	virtual bool			Update(void) = 0;

protected:
};

#endif // !__CSCENEBASE_H__

