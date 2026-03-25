#ifndef INPUT_H
#define INPUT_H

/*
    Z   Y   X   W   V   U   T   S   R   Q   P   O   N   M   L   K   J   I   H   G   F   E   D   C   B   A
                ESC F12 F11 F10 F9  F8  F7  F6  F5  F4  F3  F2  F1  9   8   7   6   5   4   3   2   1   0
*/

#define KEYBOARD

// Note: Careful for name collisions.
namespace InputM {
    enum LETTERS {
        A = 1 << 0,
        B = 1 << 1,
        C = 1 << 2,
        D = 1 << 3,
        E = 1 << 4,
        F = 1 << 5,
        G = 1 << 6,
        H = 1 << 7,
        I = 1 << 8,
        J = 1 << 9,
        K = 1 << 10,
        L = 1 << 11,
        M = 1 << 12,
        N = 1 << 13,
        O = 1 << 14,
        P = 1 << 15,
        Q = 1 << 16,
        R = 1 << 17,
        S = 1 << 18,
        T = 1 << 19,
        U = 1 << 20,
        V = 1 << 21,
        W = 1 << 22,
        X = 1 << 23,
        Y = 1 << 24,
        Z = 1 << 25,
    };
    
    enum SPECIAL {
        N0 = 1 << 0,
        N1 = 1 << 1,
        N2 = 1 << 2,
        N3 = 1 << 3,
        N4 = 1 << 4,
        N5 = 1 << 5,
        N6 = 1 << 6,
        N7 = 1 << 7,
        N8 = 1 << 8,
        N9 = 1 << 9,
        F1 = 1 << 10,
        F2 = 1 << 11,
        F3 = 1 << 12,
        F4 = 1 << 13,
        F5 = 1 << 14,
        F6 = 1 << 15,
        F7 = 1 << 16,
        F8 = 1 << 17,
        F9 = 1 << 18,
        F10 = 1 << 19,
        F11 = 1 << 20,
        F12 = 1 << 21,
        ESC = 1 << 22,
    };
    
    enum MOUSE {
        LEFT_CLICK = 1 << 0,
        RIGHT_CLICK = 1 << 1,
    };
    
    enum TOGGLES {
        Toggle_F1   = 1 << 3,  // Keyboard
        Toggle_H    = 1 << 5,  // Keyboard
        Toggle_N    = 1 << 6,  // Keyboard
        Toggle_P    = 1 << 7,  // Keyboard
        Toggle_M    = 1 << 8,  // Keyboard
        Toggle_B    = 1 << 10, // Keyboard
        Toggle_LC   = 1 << 11, // Mouse
    };
}

struct Input {
    unsigned int  letters; // xxxxxx26
    unsigned int  special; // 23 esc, f12-f1, 9-0
    unsigned int  toggles; // whatever
    unsigned char mouse;   // xxx5
    
    double x_current, y_current;
    double x_previous, y_previous;
    int wheel;
    
    double ndc_x;
    double ndc_y;
};

#endif