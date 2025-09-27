#include "level.h"

static const Obstacle level2_obstacles[] = {
            { .point = { .x = 400, .y = 260 }, .radius = 40, .color = OBSTACLE_COLOR },
            { .point = { .x = 400, .y = 340 }, .radius = 40, .color = OBSTACLE_COLOR }
};

static const Obstacle level3_obstacles[] = {
    { .point = { .x = 450, .y = 300 }, .radius = 40, .color = OBSTACLE_COLOR },
    { .point = { .x = 550, .y = 300 }, .radius = 40, .color = OBSTACLE_COLOR }
};

static const Obstacle level4_obstacles[] = {
            { .point = { .x = 400, .y = 100 }, .radius = 30, .color = OBSTACLE_COLOR },
            { .point = { .x = 200, .y = 100 }, .radius = 30, .color = OBSTACLE_COLOR },
            { .point = { .x = 300, .y = 200 }, .radius = 30, .color = OBSTACLE_COLOR },
            { .point = { .x = 300, .y = 250 }, .radius = 15, .color = OBSTACLE_COLOR },
            { .point = { .x = 300, .y = 300 }, .radius = 30, .color = OBSTACLE_COLOR },
            { .point = { .x = 200, .y = 400 }, .radius = 30, .color = OBSTACLE_COLOR },
            { .point = { .x = 500, .y = 100 }, .radius = 30, .color = OBSTACLE_COLOR },
            { .point = { .x = 600, .y = 100 }, .radius = 30, .color = OBSTACLE_COLOR },
            { .point = { .x = 400, .y = 400 }, .radius = 30, .color = OBSTACLE_COLOR },
            { .point = { .x = 500, .y = 400 }, .radius = 30, .color = OBSTACLE_COLOR },
            { .point = { .x = 600, .y = 400 }, .radius = 30, .color = OBSTACLE_COLOR },
            { .point = { .x = 700, .y = 300 }, .radius = 30, .color = OBSTACLE_COLOR },
            { .point = { .x = 700, .y = 200 }, .radius = 30, .color = OBSTACLE_COLOR },
            { .point = { .x = 700, .y = 250 }, .radius = 15, .color = OBSTACLE_COLOR },
};

static const Obstacle level5_obstacles[] = {
    { .point = { .x = 500, .y = 400 }, .radius = 20, .color = OBSTACLE_COLOR },
    { .point = { .x = 400, .y = 500 }, .radius = 20, .color = OBSTACLE_COLOR },
    { .point = { .x = 400, .y = 400 }, .radius = 20, .color = OBSTACLE_COLOR },
    { .point = { .x = 400, .y = 300 }, .radius = 20, .color = OBSTACLE_COLOR },
    { .point = { .x = 400, .y = 200 }, .radius = 20, .color = OBSTACLE_COLOR },
    { .point = { .x = 400, .y = 100 }, .radius = 20, .color = OBSTACLE_COLOR },
    { .point = { .x = 500, .y = 500 }, .radius = 20, .color = OBSTACLE_COLOR },
    { .point = { .x = 500, .y = 500 }, .radius = 20, .color = OBSTACLE_COLOR },
    { .point = { .x = 500, .y = 400 }, .radius = 20, .color = OBSTACLE_COLOR },
    { .point = { .x = 500, .y = 300 }, .radius = 20, .color = OBSTACLE_COLOR },
    { .point = { .x = 500, .y = 200 }, .radius = 20, .color = OBSTACLE_COLOR },
    { .point = { .x = 500, .y = 100 }, .radius = 20, .color = OBSTACLE_COLOR },
    { .point = { .x = 600, .y = 500 }, .radius = 20, .color = OBSTACLE_COLOR },
    { .point = { .x = 600, .y = 400 }, .radius = 20, .color = OBSTACLE_COLOR },
    { .point = { .x = 600, .y = 300 }, .radius = 20, .color = OBSTACLE_COLOR },
    { .point = { .x = 600, .y = 200 }, .radius = 20, .color = OBSTACLE_COLOR },
    { .point = { .x = 600, .y = 100 }, .radius = 20, .color = OBSTACLE_COLOR }
};

