#include "pongis_lib.h"


static void handle_input(int *running, GamePhase *phase, GameState *state, int *level_complete_displayed);
static void update_physics(GameState *state, ModeInfo mode, int *counter);
static void render_playing(GameState *state, Point prev_fp, Point prev_sp, Point prev_ball, ModeInfo mode, int *counter_flag);
static void render_level_complete(GameState *state);
static void render_all_levels_complete(void);

void pongis_init(void)
{
    clear_screen();
    ModeInfo mode;
    if (get_mode_info(&mode) != 0) {
        putString("\nError getting mode info\n");
        return;
    }
    mode.height -= UI; // Reserve space for UI
    while (1) {
        uint8_t selected = startPongisMenu(mode);
        switch (selected) {
                    case 0:
                        // One player
                        pongis(mode, ONE_PLAYER_MODE);
                        return;

                    case 1:
                        // Two players
                        pongis(mode, TWO_PLAYER_MODE);
                        clear_screen();
                        return;

                    case 2:
                        // Instructions
                        showInstructions();
                        break;

                    case 3:
                        // Back to shell
                        clear_screen();
                        set_zoom(2);
                        return;

                    default:
                        return;
                }
    }
}

void pongis(ModeInfo mode, int player_count)
{
    clear_screen();

    int running = 1;
    GameState state;
    GamePhase phase = GAME_PLAYING;

    load_level(&state, 0);

    state.numPlayers = player_count;

    // Flags to ensure “Level Complete” / “All Complete”  messages print once
    int level_complete_displayed = 0;
    int all_complete_displayed   = 0;
    int counter_displayed_flag = 0;

    // Draw the hole background exactly once at the start 
    draw_hole(state.hole, state.holeRadius);

    while (running) {
        // Handle any keypress and (possibly) change running or phase 
        handle_input(&running, &phase, &state, &level_complete_displayed);
        Point prev_ball, prev_first_player;
        Point prev_second_player = {0, 0}; // Initalize here to avoid warnings...

        // If PLAYING, update physics
        if (phase == GAME_PLAYING) {
            // Save current positions before updating
            // Previous positions, used for “clearing” only moved objects 
            prev_ball.x = state.ball.physics.position.x;
            prev_ball.y = state.ball.physics.position.y;

            prev_first_player.x = state.players[FIRST_PLAYER_ID].physics.position.x;
            prev_first_player.y = state.players[FIRST_PLAYER_ID].physics.position.y;
            
            if (state.numPlayers == TWO_PLAYER_MODE) {
                prev_second_player.x = state.players[SECOND_PLAYER_ID].physics.position.x;
                prev_second_player.y = state.players[SECOND_PLAYER_ID].physics.position.y;
            }

            update_physics(&state, mode, &state.touch_counter);
        
            if (check_ball_in_hole(&state)) {
                clear_screen();
                
                while (isCharReady()) {
                    getChar(); // Clear input buffer
                }

                if (state.currentLevel + 1 < (int)level_count) {
                    phase = GAME_LEVEL_COMPLETE;
                } else {
                    phase = GAME_ALL_COMPLETE;
                }
            }
        }
        if (phase == GAME_LEVEL_COMPLETE) {
            // If level complete, reset touch counter
            state.touch_counter = 0;
            
        }
        // Render according to current phase
        switch (phase) {
            case GAME_PLAYING:
                render_playing(&state, prev_first_player, prev_second_player, prev_ball, mode, &counter_displayed_flag);
                break;

            case GAME_LEVEL_COMPLETE:
                if (!level_complete_displayed) {
                    render_level_complete(&state);

                    level_complete_displayed = 1;
                    counter_displayed_flag = 0; // Reset counter display flag
                }
                break;

            case GAME_ALL_COMPLETE:
                if (!all_complete_displayed) {
                    render_all_levels_complete();
                    all_complete_displayed = 1;
                    
                }
                break;
        }

        wait_next_tick();
    }
    set_zoom(2);
    clear_screen();
}

