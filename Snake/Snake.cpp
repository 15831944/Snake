// Snake.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <windows.h>
#include <vector>
using namespace std;
#pragma comment(lib,"winmm.lib")

// 键盘按下
#define KEYDOWN(vk_code) ((GetAsyncKeyState(vk_code) & 0x8000)?1:0)


#define MAXROW 25
#define MAXCOL 25
#define SNAKE_BODY 2
#define CURSORY 17
enum {
	SPACE = 0,
	WALL,
};
//方向
enum 
{
	UP = 0,
	DOWN,
	LEFT,
	RIGHT,
};
//游戏状态
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
int iDir = RIGHT;//初始方向为右
int iCount = 0;//统计蛇一共吃了多少食物
bool isPause = false; //控制游戏是否暂停
int iSpeed = 500; // 控制蛇的速度
int iGameStatus = GAME_MENU; // 游戏状态
bool isInit = false; //控制游戏开始是否初始化
vector<COORD> vPos; //保存蛇的节点坐标
char szSign[3] = { 0 };//地图操作

//保存游戏的数据
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
	system("title 贪吃蛇");
	system("mode con cols=80 lines=30"); 
	//shift切换中文输入法
	keybd_event(VK_SHIFT, 0, 0, 0);
	Sleep(100);
	keybd_event(VK_SHIFT, 0, KEYEVENTF_KEYUP, 0);
	
	while (true)
	{
		//游戏菜单
		if (iGameStatus == GAME_MENU) {
			system("cls");
			OnRenderMenu();
			ControlMenu();
			Sleep(200);
		}
		//游戏开始
		else if (iGameStatus == GAME_PLAY || iGameStatus == GAME_READFILE) {
			//按下暂停
			if (KEYDOWN(VK_RETURN)) {
				isPause = true;
			}
			//取消暂停
			else if (KEYDOWN(VK_ESCAPE)) {
				isPause = false;
			}
			//保存游戏
			else if (KEYDOWN(VK_SPACE)) {
 				SaveGame();
			}
			if (!isPause) {
				if (isInit)
				{
					PlaySound("./sound/background.wav");
					//读取地图
					ReadMap();
					//渲染地图
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
					//释放内存
					releaseHeap();
				}
				Sleep(iSpeed);
			}
		}
		else if (iGameStatus == GAME_MAPEDIOR) {
			PlaySound("./sound/background.wav");
			//读取地图
			ReadMap();
			//渲染地图
			RenderMap();
			ShowOperateAndScore();
			DIYMap();
		}
		else if (iGameStatus == GAME_OVER) {
			OnRenderChar(5, 13, "游戏结束！按确认键回到菜单");
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

//初始化游戏
void InitGame() {
	//蛇头初始位置
	g_pHead = (snake*)malloc(sizeof(snake));
	int y = 5;
	g_pHead->x = 10;
	g_pHead->y = y;
	g_pHead->pNext = nullptr;
	//初始化两个蛇身
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
	//食物初始化
	g_pFood = (food*)malloc(sizeof(food));
	g_pFood->isEat = true;

	//初始化速度
	iSpeed = 500;
	//初始化方向
	iDir = RIGHT;
	//吃的食物归零
	iCount = 0;

	ShowOperateAndScore();
}

//随机食物 伪随机
void RandFood() {
	if (g_pFood->isEat) {
		//如果食物的坐标和墙重叠，重新随机
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
	OnRenderChar(g_pFood->x, g_pFood->y, "¤");
	EatFood();
}

//吃到食物之后给链表增加一个新节点
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

//蛇吃的食物越多，移动速度越快
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

//渲染地图
void RenderMap() {
	for (int i = 0; i < MAXROW; i++) {
		for (int j = 0; j < MAXCOL; j++) {
			if (MAP[i][j] == WALL) {
				OnRenderChar(i, j, "■");
			}
			else if (MAP[i][j] == SPACE) {
				OnRenderChar(i, j, "  ");
			}
		}
		printf("\n");
	}
}

//改变方向
void ChangeiDir() {
	//改变蛇的行走方向
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

//蛇身移动后面跟着前面的坐标
void MoveLogic() {
	snake *pEnd = g_pHead;

	int tmpX = 0;
	int tmpY = 0;

	int xx = 0;
	int yy = 0;

	//将每个蛇身节点坐标依次向后传递
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

//先清理蛇之后再改变蛇的坐标，再渲染蛇，就形成了蛇的移动
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

//清理蛇
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

//释放蛇和食物的内存
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

//渲染蛇
void RenderSnake() {
	snake *pEnd = g_pHead;

	while (pEnd)
	{
		OnRenderChar(pEnd->x, pEnd->y, "⊙");
		if (pEnd->pNext) {
			pEnd = pEnd->pNext;
		}
		else {
			pEnd = nullptr;
		}
	}
}

//蛇是否死亡判断有无撞墙或者撞到自己
bool IsSnakeDead() {
	//遍历蛇头有没有碰到墙
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
	//循环查找蛇头有没有碰到蛇身
	while (pEnd->pNext)
	{
		pEnd = pEnd->pNext;
		if (pEnd->x == headX && pEnd->y == headY) {
			return true;
		}
	}

	return false;
}

//渲染菜单
void OnRenderMenu() {

	OnRenderChar(5, CURSORY, "开始游戏");
	OnRenderChar(7, CURSORY, "编辑地图");
	OnRenderChar(9, CURSORY, "读取存档");
	OnRenderChar(11, CURSORY, "退出游戏");
}

//控制菜单选项
void ControlMenu() {
	//菜单箭头位置
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

//显示操作和分数
void ShowOperateAndScore() {
	//操作说明
	if (iGameStatus == GAME_PLAY || iGameStatus == GAME_READFILE) {
		OnRenderChar(11, 30, "空格键保存游戏");
		OnRenderChar(13, 30, "↑:控制向上移动");
		OnRenderChar(15, 30, "↓:控制向下移动");
		OnRenderChar(17, 30, "←:控制向左移动");
		OnRenderChar(19, 30, "→:控制向右移动");
		OnRenderChar(21, 30, "确认键暂停,ESC恢复");
		OnRenderChar(23, 30, "获得分数：");
		printf("%d", iCount * 10);
	}
	else if (iGameStatus == GAME_MAPEDIOR) {
		OnRenderChar(35, 1, "ESC保存地图并返回菜单");
		OnRenderChar(35, 25, "按下1键用鼠标编辑墙");
		OnRenderChar(36, 1, "按下2键用鼠标编辑空地");
		OnRenderChar(36, 25, "按空格键清空地图");
	}
}

// 读取地图文件
void ReadMap()
{
	char szTextName[64];
	sprintf_s(szTextName, "map.txt");
	FILE *pFileText;
	errno_t err;
	if ((err = fopen_s(&pFileText, szTextName, "r")) != 0)
	{
		printf("文件读取失败！");
		return;
	}
	fseek(pFileText, 0, SEEK_END); //把指针移动到文件的结尾 ，获取文件长度
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

// 保存自定义地图
void SaveMap()
{
	FILE *pFileText;
	errno_t err;
	if ((err = fopen_s(&pFileText, "map.txt", "w")) != 0)
	{
		printf("文件读取失败！");
		return;
	}
	fseek(pFileText, 0, SEEK_END); //把指针移动到文件的结尾 ，获取文件长度
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

// 更新地图编辑 
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
		if (KEYDOWN(VK_SPACE))															// 按下空格清空地图
			InitMap();
		else if (KEYDOWN(VK_ESCAPE))													// 按下回车保存文件,返回菜单模式.
		{
			SaveMap();																// 保存地图文件													// 菜单刷新开启
			iGameStatus = GAME_MENU;													// 状态切换
			break;
		}
		if (KEYDOWN('1'))																// 数字键1编辑砖块
			strcpy_s(szSign, 3, "■");
		else if (KEYDOWN('2'))															// 数字键3编辑空地
			strcpy_s(szSign, 3, "  ");
	}
}

//初始化地图，设置为空地
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

// 鼠标事件 
void MouseEventProc(MOUSE_EVENT_RECORD mer)
{
	switch (mer.dwEventFlags)
	{
	case 0:
	{
		if (mer.dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)								// 鼠标左键单击
		{
			if (mer.dwMousePosition.Y >= MAXROW || mer.dwMousePosition.X / 2 >= MAXCOL)
				return;
			OnRenderChar(mer.dwMousePosition.Y, mer.dwMousePosition.X / 2, szSign);
			if (strcmp(szSign, "■") == 0)
				MAP[mer.dwMousePosition.Y][mer.dwMousePosition.X / 2] = 1;
			else if (strcmp(szSign, "  ") == 0)
				MAP[mer.dwMousePosition.Y][mer.dwMousePosition.X / 2] = 0;
		}
	}
	break;
	case MOUSE_MOVED:
	{
		if (mer.dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)								// 拖动状态并且鼠标左键按下
		{
			if (mer.dwMousePosition.Y >= MAXROW || mer.dwMousePosition.X / 2 >= MAXCOL)
				return;
			OnRenderChar(mer.dwMousePosition.Y, mer.dwMousePosition.X / 2, szSign);
			if (strcmp(szSign, "■") == 0)
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

// 在地图上显示坐标 
void OutputPos(int nX, int nY)
{
	char szBuf[100] = { 0 };
	sprintf_s(szBuf, sizeof(szBuf), "x = %2d, y=%2d", nX / 2, nY);
	OnRenderChar(36, 30, szBuf);
}

//保存游戏的信息，然后写入文件
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
		printf("读取文件失败");
		return;
	}
	fwrite(&snakeInfo,sizeof(_snakeInfo),1,pFile);

	//vector保存的是指针，所以要将vector的元素一个个保存
	for (int i = 0; i < vPos.size(); i++)
	{
		fwrite(&vPos[i], sizeof(COORD), 1, pFile);
	}
	fclose(pFile);

	OnRenderChar(11, 30, "保存游戏成功..");
}

//读取之前保存的游戏信息
void ReadGame() {
	FILE *pFile = nullptr;
	errno_t err;
	err = fopen_s(&pFile, "info.i", "r");
	if (err != 0) {
		printf("读取文件失败");
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

	//读取蛇头位置
	g_pHead = (snake*)malloc(sizeof(snake));
	g_pHead->x = vPos[0].X;
	g_pHead->y = vPos[0].Y;
	g_pHead->pNext = nullptr;
	//读取蛇身位置
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
	//读取食物位置
	g_pFood = (food*)malloc(sizeof(food));
	g_pFood->isEat = snakeInfo.isEat;
	g_pFood->x = snakeInfo.fPos.X;
	g_pFood->y = snakeInfo.fPos.Y;

	//读取速度
	iSpeed = snakeInfo.iSpeed;
	//读取方向
	iDir = snakeInfo.iDir;
	//读取食物个数
	iCount = snakeInfo.iCount;

	ShowOperateAndScore();
}

//播放音乐
void PlaySound(const char *str) {
	PlaySoundA(str, NULL, SND_ASYNC | SND_NODEFAULT);
}

void OnRenderChar(int High, int Wide, char* pszChar)
{
	CONSOLE_CURSOR_INFO cci;		// 设置光标属性
	cci.dwSize = 1;
	cci.bVisible = FALSE;			// 是否显示光标
	SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cci);
	COORD loc = { Wide * 2, High };	// x的值是Wide的2倍
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), loc);
	printf(pszChar);			//打印时需要注意有些特殊字符是占两个字节
}

