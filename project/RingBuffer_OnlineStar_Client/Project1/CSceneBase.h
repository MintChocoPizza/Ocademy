
#ifndef __CSCENEBASE_H__
#define __CSCENEBASE_H__

class C_SceneBase
{
public:
	C_SceneBase();
	virtual ~C_SceneBase();

	virtual bool Update(void) = 0;

protected:
};

#endif // !__CSCENEBASE_H__
