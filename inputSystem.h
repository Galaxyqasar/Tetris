#ifndef INPUTSYSTEM_H
#define INPUTSYSTEM_H

#include "eventSystem.h"
#include <string.h>

#define KEY_RIGHT 79
#define KEY_LEFT 80
#define KEY_DOWN 81
#define KEY_UP 82

bool noobMode = false;
float scale = 3.0f;
void initInput();
void getKeyInput(SlotArgs *args);

struct InputSystem{
    int s_keyPressed;
    int s_keyboardInput;

    int currentKey;
    const void (*init)(void (*x)());
} InputSystem = (struct InputSystem){-1,-1,-1,initInput};

void initInput(void (*keyPressSlot)(SlotArgs *args)){

    InputSystem.s_keyboardInput = addConnection(getKeyInput, emptyArgs, true);  //internal slots
    InputSystem.s_keyPressed = addConnection(keyPressSlot, NULL, false);
#ifdef DEBUG
    printf("keyboardInputSlot: %i\n", InputSystem.s_keyboardInput);
    printf("keyPressedSlot: %i\n", InputSystem.s_keyPressed);
#endif
}

void getKeyInput(SlotArgs *args){
    if(InputSystem.currentKey != -1){
        SlotArgs *args = malloc(sizeof(SlotArgs));
        int *data = malloc(sizeof(int));
        memcpy(data, &InputSystem.currentKey, sizeof(int));
        memcpy(args, &(SlotArgs){createBasicString("%i"), (void*)data}, sizeof(SlotArgs));
        emit(InputSystem.s_keyPressed, args);
    }
}

bool keys[4] = {0,0,0,0};
int w = 0, a = 0, s = 0, d = 0;

void pressKey(int c, int k){
    if(c == 'w' || c == 'W' || k == KEY_UP)
        keys[0] = 1;
    else if(c == 'a' || c == 'A' || k == KEY_LEFT)
        keys[1] = 1;
    else if(c == 's' || c == 'S' || k == KEY_DOWN)
        keys[2] = 1;
    else if(c == 'd' || c == 'D' || k == KEY_RIGHT)
        keys[3] = 1;
    else
        InputSystem.currentKey = c;
}

void releaseKey(int c, int k){
    if(c == 'w' || c == 'W'){
        keys[0] = 0;
        w = 0;
    }
    else if(c == 'a' || c == 'A'){
        keys[1] = 0;
        a = 0;
    }
    else if(c == 's' || c == 'S'){
        keys[2] = 0;
        s = 0;
    }
    else if(c == 'd' || c == 'D'){
        keys[3] = 0;
        d = 0;
    }
    else
        InputSystem.currentKey = -1;
}

void pressSpecial(int c, int k){
    if(c == GLUT_KEY_UP)
        keys[0] = 1;
    else if(c == GLUT_KEY_LEFT)
        keys[1] = 1;
    else if(c == GLUT_KEY_DOWN)
        keys[2] = 1;
    else if(c == GLUT_KEY_RIGHT)
        keys[3] = 1;
    else
        InputSystem.currentKey = c;
}

void releaseSpecial(int c, int k){
    if(c == GLUT_KEY_UP){
        keys[0] = 0;
        w = 0;
    }
    else if(c == GLUT_KEY_LEFT){
        keys[1] = 0;
        a = 0;
    }
    else if(c == GLUT_KEY_DOWN){
        keys[2] = 0;
        s = 0;
    }
    else if(c == GLUT_KEY_RIGHT){
        keys[3] = 0;
        d = 0;
    }
    else
        InputSystem.currentKey = -1;
}
#endif
