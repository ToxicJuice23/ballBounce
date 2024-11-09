#include "../src/main.h"

// unit testing below
// checkmk tests/check_ball.check.c > tests/check_ball.c
#test calculate_velocity_inelastic_collision
    fail_unless(calculate_velocity_elastic_collision(100.f) == -90.f, "Incorrect velocity calculation");