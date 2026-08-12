#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

/********************
 * Game Limits      *
 ********************/
#define MAX_H         20
#define MAX_W         20
#define MAX_MONSTERS  32
#define MAX_GOLDS     64
#define MAX_FOODS     64
#define MAX_TOTEMS    4
#define MAX_TURNS    150

/********************
 * Map Cell Symbols *
 ********************/
#define CELL_EMPTY   '.'
#define CELL_WALL    '#'
#define CELL_HERO    'H'
#define CELL_MONSTER 'M'
#define CELL_GOLD    'G'
#define CELL_FOOD    'F'
#define CELL_TOTEM   'T'
#define CELL_EXIT    'E'

/****************
 * Hero Actions *
 ****************/
#define ACT_UP     0
#define ACT_DOWN   1
#define ACT_LEFT   2
#define ACT_RIGHT  3
#define ACT_STAY   4

/****************
 * Game Status  *
 ****************/
#define STATUS_ONGOING  0
#define STATUS_WIN      1
#define STATUS_DEAD     2

/********************
 * Game Parameters *
 ********************/
// Initial values
#define HP_INIT        100
#define HUNGER_INIT    30
#define HUNGER_MAX     30

#define HUNGER_COST    1

// Food
#define FOOD_GAIN      20

// Score
#define GOLD_SCORE     10
#define KILL_SCORE     30

// Damage
#define MONSTER_HIT_DMG 35
#define LASER_DMG       23
#define HUNGER_ZERO_DMG 5

// Endgame scoring
#define DIE_PENALTY   50
#define ESCAPE_BONUS  0

/********************
 * Totem / Laser   *
 ********************/
#define TOTEM_SPAWN_TURN 3

#define INF (1'000'000'007)

#endif
