#ifndef INPUTSYSTEM_H
#define INPUTSYSTEM_H

#include "eventSystem.h"
#include "header.h"
#include <string.h>

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
}

void getKeyInput(SlotArgs *args){
    InputSystem.currentKey = getch();
    if(InputSystem.currentKey != -1){
        SlotArgs *args = malloc(sizeof(SlotArgs));
        int *data = malloc(sizeof(int));
        memcpy(data, &InputSystem.currentKey, sizeof(int));
        memcpy(args, &(SlotArgs){createString("%i"), (void*)data}, sizeof(SlotArgs));
        emit(InputSystem.s_keyPressed, args);
    }
}

#endif
