import unittest
from tankman.nav import to_cell, wall_cells, has_line_of_sight, bfs_next_step, CELL


class TestFinalScenario(unittest.TestCase):
    """重現決賽:我方(左下)與敵人(右上對角)中間卡一塊牆。"""

    def setUp(self):
        self.me = (5 * CELL + 25, 6 * CELL + 25)      # 格子 (5,6)
        self.enemy = (7 * CELL + 25, 4 * CELL + 25)   # 格子 (7,4),右上對角
        self.walls_info = [{"x": 6 * CELL, "y": 5 * CELL}]   # 牆卡中間 (6,5)
        self.walls = wall_cells(self.walls_info)

    def test_wall_blocks_line_of_sight(self):
        self.assertFalse(has_line_of_sight(*self.me, *self.enemy, self.walls))

    def test_bfs_finds_a_way_around(self):
        start = to_cell(*self.me)
        goal = to_cell(*self.enemy)
        step = bfs_next_step(start, goal, self.walls)
        self.assertIsNotNone(step)
        self.assertNotIn(step, self.walls)
        self.assertEqual(abs(step[0]-start[0]) + abs(step[1]-start[1]), 1)

    def test_no_wall_means_clear_shot(self):
        self.assertTrue(has_line_of_sight(*self.me, *self.enemy, set()))
