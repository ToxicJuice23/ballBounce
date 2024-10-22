#ifndef FAST_BALL_H
#define FAST_BALL_H
#include <errno.h>
#include <unistd.h>
#include <math.h>
#include <stdarg.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_thread.h>
#include <signal.h>
#include <time.h>
#define PI 3.141592653589
#define MAX_VELOCITY 2

u_short window_size_x = 1;
u_short window_size_y = 1;
const float G = 200 * -9.8; // 200 px/m * -9.8 m/s^2
static const float clock_divisor = 1.0 / CLOCKS_PER_SEC;

typedef struct {
    float x, y;
} pos_t;

typedef struct {
    float   x, // exact starting x position of timeframe (Δx when Δtx = 0)
            y, // exact starting y position of timeframe (Δy when Δty = 0)
            mass, // in kg (used for friction and radius of ball)
            v_x, // in pixels/s
            v_y, // in pixels/s
            t0x, // time in seconds since thread birth at start or at last x collision
            t0y, // time in seconds since thread birth at start or at last x collision
            dtx, // Δt since last collision on x-axis or since start
            dty; // Δt since last collision with floor or ceiling
    int n_seg;
    SDL_FPoint* base_circle;
    pos_t p; // current position
} ball_t;

typedef struct {ball_t* b; uint8_t done;} data_t;

#endif // BALL_H
