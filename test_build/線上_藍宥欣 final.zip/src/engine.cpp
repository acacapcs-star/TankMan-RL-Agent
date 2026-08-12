#include "engine.hpp"
#include "utils.hpp"
#include <cmath>
using namespace std;
/*******************************************************************************
 * initGameState - 初始化遊戲狀態
 ******************************************************************************/
void initGameState(GameState &st) {
    st.turn = 0;
    st.hp = HP_INIT;
    st.hunger = HUNGER_INIT;
    st.score = 0;
    st.totem_count = 0;
}
/*******************************************************************************
 * Hero 行動
 ******************************************************************************/
char applyHeroMove(GameState &st, int action) {
    Pos old_pos = st.hero;
    Pos next_pos = old_pos;

    if (action == ACT_UP) {
        next_pos.x--;
    } else if (action == ACT_DOWN) {
        next_pos.x++;
    } else if (action == ACT_LEFT) {
        next_pos.y--;
    } else if (action == ACT_RIGHT) {
        next_pos.y++;
    } else {
        return CELL_HERO;
    }

    if (!isValidPos(st, next_pos)) {
        return CELL_HERO;
    }

    char target_cell = st.grid[next_pos.x][next_pos.y];
    if (target_cell == CELL_WALL || target_cell == CELL_TOTEM) {
        return CELL_HERO;
    }

    st.grid[old_pos.x][old_pos.y] = CELL_EMPTY;
    st.hero = next_pos;

    return target_cell;
}

/*******************************************************************************
 * 怪物行動
 ******************************************************************************/
void updateMonsters(GameState &st) {
    vector<Pos> monster_positions;

    for (int i = 0; i < st.H; i++) {
        for (int j = 0; j < st.W; j++) {
            if (st.grid[i][j] == CELL_MONSTER) {
                monster_positions.push_back({i, j});
            }
        }
    }

    for (int idx = 0; idx < (int)monster_positions.size(); idx++) {
        Pos m_pos = monster_positions[idx];

        if (st.grid[m_pos.x][m_pos.y] != CELL_MONSTER) {
            continue;
        }

        int dx = st.hero.x - m_pos.x;
        int dy = st.hero.y - m_pos.y;
        Pos target = m_pos;

        if (abs(dx) >= abs(dy)) {
            if (dx > 0) target.x++;
            else if (dx < 0) target.x--;
        } else {
            if (dy > 0) target.y++;
            else if (dy < 0) target.y--;
        }

        if (!isValidPos(st, target)) {
            continue;
        }

        if (target.x == st.hero.x && target.y == st.hero.y) {
            st.hp -= MONSTER_HIT_DMG;
        } else if (st.grid[target.x][target.y] == CELL_EMPTY) {
            st.grid[m_pos.x][m_pos.y] = CELL_EMPTY;
            st.grid[target.x][target.y] = CELL_MONSTER;
        }
    }
}

/*******************************************************************************
 * 圖騰 / 雷射系統
 ******************************************************************************/
void clearCell(GameState &st, int r, int c) {
    // 英雄受傷了
    if (r == st.hero.x && c == st.hero.y) {
        st.hp -= LASER_DMG;
    }
    
    // 怪物->金幣
    if (st.grid[r][c] == CELL_MONSTER) {
        st.grid[r][c] = CELL_GOLD;
    }
    // 牆壁+圖騰!=被雷射破壞，其他東西(金幣、食物)被打到會消失變空地
    else if (st.grid[r][c] != CELL_WALL && st.grid[r][c] != CELL_TOTEM) {
        st.grid[r][c] = CELL_EMPTY;
    }
}

