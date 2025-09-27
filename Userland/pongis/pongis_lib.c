#include "pongis_lib.h"

const char* menuOptions[] = {
    "Jugar (1 jugador)      ",
    "Jugar (2 jugadores)    ",
    "Instrucciones          ",
    "Volver a la Shell      "
};
const int menuCount = sizeof(menuOptions) / sizeof(menuOptions[0]);


uint8_t startPongisMenu(ModeInfo mode) {

    int selected = 0;
    drawMainMenu(selected);
    game_start_sound();
    while (1) {
        if (!isCharReady()) { // Wait for next char
            _hlt();  
            continue;
        }

        char c = getChar();
        switch (c) {
            case 'w':  // Move up
                selected = (selected + menuCount - 1) % menuCount;
                drawMainMenu(selected);
                break;

            case 's':  // Move down
                selected = (selected + 1) % menuCount;
                drawMainMenu(selected);
                break;

            case '\n':
            case '\r':
                return selected;
                break;

            default:
                break;
        }
    }
}

void drawMainMenu(int selected) {
    clear_screen();
    set_zoom(5);
    putString("       PONGIS GOLF\n\n");

    set_zoom(3);
    for (int i = 0; i < menuCount; i++) {
        putString("  ");
        putString(menuOptions[i]);
        if (i == selected) {
            putString("  <--");
        }
        putString("\n\n");
    }
}

void showInstructions() {
    drawInstructions();
    uint8_t key = 0;
    // keep reading until the player really presses 'c'
    do {
        if (isCharReady()) {
            key = getChar();
        }
    } while (key != 'c');
    // now the 'c' is consumed, and we simply return
 }

void drawInstructions(void) {
    clear_screen();
    set_zoom(4);
    putString("      INSTRUCCIONES\n\n");

    set_zoom(2);
    putString(" Jugador 1:\n");
    putString("   W    -> Arriba\n");
    putString("   S    -> Abajo\n");
    putString("   A    -> Izquierda\n");
    putString("   D    -> Derecha\n\n");

    putString(" Jugador 2:\n");
    putString("   Flecha Arriba    -> Arriba\n");
    putString("   Flecha Abajo     -> Abajo\n");
    putString("   Flecha Izquierda -> Izquierda\n");
    putString("   Flecha Derecha   -> Derecha\n\n");

    putString(" Presiona 'c' para volver al menu principal.\n");

    sys_call(SYS_RESET_KBD_BUFFER, 0, 0, 0, 0, 0);

}


float limit_vel(float vel, float min, float max)
{
    return vel < min ? min : (vel > max ? max : vel);
}

void clear_object(Point point, int radius) {
    // Clear the object by drawing a black circle over it
    sys_call(SYS_DRAW_CIRCLE, point.x, point.y, radius, BLACK_COLOR, 0); // 
}

int objects_overlap(Point point1, Point point2, int radius1, int radius2) {
    int dx = point1.x - point2.x;
    int dy = point1.y - point2.y;

    int dist_sq = dx * dx + dy * dy;
    int combined_radius = radius1 + radius2;
    return dist_sq <= (combined_radius * combined_radius);
}

void player_velocity_update(int dir_x, int dir_y, float *vel_x, float *vel_y) {
    if (dir_x != 0 || dir_y != 0) {
        // Calculate normalized input vector (ux, uy)
        float length = sqrtf((float)(dir_x * dir_x + dir_y * dir_y));
        float ux = dir_x / length;
        float uy = dir_y / length;

        // Calculate +-MAX_PLAYER_SPEED for each axis
        float target_vx = ux * MAX_PLAYER_SPEED;
        float target_vy = uy * MAX_PLAYER_SPEED;

        // Detect diagonal to straight shift 
        int shifting_to_cardinal = ((dir_x == 0 && dir_y != 0) || (dir_x != 0 && dir_y == 0));

        // Check if current speed >= SNAP_SPEED_THRESHOLD % of max_sq
        float speed_sq = (*vel_x) * (*vel_x) + (*vel_y) * (*vel_y);
        float max_sq   = MAX_PLAYER_SPEED * MAX_PLAYER_SPEED;
        int   already_fast = (speed_sq >= SNAP_SPEED_THRESHOLD * max_sq);

        if (shifting_to_cardinal && already_fast) {
            // Snap the held axis, gently decay speed of released axis
            if (dir_x != 0 && dir_y == 0) {
                *vel_x = target_vx;
                *vel_y *= RELEASED_AXIS_FRICTION;
            }
            else {
                *vel_y = target_vy;
                *vel_x *= RELEASED_AXIS_FRICTION;
            }
        }
        else {
            // Slowly to the target (using SLOW_ACCEL_RATE)
            float dvx = target_vx - *vel_x;
            float dvy = target_vy - *vel_y;
            *vel_x += dvx * SLOW_ACCEL_RATE;
            *vel_y += dvy * SLOW_ACCEL_RATE;
        }
    }
    else {
        // No key pressed -> standard friction 
        *vel_x *= PLAYER_FRICTION;
        *vel_y *= PLAYER_FRICTION;
    }

    limit_velocity(vel_x, vel_y, IS_PLAYER);
}

