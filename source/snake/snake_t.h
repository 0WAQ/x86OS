#ifndef SNAKE_T_H
#define SNAKE_T_H

#include "common/types.h"

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
	u32_t row;
	u32_t col;
	struct _snake_node_t *next;
}snake_node_t;

/**
 * @brief 蛇结构
 */
typedef struct _snake_t {
	snake_node_t* head;		// 蛇头
	u32_t direction;			// 蛇当前朝向 
	enum {
		SNAKE_HIT_NONE,
		SNAKE_HIT_SELF,
		SNAKE_HIT_WALL,
		SNAKE_HIT_FOOD,
	} status;				// 蛇状态
}snake_t;


#endif // SNAKE_T_H
