#include "ball_sdl.h"

#include <limits.h>
/*
ATTENTION POSSIBLE CONTRIBUTERS
-------------------------------
All of the functions should be using radians for consistency
-------------------------------
todo
    - add option to insert more balls and make them collide.
*/

void fatalf(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    exit(1);
}

float calculate_velocity_elastic_collision(const float v) {
    // for simplicity, simply multiply by 0.8 and inverse direction
    return -0.9f*v;
}

void set_base_circle(ball_t* ball) {
    // compute circle now and simply add x and y values to the points...
    const int n_seg = ball->n_seg;
    ball->base_circle = malloc(sizeof(SDL_FPoint) * (n_seg + 2));
    if (ball->base_circle == NULL) {
        fatalf("No memory to allocate...X(\n");
    }

    const float coeff = 2.0f * (float)PI / (float)n_seg;

    for (int i = 0; i < n_seg; i++) {
        const float theta = coeff * (float)i;
        const float px = cosf(theta) * ball->mass;
        const float py = sinf(theta) * ball->mass;
        SDL_FPoint cp; cp.x = (float)px; cp.y = (float)(py);
        ball->base_circle[i] = cp;
    }
    ball->base_circle[n_seg] = ball->base_circle[0];
}

void drawCircle(SDL_Renderer* renderer, ball_t* ball, const float x, const float y, const int n) {
    // drawCircle(SDL_Renderer*, float, float, int)
    // drawCircle takes a renderer, coordinates, the radius and the smoothness of the circle (number of segments)
    SDL_FPoint* points = malloc(sizeof(SDL_FPoint)*(n+2));
    if (ball->base_circle == NULL) {set_base_circle(ball);};
    memcpy(points, ball->base_circle, sizeof(SDL_FPoint) * (n+1));
    if (points == NULL) {
        fatalf("Memory is null.\n");
        return;
    }

    for (int i = 0; i <= n; i++) {
        points[i].x += x;
        points[i].y += y;
    }

    if (SDL_RenderDrawLinesF(renderer, points, n+1) && errno != EAGAIN) {
        fatalf("Failed to draw circle.\n%s\n", SDL_GetError());
    }
    free(points);
}

float get_time(const float start) {
    return (float)clock() * clock_divisor - start;
}

void set_pos(ball_t* ball) {
    const float dx = ball->v_x * (ball->dtx);
    const float dy = ball->v_y * ball->dty + 0.5f * G * ball->dty * ball->dty; // removed pow(dty, 2); for speed
    ball->p.x = dx; ball->p.y = dy;
}

int handle_collision(ball_t* ball, const u_short size_x, const u_short size_y, u_short* counter) {
    // ball, window size as args
    // returns the angle of collision (in radians)
    set_pos(ball);
    const float x = ball->p.x + ball->x, y = ball->p.y + ball->y;
    u_int8_t _case = 0;
    float sizes[4] = {x-ball->mass, x+ball->mass, y-ball->mass, y+ball->mass};
    if ((sizes[1] > (float)size_x && (_case = 1)) || (sizes[0] < 0.f && (_case = 2)) || (sizes[3] > (float)size_y && (_case = 3)) || (sizes[2] < 0.f && (_case = 4))) {
        switch (_case) {
            case 3:
                case 4:
                ball->v_y = calculate_velocity_elastic_collision(ball->v_y + G*ball->dty);
                ball->y += ball->p.y;
                ball->dty = 0;
                ball->t0y = get_time(0);
                break;
            case 1:
                case 2:
                ball->x += ball->p.x;
                ball->v_x = calculate_velocity_elastic_collision(ball->v_x);
                ball->dtx = 0;
                ball->t0x = get_time(0);
                break;
            default:
                fatalf("weird bug\n");
        }
        // make sure we dont wallclip
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wunused-value"
        _case == 1 && (ball->x = (float)size_x - ball->mass);
        _case == 2 && (ball->x = ball->mass);
        _case == 3 && (ball->y = (float)size_y - ball->mass);
        _case == 4 && (ball->y = ball->mass);
        *counter += 1;
        #pragma GCC diagnostic pop
        return 1;
    }
    return 0;
}

void setup_ball(ball_t* ball, const float v0x, const float v0y) {
    ball->n_seg = 100;
    ball->x = (float)window_size_x / 2.f;
    ball->y = (float)window_size_y / 2.f;
    ball->v_y = v0y; // pixels/s
    ball->v_x = v0x; // pixels/s
    ball->mass = (float)window_size_x / 20.f; // kg
    set_base_circle(ball);
    ball->t0x = get_time(0);
    ball->t0y = ball->t0x;
    ball->dtx = 0;
    ball->dty = 0;
}

