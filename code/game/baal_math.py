#! /usr/bin/env python

"""
This file contains some simple mathematical functions
"""

import math, itertools

from baal_common import prequire

###############################################################################
def exp_growth(base, value, threshold=0, diminishing_returns=None):
###############################################################################
    prequire(base >= 1.01 and base <= 1.10, "Invalid base: ", base)

    x = value - threshold

    if (x < 0.0):
        # We're more aggressive on the penalty side. We want to reward
        # the player for having all of the ingredients in place, so
        # being weak in a certain ingrediant is heavily penalized.
        return pow(base + ((base - 1.0) * 2), x)
    elif (diminishing_returns is None or x <= diminishing_returns):
        return pow(base, x)
    else:
        beyond_dim = x - diminishing_returns

        # Compute divisor of exponent based on base; IE divisor of 2 would
        # give sqrt behavior. The idea is for the more-slowly growing functions
        # to plateou faster.
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
def poly_growth(val, exp, div=1):
###############################################################################
    return 0 if val < 0 else pow(val, exp) / div

###############################################################################
def fibonacci_div(total, base):
###############################################################################
    for rv in itertools.count():
        cost = (rv+1) * base
        if (cost > total):
            return rv
        else:
            total -= cost

#
# Tests
#

###############################################################################
def _test():
###############################################################################
    pass

if (__name__ == "__main__"):
    _test()
