#ifndef EVENT_SYSTEM_H
#define EVENT_SYSTEM_H

#include <stdbool.h>

typedef unsigned char byte;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef unsigned long uint64;
typedef unsigned char uchar;

typedef struct Arguments{
    char *format;
    void* data;
} SlotArgs;

typedef struct Connection{
    uint32 index;
    bool repeat;
    void (*slot)(SlotArgs *args);
    SlotArgs *args;
} Connection;


int addConnection(void (*slot)(SlotArgs *args), SlotArgs *args, bool repeat);
int app_mainLoop();
void emit(uint32 index, SlotArgs *args);
void finished(uint32 index);
int app_initApp();
void app_quitApp();
void app_setRepeat(bool repeat);

struct App{
    bool b_quit, repeat;
    Connection **connections;
    const int (*exec)();
    const int (*init)();
    const void (*quit)();
    const void (*setRepeat)();
    int (*cleanUpFn)();
} App = (struct App){false, true, NULL, app_mainLoop, app_initApp, app_quitApp, app_setRepeat, NULL};

int app_initApp(int (*cleanUpFn)()){
    App.connections = malloc(sizeof(Connection*));
    App.connections[0] = NULL;
    App.cleanUpFn = cleanUpFn;
    return 0;
}

int app_mainLoop(){
    printf("Start main loop\n");
    while(!App.b_quit){
        uint32 index = 0;
        Connection *current;
        while((current = App.connections[index++]) != NULL){
            if(current->args != NULL){
                current->slot(current->args);
                if(!current->repeat || !App.repeat)
                    finished(index-1);
            }
        }
    }
    printf("End main loop\n");
    return App.cleanUpFn();
}

void app_quitApp(){
    App.b_quit = true;
}

void app_setRepeat(bool repeat){
    App.repeat = repeat;
}

int addConnection(void (*slot)(SlotArgs *args), SlotArgs *args, bool repeat){
    int len = 0;    while(App.connections[len] != NULL) len++;  len++;
    App.connections = realloc(App.connections, (len+1)*sizeof(Connection*));
    Connection *newConnection = malloc(sizeof(Connection));
    App.connections[len-1] = newConnection;
    App.connections[len] = NULL;
    newConnection->slot = slot;
    newConnection->index = len-1;
    newConnection->args = args;
    newConnection->repeat = repeat;
    return len-1;
}

void emit(uint32 index, SlotArgs *args){
    if(index == -1)
        return;
    if(App.connections[index] != NULL){
        App.connections[index]->args = args;
    }
}

void finished(uint32 index){
    App.connections[index]->args = NULL;
}

SlotArgs *emptyArgs = &(SlotArgs){"", NULL};

#endif