void ball_velocity_update(float *vel_x, float *vel_y)
{
    // Friction multiplier for the ball
    *vel_x *= BALL_FRICTION;
    *vel_y *= BALL_FRICTION;

    limit_velocity(vel_x, vel_y, IS_BALL);
}

void limit_velocity(float *vel_x, float *vel_y, int is_player)
{
    int max_speed = (is_player == IS_PLAYER) ? MAX_PLAYER_SPEED : MAX_BALL_SPEED;
    float speed = sqrtf((*vel_x) * (*vel_x) + (*vel_y) * (*vel_y));

    // limit player speed
    if (speed > max_speed)
    {
        float scale = max_speed / speed;
        *vel_x *= scale;
        *vel_y *= scale;
    }
}

void movement_update(PhysicsObject *obj, ModeInfo *mode, int radius)
{
    obj->position.x += (int)obj->vel_x;
    obj->position.y += (int)obj->vel_y;

    float e = (obj->radius == PLAYER_RADIUS) ? PLAYER_WALL_BOUNCE_FACTOR : BALL_WALL_BOUNCE_FACTOR;
    // bounce ball off screen edges
    if (obj->position.x < radius)
    {
        obj->position.x = radius;
        obj->vel_x = -obj->vel_x * e;
    }
    else if (obj->position.x > mode->width - radius)
    {
        obj->position.x = mode->width - radius;
        obj->vel_x = -obj->vel_x * e;
    }
    if (obj->position.y < radius)
    {
        obj->position.y = radius;
        obj->vel_y = -obj->vel_y * e;
    }
    else if (obj->position.y > mode->height - radius)
    {
        obj->position.y = mode->height - radius;
        obj->vel_y = -obj->vel_y * e;
    }


}

