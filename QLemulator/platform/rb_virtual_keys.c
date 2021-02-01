//
//  rb_virtual_keys.c
//
//  Created by Roger Boesch on Jan. 10, 2021.
//  "You can do whatever you like with it"
//

#include <string.h>
#include "rb_virtual_keys.h"

int character_to_vk(char ch) {
    const char* position_ptr = strchr(KeyCharacterSet, ch);
    long position = (position_ptr == NULL ? RBVK_Unknown : position_ptr - KeyCharacterSet);
    return (int)position;
}

int ql_shift_key_fixes(char ch, int keyCode) {
    switch (ch) {
        case '!':
            return RBVK_Num1;
        case '@':
            return RBVK_Num2;
        case '#':
            return RBVK_Num3;
        case '$':
            return RBVK_Num4;
        case '%':
            return RBVK_Num5;
        case '^':
            return RBVK_Num6;
        case '&':
            return RBVK_Num7;
        case '*':
            return RBVK_Num8;
        case '(':
            return RBVK_Num9;
        case ')':
            return RBVK_Num0;
        case '_':
            return RBVK_Dash;
        case '+':
            return RBVK_Equal;
        case '|':
           return RBVK_BackSlash;
        case '~':
           return RBVK_Grave;
        case '{':
            return RBVK_LBracket;
        case '}':
            return RBVK_RBracket;
        case '[':
            return RBVK_LBracket;
        case ']':
            return RBVK_RBracket;
        case ':':
            return RBVK_SemiColon;
        case '"':
            return RBVK_Quote;
        case '<':
            return RBVK_Comma;
        case '>':
            return RBVK_Period;
        case '?':
            return RBVK_Slash;
        default:
            return keyCode;
    }
}