int computing_thread(void* d) {
    float v0y = 0, v0x=0;

    const data_t* data = (data_t*)d;
    if (data == NULL) {
        fatalf("Corrupted data ptr\n");
        return 1;
    }

    ball_t* ball = data->b;

    v0x = ball->v0_x; v0y = ball->v0_y;

    reset:
    setup_ball(ball, v0x, v0y);

    SDL_Event e;
    u_short counter = 0;
    u_short collision_counter = 0;
    while (!data->done) {
        if (counter == 0) {
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_QUIT) {
                    free(d);
                    fatalf("SDL quit.\n");
                    return 1;
                }
            }
        }

        // actual computing:
        ball->dtx = get_time(ball->t0x);
        ball->dty = get_time(ball->t0y);
        handle_collision(ball, window_size_x, window_size_y, &collision_counter);
        set_pos(ball);
        // done

        counter++;
        if (collision_counter > 500) {
            goto reset;
        }
    }
    return 0;
}

int key_buf_to_str(const SDL_Keycode* key_buf, char* key_str) {
    if (key_buf == NULL || key_str == NULL) {
        return 1;
    }
    for (int i=0; i < 100; i++) {
        key_str[i] = key_buf[i] > UCHAR_MAX ? '\0' : (char)key_buf[i];
        if (key_str[i] == 0) {
            break;
        }
    }
    return 0;
}

void strip_str(const char* str1, char* str2, const int n) {
    if (str1 == NULL || str2 == NULL) {
        return;
    }
    memset(str2, 0, n);
    int counter = 0;
    for (int i = 0; i < n; i++) {
        if (!isalpha(str1[i]) || !isdigit(str1[i])) {
            str2[counter] = (char)tolower(str1[i]);
            counter++;
        }
    }
    str2[counter] = 0;
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

int main(void) {
    // before optimization
    // 17% cpu 22.4MB memory......
    // after optimization
    // .

    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("demo", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, SDL_WINDOW_SHOWN);
    int *wx = malloc(sizeof(int)), *wy = malloc(sizeof(int));
    SDL_GetWindowSize(window, wx, wy);
    window_size_x = (u_short)*wx;
    window_size_y = (u_short)*wy;
    free(wx); free(wy);

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);

    if (renderer == NULL) {
        fatalf("Failed to create renderer.\n");
    }

    ball_t ball;
    ball.v0_x = 0;
    ball.v0_y = 0;
    const pair_t vx = (pair_t){"vx", &ball.v0_x};
    const pair_t vy = (pair_t){"vy", &ball.v0_y};
    const pair_t g = (pair_t){"g", &G};
    #define nvars 3
    const pair_t vars[] = {vx, vy, g};

restart:
    data_t* data = malloc(sizeof(data_t)); data->b = &ball; data->done = 0;
    SDL_CreateThread(computing_thread, "ball computing", data);

    SDL_Keycode* key_buf = malloc(sizeof(SDL_Keycode) * 100);
    u_short buf_index = 0;
    memset(key_buf, 0, sizeof(SDL_Keycode) * 100);

    SDL_Event e; int quit = 0;
    while( quit == 0 ) {
        while (SDL_PollEvent( &e )) {
            if( e.type == SDL_QUIT ) {
                quit = 1;
            }
            if ( e.type == SDL_KEYUP ) {
                if ( e.key.keysym.sym == SDLK_ESCAPE ) {
                    data->done = 1;
                    free(data);
                    data = NULL;
                    buf_index = 0;
                    free(key_buf);
                    goto restart;
                }
                const SDL_Keycode k = e.key.keysym.sym;
                if ( k == SDLK_BACKSPACE || k == SDLK_DELETE) {
                    if (buf_index > 0) {
                        key_buf[buf_index-1] = 0;
                        buf_index--;
                    }
                } else if ( k == SDLK_RETURN ) {
                    char* key_str = malloc(100);
                    key_buf_to_str(key_buf, key_str);
                    char* res = malloc(100);
                    strip_str(key_str, res, (int)strlen(key_str));
                    char *var_name = malloc(100), *val_str = malloc(100);
                    sep_str(res, var_name, val_str, (u_int)strlen(key_str));
                    const float val = strtof(val_str, NULL);
                    for (int i=0; i < nvars; i++) {
                        if (strcmp(vars[i].name, var_name) == 0) {
                            *(float*)vars[i].var_ptr = val;
                        }
                    }

                    free(var_name);
                    free(val_str);
                    free(key_str);
                    free(res);
                    free(key_buf);
                    free(data);
                    goto restart;
                } else {
                    if (buf_index < 99) {
                        key_buf[buf_index] = k;
                        buf_index++;
                    }
                }
            }
        }

        if (SDL_RenderClear(renderer)) {
            fatalf("Failed to clear render: %s\n", SDL_GetError());
            goto err;
        }
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        drawCircle(renderer, &ball, ball.x + ball.p.x, (float)window_size_y - (ball.y+ball.p.y), ball.n_seg);
        SDL_SetRenderDrawColor(renderer, 10, 10, 10, 255);
        SDL_RenderPresent(renderer);
    }
    data->done = 1;
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    free(data);
    free(key_buf);
    return 0;
    err:
    data->done = 1;
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    free(data);
    free(key_buf);
    return 1;
}
