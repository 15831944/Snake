// Snake.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include <windows.h>
#include <vector>
using namespace std;
#pragma comment(lib,"winmm.lib")

// ���̰���
#define KEYDOWN(vk_code) ((GetAsyncKeyState(vk_code) & 0x8000)?1:0)


#define MAXROW 25
#define MAXCOL 25
#define SNAKE_BODY 2
#define CURSORY 17
enum {
	SPACE = 0,
	WALL,
};
//����
enum 
{
	UP = 0,
	DOWN,
	LEFT,
	RIGHT,
};
//��Ϸ״̬
enum 
{
	GAME_MENU = 0,
	GAME_PLAY,
	GAME_MAPEDIOR,
	GAME_READFILE,
	GAME_OVER,
	GAME_EXIT
	
};

struct snake
{
	int x;
	int y;
	snake *pNext;
};

struct food
{
	int x;
	int y;
	bool isEat;
};

int MAP[MAXROW][MAXCOL] = { 0 };

void InitGame();
void RenderMap();
void MoveSnake();
void ChangeiDir();
void RandFood();
void EatFood();
void MoveLogic();
void ClearSnake();
void RenderSnake();
bool IsSnakeDead();
void ControlSpeed();
void OnRenderMenu();
void ControlMenu();
void ShowOperateAndScore();
void ReadMap();
void SaveMap();
void releaseHeap();
void SaveGame();
void ReadGame();
void DIYMap();
void InitMap();
void PlaySound(const char *str);
void MouseEventProc(MOUSE_EVENT_RECORD mer);
void OutputPos(int nX, int nY);
void OnRenderChar(int High, int Wide, char* pszChar);

snake *g_pHead = nullptr;
food *g_pFood = nullptr;
int iDir = RIGHT;//��ʼ����Ϊ��
int iCount = 0;//ͳ����һ�����˶���ʳ��
bool isPause = false; //������Ϸ�Ƿ���ͣ
int iSpeed = 500; // �����ߵ��ٶ�
int iGameStatus = GAME_MENU; // ��Ϸ״̬
bool isInit = false; //������Ϸ��ʼ�Ƿ��ʼ��
vector<COORD> vPos; //�����ߵĽڵ�����
char szSign[3] = { 0 };//��ͼ����

//������Ϸ������
struct _snakeInfo
{
	int iDir;
	int iCount;
	int iSpeed;
	COORD fPos;
	bool isEat;
	int iSnakeCount;
}snakeInfo;

int main()
{
	system("title ̰����");
	system("mode con cols=80 lines=30"); 
	//shift�л��������뷨
	keybd_event(VK_SHIFT, 0, 0, 0);
	Sleep(100);
	keybd_event(VK_SHIFT, 0, KEYEVENTF_KEYUP, 0);
	
	while (true)
	{
		//��Ϸ�˵�
		if (iGameStatus == GAME_MENU) {
			system("cls");
			OnRenderMenu();
			ControlMenu();
			Sleep(200);
		}
		//��Ϸ��ʼ
		else if (iGameStatus == GAME_PLAY || iGameStatus == GAME_READFILE) {
			//������ͣ
			if (KEYDOWN(VK_RETURN)) {
				isPause = true;
			}
			//ȡ����ͣ
			else if (KEYDOWN(VK_ESCAPE)) {
				isPause = false;
			}
			//������Ϸ
			else if (KEYDOWN(VK_SPACE)) {
 				SaveGame();
			}
			if (!isPause) {
				if (isInit)
				{
					PlaySound("./sound/background.wav");
					//��ȡ��ͼ
					ReadMap();
					//��Ⱦ��ͼ
					RenderMap();
					if (iGameStatus == GAME_READFILE) {
						ReadGame();
					}
					else
					{
						InitGame();
					}
					isInit = false;
				}
				RandFood();
				MoveSnake();
				if (IsSnakeDead()) {
					PlaySound("./sound/die.wav");
					iGameStatus = GAME_OVER;
					system("cls");
					//�ͷ��ڴ�
					releaseHeap();
				}
				Sleep(iSpeed);
			}
		}
		else if (iGameStatus == GAME_MAPEDIOR) {
			PlaySound("./sound/background.wav");
			//��ȡ��ͼ
			ReadMap();
			//��Ⱦ��ͼ
			RenderMap();
			ShowOperateAndScore();
			DIYMap();
		}
		else if (iGameStatus == GAME_OVER) {
			OnRenderChar(5, 13, "��Ϸ��������ȷ�ϼ��ص��˵�");
			if (KEYDOWN(VK_RETURN))
			{
				iGameStatus = GAME_MENU;
				system("cls");
			}
			Sleep(200);
		}
		else if (iGameStatus == GAME_EXIT) {
			break;
		}
	}
	
    return 0;
}

