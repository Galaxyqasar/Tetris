#define SDL_MAIN_HANDLED

//#define DEBUG
#define CHEATS
#define INPUT1

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include "inputSystem.h"

int speeds[30] = {48,43,38,33,28,23,18,13,8,6,5,5,5,4,4,4,3,3,3,2,2,2,2,2,2,2,2,2,2,1};
int levels[30] = {0,10,20,30,40,50,60,70,80,90,100,100,100,100,100,100,100,110,120,130,140,150,160,170,180,190,200,200,200,200};
char chars[7] = {'I','O','J','L','S','T','Z'};
int mult[4] = {40,100,300,1200};
int array[7] = {0,0,0,0,0,0,0};
int stats[7] = {0,0,0,0,0,0,0};

GLuint fs, vs, prog, uScale, uLevel;
GLuint texture1;

int showAnimation_s;

clock_t start;

int state = 0;

enum Blocks{
    T,L,S,O,Z,J,I
};

enum States{
    s_running = 0,
    s_paused,
    s_scoreMenu,
    s_mainMenu,
    s_levelMenu,
    s_startScreen,
    s_creditMenu,
    s_animation
};

enum Animations{
    a_GameOver = 0,
    a_Finish
};

typedef struct Tile{
    int x, y, c;
} Tile;

#define newTile(x,y,c) (Tile){x,y,c}

typedef struct StatEntry{
    int score, lines, level;
    unsigned int seed;
    unsigned int stats[7];
    char name[10];
} StatEntry;

typedef struct GlobalStat{
    StatEntry *stats;
    int size;
} GlobalStat;

typedef struct Block{
    Tile tiles[4];
    int type;
    int x, y;       // offset of block
} Block;

typedef struct Scene{
    int width, height;
    int score;
    int top;
    int lines;
    int level;
    int type;
    int music;
    int seed;
    int startTileCount;
    Block current, next;
    Tile **tiles;
} Scene;
Scene scene;

typedef struct Image{
    unsigned char *data;
    int width, height;
} Image;

#define newImage(d,w,h) (Image){d,w,h}

Image frame, background, assets;
Image backgrounds[6];
GlobalStat statistics;
Block allBlocks[7];

int r;
int random(){
    r = ((((r >> 9) & 1) ^ ((r >> 1) & 1)) << 15) | (r >> 1);
    return r;
}

void numberToArray(int number) {
    for(int i = 0; i < 7; i++)
        array[i] = 0;
    int i = 0;
    do {
        array[6 - i] = number % 10;
        number /= 10;
        i++;
    } while (number != 0);
}

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

    allBlocks[J].tiles[0] = newTile(0,1,'L');
    allBlocks[J].tiles[1] = newTile(1,1,'L');
    allBlocks[J].tiles[2] = newTile(2,1,'L');
    allBlocks[J].tiles[3] = newTile(2,2,'L');
    allBlocks[J].type = L;

    allBlocks[L].tiles[0] = newTile(0,1,'J');
    allBlocks[L].tiles[1] = newTile(1,1,'J');
    allBlocks[L].tiles[2] = newTile(2,1,'J');
    allBlocks[L].tiles[3] = newTile(0,2,'J');
    allBlocks[L].type = J;

    allBlocks[S].tiles[0] = newTile(0,2,'Z');
    allBlocks[S].tiles[1] = newTile(1,2,'Z');
    allBlocks[S].tiles[2] = newTile(1,1,'Z');
    allBlocks[S].tiles[3] = newTile(2,1,'Z');
    allBlocks[S].type = Z;

    allBlocks[T].tiles[0] = newTile(0,1,'T');
    allBlocks[T].tiles[1] = newTile(1,1,'T');
    allBlocks[T].tiles[2] = newTile(2,1,'T');
    allBlocks[T].tiles[3] = newTile(1,2,'T');
    allBlocks[T].type = T;

    allBlocks[Z].tiles[0] = newTile(0,1,'S');
    allBlocks[Z].tiles[1] = newTile(1,1,'S');
    allBlocks[Z].tiles[2] = newTile(1,2,'S');
    allBlocks[Z].tiles[3] = newTile(2,2,'S');
    allBlocks[Z].type = S;
}

