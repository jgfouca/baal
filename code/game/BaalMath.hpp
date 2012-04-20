#ifndef BaalMath_hpp
#define BaalMath_hpp

#include <limits>

namespace baal {

const float MAX_FLOAT = std::numeric_limits<float>::max();

/**
 * Important math function used by many spells. This represents an exponential
 * growth function with diminishing returns after a certain point.
 * f(v, t, b, d) = b^(v-t)   // Simplification
 * Beyond d, grows at cube-root of ((v-t) - d)
 *
 * Expect base to be a number slightly larger than 1. 1.02 is considered
 * slow, 1.05 is considered fast.
 */
float exp_growth(float base,
                 float value,
                 float threshold = 0.0,
                 float diminishing_returns = MAX_FLOAT);

/**
 * Important math function used by many spells. Represent polynomial growth.
 * f(v, t, e) = (v-t)^e
 */
float poly_growth(float value,
                  float exp,
                  float divisor = 1.0,
                  float threshold = 0.0);

/**
 * f(v, t, m) = (v-t)*m
 */
float linear_growth(float value, float multiplier, float threshold = 0.0);

/**
 * f(v, t) = sqrt(v-t)
 */
float sqrt(float value,
           float threshold = 0.0);

float fibonacci_div(float total, float base);

}

#endif
