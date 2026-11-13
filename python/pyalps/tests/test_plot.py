# ALPS Project: https://alps.comp-phys.org/
# SPDX-License-Identifier: MIT
import matplotlib

matplotlib.use("Agg")

import matplotlib.pyplot as plt
import numpy as np
from matplotlib.container import ErrorbarContainer

from pyalps.dataset import DataSet
from pyalps.floatwitherror import FloatWithError
from pyalps.plot import MplXYPlot_core


def test_scatter_with_error_bars():
    d = DataSet(np.array([1.0, 2.0]),
                np.array([FloatWithError(10.0, 0.5), FloatWithError(20.0, 1.5)]))
    d.props["line"] = "scatter"
    d.props["label"] = "with errors"
    plt.figure()
    try:
        MplXYPlot_core()({"data": [d]})
        ax = plt.gca()
        offsets = ax.collections[0].get_offsets()
        np.testing.assert_array_equal(offsets, [[1.0, 10.0], [2.0, 20.0]])
        bars = [c for c in ax.containers if isinstance(c, ErrorbarContainer)]
        assert len(bars) == 1
        segments = bars[0].lines[2][0].get_segments()
        np.testing.assert_array_equal(segments, [[[1.0, 9.5], [1.0, 10.5]],
                                                 [[2.0, 18.5], [2.0, 21.5]]])
    finally:
        plt.close("all")
