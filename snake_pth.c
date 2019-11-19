#include <ncurses.h>
#include <pthread.h>
#include <time.h>

//snake_board dimensions. they are 2 units longer because of the boarders.
#define WORLD_WIDTH 62
#define WORLD_HEIGHT 22

//maximum of the snake's length
#define SNAKEY_LENGTH 8

//functions prototype
void* get_input(void* rank);
void* move_snake(void* rank);
void* make_frog_mine(void* rank);
void reset_for_next_round();
void reset_all();
void initialaize();
void move_it(int number);
void check_collision();
void print_winner();

//defining a new typr for direction of the snake
typedef enum direct {up,down,left,right} direction;

//defining a struct for elements place (x,y) on the board
typedef struct spart {
    int x;
    int y;
} part;

//define windows
WINDOW *snakes_world;
WINDOW *score_board;
WINDOW *message_board;
WINDOW *first_board;
WINDOW *final_board;

//defining a struct for walls places made of a 5 elements array. each element is type of part (x,y).
struct walls {
    part place[5];
};

//define 3 walls with type walls
struct walls wall[3];

//define a struct for frogs/mines made of a 5 elements array for the places of each frog/mine and a boolean variable for detecting wether the frog/mine is hit or not
struct frogs_mines {
    part place [5];
    bool alive [5];
};

//define a frog and a mine of type frogs_mies
struct frogs_mines frog, mine;

//define a struct for snakes. each variable is explained inside.
struct snakes {
    //an array for the place of each part of the snake on the snake_board
    part object [SNAKEY_LENGTH];
    //a character for the place of the snake
    char face;
    //a variable for the moving direction of the snake
    int dir;
    //a variable for the size of the snake between 1 & 8
    int size;
    //a variable for the score of the snake
    int score;
    //a variable for the speed of snake
    int speed_counter;
    //a character for the head of the snake depend on the direction it moves
    char head;
    //a varible for detecting if the snake is alive or dead
    bool alive;
};

//define a 3 elements array of type snakes for 3 snake
struct snakes snake[3];// = {snake[0], snake[1], snake[2]};

//variable for x & y of the starting point of the snake_board according to the screen size & board size. the snake_board is in the center of the screen.
int offsetx, offsety;
//input character from the keyboard
int ch;
//round number between 1 & 4
int round_number;
//a variable to show wether the snake may move in this 0.5ms or it may stay.
int indicator;
//variables for restarting, quiting & pausing the game.
bool restart, quit, restart_2, quit_2, pause;

int main(int argc, char *argv[]) {
    //initialize every thing for the screen.
    initialaize();
    //define 3 thread handles for getting key, moving snakes & making frogs/mines.
    pthread_t thread_handle_1, thread_handle_2, thread_handle_3;
    //if the scren size is smaller than required, stay in the while
    while(COLS < 62 || LINES < 32)
    {
        //clear the screen.
        clear();
        //show a message to expand the screen
        mvprintw(1, 1, "PLEASE EXPAND THE SCREEN SIZE...");
        //refresh the screen.
        refresh();
        
        usleep(100000);

    }
    
    offsetx = (COLS - WORLD_WIDTH) / 2;
    offsety = (LINES - WORLD_HEIGHT) / 2;
    
    //make the game board
    snakes_world = newwin(WORLD_HEIGHT, WORLD_WIDTH, offsety, offsetx);
    //make score board to show scores during the game.
    score_board = newwin(3, WORLD_WIDTH, offsety + WORLD_HEIGHT, offsetx);
    //make message board to show restart, quit & pause messages
    message_board = newwin(5, WORLD_WIDTH, offsety - 5 , offsetx);
    //make first_board to show the facts about the game in the start
    first_board = newwin(10, WORLD_WIDTH, (LINES - 10) / 2, offsetx);
    //make final_board to show scores & the winner at the end of the game
    final_board = newwin(7, WORLD_WIDTH, (LINES - 7) / 2, offsetx);
    //make all the quit and restart variables FALSE in the start
    quit = FALSE;
    
    quit_2 = FALSE;
    
    restart = FALSE;
    
    restart_2 = FALSE;
    //define snakes faces
    snake[0].face = '#';
    snake[1].face = '&';
    snake[2].face = 'O';
    //while quit_2 variable is not TRUE, run the game
    while(quit_2 == FALSE)
    {
        //clear the whole screen
        clear();
        //refresh the whole screen
        refresh();
        //clear the first_board
        wclear(first_board);
        //show the message at the end of the first board.
        mvwprintw(first_board, 8, 1, "PRESS s to START the GAME...");
        //show the character that snake_0 is made of
        mvwprintw(first_board, 1, 1, "Snake_0 is made of:");
        mvwaddch(first_board, 1, 21, snake[0].face | COLOR_PAIR(3));
        //show moving keys for snake_0
        mvwprintw(first_board, 2, 1, "Snake_0 moving keys are: W & S & A & D");
        //show the character that snake_1 is made of
        mvwprintw(first_board, 3, 1, "Snake_1 is made of:");
        mvwaddch(first_board, 3, 21, snake[1].face | COLOR_PAIR(5));
        //show moving keys for snake_1
        mvwprintw(first_board, 4, 1, "Snake_1 moving keys are: I & J & K & L");
        //show the character that snake_2 is made of
        mvwprintw(first_board, 5, 1, "Snake_2 is made of:");
        mvwaddch(first_board, 5, 21, snake[2].face | COLOR_PAIR(6));
        //show moving keys for snake_2
        mvwprintw(first_board, 6, 1, "Snake_2 moving keys are: ARROW Keys");
        //make border for first_board
        box(first_board, 0, 0);
        //refresh the first board
        wrefresh(first_board);
        //while the user has not pressed 's' key, stay in the while and do not start the game.
        while(1)
        {
            ch = getch();
            if (ch == 's')
                break;
            if (ch == 'S')
                break;
        }
        //clear the whole screen
        clear();
        //refresh the whole screen
        refresh();
        //clear the message_board
        wclear(message_board);
        //reset all the variables for a new game
        reset_all();
        //create threads
        //thread to get key from the keyboard
        pthread_create(&thread_handle_1, NULL, get_input, (void*)0);
        //thread to move snake and & referee
        pthread_create(&thread_handle_2, NULL, move_snake, (void*)1);
        //thread to make mine or frog if a mine or frog hit.
        pthread_create(&thread_handle_3, NULL, make_frog_mine, (void*)2);
        
        //join the threads after finishing
        pthread_join(thread_handle_1, NULL);
        pthread_join(thread_handle_2, NULL);
        pthread_join(thread_handle_3, NULL);
        //if the 4 rounds of the game is finished and the game is not restarted or quited, show the winner
        if(round_number == 5)
        {
            //clear the whole screen
            clear();
            //refresh the whole screen
            refresh();
            //clear the message_board
            wclear(message_board);
            //print restart & quit message in the message board.
            mvwprintw(message_board, 1, 1, "PRESS r to RESTART...");
            mvwprintw(message_board, 3, 1, "PRESS q to QUIT...");
            //make border for the message_board
            box(message_board, 0, 0);
            //refresh the message_board
            wrefresh(message_board);
            //print the winner
            print_winner();
        }
        //if 'r' or 'q' has not been pressed for restart or quit, wait for them to be pressed.
        if(restart_2 != TRUE && quit_2 != TRUE)
        {
            while(1)
            {
                ch = getch();
                if (ch == 'r')
                    break;
                if (ch == 'R')
                    break;
                if (ch == 'q')
                {
                    quit_2 = TRUE;
                    break;
                }
                if (ch == 'Q')
                {
                    quit_2 = TRUE;
                    break;
                }
            }
        }
    }
    //delete all the boards
    delwin(snakes_world);
    
    delwin(score_board);
    
    delwin(message_board);
    
    delwin(first_board);
    
    delwin(final_board);
    //end the windows
    endwin();

    return 0;

}