uint8_t check_collision(PhysicsObject *obj1, PhysicsObject *obj2, int *counter)
{
    //  Vector from obj1 to obj2
    float dx     = obj2->position.x - obj1->position.x;
    float dy     = obj2->position.y - obj1->position.y;
    float dist_sq = dx*dx + dy*dy;
    float radius_sum    = (float)(obj1->radius + obj2->radius);

    // No overlap 
    if (dist_sq >= radius_sum*radius_sum) {
        return 0;
    }

    // Prepare normals (nx, ny)
    float dist = sqrtf(dist_sq);
    if (dist < 1e-4f) {
        // exactly on top pick a default values
        dx = 1.0f; dy = 0.0f; dist = 1.0f;
    }
    float nx = dx / dist;
    float ny = dy / dist;

    // Detect types by radius
    int obj1IsPlayer = (obj1->radius == PLAYER_RADIUS);
    int obj2IsPlayer = (obj2->radius == PLAYER_RADIUS);
    int obj1IsBall   = (obj1->radius == BALL_RADIUS);
    int obj2IsBall   = (obj2->radius == BALL_RADIUS);

    // Ball & Player (player is infinte mass)
    if ((obj1IsPlayer && obj2IsBall) || (obj2IsPlayer && obj1IsBall)) {
        PhysicsObject *player = obj1IsPlayer ? obj1 : obj2;
        PhysicsObject *ball   = (player == obj1) ? obj2 : obj1;
        (*counter)++;

        // Recompute vector from player to ball
        dx = ball->position.x - player->position.x;
        dy = ball->position.y - player->position.y;
        dist = sqrtf(dx*dx + dy*dy);
        if (dist < 1e-4f) {
            dx = 1.0f; dy = 0.0f; dist = 1.0f;
        }
        nx = dx / dist;  
        ny = dy / dist;

        // Relative velocity (player to ball) along the normal
        float rel_vx = player->vel_x - ball->vel_x;
        float rel_vy = player->vel_y - ball->vel_y;
        float rel_dot = rel_vx*nx + rel_vy*ny;

        // Only if moving into each other
        if (rel_dot > 0.0f) {
            // Impart that normal component onto the ball
            ball->vel_x += rel_dot * nx * BALL_BOUNE_FACTOR;
            ball->vel_y += rel_dot * ny * BALL_BOUNE_FACTOR;
        }

        // Push the ball fully out of the player
        float overlap = radius_sum - dist;
        ball->position.x += (int)roundf(nx * overlap);
        ball->position.y += (int)roundf(ny * overlap);
        return 1;
    }

    // Player & Player: equal-mass elastic collision
    if (obj1IsPlayer && obj2IsPlayer) {
        // Caclulate normal speeds
        float v1n = obj1->vel_x*nx + obj1->vel_y*ny;
        float v2n = obj2->vel_x*nx + obj2->vel_y*ny;
        
        // Check velocity differences 
        if (v1n > v2n) {
            // Decompose each velocity into normal & tangential parts
            float v1t_x  = obj1->vel_x - v1n*nx;
            float v1t_y  = obj1->vel_y - v1n*ny;
            float v2t_x  = obj2->vel_x - v2n*nx;
            float v2t_y  = obj2->vel_y - v2n*ny;

            // Swap the normal components (elastic collision)
            float new_v1n = v2n;
            float new_v2n = v1n;

            // Reconstruct final velocities
            obj1->vel_x = v1t_x + new_v1n*nx;
            obj1->vel_y = v1t_y + new_v1n*ny;
            obj2->vel_x = v2t_x + new_v2n*nx;
            obj2->vel_y = v2t_y + new_v2n*ny;
        }

        // Always seperate objects
        float overlap = radius_sum - dist;
        if (overlap > 0) {
            float half = overlap * 0.5f;
            obj1->position.x -= (int)roundf(nx * half);
            obj1->position.y -= (int)roundf(ny * half);
            obj2->position.x += (int)roundf(nx * half);
            obj2->position.y += (int)roundf(ny * half);
        }
        return 0;
    }

    return 0;
}

// Collisions between obstacles and ball/player
void check_obstacle_collision(PhysicsObject *obj, const Obstacle *obs) {
    if (!obs)
        return;
    
    // Convert positions to floats for better precision
    float px = (float)obj->position.x;
    float py = (float)obj->position.y;
    float ox = (float)obs->point.x;
    float oy = (float)obs->point.y;
    
    // Compute vector from obstacle center to object center
    float dx = px - ox;
    float dy = py - oy;
    float distSq = dx * dx + dy * dy;
    float sumR = (float)(obj->radius + obs->radius);
    
    // Only act if penetration exists
    if (distSq < sumR * sumR) {
        float dist = sqrtf(distSq);
        float nx, ny;
        
        // Compute the collision normal robustly
        if (dist > 1e-4f) {
            nx = dx / dist;
            ny = dy / dist;
        } else {
            // If centers nearly coincide, pick a fallback.
            // You might use the current velocity direction if nonzero.
            float speed = sqrtf(obj->vel_x * obj->vel_x + obj->vel_y * obj->vel_y);
            if (speed > 1e-4f) {
                nx = obj->vel_x / speed;
                ny = obj->vel_y / speed;
            } else {
                nx = 1.0f;
                ny = 0.0f;
            }
        }
        
        // Compute the penetration depth
        float penetration = sumR - dist;
        // Use a small tolerance (slop) to ignore tiny overlaps from rounding.
        float slop = 0.1f;  // Adjust based on your game scale.
        if (penetration > slop) {
            // only move out by the overlap amount
            float correction = penetration - slop;  
            px += nx * correction;
            py += ny * correction;
        }

        // Write the corrected position back (rounding only for rendering if necessary)
        obj->position.x = (int)roundf(px);
        obj->position.y = (int)roundf(py);
        
        // Adjust the velocity so that it’s no longer directed into the obstacle.
        float vDot = obj->vel_x * nx + obj->vel_y * ny;
        if (vDot < 0.0f) {
            if (obj->radius == PLAYER_RADIUS) {
                // Cancel the normal component but allow tangential sliding.
                obj->vel_x -= vDot * nx * (1.0 + PLAYER_WALL_BOUNCE_FACTOR);
                obj->vel_y -= vDot * ny * (1.0 + PLAYER_WALL_BOUNCE_FACTOR);
            } else if (obj->radius == BALL_RADIUS) {
                // Reflect the velocity’s normal component with restitution.
                obj->vel_x -= (1.0f + BALL_WALL_BOUNCE_FACTOR) * vDot * nx;
                obj->vel_y -= (1.0f + BALL_WALL_BOUNCE_FACTOR) * vDot * ny;
            }
        }
    }
}





