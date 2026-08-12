#include<iostream>
#include<cstdlib>
#include "utils.hpp"

/**
 * nextRand - 產生下一個亂數
 * 
 * 這個函式已經實作完成，請勿修改。
 * 你會在更新圖騰時使用它來隨機選擇圖騰的生成位置。
 */
int nextRand(GameState &st){
    st.rng_state = (st.rng_state * 1103515245 + 12345) & 0x7fffffff;
    return st.rng_state;
}

/**
 * getDistance - 計算兩個位置之間的曼哈頓距離
 *
 * 曼哈頓距離 = |a.x - b.x| + |a.y - b.y|
 * 
 * 提示：可以使用 <cstdlib> 中的 abs()
 */
int getDistance(Pos a, Pos b) {
    return abs(a.x-b.x)+abs(a.y-b.y);//呵呵～一開始不小心把return 寫在後面
}

/**
 * isValidPos - 判斷位置 p 是否在地圖範圍內
 *
 * 地圖大小為 st.H x st.W（row 從 0 到 H-1，column 從 0 到 W-1）
 * 若 p 在地圖範圍內則回傳 true，否則回傳 false
 */
bool isValidPos(const GameState &st, Pos p) {
    bool in_range = (p.x >= 0 && p.x < st.H && p.y >= 0 && p.y < st.W);
    // 判斷剛才算出來的 in_range
    if (in_range) {
        return true;
    } else {
        return false;
    }
} 
