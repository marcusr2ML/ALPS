# ALPS Project: https://alps.comp-phys.org/
# SPDX-License-Identifier: MIT
"""Division of FloatWithError uses first-order error propagation."""

import math

import pytest

from pyalps.floatwitherror import FloatWithError


def assert_value(result, mean, error):
    assert result.mean == pytest.approx(mean, rel=1e-14), (result.mean, mean)
    assert result.error == pytest.approx(error, rel=1e-14), (result.error, error)


def test_divide_by_float_with_error():
    # d(a/b) = sqrt((da/b)^2 + (a*db/b^2)^2)
    a = FloatWithError(2.0, 0.2)
    b = FloatWithError(4.0, 0.1)
    assert_value(a / b, 0.5, math.hypot(0.2 / 4.0, 2.0 * 0.1 / 16.0))


def test_divide_by_number():
    a = FloatWithError(2.0, 0.2)
    assert_value(a / 4, 0.5, 0.05)
    assert_value(a / 4.0, 0.5, 0.05)


def test_divide_number_by_float_with_error():
    a = FloatWithError(2.0, 0.2)
    # d(c/a) = c*da/a^2
    assert_value(3.0 / a, 1.5, 3.0 * 0.2 / 4.0)
    assert_value(3 / a, 1.5, 3.0 * 0.2 / 4.0)


def test_in_place_divide():
    a = FloatWithError(2.0, 0.2)
    a /= FloatWithError(4.0, 0.1)
    assert_value(a, 0.5, math.hypot(0.2 / 4.0, 2.0 * 0.1 / 16.0))
