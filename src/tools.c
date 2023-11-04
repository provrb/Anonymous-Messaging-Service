/**
 * ****************************(C) COPYRIGHT {2023} Blue Bear****************************
 * @file       tools.c
 * @brief      Convienant functions to help
 * 
 * @note       
 * @history:
 *   Version   Date            Author          Modification    Email
 *   V1.0.0    Oct-14-2023     Ethan Oliveira                  ethanjamesoliveira@gmail.com
 * 
 * @verbatim
 * ==============================================================================
 * 
 * ==============================================================================
 * @endverbatim
 * ****************************(C) COPYRIGHT {2023} Blue Bear****************************
 */

#include "hdr/tools.h"

/**
 * @brief           Turn int into C string
 * @param[in]       value: int to become the string
 * @param[in,out]   result: int as the string
 * @param[in]       base: how to interperet the int
 * @return          char*
 * @retval         int as a string
 */
char* itoa(int value, char* result, int base) {
    // check that the base if valid
    if (base < 2 || base > 36) { *result = '\0'; return result; }

    char* ptr = result, *ptr1 = result, tmp_char;
    int tmp_value;

    do {
        tmp_value = value;
        value /= base;
        *ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz" [35 + (tmp_value - value * base)];
    } while ( value );

    // Apply negative sign
    if (tmp_value < 0) *ptr++ = '-';
    *ptr-- = '\0';
  
    // Reverse the string
    while(ptr1 < ptr) {
        tmp_char = *ptr;
        *ptr--= *ptr1;
        *ptr1++ = tmp_char;
    }
    return result;
}

/**
 * @brief           get gmt time
 * @return          struct tm*
 * @retval         struct representing current gmt time
 */
struct tm* gmt() {
    time_t timenow;
    struct tm * timestr;
    time( & timenow);
    timestr = gmtime( & timenow);
    return timestr;
}

/**
 * @brief           get gmt time from a time_t variable
 * @param[in]       now: time_t to get a tm* struct from
 * @return          struct tm*
 * @retval         gmt time of the 'now' param
 */
struct tm* gmt_from_time_t(time_t now) {
    struct tm* timestr = gmtime(&now);
    return timestr;
}

/**
 * @brief           Return a string as all lowercase
 * @param[in,out]   str: the string to fully lowercase
 * @retval         all lowercased string
 */
void toLowerCase(char* str){
    for (int i = 0; str[i]; i++) {
        str[i] = tolower((unsigned char)str[i]);
    }
}

/**
 * @brief           Generate a random integar
 * @param[in]       index: seed/number to randomize
 * @return          unsigned long
 * @retval          randomized number
 */
unsigned long randint(unsigned long index){
    index = (index << 13) ^ index;
    return ((index * (index * index * 15731 + 789221) + 1376312589) & 0x7fffffff);
}