// Función para verificar colisiones de un objeto con todos los obstáculos del nivel
void check_all_obstacle_collisions(PhysicsObject *obj, const Level *level) {
    if (!level || level->numObstacles <= 0 || !level->obstacles) {
        return;
    }
    
    for (int i = 0; i < level->numObstacles; i++) {
        check_obstacle_collision(obj, (Obstacle*)&level->obstacles[i]);
    }
}

// Función para verificar todas las colisiones (objetos + obstáculos)
void check_all_collisions_with_obstacles(GameState *state) {
    Level *level = state->current_level;

    // Check Player(s) collisions with obstacles
    for (int i = 0; i < state->numPlayers; i++) {
        check_all_obstacle_collisions(&state->players[i].physics, level);
    }

    // Check Ball collision with obstacles
    check_all_obstacle_collisions(&state->ball.physics, level);
}

// Función para dibujar obstáculos circulares
void draw_obstacles(const Level *level) {
    if (!level || level->numObstacles <= 0 || !level->obstacles) {
        return;
    }
    
    for (int i = 0; i < level->numObstacles; i++) {
        const Obstacle *obs = &level->obstacles[i];
        // Dibujar círculo del obstáculo
        sys_call(SYS_DRAW_CIRCLE, obs->point.x, obs->point.y, obs->radius, OBSTACLE_COLOR, 0);
    }
}

uint8_t check_ball_in_hole(GameState *state)
{
    int dx = state->ball.physics.position.x - state->hole.x;
    int dy = state->ball.physics.position.y - state->hole.y;

    uint32_t dist_sq = (uint32_t)(dx * dx + dy * dy);
    int dist = (int)int_sqrt(dist_sq);  

    int threshold = state->holeRadius - BALL_RADIUS;

    return dist <= threshold ? 1 : 0;
}

void draw_player(Point point, int radius, uint32_t color)
{
    sys_call(SYS_DRAW_CIRCLE, point.x, point.y, radius, color, 0);
}

void draw_ball(Point point, int radius)
{
    sys_call(SYS_DRAW_CIRCLE, point.x, point.y, radius, BALL_COLOR, 0);
}

void draw_hole(Point point, int radius)
{
    sys_call(SYS_DRAW_CIRCLE, point.x, point.y, radius, HOLE_COLOR, 0);
}

void draw_counter_box(ModeInfo mode){
    set_cursor(10, 10);
    set_zoom(4);

    // Draw counter in the bottom of screen
    sys_call(SYS_DRAW_RECT,0,mode.height+OFFSET,mode.width,UI,0x00FFFFFF);
    sys_call(SYS_DRAW_RECT,10, mode.height+OFFSET+10,mode.width - 20, UI - 20, 0x00000000); // Black background for counter

}

void draw_counter(int count, ModeInfo mode) {
    set_cursor(10, 10);
    set_zoom(2);
    set_cursor(20, 725);
    putString("Touches: ");
    char buffer[10];
    itoa(count, buffer, 10);
    putString(buffer);
}

void victory_sound() {
    unsigned int melody[] = { 392, 494, 587, 784 }; // E5, G5, B5, C6
    unsigned int duration[] = { 200, 200, 200, 400 };

    for (int i = 0; i < 4; i++) {
        sys_call(SYS_BEEP,melody[i], duration[i],0,0, 0); // Llama al syscall para hacer beep
    }
}

void game_start_sound() {
    unsigned int melody[] = {
             523, 659, 784, 880, 988, 1046
    }; // C5, E5, G5, A5, B5, C6
    unsigned int duration[] = {
             200, 200, 200, 200, 200, 400
    };
    for (int i = 0; i < 6; i++) {
        sys_call(SYS_BEEP, melody[i], duration[i], 0, 0, 0);
    }
}

