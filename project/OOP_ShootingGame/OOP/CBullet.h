#ifndef __CBULLET_H__
#define __CBULLET_H__



//class CBullet : public CBaseObject
class CBullet : public CollisionObject
{
private:
	char _Skin;
	int _iDamage;
	int _iSpeed;
	
	

public:
	CBullet(int ObjectType,
		bool Visible,
		char Skin,
		int iY,
		int iX,
		int dy,
		int dx,
		int iDamage,
		int iSpeed);
	virtual ~CBullet();

	virtual bool	Update(void);
	virtual void	Render(void);
	virtual bool	OnCollision(CBaseObject* ptr);

	int GetDamage(void);

};


#endif // !__CBULLET_H__

