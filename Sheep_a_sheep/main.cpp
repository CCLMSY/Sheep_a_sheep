#include<iostream>
#include<graphics.h>
#include<vector>
#include<string>
#include<time.h>
using namespace std;

typedef struct { int x, y; } Pos;

const auto MAX_LEVEL = 10;//�ؿ�����
const auto GROUP_PER_LEVEL = 8;//ÿ����������

const auto WINDOW_WIDTH = 600;
const auto WINDOW_HEIGHT = 800;//���ڴ�С
const auto BLOCK_WIDTH = 50;//�����С
const auto BLOCK_KINDS = 10;//��������
int cntGroup = 0;//����
int Level = 1;//�ؿ�

/*���沼��
�ȼ����		(50,50):(550,100)
��������		(50,150):(550,650)
���������	(50,700):(550,750)
*/

IMAGE imgBg;//����ͼ
IMAGE imgWin,imgLose;//ʤ��ͼ��ʧ��ͼ
IMAGE imgClearance;//ͨ��ͼ
IMAGE imgBlock[BLOCK_KINDS+1];//����ͼƬ
IMAGE imgGray[BLOCK_KINDS+1];//��ɫ����ͼƬ

vector<int> stack;//�����
vector<Pos> cardpos;//����λ��
vector<int> map;//�������ࣺ0������-���ڣ�+ͼ��

//����ʼʱ
void Load_Img();//����ͼƬ

//��Ϸ��ʼ��
void GameInit();
	void Reset();//������Ϸ
	void InitMap();//���䷽������
	void InitCardPositions();//�������λ��

//��Ϸ��
void Update();//ˢ�£����ƣ�����
int OnClick();//�������������Ч�������
bool IsCardCovered(int index);//�жϷ����Ƿ񱻸���
void Insert(int index);//�������
void Eliminate();//�������
bool IsWin();//���ʤ��

int GameLoop() {//ѭ������
	GameInit();
	Update();
	while (1) {
		int index = OnClick();
		if (index != -1) {
			Insert(index);
			Eliminate();
			if (stack.size() == 7) return 0; //ʧ��
			if (IsWin()) {
				Update();
				putimage(50, 280, &imgWin);
				Sleep(1000);
				Level++;
				if (Level > MAX_LEVEL) {
					return 1;
				}//ͨ��
				GameInit();
			}
			Update();
		}
	}
}
int main() {
	Load_Img();//����ͼƬ
	initgraph(WINDOW_WIDTH, WINDOW_HEIGHT);//��ʼ������
	setbkmode(TRANSPARENT);//���ֱ���ɫ͸��
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

void Load_Img() {//����ʼʱ ����ͼƬ
	loadimage(&imgBg, _T("src/Bg.png"));//���ر���
	loadimage(&imgClearance, _T("src/Clearance.png"));//����ͨ��ͼ
	loadimage(&imgWin, _T("src/Win.png"));//����ʤ��ͼ
	loadimage(&imgLose, _T("src/Lose.png"));//����ʧ��ͼ
	for (int i = 1; i <= BLOCK_KINDS; i++) {
		TCHAR path[256];
		_stprintf_s(path, _T("src/Block%d.png"), i);
		loadimage(&imgBlock[i], path);//���ط���
		_stprintf_s(path, _T("src/Gray%d.png"), i);
		loadimage(&imgGray[i], path);//���ػ�ɫ����
	}
}

void GameInit() {//��Ϸ��ʼ��
	Reset();
	cntGroup = Level * GROUP_PER_LEVEL;//����

	InitMap();
	InitCardPositions();
}

void Reset() {//������Ϸ
	stack.clear();
	cardpos.clear();
	map.clear();
}

void InitMap() {//���䷽������
	srand(time(0));
	int kind = min((cntGroup + 1) / 2, 10);
	//����������������һ������ȡ�������Ϊ10

	for (int i = 0; i < cntGroup; i++) {
		int type = 1 + i % kind;
		for (int j = 0; j < 3; j++)
			map.emplace_back(type);
	}//���ɷ�������

	int cnt = cntGroup * 3, t;
	for (int i = 0; i < cntGroup * 3; i++) {
		int index = rand() % cnt;
		t = map[i];
		map[i] = map[index];
		map[index] = t;
	}//������ҷ�������
}

void InitCardPositions() {//�������λ��
	srand(time(0));
	Pos temp{};
	for (int i = 0; i < cntGroup * 3; i++) {
		temp.x = 50 + rand() % (WINDOW_WIDTH - BLOCK_WIDTH - 50 - 50);
		temp.y = 150 + rand() % (WINDOW_HEIGHT - BLOCK_WIDTH - 250 - 50);
		cardpos.emplace_back(temp);
	}
}

void Update() {//ˢ�£����ƣ�����
	BeginBatchDraw();
	putimage(0, 0, &imgBg);//���ñ���ͼ

	TCHAR level[10];
	_stprintf_s(level, _T("%d"), Level);
	settextstyle(50, 0, _T("Consolas"));
	outtextxy(350, 43, level);//����ȼ�

	for (int i = 0; i < cntGroup * 3; i++) {
		if (map[i] <= 0) continue;
		if (IsCardCovered(i)) putimage(cardpos[i].x, cardpos[i].y, &imgGray[map[i]]);
		else putimage(cardpos[i].x, cardpos[i].y, &imgBlock[map[i]]);
	}//���÷���

	int cnt = stack.size();
	for (int i = 0; i < cnt; i++)
		putimage(95 + i * 60, 700 , &imgBlock[-map[stack[i]]]);//���÷����

	EndBatchDraw();
}

int OnClick() {//�������������Ч�������
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

bool IsCardCovered(int index) {//���жϷ����Ƿ񱻸���
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

void Insert(int index) {//�������
	vector<int> temp;
	for (auto i : stack) {
		temp.emplace_back(i);
		if (map[i] == -map[index]) { temp.emplace_back(index); map[index] *= -1; }
	}//ƥ�����з���
	if (stack.size() == temp.size()) { temp.emplace_back(index); map[index] *= -1; }
	stack = temp;
}

void Eliminate() {//�������
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


bool IsWin() {//���ʤ��
	for (auto i : map) if (i != 0) return false;
	return true;
}

