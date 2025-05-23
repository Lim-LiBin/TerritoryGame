#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <conio.h>
#include <Windows.h>
#include <stralign.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

// 색상 정의
#define BLACK	0
#define BLUE1	1
#define GREEN1	2
#define CYAN1	3
#define RED1	4
#define MAGENTA1 5
#define YELLOW1	6
#define GRAY1	7
#define GRAY2	8
#define BLUE2	9
#define GREEN2	10
#define CYAN2	11
#define RED2	12
#define MAGENTA2 13
#define YELLOW2	14
#define WHITE	15

#define STAR "□" // player1 표시
#define BLANK "■" // ' ' 로하면 흔적이 지워진다

#define ESC 0x1b //  ESC 누르면 종료

#define SPECIAL1 0xe0 // 특수키는 0xe0 + key 값으로 구성된다.
#define SPECIAL2 0x00 // keypad 경우 0x00 + key 로 구성된다.

#define UP  0x48 // Up key는 0xe0 + 0x48 두개의 값이 들어온다.
#define DOWN 0x50
#define LEFT 0x4b
#define RIGHT 0x4d
#define SUBMIT 4 //스페이스바

#define UP2		'w'//player2 는 AWSD 로 방향키 대신
#define DOWN2	's'
#define LEFT2	'a'
#define RIGHT2	'd'

//게임 필드 크기
#define WIDTH 115
#define HEIGHT 30

#define PLAYER1_LENGTH 1
#define PLAYER2_LENGTH 1
#define PLAYER1_LENGTH_MAX 20000
#define PLAYER2_LENGTH_MAX 20000
COORD player1_head[PLAYER1_LENGTH_MAX];
COORD player2_head[PLAYER2_LENGTH_MAX];
int player1_length = PLAYER1_LENGTH;
int player2_length = PLAYER2_LENGTH;

int Delay = 10; // 100 msec delay, 이 값을 줄이면 속도가 빨라진다.
int keep_moving = 1; // 1:계속이동, 0:한칸씩이동.
int time_out = 60; // 제한시간
int score[2] = { 0 };
int speedups[WIDTH][HEIGHT] = { 0 }; // 1이면 Gold 있다는 뜻
int speeddowns[WIDTH][HEIGHT] = { 0 };
int defines[WIDTH][HEIGHT] = { 0 };
int speedupinterval = 5;
int speeddowninterval = 7;
int defineinterval = 10;
int frame_count = 0;
int p1_frame_sync = 10;
int p1_frame_sync_start = 0;
int p2_frame_sync = 10;
int p2_frame_sync_start = 0;
int called[2];

int grid[WIDTH][HEIGHT] = { 0 };

int visited[WIDTH][HEIGHT];

void gotoxy(int x, int y) //내가 원하는 위치로 커서 이동
{
	COORD pos; // Windows.h 에 정의
	pos.X = x;
	pos.Y = y;
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}

void gotoxy2(COORD pos)
{
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);// WIN32API 함수
}

void textcolor(int fg_color, int bg_color)
{
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), fg_color | bg_color << 4);
}

void removeCursor(void) { // 커서를 안보이게 한다
	CONSOLE_CURSOR_INFO curInfo;
	GetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &curInfo);
	curInfo.bVisible = 0;
	SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &curInfo);
}

void showCursor(void) { // 커서를 보이게 한다
	CONSOLE_CURSOR_INFO curInfo;
	GetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &curInfo);
	curInfo.bVisible = 1;
	SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &curInfo);
}

void cls(int text_color, int bg_color) // 화면 지우기
{
	char cmd[100];
	system("cls");
	sprintf(cmd, "COLOR %x%x", bg_color, text_color);
	system(cmd);
}

void putstar(int x, int y)
{
	gotoxy(x, y);
	printf("%s", STAR);
}

void erasestar(int x, int y)
{
	gotoxy(x, y);
	printf("%s", BLANK);
}

//속도 증가 아이템
#define SPEEDUP "♥"
void show_speedup() {
	int x, y;
	x = rand() % WIDTH;
	y = rand() % (HEIGHT - 1) + 1;
	textcolor(RED1, WHITE);
	gotoxy(x, y);
	printf(SPEEDUP);
	speedups[x][y] = 1;
	textcolor(BLACK, WHITE);
}

