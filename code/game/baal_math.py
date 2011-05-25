#! /usr/bin/env python

"""
This file contains some simple mathematical functions
"""

import math

from baal_common import prequire

###############################################################################
def exp_growth(value, threshold, base, diminishing_returns):
###############################################################################
    prequire(base >= 1.01 && base <= 1.10, "Invalid base: ", base)

    x = value - threshold

    if (x < 0.0):
        # We're more aggressive on the penalty side. We want to reward
        # the player for having all of the ingredients in place, so
        # being weak in a certain ingrediant is heavily penalized.
        return pow(base + ((base - 1.0) * 2), x)
    elif (x <= diminishing_returns):
        return pow(base, x)
    else:
        beyond_dim = x - diminishing_returns

        # Compute divisor of exponent based on base
        divisor = 2.0
        if (base <= 1.02):
            divisor = 5.0
        elif (base <= 1.03):
            divisor = 4.0
        elif (base <= 1.05):
            divisor = 3.0

        additional = pow(beyond_dim, 1.0 / divisor) - 1 # root
        if (additional < 0.0):
            additional = 0.0

        return pow(base, diminishing_returns) + additional

###############################################################################
def _check(val):
###############################################################################
    """
    Check that val is >= 0
    """
    prequire(val >= 0, "Val is ", val)
    return val

###############################################################################
poly_growth = lambda val, exp, div: pow(_check(val), exp) / div
###############################################################################

###############################################################################
linear_growth = lambda val, mult: val * mult
###############################################################################

###############################################################################
sqrt = lambda val: math.sqrt(_check(val))
###############################################################################

#
# Tests
#

###############################################################################
def _test():
###############################################################################
    pass

if (__name__ == "__main__"):
    _test()