//��ʼ����Ϸ
void InitGame() {
	//��ͷ��ʼλ��
	g_pHead = (snake*)malloc(sizeof(snake));
	int y = 5;
	g_pHead->x = 10;
	g_pHead->y = y;
	g_pHead->pNext = nullptr;
	//��ʼ����������
	snake *pEnd = g_pHead;
	int i = 0;
	while (i < SNAKE_BODY)
	{
		if (NULL == pEnd->pNext) {
			snake* pNode = (snake*)malloc(sizeof(snake));
			pNode->x = 10;
			--y;
			pNode->y = y;
			pNode->pNext = nullptr;
			pEnd->pNext = pNode;
			pEnd = pEnd->pNext;
			++i;
		}
	}
	//ʳ���ʼ��
	g_pFood = (food*)malloc(sizeof(food));
	g_pFood->isEat = true;

	//��ʼ���ٶ�
	iSpeed = 500;
	//��ʼ������
	iDir = RIGHT;
	//�Ե�ʳ�����
	iCount = 0;

	ShowOperateAndScore();
}

//���ʳ�� α���
void RandFood() {
	if (g_pFood->isEat) {
		//���ʳ��������ǽ�ص����������
		do 
		{
			int randX = rand() % 23 + 1;
			int randY = rand() % 23 + 1;
			if (MAP[randX][randY] != WALL) {
				g_pFood->x = randX;
				g_pFood->y = randY;
				g_pFood->isEat = false;
				break;
			}
		} while (true);
	}
	OnRenderChar(g_pFood->x, g_pFood->y, "��");
	EatFood();
}

//�Ե�ʳ��֮�����������һ���½ڵ�
void EatFood() {
	if (g_pHead->x == g_pFood->x && g_pHead->y == g_pFood->y) {
		iCount++;
		g_pFood->isEat = true;
		snake *pEnd = g_pHead;
		while (pEnd->pNext != nullptr)
		{
			pEnd = pEnd->pNext;
		}
		snake *pNew = (snake*)malloc(sizeof(snake));
		pNew->pNext = nullptr;
		pEnd->pNext = pNew;

		ShowOperateAndScore();
		ControlSpeed();
	}
}

//�߳Ե�ʳ��Խ�࣬�ƶ��ٶ�Խ��
void ControlSpeed() {
	switch (iCount)
	{
	case 1:
		iSpeed = 200;
		break;
	case 2:
		iSpeed = 150;
		break;
	case 3:
		iSpeed = 100;
		break;
	case 4:
		iSpeed = 50;
		break;
	default:
		break;
	}
}

//��Ⱦ��ͼ
void RenderMap() {
	for (int i = 0; i < MAXROW; i++) {
		for (int j = 0; j < MAXCOL; j++) {
			if (MAP[i][j] == WALL) {
				OnRenderChar(i, j, "��");
			}
			else if (MAP[i][j] == SPACE) {
				OnRenderChar(i, j, "  ");
			}
		}
		printf("\n");
	}
}

