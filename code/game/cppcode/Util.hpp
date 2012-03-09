#ifndef Util_hpp
#define Util_hpp

namespace baal {

/**
 * Initialize readline library (used by InterfaceText)
 */
void initialize_readline();

/**
 * Mostly for unit-testing
 */
void setup_singletons();

}

#endif