/*
   handle_input: read a single character if available, then:
     - In GAME_PLAYING: 'c' quits immediately.
     - In GAME_LEVEL_COMPLETE: ENTER → load next level; 'c' → quit.
     - In GAME_ALL_COMPLETE: 'c' → quit.
*/
static void handle_input(int *running, GamePhase *phase, GameState *state, int *level_complete_displayed)
{
    if (!isCharReady()) {
        return;
    }

    char ch = getChar();

    switch (*phase) {
        case GAME_PLAYING:
            if (ch == 'c' || ch == 'C') {
                *running = 0;
            }
            break;

        case GAME_LEVEL_COMPLETE:
            state->touch_counter = 0;
            if (ch == '\n' || ch == '\r') {
                load_level(state, state->currentLevel + 1);
                clear_screen();               // Draw the hole for the newly loaded level               
                draw_hole(state->hole, state->holeRadius);
                *level_complete_displayed = 0;
                *phase = GAME_PLAYING; // resume physics/rendering loop
            }
            else if (ch == 'c' || ch == 'C') {
                *running = 0;
            }
            break;

        case GAME_ALL_COMPLETE:
            if (ch == 'c' || ch == 'C') {
                *running = 0;
            }
            break;
    }
}

/*
   update_physics: apply input→direction, then move player, ball, check collision.
*/
static void update_physics(GameState *state, ModeInfo mode, int *counter_displayed)
{
    ball_velocity_update(&state->ball.physics.vel_x, &state->ball.physics.vel_y);
    movement_update(&state->ball.physics, &mode, BALL_RADIUS);

    // First player checks
    int p1_dir_x = 0, p1_dir_y = 0;

    // WASD check via sys_call 
    if (sys_call(SYS_IS_KEY_DOWN, (uint64_t)'w', 0, 0, 0, 0)) p1_dir_y -= 1;
    if (sys_call(SYS_IS_KEY_DOWN, (uint64_t)'s', 0, 0, 0, 0)) p1_dir_y += 1;
    if (sys_call(SYS_IS_KEY_DOWN, (uint64_t)'a', 0, 0, 0, 0)) p1_dir_x -= 1;
    if (sys_call(SYS_IS_KEY_DOWN, (uint64_t)'d', 0, 0, 0, 0)) p1_dir_x += 1;

    // Update player velocity based on input
    player_velocity_update(p1_dir_x, p1_dir_y, &state->players[FIRST_PLAYER_ID].physics.vel_x, &state->players[FIRST_PLAYER_ID].physics.vel_y);
    movement_update(&state->players[FIRST_PLAYER_ID].physics, &mode, PLAYER_RADIUS);

    // Check collisions for first player - ball
    check_collision(&state->players[FIRST_PLAYER_ID].physics, &state->ball.physics, &state->touch_counter) == 1 ? state->ball.last_touch_id = FIRST_PLAYER_ID : 0;

    if (state->numPlayers == TWO_PLAYER_MODE) {
        // Second player checks
        int p2_dir_x = 0, p2_dir_y = 0;
        
        // Arrow keys check via sys_call 
        if (sys_call(SYS_IS_KEY_DOWN, (uint64_t)ARROW_UP, 0, 0, 0, 0)) p2_dir_y -= 1;
        if (sys_call(SYS_IS_KEY_DOWN, (uint64_t)ARROW_DOWN, 0, 0, 0, 0)) p2_dir_y += 1;
        if (sys_call(SYS_IS_KEY_DOWN, (uint64_t)ARROW_LEFT, 0, 0, 0, 0)) p2_dir_x -= 1;
        if (sys_call(SYS_IS_KEY_DOWN, (uint64_t)ARROW_RIGHT, 0, 0, 0, 0)) p2_dir_x += 1;
        
        // Update second player velocity based on input
        player_velocity_update(p2_dir_x, p2_dir_y, &state->players[SECOND_PLAYER_ID].physics.vel_x, &state->players[SECOND_PLAYER_ID].physics.vel_y);
        movement_update(&state->players[SECOND_PLAYER_ID].physics, &mode, PLAYER_RADIUS);
        
        // Check collisions for second player - ball  
        check_collision(&state->players[SECOND_PLAYER_ID].physics, &state->ball.physics, &state->touch_counter) == 1 ? state->ball.last_touch_id = SECOND_PLAYER_ID : 0;
        
        // Check collisions between players
        check_collision(&state->players[FIRST_PLAYER_ID].physics, &state->players[SECOND_PLAYER_ID].physics, &state->touch_counter);
    }

    check_all_collisions_with_obstacles(state);
    
}

