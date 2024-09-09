#ifndef __CENEMY_H__
#define __CENEMY_H__

//class CEnemy : public CBaseObject
class CEnemy : public CollisionObject
{
private:
	char		_Skin;
	int			_Hp;
	//int			_iY;
	//int			_iX;

	//------------------------------------------
	// 총알에 대한 정보를 따로 저장하여,
	// 발싸 할때마다 알맞는 총알을 검색하여
	// 생성하는 것보다
	// 
	// 총알에 대한 정보를 가지고 있는게 맞다고 생각
	// 
	//------------------------------------------
	struct Bullet
	{
		int _iCoolTime;		// 쿨타임은 총알 객체 생성시 넘기지 않는다.
		char _Skin;
		int _iDamage;
		int _dY;
		int _dX;
		int _iSpeed;		// 몇프레임에 1칸을 가는지
	};

	struct Moving
	{
		int _CoolTime;
		int dY;
		int	dX;
	};

	// 객체가 사라지면 CList의 소멸자가 노드들의 메모리를 해제한다.
	CList<Moving> MovingList;
	CList<Moving>::iterator iter;

	CList<Bullet> BulletList;
	
public:
	CEnemy(int ObjectType, bool Visible, char cSkin, int iHp, int iY, int iX, char* cMoveFile, char* GunFile);
	virtual ~CEnemy();

	virtual bool	Update(void);
	virtual void	Render(void);
	virtual bool	OnCollision(CBaseObject* ptr);

private:
	void GetMoving(char* cMoveFile);
	void GetBullet(char* cBulletFile);
	void Move(void);
	void CreateBullet(void);
};

#endif // !__CENEMY_H__

