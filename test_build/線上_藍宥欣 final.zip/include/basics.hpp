#ifndef BASICS_HPP
#define BASICS_HPP

#include <string>
#include <vector>
#include "state.hpp"

enum RunMode { MODE_AGENT, MODE_MANUAL, MODE_JUDGE };

struct RunConfig {
    RunMode mode = MODE_MANUAL;
    std::string input_file;
    std::string action_file;
    std::string record_actions_file;
    int judge_subtask = 5;
    int seed = 13958;
    bool quiet = false;
    bool judge = false;
};

RunConfig parseMode(int argc, char** argv);
void configureJudgeSubtask(GameState &st, int subtask);
int chooseMode();
int readManualAction();
bool loadFromFile(GameState &st, const char* filename);
bool loadActionsFromFile(const char* filename, std::vector<int> &actions);
const char* actionName(int action);
void printState(const GameState &st);
void printJudgeState(const GameState &st, int status);
char actionToToken(int action);

#endif
