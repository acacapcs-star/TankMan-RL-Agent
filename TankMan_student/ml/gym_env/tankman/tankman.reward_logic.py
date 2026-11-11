"""瞄準獎勵的純邏輯:角度獎勵 + 射擊獎勵。

只放「給定砲管/目標方位與動作,該給多少獎勵」的規則,
不碰 pygame、不碰 gym、不碰任何遊戲狀態,所以 env 和測試
都能 import 同一份,不會再各留一份複製品而偷偷不一致。

obs = [砲管方向 index, 目標方向 index],兩者皆為 0–7(360 度切成 8 段)。
動作:0=不動, 1=AIM_LEFT, 2=AIM_RIGHT, 3=SHOOT。
"""

TOTAL_SEGMENTS = 8

AIM_LEFT = 1
AIM_RIGHT = 2
SHOOT = 3


def cal_angle_reward(obs, action):
    gun, target = obs[0], obs[1]

    # 目標在砲管右側(+5、+6、+7 段)→ 該往左轉
    if gun in [(target + x) % TOTAL_SEGMENTS for x in (5, 6, 7)]:
        return 1.0 if action == AIM_LEFT else (-1.0 if action == AIM_RIGHT else 0.0)

    # 目標在砲管左側(+1、+2、+3 段)→ 該往右轉
    if gun in [(target + x) % TOTAL_SEGMENTS for x in (1, 2, 3)]:
        return 1.0 if action == AIM_RIGHT else (-1.0 if action == AIM_LEFT else 0.0)

    # 其餘兩種:已對準(gun==target)、或正對面 180°(gun==target+4)
    # 都沒有「正確轉向」可言 → 中性 0
    return 0.0


def cal_shoot_reward(obs, action):
    gun, target = obs[0], obs[1]
    if gun == target and action == SHOOT:
        return 1.0      # 對準了才開火
    if action == SHOOT:
        return -1.0     # 沒對準就開火
    if gun == target:
        return -0.5     # 對準了卻不開火,錯過機會
    return 0.0