#include<iostream>
#include<graphics.h>
#include<vector>
#include<string>
#include<time.h>
using namespace std;

typedef struct { int x, y; } Pos;

const auto MAX_LEVEL = 10;//关卡上限
const auto GROUP_PER_LEVEL = 8;//每关增加组数

const auto WINDOW_WIDTH = 600;
const auto WINDOW_HEIGHT = 800;//窗口大小
const auto BLOCK_WIDTH = 50;//方块大小
const auto BLOCK_KINDS = 10;//方块种类
int cntGroup = 0;//组数
int Level = 1;//关卡

/*画面布局
等级面板		(50,50):(550,100)
方块区域		(50,150):(550,650)
方块槽区域	(50,700):(550,750)
*/

IMAGE imgBg;//背景图
IMAGE imgWin,imgLose;//胜利图，失败图
IMAGE imgClearance;//通关图
IMAGE imgBlock[BLOCK_KINDS+1];//方块图片
IMAGE imgGray[BLOCK_KINDS+1];//灰色方块图片

vector<int> stack;//方块槽
vector<Pos> cardpos;//方块位置
vector<int> map;//方块种类：0消除，-槽内，+图内

//程序开始时
void Load_Img();//载入图片

//游戏初始化
void GameInit();
	void Reset();//重置游戏
	void InitMap();//分配方块种类
	void InitCardPositions();//随机方块位置

//游戏中
void Update();//刷新（绘制）画面
int OnClick();//检测点击，返回有效方块序号
bool IsCardCovered(int index);//判断方块是否被覆盖
void Insert(int index);//方块入槽
void Eliminate();//检测消除
bool IsWin();//检测胜利

int GameLoop() {//循环主体
	GameInit();
	Update();
	while (1) {
		int index = OnClick();
		if (index != -1) {
			Insert(index);
			Eliminate();
			if (stack.size() == 7) return 0; //失败
			if (IsWin()) {
				Update();
				putimage(50, 280, &imgWin);
				Sleep(1000);
				Level++;
				if (Level > MAX_LEVEL) {
					return 1;
				}//通关
				GameInit();
			}
			Update();
		}
	}
}
int main() {
	Load_Img();//载入图片
	initgraph(WINDOW_WIDTH, WINDOW_HEIGHT);//初始化窗口
	setbkmode(TRANSPARENT);//文字背景色透明
	if (GameLoop()) {
		putimage(0, 0, &imgClearance);
		Sleep(5000);
	}
	else {
		Update();
		putimage(50, 280, &imgLose);
		Sleep(50000);
	}
	return 0;
}

void Load_Img() {//程序开始时 载入图片
	loadimage(&imgBg, _T("src/Bg.png"));//加载背景
	loadimage(&imgClearance, _T("src/Clearance.png"));//加载通关图
	loadimage(&imgWin, _T("src/Win.png"));//加载胜利图
	loadimage(&imgLose, _T("src/Lose.png"));//加载失败图
	for (int i = 1; i <= BLOCK_KINDS; i++) {
		TCHAR path[256];
		_stprintf_s(path, _T("src/Block%d.png"), i);
		loadimage(&imgBlock[i], path);//加载方块
		_stprintf_s(path, _T("src/Gray%d.png"), i);
		loadimage(&imgGray[i], path);//加载灰色方块
	}
}

void GameInit() {//游戏初始化
	Reset();
	cntGroup = Level * GROUP_PER_LEVEL;//组数

	InitMap();
	InitCardPositions();
}

void Reset() {//重置游戏
	stack.clear();
	cardpos.clear();
	map.clear();
}

void InitMap() {//分配方块种类
	srand(time(0));
	int kind = min((cntGroup + 1) / 2, 10);
	//方块种类数：组数一半向上取整，最大为10

	for (int i = 0; i < cntGroup; i++) {
		int type = 1 + i % kind;
		for (int j = 0; j < 3; j++)
			map.emplace_back(type);
	}//生成方块种类

	int cnt = cntGroup * 3, t;
	for (int i = 0; i < cntGroup * 3; i++) {
		int index = rand() % cnt;
		t = map[i];
		map[i] = map[index];
		map[index] = t;
	}//随机打乱方块种类
}

void InitCardPositions() {//随机方块位置
	srand(time(0));
	Pos temp{};
	for (int i = 0; i < cntGroup * 3; i++) {
		temp.x = 50 + rand() % (WINDOW_WIDTH - BLOCK_WIDTH - 50 - 50);
		temp.y = 150 + rand() % (WINDOW_HEIGHT - BLOCK_WIDTH - 250 - 50);
		cardpos.emplace_back(temp);
	}
}

void Update() {//刷新（绘制）画面
	BeginBatchDraw();
	putimage(0, 0, &imgBg);//放置背景图

	TCHAR level[10];
	_stprintf_s(level, _T("%d"), Level);
	settextstyle(50, 0, _T("Consolas"));
	outtextxy(350, 43, level);//输出等级

	for (int i = 0; i < cntGroup * 3; i++) {
		if (map[i] <= 0) continue;
		if (IsCardCovered(i)) putimage(cardpos[i].x, cardpos[i].y, &imgGray[map[i]]);
		else putimage(cardpos[i].x, cardpos[i].y, &imgBlock[map[i]]);
	}//放置方块

	int cnt = stack.size();
	for (int i = 0; i < cnt; i++)
		putimage(95 + i * 60, 700 , &imgBlock[-map[stack[i]]]);//放置方块槽

	EndBatchDraw();
}

int OnClick() {//检测点击，返回有效方块序号
	ExMessage msg;
	if (peekmessage(&msg) && msg.message == WM_LBUTTONDOWN) {
		for (int i = 0; i < cntGroup * 3; i++) if (map[i] > 0 && !IsCardCovered(i)) {
			if (msg.x >= cardpos[i].x && msg.x < cardpos[i].x + BLOCK_WIDTH &&
				msg.y >= cardpos[i].y && msg.y < cardpos[i].y + BLOCK_WIDTH) {
				return i;
			}
		}
	}return -1;
}

bool IsCardCovered(int index) {//检判断方块是否被覆盖
	for (int i = index + 1; i < cntGroup * 3; i++) {
		if (map[i] > 0) {
			if (cardpos[index].x + BLOCK_WIDTH > cardpos[i].x &&
				cardpos[index].x < cardpos[i].x + BLOCK_WIDTH &&
				cardpos[index].y + BLOCK_WIDTH > cardpos[i].y &&
				cardpos[index].y < cardpos[i].y + BLOCK_WIDTH
			) return true;
		}
	}
	return false;
}

void Insert(int index) {//方块入槽
	vector<int> temp;
	for (auto i : stack) {
		temp.emplace_back(i);
		if (map[i] == -map[index]) { temp.emplace_back(index); map[index] *= -1; }
	}//匹配已有方块
	if (stack.size() == temp.size()) { temp.emplace_back(index); map[index] *= -1; }
	stack = temp;
}

void Eliminate() {//检测消除
	int cnt[BLOCK_KINDS + 1] = { 0 };
	for (auto i : stack) cnt[-map[i]]++;
	for (int i = 1; i <= BLOCK_KINDS; i++) {
		if (cnt[i] >= 3) {
			vector<int> temp;
			for (auto j : stack)
				if (-map[j] != i) temp.emplace_back(j);
				else map[j] = 0;
			stack = temp;
			break;
		}
	}
}


bool IsWin() {//检测胜利
	for (auto i : map) if (i != 0) return false;
	return true;
}

