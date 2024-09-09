
#ifndef __CBASEOBJECT_H__
#define __CBASEOBJECT_H__

////////////////////////////////////////////////////////////////////////////
// 오브젝트 들을 관리할 최상의 객체
// 
// 해당 오브젝트를 다양하게 사용하고 있다. 
// 
// 좀 더 좋은 충돌처리를 하고 싶다면, 
// 
// 충돌 가능한 오브젝트와 가능하지 않은 오브젝트를 분리하여 상속시킨다.
// 
// 
// 또는 CBaseObject에서 Y,X,dY,dX 값을 들고 충돌처리를 한다.
// 
// 또는 충돌처리 함수를 virtural을 사용하여 만들고, 자식 클래스에게 넘긴다.
// 
////////////////////////////////////////////////////////////////////////////
class CBaseObject
{
public:
	CBaseObject(int ObjectType, bool Visible);
	virtual ~CBaseObject();

	virtual bool		Update(void) = 0;
	virtual void		Render(void) = 0;
	virtual bool		OnCollision(CBaseObject* ptr) = 0;

	int GetVisible(void);
	int GetObjectType(void);


protected:
	int				_ObjectType; 
	int				_Visible;
};


#endif // !__CBASEOBJECT_H__
