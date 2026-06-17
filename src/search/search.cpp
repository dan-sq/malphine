#include "search/search.h"
#include "board/board.h"
#include "move/move.h"
#include "movegen/movegen.h"
#include "evaluate/evaluate.h"
#include <vector>

bool is_capture(Move move) {
    auto flag = static_cast<MOVE_FLAG>(move.get_flags());
    return flag == MOVE_FLAG::CAPTURE
        || flag == MOVE_FLAG::EP_CAPTURE
        || flag == MOVE_FLAG::KNIGHT_PROMO_CAPTURE
        || flag == MOVE_FLAG::BISHOP_PROMO_CAPTURE
        || flag == MOVE_FLAG::ROOK_PROMO_CAPTURE
        || flag == MOVE_FLAG::QUEEN_PROMO_CAPTURE;
}

Move Search::search(Position& pos, int depth) {
    Move best_move = Move::null();

    std::vector<Move> moves;
    Movegen::generate_pseudo_legal_moves(pos, pos.get_side(), moves);

    int best_score = -SEARCH_BOUND;

    for(auto& move : moves) {
        if(!Movegen::make(pos, move))
            continue;

        int score = -alphaBeta(pos, -SEARCH_BOUND, SEARCH_BOUND* 3, depth - 1, 1);

        Movegen::unmake(pos, move);

        if(score > best_score) {
            best_score = score;
            best_move = move;
        }
    }

    return best_move;
}

int Search::quiesce(Position& pos, int alpha, int beta){
    int eval = pos.get_side() == PIECE_C::WHITE ? Evaluate::eval(pos) : -Evaluate::eval(pos);

    if(eval >= beta)
        return eval;

    if(eval > alpha)
        alpha = eval;

    std::vector<Move> moves;
    Movegen::generate_pseudo_legal_moves(pos, pos.get_side(), moves);
    for(auto& move : moves) {
        if(is_capture(move)) {
            if(!Movegen::make(pos, move))
                continue;

            int score = -quiesce(pos, -beta, -alpha);

            Movegen::unmake(pos, move);

            if(score >= beta)
                return score;

            if(score > eval)
                eval = score;

            if(score > alpha)
                alpha = score;
        }
    }

    return eval;
}

int Search::alphaBeta(Position& pos, int alpha, int beta, int depth, int ply_from_root) {
    if(depth == 0) {
        return quiesce(pos, alpha, beta);
    }

    int best = -(SEARCH_BOUND);
    std::vector<Move> moves;
    Movegen::generate_pseudo_legal_moves(pos, pos.get_side(), moves);
    bool no_legal_moves = 1;

    for(auto& move : moves) {
        if(!Movegen::make(pos, move)) {
            continue;
        }

        no_legal_moves = 0;

        int score = -alphaBeta(pos, -beta, -alpha, depth - 1, ply_from_root + 1);

        Movegen::unmake(pos, move);

        if(score > best) {
            best = score;
            if(score > alpha)
                alpha = score;
        }

        if(score >= beta) {
            return score;
        }
    }

    if(no_legal_moves) {
        auto king_bb = pos.pieces.get_pieces(pos.get_side(), PIECE_T::KING);
        uint8_t king_sq = std::countr_zero(king_bb);
        PIECE_C enemy = pos.get_side() == PIECE_C::WHITE ? PIECE_C::BLACK : PIECE_C::WHITE;
        if(Movegen::is_sq_attacked_by_color(pos, king_sq, enemy)){
            return -MATE_SCORE + ply_from_root;
        } else {
            return 0;
        }
    }

    return best;
}
