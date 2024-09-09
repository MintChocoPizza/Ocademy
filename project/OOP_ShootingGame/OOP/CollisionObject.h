#ifndef __COLLISIONOBJECT_H__
#define __COLLISIONOBJECT_H__

///////////////////////////////////////////////////////////
// 
// 이 클래스는 충돌 처리를 하기 위하여 만들었다. 
// 
// 
// 
///////////////////////////////////////////////////////////
class CollisionObject : public CBaseObject
{
public :
	CollisionObject(int ObjectType, bool Visible);
	virtual ~CollisionObject();

	virtual bool	Update(void) = 0;
	virtual void	Render(void) = 0;
	virtual bool	OnCollision(CBaseObject* ptr) = 0;

	bool Collision(int iY, int iX);

protected:
	int _iY;
	int _iX;
	int _dy;
	int _dx;
};

#endif // !__COLLISIONOBJECT_H__