void saveStats(char str[10]){
    if(scene.lines == 0)
        return;
    FILE *f = fopen("./score.txt", "a");
    if(!f)
        f = fopen("./score.txt", "w");
    fprintf(f, "score: %i, lines: %i, level: %i, "
               "stats:{T:%i,J:%i,Z:%i,O:%i,S:%i,L:%i,I:%i}, seed: %u, \"%c%c%c%c%c%c%c%c%c\"\n",
            scene.score, scene.lines, scene.level,
            stats[T], stats[J], stats[Z], stats[O], stats[S], stats[L], stats[I], scene.seed,
                &str[0], &str[1], &str[2], &str[3], &str[4], &str[5], &str[6], &str[7], &str[8]);
    fclose(f);
}

int cmpStatEntry(const void *a, const void *b){
    StatEntry *s1 = a, *s2 = b;
    if(s1->score < s2->score)
        return 1;
    else if (s1->score == s2->score)
        return 0;
    return -1;
}

GlobalStat loadGlobalStats(char *filename){
    StatEntry *statistics = malloc(0);
    FILE *f = fopen(filename, "r");
    int size = 0;
    if(!f)
        f = fopen(filename, "w+");
    if(f){
        while(!feof(f)){
            int score, lines, level, seed;
            int stats[7];
            char *str[10];
            if(fscanf(f, "score: %i, lines: %i, level: %i, "
               "stats:{T:%i,J:%i,Z:%i,O:%i,S:%i,L:%i,I:%i}, seed: %u, \"%c%c%c%c%c%c%c%c%c\"\n",
                &score, &lines, &level,
                &stats[T], &stats[J], &stats[Z], &stats[O], &stats[S], &stats[L], &stats[I], &seed,
                &str[0], &str[1], &str[2], &str[3], &str[4], &str[5], &str[6], &str[7], &str[8]))
            {
                str[9] = 0;
                StatEntry current = {score, lines, level, seed, stats, str};
                statistics = realloc(statistics, sizeof(StatEntry)*(size+1));
                statistics[size++] = current;
            }
            else
                break;
        }
    }
    qsort(statistics, size, sizeof(StatEntry), cmpStatEntry);
    GlobalStat stat = {statistics, size};
    return stat;
}

void initScene(unsigned int seed){
    scene.seed = seed;
    r = scene.seed;
    scene.current = allBlocks[random()%7];
    stats[scene.current.type]++;
    scene.current.x = 3;
    scene.next = allBlocks[random()%7];
    scene.lines = 0;
    scene.score = 0;
    scene.width = 10;
    scene.height = 20;
    scene.level = 0;
	scene.type = 0;
	scene.music = 0;
	scene.startTileCount = 20;
    statistics = loadGlobalStats("score.txt");
    printf("%i scores loaded, best: %i\n", statistics.size, statistics.stats[0].score);
    scene.top = statistics.stats[0].score;
    scene.tiles = malloc(sizeof(Tile*) * scene.height);
    for(int y = 0; y < scene.height; y++){
        scene.tiles[y] = malloc(sizeof(Tile) * scene.width);
        for(int x = 0; x < scene.width; x++){
            scene.tiles[y][x] = newTile(x,y,' ');
        }
    }
}

void initField(){
    if(scene.type%2){
        for(int i = 0; i < scene.startTileCount; i++){
            int x = random()%scene.width;
            int y = random()%(scene.height-10)+10;
            while(scene.tiles[y][x].c != ' '){
                x = random()%scene.width;
                y = random()%(scene.height-10)+10;
            }
            scene.tiles[y][x] = newTile(x,y,chars[random()%7]);
        }
    }
}

void renderPause(){
    int fWidth = frame.width, fHeight = frame.height;
    int aWidth = assets.width, aHeight = assets.height;
    int size = frame.width*frame.height*3;
    unsigned char *dFrame = frame.data;
    unsigned char *dAssets = assets.data;
    for(int i = 0; i < size; i++){
        dFrame[i] = 0;
    }
    char *str = "PAUSE";
    for(int i = 0; i < strlen(str); i++){
        for(int y = 0; y < 8; y++){
            int c = str[i]-65+10;
            memcpy(&dFrame[((fHeight-(y+107))*fWidth*3-3*y)-102+24*i], &dAssets[(aHeight-y-1-8*(c/16))*aWidth*3+24*(c%16)], 8*3);
        }
    }
}