//�ı䷽��
void ChangeiDir() {
	//�ı��ߵ����߷���
	if (KEYDOWN(VK_UP)) {
		if (iDir != DOWN)
			iDir = UP;
	}
	else if (KEYDOWN(VK_DOWN)) {
		if (iDir != UP)
			iDir = DOWN;
	}
	else if (KEYDOWN(VK_LEFT)) {
		if (iDir != RIGHT)
			iDir = LEFT;
	}
	else if (KEYDOWN(VK_RIGHT)) {
		if (iDir != LEFT)
			iDir = RIGHT;
	}
	
}

//�����ƶ��������ǰ�������
void MoveLogic() {
	snake *pEnd = g_pHead;

	int tmpX = 0;
	int tmpY = 0;

	int xx = 0;
	int yy = 0;

	//��ÿ������ڵ�����������󴫵�
	while (pEnd)
	{
		if (pEnd == g_pHead) {
			tmpX = pEnd->x;
			tmpY = pEnd->y;
		}
		else {
			tmpX = xx;
			tmpY = yy;
		}

		if (pEnd->pNext)
		{
			pEnd = pEnd->pNext;
			xx = pEnd->x;
			yy = pEnd->y;
			pEnd->x = tmpX;
			pEnd->y = tmpY;
		}
		else {
			pEnd = nullptr;
		}
	}
}

//��������֮���ٸı��ߵ����꣬����Ⱦ�ߣ����γ����ߵ��ƶ�
void MoveSnake() { 
	ClearSnake();
	ChangeiDir();
	switch (iDir)
	{
	case UP:
		MoveLogic();
		--g_pHead->x;
		break;
	case DOWN:
		MoveLogic();
		++g_pHead->x;
		break;
	case LEFT:
		MoveLogic();
		--g_pHead->y;
		break;
	case RIGHT:
		MoveLogic();
		++g_pHead->y;
		break;
	default:
		break;
	}
	
	RenderSnake();
}

//������
void ClearSnake() {
	snake *pEnd = g_pHead;

	while (pEnd)
	{
		OnRenderChar(pEnd->x, pEnd->y, " ");
		if (pEnd->pNext) {
			pEnd = pEnd->pNext;
		}
		else {
			pEnd = nullptr;
		}
	}
}

//�ͷ��ߺ�ʳ����ڴ�
void releaseHeap() {
	snake *pEnd = g_pHead;
	snake *pTemp = nullptr;
	while (pEnd)
	{
		OnRenderChar(pEnd->x, pEnd->y, " ");
		if (pEnd->pNext) {
			pTemp = pEnd->pNext;
			free(pEnd);
			pEnd = nullptr;
			pEnd = pTemp;
		}
		else {
			pEnd = nullptr;
		}
	}
	g_pHead = nullptr;
	free(g_pFood);
	g_pFood = nullptr;
}

//��Ⱦ��
void RenderSnake() {
	snake *pEnd = g_pHead;

	while (pEnd)
	{
		OnRenderChar(pEnd->x, pEnd->y, "��");
		if (pEnd->pNext) {
			pEnd = pEnd->pNext;
		}
		else {
			pEnd = nullptr;
		}
	}
}

//���Ƿ������ж�����ײǽ����ײ���Լ�
bool IsSnakeDead() {
	//������ͷ��û������ǽ
	for (int i = 0; i < MAXROW; i++) {
		for (int j = 0; j < MAXCOL; j++) {
			if (MAP[i][j] == WALL && g_pHead->x == i && g_pHead->y == j) {
				return true;
			}
		}
	}

	int headX = g_pHead->x;
	int headY = g_pHead->y;
	snake *pEnd = g_pHead;
	//ѭ��������ͷ��û����������
	while (pEnd->pNext)
	{
		pEnd = pEnd->pNext;
		if (pEnd->x == headX && pEnd->y == headY) {
			return true;
		}
	}

	return false;
}

