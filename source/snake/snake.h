/**
 * 
 * 贪吃蛇游戏头文件
 *
 */
#ifndef CMD_H
#define CMD_H

#define ESC_CMD2(Pn, cmd)		    "\x1b["#Pn#cmd
#define ESC_CLEAR_SCREEN		    ESC_CMD2(2, J)	// 擦除整屏幕

#define MAX_ROW					25
#define MAX_COL					80

#define PLAYER1_KEY_UP			'w'
#define PLAYER1_KEY_DOWN		's'
#define PLAYER1_KEY_LEFT		'a'
#define PLAYER1_KEY_RIGHT		'd'
#define PLAYER1_KEY_QUITE		'q'

#define SHOW_CHAR(row, col, c)  printf("\x1b[%d;%dH%c\x1b[%d;%dH", row, col, c, row, col)
#define SHOW_STR(row, col, str) printf("\x1b[%d;%dH%s", row, col, str)
#define CLEAR_MAP() 			printf("%s", ESC_CLEAR_SCREEN)
#define FLUSH()					fflush(stdout)

/**
 * @brief 蛇身节点
 */
typedef struct _snake_node_t {
	int row;
	int col;
	struct _snake_node_t *next;
}snake_node_t;

/**
 * @brief 蛇结构
 */
typedef struct _snake_t {
	snake_node_t* head;		// 蛇头
	int direction;			// 蛇当前朝向 
	enum {
		SNAKE_HIT_NONE,
		SNAKE_HIT_SELF,
		SNAKE_HIT_WALL,
		SNAKE_HIT_FOOD,
	} status;				// 蛇状态
}snake_t;

/**
 * @brief 小游戏的入口
 */
int main(int argc, char** argv);

/**
 * @brief 打印欢迎信息
 */
static void show_welcome();

/**
 * @brief 初始化游戏
 */
static void init_game();

/**
 * @brief 启动游戏
 */
static void start_game();

/**
 * @brief 初始化地图
 */
static void init_map();

/**
 * @brief 初始化蛇头
 */
static void init_snake();

/**
 * @brief 创建食物
 */
static void create_food();

/**
 * @brief 向direction移动
 */
static void move_forward(int direction);

/**
 * @brief 给蛇添加头部
 */
static void add_head(int row, int col);

/**
 * @brief 移除蛇尾的最后一个结点
 */
static void remove_tail();

/**
 * @brief 释放掉食物
 */
static void free_food();

/**
 * @brief 判断蛇是否咬到自己
 */
static int is_hit_self();

/**
 * @brief 判断是否碰到墙
 */
static int is_hit_wall();

/**
 * @brief 判断是否吃到食物
 */
static int is_hit_food();

#endif
