//#include <windows.h>
#include "stdafx.h"

#include "resource.h"



LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK SlotProc(HWND, UINT, WPARAM, LPARAM);
HINSTANCE g_hInst;
LPCWSTR lpszClass = L"first";

RECT rt = { 0, 0, 280, 340 };

Replay replay;
HWND	slotHwnd;

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance
	, LPSTR lpszCmdParam, int nCmdShow)
{
	HWND hWnd;
	MSG Message;
	WNDCLASS WndClass;
	g_hInst = hInstance;

	AdjustWindowRect(&rt, WS_OVERLAPPEDWINDOW, false);

	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	WndClass.hInstance = hInstance;
	WndClass.lpfnWndProc = (WNDPROC)WndProc;
	WndClass.lpszClassName = lpszClass;
	WndClass.lpszMenuName = MAKEINTRESOURCE(IDC_STLREPLAYPROJECT);
	WndClass.style = CS_HREDRAW | CS_VREDRAW;
	RegisterClass(&WndClass);



	hWnd = CreateWindow(lpszClass, lpszClass, WS_BORDER | WS_SYSMENU,
		CW_USEDEFAULT, CW_USEDEFAULT, rt.right - rt.left, rt.bottom - rt.top,
		NULL, (HMENU)NULL, hInstance, NULL);
	ShowWindow(hWnd, nCmdShow);

	while (GetMessage(&Message, 0, 0, 0)) {

		TranslateMessage(&Message);
		DispatchMessage(&Message);

	}
	return Message.wParam;
}
#define up 1
#define down 0
struct grid
{
	int statue;
	int num;
	bool mine;
	bool flag;
	bool blank;
	bool error;
	bool temp;
	bool boom;
	bool mouse;
};
int x[8] = { -1, 0, 1, -1, 1, -1, 0, 1 };
int y[8] = { -1, -1, -1, 0, 0, 1, 1, 1 };
int Xnum, Ynum, mine;
grid map[30][16];

enum gameType { nowPlay = 0, nowReplay, checkScene };

static auto tp = chrono::system_clock::now();
static time_t start = chrono::system_clock::to_time_t(tp);
static time_t now;

struct nowscene
{
public:
	time_t time;
	int now_mine;
	grid map[30][16];
};
vector<nowscene> snapShot;

