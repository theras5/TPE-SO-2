#include "commands.h"

void cmd_help() {
    clear_screen(); //funcion para limpiar pantalla
    int prev_zoom = get_zoom();
    set_zoom(2);
    set_cursor(0, 0);
    putString("Available commands:\n");
    putString("  help          - Show this help message\n");
    putString("  time          - Display current system date and time\n");
    putString("  regs          - Show CPU registers\n");
    putString("  pongis        - Launch the Pong-Golf mini-game\n");
    putString("  div0          - Trigger a divide-by-zero exception\n");
    putString("  invalidOp     - Trigger an invalid-opcode exception\n");
    putString("  zoom <NUMBER> - Set text zoom level (1-10)\n");
    putString("  exit          - Exit the shell\n");
    putChar('\n');
    putString("\t Press 'c' to exit window");
    hltUntil_c();
    set_zoom(prev_zoom);
    set_cursor(0,0);
}

void cmd_time() {
    putString("\nCurrent system time: ");
    getTime(); // Assuming getTime() prints the time directly
    putChar('\n');
}

void cmd_registers() {
    clear_screen();
    char buff[1000];
    uint8_t flag = sys_call(SYS_REGS, (uint64_t)buff, 0, 0, 0, 0);
    if (flag == 0) {
        putString(buff);
        putChar('\n');
        putString("\t Press 'c' to quit");
        hltUntil_c();
    } else {
        putString("Registers not available, they need to be saved first.\n");
        putString("Press ESC key to capture registers, then try again.\n");
    }
}


void cmd_div0() {
    exception_sound();
    _div0();
}

void cmd_invalid_opcode() {
    exception_sound();
    _ioc();
}

void cmd_clear(void) {
    clear_screen();
}

void cmd_zoom(int zoom_level) {
    set_zoom(zoom_level);
}

void cmd_exit(void) {
    sys_call(SYS_EXIT, 0, 0, 0, 0, 0);
}