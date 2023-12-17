/**
 * ****************************(C) COPYRIGHT 2023 Blue Bear****************************
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
 * ****************************(C) COPYRIGHT 2023 Blue Bear****************************
 */

#include "Headers/tools.h"
#include <stdlib.h>

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
 * @brief           Return a string as all lowercase
 * @param[in,out]   str: the string to fully lowercase
 * @retval         all lowercased string
 */
void toLowerCase(char* str){
    for (int i = 0; str[i]; i++) {
        str[i] = tolower((unsigned char)str[i]);
    }
}