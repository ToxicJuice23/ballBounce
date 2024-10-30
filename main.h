#ifndef MAIN_H
#define MAIN_H
#include <errno.h>
#include <unistd.h>
#include <math.h>
#include <stdarg.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_thread.h>
#include <signal.h>
#include <time.h>
#include <limits.h>
#define PI 3.141592653589
#define MAX_VELOCITY 2

u_short window_size_x = 1;
u_short window_size_y = 1;
float G = 200.f * -9.8f; // 200 px/m * -9.8 m/s^2
float CLOCK_DIVISOR = 1.f / CLOCKS_PER_SEC;

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
            dty, // Δt since last collision with floor or ceiling
            v0_x, // initial speed on x at instant 0
            v0_y; // initial speed on y at instant 0
    int n_seg;
    SDL_FPoint* base_circle;
    pos_t p; // current position
} ball_t;

typedef struct {
    char* name;
    void* var_ptr;
} pair_t;

typedef struct {ball_t* b; uint8_t done;} data_t;

float calculate_velocity_elastic_collision( float v);
void set_base_circle( ball_t* ball);
void drawCircle( SDL_Renderer* renderer, ball_t* ball,  float x,  float y,  int n);
float get_time( float start);
void set_pos( ball_t* ball);
int handle_collision( ball_t* ball,  u_short size_x,  u_short size_y, u_short* counter);
void setup_ball( ball_t* ball, float v0x, float v0y);
int computing_thread( void* d);
int key_buf_to_str(const SDL_Keycode* key_buf, char* key_str);
void strip_str( char** str,  int n);
int sep_str(const char* res, char* var_name, char* val,  u_int n);
void fatalf(const char* fmt, ...);
SDL_Window* setup_window_sdl( void );

#endif // BALL_H