//게임 초기화 관련
void CreateMap()
{
	int mine_num = mine;
	//srand(time(NULL));
	while (1)
	{
		for (int i = 0; i<Ynum; i++) {
			for (int j = 0; j<Xnum; j++)
			{
				if (map[i][j].mine == false && rand() % 6 == 2) {
					map[i][j].mine = true;
					replay.saveMine(j, i);
					mine_num--;
					if (mine_num == 0)
						break;
				}
				if (mine_num == 0)
					break;
			}
			if (mine_num == 0)
				break;
		}
		if (mine_num == 0)
			break;
	}
	for (int i = 0; i<Ynum; i++)
	{
		for (int j = 0; j<Xnum; j++)
		{
			for (int q = 0; q<8; q++)
			{
				if (i + y[q] >= 0 && i + y[q] <= Ynum && j + x[q] >= 0 && j + x[q] <= Xnum)
					if (map[i][j].mine == false && map[i + y[q]][j + x[q]].mine == true)
						map[i][j].num++;

			}
			if (map[i][j].num == 0 && map[i][j].mine == false)
				map[i][j].blank = true;
		}
	}
}
void CheckBlank(int i, int j)
{

	for (int p = -1; p <= 1; p++)
	{
		for (int q = -1; q <= 1; q++)
		{
			if (i + p<0 || i + p >= Ynum || j + q<0 || j + q >= Xnum)
				continue;
			if (map[i + p][j + q].num != 0 && map[i + p][j + q].statue == up) {
				map[i + p][j + q].statue = down;
				continue;
			}
			else if (map[i + p][j + q].statue == up && map[i + p][j + q].blank == true) {
				map[i + p][j + q].statue = down;
				CheckBlank(i + p, j + q);
			}

		}
	}
}
void gameover(HWND hwnd, int select, int type)
{
	KillTimer(hwnd, 2);
	KillTimer(hwnd, 3);
	for (int i = 0; i<Ynum; i++) {
		for (int j = 0; j<Xnum; j++) {
			if (map[i][j].mine == false && map[i][j].flag == true)
				map[i][j].error = true;
			map[i][j].statue = down;
		}
	}
	switch (select)
	{
	case 1:
		if (type == gameType::nowReplay) {
			MessageBox(hwnd, L"패배\n지뢰를 밟았습니다.", L"리플레이 종료", MB_OK);
			return;
		}
		if (MessageBox(hwnd, L"리플레이를\n저장하시겠습니까?", L"패배 - 지뢰를 밟았습니다.", MB_YESNO) == IDYES) {
			if (!IsWindow(slotHwnd)) {
				slotHwnd = CreateDialog(g_hInst, MAKEINTRESOURCE(IDD_DIALOG1),
					hwnd, SlotProc);
				ShowWindow(slotHwnd, SW_SHOW);
			}
		}
		break;
	case 2:
		if (type == gameType::nowReplay) {
			MessageBox(hwnd, L"패배\n잘못된 곳에 지뢰를 선택", L"리플레이 종료", MB_OK);
			return;
		}
		if (MessageBox(hwnd, L"리플레이를\n저장하시겠습니까?", L"패배 - 잘못된 곳에 지뢰를 선택", MB_YESNO) == IDYES) {
			if (!IsWindow(slotHwnd)) {
				slotHwnd = CreateDialog(g_hInst, MAKEINTRESOURCE(IDD_DIALOG1),
					hwnd, SlotProc);
				ShowWindow(slotHwnd, SW_SHOW);
			}
		}
		break;
	}

}
void result(HWND hwnd, int type)
{
	int select = 0;
	KillTimer(hwnd, 2);
	KillTimer(hwnd, 3);
	for (int i = 0; i<Ynum; i++) {
		for (int j = 0; j<Xnum; j++) {
			if (map[i][j].flag == true && map[i][j].mine == false) {
				select = 1;
				break;
			}
			if (map[i][j].boom == true) {
				select = 2;
				break;
			}
		}
		if (select == 1 || select == 2)
			break;
	}

	switch (select)
	{
	case 1:
		if (type == gameType::nowReplay) {
			MessageBox(hwnd, L"패배\n잘못된 곳에 지뢰를 선택", L"리플레이 종료", MB_OK);
			return;
		}
		if (MessageBox(hwnd, L"리플레이를\n저장하시겠습니까?", L"패배 - 잘못된 곳에 지뢰를 선택", MB_YESNO) == IDYES) {
			if (!IsWindow(slotHwnd)) {
				slotHwnd = CreateDialog(g_hInst, MAKEINTRESOURCE(IDD_DIALOG1),
					hwnd, SlotProc);
				ShowWindow(slotHwnd, SW_SHOW);
			}
		}
		break;
	case 0:
		if (type == gameType::nowReplay) {
			MessageBox(hwnd, L"승리하셨습니다.", L"리플레이 종료", MB_OK);
			return;
		}
		if (MessageBox(hwnd, L"리플레이를\n저장하시겠습니까?", L"승리!", MB_YESNO)) {
			if (!IsWindow(slotHwnd)) {
				slotHwnd = CreateDialog(g_hInst, MAKEINTRESOURCE(IDD_DIALOG1),
					hwnd, SlotProc);
				ShowWindow(slotHwnd, SW_SHOW);
			}
		}
		break;
	case 2:
		if (type == gameType::nowReplay) {
			MessageBox(hwnd, L"패배\n지뢰를 밟았습니다.", L"리플레이 종료", MB_OK);
			return;
		}
		if (MessageBox(hwnd, L"리플레이를\n저장하시겠습니까?", L"패배 - 지뢰를 밟았습니다.", MB_YESNO) == IDYES) {
			if (!IsWindow(slotHwnd)) {
				slotHwnd = CreateDialog(g_hInst, MAKEINTRESOURCE(IDD_DIALOG1),
					hwnd, SlotProc);
				ShowWindow(slotHwnd, SW_SHOW);
			}
		}
		break;
	}

}
void SetNewMap(HWND hwnd)
{
	for (int i = 0; i<30; i++)
		for (int j = 0; j<16; j++) {
			map[i][j].statue = up;
			map[i][j].mine = false;
			map[i][j].num = 0;
			map[i][j].flag = false;
			map[i][j].blank = false;
			map[i][j].error = false;
			map[i][j].temp = false;
			map[i][j].boom = false;
		}

	AdjustWindowRect(&rt, WS_OVERLAPPEDWINDOW, false);
	SetWindowPos(hwnd, HWND_TOP, 100, 100, rt.right - rt.left, rt.bottom - rt.top, NULL);
}
////////////////////////////