void print_winner(){
    //clear the final_board
    wclear(final_board);
    //show all the snakes scores
    mvwprintw(final_board, 1, 1, "Snake_0 Score: %d", snake[0].score);
    mvwprintw(final_board, 2, 1, "Snake_1 Score: %d", snake[1].score);
    mvwprintw(final_board, 3, 1, "Snake_2 Score: %d", snake[2].score);
    //show the winner
    if(snake[0].score > snake[1].score && snake[0].score > snake[2].score)
        mvwprintw(final_board, 5, 1, "Snake_0 is Winner");
                  
    else if(snake[1].score > snake[0].score && snake[1].score > snake[2].score)
        mvwprintw(final_board, 5, 1, "Snake_1 is Winner");
                  
    else if(snake[2].score > snake[0].score && snake[2].score > snake[1].score)
        mvwprintw(final_board, 5, 1, "Snake_2 is Winner");
    //show a message if there is no winner
    else
        mvwprintw(final_board, 5, 1, "There is No Winner");
    //make border for final_board
    box(final_board, 0, 0);
    //refresh final_board
    wrefresh(final_board);
    
}
//thread to move snake & referee
void* move_snake(void* rank) {
    
    int i, j, k;
    //while the round is less than 5 and the quit & restart variables are FALSE, run the game
    while(round_number <= 4 && quit_2 == FALSE  && restart_2 == FALSE)
    {
        //clear the snakes_world
        wclear(snakes_world);
        //clear the score_board
        wclear(score_board);
        //move all the snakes
        for(j = 0; j < 3; j++)
        {
            //if a snake is alive, move it
            if(snake[j].alive)
                move_it(j);
        }

        //increase indicator
        indicator++;
        //keep indicator between 1 & 4
        if(indicator == 5)
            indicator = 1;
            
        
        for(i = 0; i < 5; i++)
        {
            //show all the alive frogs with green 'F'
            if(frog.alive[i] == TRUE)
                mvwaddch(snakes_world, (frog.place[i]).y, (frog.place[i]).x, 'F' | COLOR_PAIR(2));
            //show all the alive mines with red 'M'
            if(mine.alive[i] == TRUE)
                mvwaddch(snakes_world, (mine.place[i]).y, (mine.place[i]).x, 'M' | COLOR_PAIR(1));
        }
        for(i = 0; i < 3; i++)
        {
            for(k = 0; k < 5; ++k)
            {
                if(i != 2)
                    //show the vertical walls
                    mvwaddch(snakes_world, (wall[i].place[k]).y, (wall[i].place[k]).x, '|' | COLOR_PAIR(4));
                else
                    //show horizantal walls
                    mvwaddch(snakes_world, (wall[i].place[k]).y, (wall[i].place[k]).x, '-' | COLOR_PAIR(4));
            }
        }

        //check for all the collisions
        check_collision();
        //make border for snakes_world
        box(snakes_world, 0, 0);
        //make border for score_board
        box(score_board, 0, 0);
        //refresh snakes_world
        wrefresh(snakes_world);
        //refresh score_board
        wrefresh(score_board);
        //if quit & restart & pause variable is FALSE, show their messages
        if(quit == FALSE && restart == FALSE && pause == FALSE)
        {
            wclear(message_board);
            
            mvwprintw(message_board, 1, 1, "PRESS r to RESTART...");
            mvwprintw(message_board, 2, 1, "PRESS q to QUIT...");
            mvwprintw(message_board, 3, 1, "PRESS p to PAUSE...");
            
            box(message_board, 0, 0);
            
            wrefresh(message_board);
        }
        //if quit is TRUE, show the message if the user is sure to quit
        if(quit == TRUE)
        {
            wclear(message_board);
            mvwprintw(message_board, 1, 1, "ARE YOU SURE YOU WANT TO QUIT ? (Y/N)");
            box(message_board, 0, 0);
            wrefresh(message_board);
            //stay here until the user says wether is sure to quit or not
            while(quit == TRUE && quit_2 == FALSE);
        }
        //if restart is TRUE, show the message if the user is sure to restart
        if(restart == TRUE)
        {
            wclear(message_board);
            mvwprintw(message_board, 1, 1, "ARE YOU SURE YOU WANT TO RESTART ? (Y/N)");
            box(message_board, 0, 0);
            wrefresh(message_board);
            //stay here until the user says wether is sure to restart or not
            while(restart == TRUE && restart_2 == FALSE);
        }
        //if the pause is TRUE, show the message that press 'r' to resume
        if(pause == TRUE)
        {
            wclear(message_board);
            mvwprintw(message_board, 1, 1, "PRESS r to RESUME...");
            box(message_board, 0, 0);
            wrefresh(message_board);
            //stay here until the user press 'r'
            while(pause == TRUE);
        }
        //if restart & quit variables are FALSE, delay 0.5ms
        if(restart_2 == FALSE && quit_2 == FALSE)
            usleep(500000);
        
    }

}
//thread to make frog/mine if a frog/mine hit
void* make_frog_mine(void* rank)
{
    int i, j, k, m, x, y, wall_0, wall_1, wall_2;
    //while the round is less than 5 and the quit & restart variables are FALSE, run the game
    while(round_number <= 4 && quit_2 == FALSE && restart_2 == FALSE)
    {
        for(m = 0; m < 5; ++m)
        {
            //if a frog is dead, go in the while and don't come out until the frog becomes alive
            while(frog.alive[m] == FALSE)
            {
                x = rand() % 60 + 1;
                y = rand() % 20 + 1;
                //check wether the chosen random place is not on an alive snake
                for(j = 0; j < 3; ++j)
                {
                    if(snake[j].alive == FALSE)
                        continue;
                    for(i = 0; i < snake[j].size; ++i)
                    {
                        if((snake[j].object[i]).x == x && (snake[j].object[i]).y == y)
                            break;
                    }
                    if(i < snake[j].size)
                        break;
                }
                //check wether the chosen random place is not on a wall
                if(j == 3)
                {
                    for(wall_0 = 0; wall_0 < 5; ++wall_0)
                    {
                        if((wall[0].place[wall_0]).x == x && (wall[0].place[wall_0]).y == y)
                            break;
                    }
                    for(wall_1 = 0; wall_1 < 5; ++wall_1)
                    {
                        if((wall[1].place[wall_1]).x == x && (wall[1].place[wall_1]).y == y)
                            break;
                    }
                    for(wall_2 = 0; wall_2 < 5; ++wall_2)
                    {
                        if((wall[2].place[wall_2]).x == x && (wall[2].place[wall_2]).y == y)
                            break;
                    }
                }
                //check wether the chosen random place is not on an alive frog
                if(j == 3 && wall_0 == 5 && wall_1 == 5 && wall_2 == 5)
                {
                    for(i = 0; i < 5; ++i)
                    {
                        if((frog.place[i]).x == x && (frog.place[i]).y == y && frog.alive[i] == TRUE)
                            break;
                    }
                }
                //check wether the chosen random place is not on an alive mine
                if(j == 3 && wall_0 == 5 && wall_1 == 5 && wall_2 == 5 && i == 5)
                {
                    for(k = 0; k < 5; ++k)
                    {
                        if((mine.place[k]).x == x && (mine.place[k]).y == y && mine.alive[k] == TRUE)
                            break;
                    }
                }
                //if all the above criteria met, chose the chosen random place for the frog & make it alive
                if(j == 3 && wall_0 == 5 && wall_1 == 5 && wall_2 == 5 && i == 5 && k == 5)
                {
                    (frog.place[m]).x = x;
                    (frog.place[m]).y = y;
                    frog.alive[m] = TRUE;
                }
            }
               
        }
        for(m = 0; m < 5; ++m)
        {
            //if a mine is dead, go in the while and don't come out until the mine becomes alive
            while(mine.alive[m] == FALSE)
            {
                x = rand() % 60 + 1;
                y = rand() % 20 + 1;
                //check wether the chosen random place is not on an alive snake
                for(j = 0; j < 3; ++j)
                {
                    if(snake[j].alive == FALSE)
                        continue;
                    for(i = 0; i < snake[j].size; ++i)
                    {
                        if((snake[j].object[i]).x == x && (snake[j].object[i]).y == y)
                            break;
                    }
                    if(i < snake[j].size)
                        break;
                }
                //check wether the chosen random place is not on a wall
                if(j == 3)
                {
                    for(wall_0 = 0; wall_0 < 5; ++wall_0)
                    {
                        if((wall[0].place[wall_0]).x == x && (wall[0].place[wall_0]).y == y)
                            break;
                    }
                    for(wall_1 = 0; wall_1 < 5; ++wall_1)
                    {
                        if((wall[1].place[wall_1]).x == x && (wall[1].place[wall_1]).y == y)
                            break;
                    }
                    for(wall_2 = 0; wall_2 < 5; ++wall_2)
                    {
                        if((wall[2].place[wall_2]).x == x && (wall[2].place[wall_2]).y == y)
                            break;
                    }
                }
                //check wether the chosen random place is not on an alive frog
                if(j == 3 && wall_0 == 5 && wall_1 == 5 && wall_2 == 5)
                {
                    for(i = 0; i < 5; ++i)
                    {
                        if((frog.place[i]).x == x && (frog.place[i]).y == y && frog.alive[i] == TRUE)
                            break;
                    }
                }
                //check wether the chosen random place is not on an alive mine
                if(j == 3 && wall_0 == 5 && wall_1 == 5 && wall_2 == 5 && i == 5)
                {
                    for(k = 0; k < 5; ++k)
                    {
                        if((mine.place[k]).x == x && (mine.place[k]).y == y && mine.alive[k] == TRUE)
                            break;
                    }
                }
                //if all the above criteria met, chose the chosen random place for the mine & make it alive
                if(j == 3 && wall_0 == 5 && wall_1 == 5 && wall_2 == 5 && i == 5 && k == 5)
                {
                    (mine.place[m]).x = x;
                    (mine.place[m]).y = y;
                    mine.alive[m] = TRUE;
                }
            }
            
        }
    }
}


