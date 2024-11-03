//
// Created by jujur on 30/10/24.
//

#include "main.h"

void fatalf(const char* fmt, ...) {
    // printf and exit with error
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    exit(1);
}

int key_buf_to_str(const SDL_Keycode* key_buf, char* key_str) {
    // transform array of keys into string
    if (key_buf == NULL || key_str == NULL) {
        return -1;
    }
    for (int i=0; i < 100; i++) {
        key_str[i] = key_buf[i] > UCHAR_MAX ? ' ' : (char)key_buf[i];
        if (key_str[i] == 0) {
            break;
        }
    }
    return 0;
}

int is_allowed(const char c) {
    // check if char is allowed in variable strings
    const char allowed_extra[] = {'=', '+', '-'}; const int len = 3;
    for (int i = 0; i < len; i++) {
        if (c == allowed_extra[i] || isalpha(c) || isdigit(c)) return 1;
    }
    return 0;
}

int strip_str(char** str, const int n) {
    // remove unwanted characters
    if (str == NULL || *str == NULL) {
        return -1;
    }
    int counter = 0;
    for (int i = 0; i < n; i++) {
        if ((*str)[i] == '\0') {
            return -1;
        }
        if (is_allowed((*str)[i])) {
            (*str)[counter] = (char)tolower((*str)[i]);
            counter++;
        }
    }
    (*str)[counter] = 0;
    return 0;
}

int sep_str(const char* res, char* var_name, char* val, const u_int n) {
    if (res == NULL || var_name == NULL || val == NULL || n == 0) {
        return -1;
    }

    memset(var_name, 0, n);
    memset(val, 0, n);

    int space_index = -1;
    for (u_int i = 0; i < n; i++) {
        if (res[i] == '=') {
            space_index = (int)i;
        } else if (space_index == -1) {
            var_name[i] = res[i];
        } else {
            val[i-space_index-1] = res[i];
        }
    }
    var_name[space_index] = 0;

    if (space_index == -1) {
        return space_index;
    }

    // slice strs
    return 0;
}