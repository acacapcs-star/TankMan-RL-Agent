#ifndef STATE_HPP
#define STATE_HPP

#include <vector>
#include <string>
#include "constants.hpp"

struct Pos {
    int x;
    int y;
};


struct TotemPair {
    Pos A, B;
    int min_index = 0;
    int max_index = 0;
    int dist = 0;
    bool rowwise = true;
};

struct GameState {
    bool enable_oob_death = true;
    bool enable_hunger_cost = true;
    bool enable_landing = true;
    bool enable_totems = true;
    bool enable_monsters = true;
    bool enable_hunger_penalty = true;
    
    int rng_state;   // random number generator state

    int H, W;        // map size
    int turn;        // current turn

    int hp;
    int hunger;
    int score;

    char grid[MAX_H][MAX_W];
    
    // int monster_count;
    int totem_count;

    Pos hero;
    // Pos monsters[MAX_MONSTERS];
    Pos totems[MAX_TOTEMS + 1];
    Pos exit;
};

#endif