//속도 감소 아이템
#define SPEEDDOWN "↓"
void show_speeddown() {
	int x, y;
	x = rand() % WIDTH;
	y = rand() % (HEIGHT - 1) + 1;
	textcolor(BLUE1, WHITE);
	gotoxy(x, y);
	printf(SPEEDDOWN);
	speeddowns[x][y] = 1;
	textcolor(BLACK, WHITE);
}

//방어 아이템
#define DEFINE "※"
void show_define() {
	int x, y;
	x = rand() % WIDTH;
	y = rand() % (HEIGHT - 1) + 1;
	textcolor(GREEN1, WHITE);
	gotoxy(x, y);
	printf(DEFINE);
	defines[x][y] = 1;
	textcolor(BLACK, WHITE);
}

// box 그리기 함수, ch 문자로 (x1,y1) ~ (x2,y2) box를 그린다.
void draw_box(int x1, int y1, int x2, int y2, char ch)
{
	int x, y;
	for (x = x1; x <= x2; x += 1) {
		gotoxy(x, y1);
		printf("%c", ch);
		gotoxy(x, y2);
		printf("%c", ch);
	}
	for (y = y1; y <= y2; y++) {
		gotoxy(x1, y);
		printf("%c", ch);
		gotoxy(x2, y);
		printf("%c", ch);
	}
}

// box 그리기 함수, ch 문자열로 (x1,y1) ~ (x2,y2) box를 그린다.
// 한글 문자를 그리는 용도로 사용 (예, "□" 로 ㅁ 벽을 그리는 경우)
void draw_box2(int x1, int y1, int x2, int y2, char* ch)
{
	int x, y;
	int len = strlen(ch);
	for (x = x1; x <= x2; x += len) {
		gotoxy(x, y1);
		printf("%s", ch);
		gotoxy(x, y2);
		printf("%s", ch);
	}
	for (y = y1; y <= y2; y++) {
		gotoxy(x1, y);
		printf("%s", ch);
		gotoxy(x2, y);
		printf("%s", ch);
	}
}

//점수 표시시
void showscore(int player, int player1_color, int player2_color)
{
	switch (player) {
	case 0: // player 1
		textcolor(player1_color, WHITE);
		gotoxy(80, 0);
		printf("Player1 : ");
		textcolor(BLACK, WHITE);
		printf("%d", score[player]);
		break;
	case 1: // player 2
		textcolor(player2_color, WHITE);
		gotoxy(25, 0);
		printf("Player2 : ");
		textcolor(BLACK, WHITE);
		printf("%d", score[player]);
		break;
	}
	textcolor(BLACK, WHITE);
}

//시간 표시시
void show_time(int remain_time)
{
	textcolor(WHITE, BLACK);
	gotoxy(50, 0);
	printf(" 남은시간 : %02d ", remain_time);
	textcolor(BLACK, WHITE);
}

//player들이 방문한 곳 체크, 기본은 0
void visit() {
	for (int i = 0; i < WIDTH; i++) {
		for (int j = 0; j < HEIGHT; j++) {
			visited[i][j] = 0;
		}
	}
}

//잠시 멈추기
void delay(int time) {
	clock_t start_time = clock();

	while ((clock() - start_time) * 1000 / CLOCKS_PER_SEC < time) {
		//시간이 경과할 때까지 대기
	}
}

//내부 채우기 - 왼쪽부터 검사
void boxfill_left(int player, int color)
{
	int x, y;
	int found = 0;

	for (y = 0; y < HEIGHT; y++) {
		found = 0;
		for (x = 0; x < WIDTH; x += 2) { // grid에서 player가 시작되는 좌측 좌표를 찾는다
			if (grid[x][y] == player) {
				found = 1;
				break;
			}
		}
		if (found) { // player의 왼쪽 끝이 
			x += 2; // 가로축 그 다음 칸부터 
			for (; x < WIDTH; x += 2) {
				if (grid[x][y] == player) // 11 이거나 100001 에서 마지막 1이면 종료
					break;
				if (grid[x][y] == 0) { // 1000000 이면 0을 1로 채우고 .. 마지막 1은 위어서 끝
					int collision = 0;
					int other = (player == 1) ? 2 : 1;
					int startX = x;
					while (startX > 0 && grid[startX - 2][y] == player)
						startX -= 2;
					if (startX >= 0 && grid[startX - 2][y] == other)
						collision = 1;
					if (!collision) {
						grid[x][y] = player;
						//gotoxy(x, y);
						textcolor(color, WHITE);
						erasestar(x, y);
					}
				}
			}
		}

	}
}

