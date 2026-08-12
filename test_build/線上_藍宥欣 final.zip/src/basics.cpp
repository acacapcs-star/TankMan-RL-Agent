#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <sstream>
#include <vector>
#include "basics.hpp"
#include "constants.hpp"

static void usage(const char* prog) {
    std::cerr
        << "Usage: " << prog << " --input <file> [--quiet] [--judge --action-file <file>]\n"
        << "\n"
        << "Normal run:\n"
        << "  " << prog << " --input testcase/001.in\n"
        << "  (program will ask you to choose Manual / Agent mode after launch)\n"
        << "\n"
        << "Judge run:\n"
        << "  " << prog << " --judge --input testcase/001.in --action-file actions.txt\n"
        << "  --record-actions <file>  Record manual inputs to an action file\n"
        << "  --subtask <1..5>        Judge subtask profile\n";
}

void configureJudgeSubtask(GameState &st, int subtask) {
    st.enable_oob_death = false;
    st.enable_hunger_cost = true;
    st.enable_landing = false;
    st.enable_totems = false;
    st.enable_monsters = false;
    st.enable_hunger_penalty = true;

    switch (subtask) {
        case 1:
            // 1, 3, 7, 8
            break;
        case 2:
            // 1, 2, 3, 4, 7, 8
            st.enable_oob_death = true;
            st.enable_landing = true;
            break;
        case 3:
            // 1, 2, 3, 4, 6, 7, 8
            st.enable_oob_death = true;
            st.enable_landing = true;
            st.enable_monsters = true;
            break;
        case 4:
            // 1, 2, 3, 4, 5, 7, 8
            st.enable_oob_death = true;
            st.enable_landing = true;
            st.enable_totems = true;
            break;
        case 5:
        default:
            // 1 -- 8
            st.enable_oob_death = true;
            st.enable_landing = true;
            st.enable_totems = true;
            st.enable_monsters = true;
            break;
    }
}

static bool parseActionToken(const std::string &tok, int &action) {
    if (tok == "w" || tok == "W") {
        action = ACT_UP; return true;
    }
    if (tok == "s" || tok == "S") {
        action = ACT_DOWN; return true;
    }
    if (tok == "a" || tok == "A") {
        action = ACT_LEFT; return true;
    }
    if (tok == "d" || tok == "D") {
        action = ACT_RIGHT; return true;
    }
    if (tok == ".") {
        action = ACT_STAY; return true;
    }
    return false;
}

RunConfig parseMode(int argc, char** argv) {
    RunConfig config;

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];

        if (arg == "--input") {
            if (i + 1 >= argc) { usage(argv[0]); std::exit(1); }
            config.input_file = argv[++i];
        }
        else if (arg == "--action-file") {
            if (i + 1 >= argc) { usage(argv[0]); std::exit(1); }
            config.action_file = argv[++i];
        }
        else if (arg == "--judge") {
            config.judge = true;
            config.mode = MODE_JUDGE;
        }
        else if (arg == "--help" || arg == "-h") {
            usage(argv[0]);
            std::exit(0);
        }
        else if (arg == "--seed") {
            if (i + 1 >= argc) { usage(argv[0]); std::exit(1); }
            config.seed = std::stoi(argv[++i]);
        }
        else if (arg == "--quiet") {
            config.quiet = true;
        }
        else if (arg == "--record-actions") {
            if (i + 1 >= argc) {
                std::cerr << "--record-actions requires a file path\n";
                std::exit(1);
            }
            config.record_actions_file = argv[++i];
        }
        else if (arg == "--subtask") {
            if (i + 1 >= argc) { usage(argv[0]); std::exit(1); }
            config.judge_subtask = std::stoi(argv[++i]);
            if (config.judge_subtask < 1 || config.judge_subtask > 5) {
                std::cerr << "subtask must be between 1 and 5\n";
                std::exit(1);
            }
        }
        else {
            std::cerr << "Unknown argument: " << arg << "\n";
            usage(argv[0]);
            std::exit(1);
        }
    }

    if (config.input_file.empty()) {
        std::cerr << "Missing required argument: --input <file>\n";
        usage(argv[0]);
        std::exit(1);
    }
    if (config.judge && config.action_file.empty()) {
        std::cerr << "Judge mode requires --action-file <file>\n";
        usage(argv[0]);
        std::exit(1);
    }

    return config;
}

int chooseMode() {
    while (true) {
        std::cout << "Select mode:\n";
        std::cout << "  1) Manual mode\n";
        std::cout << "  2) Agent mode\n";
        std::cout << "Choice: " << std::flush;

        std::string line;
        if (!std::getline(std::cin, line)) return MODE_MANUAL;
        if (line == "1") return MODE_MANUAL;
        if (line == "2") return MODE_AGENT;
        std::cout << "Invalid choice.\n";
    }
}

