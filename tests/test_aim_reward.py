"""AimEnv 獎勵函數與角度邏輯的單元測試。

測的是 RL 環境裡最關鍵、也最容易寫錯的一塊：**環狀角度的相對方位判斷**。

360 度被切成 8 段（DEGREES_PER_SEGMENT = 45），砲管方向與目標方向都是 0–7 的索引。
判斷「該往左轉還是往右轉」必須在模 8 的環上做——一旦寫成普通的大小比較，
砲管在 index 0、目標在 index 7 時就會轉錯方向，而且訓練時不會報錯，
只會讓 agent 學不起來。

這些測試把獎勵函數的規則獨立抽出來驗證，不需要啟動 pygame 或完整遊戲環境。

執行：
    python -m pytest tests/ -v
或：
    python tests/test_aim_reward.py
"""

import unittest

# ── 環境常數（與 gym_env/tankman/aim_env.py 一致）─────────────
DEGREES_PER_SEGMENT = 45
TOTAL_SEGMENTS = 360 // DEGREES_PER_SEGMENT  # 8

# 動作編號
AIM_LEFT = 1
AIM_RIGHT = 2
SHOOT = 3
IDLE = 0


# ── 被測邏輯：與 AimEnv.cal_angle_reward / cal_shoot_reward 相同 ──
def cal_angle_reward(obs, action):
    """obs = [砲管方向 index, 目標方向 index]，兩者皆為 0–7。"""
    gun, target = obs[0], obs[1]

    # 目標在砲管的右側（順時針 5、6、7 段外）→ 應該往左轉
    if gun in [(target + x) % TOTAL_SEGMENTS for x in [5, 6, 7]]:
        return 1.0 if action == AIM_LEFT else (-1.0 if action == AIM_RIGHT else 0.0)

    # 目標在砲管的左側 → 應該往右轉
    if gun in [(target + x) % TOTAL_SEGMENTS for x in [1, 2, 3]]:
        return 1.0 if action == AIM_RIGHT else (-1.0 if action == AIM_LEFT else 0.0)

    # 正對面（相差 4 段 = 180 度）：左右皆可，但轉錯沒有意義，給小懲罰
    if gun == (target + 4) % TOTAL_SEGMENTS and action in (AIM_LEFT, AIM_RIGHT):
        return -0.5

    return 0.0


def cal_shoot_reward(obs, action):
    gun, target = obs[0], obs[1]
    if gun == target and action == SHOOT:
        return 1.0          # 對準了才開火
    if action == SHOOT:
        return -1.0         # 沒對準就開火
    if gun == target:
        return -0.5         # 對準了卻不開火
    return 0.0


def total_reward(obs, action):
    return cal_angle_reward(obs, action) + cal_shoot_reward(obs, action)


# ══════════════════════════════════════════════════════════
class TestAngleRewardBasics(unittest.TestCase):
    """基本方位：目標在左轉左、在右轉右才有正獎勵。"""

    def test_target_to_the_left_reward_turning_right(self):
        # gun=1, target=0 → gun 在 target 的 +1 段 → 應往右轉
        self.assertEqual(cal_angle_reward([1, 0], AIM_RIGHT), 1.0)

    def test_target_to_the_left_penalise_turning_left(self):
        self.assertEqual(cal_angle_reward([1, 0], AIM_LEFT), -1.0)

    def test_target_to_the_right_reward_turning_left(self):
        # gun=5, target=0 → gun 在 target 的 +5 段 → 應往左轉
        self.assertEqual(cal_angle_reward([5, 0], AIM_LEFT), 1.0)

    def test_target_to_the_right_penalise_turning_right(self):
        self.assertEqual(cal_angle_reward([5, 0], AIM_RIGHT), -1.0)

    def test_aligned_gives_no_angle_reward(self):
        """已經對準時，轉動不該有角度獎勵。"""
        self.assertEqual(cal_angle_reward([3, 3], AIM_LEFT), 0.0)
        self.assertEqual(cal_angle_reward([3, 3], AIM_RIGHT), 0.0)


class TestAngleWrapAround(unittest.TestCase):
    """環狀邊界——這是整個獎勵函數最容易寫錯的地方。

    如果判斷寫成普通大小比較而不是模 8，這幾個 case 全部會反過來。
    """

    def test_gun_at_zero_target_at_seven(self):
        """gun=0、target=7：目標其實只差一段，在左側，應往右轉。"""
        self.assertEqual(cal_angle_reward([0, 7], AIM_RIGHT), 1.0)
        self.assertEqual(cal_angle_reward([0, 7], AIM_LEFT), -1.0)

    def test_gun_at_seven_target_at_zero(self):
        """gun=7、target=0：只差一段，在右側，應往左轉。"""
        self.assertEqual(cal_angle_reward([7, 0], AIM_LEFT), 1.0)
        self.assertEqual(cal_angle_reward([7, 0], AIM_RIGHT), -1.0)

    def test_gun_at_one_target_at_six(self):
        """跨越 0/7 邊界的另一組：gun=1、target=6 → (6+3)%8=1，在左側。"""
        self.assertEqual(cal_angle_reward([1, 6], AIM_RIGHT), 1.0)

    def test_gun_at_six_target_at_one(self):
        """對稱情況：gun=6、target=1 → (1+5)%8=6，在右側。"""
        self.assertEqual(cal_angle_reward([6, 1], AIM_LEFT), 1.0)

    def test_every_index_pair_is_classified(self):
        """全部 64 種組合都必須落入某個分類，不能有未定義的狀態。"""
        for gun in range(TOTAL_SEGMENTS):
            for target in range(TOTAL_SEGMENTS):
                r_left = cal_angle_reward([gun, target], AIM_LEFT)
                r_right = cal_angle_reward([gun, target], AIM_RIGHT)
                self.assertIn(r_left, (1.0, -1.0, -0.5, 0.0))
                self.assertIn(r_right, (1.0, -1.0, -0.5, 0.0))

    def test_left_and_right_are_never_both_rewarded(self):
        """同一個狀態下，不能左轉右轉都給正獎勵——那樣 agent 學不到東西。"""
        for gun in range(TOTAL_SEGMENTS):
            for target in range(TOTAL_SEGMENTS):
                r_left = cal_angle_reward([gun, target], AIM_LEFT)
                r_right = cal_angle_reward([gun, target], AIM_RIGHT)
                self.assertFalse(r_left > 0 and r_right > 0,
                                 f"gun={gun}, target={target} 兩邊都給正獎勵")