static const Obstacle level6_obstacles[] = {
    { .point = { .x = 600, .y = 400 }, .radius = 10, .color = OBSTACLE_COLOR },
    { .point = { .x = 700, .y = 400 }, .radius = 10, .color = OBSTACLE_COLOR },
    { .point = { .x = 650, .y = 350 }, .radius = 10, .color = OBSTACLE_COLOR },
    { .point = { .x = 650, .y = 450 }, .radius = 10, .color = OBSTACLE_COLOR },
    { .point = { .x = 600, .y = 450 }, .radius = 10, .color = OBSTACLE_COLOR },
    { .point = { .x = 600, .y = 350 }, .radius = 10, .color = OBSTACLE_COLOR },
    { .point = { .x = 700, .y = 350 }, .radius = 10, .color = OBSTACLE_COLOR },
    { .point = { .x = 700, .y = 450 }, .radius = 10, .color = OBSTACLE_COLOR }
};

static const Obstacle level7_obstacles[] = {
    { .point = { .x = 500, .y = 400 }, .radius = 65, .color = OBSTACLE_COLOR },
    { .point = { .x = 700, .y = 400 }, .radius = 65, .color = OBSTACLE_COLOR },
    { .point = { .x = 600, .y = 500 }, .radius = 65, .color = OBSTACLE_COLOR },
    { .point = { .x = 600, .y = 300 }, .radius = 65, .color = OBSTACLE_COLOR }
};

static const Obstacle level8_obstacles[] = {
    { .point = { .x = 400, .y = 255 }, .radius = 18, .color = OBSTACLE_COLOR },  
    { .point = { .x = 432, .y = 268 }, .radius = 18, .color = OBSTACLE_COLOR },  
    { .point = { .x = 432, .y = 332 }, .radius = 18, .color = OBSTACLE_COLOR },  
    { .point = { .x = 400, .y = 345 }, .radius = 18, .color = OBSTACLE_COLOR },  
    { .point = { .x = 368, .y = 332 }, .radius = 18, .color = OBSTACLE_COLOR },  
    { .point = { .x = 355, .y = 300 }, .radius = 18, .color = OBSTACLE_COLOR },  
    { .point = { .x = 368, .y = 268 }, .radius = 18, .color = OBSTACLE_COLOR }   
};

