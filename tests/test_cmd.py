import unittest
from tankman.nav import aim_command, turn_command


class TestAimCommand(unittest.TestCase):
    def test_aligned_shoots(self):
        self.assertEqual(aim_command(3, 3), "SHOOT")

    def test_turns_toward_target(self):
        self.assertEqual(aim_command(5, 0), "AIM_LEFT")
        self.assertEqual(aim_command(1, 0), "AIM_RIGHT")

    def test_never_both_sides(self):
        for g in range(8):
            for t in range(8):
                self.assertIn(aim_command(g, t), ("SHOOT", "AIM_LEFT", "AIM_RIGHT"))


class TestTurnCommand(unittest.TestCase):
    def test_aligned_moves_forward(self):
        self.assertEqual(turn_command(2, 2), "FORWARD")

    def test_turns_toward_heading(self):
        self.assertEqual(turn_command(5, 0), "TURN_LEFT")
        self.assertEqual(turn_command(1, 0), "TURN_RIGHT")