//내부 채우기 - 오른쪽부터 검사
void boxfill_right(int player, int color)
{
	int x, y;
	int found = 0;

	for (y = 0; y < HEIGHT; y++) {
		found = 0;
		for (x = WIDTH - 1; x >= 0; x -= 2) { // grid에서 player가 시작되는 우측 좌표를 찾는다
			if (grid[x][y] == player) {
				found = 1;
				break;
			}
		}
		if (found) { // player의 오른쪽 끝이 
			x -= 2; // 가로축 그 다음 칸부터 
			for (; x >= 0; x -= 2) {
				if (grid[x][y] == player) // 11 이거나 100001 에서 마지막 1이면 종료
					break;
				if (grid[x][y] == 0) { // 1000000 이면 0을 1로 채우고 .. 마지막 1은 위어서 끝
					int collision = 0;
					int other = (player == 1) ? 2 : 1;
					int startX = x;
					while (startX < WIDTH - 1 && grid[startX + 2][y] == player)
						startX += 2;
					if (startX < WIDTH - 1 && grid[startX + 2][y] == other)
						collision = 1;
					if (!collision) {
						grid[x][y] = player;
						//gotoxy(x, y);
						textcolor(color, WHITE);
						erasestar(x, y);
					}
					else {
						break;
					}
				}
			}
		}
	}
}

//내부 채우기 - 위에서부터 검사
void boxfill_up(int player, int color)
{
	int x, y;
	int found = 0;

	for (x = 0; x < WIDTH; x++) {
		found = 0;
		for (y = 0; y < HEIGHT; y++) { // grid에서 player가 시작되는 좌측 좌표를 찾는다
			if (grid[x][y] == player) {
				found = 1;
				break;
			}
		}
		if (found) { // player의 왼쪽 끝이 
			y++; // 가로축 그 다음 칸부터 
			for (; y < HEIGHT; y++) {
				if (grid[x][y] == player) { // 11 이거나 100001 에서 마지막 1이면 종료
					found = 1;
					break;
				}
				if (found) {
					y++;
					for (; x < HEIGHT; y++) {
						if (grid[x][y] == player)
							break;
						if (grid[x][y] == 0) {
							grid[x][y] = player;
							textcolor(color, WHITE);
							erasestar(x, y);
						}
					}
				}
			}
		}

	}
}

//내부 채우기 - 아래부터 검사
void boxfill_down(int player, int color)
{
	int x, y;
	int found = 0;

	for (x = 0; x < WIDTH; x++) {
		found = 0;
		for (y = HEIGHT - 1; y >= 0; y--) { // grid에서 player가 시작되는 좌측 좌표를 찾는다
			if (grid[x][y] == player) {
				found = 1;
				break;
			}
		}
		if (found) { // player의 왼쪽 끝이 
			y--; // 가로축 그 다음 칸부터 
			for (; y >= 0; y--) {
				if (grid[x][y] == player) { // 11 이거나 100001 에서 마지막 1이면 종료
					found = 1;
					break;
				}
				if (found) {
					y--;
					for (; y >= 0; y--) {
						if (grid[x][y] == player)
							break;
						if (grid[x][y] == 0) {
							grid[x][y] = player;
							textcolor(color, WHITE);
							erasestar(x, y);
						}
					}
				}
			}
		}

	}
}

//player들이 차지한 영역만큼 점수 계산
int score_area(int player) {
	int area = 0;

	for (int y = 0; y < HEIGHT; y++) {
		for (int x = 0; x < WIDTH; x += 2) {
			if (grid[x][y] == player) {
				area++;
			}
		}
	}
	return area;
}

