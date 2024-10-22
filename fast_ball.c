#include "fast_ball.h"
/*
ATTENTION POSSIBLE CONTRIBUTERS
-------------------------------
All of the functions should be using radians for consistency
-------------------------------
todo
    - listen for keystrokes so for example x=500(enter) or x=24f(esc)x=500 will set initial speed on x to 500 and restart
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
    // prompt for x
    printf("Vitesse Initiale sur x (pixels/s): ");
    char* v0_str = malloc(100);
    size_t n = sizeof(v0_str);
    if (getline(&v0_str, &n, stdin) == -1) {
        fatalf("Couldnt read from stdin correctly.\n");
        return -1;
    }
    v0x = strtof(v0_str, NULL);
    // prompt for y
    memset(v0_str, 0, 100);
    printf("Vitesse Initiale sur y (pixels/s): ");
    if (getline(&v0_str, &n, stdin) == -1) {
        fatalf("Couldnt read from stdin correctly.\n");
    }
    v0y = strtof(v0_str, NULL);
    memset(v0_str, 0, 100);
    free(v0_str);

    const data_t* data = (data_t*)d;
    if (data == NULL) {
        fatalf("Corrupted data ptr\n");
        return 1;
    }

    ball_t* ball = data->b;

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

restart:
    ball_t ball;

    data_t* data = malloc(sizeof(data_t)); data->b = &ball; data->done = 0;
    SDL_CreateThread(computing_thread, "ball computing", data);

    u_short counter = 0;
    SDL_Event e; int quit = 0; while( quit == 0 ) {
        if (counter % 100 == 0) {
            while (SDL_PollEvent( &e )) {
                if( e.type == SDL_QUIT ) {
                    quit = 1;
                }
                if ( e.type == SDL_KEYUP ) {
                    if ( e.key.keysym.sym == SDLK_ESCAPE ) {
                        data->done = 1;
                        free(data);
                        data = NULL;
                        goto restart;
                    }
                }
            }
        }

        if (SDL_RenderClear(renderer)) {
            fatalf("Failed to clear render: %s\n", SDL_GetError());
        }
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        drawCircle(renderer, &ball, ball.x + ball.p.x, (float)window_size_y - (ball.y+ball.p.y), ball.n_seg);
        SDL_SetRenderDrawColor(renderer, 10, 10, 10, 255);
        SDL_RenderPresent(renderer);
        counter++;
    }
    data->done = 1;
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    free(data);
    return 0;
}
