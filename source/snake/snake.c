/**
 * 
 * 贪吃蛇游戏
 *
 */
#include "snake.h"
#include "lib_syscall.h"

#include "dev/tty_t.h"

#include <stdio.h>
#include <stdlib.h>

// 屏幕范围
static int row_max = MAX_ROW;
static int col_max = MAX_COL;

static int is_quit = 0;

// 蛇
static snake_node_t* food;
static snake_t snake;

int main (int argc, char ** argv) {
	// 关闭按键回显
	ioctl(0, TTY_CMD_ECHO, 0, 0);

	// 欢迎信息
	show_welcome();
    
	// 判断是否要继续游戏
	while(!is_quit) {
		// 初始化游戏
		init_game();

    	// 启动游戏
		start_game();
	}

	// FIXME: 如果进程异常退出，将导致回显失败
	ioctl(0, TTY_CMD_ECHO, 1, 0);
	CLEAR_MAP();
    return 0;
}

static void show_welcome() {
	
	// 清空屏幕
	CLEAR_MAP();

	// 打印
    SHOW_STR(0, 0, "Welcome to snake game");
    SHOW_STR(1, 0, "Use a.w.s.d to move snake");
    SHOW_STR(2, 0, "Press any key to start game");
	FLUSH();
	
	// 无io缓冲
	setvbuf(stdin, NULL, _IONBF, 0);
    
	getchar();
}

static void init_game() {
	// 初始化地图
    init_map();

	// 初始化蛇头
    init_snake();

	// 创建食物
    create_food();

	FLUSH();
}

static void start_game() {
	int count;
	int cnt = 0, flag = 0;
	do {
		// 检测键盘缓冲区是否有输入
		ioctl(0, TTY_CMD_IN_COUNT, (int)&count, 0);

		// 若有输入, 则向对应方向移动
		if(count) {
			char ch = getchar();
			move_forward(ch);
		}
		// 若没有输入, 则每隔50次循环(即500ms ==> 0.5s)向前移动一次
		else if(++cnt % 50 == 0) {
			move_forward(snake.direction);
		}

		// 终止游戏
		if (snake.status == SNAKE_HIT_SELF || snake.status == SNAKE_HIT_WALL) {
			// 在屏幕中央开始打印失败信息
			int row = row_max / 2, col = col_max / 2;
			SHOW_STR(row, col, "GAME OVER.");
			SHOW_STR(row + 1, col, "Press Enter to continue, q to quit.");
			FLUSH();

			while(1) {
				char ch = getchar();
				if(ch == 'q' || ch == 'Q') {
					is_quit = 1;
					break;
				}
				else if(ch == '\n') {
					is_quit = 0;
					break;
				}
			}
			break;
		}

		// 睡眠10s
		msleep(10);
	}while (1);
}

static void init_map() {
	// 清空地图
	CLEAR_MAP();

	// 上下两行
    for(int col = 1; col < col_max - 1; col++) {
        SHOW_CHAR(0, col, '=');
        SHOW_CHAR(row_max -1, col, '=');
    }

	// 左右两列
    for(int row = 1; row < row_max - 1; row++) {
        SHOW_CHAR(row, 0, '|');
        SHOW_CHAR(row, col_max - 1, '|');
    }
}

static void init_snake() {
	snake.head = (snake_node_t*)malloc(sizeof(snake_node_t));
	snake.head->row = 10;
	snake.head->col = 20;
	snake.head->next = NULL;

	snake.status = SNAKE_HIT_NONE;
	snake.direction = PLAYER1_KEY_LEFT;
    
	SHOW_CHAR(snake.head->row, snake.head->col, '*');
}

static void create_food()  {
	food = (snake_node_t *)malloc(sizeof(snake_node_t));

	// 创建一个food
	do {
		// 随机food的位置, {0 ~ max - 3} + 1, 这样food一定不在墙上
		food->row = 1 + rand() % (row_max - 2);
		food->col = 1 + rand() % (col_max - 2);

		// 食物不能在蛇身上, 若在蛇上, 则重来
		snake_node_t* node = snake.head;
		while(node != NULL) {
			// 若food不在蛇上, 则显示出来并退出
			if ((food->row != snake.head->row) || (food->col != snake.head->col)) {
				SHOW_CHAR(food->row, food->col, '*');
				return;
			}
			node = node->next;
		}
	} while (1);
}

static void move_forward (int direction) {
	// 计算蛇头下一位置
	int next_row, next_col;
	switch (direction) {
	case PLAYER1_KEY_LEFT:
		next_row = snake.head->row;
		next_col = snake.head->col - 1;
		break;
	case PLAYER1_KEY_RIGHT:
		next_row = snake.head->row;
		next_col = snake.head->col + 1;
		break;
	case PLAYER1_KEY_UP:
		next_row = snake.head->row - 1;
		next_col = snake.head->col;
		break;
	case PLAYER1_KEY_DOWN:
		next_row = snake.head->row + 1;
		next_col = snake.head->col;
		break;
	default:
		return;
	}

	// 若蛇头的下一个位置与蛇身第二个节点不能重合
	// 即蛇头不能朝着蛇身的方向移动
	snake_node_t* next_node = snake.head->next;
	if (next_node != NULL) {
		if ((next_row == next_node->row) && (next_col == next_node->col)) {
			return;
		}
	}

	// 先不管有没有食物，都生成一个头部，然后前移, 若没有食物, 则将尾部删除
	// 这样可以避免所有蛇身节点的移动
	add_head(next_row, next_col);

	// 检查异常
	if(is_hit_self()) {
		snake.status = SNAKE_HIT_SELF;
		remove_tail();
	}
	else if(is_hit_wall()) {
		snake.status = SNAKE_HIT_WALL;
		remove_tail();
	}
	// 食物被吃掉, 回收，重新生成
	else if(is_hit_food()){
		free_food();
		create_food();
		snake.status = SNAKE_HIT_FOOD;
	}
	// 没有吃到食物，需要移除尾部
	else {
		remove_tail();
		snake.status = SNAKE_HIT_NONE;
	}
	snake.direction = direction;
	FLUSH();
}

static void add_head(int row, int col) {
	snake_node_t* node = (snake_node_t *)malloc(sizeof(snake_node_t));
	node->row = row;
	node->col = col;
	node->next = snake.head;

	// 设置新蛇头
	snake.head = node;

	SHOW_CHAR(row, col, '*');
}

static void remove_tail() {



	// 先定位curr到最后一个结点
	snake_node_t * pre = NULL;
	snake_node_t * curr = snake.head;
	while (curr->next) {
		pre = curr;
		curr = curr->next;
	}

	SHOW_CHAR(curr->row, curr->col, ' ');

	// 再移除
	pre->next = curr->next;
	curr->next = NULL;
	free(curr);
}

static void free_food() {
	free(food);
	food = NULL;
}

static int is_hit_self() {
	for(snake_node_t* node = snake.head->next; node != NULL; node = node->next) {
		if ((node->row == snake.head->row) && (node->col == snake.head->col)) {
			return 1;
		}
	}
	return 0;
}

static int is_hit_wall() {
	snake_node_t* node = snake.head;
	if(node->row <= 0 || node->col <= 0 || 
	   node->row >= row_max - 1 || node->col >= col_max - 1)
	{
		return 1;
	}
	return 0;
}

static int is_hit_food() {
	snake_node_t* node = snake.head;
	if(node->row == food->row && node->col == food->col) {
		return 1;
	}
	return 0;
}
