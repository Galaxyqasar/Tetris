#ifndef EVENT_SYSTEM_H
#define EVENT_SYSTEM_H

#include <stdbool.h>
#include <stdarg.h>
#include <stdio.h>

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


char* createBasicString(char *str){
    char *string = malloc(sizeof(char)*strlen(str));
    strcpy(string, str);
    return string;
}

int charCount(char *str, char c){
    int count = 0;
    for(int i = 0; str[i] != 0; i++)
        if(str[i] == c)
            count++;
    return count;
}

int compareStr(char *str1, char *str2){
    for(int i = 0; str1[i] != 0 && str2[i] != 0; i++){
        if(str1[i] != str2[i])
            return 0;
    }
    return 1;
}

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
                SlotArgs *args = current->args;
                if(!current->repeat || !App.repeat)
                    current->args = NULL;
#ifdef DEBUG
                printf("calling slot %i\n", index-1);
#endif
                current->slot(args);
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
    int len = 0;
    for(;App.connections[len] != NULL; len++);  len++;
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

SlotArgs *createArgs(char *format, ...){
    int argc = charCount(format, '%');
    unsigned char *data = malloc(0);
    int index = 0;
    va_list ptr;
    va_start(ptr, format);
    for(int i = 0; i < argc; i++){
        switch(format[2*i+1]){
            case 'c':{
                int8_t x = va_arg(ptr, int8_t);
                data = realloc(data, index+1);
                memcpy(&data[index], &x, 1);
                index += 1;
            }break;
            case 'hd':{
                int16_t x = va_arg(ptr, int16_t);
                data = realloc(data, index+2);
                memcpy(&data[index], &x, 2);
                index += 2;
            }break;
            case 'd':
            case 'ld':
            case 'i':{
                int32_t x = va_arg(ptr, int32_t);
#ifdef DEBUG
                printf("%i;", x);
#endif
                data = realloc(data, index+4);
                memcpy(&data[index], &x, 4);
                index += 4;
            }break;
            case 'f':{
                float x = va_arg(ptr, float);
                data = realloc(data, index+8);
                memcpy(&data[index], &x, 8);
                index += 8;
            }break;
            case 'lf':{
                double x = va_arg(ptr, double);
                data = realloc(data, index+10);
                memcpy(&data[index], &x, 10);
                index += 10;
            }break;
            case 's':{
                int x = va_arg(ptr, void*);
                data = realloc(data, index+4);
                memcpy(&data[index], &x, 4);
                index += 4;
            }break;
        }
    }
    va_end(ptr);
    int *x = &data[0];
    int *y = &data[4];
#ifdef DEBUG
    printf("%i,%i\n", *x, *y);
#endif

    SlotArgs *args = malloc(sizeof(SlotArgs));
    args->format = format;
    args->data = data;
    return args;
}

void cleanSlotArgs(SlotArgs *args){
    free(args->data);
    free(args->format);
    free(args);
}

SlotArgs *emptyArgs = &(SlotArgs){"", NULL};

#endif
