"""純導航邏輯:射線判斷 + BFS 繞路。不依賴 pygame,可單獨測試。

地圖 1000x600、每格 50px → 20 欄 x 12 列。
座標都用 scene_info 的原始 pixel (x, y)(左上為原點,y 向下)。
"""
from collections import deque

CELL = 50
COLS = 1000 // CELL   # 20
ROWS = 600 // CELL     # 12


def to_cell(x, y):
    """pixel 座標 → 格子 (col, row)。"""
    return (int(x) // CELL, int(y) // CELL)


def wall_cells(walls_info):
    """walls_info(list of {x,y}) → 牆佔據的格子集合。"""
    return {to_cell(w["x"], w["y"]) for w in walls_info}


def has_line_of_sight(ax, ay, bx, by, walls):
    """A(ax,ay) 到 B(bx,by) 之間有沒有牆擋。沿線取樣,碰到牆格就 False。"""
    dist = max(abs(bx - ax), abs(by - ay))
    steps = int(dist // (CELL // 4)) + 1   # 取樣密一點,別跳過薄牆
    for i in range(steps + 1):
        t = i / steps
        if to_cell(ax + (bx - ax) * t, ay + (by - ay) * t) in walls:
            return False
    return True


def bfs_next_step(start_cell, goal_cell, walls):
    """從 start 往 goal 的最短路,回傳『下一格』。走不到回 None。"""
    if start_cell == goal_cell:
        return None
    q = deque([start_cell])
    prev = {start_cell: None}
    while q:
        c = q.popleft()
        if c == goal_cell:
            break
        cx, cy = c
        for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
            n = (cx + dx, cy + dy)
            if 0 <= n[0] < COLS and 0 <= n[1] < ROWS and n not in walls and n not in prev:
                prev[n] = c
                q.append(n)
    if goal_cell not in prev:
        return None          # 沒有路
    # 從 goal 回溯到「start 的下一步」
    c = goal_cell
    while prev[c] != start_cell:
        c = prev[c]
    return c


def obstacle_cells(walls_info, teammate_info, my_id):
    """障礙格子 = 牆 ∪ 隊友(但跳過自己)。

    teammate_info 裡包含自己,所以用 my_id 把自己濾掉,
    不然會把自己站的格子標成不能走、反而卡死。
    """
    cells = {to_cell(w["x"], w["y"]) for w in walls_info}
    for t in teammate_info:
        if t.get("id") != my_id:
            cells.add(to_cell(t["x"], t["y"]))
    return cells


def pick_open_neighbor(my_cell, enemy_cell, obstacles):
    """貪心局部繞行:看四個鄰格,挑一個『開著且離敵人最近』的走。

    平手時挑先找到的。走得動回傳那一格,四周全被堵回 None。
    注意:這是局部貪心,遇到凹形牆會卡(局部最低點)——那時要改用 BFS。
    """
    cx, cy = my_cell
    best, best_d = None, None
    for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
        n = (cx + dx, cy + dy)
        if not (0 <= n[0] < COLS and 0 <= n[1] < ROWS):
            continue          # 界外
        if n in obstacles:
            continue          # 牆或隊友
        d = abs(n[0] - enemy_cell[0]) + abs(n[1] - enemy_cell[1])   # 曼哈頓距離
        if best_d is None or d < best_d:
            best, best_d = n, d
    return best


def aim_command(gun_idx, target_idx):
    """砲口方位 gun_idx(0-7)對準 target_idx:回傳遊戲指令字串。

    對準了就開火;否則往縮小夾角的方向轉。規則同 reward_logic。
    """
    if gun_idx == target_idx:
        return "SHOOT"
    if gun_idx in [(target_idx + x) % 8 for x in (5, 6, 7)]:
        return "AIM_LEFT"
    if gun_idx in [(target_idx + x) % 8 for x in (1, 2, 3)]:
        return "AIM_RIGHT"
    return "AIM_LEFT"      # 正對面 180°:先往一邊轉開


def turn_command(body_idx, desired_idx):
    """車身方位 body_idx 轉向 desired_idx:對準就前進,否則轉身。"""
    if body_idx == desired_idx:
        return "FORWARD"
    if body_idx in [(desired_idx + x) % 8 for x in (5, 6, 7)]:
        return "TURN_LEFT"
    if body_idx in [(desired_idx + x) % 8 for x in (1, 2, 3)]:
        return "TURN_RIGHT"
    return "TURN_LEFT"