//player1
void player1(unsigned char ch, int color)
{
	static int oldx = 100, oldy = 13, newx = 60, newy = 13;
	int move_flag = 0;
	static unsigned char last_ch = 0;

	if (called[0] == 0) { // 처음 또는 Restart
		oldx = 100, oldy = 13, newx = 60, newy = 13;
		putstar(oldx, oldy);
		called[0] = 1;
		last_ch = 0;
		ch = 0;
	}

	if (last_ch == ch && frame_count % p1_frame_sync != 0)
		return;
	if (keep_moving && ch == 0)
		ch = last_ch;
	last_ch = ch;

	switch (ch) {
	case UP:
		if (oldy > 1 && grid[oldx][oldy - 1] != 2) //player2가 간 곳 못가게
			newy = oldy - 1;
		move_flag = 1;
		break;
	case DOWN:
		if (oldy < HEIGHT - 1 && grid[oldx][oldy + 1] != 2)
			newy = oldy + 1;
		move_flag = 1;
		break;
	case LEFT:
		if (oldx > 0 && grid[oldx - 2][oldy] != 2)
			newx = oldx - 2;
		move_flag = 1;
		break;
	case RIGHT:
		if (oldx < WIDTH - 1 && grid[oldx + 2][oldy] != 2)
			newx = oldx + 2;
		move_flag = 1;
		break;
	}

	if (move_flag) {
		textcolor(color, WHITE);
		erasestar(oldx, oldy); // 마지막 위치의 * 를 지우고
		putstar(newx, newy); // 새로운 위치에서 * 를 표시한다.

		oldx = newx; // 마지막 위치를 기억한다.
		oldy = newy;

		//내부 채우기 & 방문한 곳 1로 표시(player2가 못오게)
		if (grid[newx][newy] == 1 && visited[newx][newy] != 1) {
			visited[newx][newy] = 1;
			boxfill_left(1, color); // box 채우는 함수 호출
			boxfill_right(1, color);
			boxfill_up(1, color);
		}
		else
			grid[newx][newy] = 1;

		score[0] = score_area(1);

		showscore(0, color, color);

		//속도 증가 효과
		if (speedups[newx][newy]) {
			speedups[newx][newy] = 0;
			if (p1_frame_sync > 5)
				p1_frame_sync--;
		}

		//속도 감소 효과
		if (speeddowns[newx][newy]) {
			speeddowns[newx][newy] = 0;
			if (p1_frame_sync < 5)
				p1_frame_sync++;
		}

		//방어 효과
		if (defines[newx][newy]) {
			defines[newx][newy] = 0;

			delay(5000);
		}
	}
}

//player2
void player2(unsigned char ch, int color)
{
	static int oldx = 20, oldy = 13, newx = 20, newy = 13;
	int move_flag = 0;
	static char last_ch = 0;

	if (called[1] == 0) { // 처음 또는 Restart
		oldx = 20, oldy = 13, newx = 20, newy = 13;
		putstar(oldx, oldy);
		called[1] = 1;
		last_ch = 0;
		ch = 0;
	}
	// 방향키가 눌려지지 않은 경우 예전에 이동하던 방향으로 계속 이동
	if (keep_moving && ch == 0)
		ch = last_ch;
	last_ch = ch;

	switch (ch) {
	case UP2:
		if (oldy > 1 && grid[oldx][oldy - 1] != 1) // player1과 겹치지 않게
			newy = oldy - 1;
		move_flag = 1;
		break;
	case DOWN2:
		if (oldy < HEIGHT - 1 && grid[oldx][oldy + 1] != 1)
			newy = oldy + 1;
		move_flag = 1;
		break;
	case LEFT2:
		if (oldx > 0 && grid[oldx - 2][oldy] != 1)
			newx = oldx - 2;
		move_flag = 1;
		break;
	case RIGHT2:
		if (oldx < WIDTH - 1 && grid[oldx + 2][oldy] != 1)
			newx = oldx + 2;
		move_flag = 1;
		break;
	}
	if (move_flag) {
		textcolor(color, WHITE);
		erasestar(oldx, oldy); // 마지막 위치의 # 를 지우고
		putstar(newx, newy); // 새로운 위치에서 # 를 표시한다.
		oldx = newx; // 마지막 위치를 기억한다.
		oldy = newy;
		move_flag = 1; // 1:계속 이동, 0:한칸씩 이동

		//내부 채우기 & player2가 방문한 곳 2로 설정(player1이 못가게)
		if (grid[newx][newy] == 2 && visited[newx][newy] != 2) {
			visited[newx][newy] = 2;
			boxfill_left(2, color);
			boxfill_right(2, color);
			boxfill_up(2, color);
		}
		else
			grid[newx][newy] = 2;

		score[1] = score_area(2);

		showscore(1, color, color);

		//속도 증가 효과
		if (speedups[newx][newy]) {
			speedups[newx][newy] = 0;

			if (p2_frame_sync < 5)
				p2_frame_sync--;
		}

		//속도 감소 효과
		if (speeddowns[newx][newy]) {
			speeddowns[newx][newy] = 0;

			if (p2_frame_sync > 5)
				p2_frame_sync++;
		}

		//방어 효과
		if (defines[newx][newy]) {
			defines[newx][newy] = 0;

			delay(5000); //5초 동안 멈추기
		}
	}
}

