
#ifndef __COBJECTBASE_H__
#define __COBJECTBASE_H__

class C_ObjectBase
{
public:
	C_ObjectBase();
	virtual ~C_ObjectBase();

	virtual bool		Update(void) = 0;
	virtual void		Render(void) = 0;
	virtual bool		OnCollision(C_ObjectBase* ptr) = 0;

protected:


};



#endif // !__COBJECTBASE_H__