//��Ⱦ�˵�
void OnRenderMenu() {

	OnRenderChar(5, CURSORY, "��ʼ��Ϸ");
	OnRenderChar(7, CURSORY, "�༭��ͼ");
	OnRenderChar(9, CURSORY, "��ȡ�浵");
	OnRenderChar(11, CURSORY, "�˳���Ϸ");
}

//���Ʋ˵�ѡ��
void ControlMenu() {
	//�˵���ͷλ��
	static int iCursorX = 5;
	OnRenderChar(iCursorX, CURSORY - 1, "->");
	if (KEYDOWN(VK_UP)) {
		OnRenderChar(iCursorX, CURSORY - 1, "  ");
		iCursorX -= 2;
		if (iCursorX < 5)
		{
			iCursorX = 11;
		}
		OnRenderChar(iCursorX, CURSORY - 1, "->");
	}
	else if (KEYDOWN(VK_DOWN)) {
		OnRenderChar(iCursorX, CURSORY - 1, "  ");
		iCursorX += 2;
		if (iCursorX > 11)
		{
			iCursorX = 5;
		}
		OnRenderChar(iCursorX, CURSORY - 1, "->");
	}
	else if (KEYDOWN(VK_RETURN)) {
		if (iCursorX == 5) {
			iGameStatus = GAME_PLAY;
		}
		else if (iCursorX == 7) {
			iGameStatus = GAME_MAPEDIOR;
		}
		else if (iCursorX == 9) {
			iGameStatus = GAME_READFILE;
		}
		else if (iCursorX == 11) {
			iGameStatus = GAME_EXIT;
		}

		system("cls");
		isInit = true;
	}
}

//��ʾ�����ͷ���
void ShowOperateAndScore() {
	//����˵��
	if (iGameStatus == GAME_PLAY || iGameStatus == GAME_READFILE) {
		OnRenderChar(11, 30, "�ո��������Ϸ");
		OnRenderChar(13, 30, "��:���������ƶ�");
		OnRenderChar(15, 30, "��:���������ƶ�");
		OnRenderChar(17, 30, "��:���������ƶ�");
		OnRenderChar(19, 30, "��:���������ƶ�");
		OnRenderChar(21, 30, "ȷ�ϼ���ͣ,ESC�ָ�");
		OnRenderChar(23, 30, "��÷�����");
		printf("%d", iCount * 10);
	}
	else if (iGameStatus == GAME_MAPEDIOR) {
		OnRenderChar(35, 1, "ESC�����ͼ�����ز˵�");
		OnRenderChar(35, 25, "����1�������༭ǽ");
		OnRenderChar(36, 1, "����2�������༭�յ�");
		OnRenderChar(36, 25, "���ո����յ�ͼ");
	}
}

// ��ȡ��ͼ�ļ�
void ReadMap()
{
	char szTextName[64];
	sprintf_s(szTextName, "map.txt");
	FILE *pFileText;
	errno_t err;
	if ((err = fopen_s(&pFileText, szTextName, "r")) != 0)
	{
		printf("�ļ���ȡʧ�ܣ�");
		return;
	}
	fseek(pFileText, 0, SEEK_END); //��ָ���ƶ����ļ��Ľ�β ����ȡ�ļ�����
	int len = ftell(pFileText);
	fseek(pFileText, 0, SEEK_SET);
	for (int i = 0; i < MAXROW; i++)
	{
		for (int j = 0; j < MAXCOL; j++) {
			fscanf_s(pFileText, "%d ", &MAP[i][j]);
			int i = ftell(pFileText);
		}
			
	}
	fclose(pFileText);
}

// �����Զ����ͼ
void SaveMap()
{
	FILE *pFileText;
	errno_t err;
	if ((err = fopen_s(&pFileText, "map.txt", "w")) != 0)
	{
		printf("�ļ���ȡʧ�ܣ�");
		return;
	}
	fseek(pFileText, 0, SEEK_END); //��ָ���ƶ����ļ��Ľ�β ����ȡ�ļ�����
	int len = ftell(pFileText);
	fseek(pFileText, 0, SEEK_SET);
	for (int i = 0; i < MAXROW; i++)
	{
		for (int j = 0; j < MAXCOL; j++)
		{
			fprintf(pFileText, "%d ", MAP[i][j]);
		}
		fprintf(pFileText, "\n");
	}
	fclose(pFileText);
}

