
#ifndef __CBASEOBJECT_H__
#define __CBASEOBJECT_H__

////////////////////////////////////////////////////////////////////////////
// ������Ʈ ���� ������ �ֻ��� ��ü
// 
// �ش� ������Ʈ�� �پ��ϰ� ����ϰ� �ִ�. 
// 
// �� �� ���� �浹ó���� �ϰ� �ʹٸ�, 
// 
// �浹 ������ ������Ʈ�� �������� ���� ������Ʈ�� �и��Ͽ� ��ӽ�Ų��.
// 
// 
// �Ǵ� CBaseObject���� Y,X,dY,dX ���� ��� �浹ó���� �Ѵ�.
// 
// �Ǵ� �浹ó�� �Լ��� virtural�� ����Ͽ� �����, �ڽ� Ŭ�������� �ѱ��.
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
