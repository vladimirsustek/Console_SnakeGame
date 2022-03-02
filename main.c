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

#define ARENA_MAX_X			(uint16_t)(35)
#define ARENA_MAX_Y			(uint16_t)(20)
#define ARENA_MIN_X			(uint16_t)(1)
#define ARENA_MIN_Y			(uint16_t)(1)

#define SNAKE_INIT_X_CORD	(uint16_t)(15)
#define SNAKE_INIT_Y_CORD	(uint16_t)(15)


#define SNAKE_MOVE_OK		(uint16_t)(0)
#define SNAKE_MOVE_CRASH	(uint16_t)(1)
#define SNAKE_WON			(uint16_t)(2)

#define FOOD_MAX_X			(uint16_t)(33)
#define FOOD_MIN_X			(uint16_t)(3)
#define FOOD_MAX_Y			(uint16_t)(18)
#define FOOD_MIN_Y			(uint16_t)(3)

#define FOOD_MAX_ITER		(uint16_t)(3)

#define GENERAL_ERROR		(uint16_t)(-1)
#define INVALID_COORDS		(uint16_t)(-1)

typedef enum { UP = 'w', DOWN = 's', LEFT =  'a', RIGHT = 'd', PAUSE = 'p', QUIT = 'q' } snake_dir_e;

typedef enum { WAITING, PLACED, EATEN } foodstate_e;

typedef enum { PLAYING, CRASHED, WON } snake_state_e;

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
	snake_state_e state;
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
void platform_get_control(snake_t* snake);

