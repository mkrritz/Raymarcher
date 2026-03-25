
#include "pch.h"

#include "mkr/Assert.h"
#include "mkr/Input.h"
#include "mkr/Memory.h"
#include "mkr/Window.h"

//#define DEBUG

static int normalize(int k) {
    if (k >= GLFW_KEY_0 && k <= GLFW_KEY_9) {
        k = 1 << (k - GLFW_KEY_0 + 0);
    }else if (k >= GLFW_KEY_F1 && k <= GLFW_KEY_F12) {
        k = 1 << (k - GLFW_KEY_F1 + 10);
    }else if (k == GLFW_KEY_ESCAPE) {
        k = 1 << (k - GLFW_KEY_ESCAPE + 22);
    } else {
        k = 0;
    }
    return k;
}

static Input _input;

Input* NextRecordedInputState() {
    return &_input;
}

void UpdateInputStateKeyboard(int key, int scancode, int action, int mods) {
    if (key >= GLFW_KEY_A && key <= GLFW_KEY_Z) {
        key = 1 << (key - GLFW_KEY_A); // Letters
        _input.letters = (action == GLFW_PRESS) ? (_input.letters | key) : (_input.letters & ~key);
    } else {
        key = normalize(key); // Numbers F Esc
        _input.special = (action == GLFW_PRESS) ? (_input.special | key) : (_input.special & ~key);
    }
#ifdef DEBUG
    std::bitset<8 * sizeof(_input.letters)> lett(_input.letters);
    std::bitset<8 * sizeof(_input.special)> spec(_input.special);
    std::cout <<"\tLetter state: " << lett << " (" << _input.letters << ")\n";
    std::cout <<"\tSpecial state: " << spec << " (" << _input.special << ")\n";
#endif
}

void UpdateInputStateMouse(int key, int action, int mods) {
    key = 1 << key; // key is 0, 1, ..
    _input.mouse = (action == GLFW_PRESS) ? (_input.mouse | key) : (_input.mouse & ~key);
#ifdef DEBUG
    std::bitset<8 * sizeof(_input.mouse)> mouse(_input.mouse);
    std::cout <<"\tMouse state: " << mouse << " (" << _input.mouse << ")\n";
#endif
}

void UpdateInputMousePosition(double x, double y) {
    _input.x_previous = _input.x_current;
    _input.y_previous = _input.y_current;
    _input.x_current  = x;
    _input.y_current  = y;
    
    _input.ndc_x = -1.0f + 2.0f * x / WINDOW_WIDTH;
    _input.ndc_y =  1.0f - 2.0f * y / WINDOW_HEIGHT;
}

void UpdateInputMouseWheel(double xoffset, double yoffset) {
    _input.wheel = yoffset;
}