void draw_hline(int y, int x1, int x2, char ch) {
	gotoxy(x1, y);
	for (; x1 <= x2; x1++)
		putchar(ch);
}

void init_game()
{
	int x, y;
	char cmd[100];

	srand(time(NULL));
	score[0] = score[1] = 0;
	called[0] = called[1] = 0;
	for (x = 0; x < WIDTH; x++)
		for (y = 0; y < HEIGHT; y++) {
			speedups[x][y] = 0;
			speeddowns[x][y] = 0;
			defines[x][y] = 0;
		}
	speedupinterval = 5; //속도 증가 아이템
	speeddowninterval = 5; //속도 감소 아이템
	defineinterval = 5; //방어 아이템
	time_out = 30;
	keep_moving = 1;
	//Delay = 100;
	frame_count = 0;
	p1_frame_sync = 10;
	p1_frame_sync_start = 0;
	p2_frame_sync = 10;
	p2_frame_sync_start = 0;

	cls(BLACK, WHITE);
	removeCursor();
	textcolor(BLACK, WHITE);
	draw_hline(0, 0, 79, ' ');
	textcolor(BLACK, WHITE);
	sprintf(cmd, "mode con cols=%d lines=%d", WIDTH, HEIGHT);
	system(cmd);
}

//타이틀
void title() {
	system("cls");

	PlaySound(TEXT("Opening.wav"), NULL, SND_ASYNC);

	int c1, c2;
	do {
		c1 = rand() % 16;
		c2 = rand() % 16;
	} while (c1 == c2); //초기화면으로 돌아갈 때마다 타이틀 색이 변함

	textcolor(c1, WHITE);
	gotoxy(1, 1);
	printf("\n");
	printf("             ■■■■■■■  ■      ■■■■■■■  ■      ■■■■■■    ■■    ■■■■■■    ■■\n");
	printf("           ■■    ■■      ■    ■■    ■■      ■      ■      ■■    ■■            ■■    ■■\n");
	printf("           ■■    ■■      ■    ■■    ■■      ■      ■      ■■■■■■            ■■    ■■\n");
	printf("           ■■    ■■      ■■■■■    ■■      ■      ■      ■■    ■■            ■■    ■■\n");
	printf("           ■■    ■■      ■    ■■    ■■      ■■■  ■      ■■    ■■          ■■■    ■■\n");
	printf("             ■■■■■■■■■    ■■    ■■      ■      ■■■■■■    ■■        ■■■      ■■\n");
	printf("                             ■    ■■    ■■      ■                              ■■■■        ■■\n");
	printf("                 ■■■■■■        ■■■■■■■■■        ■■■■■■■■■    ■■            ■■\n");
	printf("               ■■■■■■■■                      ■                      ■■                    ■■\n");
	printf("               ■■          ■                      ■                      ■■                    ■■\n");
	printf("               ■■■■■■■■                      ■                      ■■                    ■■\n");
	printf("                 ■■■■■■                                                ■■                    ■■\n");

	Sleep(100);
}

