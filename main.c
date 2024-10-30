//
// Created by jujur on 30/10/24.
//
#include "main.h"

int main(void) {
    // before optimization
    // 17% cpu 22.4MB memory......
    // after optimization
    // .

    // setup window and renderer
    SDL_Window* window = setup_window_sdl();
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
    if (renderer == NULL) {
        fatalf("Failed to create renderer.\n");
        return -1;
    }

    ball_t ball;
    ball.v0_x = 0;
    ball.v0_y = 0;
    // define pair array for in game vars
    const pair_t vars[] = {(pair_t){"vx",
        &ball.v0_x},
        (pair_t){"vy", &ball.v0_y},
        (pair_t){"g", &G},
        // null terminator
        (pair_t){NULL, NULL}};

    // set length of the array
    int nvars = 0;
    // explanation: ++nvars < -1 will evaluate to false, it only gets evaluated if the other part of the if is false so its like if else.
    while (nvars >= 0) { if ((vars[nvars].name == NULL && vars[nvars].var_ptr == NULL) || ++nvars < -1) break;}
    // dont kill the thread every restart lol
    data_t* data = malloc(sizeof(data_t)); data->b = &ball; data->done = 0;
    SDL_CreateThread(computing_thread, "ball computing", data);

restart:
    SDL_Keycode* key_buf = malloc(sizeof(SDL_Keycode) * 100);
    u_short buf_index = 0;
    memset(key_buf, 0, sizeof(SDL_Keycode) * 100);

    SDL_Event e; int quit = 0;
    while( quit == 0 ) {
        while (SDL_PollEvent( &e )) {
            if( e.type == SDL_QUIT ) {
                quit = 1;
                break;
            }
            if ( e.type == SDL_KEYUP ) {
                const SDL_Keycode k = e.key.keysym.sym;
                if ( k == SDLK_ESCAPE ) {
                    free(key_buf);
                    goto restart;
                }
                if ( k == SDLK_BACKSPACE || k == SDLK_DELETE) {
                    if (buf_index > 0) {
                        key_buf[buf_index-1] = 0;
                        buf_index--;
                    }
                } else if ( k == SDLK_RETURN ) {
                    if (buf_index == 0) goto l84;

                    // allocate
                    char* key_str = malloc(100), *var_name = malloc(100), *val_str = malloc(100);;
                    key_buf_to_str(key_buf, key_str);
                    strip_str(&key_str, (int)strlen(key_str));

                    sep_str(key_str, var_name, val_str, (u_int)strlen(key_str));
                    const float val = strtof(val_str, NULL);
                    for (int i=0; i < nvars; i++) {
                        if (strcmp(vars[i].name, var_name) == 0) {
                            *(float*)vars[i].var_ptr = val;
                        }
                    }

                    free(var_name);
                    free(val_str);
                    free(key_str);
                    free(key_buf);
                    goto restart;
                } else {
                    l84:
                    if (buf_index < 99) {
                        key_buf[buf_index] = k;
                        buf_index++;
                    }
                }
            }
        }

        if (SDL_RenderClear(renderer)) {
            fatalf("Failed to clear render: %s\n", SDL_GetError());
        }
        // draw frame
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        drawCircle(renderer, &ball, ball.x + ball.p.x, (float)window_size_y - (ball.y+ball.p.y), ball.n_seg);
        SDL_SetRenderDrawColor(renderer, 10, 10, 10, 255);
        SDL_RenderPresent(renderer);
    }
    // clean up
    data->done = 1;
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    free(data);
    free(key_buf);
    return 0;
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