// ���µ�ͼ�༭ 
void DIYMap()
{
	HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
	INPUT_RECORD stcRecord = { 0 };
	DWORD dwRead;
	SetConsoleMode(hStdin, ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT);
	while (true)
	{
		ReadConsoleInput(hStdin, &stcRecord, 1, &dwRead);
		if (stcRecord.EventType == MOUSE_EVENT)
			MouseEventProc(stcRecord.Event.MouseEvent);
		if (KEYDOWN(VK_SPACE))															// ���¿ո���յ�ͼ
			InitMap();
		else if (KEYDOWN(VK_ESCAPE))													// ���»س������ļ�,���ز˵�ģʽ.
		{
			SaveMap();																// �����ͼ�ļ�													// �˵�ˢ�¿���
			iGameStatus = GAME_MENU;													// ״̬�л�
			break;
		}
		if (KEYDOWN('1'))																// ���ּ�1�༭ש��
			strcpy_s(szSign, 3, "��");
		else if (KEYDOWN('2'))															// ���ּ�3�༭�յ�
			strcpy_s(szSign, 3, "  ");
	}
}

//��ʼ����ͼ������Ϊ�յ�
void InitMap() {
	for (int i = 0; i < MAXROW; i++)
	{
		for (int j = 0; j < MAXCOL; j++)
		{
			MAP[i][0] = WALL;
			MAP[0][j] = WALL;
			MAP[MAXROW - 1][j] = WALL;
			MAP[i][MAXCOL - 1] = WALL;
			if (i >= 1 && i < MAXROW - 1 && j >= 1 && j < MAXCOL - 1)
				MAP[i][j] = 0;
		}
	}
	RenderMap();
}

// ����¼� 
void MouseEventProc(MOUSE_EVENT_RECORD mer)
{
	switch (mer.dwEventFlags)
	{
	case 0:
	{
		if (mer.dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)								// ����������
		{
			if (mer.dwMousePosition.Y >= MAXROW || mer.dwMousePosition.X / 2 >= MAXCOL)
				return;
			OnRenderChar(mer.dwMousePosition.Y, mer.dwMousePosition.X / 2, szSign);
			if (strcmp(szSign, "��") == 0)
				MAP[mer.dwMousePosition.Y][mer.dwMousePosition.X / 2] = 1;
			else if (strcmp(szSign, "  ") == 0)
				MAP[mer.dwMousePosition.Y][mer.dwMousePosition.X / 2] = 0;
		}
	}
	break;
	case MOUSE_MOVED:
	{
		if (mer.dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)								// �϶�״̬��������������
		{
			if (mer.dwMousePosition.Y >= MAXROW || mer.dwMousePosition.X / 2 >= MAXCOL)
				return;
			OnRenderChar(mer.dwMousePosition.Y, mer.dwMousePosition.X / 2, szSign);
			if (strcmp(szSign, "��") == 0)
				MAP[mer.dwMousePosition.Y][mer.dwMousePosition.X / 2] = 1;
			else if (strcmp(szSign, "  ") == 0)
				MAP[mer.dwMousePosition.Y][mer.dwMousePosition.X / 2] = 0;
		}
		//OutputPos(mer.dwMousePosition.X, mer.dwMousePosition.Y);
	}
	default:
		break;
	}
}

// �ڵ�ͼ����ʾ���� 
void OutputPos(int nX, int nY)
{
	char szBuf[100] = { 0 };
	sprintf_s(szBuf, sizeof(szBuf), "x = %2d, y=%2d", nX / 2, nY);
	OnRenderChar(36, 30, szBuf);
}

