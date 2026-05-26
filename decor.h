#ifndef DECOR_H
#define DECOR_H

#include <stdio.h>
#include <string.h>

// Style macros
#define RESET       "\033[0m"
#define BOLD        "\033[1m"
#define DIM         "\033[2m"
#define ITALIC      "\033[3m"
#define UNDERLINE   "\033[4m"
#define BLINK       "\033[5m"
#define REVERSE     "\033[7m"
#define HIDDEN      "\033[8m"
#define STRIKE      "\033[9m"

// Foreground color macros
#define FG_BLACK    "\033[30m"
#define FG_RED      "\033[31m"    // For error
#define FG_GREEN    "\033[32m"    // For completion
#define FG_YELLOW   "\033[33m"
#define FG_BLUE     "\033[34m"
#define FG_MAGENTA  "\033[35m"
#define FG_CYAN     "\033[36m"    // For debug
#define FG_WHITE    "\033[37m"

// Bright foreground colors
#define FG_BBLACK   "\033[90m"
#define FG_BRED     "\033[91m"    //for Error
#define FG_BGREEN   "\033[92m"    //For Completion
#define FG_BYELLOW  "\033[93m"
#define FG_BBLUE    "\033[94m"
#define FG_BMAGENTA "\033[95m"
#define FG_BCYAN    "\033[96m"    // For debug
#define FG_BWHITE   "\033[97m"

// Background colors
#define BG_BLACK    "\033[40m"
#define BG_RED      "\033[41m"
#define BG_GREEN    "\033[42m"
#define BG_YELLOW   "\033[43m"
#define BG_BLUE     "\033[44m"
#define BG_MAGENTA  "\033[45m"
#define BG_CYAN     "\033[46m"
#define BG_WHITE    "\033[47m"

// Bright background colors
#define BG_BBLACK    "\033[100m"
#define BG_BRED      "\033[101m"
#define BG_BGREEN    "\033[102m"
#define BG_BYELLOW   "\033[103m"
#define BG_BBLUE     "\033[104m"
#define BG_BMAGENTA  "\033[105m"
#define BG_BCYAN     "\033[106m"
#define BG_BWHITE    "\033[107m"

#endif