#ifndef TERM_H
#define TERM_H

/*
 * Expanded Stringify and Stringify macros.
 * Taken from http://gcc.gnu.org/onlinedocs/cpp/Stringification.html
 */
#define XSTR(s) STR(s)
#define STR(s) #s

// Everything in here is defined as a string.
// Use this to convert to a character
#define CHAR(s) (s[0])

// Symbolic names for control sequences
// This is a mixture of ASCII and emacs
#define XOFF "\023"
#define XON  "\021"
#define ESC  "\033"

#define FORWARD_CHAR  "\006"
#define BACKWARD_CHAR "\002"
#define NEXT_LINE     "\016"
#define PREVIOUS_LINE "\020"

// ANSI terminal control sequences
#define CRLF              "\r\n"
#define TERM_CLEAR        ESC "[2J\033[H"
#define TERM_CLEAR_AFTER  ESC "[J"
#define TERM_CURSOR_DOWN  ESC "[B"
#define TERM_CURSOR_RESET ESC "[H"
#define TERM_CURSOR_RIGHT ESC "[C"
#define TERM_KILL_LINE    ESC "[K"
#define TERM_INVERT       ESC "[7m"
#define TERM_STANDARD     ESC "[m"

#endif