void renderMain(){
    int fWidth = frame.width, fHeight = frame.height;
    int aWidth = assets.width, aHeight = assets.height;
    int size = frame.width*frame.height*3;
    unsigned char *dFrame = frame.data;
    unsigned char *dAssets = assets.data;

    memcpy(dFrame, background.data, size);

    for(int y = 0; y < scene.height; y++){
        for(int x = 0; x < scene.width; x++){
            int type = scene.tiles[y][x].c;
            if(type == ' ')
                continue;
            if(type == '-'){
                int tx = x+1;
                int ty = y+1;
                for(int y = 0; y < 8; y++){
                    memcpy(&dFrame[((fHeight-(y+32+8*ty))*fWidth*3-3*y)-102+24*tx+88*3-(32*3+24*ty)], &dAssets[(aHeight-y-1-4*8)*aWidth*3+24*15], 8*3);
                }
                continue;
            }
            int tx = x+1;
            int ty = y+1;
            int t = (type == 'T' || type == 'O' || type == 'I') ? 0 : ((type == 'Z' || type == 'L') ? 2 : 1);
            for(int y = 0; y < 8; y++){
                memcpy(&dFrame[((fHeight-(y+32+8*ty))*fWidth*3-3*y)-102+24*tx+88*3-(32*3+24*ty)], &dAssets[(aHeight-y-1-9*8)*aWidth*3+24*t+24*8], 8*3);
            }
        }
    }
    if(state == s_running){
        for(int i = 0; i < 4; i++){
            int type = scene.current.type;
            int tx = scene.current.tiles[i].x + scene.current.x+1;
            int ty = scene.current.tiles[i].y + scene.current.y+1;
            int t = (type == T || type == O || type == I) ? 0 : ((type == Z || type == L) ? 2 : 1);
            for(int y = 0; y < 8; y++){
                memcpy(&dFrame[((fHeight-(y+32+8*ty))*fWidth*3-3*y)-102+24*tx+88*3-(32*3+24*ty)], &dAssets[(aHeight-y-1-9*8)*aWidth*3+24*t+24*8], 8*3);
            }
        }
    }
    for(int i = 0; i < 4; i++){
        int type = scene.next.type;
        int tx = scene.next.tiles[i].x;
        int ty = scene.next.tiles[i].y+1;
        int t = (type == T || type == O || type == I) ? 0 : ((type == Z || type == L) ? 2 : 1);
        for(int y = 0; y < 8; y++){
            memcpy(&dFrame[((fHeight-(y+96+8*ty))*fWidth*3-3*y)-102+24*tx+192*3-(96*3+24*ty)], &dAssets[(aHeight-y-1-9*8)*aWidth*3+24*t+24*8], 8*3);
        }
    }
    numberToArray(scene.score);
    for(int i = 0; i < 6; i++){
        for(int y = 0; y < 8; y++){
            memcpy(&dFrame[((fHeight-(y+56))*fWidth*3-3*y)-102+24*i+192*3-56*3], &dAssets[(aHeight-y-1)*aWidth*3+24*array[i+1]], 8*3);
        }
    }
    numberToArray(scene.top);
    for(int i = 0; i < 6; i++){
        for(int y = 0; y < 8; y++){
            memcpy(&dFrame[((fHeight-(y+32))*fWidth*3-3*y)-102+24*i+192*3-32*3], &dAssets[(aHeight-y-1)*aWidth*3+24*array[i+1]], 8*3);
        }
    }
    numberToArray(scene.level);
    for(int i = 0; i < 2; i++){
        for(int y = 0; y < 8; y++){
            memcpy(&dFrame[((fHeight-(y+160))*fWidth*3-3*y)-102+24*i+208*3-160*3], &dAssets[(aHeight-y-1)*aWidth*3+24*array[i+5]], 8*3);
        }
    }
    numberToArray(scene.lines);
    for(int i = 0; i < 3; i++){
        for(int y = 0; y < 8; y++){
            memcpy(&dFrame[((fHeight-(y+16))*fWidth*3-3*y)-102+24*i+152*3-16*3], &dAssets[(aHeight-y-1)*aWidth*3+24*array[i+4]], 8*3);
        }
    }
    for(int s = 0; s < 7; s++){
        numberToArray(stats[s]);
        for(int i = 0; i < 3; i++){
            for(int y = 0; y < 8; y++){
                memcpy(&dFrame[((fHeight-(y+85+16*s))*fWidth*3-3*y)-102+24*i+50*3-(85+16*s)*3], &dAssets[(aHeight-y-1)*aWidth*3+24*array[i+4]], 8*3);
            }
        }
    }
    for(int y = 0; y < 8; y++){
        memcpy(&dFrame[((fHeight-(y+24))*fWidth*3-3*y)-102+24*3-24*3], &dAssets[(aHeight-y-1)*aWidth*3+24*scene.type+24*10], 8*3);
    }
}