void check_collision()
{
    int i, j, k;
    //if snake_0 hit snake_1, snake_0 get 2 points and snake_1 becomes dead
    for(i = 1; i < snake[1].size; i++)//0 hit 1
        if(((snake[0].object[0]).x == (snake[1].object[i]).x) && ((snake[0].object[0]).y == (snake[1].object[i]).y) && (snake[1].alive == TRUE)){
            snake[0].score += 2;
            snake[1].alive = FALSE;
        }
    //if snake_1 hit snake_0, snake_1 get 2 points and snake_0 becomes dead
    for(i = 1; i < snake[0].size; i++)//1 hit 0
        if(((snake[1].object[0]).x == (snake[0].object[i]).x) && ((snake[1].object[0]).y == (snake[0].object[i]).y) && (snake[0].alive == TRUE)){
            snake[1].score += 2;
            snake[0].alive = FALSE;
        }
    //if snake_0 hit snake_2, snake_0 get 2 points and snake_2 becomes dead
    for(i = 1; i < snake[2].size; i++)//0 hit 2
        if(((snake[0].object[0]).x == (snake[2].object[i]).x) && ((snake[0].object[0]).y == (snake[2].object[i]).y) && (snake[2].alive == TRUE)){
            snake[0].score += 2;
            snake[2].alive = FALSE;
        }
    //if snake_2 hit snake_0, snake_2 get 2 points and snake_0 becomes dead
    for(i = 1; i < snake[0].size; i++)//2 hit 0
        if(((snake[2].object[0]).x == (snake[0].object[i]).x) && ((snake[2].object[0]).y == (snake[0].object[i]).y) && (snake[0].alive == TRUE)){
            snake[2].score += 2;
            snake[0].alive = FALSE;
        }
    //if snake_1 hit snake_2, snake_1 get 2 points and snake_2 becomes dead
    for(i = 1; i < snake[2].size; i++)//1 hit 2
        if(((snake[1].object[0]).x == (snake[2].object[i]).x) && ((snake[1].object[0]).y == (snake[2].object[i]).y) && (snake[2].alive == TRUE)){
            snake[1].score += 2;
            snake[2].alive = FALSE;
        }
    //if snake_2 hit snake_1, snake_2 get 2 points and snake_1 becomes dead
    for(i = 1; i < snake[1].size; i++)//2 hit 1
        if(((snake[2].object[0]).x == (snake[1].object[i]).x) && ((snake[2].object[0]).y == (snake[1].object[i]).y) && (snake[1].alive == TRUE)){
            snake[2].score += 2;
            snake[1].alive = FALSE;
        }
    //if the heads of snake_0 & snake_1 hit, both of them become dead and snake_2 get 1 point
    if(((snake[0].object[0]).x == (snake[1].object[0]).x) && ((snake[0].object[0]).y == (snake[1].object[0]).y)){//0 & 1 head hit
        snake[2].score++;
        snake[0].alive = FALSE;
        snake[1].alive = FALSE;
    }
    //if the heads of snake_0 & snake_2 hit, both of them become dead and snake_1 get 1 point
    if(((snake[0].object[0]).x == (snake[2].object[0]).x) && ((snake[0].object[0]).y == (snake[2].object[0]).y)){//0 & 2 head hit
        snake[1].score++;
        snake[0].alive = FALSE;
        snake[2].alive = FALSE;
    }
    //if the heads of snake_1 & snake_2 hit, both of them become dead and snake_0 get 1 point
    if(((snake[1].object[0]).x == (snake[2].object[0]).x) && ((snake[1].object[0]).y == (snake[2].object[0]).y)){//1 & 2 head hit
        snake[0].score++;
        snake[1].alive = FALSE;
        snake[2].alive = FALSE;
    }
    for(i = 0; i < 5; ++i)
    {
        for(j = 0; j < 3; ++j)
        {
            //check if a snake has hit a frog and the snake and the frog are both alive
            if((snake[j].object[0]).x == (frog.place[i]).x && (snake[j].object[0]).y == (frog.place[i]).y && snake[j].alive == TRUE && frog.alive[i] == TRUE)
            {
                //if snake is in normal move, it goes to fast move
                if(snake[j].speed_counter == 0)
                    snake[j].speed_counter = 20;
                //if snake is in slow move, it goes to normal move
                if(snake[j].speed_counter < 0)
                    snake[j].speed_counter = 0;
                if(snake[j].speed_counter > 0)
                    snake[j].speed_counter = 20;
                //frog becomes dead
                frog.alive[i] = FALSE;
            }
        }
    }
    
    for(i = 0; i < 5; ++i)
    {
        for(j = 0; j < 3; ++j)
        {
            //check if a snake has hit a mine and the snake and the mine are both alive
            if((snake[j].object[0]).x == (mine.place[i]).x && (snake[j].object[0]).y == (mine.place[i]).y && snake[j].alive == TRUE && mine.alive[i] == TRUE)
            {
                //if snake is in normal move, it goes to slow move
                if(snake[j].speed_counter == 0)
                    snake[j].speed_counter = -20;
                //if snake is in fast move, it goes to normal move
                if(snake[j].speed_counter > 0)
                    snake[j].speed_counter = 0;
                if(snake[j].speed_counter < 0)
                    snake[j].speed_counter = -20;
                //mine becomes dead
                mine.alive[i] = FALSE;
            }
        }
    }
    
    for(i = 0; i < 3; i++)
    {
        for(k = 0; k < 5; ++k)
        {
            for(j = 0; j < 3; ++j)
            {
                //if a snake has hit a wall, it becomes dead
                if((snake[j].object[0]).x == (wall[i].place[k]).x && (snake[j].object[0]).y == (wall[i].place[k]).y && snake[j].alive == TRUE)
                    snake[j].alive = FALSE;
            }
        }
    }
    
    for(j = 0; j < 3; ++j)
    {
        for(i = 1; i < snake[j].size; i++)
        {
            //if a snake has hit itself, it becomes dead
            if((snake[j].object[0]).x == (snake[j].object[i]).x && (snake[j].object[0]).y == (snake[j].object[i]).y)
                snake[j].alive = FALSE;
        }
    }
    //if 2 snakes become dead, the game goes to next round
    if(((snake[0].alive == FALSE) && (snake[1].alive == FALSE))|((snake[0].alive == FALSE) && (snake[2].alive == FALSE))|((snake[1].alive == FALSE) && (snake[2].alive == FALSE)))
    {
        //increase round
        round_number++;
        //reset all the variables for next round
        reset_for_next_round();
    }
    //if the round number is less than 5, show the score_board
    if(round_number <= 4)
        mvwprintw(score_board, 1, 1, "|snake_0: %d| |score_1: %d| |score_2: %d| |round: %d|", snake[0].score, snake[1].score, snake[2].score, round_number);
}
//move for a snake
void move_it(int number)
{
    int i, j;
    //move the snake 1 unit
    if((indicator == 1 && snake[number].speed_counter == 0) || (indicator == 3 && snake[number].speed_counter == 0) || (indicator == 1 && snake[number].speed_counter < 0) || snake[number].speed_counter > 0)
    {
        //if snake size is less than 8, increase snake size
        if(snake[number].size < 8)
            (snake[number].size)++;
        //move the snakes part 1 unit forward
        for (i = 7; i > 0; i--)
            (snake[number].object[i]) = (snake[number].object[i-1]);
        //show all the snakes parts but head
        for (i = 1; i < snake[number].size; i++)
            mvwaddch(snakes_world, (snake[number].object[i]).y, (snake[number].object[i]).x, snake[number].face | ((number == 0) ? COLOR_PAIR(3) :
                                                                                                                   (number == 1) ? COLOR_PAIR(5) : COLOR_PAIR(6)));
        
        int x = (snake[number].object[0]).x;
        int y = (snake[number].object[0]).y;
        //make head according to the user pressed key
        switch (snake[number].dir)
        {
            case up:
                y - 1 == 0 ? y = WORLD_HEIGHT - 2 : y--;
                snake[number].head = '^';
                break;
            case down:
                y + 1 == WORLD_HEIGHT - 1 ? y = 1 : y++;
                snake[number].head = 'v';
                break;
            case right:
                x + 1 == WORLD_WIDTH - 1 ? x = 1 : x++;
                snake[number].head = '>';
                break;
            case left:
                x - 1 == 0 ? x = WORLD_WIDTH - 2 : x--;
                snake[number].head = '<';
                break;
            default:
                break;
        }
        
        (snake[number].object[0]).x = x;
        (snake[number].object[0]).y = y;
        //show the head of the snake
        mvwaddch(snakes_world, y, x, snake[number].head | ((number == 0) ? COLOR_PAIR(3) :
                                                           (number == 1) ? COLOR_PAIR(5) : COLOR_PAIR(6)));
    }
    //show the snake in its last position
    if((indicator == 2 && snake[number].speed_counter == 0) || (indicator == 4 && snake[number].speed_counter == 0) || (indicator != 1 && snake[number].speed_counter < 0))
    {
        //show the snake in its last place
        for (i = 1; i < snake[number].size; i++)
            mvwaddch(snakes_world, (snake[number].object[i]).y, (snake[number].object[i]).x, snake[number].face | ((number == 0) ? COLOR_PAIR(3) :
                                                                                                                   (number == 1) ? COLOR_PAIR(5) : COLOR_PAIR(6)));
        //show the last head of the snake
        mvwaddch(snakes_world, (snake[number].object[0]).y, (snake[number].object[0]).x, snake[number].head | ((number == 0) ? COLOR_PAIR(3) :
                                                                                                               (number == 1) ? COLOR_PAIR(5) : COLOR_PAIR(6)));
    }
    //if the snake[number].speed_counter is more than 0, decrease it.
    if(snake[number].speed_counter > 0)
        snake[number].speed_counter--;
    //if the snake[number].speed_counter is less than 0, increase it.
    if(snake[number].speed_counter < 0)
        snake[number].speed_counter++;
    
}