/*
   render_playing: 
     - Clear only objects that moved (using prev positions).
     - Draw player, ball, and possibly redraw hole if overlapping.
*/
static void render_playing(GameState *state, Point prev_fp, Point prev_sp, Point prev_ball, ModeInfo mode, int *counter_flag)
{
    // If player moved, erase its previous circle
    if (prev_fp.x != state->players[FIRST_PLAYER_ID].physics.position.x || prev_fp.y != state->players[FIRST_PLAYER_ID].physics.position.y) {
        clear_object(prev_fp, PLAYER_RADIUS);
    }
    // If ball moved, erase its previous circle
    if (prev_ball.x != state->ball.physics.position.x || prev_ball.y != state->ball.physics.position.y) {
        clear_object(prev_ball, BALL_RADIUS);
    }

    if (state->numPlayers == TWO_PLAYER_MODE) {
        // If second player moved, erase its previous circle
        if (prev_sp.x != state->players[SECOND_PLAYER_ID].physics.position.x || prev_sp.y != state->players[SECOND_PLAYER_ID].physics.position.y) {
            clear_object(prev_sp, PLAYER_RADIUS);
        }
            //If something overlapped with hole, redraw it
        if (objects_overlap(prev_fp, state->hole, PLAYER_RADIUS, state->holeRadius) ||
            objects_overlap(prev_ball, state->hole, BALL_RADIUS, state->holeRadius) || 
            objects_overlap(prev_sp, state->hole, PLAYER_RADIUS, state->holeRadius)) {
            draw_hole(state->hole, state->holeRadius); 
        }

        draw_player(state->players[SECOND_PLAYER_ID].physics.position, PLAYER_RADIUS, state->players[SECOND_PLAYER_ID].physics.color);

    } else {
        if (objects_overlap(prev_fp, state->hole, PLAYER_RADIUS, state->holeRadius) || objects_overlap(prev_ball, state->hole, BALL_RADIUS, state->holeRadius)) {
            // If first player or ball overlaps with the hole, redraw the hole
            draw_hole(state->hole, state->holeRadius); 
        } 
    }

    draw_player(state->players[FIRST_PLAYER_ID].physics.position, PLAYER_RADIUS, state->players[FIRST_PLAYER_ID].physics.color);
    draw_ball(state->ball.physics.position, BALL_RADIUS);
    draw_obstacles(state->current_level);

    if(!(*counter_flag)){ //draw the counter only once at the start
        draw_counter_box(mode);
        draw_counter(0, mode);
        *counter_flag = 1;
        
    }

    if(state->prev_touch_counter < state->touch_counter){
        draw_counter(state->touch_counter, mode);
        state->prev_touch_counter = state->touch_counter;
    }

}

/*
   render_level_complete: show “Level Complete!” only once per level.
*/
static void render_level_complete(GameState *state)
{
    clear_screen();
    victory_sound();
    // Ajusta el número de '\n' para posicionar verticalmente
    putString("\n\n\n\n");

    // Título principal en Zoom 6, con espacios para centrar horizontalmente
    set_zoom(5);
    putString("  Level Complete!\n\n");

    // Instrucciones en Zoom 4
    set_zoom(3);
    putString("Press ENTER for next level or 'c' to quit\n");
    if (state->numPlayers == TWO_PLAYER_MODE)
    {
        putString("Player ");
        char buff[3] = {0};
        itoa(state->ball.last_touch_id + 1, buff, 10);
        putString(buff); // Convert to string
        putString(" wins!\n");
    }
}

static void render_all_levels_complete()
{
    clear_screen();

    // Ajusta el número de '\n' para posicionar verticalmente
    putString("\n\n\n\n");

    // Título principal en Zoom 6
    set_zoom(6);
    putString("All levels complete!\n\n");

    // Instrucción en Zoom 4
    set_zoom(4);
    putString("Press 'c' to return to shell\n");
}