void renderCreditMenu(){
    memcpy(frame.data, backgrounds[1].data, frame.width*frame.height*3);
}

void renderStartScreen(){
    memcpy(frame.data, backgrounds[2].data, frame.width*frame.height*3);
}

void renderMainMenu(){
    memcpy(frame.data, backgrounds[3].data, frame.width*frame.height*3);

    int fWidth = frame.width, fHeight = frame.height;
    int aWidth = assets.width, aHeight = assets.height;
    int size = frame.width*frame.height*3;
    unsigned char *dFrame = frame.data;
    unsigned char *dAssets = assets.data;

    char c = 39;
    int xoff = -33, yoff = 160;
    if(scene.type%2){
        xoff = +63;
    }
    for(int y = 0; y < 8; y++){
        memcpy(&dFrame[(fWidth*((yoff+y))+xoff)*3+y*3], &dAssets[(aHeight-y-1-8*(c/16))*aWidth*3+24*(c%16)], 8*3);
    }
    int m = scene.music%4;
    switch(m >= 0 ? m : 4 - abs(m)){
        case 0:{
            yoff = 80;
            xoff = -73;
        }break;
        case 1:{
            yoff = 64;
            xoff = -89;
        }break;
        case 2:{
            yoff = 48;
            xoff = -105;
        }break;
        case 3:{
            yoff = 32;
            xoff = -105;
        }break;
    }
    for(int y = 0; y < 8; y++){
        memcpy(&dFrame[(fWidth*((yoff+y))+xoff)*3+y*3], &dAssets[(aHeight-y-1-8*(c/16))*aWidth*3+24*(c%16)], 8*3);
    }
}

void renderLevelMenu(){
    int level = scene.level;
    memcpy(frame.data, backgrounds[4].data, frame.width*frame.height*3);

    char c = scene.type%2 +10;
    int xoff = 53, yoff = 206;
    for(int y = 0; y < 8; y++){
        memcpy(&frame.data[(frame.width*((yoff-y))+xoff)*3-y*3], &assets.data[(assets.height-y-1-8*(c/16))*assets.width*3+24*(c%16)], 8*3);
    }

    static int blink = 0;
    blink++;
    if(blink%5){
        unsigned char pixel[3];
        int xoff = 0, yoff = 0;
        if(scene.level < 5)
            yoff = 132;
        else
            yoff = 115;
        xoff = -204 + (scene.level%5)*16;
        for(int y = 0; y < 16; y++){
            for(int x = 0; x < 16; x++){
                int xpos = x + xoff;
                int ypos = y + yoff;
                memcpy(pixel, &frame.data[(ypos*frame.width+xpos+ypos)*3], 3);
                if(pixel[0] == 0 && pixel[1] == 0 && pixel[2] == 0){
                    pixel[0] = 252;
                    pixel[1] = 200;
                    pixel[2] = 0;
                }
                memcpy(&frame.data[(ypos*frame.width+xpos+ypos)*3], pixel, 3);
            }
        }
    }
}

void renderScore(){
    memcpy(frame.data, backgrounds[5].data, frame.width*frame.height*3);
}

void render(SlotArgs *args){
    static int start = 0;

    float ft = glutGet(GLUT_ELAPSED_TIME)-start;
    char str[64];
    sprintf(str, "Tetris: %2.2f ms, %2.1 fps", ft, CLOCKS_PER_SEC/ft);
    glutSetWindowTitle(str);

    start = glutGet(GLUT_ELAPSED_TIME);
    switch(state){
        case s_running:
        case s_animation:{
            renderMain();
        } break;
        case s_paused:{
            renderPause();
        } break;
        case s_scoreMenu:{
            renderScore();
        } break;
        case s_mainMenu:{
            renderMainMenu();
        } break;
        case s_levelMenu:{
            renderLevelMenu();
        } break;
        case s_creditMenu:{
            renderCreditMenu();
        } break;
        case s_startScreen:{
            renderStartScreen();
        } break;
    }
}

