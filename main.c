/*
* Created by Vladimir Sustek 26.2.2022
* Use/copy/distribute me as you wish
* Bless you all
*/


/* Needed for printf*/
#include <stdio.h>

/* Needed for uints*/
#include <stdint.h>

/* Needed for SetConsoleCursorInfo and Sleep*/
#include <windows.h>

/* Needed for getch - no*/
#include <conio.h>

#define SNAKE_MAX_LNG		(uint16_t)(50)
#define SNAKE_INIT_LNG		(uint16_t)(3)
#define SNAKE_WON_LIMIT		(uint16_t)(SNAKE_MAX_LNG - 1)

#define ARENA_MAX_X			(uint16_t)(70)
#define ARENA_MAX_Y			(uint16_t)(20)
#define ARENA_MIN_X			(uint16_t)(1)
#define ARENA_MIN_Y			(uint16_t)(1)

#define SNAKE_INIT_X_CORD	(uint16_t)(15)
#define SNAKE_INIT_Y_CORD	(uint16_t)(15)


#define SNAKE_MOVE_OK		(uint16_t)(0)
#define SNAKE_MOVE_CRASH	(uint16_t)(1)
#define SNAKE_WON			(uint16_t)(2)

#define FOOD_MAX_X			(uint16_t)(68)
#define FOOD_MIN_X			(uint16_t)(3)
#define FOOD_MAX_Y			(uint16_t)(18)
#define FOOD_MIN_Y			(uint16_t)(3)

#define FOOD_MAX_ITER		(uint16_t)(3)

#define GENERAL_ERROR		(uint16_t)(-1)
#define INVALID_COORDS		(uint16_t)(-1)

typedef enum {UP = 'w', DOWN = 's', LEFT =  'a', RIGHT = 'd', STOP = 'q' } snake_dir_e;

typedef enum { WAITING, PLACED, EATEN } foodstate_e;

typedef struct coord_tag
{
	uint16_t x;
	uint16_t y;
} coord_t;
;
typedef struct snake_tag
{
	snake_dir_e direction;
	coord_t body[SNAKE_MAX_LNG];
	uint16_t length;
	coord_t ghost;
} snake_t;

typedef struct food_tag
{
	coord_t coord;
	foodstate_e state;
	uint16_t time_elapsed;
} food_t;

static uint32_t gPrgCycle = 0;

/* Platform dependencies */

void platform_init(void);
void platform_printXY(uint16_t x, uint16_t y, char symbol);
uint16_t platform_randomize(void);
void platform_sleep(uint16_t ms);
void platform_showInformal(char* str, uint16_t length);
void platform_fatal(void);

void snake_init(snake_t* snake)
{
	snake->length = SNAKE_INIT_LNG;
	snake->direction = RIGHT;

	for (int idx = 0; idx < SNAKE_INIT_LNG; idx++)
	{
		snake->body[idx].x = SNAKE_INIT_X_CORD + idx;
		snake->body[idx].y = SNAKE_INIT_Y_CORD;
	}

}

void snake_display(snake_t* snake)
{
	if (-1 != snake->ghost.x && -1 != snake->ghost.y)
	{
		platform_printXY(snake->ghost.x, snake->ghost.y, ' ');
	}
	for (int idx = 0; idx < snake->length; idx++)
	{
		platform_printXY(snake->body[idx].x, snake->body[idx].y, '#');
	}
}


void snake_diplay_borders(void)
{
	for (int x = 0; x < ARENA_MAX_X; x++)
	{
		platform_printXY(x, ARENA_MIN_Y, '-');
	}
	for (int x = 0; x < ARENA_MAX_X; x++)
	{
		platform_printXY(x, ARENA_MAX_Y, '-');
	}
	for (int y = 0; y < ARENA_MAX_Y; y++)
	{
		platform_printXY(ARENA_MIN_X, y, '|');
	}
	for (int y = 0; y < ARENA_MAX_Y; y++)
	{
		platform_printXY(ARENA_MAX_X, y, '|');
	}
}

