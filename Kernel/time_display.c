#include "time.h"
#include "time_display.h"

// static void format(uint8_t v, char *dst);

static char bcd2char(uint8_t bcd);

//static time t;

char* getTime(void) {
    static char timeStr[9]; 
    
    uint8_t sec = bcd2char(read_RTC(0));
    uint8_t min = bcd2char(read_RTC(2));
    uint8_t hour = bcd2char(read_RTC(4));
    
    // Apply timezone correction (subtract 3 hours)
    if (hour < 3) {
        hour = hour + 24 - 3;  // Handle edge cases
    } else {
        hour = hour - 3;
    }

    // Format the time string
    timeStr[0] = '0' + hour/10;
    timeStr[1] = '0' + hour%10 ;
    timeStr[2] = ':';
    timeStr[3] = '0' + min/10;
    timeStr[4] = '0' + min%10;
    timeStr[5] = ':';
    timeStr[6] = '0' + sec/10;
    timeStr[7] = '0' + sec%10;
    timeStr[8] = '\0';
    
    return timeStr;
}

uint8_t get_secs(void) {
    uint8_t seconds = read_RTC(0);
    return seconds;
}

uint8_t get_mins(void) {
    uint8_t minutes = read_RTC(2);
    return minutes;
}

uint8_t get_hours(void) {
    uint8_t hours = read_RTC(4);
    return hours;
}

uint8_t get_day(void) {
    uint8_t day = read_RTC(7);
    return day;
}

uint8_t get_month(void) {
    uint8_t day = read_RTC(8);
    return day;
}

uint8_t get_year(void) {
    uint8_t year = read_RTC(9);
    return year;
}

// Converts BCD to binary
static char bcd2char(uint8_t bcd) {
    return (char)(((bcd >> 4) * 10) + (bcd & 0x0F));
}
