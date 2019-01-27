#include <stdio.h>
#include <stdlib.h>
#include <curses.h>
#include <time.h>
#include "inputSystem.h"

int speeds[30] = {48,43,38,33,28,23,18,13,8,6,5,5,5,4,4,4,3,3,3,2,2,2,2,2,2,2,2,2,2,1};
int levels[30] = {0,10,20,30,40,50,60,70,80,90,100,100,100,100,100,100,100,110,120,130,140,150,160,170,180,190,200,200,200,200};
int stats[7] = {0,0,0,0,0,0,0};
char chars[7] = {'I','O','J','L','S','T','Z'};
int mult[4] = {40,100,300,1200};

clock_t start;

enum blocks{
    T,L,Z,O,S,J,I
};

typedef struct Tile{
    int x, y;
    char c;
} Tile;

#define newTile(x,y,c) (Tile){x,y,c}

typedef struct Block{
    Tile tiles[4];
    int type;
    int x, y;       // offset of block
} Block;

typedef struct Scene{
    Tile **tiles;
    int width, height;
    int level;
    int lines, score, top;
    Block current, next;
} Scene;

Block allBlocks[7];

Scene scene;

void initBlocks(){
    allBlocks[I].tiles[0] = newTile(0,2,'I');
    allBlocks[I].tiles[1] = newTile(1,2,'I');
    allBlocks[I].tiles[2] = newTile(2,2,'I');
    allBlocks[I].tiles[3] = newTile(3,2,'I');
    allBlocks[I].type = I;

    allBlocks[O].tiles[0] = newTile(1,1,'O');
    allBlocks[O].tiles[1] = newTile(1,2,'O');
    allBlocks[O].tiles[2] = newTile(2,1,'O');
    allBlocks[O].tiles[3] = newTile(2,2,'O');
    allBlocks[O].type = O;

    allBlocks[J].tiles[0] = newTile(0,1,'J');
    allBlocks[J].tiles[1] = newTile(1,1,'J');
    allBlocks[J].tiles[2] = newTile(2,1,'J');
    allBlocks[J].tiles[3] = newTile(2,2,'J');
    allBlocks[J].type = J;

    allBlocks[L].tiles[0] = newTile(0,1,'L');
    allBlocks[L].tiles[1] = newTile(1,1,'L');
    allBlocks[L].tiles[2] = newTile(2,1,'L');
    allBlocks[L].tiles[3] = newTile(0,2,'L');
    allBlocks[L].type = L;

    allBlocks[S].tiles[0] = newTile(0,2,'S');
    allBlocks[S].tiles[1] = newTile(1,2,'S');
    allBlocks[S].tiles[2] = newTile(1,1,'S');
    allBlocks[S].tiles[3] = newTile(2,1,'S');
    allBlocks[S].type = S;

    allBlocks[T].tiles[0] = newTile(0,1,'T');
    allBlocks[T].tiles[1] = newTile(1,1,'T');
    allBlocks[T].tiles[2] = newTile(2,1,'T');
    allBlocks[T].tiles[3] = newTile(1,2,'T');
    allBlocks[T].type = T;

    allBlocks[Z].tiles[0] = newTile(0,1,'Z');
    allBlocks[Z].tiles[1] = newTile(1,1,'Z');
    allBlocks[Z].tiles[2] = newTile(1,2,'Z');
    allBlocks[Z].tiles[3] = newTile(2,2,'Z');
    allBlocks[Z].type = Z;
}

void initScene(){
    scene.current = allBlocks[I];
    stats[scene.current.type]++;
    scene.current.x = 2;
    scene.current.y = 0;
    scene.next = allBlocks[rand()%7];
    scene.lines = 0;
    scene.score = 0;
    scene.width = 10;
    scene.height = 20;
    scene.level = 0;
    scene.top = 10000;
    scene.tiles = malloc(sizeof(Tile*) * scene.height);
    for(int y = 0; y < scene.height; y++){
        scene.tiles[y] = malloc(sizeof(Tile) * scene.width);
        for(int x = 0; x < scene.width; x++){
            scene.tiles[y][x] = newTile(x,y,' ');
        }
    }
}

void drawFrame(int x, int y, int w, int h){
    mvprintw(y,x,"+");
    for(int i = 1; i <= w; i++)
        mvprintw(y,x+i,"-");
    mvprintw(y,x+w+1,"+");
    for(int i = 1; i <= h; i++){
        mvprintw(y+i,x,"|");
        mvprintw(y+i,x+w+1,"|");
    }
    mvprintw(y+h+1,x,"+");
    for(int i = 1; i <= w; i++)
        mvprintw(y+h+1,x+i,"-");
    mvprintw(y+h+1,x+w+1,"+");
}

