#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <termios.h>

#define ROWS 30
#define COLS 80
#define MAX_SNAKE_SIZE 20
#define MAX_FOOD 5

void enable_raw_mode()
{
    struct termios t;
    tcgetattr(STDIN_FILENO, &t);
    t.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &t);
}

void disable_raw_mode()
{
    struct termios t;
    tcgetattr(STDIN_FILENO, &t);
    t.c_lflag |= (ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &t);
}

struct Point
{
    int x;
    int y;
};

struct Velocity
{
    int x;
    int y;
};

struct Snake
{
    struct Velocity vel;
    struct Point body[MAX_SNAKE_SIZE];
    char form;
    int size;
};

char GRID[ROWS][COLS+1] = {0};
struct Snake snake;
struct Point FOOD[MAX_FOOD];
int RUNNING = 1;
pthread_mutex_t LOCK;

void spawn_food(struct Snake snake)
{
    // Generate a random number between min and max (inclusive)
gen_ran:
    int random_x = rand() % (COLS + 1);
    int random_y = rand() % (ROWS + 1);
    struct Point food;

    for (int i=0; i<MAX_SNAKE_SIZE; i++)
    {
        if (random_x == snake.body[i].x && random_y == snake.body[i].y)
        {
            goto gen_ran;
        }
    }
exit:
    food.x = random_x;
    food.y = random_y;
    for (int i=0; i<MAX_FOOD; i++)
    {
        if (FOOD[i].x == -1 && FOOD[i].y == -1)
        {
            FOOD[i].x = random_x;
            FOOD[i].y = random_y;
            break;
        }
    }
}

void render(struct Snake snake)
{
    system("clear");
    // Clear the grid
    printf("\n");
    for (int j = 0; j < COLS; j++) 
    {
        GRID[0][j] = '-';
    }
    GRID[0][COLS] = '\n';
    for (int i = 1; i < ROWS - 2; i++) 
    {
        GRID[i][0] = '|';
        for (int j = 1; j < COLS-1; j++)
        {
            GRID[i][j] = ' ';
        }
        GRID[i][COLS-1] = '|';
        GRID[i][COLS] = '\n';
    }
    for (int j = 0; j < COLS; j++) 
    {
        GRID[ROWS - 1][j] = '-';
    }
    GRID[ROWS-1][COLS] = '\n';
    // Insert the snake into the grid
    for (int i=0; i<MAX_SNAKE_SIZE; i++)
    {
        if (snake.body[i].x > -1 && snake.body[i].y > -1)
        {
            GRID[snake.body[i].y][snake.body[i].x] = snake.form;
        }
    }
    for (int i=0; i<MAX_FOOD; i++)
    {
        if (FOOD[i].x > -1 && FOOD[i].y > -1)
        {
            GRID[FOOD[i].y][FOOD[i].x] = 'x';
        }
    }
    for (int i=0; i<ROWS; i++)
    {
        for (int j=0; j<COLS+1; j++)
        {
            printf("%c", GRID[i][j]);
        }
    }
}

void *get_input(void *ptr)
{
    struct Snake *snake = (struct Snake*)(ptr);
    while (RUNNING)
    {
        switch(getchar()) 
        { // the real value
            case 'w':
                // code for arrow up
                snake->vel.x = 0;
                snake->vel.y = 1;
                break;
            case 's':
                // code for arrow down
                snake->vel.x = 0;
                snake->vel.y = -1;
                break;
            case 'd':
                // code for arrow right
                snake->vel.x = 3;
                snake->vel.y = 0;
                break;
            case 'a':
                // code for arrow left
                snake->vel.x = -3;
                snake->vel.y = 0;
                break;
            case 'q':
                // code for arrow left
                pthread_mutex_lock(&LOCK);
                RUNNING = 0;
                pthread_mutex_unlock(&LOCK);
                break;
        }

    }
    return NULL;
}

void update_snake(struct Snake *snake)
{
    while (RUNNING)
    {
        for (int i = MAX_SNAKE_SIZE-1; i > 0; i--)
        {
            if (snake->body[i].x > -1 && snake->body[i].y > -1)
            {
                if (i > 0)
                {
                    snake->body[i].x = snake->body[i-1].x;
                    snake->body[i].y = snake->body[i-1].y;
                }
            }
        }
        snake->body[0].x = snake->body[0].x + snake->vel.x;
        snake->body[0].y = snake->body[0].y - snake->vel.y;

        for (int i=0; i<MAX_FOOD; i++)
        {
            if (snake->body[0].x == FOOD[i].x && snake->body[0].y == FOOD[i].y)
            {
                for (int j=1; j<MAX_SNAKE_SIZE; j++)
                {
                    if (snake->body[j].x == -1 && snake->body[j].y == -1)
                    {
                        snake->body[j].x = 1;
                        snake->body[j].y = 1;
                        break;
                    }
                }
                FOOD[i].x = -1;
                FOOD[i].y = -1;
            }
        }

        if (snake->body[0].x > COLS || snake->body[0].x < 0 || snake->body[0].y > ROWS || snake->body[0].y < 0)
        {
            pthread_mutex_lock(&LOCK);
            RUNNING = 0;
            pthread_mutex_unlock(&LOCK);
        }
    }
}


void main()
{
    snake.vel.x = 1;
    snake.vel.y = 0;
    snake.size = 1;
    snake.form = 'o';
    for (int i=0; i<MAX_SNAKE_SIZE; i++)
    {
        snake.body[i].x = -1;
        snake.body[i].y = -1;
    }
    snake.body[0].x = 0;
    snake.body[0].y = 0;

    for (int i=0; i<MAX_FOOD; i++)
    {
        FOOD[i].x = -1;
        FOOD[i].y = -1;
    }

    int food_counter = 50;

    pthread_t thread_1;

    enable_raw_mode();
    pthread_create(&thread_1, NULL, get_input, &snake);
    while (RUNNING)
    {
        if (food_counter == 0)
        {
            spawn_food(snake);
            food_counter = 50;
        }
        render(snake);
        update_snake(&snake);
        food_counter--;
        usleep(100000);
    }
    pthread_join(thread_1, NULL);
    disable_raw_mode();
}
