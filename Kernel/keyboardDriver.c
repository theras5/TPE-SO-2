#include "keyboardDriver.h"
#include "interrupts.h"        


static char           buf[BUF_SIZE];
static volatile int   head = 0;
static volatile int   tail = 0;
static uint8_t        shift = 0;
static uint8_t        caps = 0;
static uint8_t        extended;              // for 0xE0 prefix

static char           buff_reg[1000]; 
static uint8_t registers_formatted = 0; // Flag to track if registers are formatted
static volatile uint8_t key_state[KEY_STATE_SIZE] = {0};

static inline char to_lower(char c) {
    if (c >= 'A' && c <= 'Z') {
        return (char)(c - 'A' + 'a');
    }
    return c;
}

static inline char to_upper(char c) {
    if (c >= 'a' && c <= 'z') {
        return (char)(c - 'a' + 'A');
    }
    return c;
}

/* Partial scancode→ASCII tables (fill out the rest as needed) */
static const char tbl_no_shift[128] = {
    [0x01] = 27,    // ESC
    [0x02] = '1', [0x03] = '2', [0x04] = '3', [0x05] = '4',
    [0x06] = '5', [0x07] = '6', [0x08] = '7', [0x09] = '8',
    [0x0A] = '9', [0x0B] = '0',
    [0x0C] = '-', [0x0D] = '=',
    [0x0E] = '\b', // Backspace
    [0x0F] = '\t', // Tab
    [0x10] = 'q', [0x11] = 'w', [0x12] = 'e', [0x13] = 'r',
    [0x14] = 't', [0x15] = 'y', [0x16] = 'u', [0x17] = 'i',
    [0x18] = 'o', [0x19] = 'p',
    [0x1A] = '[', [0x1B] = ']',
    [0x1C] = '\n',// Enter
    /* 0x1D = Left Ctrl */
    [0x1E] = 'a', [0x1F] = 's', [0x20] = 'd', [0x21] = 'f',
    [0x22] = 'g', [0x23] = 'h', [0x24] = 'j', [0x25] = 'k',
    [0x26] = 'l',
    [0x27] = ';', [0x28] = '\'', [0x29] = '`',
    /* 0x2A = Left Shift */
    [0x2B] = '\\',
    [0x2C] = 'z', [0x2D] = 'x', [0x2E] = 'c', [0x2F] = 'v',
    [0x30] = 'b', [0x31] = 'n', [0x32] = 'm',
    [0x33] = ',', [0x34] = '.', [0x35] = '/',
    /* 0x36 = Right Shift */
    [0x37] = '*',
    /* 0x38 = Left Alt */
    [0x39] = ' ', // Space
    /* 0x3A = Caps Lock, function keys, etc. left as 0 */
};

static const char tbl_shift[128] = {
    [0x01] = 27,    // ESC
    [0x02] = '!', [0x03] = '@', [0x04] = '#', [0x05] = '$',
    [0x06] = '%', [0x07] = '^', [0x08] = '&', [0x09] = '*',
    [0x0A] = '(', [0x0B] = ')',
    [0x0C] = '_', [0x0D] = '+',
    [0x0E] = '\b', // Backspace
    [0x0F] = '\t', // Tab
    [0x10] = 'Q', [0x11] = 'W', [0x12] = 'E', [0x13] = 'R',
    [0x14] = 'T', [0x15] = 'Y', [0x16] = 'U', [0x17] = 'I',
    [0x18] = 'O', [0x19] = 'P',
    [0x1A] = '{', [0x1B] = '}',
    [0x1C] = '\n',// Enter
    /* 0x1D = Left Ctrl */
    [0x1E] = 'A', [0x1F] = 'S', [0x20] = 'D', [0x21] = 'F',
    [0x22] = 'G', [0x23] = 'H', [0x24] = 'J', [0x25] = 'K',
    [0x26] = 'L',
    [0x27] = ':', [0x28] = '"', [0x29] = '~',
    /* 0x2A = Left Shift */
    [0x2B] = '|',
    [0x2C] = 'Z', [0x2D] = 'X', [0x2E] = 'C', [0x2F] = 'V',
    [0x30] = 'B', [0x31] = 'N', [0x32] = 'M',
    [0x33] = '<', [0x34] = '>', [0x35] = '?',
    /* 0x36 = Right Shift */
    [0x37] = '*',
    /* 0x38 = Left Alt */
    [0x39] = ' ', // Space
    /* others left as 0 */
};

