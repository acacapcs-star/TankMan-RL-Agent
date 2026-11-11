import unittest
from tankman.nav import (to_cell, obstacle_cells, pick_open_neighbor,
                         bfs_next_step, CELL)


class TestObstacles(unittest.TestCase):
    def test_teammate_counts_as_obstacle_but_not_self(self):
        walls = [{"x": 6*CELL, "y": 5*CELL}]
        mates = [{"id": "1P", "x": 5*CELL, "y": 6*CELL},   # 自己
                 {"id": "2P", "x": 3*CELL, "y": 3*CELL}]   # 隊友
        obs = obstacle_cells(walls, mates, my_id="1P")
        self.assertIn((6, 5), obs)
        self.assertIn((3, 3), obs)
        self.assertNotIn((5, 6), obs)      # 自己不被算進去


class TestGreedyFinals(unittest.TestCase):
    def test_sidesteps_single_block(self):
        me, enemy = (5, 6), (7, 4)
        obstacles = {(6, 5)}
        step = pick_open_neighbor(me, enemy, obstacles)
        self.assertIsNotNone(step)
        self.assertNotIn(step, obstacles)
        d_old = abs(me[0]-enemy[0]) + abs(me[1]-enemy[1])
        d_new = abs(step[0]-enemy[0]) + abs(step[1]-enemy[1])
        self.assertLess(d_new, d_old)


def simulate(stepper, start, goal, obstacles, max_steps=50):
    cur, seen = start, set()
    for _ in range(max_steps):
        if cur == goal:
            return True
        if cur in seen:
            return False          # 鬼打牆
        seen.add(cur)
        nxt = (pick_open_neighbor(cur, goal, obstacles) if stepper == "greedy"
               else bfs_next_step(cur, goal, obstacles))
        if nxt is None:
            return False
        cur = nxt
    return False


class TestConcaveTrap(unittest.TestCase):
    """凹形牆:貪心會卡,BFS 會繞出去。"""

    def setUp(self):
        self.start = (5, 6)
        self.goal = (5, 2)
        self.obstacles = {(4, 5), (6, 5), (5, 4)}

    def test_greedy_gets_stuck(self):
        self.assertFalse(simulate("greedy", self.start, self.goal, self.obstacles))

    def test_bfs_escapes(self):
        self.assertTrue(simulate("bfs", self.start, self.goal, self.obstacles))