//������Ϸ����Ϣ��Ȼ��д���ļ�
void SaveGame() {
	snakeInfo.iDir = iDir;
	snakeInfo.iCount = iCount;
	snakeInfo.iSpeed = iSpeed;
	snakeInfo.fPos.X = g_pFood->x;
  	snakeInfo.fPos.Y = g_pFood->y;
	snakeInfo.isEat = g_pFood->isEat;
	vPos.clear();
	snake *pEnd = g_pHead;
	COORD t;
	while (pEnd)
	{
		t.X = pEnd->x;
		t.Y = pEnd->y;
		vPos.push_back(t);
		if (pEnd->pNext) {
			pEnd = pEnd->pNext;
		}
		else {
			pEnd = nullptr;
		}
	}
	snakeInfo.iSnakeCount = vPos.size();
	FILE *pFile = nullptr;
	errno_t err;
	err = fopen_s(&pFile, "info.i", "w");
	if (err != 0) {
		printf("��ȡ�ļ�ʧ��");
		return;
	}
	fwrite(&snakeInfo,sizeof(_snakeInfo),1,pFile);

	//vector�������ָ�룬����Ҫ��vector��Ԫ��һ��������
	for (int i = 0; i < vPos.size(); i++)
	{
		fwrite(&vPos[i], sizeof(COORD), 1, pFile);
	}
	fclose(pFile);

	OnRenderChar(11, 30, "������Ϸ�ɹ�..");
}

//��ȡ֮ǰ�������Ϸ��Ϣ
void ReadGame() {
	FILE *pFile = nullptr;
	errno_t err;
	err = fopen_s(&pFile, "info.i", "r");
	if (err != 0) {
		printf("��ȡ�ļ�ʧ��");
		return;
	}
	fread(&snakeInfo, sizeof(_snakeInfo), 1, pFile);	
	vPos.clear();
	COORD t;
	for (int i = 0; i < snakeInfo.iSnakeCount; i++)
	{
		fread(&t, sizeof(COORD), 1, pFile);
		vPos.push_back(t);
	}

	fclose(pFile);

	//��ȡ��ͷλ��
	g_pHead = (snake*)malloc(sizeof(snake));
	g_pHead->x = vPos[0].X;
	g_pHead->y = vPos[0].Y;
	g_pHead->pNext = nullptr;
	//��ȡ����λ��
	snake *pEnd = g_pHead;
	int i = 1;
	while (i < vPos.size())
	{
		if (NULL == pEnd->pNext) {
			snake* pNode = (snake*)malloc(sizeof(snake));
			pNode->x = vPos[i].X;
			pNode->y = vPos[i].Y;
			pNode->pNext = nullptr;
			pEnd->pNext = pNode;
			pEnd = pEnd->pNext;
			++i;
		}
	}
	//��ȡʳ��λ��
	g_pFood = (food*)malloc(sizeof(food));
	g_pFood->isEat = snakeInfo.isEat;
	g_pFood->x = snakeInfo.fPos.X;
	g_pFood->y = snakeInfo.fPos.Y;

	//��ȡ�ٶ�
	iSpeed = snakeInfo.iSpeed;
	//��ȡ����
	iDir = snakeInfo.iDir;
	//��ȡʳ�����
	iCount = snakeInfo.iCount;

	ShowOperateAndScore();
}

//��������
void PlaySound(const char *str) {
	PlaySoundA(str, NULL, SND_ASYNC | SND_NODEFAULT);
}

void OnRenderChar(int High, int Wide, char* pszChar)
{
	CONSOLE_CURSOR_INFO cci;		// ���ù������
	cci.dwSize = 1;
	cci.bVisible = FALSE;			// �Ƿ���ʾ���
	SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cci);
	COORD loc = { Wide * 2, High };	// x��ֵ��Wide��2��
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), loc);
	printf(pszChar);			//��ӡʱ��Ҫע����Щ�����ַ���ռ�����ֽ�
}

