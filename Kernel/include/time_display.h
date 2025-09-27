#ifndef RTC_H
#define RTC_H

#include <stdint.h>

typedef struct time {
    char day;
    char month;
    char year;
    char hour;
    char min;
    char sec;
} time;

/**
 * @brief Retrieves the current system time.
 *
 * This function returns the current system time as a time structure.
 *
 * @return A time structure containing the current system time.
 */
char* getTime();

extern uint8_t read_RTC(uint8_t reg);

uint8_t get_secs(void);

uint8_t get_mins(void);

uint8_t get_hours(void);

uint8_t get_day(void);

uint8_t get_month(void);

uint8_t get_year(void);

//? necesaria?
void time_display(void);

#endif 
