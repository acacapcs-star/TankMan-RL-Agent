#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <cstdlib>
#include <vector>

#include "constants.hpp"
#include "engine.hpp"
#include "agent.hpp"
#include "utils.hpp"
#include "basics.hpp"

int main(int argc, char** argv) {
    RunConfig config = parseMode(argc, argv);

    GameState st;
    initGameState(st);

    if (!loadFromFile(st, config.input_file.c_str())) {
        std::cerr << "Failed to load game state from file.\n";
        return 1;
    }

    configureJudgeSubtask(st, config.judge_subtask);


    std::vector<int> judge_actions;
    size_t judge_index = 0;

    if (config.mode == MODE_JUDGE) {
        if (!loadActionsFromFile(config.action_file.c_str(), judge_actions)) {
            return 1;
        }
    } else {
        config.mode = static_cast<RunMode>(chooseMode());
    std::cout << "Running mode: " << (config.mode == MODE_MANUAL ? "manual" : "agent") << "\n";
    }

    if (config.mode == MODE_JUDGE) {
        printJudgeState(st, STATUS_ONGOING);
    }

    std::ofstream record_out;
    bool first_recorded_action = true;

    if (!config.record_actions_file.empty()) {
        if (config.mode != MODE_MANUAL) {
            std::cerr << "--record-actions can only be used in manual mode\n";
            return 1;
        }

        record_out.open(config.record_actions_file);
        if (!record_out) {
            std::cerr << "Failed to open record file: "
                    << config.record_actions_file << "\n";
            return 1;
        }
    }

    int status = STATUS_ONGOING;

    while (1) {
        if (config.mode != MODE_JUDGE && !config.quiet) {
            printState(st);
        }

        int action = ACT_STAY;
        if (config.mode == MODE_AGENT) {
            action = agent(st);
        } else if (config.mode == MODE_MANUAL) {
            action = readManualAction();

            if (action == -1) {
                std::cout << "Quit.\n";
                break;
            }
            
            if (record_out.is_open()) {
                char token = actionToToken(action);
                if (!first_recorded_action) record_out << ' ';
                record_out << token;
                first_recorded_action = false;
            }
        } else {
            if (judge_index >= judge_actions.size()) {

                break;
            }
            action = judge_actions[judge_index++];
            std::cout << "\nACTION step=" << (st.turn + 1) << ' ' << "action=" << actionName(action) << "\n";
        }

        status = step(st, action);

        if (config.mode == MODE_JUDGE) {
            printJudgeState(st, status);
        }

        if (status != STATUS_ONGOING) {
            break;
        }

        if (st.turn >= MAX_TURNS) {
            break;
        }
    }

    if (status == STATUS_DEAD) {
        if (config.mode != MODE_JUDGE) {
            std::cout << "Hero died.\n";
        }
        st.score -= DIE_PENALTY;
    } else if (status == STATUS_WIN) {
        if (config.mode != MODE_JUDGE) {
            std::cout << "Hero escaped!\n";
        }
        st.score += ESCAPE_BONUS;
    } else {
        if (config.mode != MODE_JUDGE) {
            std::cout << "Time is up!\n";
        }
        st.score -= DIE_PENALTY;
    }

    std::cout << "RESULT "
              << "win=" << (status == STATUS_WIN ? "1" : "0")
              << " "
              << "turns=" << st.turn
              << " "
              << "score=" << st.score
              << " "
              << "hp=" << st.hp
              << " "
              << "timeout=" << (st.turn >= MAX_TURNS ? "1" : "0")
              << "\n";

    return 0;
}
