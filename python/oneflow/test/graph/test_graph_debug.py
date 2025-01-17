"""
Copyright 2020 The OneFlow Authors. All rights reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
"""
import os
import sys
import unittest
import numpy as np

import oneflow as flow
import oneflow.unittest


rank = flow.env.get_rank()


def _graph_debug(test_case, ranks=None):
    class DebugGraph(flow.nn.Graph):
        def __init__(self):
            super().__init__()
            self.m = flow.nn.Linear(3, 3)

        def build(self, x):
            return x

    d_g = DebugGraph()
    d_g.debug(True, 0, ranks)

    if ranks is None:
        rank_list = [0]
    elif isinstance(ranks, int):
        rank_list = [ranks]
    elif isinstance(ranks, list):
        rank_list = ranks

    if -1 in rank_list or rank in rank_list:
        test_case.assertTrue(d_g._debug)
        test_case.assertTrue(d_g.m._debug)
        print(f"ranks {ranks} rank {rank} debug is opened.")
    else:
        test_case.assertTrue(not d_g._debug)
        test_case.assertTrue(not d_g.m._debug)
        print(f"ranks {ranks} rank {rank} debug is closed.")


@unittest.skipIf(os.getenv("ONEFLOW_TEST_CPU_ONLY"), "only test cpu cases")
@flow.unittest.skip_unless_1n4d()
class TestGraphDebug(oneflow.unittest.TestCase):
    def test_graph_debug_rank_null(test_case):
        _graph_debug(test_case)

    def test_graph_debug_rank_0(test_case):
        _graph_debug(test_case, 0)

    def test_graph_debug_rank_1(test_case):
        _graph_debug(test_case, 1)

    def test_graph_debug_rank_1_and_2(test_case):
        _graph_debug(test_case, [1, 2])

    def test_graph_debug_rank_all(test_case):
        _graph_debug(test_case, -1)


if __name__ == "__main__":
    unittest.main()
