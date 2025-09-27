#include "shell.h"

#define MAX_CMD_LENGTH 256 

static int first_run = 1;
void shell(void) {
    if(first_run){
        first_run = 0;
        clear_screen();
        draw_welcome();
        system_start_sound();
        hltUntil_c();
        putString("Write help for a list of commands (CAPS Sensitive)\n");
    }

    while (1) {
        char command[MAX_CMD_LENGTH];
        int len = 0;

        // Prompt for input
        putString("\\> ");

        // Read command
        while (len < MAX_CMD_LENGTH - 1) {
            char c = getChar();
            if (c == '\n' || c == '\r') {
                break; // End of command
            } else if ((c == 0x7F || c == 0x08) && len > 0) { // Handle backspace
                putChar('\b');
                len--;
            } else if (len < MAX_CMD_LENGTH-1 && c >= ' ') {
                command[len++] = c;
                
                putChar(c);
            } else if (len >= MAX_CMD_LENGTH - 1) {
                putString("\nCommand too long, try again.\n");
                len = 0; // Reset length for new command
                break; // Exit the input loop
            }
        }
        command[len] = '\0';   
             
        if      (strcmp(command, "help")==0)            cmd_help();
        else if (strcmp(command, "time")==0)            cmd_time();
        else if (strcmp(command, "clear")==0)           cmd_clear();
        else if (strcmp(command, "regs")==0)            cmd_registers();
        else if (strcmp(command, "div0")==0)            cmd_div0();
        else if (strcmp(command, "invalidOp")==0)       cmd_invalid_opcode();
        else if (strcmp(command, "exit")==0)            cmd_exit();
        else if (strcmp(command, "pongis")==0){
            int prev_zoom = get_zoom();
            pongis_init();
            set_zoom(prev_zoom);
        }
        else if (strncmp(command, "zoom ", 5)==0) {     // Check if command starts with "zoom "
            char* arg = command + 4;                    // Point to the argument after "zoom "
            int valid = 1;      
            int zoom_level = 0;         
            if (*arg == ' ') {
                arg++;                                  // Skip the space after "zoom "
            
                // Validate and convert the numeric argument
                while (*arg != '\0') {
                    if (*arg >= '0' && *arg <= '9') {
                        zoom_level = zoom_level * 10 + (*arg - '0');
                    } else {
                        valid = 0;
                        break;
                    }
                    arg++;
                }
                
                
            } 
            if (valid) {
                cmd_zoom(zoom_level);
            } else {
                putString("\nInvalid zoom level. Usage: zoom <number (Between 1 and 10)>\n");
                
            }
        }
        else {
            putString("\nUnknown command\n");
        }
    }
}