//게임 플레이 관련
void setReplay();
void Replayover();
void Replayresult();

void ClickBlank(HWND& hwnd, const int& xPos, const int& yPos, const int& type) {
	replay.input(click::left, xPos, yPos);
	map[yPos][xPos].statue = down;
	if (map[yPos][xPos].mine == true) {
		map[yPos][xPos].boom = true;
		if (type != gameType::checkScene)
			gameover(hwnd, 1, type);
		else
			Replayover();
		return;
	}
	if (map[yPos][xPos].blank == true)
		CheckBlank(yPos, xPos);
}
void ClickLR(HWND& hwnd, const int& xPos, const int& yPos, int& t_num, int& f_num, int type) {
	int j = xPos;
	int i = yPos;
	replay.input(click::both, j, i);
	t_num = map[i][j].num;
	for (int q = 0; q < 8; q++)
	{
		if (map[i + y[q]][j + x[q]].flag == true)
			f_num++;
		if (map[i + y[q]][j + x[q]].mine == false && map[i + y[q]][j + x[q]].flag == true) {
			if (type != gameType::checkScene)
				gameover(hwnd, 2, type);
			else
				Replayover();
			return;
		}

		else {
			if (i + y[q] < 0 || i + y[q] >= Ynum || j + x[q] < 0 || j + x[q] >= Xnum)
				continue;
			if (map[i + y[q]][j + x[q]].statue == up)
				map[i + y[q]][j + x[q]].temp = true;
		}
	}
	if (f_num == t_num)
	{
		CheckBlank(i, j);
		for (int q = 0; q < 8; q++)
		{
			if (map[i + y[q]][j + x[q]].temp == true)
				map[i + y[q]][j + x[q]].statue = down;
		}
	}
}
void UpLR() {
	for (int i = 0; i < Ynum; i++) {
		for (int j = 0; j < Xnum; j++) {
			map[i][j].temp = false;
		}
	}
}
void ClickFlag(const int& xPos, const int& yPos, int& mine_num) {
	int j = xPos;
	int i = yPos;
	replay.input(click::right, j, i);
	if (map[i][j].statue == up) {
		map[i][j].statue = down;
		map[i][j].flag = true;
		mine_num--;
	}
	else if (map[i][j].statue == down && map[i][j].flag == true) {
		map[i][j].statue = up;
		map[i][j].flag = false;
		mine_num++;
	}
}
void SetLevel(int level, HWND& hwnd, int& mine_num, int type) {
	switch (level)
	{
	case 1:
		rt.top = 0;
		rt.left = 0;
		rt.bottom = 340;
		rt.right = 280;

		Xnum = 9; Ynum = 9;
		mine_num = mine = 10;
		break;
	case 2:
		rt.top = 0;
		rt.left = 0;
		rt.bottom = 480;
		rt.right = 420;

		Xnum = 16; Ynum = 16;
		mine_num = mine = 50;
		break;
	case 3:
		rt.top = 0;
		rt.left = 0;
		rt.bottom = 480;
		rt.right = 700;

		Xnum = 16; Ynum = 30;
		mine_num = mine = 99;
		break;
	}
	SetNewMap(hwnd);
	if (type == gameType::nowPlay) {
		replay.saveLevel(level);
		replay.input(click::startP, -1, -1);
		SetTimer(hwnd, 2, 10, NULL);
		CreateMap();
		
	}
	else {
		setReplay();
	}
	KillTimer(hwnd, 4);
	tp = chrono::system_clock::now();
	start = chrono::system_clock::to_time_t(tp);
	now = 0;
	mine_num = mine;
	SetTimer(hwnd, 3, 70, NULL);
	
}
////////////////////////////

using vData = vector<Data>;

