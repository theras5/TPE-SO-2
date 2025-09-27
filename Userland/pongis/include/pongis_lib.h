#ifndef PONGIS_LIB_H
#define PONGIS_LIB_H

#include "pongis.h"
#include "libc.h"
#include "math.h"
#include "pongis_types.h"

// Physics constants
#define ACCELERATION                0.05f   // px/tick² 
#define MAX_PLAYER_SPEED            7.0f   // px/tick 
#define MAX_BALL_SPEED              15.0f  // px/tick 
#define ACCELERATION_RATE           0.6f  

#define BALL_BOUNE_FACTOR           2.0f  // bounce factor for ball when colliding with player
#define PLAYER_WALL_BOUNCE_FACTOR   0.75f  // bounce factor for player when colliding with walls
#define BALL_WALL_BOUNCE_FACTOR     0.9f  // bounce factor for ball when colliding with walls
#define SLOW_ACCEL_RATE             0.3f   // px/tick² 
#define RELEASED_AXIS_FRICTION      0.99f // friction when one axis is released 
#define SNAP_SPEED_THRESHOLD        0.75

#define PLAYER_FRICTION             0.97f  // velocity scaling per frame 
#define BALL_FRICTION               0.99f  // ball velocity scaling per frame 


#define ARROW_UP                    0x80
#define ARROW_DOWN                  0x81
#define ARROW_LEFT                  0x82
#define ARROW_RIGHT                 0x83


uint8_t startPongisMenu(ModeInfo mode);

void drawMainMenu(int selected);

void drawInstructions(void);

int objects_overlap(Point point1, Point point2, int radius1, int radius2);

/**
   @brief: Update velocity for player or ball. If is_player is non-zero, apply acceleration based on dir_x/dir_y, then apply friction. Otherwise, apply friction only.
   @param vel_x:      Pointer to the current x-component of velocity (modifiable).
   @param vel_y:      Pointer to the current y-component of velocity (modifiable).
**/
void ball_velocity_update(float *vel_x, float *vel_y);

/**
   @brief: Clamp a velocity vector so its magnitude does not exceed maximum speed.
   @param vel_x:  Pointer to the x-component of velocity (modifiable).
   @param vel_y:  Pointer to the y-component of velocity (modifiable).
**/
void limit_velocity(float *vel_x, float *vel_y, int is_player);

/**
   @brief: Update an object's position based on its velocity, handling screen boundaries.
   @param raduis: Radius of the object to handle screen boundaries.
   @param obj: Pointer to PhysicsObject containing position and velocity.
**/
void movement_update(PhysicsObject *obj, ModeInfo *mode, int radius);

/**
   @brief: Apply friction or other limits to the ball's velocity only.
   @param state: Pointer to the GameState containing ball_vel_x and ball_vel_y (modifiable).
**/
void ball_limit_velocity(GameState *state);

/**
   @brief: Update the ball's position based on its velocity, handling screen boundaries.
   @param state: Pointer to the GameState containing ball_x, ball_y, ball_vel_x, ball_vel_y.
   @param mode:  Pointer to ModeInfo containing screen dimensions.
**/
void ball_movement_update(GameState *state, ModeInfo *mode);

/**
   @brief: Check if the ball is within the hole region (win condition).
   @param state: Pointer to the GameState containing ball_x, ball_y, and hole parameters.
   @return:      Non-zero (1) if ball is in the hole; zero otherwise.
**/
uint8_t check_ball_in_hole(GameState *state);


/**
   @brief: Display the initial welcome screen and any instructions before the game starts. Blocks until user proceeds.
**/
void display_welcome_screen(ModeInfo mode);

/**
   @brief: Handle user input when in the main menu or pause screen.
   @param mode: ModeInfo containing screen dimensions and menu state.
**/
void handle_menu_input(ModeInfo mode);

void showInstructions();

/**
   @brief: Update the player's velocity using acceleration toward a desired direction and apply friction.
   @param dir_x:  Direction on the x-axis (-1, 0, +1) based on user input.
   @param dir_y:  Direction on the y-axis (-1, 0, +1) based on user input.
   @param vel_x:  Pointer to the current x-component of the player's velocity (modifiable).
   @param vel_y:  Pointer to the current y-component of the player's velocity (modifiable).
**/
void player_velocity_update(int dir_x, int dir_y, float *vel_x, float *vel_y);

/**
   @brief: Check if two objects overlap based on their positions and radius.
   @param obj1: Pointer to the first PhysicsObject.
   @param obj2: Pointer to the second PhysicsObject.
**/
uint8_t check_collision(PhysicsObject *obj1, PhysicsObject *obj2, int *counter_displayed);

void check_obstacle_collision(PhysicsObject *obj, const Obstacle *obs);

void check_all_obstacle_collisions(PhysicsObject *obj, const Level *level);

void check_all_collisions_with_obstacles(GameState *state);
void draw_obstacles(const Level *level);
int is_point_clear_of_obstacles(Point point, int radius, const Level *level);

/** 
   @brief: Verify if the ball is within the hole's radius, indicating a win condition.
   @param state: Pointer to the GameState containing ball_x, ball_y, holeX, holeY, and holeRadius.  
**/
uint8_t check_ball_in_hole(GameState *state);


void draw_player(Point point, int radius, uint32_t color);

void draw_ball(Point point, int radius);

void draw_hole(Point point, int radius);

void draw_counter(int count, ModeInfo mode);
void draw_counter_box(ModeInfo mode);

void clear_object(Point point, int radius);
void victory_sound();
void game_start_sound();
#endif // PONGIS_LIB_H