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