void render(SlotArgs *args){
    erase();
    drawFrame(0,0,scene.width + 22, scene.height + 5);
    drawFrame(2,2,6,1);
    drawFrame(13,1,scene.width,1);
    drawFrame(13,4,scene.width,scene.height);
    drawFrame(1,6,10,15);
    drawFrame(scene.width+12+3,1,6,7);
    drawFrame(scene.width+12+3,11,4,5);
    drawFrame(scene.width+12+3,18,5,2);

    mvprintw(3,3,"A-TYPE");
    mvprintw(7,2,"STATISTICS");
    mvprintw(2,15,"LINES-%03d", scene.lines);
    mvprintw(19,scene.width+12+4,"LEVEL");
    mvprintw(20,scene.width+12+6,"%02d", scene.level);
    mvprintw(12,scene.width+12+4,"NEXT");
    mvprintw(3,scene.width+12+4,"TOP");
    mvprintw(4,scene.width+12+4,"%06d", scene.top);
    mvprintw(6,scene.width+12+4,"SCORE");
    mvprintw(7,scene.width+12+4,"%06d", scene.score);

    for(int y = 0; y < scene.height; y++){
        for(int x = 0; x < scene.width; x++)
            mvprintw(y+5, x+14, "%c", scene.tiles[y][x].c);
    }
    for(int i = 0; i < 4; i++){
        Tile current = scene.current.tiles[i];
        int x = scene.current.x + current.x;
        int y = scene.current.y + current.y;
        mvprintw(y+5,x+14,"%c", current.c);
    }
    for(int i = 0; i < 4; i++){
        Tile current = scene.next.tiles[i];
        int x = scene.next.x + current.x;
        int y = scene.next.y + current.y;
        mvprintw(y+13,x+26,"%c", current.c);
    }
    for(int i = 0; i < 7; i++){
        for(int k = 0; k < 4; k++){
            Tile current = allBlocks[i].tiles[k];
            int x = current.x + 3;
            int y = current.y + i*2 + 8;
            if(i == I)
                y--;
            if(i == O)
                x--;
            mvprintw(y,x,"%c", current.c);
        }
        mvprintw(9+i*2,8,"%03d", stats[i]);
    }
}

bool checkCollision(Block block){
    for(int i = 0; i < 4; i++){
        int x = block.tiles[i].x + block.x;
        int y = block.tiles[i].y + block.y;
        if(x >= scene.width || x < 0)
            return false;
        if(y >= scene.height || y < 0)
            return false;
        if(scene.tiles[y][x].c != ' ')
            return false;
    }
    return true;
}

Block rotate(Block block){
    Block backup = block;
    switch(block.type){
        case O:
            break;
        case I:{
            for(int i = 0; i < 4; i++){
                int x = block.tiles[i].y;
                int y = block.tiles[i].x;
                block.tiles[i].x = x;
                block.tiles[i].y = y;
            }
        } break;
        case S:
        case Z:{
            for(int i = 0; i < 4; i++){
                int x = 2-block.tiles[i].y;
                int y = block.tiles[i].x;
                block.tiles[i].x = x;
                block.tiles[i].y = y;
            }
            for(int i = 0; i < 4; i++)
                if(block.tiles[i].x == 0 && block.tiles[i].y == (block.type == S ? 0 : 2))
                    for(int k = 0; k < 4; k++)
                        block.tiles[k].x += 1;
        } break;
        case T:
        case L:
        case J:{
            for(int i = 0; i < 4; i++){
                int x = 2-block.tiles[i].y;
                int y = block.tiles[i].x;
                block.tiles[i].x = x;
                block.tiles[i].y = y;
            }
        } break;
    }
    if(checkCollision(block))
        return block;
    return backup;
}

void drop(SlotArgs *args){
    if(clock() >= start + speeds[scene.level]*16){
        Block tmp = scene.current;
        tmp.y += 1;
        if(checkCollision(tmp)){
            scene.current = tmp;
        }
        else{
            for(int i = 0; i < 4; i++){
                int x = scene.current.tiles[i].x + scene.current.x;
                int y = scene.current.tiles[i].y + scene.current.y;
                char c = scene.current.tiles[i].c;
                scene.tiles[y][x].c = c;
            }
            scene.current = scene.next;
            scene.current.x = 2;
            stats[scene.current.type]++;
            if(!checkCollision(scene.current))
                App.quit();
            scene.next = allBlocks[rand()%7];
            int rot = rand()%4;
            for(int i = 0; i < rot; i++)
                scene.next = rotate(scene.next);
            int lines = 0;
            for(int y = 1; y < scene.height; y++){
                int tiles = 0;
                for(int x = 0; x < scene.width; x++){
                    if(scene.tiles[y][x].c == ' ')
                        continue;
                    tiles++;
                }
                if(tiles == scene.width){
                    scene.tiles[0] = scene.tiles[y];
                    for(int x = 0; x < scene.width; x++)
                        scene.tiles[0][x].c = ' ';
                    for(int y2 = y; y2 > 0; y2--)
                        scene.tiles[y2] = scene.tiles[y2-1];
                    lines++;
                }
            }
            int dscore = 0;
            if(lines > 0 && lines < 5)
                dscore = mult[lines-1] * (scene.level + 1);
            scene.score += dscore;
            scene.lines += lines;
            for(int i = 0; i < 30; i++){
                if(scene.lines >= levels[i])
                    scene.level = i;
            }
        }
        start = clock();
    }
}

void keyPressed(SlotArgs *args){
    int *p = args->data;
    int c = *p;
    Block tmp = scene.current;
    int dscore = 0;
    switch(c){
        case 'q' : {
            App.quit();
        } break;
        case 'w' : {
            tmp = rotate(tmp);
        } break;
        case 'a' : {
            tmp.x -= 1;
        } break;
        case 's' : {
            tmp.y += 1;
            dscore = 1;
        } break;
        case 'd' : {
            tmp.x += 1;
        } break;
        default : {
            printw("%c", c);
        }
    }
    if(checkCollision(tmp)){
        scene.current = tmp;
        scene.score += dscore;
    }
}

void init(){
    initscr();
    timeout(1);
    noecho();
    keypad(stdscr, true);
    cbreak();
    curs_set(0);
    srand((unsigned)time(NULL));
}

void cleanUp(){
    endwin();
}

int main()
{
    init();
    App.init(cleanUp);
    InputSystem.init(keyPressed);
    addConnection(render, emptyArgs, true);
    addConnection(drop, emptyArgs, true);
    start = clock();
    initBlocks();
    initScene();
    return App.exec();
}
