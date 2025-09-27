#ifndef PONGIS_TYPES_H
#define PONGIS_TYPES_H

#include <stdint.h>

// Game constants
#define MAX_PLAYERS            2 
#define FIRST_PLAYER_ID        0
#define SECOND_PLAYER_ID       1

#define ONE_PLAYER_MODE       1
#define TWO_PLAYER_MODE       2

#define IS_PLAYER                   1
#define IS_BALL                     0

// Physics constants
#define BALL_RADIUS         10      /* px */
#define PLAYER_RADIUS       20     /* px */

// UI constants
#define UI                     60     /* px reserved for UI messages */
#define OFFSET                 3     /* 60 px for UI + 3 px for offset */

// Colors
#define PLAYER_ONE_COLOR        0x0000FF00  // Green player
#define PLAYER_TWO_COLOR        0x00FF0000  // Red player
#define BALL_COLOR              0x000000FF  // Blue ball
#define HOLE_COLOR              0x00808080  // Grey hole
#define BLACK_COLOR             0x00000000  // Black color for background
#define OBSTACLE_COLOR          0xFFFF00    // Yellow obstacles



// Game phases
typedef enum {
    GAME_PLAYING,
    GAME_LEVEL_COMPLETE,
    GAME_ALL_COMPLETE
} GamePhase;

// Basic point structure
typedef struct Point {
    int x;
    int y;
} Point;

// Physics object structure
typedef struct PhysicsObject {
    Point position;     // Position of the object (x, y)
    float vel_x;
    float vel_y;
    int radius;
    uint32_t color;     // Color of the object
} PhysicsObject;

// Player structure
typedef struct Player {
    PhysicsObject physics;
    int id;             // Player ID (1 or 2)
} Player;

// Ball structure
typedef struct Ball {
    PhysicsObject physics;
    int last_touch_id;
} Ball;

typedef struct {
    Point point;
    int radius;
    uint32_t color; // Color of the obstacle
} Obstacle;

typedef struct {
    Point ball_start;
    Point player_start[MAX_PLAYERS];
    Point hole;
    int holeRadius;
    int numObstacles;
    const Obstacle *obstacles;
} Level;

// Game state structure
typedef struct GameState {
    int currentLevel;
    int numPlayers;     // 1 or 2 players
    
    Player players[MAX_PLAYERS];
    Ball ball;
    
    Point hole;         // Position of the hole
    int holeRadius;
    
    Level * current_level;      // Pointer to the current level
    
    int touch_counter;
    int prev_touch_counter;
} GameState;



#endif // PONGIS_TYPES_H