//리플레이 관련
void setReplay() {
	
	for (const auto& p : replay.getMine()) {
		map[p.second][p.first].mine = true;
	}

	for (int i = 0; i<Ynum; i++)
	{
		for (int j = 0; j<Xnum; j++)
		{
			for (int q = 0; q<8; q++)
			{
				if (i + y[q] >= 0 && i + y[q] <= Ynum && j + x[q] >= 0 && j + x[q] <= Xnum)
					if (map[i][j].mine == false && map[i + y[q]][j + x[q]].mine == true)
						map[i][j].num++;

			}
			if (map[i][j].num == 0 && map[i][j].mine == false)
				map[i][j].blank = true;
		}
	}
}
void getReplay(vData::iterator& iter, HWND& hwnd, int& t_num, int& f_num, int& mine_num, int type) {
	int xPos = iter->getX();
	int yPos = iter->getY();
	int rtype = iter->getType();
	switch (rtype) {
	case click::left:
		ClickBlank(hwnd, xPos, yPos, type);
		break;
	case click::right:
		ClickFlag(xPos, yPos, mine_num);
		break;
	case click::both:
		ClickLR(hwnd, xPos, yPos, t_num, f_num, type);
		SetTimer(hwnd, 5, 200, NULL);
		break;
	case click::startP:
		break;
	}
}

void initReplay(vData& inputData, vData::iterator& iter,
	int& type, time_t& timeCount, int& mine_num, HWND& hwnd) {
	inputData = replay.getInput();
	timeCount = inputData[0].getTime();
	iter = inputData.begin();
	type = gameType::nowReplay;
	SetLevel(replay.getLevel(), hwnd, mine_num, type);
	
	start = iter->getTime();
	KillTimer(hwnd, 2);
	SetTimer(hwnd, 4, 1000, NULL);
}

void getShapShot(vData& inputData, HWND& hwnd, int& t_num, int& f_num, int& mine_num) {
	vData::iterator iter = inputData.begin();
	while (iter != inputData.end()) {
		getReplay(iter, hwnd, t_num, f_num, mine_num, gameType::checkScene);
		nowscene tmp;
		for (int i = 0; i < 30; ++i)
			for (int j = 0; j < 16; ++j)
				tmp.map[i][j] = map[i][j];
		tmp.time = iter->getTime();
		tmp.now_mine = mine_num;
		snapShot.emplace_back(tmp);
		iter += 1;
	}
	for (int i = 0; i < 30; ++i)
		for (int j = 0; j < 16; ++j)
			map[i][j] = snapShot[0].map[i][j];
	mine_num = mine;
}

void Replayover()
{
	for (int i = 0; i<Ynum; i++) {
		for (int j = 0; j<Xnum; j++) {
			if (map[i][j].mine == false && map[i][j].flag == true)
				map[i][j].error = true;
			map[i][j].statue = down;
		}
	}
}
void Replayresult()
{
	int select = 0;
	for (int i = 0; i<Ynum; i++) {
		for (int j = 0; j<Xnum; j++) {
			if (map[i][j].flag == true && map[i][j].mine == false) {
				select = 1;
				break;
			}
			if (map[i][j].boom == true) {
				select = 2;
				break;
			}
		}
		if (select == 1 || select == 2)
			break;
	}


}
////////////////////////////


LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	static HDC hdc, mem1dc, mem2dc, mem3dc, mem4dc;
	static HBITMAP hBit1, BackGround, oldBit1, oldBit2, oldBit3, oldBit4;
	static RECT rectView;
	static HBITMAP tile, Btile, one, two, three, four, five, six, seven, eight, MINE, flag, failflag, boom, nmine, smile, smile2, mouse;
	int t_num = 0, f_num = 0;
	static int level = 1, temp = 0, mine_num = 0, mx, my, check = 0;
	static bool statue, select;
	static TCHAR buf[10], buf2[20];
	PAINTSTRUCT ps;

	static int type;
	static vData inputData;
	static vData::iterator iter;
	
	static time_t timeCount;

	switch (iMsg)
	{
	case WM_CREATE:
	{
		GetClientRect(hwnd, &rectView);
		SetTimer(hwnd, 1, 70, NULL);

		select = false;
		//배경화면
		BackGround = LoadBitmap(((LPCREATESTRUCT)lParam)->hInstance, MAKEINTRESOURCE(IDB_BITMAP1));
		//숫자
		one = LoadBitmap(((LPCREATESTRUCT)lParam)->hInstance, MAKEINTRESOURCE(IDB_BITMAP6));
		two = LoadBitmap(((LPCREATESTRUCT)lParam)->hInstance, MAKEINTRESOURCE(IDB_BITMAP7));
		three = LoadBitmap(((LPCREATESTRUCT)lParam)->hInstance, MAKEINTRESOURCE(IDB_BITMAP8));
		four = LoadBitmap(((LPCREATESTRUCT)lParam)->hInstance, MAKEINTRESOURCE(IDB_BITMAP9));
		five = LoadBitmap(((LPCREATESTRUCT)lParam)->hInstance, MAKEINTRESOURCE(IDB_BITMAP10));
		six = LoadBitmap(((LPCREATESTRUCT)lParam)->hInstance, MAKEINTRESOURCE(IDB_BITMAP11));
		seven = LoadBitmap(((LPCREATESTRUCT)lParam)->hInstance, MAKEINTRESOURCE(IDB_BITMAP12));
		eight = LoadBitmap(((LPCREATESTRUCT)lParam)->hInstance, MAKEINTRESOURCE(IDB_BITMAP13));
		//지뢰
		boom = LoadBitmap(((LPCREATESTRUCT)lParam)->hInstance, MAKEINTRESOURCE(IDB_BITMAP2));
		MINE = LoadBitmap(((LPCREATESTRUCT)lParam)->hInstance, MAKEINTRESOURCE(IDB_BITMAP4));
		failflag = LoadBitmap(((LPCREATESTRUCT)lParam)->hInstance, MAKEINTRESOURCE(IDB_BITMAP3));
		flag = LoadBitmap(((LPCREATESTRUCT)lParam)->hInstance, MAKEINTRESOURCE(IDB_BITMAP18));
		//UI관련
		nmine = LoadBitmap(((LPCREATESTRUCT)lParam)->hInstance, MAKEINTRESOURCE(IDB_BITMAP20));
		smile = LoadBitmap(((LPCREATESTRUCT)lParam)->hInstance, MAKEINTRESOURCE(IDB_BITMAP15));
		smile2 = LoadBitmap(((LPCREATESTRUCT)lParam)->hInstance, MAKEINTRESOURCE(IDB_BITMAP16));
		//타일
		mouse = LoadBitmap(((LPCREATESTRUCT)lParam)->hInstance, MAKEINTRESOURCE(IDB_BITMAP17));
		tile = LoadBitmap(((LPCREATESTRUCT)lParam)->hInstance, MAKEINTRESOURCE(IDB_BITMAP14));
		Btile = LoadBitmap(((LPCREATESTRUCT)lParam)->hInstance, MAKEINTRESOURCE(IDB_BITMAP5));

		type = gameType::nowPlay;
		SetLevel(level, hwnd, mine_num, type);
	}
	break;
	case WM_MOUSEMOVE:
	{
		if (type != gameType::nowPlay)
			break;
		mx = HIWORD(lParam);
		my = LOWORD(lParam);
		for (int i = 0; i < Ynum; i++) {
			for (int j = 0; j < Xnum; j++)
			{
				map[i][j].mouse = false;
			}
		}
		for (int i = 0; i < Ynum; i++) {
			for (int j = 0; j < Xnum; j++)
			{
				if (map[i][j].statue == up && mx >= j * 20 + 80 && mx <= (j + 1) * 20 + 80 && my >= i * 20 + 50 && my <= (i + 1) * 20 + 50)
				{
					map[i][j].mouse = true;
				}
			}
		}
	}
	break;
	case WM_LBUTTONDOWN:
	{
		if (type != gameType::nowPlay)
			break;
		mx = HIWORD(lParam);
		my = LOWORD(lParam);
		statue = true;
		for (int i = 0; i < Ynum; i++) {
			for (int j = 0; j < Xnum; j++)
			{
				//빈 칸 누르기
				if (map[i][j].statue == up && mx >= j * 20 + 80 && mx <= (j + 1) * 20 + 80 && my >= i * 20 + 50 && my <= (i + 1) * 20 + 50)
				{
					ClickBlank(hwnd, j, i, gameType::nowPlay);
				}
			}
		}
		//이모티콘 누르기
		if (my > rt.right / 2 - 20 && mx > 20 && my < rt.right / 2 + 10 && mx < 50) {
			select = true;
			SetLevel(level, hwnd, mine_num, gameType::nowPlay);
		}
	}
	break;
	case WM_LBUTTONUP:
	{
		if (type != gameType::nowPlay)
			break;
		statue = false;
		select = false;
	}
	break;
	case WM_RBUTTONDOWN:
	{
		if (type != gameType::nowPlay)
			break;
		mx = HIWORD(lParam);
		my = LOWORD(lParam);
		for (int i = 0; i < Ynum; i++) {
			for (int j = 0; j < Xnum; j++)
			{
				if (mx >= j * 20 + 80 && mx <= (j + 1) * 20 + 80 && my >= i * 20 + 50 && my <= (i + 1) * 20 + 50) {
					if (statue == true && map[i][j].statue == down)
						ClickLR(hwnd, j, i, t_num, f_num, gameType::nowPlay);
					else
						ClickFlag(j, i, mine_num);
				}
			}
			t_num = 0;
			f_num = 0;
		}
	}
	break;
	case WM_RBUTTONUP:
	{
		if (type != gameType::nowPlay)
			break;
		UpLR();
	}
	break;
	case WM_PAINT:
	{
		hdc = BeginPaint(hwnd, &ps);
		GetClientRect(hwnd, &rectView);
		mem1dc = CreateCompatibleDC(hdc);
		oldBit1 = (HBITMAP)SelectObject(mem1dc, hBit1);
		BitBlt(hdc, 0, 0, rectView.right, rectView.bottom, mem1dc, 0, 0, SRCCOPY);
		wsprintf(buf2, L"time : %d", now);
		wsprintf(buf, L"%d", mine_num);
		SetBkMode(hdc, TRANSPARENT);
		TextOut(hdc, rt.right / 2 + 10, rt.bottom - 63, buf, lstrlen(buf));
		TextOut(hdc, rt.right / 2 + 50, rt.bottom - 63, buf2, lstrlen(buf2));
		SelectObject(mem1dc, oldBit1);
		DeleteDC(mem2dc);
		EndPaint(hwnd, &ps);
	}
	return 0;
	case WM_TIMER:
		GetClientRect(hwnd, &rectView);
		switch (wParam)
		{
		case 1: //랜더링
		{
			temp = 0;
			hdc = GetDC(hwnd);
			if (hBit1 == NULL)
				hBit1 = CreateCompatibleBitmap(hdc, 1024, 768);

			mem1dc = CreateCompatibleDC(hdc);
			mem2dc = CreateCompatibleDC(mem1dc);
			mem3dc = CreateCompatibleDC(mem1dc);
			mem4dc = CreateCompatibleDC(mem1dc);
			oldBit1 = (HBITMAP)SelectObject(mem1dc, hBit1);
			oldBit2 = (HBITMAP)SelectObject(mem2dc, tile);
			oldBit3 = (HBITMAP)SelectObject(mem3dc, Btile);
			oldBit4 = (HBITMAP)SelectObject(mem4dc, BackGround);
			StretchBlt(mem1dc, 0, 0, rectView.right, rectView.bottom, mem4dc, 0, 0, 200, 400, SRCCOPY);
			for (int i = 0; i < Ynum; i++) {
				for (int j = 0; j < Xnum; j++) {
					if (map[i][j].statue == up) {
						BitBlt(mem1dc, i * 20 + 50, j * 20 + 80, 20, 20, mem2dc, 0, 0, SRCCOPY);
						if (map[i][j].temp == true) {
							oldBit3 = (HBITMAP)SelectObject(mem3dc, Btile);
							BitBlt(mem1dc, i * 20 + 50, j * 20 + 80, 20, 20, mem3dc, 0, 0, SRCCOPY);
						}
						if (map[i][j].mouse == true) {
							oldBit3 = (HBITMAP)SelectObject(mem3dc, mouse);
							BitBlt(mem1dc, i * 20 + 50, j * 20 + 80, 20, 20, mem3dc, 0, 0, SRCCOPY);
						}
					}
					else {
						oldBit3 = (HBITMAP)SelectObject(mem3dc, Btile);
						BitBlt(mem1dc, i * 20 + 50, j * 20 + 80, 20, 20, mem3dc, 0, 0, SRCCOPY);
						if (map[i][j].mine == true) {
							oldBit3 = (HBITMAP)SelectObject(mem3dc, MINE);
							BitBlt(mem1dc, i * 20 + 50, j * 20 + 80, 20, 20, mem3dc, 0, 0, SRCCOPY);
						}
						if (map[i][j].boom == true) {
							oldBit3 = (HBITMAP)SelectObject(mem3dc, boom);
							BitBlt(mem1dc, i * 20 + 50, j * 20 + 80, 20, 20, mem3dc, 0, 0, SRCCOPY);
						}
						if (map[i][j].num == 1) {
							oldBit3 = (HBITMAP)SelectObject(mem3dc, one);
							BitBlt(mem1dc, i * 20 + 50, j * 20 + 80, 20, 20, mem3dc, 0, 0, SRCCOPY);
						}
						if (map[i][j].num == 2) {
							oldBit3 = (HBITMAP)SelectObject(mem3dc, two);
							BitBlt(mem1dc, i * 20 + 50, j * 20 + 80, 20, 20, mem3dc, 0, 0, SRCCOPY);
						}
						if (map[i][j].num == 3) {
							oldBit3 = (HBITMAP)SelectObject(mem3dc, three);
							BitBlt(mem1dc, i * 20 + 50, j * 20 + 80, 20, 20, mem3dc, 0, 0, SRCCOPY);
						}
						if (map[i][j].num == 4) {
							oldBit3 = (HBITMAP)SelectObject(mem3dc, four);
							BitBlt(mem1dc, i * 20 + 50, j * 20 + 80, 20, 20, mem3dc, 0, 0, SRCCOPY);
						}
						if (map[i][j].num == 5) {
							oldBit3 = (HBITMAP)SelectObject(mem3dc, five);
							BitBlt(mem1dc, i * 20 + 50, j * 20 + 80, 20, 20, mem3dc, 0, 0, SRCCOPY);
						}
						if (map[i][j].num == 6) {
							oldBit3 = (HBITMAP)SelectObject(mem3dc, six);
							BitBlt(mem1dc, i * 20 + 50, j * 20 + 80, 20, 20, mem3dc, 0, 0, SRCCOPY);
						}
						if (map[i][j].num == 7) {
							oldBit3 = (HBITMAP)SelectObject(mem3dc, seven);
							BitBlt(mem1dc, i * 20 + 50, j * 20 + 80, 20, 20, mem3dc, 0, 0, SRCCOPY);
						}
						if (map[i][j].num == 8) {
							oldBit3 = (HBITMAP)SelectObject(mem3dc, eight);
							BitBlt(mem1dc, i * 20 + 50, j * 20 + 80, 20, 20, mem3dc, 0, 0, SRCCOPY);
						}
						if (map[i][j].flag == true) {
							oldBit3 = (HBITMAP)SelectObject(mem3dc, flag);
							BitBlt(mem1dc, i * 20 + 50, j * 20 + 80, 20, 20, mem3dc, 0, 0, SRCCOPY);
						}
						if (map[i][j].error == true) {
							oldBit3 = (HBITMAP)SelectObject(mem3dc, failflag);
							BitBlt(mem1dc, i * 20 + 50, j * 20 + 80, 20, 20, mem3dc, 0, 0, SRCCOPY);
						}

					}
				}
			}
			/*oldBit4 = (HBITMAP)SelectObject(mem4dc, timer);
			TransparentBlt(mem1dc, rt.left + 60, rt.bottom - 70, 30, 30, mem4dc, 0, 0, 80, 80, RGB(107, 174, 255));*/
			oldBit4 = (HBITMAP)SelectObject(mem4dc, nmine);
			TransparentBlt(mem1dc, rt.right / 2 - 30, rt.bottom - 70, 30, 30, mem4dc, 0, 0, 80, 80, RGB(107, 174, 255));
			if (select == false) {
				oldBit4 = (HBITMAP)SelectObject(mem4dc, smile);
				TransparentBlt(mem1dc, rt.right / 2 - 20, 20, 30, 30, mem4dc, 0, 0, 595, 595, RGB(255, 255, 255));
			}
			else {
				oldBit4 = (HBITMAP)SelectObject(mem4dc, smile2);
				TransparentBlt(mem1dc, rt.right / 2 - 20, 20, 30, 30, mem4dc, 0, 0, 595, 595, RGB(255, 255, 255));
			}

			SelectObject(mem4dc, oldBit4); DeleteDC(mem4dc);
			SelectObject(mem3dc, oldBit3); DeleteDC(mem3dc);
			SelectObject(mem2dc, oldBit2); DeleteDC(mem2dc);
			SelectObject(mem1dc, oldBit1); DeleteDC(mem1dc);
			ReleaseDC(hwnd, hdc);
		}
		break;
		case 2: //타이머
			tp = chrono::system_clock::now();
			now = chrono::system_clock::to_time_t(tp)- start;
			break;
		case 3: //랜더링
		{
			for (int i = 0; i < Ynum; i++) {
				for (int j = 0; j < Xnum; j++)
				{
					if (map[i][j].statue == up) {
						temp = 1;
						break;
					}
					if (i == Ynum - 1 && j == Xnum - 1) {
						if (type != gameType::checkScene)
							result(hwnd, type);
						else
							Replayresult();
					}
				}
				if (temp == 1)
					break;
			}
		}
		break;
		case 4: //리플레이
		{
			int tmpp = iter->getTime();
			now = timeCount - start;
			
			if (timeCount != tmpp) {
				timeCount++;
				break;
			}
			if (iter >= inputData.end() - 1) {
				KillTimer(hwnd, 4);
				getReplay(iter, hwnd, t_num, f_num, mine_num, gameType::nowReplay);
				break;
			}
			getReplay(iter, hwnd, t_num, f_num, mine_num, gameType::nowReplay);
			iter += 1;
			if (iter->getTime() == tmpp) {
				getReplay(iter, hwnd, t_num, f_num, mine_num, gameType::nowReplay);
				
				iter += 1;
			}
			timeCount++;
			break;
		}
		break;
		case 5: //리플레이
		{
			UpLR();
			KillTimer(hwnd, 5);
		}
		break;
		}


		InvalidateRgn(hwnd, NULL, false);
		return 0;
	case WM_COMMAND:
		switch (wParam)
		{
		case ID_NEW:
			SetLevel(level, hwnd, mine_num, gameType::nowPlay);
			type = gameType::nowPlay;
			break;
		case ID_EXIT:
			PostQuitMessage(0);
			break;
		case ID_EASY:
			level = 1;
			type = gameType::nowPlay;
			SetLevel(level, hwnd, mine_num, type);
			break;
		case ID_MEDIUM:
			level = 2;
			type = gameType::nowPlay;
			SetLevel(level, hwnd, mine_num, type);
			break;
		case ID_HARD:
			level = 3;
			type = gameType::nowPlay;
			SetLevel(level, hwnd, mine_num, type);
			break;
		case ID_REPLAY_SLOT1:
			if (!replay.fileOpen("replay1.txt")) {
				MessageBox(hwnd, L"리플레이 파일이\n존재하지않습니다.", L"에러", MB_OK);
				break;
			}
			initReplay(inputData, iter, type, timeCount, mine_num, hwnd);
			getShapShot(inputData, hwnd, t_num, f_num, mine_num);

			break;
		case ID_REPLAY_SLOT2:
			if (!replay.fileOpen("replay2.txt")) {
				MessageBox(hwnd, L"리플레이 파일이\n존재하지않습니다.", L"에러", MB_OK);
				break;
			}
			initReplay(inputData, iter, type, timeCount, mine_num, hwnd);
			getShapShot(inputData, hwnd, t_num, f_num, mine_num);
			
			break;
		case ID_REPLAY_SLOT3:
			if (!replay.fileOpen("replay3.txt")) {
				MessageBox(hwnd, L"리플레이 파일이\n존재하지않습니다.", L"에러", MB_OK);
				break;
			}
			initReplay(inputData, iter, type, timeCount, mine_num, hwnd);
			getShapShot(inputData, hwnd, t_num, f_num, mine_num);

			break;
		}
		InvalidateRgn(hwnd, NULL, false);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}

	return(DefWindowProc(hwnd, iMsg, wParam, lParam));
}

INT_PTR CALLBACK SlotProc(HWND hDig, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	switch (iMessage) {
	case WM_COMMAND:
	{
		switch (LOWORD(wParam)) {
		case IDC_REPLAY_SLOT1:
			replay.fileSave("replay1.txt");
			DestroyWindow(hDig);
			hDig = NULL;
			return true;
		case IDC_REPLAY_SLOT2:
			replay.fileSave("replay2.txt");
			DestroyWindow(hDig);
			hDig = NULL;
			return true;
		case IDC_REPLAY_SLOT3:
			replay.fileSave("replay3.txt");
			DestroyWindow(hDig);
			hDig = NULL;
			return true;
		default:
			return false;
		}
	}
	case WM_CLOSE:
		DestroyWindow(hDig);
		hDig = NULL;
		return true;
	}
	return 0;
}