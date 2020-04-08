#include "../src/helpers.h"

void logger(const char *format, ...)
{
    va_list argList;

    // Initialize variable argument list
    va_start(argList, format);

    vprintf(format, argList);
    fflush(stdout);  // Immediately print to terminal

    // Free variable argument list
    va_end(argList);
}