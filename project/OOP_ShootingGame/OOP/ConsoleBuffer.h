#ifndef __CONSOLEBUFFER_H__
#define __CONSOLEBUFFER_H__

#define dfSCREEN_WIDTH 81
#define dfSCREEN_HEIGHT 25

class ConsoleBuffer
{
private:
	static ConsoleBuffer _ConsoleBuffer;
	char ScreenBuffer[dfSCREEN_HEIGHT][dfSCREEN_WIDTH];

	HANDLE hConsole;

private:
	ConsoleBuffer();
	~ConsoleBuffer();



public:
	static ConsoleBuffer* GetInstance(void);

	void Buffer_Flip(void);
	void Buffer_Clear(void);
	void Sprite_Null(void);
	void Sprite_Background(unsigned char* pFileMemory);
	void Sprite_Center_String(int iY, const char* pSpriteString);
	void Sprite_Draw(int iY, int iX, char chSprite);

	// friend ½È¾î¼­ Æí¹ý
	void print_FPS(void);

private:
	//--------------------------------------------
	// 
	// 
	//--------------------------------------------
	void initial(void);

	void MoveCursor(int iPosY, int iPosX);

	void ClearScreen(void);

};



#endif // !__CONSOLEBUFFER_H__