void showAnimation(SlotArgs *args){
    state = s_animation;
    if(!compareStr(args->format, "%i%i")){
#ifdef DEBUG
        printf("format different");
#endif
        return;
    }
#ifdef DEBUG
    printf("game Over\n");
#endif
    int *data = args->data;
    if(!data)
        return;
    int animation = data[0];
    int process = data[1]+1;
    cleanSlotArgs(args);
    for(int y = 0; y < process/5; y++){
        for(int x = 0; x < 10; x++)
            scene.tiles[y][x].c = '-';
    }
#ifdef DEBUG
    printf("animation: %i, %i\n", animation, process);
#endif
    if(process < 100)
        emit(showAnimation_s, createArgs(createBasicString("%i%i"), animation,process));
    else{
        state = s_scoreMenu;
        initScene(time(NULL));
    }
}

void refresh(SlotArgs *args){
    glBindTexture(GL_TEXTURE_2D, texture1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, frame.width, frame.height-1, 0, GL_RGB, GL_UNSIGNED_BYTE, frame.data);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture1);

    glClear(GL_COLOR_BUFFER_BIT);

    glUniform1f(uScale, scale);
    glUniform1i(uLevel, scene.level);

	glLoadIdentity();

	glBegin(GL_QUADS);
		glVertex3f(-1, 1, 0);
		glVertex3f(1, 1, 0);
		glVertex3f(1, -1, 0);
		glVertex3f(-1, -1, 0);
	glEnd();
    glutSwapBuffers();
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

void gameOver(){
    saveStats("NickName ");
    for(int i = 0; i < 7; i++)
        stats[i] = 0;
    for(int i = 0; i < 4; i++){
        int x = scene.current.tiles[i].x + scene.current.x;
        int y = scene.current.tiles[i].y + scene.current.y;
        char c = scene.current.tiles[i].c;
        scene.tiles[y][x].c = c;
    }
    //initScene();
    emit(showAnimation_s, createArgs(createBasicString("%i%i"), 0,0));
}

unsigned diff = 0;

