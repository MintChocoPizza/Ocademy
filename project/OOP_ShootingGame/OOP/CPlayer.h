#ifndef __CPLAYER_H__
#define __CPLAYER_H__


class CPlayer : public CollisionObject
{
private:
	int _iHP;

	char _cSkin;
	char _cBulletSkin;

	int _iDamage;

	//int _iX;
	//int _iY;

	int _iPlayerSpeed;
	int _iBulletCoolTime;
	int _iBulletSpeed;


public:
	CPlayer(int ObjectType, bool Visible, char* PlayerFile);
	virtual ~CPlayer();

	virtual bool Update(void);
	virtual void Render(void);
	virtual bool OnCollision(CBaseObject* ptr);

	void KeyboardInput(void);
};


#endif // !__PLAYER_H__

