#include <iostream>
#include <vector>
#include <cmath>
#include "agent.hpp" 
#include "utils.hpp"
using namespace std; 
int agent(const GameState &st) {
    // 基礎參數設定
    vector<vector<int>> d = {{-1, 1, 0, 0}, {0, 0, -1, 1}};
    vector<int> a = {ACT_UP, ACT_DOWN, ACT_LEFT, ACT_RIGHT};
    int r = st.hero.x, c = st.hero.y, h = st.H, w = st.W;
    vector<Pos> m_list;
    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            if (st.grid[i][j] == CELL_MONSTER) {
                m_list.push_back({i, (int)j});
            }
        }
    }

    // 2. 搜尋最近目標 (金幣、食物、出口)
    int tr = -1, tc = -1;
    long long min_d = 1e15; 
    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            if (st.grid[i][j] == CELL_GOLD || st.grid[i][j] == CELL_FOOD || st.grid[i][j] == CELL_EXIT) {
                long long dis = abs(i - r) + abs(j - c);
                // 食物加權
                if (st.hp < 50 && st.grid[i][j] == CELL_FOOD) dis -= 20;
                if (dis < min_d) {
                    min_d = dis;
                    tr = i;
                    tc = j;
                }
            }
        }
    }

    if (tr == -1) return ACT_STAY;

    // 3. 戰術預判：目標是否被封鎖
    bool door_blocked = false;
    for (auto &m : m_list) {
        if (abs(m.x - tr) + abs(m.y - tc) <= 2) {
            door_blocked = true;
        }
    }

    // 4. 行動評分系統
    int final_act = ACT_STAY;
    long long great_val = -2e18; // 極小值

    for (int k = 0; k < 4; k++) {
        int nr = r + d[0][k];
        int nc = c + d[1][k];

        // 邊界與障礙物檢查
        if (nr >= 0 && nr < h && nc >= 0 && nc < w) {
            char tar = st.grid[nr][nc];
            if (tar != CELL_WALL && tar != CELL_TOTEM && tar != CELL_MONSTER) {
                
                int d_to_t = abs(nr - tr) + abs(nc - tc);
                int curr_d = abs(r - tr) + abs(c - tc);
                
                // 4.1 怪物壓力場計算
                long long m_p = 0;
                int near_m = 0;
                for (auto &m : m_list) {
                    int md = abs(nr - m.x) + abs(nc - m.y);
                    if (md == 0) m_p += 200000000;
                    else if (md == 1) { m_p += 150000000; near_m++; }
                    else if (md == 2) m_p += 80000000; 
                }

                // 4.2 倒退嚕獎勵 (當目標危險時)
                long long t_bonus = 0;
                if (door_blocked) {
                    // 如果往目標的反方向走
                    if (d_to_t > curr_d) t_bonus += 120000000;
                    // 針對性的橫向逃脫
                    if (tc > c && a[k] == ACT_LEFT) t_bonus += 200000000;
                }
                
                // 終點大獎
                if (tar == CELL_EXIT) t_bonus += 500000000;

                // 4.3 總分結算
                long long score = -(long long)d_to_t * 10 - m_p + t_bonus;
                
                // 夾擊修正
                if (near_m >= 2) score -= 600000000; 

                if (score > great_val) {
                    great_val = score;
                    final_act = a[k];
                }
            }
        }
    }
    return final_act;
} 