void snake_init(snake_t* snake)
{
	snake->length = SNAKE_INIT_LNG;
	snake->direction = PAUSE;

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

void snake_move(snake_t* snake)
{
	if (NULL == snake || PAUSE == snake->direction)
	{
		return;
	}
	snake->ghost = snake->body[0];
	memcpy(&snake->body[0], &snake->body[1], sizeof(coord_t) * (snake->length - 1));

	switch (snake->direction)
	{
	case UP:
	{
		if ((snake->body[snake->length - 1].y - 1) == ARENA_MIN_Y)
		{
			snake->state = CRASHED;
			break;
		}
		for (int idx = 0; idx < snake->length; idx++)
		{
			if (((snake->body[snake->length - 1].y - 1) == snake->body[idx].y) &&
				((snake->body[snake->length - 1].x) == snake->body[idx].x))
			{
				snake->state = CRASHED;
			}
		}
		snake->body[snake->length - 1].y--;
	}
	break;
	case DOWN:
	{
		if ((snake->body[snake->length - 1].y + 1) == ARENA_MAX_Y)
		{
			snake->state = CRASHED;
			break;
		}
		for (int idx = 0; idx < snake->length; idx++)
		{
			if (((snake->body[snake->length - 1].y + 1) == snake->body[idx].y) &&
				((snake->body[snake->length - 1].x) == snake->body[idx].x))
			{
				snake->state = CRASHED;
			}
		}

		snake->body[snake->length - 1].y++;
	}
	break;
	case RIGHT:
	{
		if ((snake->body[snake->length - 1].x + 1) == ARENA_MAX_X)
		{
			snake->state = CRASHED;
			break;
		}
		for (int idx = 0; idx < snake->length; idx++)
		{
			if (((snake->body[snake->length - 1].x + 1) == snake->body[idx].x) &&
				((snake->body[snake->length - 1].y) == snake->body[idx].y))
			{
				snake->state = CRASHED;
			}
		}
		snake->body[snake->length - 1].x++;
	}
	break;
	case LEFT:
	{
		if ((snake->body[snake->length - 1].x - 1) == ARENA_MIN_X)
		{
			snake->state = CRASHED;
			break;
		}
		for (int idx = 0; idx < snake->length; idx++)
		{
			if (((snake->body[snake->length - 1].x - 1) == snake->body[idx].x) &&
				((snake->body[snake->length - 1].y) == snake->body[idx].y))
			{
				snake->state = CRASHED;
			}
		}
		snake->body[snake->length - 1].x--;
	}
	break;
	default:
	{
	}
	}

	if (snake->length == SNAKE_WON_LIMIT)
	{
		snake->state = WON;
	}
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


void snake_place_food(snake_t* snake, food_t* food, uint32_t tick)
{
	if (0 == gPrgCycle % 10 || food->time_elapsed)
	{
		if (food->state != PLACED)
		{
			if (GENERAL_ERROR == generate_food(snake, food))
			{
				platform_fatal();
			}
			else
			{
				food->time_elapsed = 0;
				food->state = PLACED;
			}
		}
		else
		{
			food->time_elapsed = 1;
		}

	}

}
void snake_haseaten(snake_t* snake, food_t* food)
{
	if ((snake->body[snake->length - 1].x == food->coord.x)
		&& (snake->body[snake->length - 1].y == food->coord.y))
	{
		/* Needed temporary copy for shifting the whole array right - for embedded*/
		coord_t tempSnake[SNAKE_MAX_LNG] = { 0 };
		memcpy(tempSnake, &(snake->body[0]), (size_t)snake->length * sizeof(coord_t));

		/* Just append the ghost to the end, increment length and disable ghost*/
		memcpy(&(snake->body[1]), tempSnake, snake->length*sizeof(coord_t));
		snake->body[0] = snake->ghost;
		snake->ghost.x = INVALID_COORDS;
		snake->ghost.y = INVALID_COORDS;
		snake->length++;

		food->state = EATEN;
	}
}

void snake_inform(snake_t* snake)
{
	switch (snake->state)
	{
	case PLAYING: 
	{
		/* Keep playing*/
		snake->state = snake->state;
	}
	break;
	case CRASHED :
	{
		platform_showInformal("Snake Crashed           ", (uint16_t)strlen("Snake Crashed           "));
	}
	break;
	case WON:
	{
		platform_showInformal("Snake won           ", (uint16_t)strlen("Snake won           "));
	}
	break;
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

	platform_showInformal("w-a-s-d control, p-pause", (uint16_t)strlen("w-a-s-d control, p-pause"));

	while (snake.direction != QUIT)
	{	
		platform_get_control(&snake);

		snake_move(&snake);

		if (snake.state != PLAYING)
		{
			snake_inform(&snake);
			platform_sleep(3000);
			break;
		};

		snake_haseaten(&snake, &food);
		snake_display(&snake);
		snake_place_food(&snake, &food, gPrgCycle);

		gPrgCycle++;

		platform_sleep(100);
	}

	/* Any key to close */
	platform_showInformal("Game is being quit\n", strlen("Game is being quit\n"));
	platform_sleep(3000);
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

void platform_get_control(snake_t * snake)
{
	snake_dir_e direction = 0;
	static prev_direction = RIGHT;
	if (_kbhit())
	{
		direction = (snake_dir_e)_getch();

		if ((direction != LEFT) && (direction != RIGHT) && (direction != UP) &&
			(direction != DOWN) && (direction != PAUSE) && (direction != QUIT))
		{
			prev_direction = snake->direction;
			snake->direction = PAUSE;
		}
		else
		{
			if (direction == PAUSE)
			{
				if (snake->direction != PAUSE)
				{
					prev_direction = snake->direction;
					snake->direction = PAUSE;
				}
				else
				{
					snake->direction = prev_direction;
				}
			}

			else
			{
				if ((snake->direction != PAUSE) &&
					!(snake->direction == LEFT && direction == RIGHT) &&
					!(snake->direction == RIGHT && direction == LEFT) &&
					!(snake->direction == UP && direction == DOWN) &&
					!(snake->direction == DOWN && direction == UP))
				{
					snake->direction = direction;
				}
			}
		}
	}
}
