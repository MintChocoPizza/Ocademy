#ifndef __COLLISIONOBJECT_H__
#define __COLLISIONOBJECT_H__

///////////////////////////////////////////////////////////
// 
// �� Ŭ������ �浹 ó���� �ϱ� ���Ͽ� �������. 
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