void drop(SlotArgs *args){
    if(state != s_running){
        diff = clock()-start;
        return;
    }
    if(diff != 0){
        start += diff;
        diff = 0;
    }
    if(clock() >= start + speeds[scene.level > 30 ? 30 : scene.level]*16 || args == 1){
        Block tmp = scene.current;
        tmp.y += 1;
        if(checkCollision(tmp)){
            scene.current = tmp;
            if(args == 1)
                scene.score += 1;
        }
        else{
            for(int i = 0; i < 4; i++){
                int x = scene.current.tiles[i].x + scene.current.x;
                int y = scene.current.tiles[i].y + scene.current.y;
                char c = scene.current.tiles[i].c;
                scene.tiles[y][x].c = c;
            }
            scene.current = scene.next;
            scene.current.x = 3;
            scene.current.y = 0;
            stats[scene.current.type]++;
            if(!checkCollision(scene.current))
                gameOver();
            scene.next = allBlocks[random()%7];
            int rot = random()%4;
            for(int i = 0; i < rot; i++)
                scene.next = rotate(scene.next);
            int lines = 0;
            for(int y = 1; y < scene.height; y++){
                int tiles = 0;
                for(int x = 0; x < scene.width; x++){
                    if(scene.tiles[y][x].c == ' ')
                        break;
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
            for(int i = scene.level; i < 30; i++){
                if(scene.lines >= levels[i])
                    scene.level = i;
            }
        }
        start = clock();
    }
}

void moveVertical(int dir){
    Block tmp = scene.current;
    tmp.x += dir;
    if(checkCollision(tmp)){
        scene.current = tmp;
    }
}

void keyPressed(SlotArgs *args){
    int dscore = 0;
    int key = InputSystem.currentKey;
    if(state == s_startScreen && key != -1){
        if(key == ' ')
            state = s_mainMenu;
        else
            printf("hi1\n");
    }
    else if(state == s_mainMenu){
        if(key != -1){
            if(key == 13)
                state = s_levelMenu;
            else{
                printf("hi2, %i\n", key);
            }
        }
        else{
            if(keys[2])
                scene.music++, keys[2] = 0;
            if(keys[0])
                scene.music--, keys[0] = 0;
            if(keys[3])
                scene.type++, keys[3] = 0;
            if(keys[1])
                scene.type--, keys[1] = 0;
        }
    }
    else if(state == s_levelMenu){
        if(key != -1){
            if(key == 13){
                state = s_running;
                initField();
            }
            else if(key == 27)
                state = s_mainMenu;
            else{
                printf("hi2, %i\n", key);
            }
        }
        else{
            if(keys[2] && scene.level < 5)
                scene.level += 5, keys[2] = 0;
            if(keys[0] && scene.level > 4)
                scene.level -=5, keys[0] = 0;
            if(keys[3] && scene.level < 9)
                scene.level++, keys[3] = 0;
            if(keys[1] && scene.level > 0)
                scene.level--, keys[1] = 0;
        }
    }
    else if(state == s_scoreMenu && key != -1){
        if(key == 13)
            state = s_levelMenu;
        else
            printf("hi4, %i\n", key);
    }
    else if(state == s_running){
#ifdef INPUT1
        int m = scene.level < 9 ? 7 : 4;
        if(keys[0]){
            if(w == 0)
                scene.current = rotate(scene.current);
            w++;
            if(w > m+2)
                w = 0;
        }
        if(keys[1]){
            if(a == 0)
                moveVertical(-1);
            a++;
            if(a > m)
                a = 0;
        }
        if(keys[2]){
            if(s == 0)
                drop(1);
            s++;
            if(s > m)
                s = 0;
        }
        if(keys[3]){
            if(d == 0)
                moveVertical(1);
            d++;
            if(d > m)
                d = 0;
        }
#else
        if(keys[0]){
            scene.current = rotate(scene.current);
            keys[0] = 0;
        }
        if(keys[1]){
            moveVertical(-1);
            keys[1] = 0;
        }
        if(keys[2]){
            drop(1);
            keys[2] = 0;
        }
        if(keys[3]){
            moveVertical(1);
            keys[3] = 0;
        }
#endif
        switch(key){
            case 'q' : {
                App.quit();
            } break;
#ifdef CHEATS
            case '1':{
                scene.level++;
            } break;
            case '2':{
                scene.lines++;
            } break;
            case '3':{
                scene.score++;
            }break;
            case '4':{
#else
            case 'N':
            case 'n':{
#endif
                noobMode = !noobMode;
            }break;
        }
    }
    if(key == 'p' || key == 'P'){
        if(state == s_paused)
            state = s_running;
        else if(state == s_running)
            state = s_paused;
    }
    InputSystem.currentKey = -1;
}

void windowEvent(SlotArgs *args){
    glutMainLoopEvent();
}

void init(){
}

void cleanUp(){
    saveStats("NickName ");
}

char *loadFile(char *filename){
    char *data = malloc(1);
    data[0] = 0;
    FILE *file = fopen(filename, "r");
    if(!file){
#ifdef DEBUG
        printf("error file \"%s\" doesn't exist", filename);
#endif
        return data;
    }
    int len = 0;
    while(!feof(file)){
        char c = fgetc(file);
        data = realloc(data, sizeof(char)*(len + 2));
        len++;
        data[len-1] = c;
        data[len] = 0;
    }
    data[len-1] = 0;
    fclose(file);
    return data;
}

int checkShaderError(GLuint shader){
	GLint success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if(!success){
        GLint logSize = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logSize);
        char *str = malloc(logSize);
        glGetShaderInfoLog(shader, logSize, &logSize, str);
        printf("errors: \n\%s\n\n", str);
        return -1;
    }
    else{
        printf("no errors in shader\n");
        return 0;
    }
}

void set_shader()
{
    char *f, *v;

    v = loadFile("./assets/shaders/vshader.glsl");
    f = loadFile("./assets/shaders/fshader.glsl");
#ifdef DEBUG
    printf("Vertex shader : \n%s\nEnd vertex shader\nFragment shader:\n%s\nEnd fragment shader\n%i %i\n", v, f, strlen(v), strlen(f));
#endif
	vs = glCreateShader(GL_VERTEX_SHADER);
	fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fs, 1, &f, 0);
	glShaderSource(vs, 1, &v, 0);
    free(v);
    free(f);
	glCompileShader(vs);
	glCompileShader(fs);

	checkShaderError(fs);

	prog = glCreateProgram();
	glAttachShader(prog, fs);
	glAttachShader(prog, vs);

	glLinkProgram(prog);
	glUseProgram(prog);
	uScale = glGetUniformLocation(prog, "s");
	uLevel = glGetUniformLocation(prog, "level");

    glGenTextures(1, &texture1);
    glBindTexture(GL_TEXTURE_2D, texture1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, background.width, background.height-1, 0, GL_RGB, GL_UNSIGNED_BYTE, background.data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glGenerateMipmap(GL_TEXTURE_2D);
    glUniform1i(glGetUniformLocation(prog, "frame"), 0);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture1);
}

Image loadTexture(char *filename){
    FILE *f = fopen(filename, "rb");
    if(!f){
        printf("Image \"%s\" doesn't exist\n", filename);
        return newImage(NULL,0,0);
    }
    char bmpfileheader[14];
    char bmpinfoheader[40];
    fread(bmpfileheader, 1, 14, f);
    fread(bmpinfoheader, 1, 40, f);
    int w, h;
    memcpy(&w, &bmpinfoheader[4], sizeof(int));
    memcpy(&h, &bmpinfoheader[8], sizeof(int));
    int x = 2;
    int n = w * h;
    if(n < 1000)
        x = 0;
    unsigned char *texture = malloc(3*(n+1));
    if(texture != NULL){
        for(int i = 0; i < n; i++){
            unsigned char color[3];
            fread(color,1,3,f);
            for(int k = 0; k < 3; k++)
                memcpy(&texture[i*3 + k], &color[abs(x-k)], 1);
        }
    }
    fclose(f);
    return newImage(texture,w,h);
}

void resize(int w, int h) {
	int width = 255*scale, height = 222*scale;
    glutReshapeWindow( width, height);
}

void show(){
    refresh(NULL);
}

int main(int argc, char **argv)
{
    backgrounds[0] = loadTexture("./assets/images/background0.bmp");
    backgrounds[1] = loadTexture("./assets/images/background1.bmp");
    backgrounds[2] = loadTexture("./assets/images/background2.bmp");
    backgrounds[3] = loadTexture("./assets/images/background3.bmp");
    backgrounds[4] = loadTexture("./assets/images/background4.bmp");
    backgrounds[5] = loadTexture("./assets/images/background5.bmp");

    background = loadTexture("./assets/images/background0.bmp");
    assets = loadTexture("./assets/images/assets.bmp");
    frame.width = background.width;
    frame.height = background.height;
    frame.data = malloc(3*frame.width*frame.height);
#ifdef DEBUG
    printf("%i, %i\n", background.width, background.height);
#endif
    init();
    App.init(cleanUp);
    InputSystem.init(keyPressed);

#ifdef DEBUG
    printf("renderSlot: %i\n", addConnection(render, emptyArgs, true));
    printf("refreshSlot: %i\n", addConnection(refresh, emptyArgs, true));
    printf("dropSlot: %i\n", addConnection(drop, emptyArgs, true));
    printf("window events lot: %i\n", addConnection(windowEvent, emptyArgs, true));
    printf("kePressedSlot: %i\n", addConnection(keyPressed, emptyArgs, true));
    printf("animationSlot: %i\n", showAnimation_s = addConnection(showAnimation, NULL, false));
#else
    addConnection(render, emptyArgs, true);
    addConnection(refresh, emptyArgs, true);
    addConnection(drop, emptyArgs, true);
    addConnection(windowEvent, emptyArgs, true);
    addConnection(keyPressed, emptyArgs, true);
    showAnimation_s = addConnection(showAnimation, NULL, false);
#endif
    start = clock();
    initBlocks();
    initScene(time(NULL));
    state = s_startScreen;
	int width = 255*scale, height = 222*scale;

    glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowSize(width, height);
	glutCreateWindow("Tetris");
	glutKeyboardFunc(pressKey);
	glutKeyboardUpFunc(releaseKey);
	glutSpecialFunc(pressSpecial);
	glutSpecialUpFunc(releaseSpecial);
	//glutMouseFunc(mouseDown);
    glutReshapeFunc(resize);
    //glutFullScreen();
	glewInit();
	//glutIdleFunc(show);
	glutDisplayFunc(show);
#ifdef DEBUG
    printf("Opengl version : %s\n", glGetString(GL_VERSION));
#endif
	set_shader();
	glClearColor(0.0,0.5,0.5,1.0);
    return App.exec();
}   ///1347 lines
