#include "BaalMath.hpp"
#include "BaalExceptions.hpp"

#include <cmath>

namespace baal {

///////////////////////////////////////////////////////////////////////////////
float exp_growth(float value,
                 float threshold,
                 float base,
                 float diminishing_returns)
///////////////////////////////////////////////////////////////////////////////
{
  Require(base >= 1.01 && base <= 1.10, "Invalid base: " << base);

  float x = value - threshold;

  if (x < 0.0) {
    // We're more aggressive on the penalty side. We want to reward the player
    // for having all of the ingredients in place, so being weak in a certain
    // ingrediant is heavily penalized.
    return std::pow(base + ((base - 1.0) * 2), x);
  }
  else if (x <= diminishing_returns) {
    return std::pow(base, x);
  }
  else {
    float beyond_dim = x - diminishing_returns;

    // Compute divisor of exponent based on base
    float divisor = 2.0;
    if (base <= 1.02) {
      divisor = 5.0;
    }
    else if (base <= 1.03) {
      divisor = 4.0;
    }
    else if (base <= 1.05) {
      divisor = 3.0;
    }

    float additional = std::pow(beyond_dim, 1.0 / divisor) - 1; // root
    if (additional < 0.0) {
      additional = 0.0;
    }

    return std::pow(base, diminishing_returns) + additional;
  }
}

///////////////////////////////////////////////////////////////////////////////
float poly_growth(float value, float threshold, float exp, float divisor)
///////////////////////////////////////////////////////////////////////////////
{
  float x = value - threshold;
  Require(x >= 0.0,
          "poly_growth not intended to be called for values below zero");

  return std::pow(x, exp) / divisor;
}

///////////////////////////////////////////////////////////////////////////////
float linear_growth(float value, float threshold, float multiplier)
///////////////////////////////////////////////////////////////////////////////
{
  return (value - threshold) * multiplier;
}

///////////////////////////////////////////////////////////////////////////////
float sqrt(float value, float threshold)
///////////////////////////////////////////////////////////////////////////////
{
  float x = value - threshold;
  Require(x >= 0.0,
          "poly_growth not intended to be called for values below zero");
  return std::sqrt(x);
}

} // namespace baal