int readManualAction() {
    std::cout << "Command [w/a/s/d/./q]: " << std::flush;

    std::string line;
    if (!std::getline(std::cin, line)) return -1;
    if (line.empty()) return ACT_STAY;

    char ch = line[0];
    if (ch == 'w') return ACT_UP;
    if (ch == 's') return ACT_DOWN;
    if (ch == 'a') return ACT_LEFT;
    if (ch == 'd') return ACT_RIGHT;
    if (ch == '.') return ACT_STAY;
    if (ch == 'q') return -1;

    return ACT_STAY;
}

bool loadActionsFromFile(const char* filename, std::vector<int> &actions) {
    std::ifstream infile(filename);
    if (!infile) {
        std::cerr << "Error opening action file: " << filename << "\n";
        return false;
    }

    std::string line;
    if (!std::getline(infile, line)) {
        std::cerr << "Empty action file: " << filename << "\n";
        return false;
    }

    std::stringstream ss(line);
    std::string tok;
    while (ss >> tok) {
        int action = ACT_STAY;
        if (!parseActionToken(tok, action)) {
            std::cerr << "Invalid action token in action file: " << tok << "\n";
            return false;
        }
        actions.push_back(action);
    }

    return true;
}

const char* actionName(int action) {
    switch (action) {
        case ACT_UP: return "UP";
        case ACT_DOWN: return "DOWN";
        case ACT_LEFT: return "LEFT";
        case ACT_RIGHT: return "RIGHT";
        case ACT_STAY: return "STAY";
        default: return "UNKNOWN";
    }
}

static bool scanMapForEntities(GameState &st) {
    for (int r = 0; r < st.H; r++) {
        for (int c = 0; c < st.W; c++) {
            if (st.grid[r][c] == CELL_HERO) {
                st.hero = {r, c};
            } else if (st.grid[r][c] == CELL_EXIT) {
                st.exit = {r, c};
            } else if (st.grid[r][c] == CELL_TOTEM) {
                if (st.totem_count < MAX_TOTEMS) {
                    st.totems[st.totem_count++] = {r, c};
                }
            } else if (st.grid[r][c] == CELL_MONSTER || 
                       st.grid[r][c] == CELL_FOOD ||
                       st.grid[r][c] == CELL_GOLD ||
                       st.grid[r][c] == CELL_EMPTY ||
                       st.grid[r][c] == CELL_WALL) {
                // do nothing
            } else {
                return false;
            }
        }
    }
    return true;
}

bool loadFromFile(GameState &st, const char* filename) {
    std::ifstream infile(filename);
    if (!infile) {
        std::cerr << "Error opening file: " << filename << "\n";
        return false;
    }

    if (!(infile >> st.H >> st.W >> st.rng_state)) {
        std::cerr << "Invalid header in input file: " << filename << "\n";
        return false;
    }

    for (int r = 0; r < st.H; r++) {
        std::string line;
        if (!(infile >> line)) {
            std::cerr << "Missing row " << r << " in input file.\n";
            return false;
        }
        if ((int)line.size() < st.W) {
            std::cerr << "Row " << r << " is too short.\n";
            return false;
        }

        for (int c = 0; c < st.W; c++) {
            st.grid[r][c] = line[c];
        }
    }

    return scanMapForEntities(st);
}


void printState(const GameState &st) {
    std::cout << "=============================\n";
    std::cout << "Turn: " << st.turn
              << "  HP: " << st.hp
              << "  Hunger: " << st.hunger
              << "  Score: " << st.score
              << "\n\n";

    for (int r = 0; r < st.H; r++) {
        for (int c = 0; c < st.W; c++) {
            std::cout << st.grid[r][c];
        }
        std::cout << "\n";
    }
    std::cout << "=============================\n";
}

void printJudgeState(const GameState &st, int status) {
    std::cout << "STATE "
              << "turn=" << st.turn << ' '
              << "hero_x=" << st.hero.x << ' '
              << "hero_y=" << st.hero.y << ' '
              << "exit_x=" << st.exit.x << ' '
              << "exit_y=" << st.exit.y << ' '
              << "hp=" << st.hp << ' '
              << "hunger=" << st.hunger << ' '
              << "score=" << st.score << ' '
              << "rng=" << st.rng_state << ' '
              << "totem_count=" << st.totem_count << ' '
              << "status=" << status << ' '
              << "totems=";
    for (int i = 0; i < st.totem_count; i++) {
        if (i) std::cout << ';';
        std::cout << st.totems[i].x << ',' << st.totems[i].y;
    }
    std::cout << "\n";

    for (int r = 0; r < st.H; r++) {
        for (int c = 0; c < st.W; c++) {
            std::cout << st.grid[r][c];
        }
        std::cout << "\n";
    }
}

char actionToToken(int action) {
    switch (action) {
        case ACT_UP: return 'W';
        case ACT_DOWN: return 'S';
        case ACT_LEFT: return 'A';
        case ACT_RIGHT: return 'D';
        case ACT_STAY: return '.';
        default: return '.';
    }
}