static const char * reg_labels[] = {
        "RAX:    0x", "RBX:    0x", "RCX:    0x", "RDX:    0x", 
        "RBP:    0x", "RDI:    0x", "RSI:    0x", "R8:     0x", 
        "R9:     0x", "R10:    0x", "R11:    0x", "R12:    0x", 
        "R13:    0x", "R14:    0x", "R15:    0x", "RIP:    0x",
        "CS:     0x", "RFLAGS: 0x", "RSP:    0x", "SS:     0x"
    };
    
void kbd_get_key(void) {
    uint8_t sc = getPressedKey();

    if (sc == 0xE0) {
        extended = 1;
        return;
    }

    uint8_t make_flag = !(sc & 0x80);
    uint8_t code = sc & 0x7F;

    // Check for arrows 
    if (extended) {
        extended = 0;
        uint8_t arrow = 0;
        switch (code) {
            case 0x48: arrow = ARROW_UP;    break;
            case 0x50: arrow = ARROW_DOWN;  break;
            case 0x4B: arrow = ARROW_LEFT;  break;
            case 0x4D: arrow = ARROW_RIGHT; break;
            default:  return;  // some other extended key
        }
        key_state[arrow] = make_flag;
        return;
    }

    // Shift keys 
    if (code == 0x2A || code == 0x36) {
        shift = make_flag;
        return;
    }
    // Caps Lock 
    else if (code == 0x3A && make_flag) {
        caps = !caps;
        return;
    }
    
    char key = tbl_no_shift[code];
    if (key == 0) { // check if the key is printable
        return;     // not printable, skip
    }
    
    if (!make_flag) { // break code
        key_state[(uint8_t)key] = 0; // turn off key state
        return; 
    }

    // we now know its a make code and a printable key
    char ch = shift ? tbl_shift[code] : tbl_no_shift[code];
    
    key_state[(uint8_t)tbl_no_shift[code]] = 1; // mark key as pressed (ignore shift state)
    
    // apply CapsLock for letters 
    if (caps) ch = to_upper(ch);

    buf[head] = ch;
    head = (head + 1) % BUF_SIZE;
    if (head == tail) {
        // buffer overrun → drop oldest
        tail = (tail + 1) % BUF_SIZE;
    }
}

uint8_t kbd_has_char(void) {
    return head != tail;
}

char kbd_get_char(void) {
    if (head == tail) {
        return 0;
    } 
    char c = buf[tail];
    tail = (tail + 1) % BUF_SIZE; 
    return c;
}

void kbd_reset_buff() {
    head = tail;
    return;
}

uint64_t kbd_is_key_down(char key) {
    return key_state[(uint8_t) key];
}

void format_registers() {
    
    uint32_t k = 0; // Index for buff_reg

    for (int i = 0; i < 20; i++) {
        // Copy the label (e.g., "RAX:    0x")
        int j = 0;
        while(reg_labels[i][j] != 0) {
            buff_reg[k++] = reg_labels[i][j++];
        }

        // Add the register value in hex format
        k += uint64ToRegisterFormat(reg_array[i], buff_reg + k);
        
        // Add newline
        buff_reg[k++] = '\n';
    } 
    buff_reg[k] = 0; // Fixed: buff_reg instead of reg_buff
    registers_formatted = 1; // Mark as formatted
}

uint32_t uint64ToRegisterFormat(uint64_t value, char *dest) {
    // We want exactly 16 hex digits (64 bits = 16 hex chars)
    const char hex_chars[] = "0123456789ABCDEF";
    uint32_t chars_written = 0;
    
    // Write 16 hex digits, starting from most significant
    for (int i = 15; i >= 0; i--) {
        uint8_t hex_digit = (value >> (i * 4)) & 0xF;
        dest[chars_written++] = hex_chars[hex_digit];
    }
    
    return chars_written; // Always returns 16
}


uint64_t copy_regs(char *copy) {
    // Check if ESC was pressed (registers were saved)
    if (!getEscPressed()) {
        return 1; // No registers available
    }

    // Format registers only when we need to print them
    if (!registers_formatted) {
        format_registers();
    }

    // Copy the formatted string
    int i = 0;
    while (buff_reg[i] != 0 && i < 999) { // Prevent buffer overflow
        copy[i] = buff_reg[i];
        i++;
    }
    copy[i] = 0; // null terminate

    // Reset flags for next use
    registers_formatted = 0;

    // We might consider resetting the ESC flag here... it depends
    
    return 0; // Success
}