//게임 설명 메뉴
void explain() {
	system("cls");
	gotoxy(56, 3);
	textcolor(GREEN2, BLACK);
	printf("방향키\n");
	textcolor(BLACK, WHITE);
	gotoxy(45, 5);
	printf("PLAYER1은 방향키로 이동합니다.\n");
	gotoxy(45, 6);
	printf("PLAYER2는 AWSD로 이동합니다.\n");

	gotoxy(56, 9);
	textcolor(GREEN2, BLACK);
	printf("아이템\n");
	textcolor(RED1, WHITE);
	gotoxy(52, 11);
	printf("♥ ");
	textcolor(BLACK, WHITE);
	printf("속도 증가\n");
	textcolor(RED1, WHITE);
	gotoxy(52, 13);
	printf("↓ ");
	textcolor(BLACK, WHITE);
	printf("속도 감소\n");
	textcolor(RED1, WHITE);
	gotoxy(52, 15);
	printf("※ ");
	textcolor(BLACK, WHITE);
	printf("모든 player 5초 멈추기\n");

	textcolor(GREEN2, BLACK);
	gotoxy(55, 18);
	printf("게임 규칙\n");
	textcolor(BLACK, WHITE);
	gotoxy(35, 20);
	textcolor(BLUE2, WHITE);
	printf("제한시간 60초 안에 더 많은 영역을 차지한 사람이 승리!\n");
	gotoxy(43, 21);
	textcolor(BLUE2, WHITE);
	printf("상대방의 영역과 중복할 수 없습니다.");

	gotoxy(37, 25);
	textcolor(WHITE, BLACK);
	printf(" 스페이스바를 누르면 메인화면으로 이동합니다. \n");


	while (1) {
		if (keyControl() == SUBMIT) {
			break;
		}
	}
	cls(BLACK, WHITE);
	title();
}

//초기 화면 메뉴
int menu() {
	int x = 24;
	int y = 21;

	int pos = 0;
	textcolor(GREEN2, WHITE);
	draw_box2(8, 17, 104, 23, "■");
	textcolor(BLACK, WHITE);
	gotoxy(32, 19);
	printf("          선택은 스페이스바를 눌러주세요.\n");
	gotoxy(x, y);
	textcolor(RED1, WHITE);
	printf("▶");
	textcolor(BLACK, WHITE);
	printf("  게임 시작 \n");
	gotoxy(x + 28, y);
	printf("게임 설명 \n");
	gotoxy(x + 52, y);
	printf("게임 종료 \n");

	//화살표 이동 & 선택 시 해당 화면으로 전환
	while (1) {
		int n = keyControl();
		switch (n) {
		case LEFT: {
			if (x > 24) {
				gotoxy(x, y);
				printf(" ");
				x -= 24;
				gotoxy(x, y);
				textcolor(RED1, WHITE);
				printf("▶");
			}
			break;
		}
		case RIGHT: {
			if (x < 79) {
				gotoxy(x, y);
				printf(" ");
				x += 24;
				gotoxy(x, y);
				textcolor(RED1, WHITE);
				printf("▶");
			}
			break;
		}
		case SUBMIT:
			return 24 - x;
		}
	}
}

//스페이스바
int keyControl() {
	char temp = getch();

	PlaySound(TEXT("space.wav"), NULL, SND_ASYNC); //스페이스바 눌렀을 때 효과음

	if (temp == ' ')
		return SUBMIT;
}

//플레이어의 색상 선택 메뉴
int colorsetting() {
	int select_color;

	gotoxy(50, 5);
	textcolor(BLACK, WHITE);
	printf("색상을 선택하세요.\n");
	gotoxy(41, 7);
	printf("기본 색상은 검은색입니다.\n");
	printf("\n");
	gotoxy(49, 9);
	textcolor(BLACK, WHITE);
	printf("0. BLACK\n");
	gotoxy(49, 10);
	printf("1.");
	textcolor(BLUE1, WHITE);
	printf(" BLUE\n");
	gotoxy(49, 11);
	textcolor(BLACK, WHITE);
	printf("2.");
	textcolor(GREEN1, WHITE);
	printf(" GREEN\n");
	gotoxy(49, 12);
	textcolor(BLACK, WHITE);
	printf("3.");
	textcolor(CYAN1, WHITE);
	printf(" CYAN\n");
	gotoxy(49, 13);
	textcolor(BLACK, WHITE);
	printf("4.");
	textcolor(RED1, WHITE);
	printf(" RED\n");
	gotoxy(49, 14);
	textcolor(BLACK, WHITE);
	printf("5.");
	textcolor(MAGENTA1, WHITE);
	printf(" MAGENTA\n");
	gotoxy(49, 15);
	textcolor(BLACK, WHITE);
	printf("6.");
	textcolor(YELLOW1, WHITE);
	printf(" YELLOW\n");

	gotoxy(49, 17);
	textcolor(BLACK, WHITE);
	printf("선택: ");
	scanf("%d", &select_color);
	PlaySound(TEXT("space.wav"), NULL, SND_ASYNC); //엔터키 쳤을 때 효과음
	printf("\n");
	printf("\n");

	//주어진 색을 선택하지 않았을 때 자동으로 검정색
	if (select_color != 0 && select_color != 1 && select_color != 2 && select_color != 3 && select_color != 4 && select_color != 5 && select_color != 6) {
		return select_color = 0;
	}
	system("cls");

	return select_color;
}

