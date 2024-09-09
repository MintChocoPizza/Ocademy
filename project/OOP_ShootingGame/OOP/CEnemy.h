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
	// �Ѿ˿� ���� ������ ���� �����Ͽ�,
	// �߽� �Ҷ����� �˸´� �Ѿ��� �˻��Ͽ�
	// �����ϴ� �ͺ���
	// 
	// �Ѿ˿� ���� ������ ������ �ִ°� �´ٰ� ����
	// 
	//------------------------------------------
	struct Bullet
	{
		int _iCoolTime;		// ��Ÿ���� �Ѿ� ��ü ������ �ѱ��� �ʴ´�.
		char _Skin;
		int _iDamage;
		int _dY;
		int _dX;
		int _iSpeed;		// �������ӿ� 1ĭ�� ������
	};

	struct Moving
	{
		int _CoolTime;
		int dY;
		int	dX;
	};

	// ��ü�� ������� CList�� �Ҹ��ڰ� ������ �޸𸮸� �����Ѵ�.
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

