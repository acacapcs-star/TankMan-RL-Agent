"""繞路 Agent(校正版:印原始角度資料)。"""
import sys
import os
import math

_REPO_ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
if _REPO_ROOT not in sys.path:
    sys.path.insert(0, _REPO_ROOT)

from tankman.nav import (
    CELL, to_cell, wall_cells, obstacle_cells,
    has_line_of_sight, pick_open_neighbor, bfs_next_step,
    aim_command, turn_command,
)

DEGREES_PER_SEGMENT = 45


def _angle_to_index(angle):
    angle = (angle + 360) % 360
    seg = (angle + DEGREES_PER_SEGMENT / 2) // DEGREES_PER_SEGMENT
    return int(seg % 8)


def _dir_index(fx, fy, tx, ty):
    return _angle_to_index(math.degrees(math.atan2(ty - fy, tx - fx)) + 180)


class MLPlay:
    def __init__(self, ai_name, *args, **kwargs):
        self.side = ai_name
        self.id = None
        self.frame = 0
        print(f"Initial Game {ai_name} ml script (nav agent)")

    def update(self, scene_info: dict, keyboard=[], *args, **kwargs):
        if scene_info["status"] != "GAME_ALIVE":
            return "RESET"
        self.frame += 1
        self.id = scene_info.get("id", self.id)

        mx = scene_info["x"] + CELL / 2
        my = scene_info["y"] + CELL / 2
        enemies = scene_info.get("competitor_info", []) or []
        if not enemies:
            return "NONE"
        enemy = min(enemies, key=lambda e: (e["x"] - scene_info["x"]) ** 2
                                          + (e["y"] - scene_info["y"]) ** 2)
        ex = enemy["x"] + CELL / 2
        ey = enemy["y"] + CELL / 2
        walls = wall_cells(scene_info.get("walls_info", []))
        los = has_line_of_sight(mx, my, ex, ey, walls)

        show = (self.frame % 15 == 0 and self.id == "1P")
        if los:
            gun_idx = _angle_to_index(scene_info.get("gun_angle", 0)
                                      + scene_info.get("angle", 0))
            target_idx = _dir_index(mx, my, ex, ey)
            cmd = aim_command(gun_idx, target_idx)
            if show:
                print(f"[AIM] angle={scene_info.get('angle')} gun_angle={scene_info.get('gun_angle')} "
                      f"me=({scene_info['x']},{scene_info['y']}) en=({enemy['x']},{enemy['y']}) "
                      f"gidx={gun_idx} tidx={target_idx} -> {cmd}")
            return [cmd]

        obstacles = obstacle_cells(scene_info.get("walls_info", []),
                                   scene_info.get("teammate_info", []), self.id)
        my_cell = to_cell(mx, my)
        enemy_cell = to_cell(ex, ey)
        nxt = pick_open_neighbor(my_cell, enemy_cell, obstacles) \
            or bfs_next_step(my_cell, enemy_cell, obstacles)
        if nxt is None:
            return "NONE"
        ncx = nxt[0] * CELL + CELL / 2
        ncy = nxt[1] * CELL + CELL / 2
        body_idx = _angle_to_index(scene_info.get("angle", 0))
        desired_idx = _dir_index(mx, my, ncx, ncy)
        cmd = turn_command(body_idx, desired_idx)
        if show:
            print(f"[MOVE] angle={scene_info.get('angle')} "
                  f"me=({scene_info['x']},{scene_info['y']}) en=({enemy['x']},{enemy['y']}) "
                  f"body={body_idx} want={desired_idx} cell={my_cell}->{nxt} -> {cmd}")
        return [cmd]

    def reset(self):
        print(f"reset Game {self.side}")
