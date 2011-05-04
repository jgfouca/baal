#ifndef BaalMath_hpp
#define BaalMath_hpp

namespace baal {

/**
 * Important math function used by many spells. This represents an exponential
 * growth function with diminishing returns after a certain point.
 * f(v, t, b, d) = b^(v-t)   // Simplification
 * Beyond d, grows at cube-root of ((v-t) - d)
 *
 * Expect base to be a number slightly larger than 1. 1.02 is considered
 * slow, 1.05 is considered fast.
 */
float exp_growth(float value,
                 float threshold,
                 float base,
                 float diminishing_returns);

/**
 * Important math function used by many spells. Represent polynomial growth.
 * f(v, t, e) = (v-t)^e
 */
float poly_growth(float value, float threshold, float exp, float divisor);

/**
 * f(v, t, m) = (v-t)*m
 */
float linear_growth(float value, float threshold, float multiplier);

/**
 * f(v, t) = sqrt(v-t)
 */
float sqrt(float value, float threshold);

}

#endif
