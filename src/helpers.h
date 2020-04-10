#ifndef HELPERS_H_
#define HELPERS_H_

#include "../src/cpuStructures.h"

/**
 Immediately prints content to console.
 Same call signature as printf().

 Achieves this by calling vprintf & fflush(stdout)

 @param format - print format specifier
 @param ... - variable quantity of arguments to be printed
 */
void logger(const char *format, ...);

/**
 Returns the 2's complement of an 8-bit number

 @param num - number to complement
 @return - 2's complement of num
 */
uint8_t twosComplement(uint8_t num);

#endif  // HELPERS_H_