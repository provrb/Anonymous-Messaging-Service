#include <time.h>
#include <ctype.h>

// Get the formatted gmt time
struct tm* gmt();
struct tm* gmt_from_time_t(time_t now);

// Convert string to lowercase
void toLowerCase(char* str);

unsigned long randint(unsigned long index);