const Level levels[9] = {
    // Level 0: A straight shot, no obstacles.
    {
        .ball_start    = { .x = 300, .y = 300 },
        .player_start  = { { .x = 150, .y = 200 }, { .x = 150, .y = 400 } },
        .hole          = { .x = 600, .y = 300 },
        .holeRadius    = 30,
        .numObstacles  = 0,
        .obstacles     = (Obstacle[]){}
    },
    // Level 1: Hole tucked up in a corner.
    {
        .ball_start    = { .x = 300, .y = 300 },
        .player_start  = { { .x = 150, .y = 200 }, { .x = 150, .y = 400 } },
        .hole          = { .x = 550, .y = 100 },
        .holeRadius    = 30,
        .numObstacles  = 0,
        .obstacles     = (Obstacle[]){}
    },
    // Level 2
    {
        .ball_start    = { .x = 300, .y = 300 },
        .player_start  = { { .x = 150, .y = 200 }, { .x = 150, .y = 400 } },
        .hole          = { .x = 600, .y = 300 },
        .holeRadius    = 30,
        .obstacles     = level2_obstacles,
        .numObstacles = sizeof level2_obstacles / sizeof level2_obstacles[0]
    },
    // Level 3
    {
        .ball_start    = { .x = 300, .y = 300 },
        .player_start  = { { .x = 150, .y = 200 }, { .x = 150, .y = 400 } },
        .hole          = { .x = 650, .y = 300 },
        .holeRadius    = 30,
        .obstacles     = level3_obstacles,
        .numObstacles  = sizeof level3_obstacles / sizeof level3_obstacles[0]
    },
    // Level 4
    { 
        .ball_start    = { .x = 100, .y = 250 },
        .player_start  = { { .x = 100, .y = 200 }, { .x = 100, .y = 300 } },
        .hole          = { .x = 500, .y = 250 },
        .holeRadius    = 30,
        .obstacles     = level4_obstacles,
        .numObstacles = sizeof level4_obstacles / sizeof level4_obstacles[0]
    },
    // Level 5
    {
        .ball_start    = { .x = 300, .y = 300 },
        .player_start  = { { .x = 150, .y = 300 }, { .x = 150, .y = 200 } },
        .hole          = { .x = 675, .y = 300 },
        .holeRadius    = 30,
        .obstacles     = level5_obstacles,
        .numObstacles = sizeof level5_obstacles / sizeof level5_obstacles[0]
    },
    // Level 6
    {
        .ball_start    = { .x = 300, .y = 300 },
        .player_start  = { { .x = 150, .y = 300 }, { .x = 150, .y = 200 } },
        .hole          = { .x = 650, .y = 400 },
        .holeRadius    = 30,
        .obstacles     = level6_obstacles,
        .numObstacles = sizeof level6_obstacles / sizeof level6_obstacles[0],
    },
    // Level 7
    {
        .ball_start    = { .x = 300, .y = 400 },
        .player_start  = { { .x = 150, .y = 300 }, { .x = 150, .y = 200 } },
        .hole          = { .x = 600, .y = 400 },
        .holeRadius    = 30,
        .obstacles     = level7_obstacles,
        .numObstacles  = sizeof level7_obstacles / sizeof level7_obstacles[0],
    },
    {
        .ball_start    = { .x = 200, .y = 300 },
        .player_start  = { { .x = 100, .y = 200 }, { .x = 100, .y = 400 } },
        .hole          = { .x = 400, .y = 300 },
        .holeRadius    = 20,
        .obstacles     = level8_obstacles,
        .numObstacles  = sizeof level7_obstacles / sizeof level7_obstacles[0],
    }
};


const uint64_t level_count = sizeof(levels) / sizeof(levels[0]);

void load_level(GameState *state, int index) {
    if (index < 0 || index >= (int)level_count) return;

    state->currentLevel = index;

    state->touch_counter = 0;
    state->prev_touch_counter = 0;

    state->ball.physics.position.x =  levels[index].ball_start.x;
    state->ball.physics.position.y =  levels[index].ball_start.y;
    state->ball.physics.vel_x  = state->ball.physics.vel_y = 0.0f;
    state->ball.physics.radius = BALL_RADIUS;
    state->ball.physics.color  = BALL_COLOR;


    for (int i = 0; i < MAX_PLAYERS; i++) {
        state->players[i].id = i;
        state->players[i].physics.position.x = levels[index].player_start[i].x;
        state->players[i].physics.position.y = levels[index].player_start[i].y;
        state->players[i].physics.radius = PLAYER_RADIUS;
        state->players[i].physics.vel_x = 0.0f;
        state->players[i].physics.vel_y = 0.0f;
    }
    state->players[FIRST_PLAYER_ID].physics.color = PLAYER_ONE_COLOR;
    state->players[SECOND_PLAYER_ID].physics.color = PLAYER_TWO_COLOR;
    state->players[FIRST_PLAYER_ID].id = FIRST_PLAYER_ID;
    state->players[SECOND_PLAYER_ID].id = SECOND_PLAYER_ID;

    state->hole.x = levels[index].hole.x;
    state->hole.y = levels[index].hole.y;
    state->holeRadius = levels[index].holeRadius;

    state->touch_counter =state->prev_touch_counter= 0;

    state->current_level = (Level*)&levels[index];

}
