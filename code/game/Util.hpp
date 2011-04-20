#ifndef Util_hpp
#define Util_hpp

#include <stdlib.h>
#include <string.h>

void initialize_readline ();

char * dupstr (char *s);

char ** baal_completion (const char *text, int start, int end);

char * command_generator (const char *text, int state);

#endif
