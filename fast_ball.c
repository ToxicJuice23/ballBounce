#include "fast_ball.h"
/*
ATTENTION POSSIBLE CONTRIBUTERS
-------------------------------
All of the functions should be using radians for consistency
-------------------------------
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

void set_base_circle(ball_t* ball, const int n_seg) {
    // compute circle now and simply add x and y values to the points...
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

void drawCircle(SDL_Renderer* renderer, const ball_t* ball, const float x, const float y, const int n) {
    // drawCircle(SDL_Renderer*, float, float, int)
    // drawCircle takes a renderer, coordinates, the radius and the smoothness of the circle (number of segments)
    register SDL_FPoint* points = malloc(sizeof(SDL_FPoint)*(n+2));
    memcpy(points, ball->base_circle, sizeof(SDL_FPoint) * (n+1));

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

pos_t get_pos(const ball_t* ball) {
    const float dx = ball->v_x * (ball->dtx);
    const float dy = ball->v_y * ball->dty + 0.5f * G * ball->dty * ball->dty; // removed pow(dty, 2); for speed
    return (pos_t){dx, dy};
}

int handle_collision(ball_t* ball, const u_short size_x, const u_short size_y, u_short* counter) {
    // ball, window size as args
    // returns the angle of collision (in radians)
    const pos_t pos = get_pos(ball);
    const float x = pos.x + ball->x, y = pos.y + ball->y;
    u_int8_t _case = 0;
    float sizes[4] = {x-ball->mass, x+ball->mass, y-ball->mass, y+ball->mass};
    if ((sizes[1] > (float)size_x && (_case = 1)) || (sizes[0] < 0.f && (_case = 2)) || (sizes[3] > (float)size_y && (_case = 3)) || (sizes[2] < 0.f && (_case = 4))) {
        const pos_t p = get_pos(ball);
        switch (_case) {
            case 3:
                case 4:
                ball->v_y = calculate_velocity_elastic_collision(ball->v_y + G*ball->dty);
                ball->y += p.y;
                ball->dty = 0;
                ball->t0y = get_time(0);
                break;
            case 1:
                case 2:
                ball->x += p.x;
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

int main(void)
{
    // before optimization
    // 17% cpu 22.4MB memory......
    // after optimization
    // .
    float v0y = 0, v0x=0;
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wunused-result"
    /*printf("Vitesse Initiale sur x (pixels/s): ");
    scanf("%lf", &v0x);
    printf("Vitesse Initiale sur y (pixels/s): ");
    scanf("%lf", &v0y);
    */
    #pragma GCC diagnostic pop

    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("demo", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP);
    SDL_MaximizeWindow(window);
    int *wx = (int*)malloc(sizeof(int)), *wy = (int*)malloc(sizeof(int));
    SDL_GetWindowSize(window, wx, wy);
    window_size_x = (u_short)*wx;
    window_size_y = (u_short)*wy;
    free(wx); free(wy);

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);

    if (renderer == NULL) {
        fatalf("Failed to create renderer.\n");
    }

    const int n_seg = 100;
reset:
    ball_t ball;
    ball.x = (float)window_size_x / 2.f;
    ball.y = (float)window_size_y / 2.f;
    ball.v_y = v0y; // pixels/s
    ball.v_x = v0x; // pixels/s
    ball.mass = (float)window_size_x / 20.f; // kg
    set_base_circle(&ball, n_seg);
    ball.t0x = get_time(0);
    ball.t0y = ball.t0x;
    ball.dtx = 0;
    ball.dty = 0;
    u_short collision_counter = 0;

    SDL_Event e; int quit = 0; while( quit == 0 ) {
        while (SDL_PollEvent( &e )) {
            if( e.type == SDL_QUIT ) quit = 1;
        }
        ball.dtx = get_time(ball.t0x);
        ball.dty = get_time(ball.t0y);

        handle_collision(&ball, window_size_x, window_size_y, &collision_counter);
        const pos_t p = get_pos(&ball);
        if (SDL_RenderClear(renderer)) {
            fatalf("%s\n", SDL_GetError());
        }

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        drawCircle(renderer, &ball, ball.x + p.x, (float)window_size_y - (ball.y+p.y), n_seg);
        SDL_SetRenderDrawColor(renderer, 10, 10, 10, 255);
        SDL_RenderPresent(renderer);
        if (collision_counter > 500) {
            goto reset;
        }
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    return 0;
}
