#pragma once

#include "board/board.h"
#include "move/move.h"

namespace Search {
    int alphaBeta(Position& pos, int alpha, int beta, int depth, int ply_from_root);
    int quiesce(Position& pos, int alpha, int beta);
    Move search(Position& pos, int depth);
    constexpr int MAX_SCORE = 64 * (900 * 2);
    constexpr int MATE_SCORE = MAX_SCORE * 2;
    constexpr int SEARCH_BOUND = MAX_SCORE * 4;
}