void updateTotems(GameState &st) {
    // 圖騰生成
    if (st.turn > 0 && st.turn % TOTEM_SPAWN_TURN == 0) {
        vector<Pos> candid;

        for (int i = 0; i < st.H; ++i) {
            for (int j = 0; j < st.W; ++j) {
                int m_dist = abs(i - st.hero.x) + abs(j - st.hero.y);
                if (st.grid[i][j] == CELL_EMPTY && m_dist > 1) {
                    candid.push_back({i, j});
                }
            }
        }

        if (!candid.empty()) {
            if (st.totem_count >= MAX_TOTEMS) {
                Pos oldest = st.totems[0];
                st.grid[oldest.x][oldest.y] = CELL_EMPTY;
                for (int i = 1; i < st.totem_count; i++) {
                    st.totems[i - 1] = st.totems[i];
                }
                st.totem_count--;
            }

            int idx = nextRand(st) % (int)candid.size();
            Pos p = candid[idx];
            st.grid[p.x][p.y] = CELL_TOTEM;
            st.totems[st.totem_count] = p;
            st.totem_count++;
        }
    }

    if (st.totem_count < 2) {
        return;
    }

    // 共鳴判定
    int best_min_i = -1; // 找到最小跟最大的best值
    int best_max_i = -1;
    int min_m_dis = 1e9;
    bool best_is_row = false;

    for (int i = 0; i < st.totem_count; i++) { // 畢竟因為他要轉換成整數可以避免float受影響
        for (int j = i + 1; j < st.totem_count; j++) {
            Pos p1 = st.totems[i];
            Pos p2 = st.totems[j];

            if (p1.x == p2.x || p1.y == p2.y) {
                int d = abs(p1.x - p2.x) + abs(p1.y - p2.y);
                bool current_is_row = (p1.x == p2.x);
                bool should_update = false;

                if (best_min_i == -1 || d < min_m_dis) {
                    should_update = true;
                } else if (d == min_m_dis) {
                    if (current_is_row && !best_is_row) {
                        should_update = true;
                    } else if (current_is_row == best_is_row) {
                        if (i < best_min_i) {
                            should_update = true;
                        } else if (i == best_min_i && j < best_max_i) {
                            should_update = true;
                        }
                    }
                }

                if (should_update) {
                    best_min_i = i;
                    best_max_i = j;
                    min_m_dis = d;
                    best_is_row = current_is_row;
                }
            }
        }
    }

    // --- (C) 雷射結算 ---
    if (best_min_i != -1) {
        Pos p1 = st.totems[best_min_i];
        Pos p2 = st.totems[best_max_i];

        if (best_is_row) {
            int sy, ey;
            if (p1.y < p2.y) {
                sy = p1.y;
                ey = p2.y;
            } else {
                sy = p2.y;
                ey = p1.y;
            }
            for (int k = sy + 1; k < ey; k++) {
                clearCell(st, p1.x, k);
            }
        } else {
            int sx, ex;
            if (p1.x < p2.x) {
                sx = p1.x;
                ex = p2.x;
            } else {
                sx = p2.x;
                ex = p1.x;
            }
            for (int k = sx + 1; k < ex; k++) {
                clearCell(st, k, p1.y);
            }
        }
        
        st.grid[p1.x][p1.y] = CELL_EMPTY;
        st.grid[p2.x][p2.y] = CELL_EMPTY;

        // 確保先刪索引大的
        for (int i = best_max_i; i < st.totem_count - 1; i++) {
            st.totems[i] = st.totems[i + 1];
        }
        st.totem_count--;

        for (int i = best_min_i; i < st.totem_count - 1; i++) {
            st.totems[i] = st.totems[i + 1];
        }
        st.totem_count--;
    }
}

/*******************************************************************************
 * step - 每回合主流程
 ******************************************************************************/
int step(GameState &st, int action) {
    // --- 1. Hero 行動 ---
    char target = applyHeroMove(st, action);

    // --- 2. 越界死亡判定 ---
    if (!isValidPos(st, st.hero)) {
        st.hp = 0;
        return STATUS_DEAD;
    }

    // --- 3. 移動消耗 Hunger ---
    if (action != ACT_STAY) {
        if (st.hunger - HUNGER_COST < 0) {
            st.hunger = 0;
        } else {
            st.hunger = st.hunger - HUNGER_COST;
        }
    }

    // --- 4. Hero 落點結算 ---
    if (target == CELL_FOOD) {
        if (st.hunger + FOOD_GAIN > HUNGER_MAX) {
            st.hunger = HUNGER_MAX;
        } else {
            st.hunger = st.hunger + FOOD_GAIN;
        }
    } else if (target == CELL_GOLD) {
        st.score += GOLD_SCORE;
    } else if (target == CELL_MONSTER) {
        st.hp -= MONSTER_HIT_DMG;
        st.score += KILL_SCORE;
    } else if (target == CELL_EXIT) {
        // 踩到終點，立刻更新地圖並回傳勝利
        st.grid[st.hero.x][st.hero.y] = CELL_HERO;
        return STATUS_WIN;
    }

    // 紀錄完成->hero放到對的位置
    st.grid[st.hero.x][st.hero.y] = CELL_HERO;

    // 圖騰系統
    updateTotems(st);

    // 怪物行動
    updateMonsters(st);

    // 飢餓懲罰
    if (st.hunger == 0) {
        st.hp -= HUNGER_ZERO_DMG;
    }

    // 最終死亡判定
    if (st.hp <= 0) {
        st.hp = 0; // 避免hp不要變成負的
        return STATUS_DEAD;
    }

    st.turn++;
    return STATUS_ONGOING;
}