#ifndef HEADER_H
#define HEADER_H

#include <string.h>

typedef SlotArgs;
typedef struct Vector2{
    int x, y;
} Vector2;

char* createString(char *str){
    char *string = malloc(sizeof(char)*strlen(str));
    memcpy(string, str, sizeof(char)*strlen(str));
    return string;
}

void cleanSlotArgs(SlotArgs *args){
    free(args->data);
    free(args->format);
    free(args);
}

#endif