//thread to get character from keyboard
void* get_input (void* rank)
{
    //while the round is less than 5 and the quit & restart variables are FALSE, get input
    while (round_number <= 4 && quit_2 == FALSE && restart_2 == FALSE) {
        
        ch = getch();
        if(ch != ERR) {
            switch(ch) {
                //if user pressed 'w' and the snake_0 did not go down, it goes up
                case 'w':
                case 'W':
                    if(snake[0].dir != down)
                        snake[0].dir = up;
                    break;
                //if user pressed 's' and the snake_0 did not go up, it goes down
                case 's':
                case 'S':
                    if(snake[0].dir != up)
                        snake[0].dir = down;
                    break;
                //if user pressed 'd' and the snake_0 did not go left, it goes right
                case 'd':
                case 'D':
                    if(snake[0].dir != left)
                        snake[0].dir = right;
                    break;
                //if user pressed 'a' and the snake_0 did not go right, it goes left
                case 'a':
                case 'A':
                    if(snake[0].dir != right)
                        snake[0].dir = left;
                    break;
                //if user pressed 'i' and the snake_1 did not go down, it goes up
                case 'i':
                case 'I':
                    if(snake[1].dir != down)
                        snake[1].dir = up;
                    break;
                //if user pressed 'k' and the snake_1 did not go up, it goes down
                case 'k':
                case 'K':
                    if(snake[1].dir != up)
                        snake[1].dir = down;
                    break;
                //if user pressed 'l' and the snake_1 did not go left, it goes right
                case 'l':
                case 'L':
                    if(snake[1].dir != left)
                        snake[1].dir = right;
                    break;
                //if user pressed 'j' and the snake_1 did not go right, it goes left
                case 'j':
                case 'J':
                    if(snake[1].dir != right)
                        snake[1].dir = left;
                    break;
                //if user pressed KEY_UP and the snake_2 did not go down, it goes up
                case KEY_UP:
                    if(snake[2].dir != down)
                        snake[2].dir = up;
                    break;
                //if user pressed KEY_DOWN and the snake_2 did not go up, it goes down
                case KEY_DOWN:
                    if(snake[2].dir != up)
                        snake[2].dir = down;
                    break;
                //if user pressed KEY_RIGHT and the snake_2 did not go left, it goes right
                case KEY_RIGHT:
                    if(snake[2].dir != left)
                        snake[2].dir = right;
                    break;
                //if user pressed KEY_LEFT and the snake_2 did not go right, it goes left
                case KEY_LEFT:
                    if(snake[2].dir != right)
                        snake[2].dir = left;
                    break;
                //if the user pressed 'q', quit variable becomes TRUE
                case 'q':
                case 'Q':
                    quit = TRUE;
                    break;
                case 'r':
                case 'R':
                    //if the user press 'r' and is in the pause, pause variable becomes FALSE
                    if(pause == TRUE)
                    {
                        pause = FALSE;
                        break;
                    }
                    //else, restart variable becomes TRUE
                    else
                    {
                        restart = TRUE;
                        break;
                    }
                case 'y':
                case 'Y':
                    //if the user press 'y' and restart variable is TRUE, restart 2 variable becomes TRUE
                    if(restart == TRUE)
                    {
                        restart_2 = TRUE;
                        break;
                    }
                    //if the user press 'y' and quit variable is TRUE, quit 2 variable becomes TRUE
                    if(quit == TRUE)
                    {
                        quit_2 = TRUE;
                        break;
                    }
                    else
                        break;
                case 'n':
                case 'N':
                    //if the user press 'n' and restart variable is TRUE, restart variable becomes FALSE
                    if(restart == TRUE)
                    {
                        restart = FALSE;
                        break;
                    }
                    //if the user press 'n' and quit variable is TRUE, quit variable becomes FALSE
                    if(quit == TRUE)
                    {
                        quit = FALSE;
                        break;
                    }
                    else
                        break;
                case 'p':
                case 'P':
                    //if the user press 'p' and pause variable is TRUE, pause variable becomes FALSE
                    if(pause == FALSE)
                    {
                        pause = TRUE;
                        break;
                    }
                    else
                        break;
                default:
                    break;
            }

        }
    }
}
//reset every thing for starting the game
void reset_all()
{
    //reset every thing for the next round
    reset_for_next_round();
    //reset snakes scores
    snake[0].score = 0;
    
    snake[1].score = 0;
    
    snake[2].score = 0;
    //reset round_number
    round_number = 1;
    //reset restart variables
    restart = FALSE;
    
    restart_2 = FALSE;
}
//reset every thing for the next round
void reset_for_next_round()
{

    int i, j, k, m, x, y;
    //reset move indicator
    indicator = 1;
    
    for(j = 0; j < 3; ++j)
    {
        //reset size of snakes
        snake[j].size = 0;
        //reset speed_counter
        snake[j].speed_counter = 0;
        //make all the snakes alive
        snake[j].alive = TRUE;
        //give a random starting direction to snake
        snake[j].dir = (direction)(rand() % 4);
    }
    //give a rondom starting point to snake 0
    (snake[0].object[0]).x = rand() % 10 + 11;
    (snake[0].object[0]).y = rand() % 20 + 1;
    //give a random starting point to snake 1
    (snake[1].object[0]).x = rand() % 10 + 41;
    (snake[1].object[0]).y = rand() % 10 + 1;
    //give a random starting point to snake 2
    (snake[2].object[0]).x = rand() % 10 + 41;
    (snake[2].object[0]).y = rand() % 10 + 11;
    //give the places for all other 7 parts of the snake depend on its random starting direction
    for(j = 0; j < 3; ++j)
    {
        for (i = 1; i < SNAKEY_LENGTH; i++)
        {
            if(snake[j].dir == up)
            {
                (snake[j].object[i]).x = (snake[0].object[0]).x;
                (snake[j].object[i]).y = ((snake[0].object[0]).y + i) < 21 ? (snake[0].object[0]).y + i : (snake[0].object[0]).y + i - 20;
            }
            if(snake[j].dir == down)
            {
                (snake[j].object[i]).x = (snake[0].object[0]).x;
                (snake[j].object[i]).y = ((snake[0].object[0]).y - i) > 0 ? (snake[0].object[0]).y - i : (snake[0].object[0]).y - i + 20;
            }
            if(snake[j].dir == right)
            {
                (snake[j].object[i]).x = ((snake[0].object[0]).x - i) > 0 ? (snake[0].object[0]).x - i : (snake[0].object[0]).x - i + 60;
                (snake[j].object[i]).y = (snake[0].object[0]).y;
            }
            if(snake[j].dir == left)
            {
                (snake[j].object[i]).x = ((snake[0].object[0]).x + i) < 61 ? (snake[0].object[0]).x + i : (snake[0].object[0]).x + i - 60;
                (snake[j].object[i]).y = (snake[0].object[0]).y;
            }
            
        }
    }
    
    bool again = TRUE;
    //find a random place for the first vertical wall with x between 1 & 10
    while(again)
    {
        x = rand() % 10 + 1;
        y = rand() % 15 + 1;
        //if the x or y is equal to a x or y of the head of a snake, repeat
        if((snake[0].object[0]).x == x || (snake[0].object[0]).y == y)
            continue;
        if((snake[1].object[0]).x == x || (snake[1].object[0]).y == y)
            continue;
        if((snake[2].object[0]).x == x || (snake[2].object[0]).y == y)
            continue;
        //else, the x & y are good and fill the wall.place array with appropriate amount
        for(i = 0; i < 5; ++i)
        {
            (wall[0].place[i]).x = x;
            (wall[0].place[i]).y = y + i;
            //make again FALSE so it comes out of the while loop
            again = FALSE;
        }
        
    }
    
    again = TRUE;
    //find a random place for the second vertical wall with x between 41 & 50
    while(again)
    {
        x = rand() % 10 + 41;
        y = rand() % 15 + 1;
        //if the x or y is equal to a x or y of the head of a snake, repeat
        if((snake[0].object[0]).x == x || (snake[0].object[0]).y == y)
            continue;
        if((snake[1].object[0]).x == x || (snake[1].object[0]).y == y)
            continue;
        if((snake[2].object[0]).x == x || (snake[2].object[0]).y == y)
            continue;
        //else, the x & y are good and fill the wall.place array with appropriate amount
        for(i = 0; i < 5; ++i)
        {
            (wall[1].place[i]).x = x;
            (wall[1].place[i]).y = y + i;
            //make again FALSE so it comes out of the while loop
            again = FALSE;
        }
        
    }
    
    again = TRUE;
    //find a random place for the horizantal wall with starting x between 21 & 30 and y between 1 & 20
    while(again)
    {
        x = rand() % 10 + 21;
        y = rand() % 20 + 1;
        //if the x or y is equal to a x or y of the head of a snake, repeat
        if((snake[0].object[0]).x == x || (snake[0].object[0]).y == y)
            continue;
        if((snake[1].object[0]).x == x || (snake[1].object[0]).y == y)
            continue;
        if((snake[2].object[0]).x == x || (snake[2].object[0]).y == y)
            continue;
        //else, the x & y are good and fill the wall.place array with appropriate amount
        for(i = 0; i < 5; ++i)
        {
            (wall[2].place[i]).x = x + i;
            (wall[2].place[i]).y = y;
            //make again FALSE so it comes out of the while loop
            again = FALSE;
        }
        
    }
    
    
    m = 0;
    
    int wall_0, wall_1, wall_2;
    //find random place for frogs
    while(m < 5)
    {
        x = rand() % 60 + 1;
        y = rand() % 20 + 1;
        //if x & y is equal to x & y of a snakes head, repeat
        for(j = 0; j < 3; ++j)
        {
            if((snake[j].object[0]).x == x && (snake[j].object[0]).y == y)
                break;
        }
        //if x & y is equal to x & y of a walls place, repeat
        if(j == 3)
        {
            for(wall_0 = 0; wall_0 < 5; ++wall_0)
            {
                if((wall[0].place[wall_0]).x == x && (wall[0].place[wall_0]).y == y)
                    break;
            }
            for(wall_1 = 0; wall_1 < 5; ++wall_1)
            {
                if((wall[1].place[wall_1]).x == x && (wall[1].place[wall_1]).y == y)
                    break;
            }
            for(wall_2 = 0; wall_2 < 5; ++wall_2)
            {
                if((wall[2].place[wall_2]).x == x && (wall[2].place[wall_2]).y == y)
                    break;
            }
        }
        //if x & y is equal to x & y of a previous frog, repeat
        if(j == 3 && wall_0 == 5 && wall_1 == 5 && wall_2 == 5)
        {
            for(i = 0; i < m; ++i)
            {
                if((frog.place[i]).x == x && (frog.place[i]).y == y)
                    break;
            }
        }
        //if all the above criteria met, fill the frog.place with above x & y
        if(j == 3 && wall_0 == 5 && wall_1 == 5 && wall_2 == 5 && i == m)
        {
            (frog.place[m]).x = x;
            (frog.place[m]).y = y;
            //increase m
            m++;
        }
    }
    
    m = 0;
    //find random place for mines
    while(m < 5)
    {
        x = rand() % 60 + 1;
        y = rand() % 20 + 1;
        //if x & y is equal to x & y of a snakes head, repeat
        for(j = 0; j < 3; ++j)
        {
            if((snake[j].object[0]).x == x && (snake[j].object[0]).y == y)
                break;
        }
        //if x & y is equal to x & y of a walls place, repeat
        if(j == 3)
        {
            for(wall_0 = 0; wall_0 < 5; ++wall_0)
            {
                if((wall[0].place[wall_0]).x == x && (wall[0].place[wall_0]).y == y)
                    break;
            }
            for(wall_1 = 0; wall_1 < 5; ++wall_1)
            {
                if((wall[1].place[wall_1]).x == x && (wall[1].place[wall_1]).y == y)
                    break;
            }
            for(wall_2 = 0; wall_2 < 5; ++wall_2)
            {
                if((wall[2].place[wall_2]).x == x && (wall[2].place[wall_2]).y == y)
                    break;
            }
        }
        //if x & y is equal to x & y of a frog, repeat
        if(j == 3 && wall_0 == 5 && wall_1 == 5 && wall_2 == 5)
        {
            for(i = 0; i < 5; ++i)
            {
                if((frog.place[i]).x == x && (frog.place[i]).y == y)
                    break;
            }
        }
        //if x & y is equal to x & y of a previous mine, repeat
        if(j == 3 && wall_0 == 5 && wall_1 == 5 && wall_2 == 5 && i == 5)
        {
            for(k = 0; k < m; ++k)
            {
                if((mine.place[k]).x == x && (mine.place[k]).y == y)
                    break;
            }
        }
        //if all the above criteria met, fill the mine.place with above x & y
        if(j == 3 && wall_0 == 5 && wall_1 == 5 && wall_2 == 5 && i == 5 && k == m)
        {
            (mine.place[m]).x = x;
            (mine.place[m]).y = y;
            //increase m
            m++;
        }
    }
    //make all the frogs and mines alive
    for(i = 0; i < 5; ++i)
    {
        frog.alive[i] = TRUE;
        mine.alive[i] = TRUE;
    }
    
}
//initialize screen
void initialaize()
{
    initscr();
    //no echo for input
    noecho();
    cbreak();
    //no delay for input
    nodelay(stdscr, TRUE);
    //use this to use keyboard arrow keys
    keypad(stdscr, TRUE);
    //make curser disapper
    curs_set(FALSE);
    start_color();
    //color pairs for making colorful objects
    init_pair(1, COLOR_RED, COLOR_BLACK);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);
    init_pair(3, COLOR_YELLOW, COLOR_BLACK);
    init_pair(4, COLOR_BLUE, COLOR_BLACK);
    init_pair(5, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(6, COLOR_CYAN, COLOR_BLACK);
    init_pair(7, COLOR_WHITE, COLOR_BLACK);
    //for making random number
    srand(time(NULL));
}