uint16_t snake_move(snake_t* snake)
{
	uint16_t result = SNAKE_MOVE_OK;

	snake->ghost = snake->body[0];
	memcpy(&snake->body[0], &snake->body[1], sizeof(coord_t) * (snake->length - 1));

	switch (snake->direction)
	{
	case UP : 
	{
		if ((snake->body[snake->length - 1].y - 1) == ARENA_MIN_Y)
		{
			result = SNAKE_MOVE_CRASH;
			break;
		}
		for (int idx = 0; idx < snake->length; idx++)
		{
			if (((snake->body[snake->length - 1].y - 1) == snake->body[idx].y) && 
				((snake->body[snake->length - 1].x) == snake->body[idx].x))
			{
				result = SNAKE_MOVE_CRASH;
			}
		}
		snake->body[snake->length - 1].y--;
	}
	break;
	case DOWN :
	{
		if ((snake->body[snake->length - 1].y + 1) == ARENA_MAX_Y)
		{
			result = SNAKE_MOVE_CRASH;
			break;
		}
		for (int idx = 0; idx < snake->length; idx++)
		{
			if (((snake->body[snake->length - 1].y + 1) == snake->body[idx].y) &&
				((snake->body[snake->length - 1].x) == snake->body[idx].x))
			{
				result = SNAKE_MOVE_CRASH;
			}
		}

		snake->body[snake->length - 1].y++;
	}
	break;
	case RIGHT:
	{
		if ((snake->body[snake->length - 1].x + 1) == ARENA_MAX_X)
		{
			result = SNAKE_MOVE_CRASH;
			break;
		}
		for (int idx = 0; idx < snake->length; idx++)
		{
			if (((snake->body[snake->length - 1].x + 1) == snake->body[idx].x) &&
				((snake->body[snake->length - 1].y) == snake->body[idx].y))
			{
				result = SNAKE_MOVE_CRASH;
			}
		}
		snake->body[snake->length - 1].x++;
	}
	break;
	case LEFT :
	{
		if ((snake->body[snake->length - 1].x - 1) == ARENA_MIN_X)
		{
			result = SNAKE_MOVE_CRASH;
			break;
		}
		for (int idx = 0; idx < snake->length; idx++)
		{
			if (((snake->body[snake->length - 1].x - 1) == snake->body[idx].x) &&
				((snake->body[snake->length - 1].y) == snake->body[idx].y)) 
			{
				result = SNAKE_MOVE_CRASH;
			}
		}
			snake->body[snake->length - 1].x--;
	}
	break;
	default : {}
	}

	if (snake->length == SNAKE_WON_LIMIT)
	{
		result = SNAKE_WON;
	}
	return result;
}

uint16_t generate_food(snake_t* snake, food_t *food)
{
	uint16_t isInvalid = 0;
	uint16_t iter = 0;

	do
	{
		food->coord.x = (uint16_t)((platform_randomize() % (FOOD_MAX_X - FOOD_MIN_X + 1)) + FOOD_MIN_X);
		food->coord.y = (uint16_t)((platform_randomize() % (FOOD_MAX_Y - FOOD_MIN_Y + 1)) + FOOD_MIN_Y);

		for (int idx = 0; idx < snake->length; idx++)
		{
			if ((snake->body[idx].x == food->coord.x) && 
				(snake->body[idx].y == food->coord.y))
			{
				isInvalid = GENERAL_ERROR;
				break;
			}
		}
		if (iter > FOOD_MAX_ITER)
		{
			break;
		}

	} while (isInvalid);

	if (!isInvalid)
	{
		platform_printXY(food->coord.x, food->coord.y, 'o');
	}

	return isInvalid;

}

void snake_eaten(snake_t* snake, food_t* food)
{
	if ((snake->body[snake->length - 1].x == food->coord.x)
		&& (snake->body[snake->length - 1].y == food->coord.y))
	{
		/* Just append the ghost to the end, increment length and disable ghost*/
		memcpy(&(snake->body[1]), &(snake->body[0]), snake->length*sizeof(coord_t));
		snake->body[0] = snake->ghost;
		snake->ghost.x = INVALID_COORDS;
		snake->ghost.y = INVALID_COORDS;
		snake->length++;

		food->state = EATEN;
	}
}

int main(void)
{
	snake_t snake = { 0 };
	food_t food = { 0 };

	platform_init();
	snake_diplay_borders();

	snake_init(&snake);
	snake_display(&snake);

	while (snake.direction != STOP)
	{	
		if (_kbhit()) snake.direction = (snake_dir_e)_getch();
		uint16_t state = snake_move(&snake);
		snake_eaten(&snake, &food);
		

		if (state == SNAKE_MOVE_CRASH)
		{
			platform_showInformal("Snake Crashed\n", (uint16_t)strlen("Snake Crashed\n"));
			break;
		}
		if (state == SNAKE_WON)
		{
			platform_showInformal("Snake won\n", (uint16_t)strlen("Snake won\n"));
			break;
		}
		snake_display(&snake);

		gPrgCycle++;

		if (0 == gPrgCycle % 10 || food.time_elapsed)
		{
			if (food.state != PLACED)
			{
				if (GENERAL_ERROR == generate_food(&snake, &food))
				{
					platform_fatal();
				}
				else
				{
					food.time_elapsed = 0;
					food.state = PLACED;
				}
			}
			else
			{
				food.time_elapsed = 1;
			}

		}

		platform_sleep(100);
	}

	/* Any key to close */
	getchar();
	return 0;
}


/* Platform dependencies */

void hidecursor(void)
{
	HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_CURSOR_INFO info;
	info.dwSize = 100;
	info.bVisible = FALSE;
	SetConsoleCursorInfo(consoleHandle, &info);
}

void platform_init(void)
{
	hidecursor();
}
void platform_printXY(uint16_t x, uint16_t y, char symbol)
{
	COORD c;
	c.X = (short)x;
	c.Y = (short)y;
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), c);
	printf("%c", symbol);

}

void platform_showInformal(char* str, uint16_t length)
{
	platform_printXY(0, 0, '>');

	for (int idx = 0; idx < length; idx++)
	{
		printf("%c", str[idx]);
	}
}

uint16_t platform_randomize(void)
{
	return (uint16_t)rand();
}

void platform_sleep(uint16_t ms)
{
	Sleep(ms);
}

void platform_fatal(void)
{
	platform_showInformal("FatalError\n", strlen("FatalError\n"));
	while (1);
}