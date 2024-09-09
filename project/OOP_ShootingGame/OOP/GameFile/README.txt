파일 구조

MaxStage.txt: 스테이지 크기
	스테이지 크기만큼 스테이지를 읽어들인다.


stageN.txt: 스테이지에 대한 정보
	

PlayetN.txt: 플레이어에 대한 정보
	Speed: (int)프레임에 한번 이동한다.
	CoolTime: (int)프레임에 한번 총알을 발사한다.
	BulletSpeed: 미구현


movingN.txt: 적의 움직임에 대한 정보
	MovingCoolN: (int)프레임 후에 N번째 움직임을 취한다. 
	MovingYN: Y축 상대좌표
	MovingXN: X축 상대좌표


bulletN.txt: 적 총알에 대한 움직임 정보
	SpeedN: 미구현