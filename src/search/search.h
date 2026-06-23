#pragma once

#include "board/board.h"
#include "move/move.h"
#include <chrono>
#include <atomic>

namespace Search {
    struct Timer {
        std::chrono::time_point<std::chrono::steady_clock> deadline;
        std::atomic<bool> stop{false};
    };
    int alphaBeta(Position& pos, int alpha, int beta, int depth, int ply_from_root, Timer& timer);
    int quiesce(Position& pos, int alpha, int beta, Timer& timer);
    Move search(Position& pos, int time_ms);
    constexpr int MAX_SCORE = 64 * (900 * 2);
    constexpr int MATE_SCORE = MAX_SCORE * 2;
    constexpr int SEARCH_BOUND = MAX_SCORE * 4;
    constexpr int DEPTH_MAX = 30;
}