class TestOppositeDirection(unittest.TestCase):
    """正對面（相差 180 度）：左右轉距離相同，不獎勵也不重罰。"""

    def test_opposite_penalises_both_turns_slightly(self):
        for target in range(TOTAL_SEGMENTS):
            gun = (target + 4) % TOTAL_SEGMENTS
            self.assertEqual(cal_angle_reward([gun, target], AIM_LEFT), -0.5)
            self.assertEqual(cal_angle_reward([gun, target], AIM_RIGHT), -0.5)

    def test_opposite_is_milder_than_wrong_turn(self):
        """轉錯方向 (-1) 應該比正對面時亂轉 (-0.5) 罰得更重。"""
        wrong_turn = cal_angle_reward([1, 0], AIM_LEFT)      # -1.0
        opposite = cal_angle_reward([4, 0], AIM_LEFT)        # -0.5
        self.assertLess(wrong_turn, opposite)


class TestShootReward(unittest.TestCase):
    """開火時機：對準才開、對準就要開。"""

    def test_shoot_when_aligned_is_rewarded(self):
        self.assertEqual(cal_shoot_reward([3, 3], SHOOT), 1.0)

    def test_shoot_when_not_aligned_is_penalised(self):
        self.assertEqual(cal_shoot_reward([3, 5], SHOOT), -1.0)

    def test_not_shooting_when_aligned_is_penalised(self):
        """對準了卻不開火，罰 -0.5——比亂開火輕，但仍是錯過機會。"""
        self.assertEqual(cal_shoot_reward([3, 3], AIM_LEFT), -0.5)
        self.assertEqual(cal_shoot_reward([3, 3], IDLE), -0.5)

    def test_missing_the_shot_is_worse_than_hesitating(self):
        """亂開火 (-1) 比猶豫不開 (-0.5) 罰得更重——子彈有限。"""
        self.assertLess(cal_shoot_reward([3, 5], SHOOT),
                        cal_shoot_reward([3, 3], IDLE))

    def test_no_reward_when_neither_aligned_nor_shooting(self):
        self.assertEqual(cal_shoot_reward([3, 5], AIM_LEFT), 0.0)


class TestTotalReward(unittest.TestCase):
    """角度與開火獎勵疊加後的整體行為。"""

    def test_best_case_is_aligned_and_shooting(self):
        """對準並開火應該是所有狀態中的最高分。"""
        best = total_reward([3, 3], SHOOT)
        for gun in range(TOTAL_SEGMENTS):
            for target in range(TOTAL_SEGMENTS):
                for action in (IDLE, AIM_LEFT, AIM_RIGHT, SHOOT):
                    self.assertLessEqual(total_reward([gun, target], action), best)

    def test_correct_turn_is_positive(self):
        """尚未對準時，轉對方向的總獎勵應為正。"""
        self.assertGreater(total_reward([1, 0], AIM_RIGHT), 0.0)
        self.assertGreater(total_reward([5, 0], AIM_LEFT), 0.0)

    def test_wrong_turn_plus_wild_shot_is_worst(self):
        """轉錯又亂開火，應該是明顯的負分。"""
        self.assertLess(total_reward([1, 0], SHOOT), 0.0)

    def test_reward_is_bounded(self):
        """獎勵必須有界——無界獎勵會讓 PPO 的 advantage 估計爆掉。"""
        for gun in range(TOTAL_SEGMENTS):
            for target in range(TOTAL_SEGMENTS):
                for action in (IDLE, AIM_LEFT, AIM_RIGHT, SHOOT):
                    r = total_reward([gun, target], action)
                    self.assertGreaterEqual(r, -2.0)
                    self.assertLessEqual(r, 2.0)


class TestSegmentation(unittest.TestCase):
    """角度離散化：360 度切成 8 段，每段 45 度。"""

    def test_total_segments_is_integer(self):
        self.assertEqual(360 % DEGREES_PER_SEGMENT, 0)
        self.assertEqual(TOTAL_SEGMENTS, 8)

    def test_opposite_is_half_the_segments(self):
        """180 度必須剛好是總段數的一半，否則正對面的判斷會偏。"""
        self.assertEqual(TOTAL_SEGMENTS % 2, 0)
        self.assertEqual(180 // DEGREES_PER_SEGMENT, TOTAL_SEGMENTS // 2)

    def test_left_and_right_zones_are_symmetric(self):
        """左側 [1,2,3] 與右側 [5,6,7] 段數相同——不對稱會讓 agent 偏好單邊。"""
        left_zone = [1, 2, 3]
        right_zone = [5, 6, 7]
        self.assertEqual(len(left_zone), len(right_zone))
        # 兩區加上對準(0)與正對面(4)，剛好覆蓋全部 8 段
        self.assertEqual(len(left_zone) + len(right_zone) + 2, TOTAL_SEGMENTS)


if __name__ == "__main__":
    unittest.main(verbosity=2)
