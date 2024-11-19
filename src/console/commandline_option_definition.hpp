/*
    Egaroucid Project

    @file commandline_option_definition.hpp
        Definition of commandline options
    @date 2021-2024
    @author Takuto Yamana
    @license GPL-3.0 license
*/

#pragma once
#include <string>
#include <vector>

#define N_COMMANDLINE_OPTIONS 23

#define ID_NONE -1
#define ID_VERSION 0
#define ID_HELP 1
#define ID_LEVEL 2
#define ID_THREAD 3
#define ID_LOG 4
#if USE_CHANGEABLE_HASH_LEVEL
    #define ID_HASH 5
#endif
#define ID_LEVEL_INFO 6
#define ID_BOOK_FILE 7
#define ID_EVAL_FILE 8
#define ID_NOBOOK 9
#define ID_SOLVE 10
#define ID_MODE 11
#define ID_GTP 12
#define ID_QUIET 13
#define ID_SELF_PLAY 14
#define ID_SELF_PLAY_LINE 15
#define ID_PERFT 16
#define ID_TIME_ALLOCATE 17
#define ID_PONDER 18
#define ID_DISABLE_AUTO_CACHE_CLEAR 19
#define ID_NOBOARD 20
#define ID_LOG 21
#define ID_NOAUTOPASS 22

struct Commandline_option_info{
    int id;
    std::vector<std::string> names;
    int n_args;
    std::string arg;
    std::string description;
};

const Commandline_option_info commandline_option_data[N_COMMANDLINE_OPTIONS] = {
    {ID_VERSION,            {"-v", "-version"},                                 0, "",                 "Check Egaroucid for Console version"}, 
    {ID_HELP,               {"-h", "-help", "-?"},                              0, "",                 "See help"}, 
    {ID_LEVEL,              {"-l", "-level"},                                   1, "<level>",          "Set level to <level> (0 to 60)"}, 
    {ID_THREAD,             {"-t", "-thread", "-threads"},                      1, "<n_threads>",      "Set number of threads (more than 0)"},
    {ID_LOG,                {"-noise"},                                         0, "",                 "Show all logs"},
#if USE_CHANGEABLE_HASH_LEVEL
    {ID_HASH,               {"-hash", "-hashlevel"},                            1, "<hash_level>",     "Set hash level to <hash_level> (0 to 29)"},
#else
    {ID_NONE},
#endif
    {ID_LEVEL_INFO,         {"-linfo", "-levelinfo"},                           0, "",                 "See level information"},
    {ID_BOOK_FILE,          {"-b", "-book"},                                    1, "<book_file>",      "Import <book_file> as Egaroucid's book"},
    {ID_EVAL_FILE,          {"-eval", "-evaluation"},                           1, "<eval_file>",      "Import <eval_file> as Egaroucid's evaluation function"},
    {ID_NOBOOK,             {"-nobook"},                                        0, "",                 "Run Egaroucid without book"},
    {ID_SOLVE,              {"-s", "-solve", "-sol"},                           1, "<problem file>",   "Solve problems written in <problem file>"},
    {ID_MODE,               {"-m", "-mode"},                                    1, "<mode>",           "Set mode to <mode> (0: You vs Egaroucid, 1: Egaroucid vs You, 2: Egaroucid vs Egaroucid, 3: You vs You)"},
    {ID_GTP,                {"-gtp"},                                           0, "",                 "Use GTP (Go Text Protocol) mode"},
    {ID_QUIET,              {"-q", "-quiet", "-silent"},                        0, "",                 "Quiet mode"},
    {ID_SELF_PLAY,          {"-sf", "-selfplay"},                               2, "<n> <m>",          "Self play <n> games (play randomly first <m> moves)"},
    {ID_SELF_PLAY_LINE,     {"-sfl", "-selfplayline"},                          1, "<file>",           "Self play with given openings"},
    {ID_PERFT,              {"-perft"},                                         2, "<depth> <mode>",   "Perft for Othello with <depth> in <mode>, 1: pass is counted as 1 move (normal perft), 2: pass is not counted as a move"},
    {ID_TIME_ALLOCATE,      {"-time"},                                          1, "<seconds>",        "Time allocate <seconds> seconds. -level will be ignored"},
    {ID_PONDER,             {"-ponder"},                                        0, "",                  "Enable ponder"},
    {ID_DISABLE_AUTO_CACHE_CLEAR, {"-noautocacheclear"},                        0, "",                  "Disable auto cache clearing"},
    {ID_NOBOARD,            {"-noboard"},                                       0, "",                  "Hide Board"},
    {ID_LOG,                {"-logfile"},                                           1, "<file>",            "Save search log to <file>"},
    {ID_NOAUTOPASS,         {"-noautopass"},                                    0, "",                  "No auto-pass"}
};