void main()
{
	unsigned char ch;
	int run_time, start_time, remain_time, last_remain_time;
	int speedup_time, speeddown_time, define_time;
	int i, x, y;
	char buf[100];
	int pos = 0;
	int player1_color, player2_color;

	removeCursor(); // 커서를 안보이게 한다

MAIN:
	system("cls");

	PlaySound(TEXT("Opening.wav"), NULL, SND_ASYNC);

	while (1) {
		title();
		int pos = menu();
		if (pos == 0)
			goto START;
		else if (pos == -24) {
			system("cls");
			explain();
		}
		else if (pos == -48)
			return 0;

		system("cls");
	}

START:

	system("cls");
	gotoxy(41, 5);
	textcolor(BLUE1, WHITE);
	printf("PLAYER1, ");
	player1_color = colorsetting();

	gotoxy(41, 5);
	textcolor(MAGENTA1, WHITE);
	printf("PLAYER2, ");

	player2_color = colorsetting();

	init_game();

	PlaySound(TEXT("Sound.wav"), NULL, SND_ASYNC);

	speedup_time = 0;
	speeddown_time = 0;
	define_time = 0;
	start_time = time(NULL);
	last_remain_time = remain_time = time_out;

	showscore(0, player1_color, player2_color);
	showscore(1, player1_color, player2_color);
	show_time(remain_time);
	while (1) {
		run_time = time(NULL) - start_time;
		if (run_time > speedup_time && (run_time % speedupinterval == 0)) {
			show_speedup();
			speedup_time = run_time;
		}
		if (run_time > speeddown_time && (run_time % speeddowninterval == 0)) {
			show_speeddown();
			speeddown_time = run_time;
		}
		if (run_time > define_time && (run_time % defineinterval == 0)) {
			show_define();
			define_time = run_time;
		}
		remain_time = time_out - run_time;
		if (remain_time < last_remain_time) {
			show_time(remain_time);
			last_remain_time = remain_time;
		}
		if (remain_time == 0)
			break;

		if (kbhit() == 1) {
			ch = getch();
			if (ch == ESC)
				break;
			if (ch == SPECIAL1 || ch == SPECIAL2) {
				ch = getch();
				switch (ch) {
				case UP:
				case DOWN:
				case LEFT:
				case RIGHT:
					player1(ch, player1_color);
					if (frame_count % p2_frame_sync == 0)
						player2(0, player2_color);
					break;
				default:
					if (frame_count % p1_frame_sync == 0)
						player1(0, player1_color);
					if (frame_count % p2_frame_sync == 0)
						player2(0, player2_color);
				}
			}
			else {
				switch (ch) {
				case UP2:
				case DOWN2:
				case LEFT2:
				case RIGHT2:
					if (frame_count % p1_frame_sync == 0)
						player1(0, player1_color);
					player2(ch, player2_color);
					break;
				default:
					if (frame_count % p1_frame_sync == 0)
						player1(0, player1_color);
					if (frame_count % p2_frame_sync == 0)
						player2(0, player2_color);
				}
			}
		}
		else {
			if (frame_count % p1_frame_sync == 0)
				player1(0, player1_color);
			if (frame_count % p2_frame_sync == 0)
				player2(0, player2_color);
		}
		Sleep(Delay);
		frame_count++;
	}
	cls(BLACK, WHITE);

	//이긴 플레이어의 색으로 "이긴 플레이어 WIN"이라는 문구가 나옴
	//점수가 같으면 "DRAW"가 나옴
	if (score[0] > score[1]) {
		gotoxy(50, 3);
		textcolor(BLACK, WHITE);

		gotoxy(10, 5);
		textcolor(player1_color, WHITE);
		printf(" ######   ##         ##     ##  ##   #######  ######      ##         ##   ##  ######   ##   ## \n");
		gotoxy(10, 6);
		printf(" ##   ##  ##         ##     ##  ##   ##       ##   ##    ###         ##   ##    ##     ###  ## \n");
		gotoxy(10, 7);
		printf(" ##   ##  ##        ####     ####    ##       ##   ##     ##         ## # ##    ##     ###  ## \n");
		gotoxy(10, 8);
		printf(" ######   ##        ## #      ##     #####    ######      ##         ## # ##    ##     ## # ## \n");
		gotoxy(10, 9);
		printf(" ##       ##       ######     ##     ##       ## ##       ##         ## # ##    ##     ## # ## \n");
		gotoxy(10, 10);
		printf(" ##       ##       ##   #     ##     ##       ##  ##      ##         ### ###    ##     ##  ### \n");
		gotoxy(10, 11);
		printf(" ##       ######  ###   ##    ##     #######  ##   ##     ##         ##   ##  ######   ##   ## \n");
	}
	else if (score[0] < score[1]) {
		gotoxy(50, 3);
		textcolor(BLACK, WHITE);

		gotoxy(10, 5);
		textcolor(player2_color, WHITE);
		printf(" ######   ##         ##     ##  ##   #######  ######    ####         ##   ##  ######   ##   ## \n");
		gotoxy(10, 6);
		printf(" ##   ##  ##         ##     ##  ##   ##       ##   ##  ##  ##        ##   ##    ##     ###  ## \n");
		gotoxy(10, 7);
		printf(" ##   ##  ##        ####     ####    ##       ##   ##      ##        ## # ##    ##     ###  ## \n");
		gotoxy(10, 8);
		printf(" ######   ##        ## #      ##     #####    ######      ##         ## # ##    ##     ## # ## \n");
		gotoxy(10, 9);
		printf(" ##       ##       ######     ##     ##       ## ##      ##          ## # ##    ##     ## # ## \n");
		gotoxy(10, 10);
		printf(" ##       ##       ##   #     ##     ##       ##  ##    ##           ### ###    ##     ##  ### \n");
		gotoxy(10, 11);
		printf(" ##       ######  ###   ##    ##     #######  ##   ##  ######        ##   ##  ######   ##   ## \n");
	}
	else {
		textcolor(GREEN1, WHITE);
		gotoxy(35, 5);
		printf(" #####    ######     ##     ##   ## \n");
		gotoxy(35, 6);
		printf(" ##  ##   ##   ##    ##     ##   ## \n");
		gotoxy(35, 7);
		printf(" ##   ##  ##   ##   ####    ## # ## \n");
		gotoxy(35, 8);
		printf(" ##   ##  ######    ## #    ## # ## \n");
		gotoxy(35, 9);
		printf(" ##   ##  ## ##    ######   ## # ## \n");
		gotoxy(35, 10);
		printf(" ##  ##   ##  ##   ##   #   ### ### \n");
		gotoxy(35, 11);
		printf(" #####    ##   ## ###   ##  ##   ## \n");
	}
	while (1) {
		int c1, c2;
		do {
			c1 = rand() % 16;
			c2 = rand() % 16;
		} while (c1 == c2);
		textcolor(c1, BLACK); //score의 배경색이 랜덤
		gotoxy(55, 15); 
		printf("score");

		textcolor(WHITE, BLACK);
		gotoxy(50, 17);
		printf(" PLAYER1 : %d ", score[0]);
		gotoxy(50, 19);
		printf(" PLAYER2 : %d ", score[1]);
		Sleep(10000);

		textcolor(BLACK, WHITE);
		system("cls");
		goto MAIN; // 10초 후에 main으로 돌아감
		Sleep(300);
	}
}

