#ifndef __OBJECTLOAD_H__
#define __OBJECTLOAD_H__

///////////////////////////////////////////////////////////////////////
// 
// ���� ������ �����Ѵ�?
// 
///////////////////////////////////////////////////////////////////////
class C_ObjectLoad : public C_ObjectBase
{
public:
	C_ObjectLoad();

	virtual ~C_ObjectLoad();
	virtual bool Update(void);
	virtual void Render(void);
	virtual bool OnCollision(C_ObjectBase* ptr);

public:

};

#endif // !__OBJECTLOAD_H__
