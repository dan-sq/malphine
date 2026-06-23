#include "search/search.h"
#include "board/board.h"
#include "move/move.h"
#include "movegen/movegen.h"
#include "evaluate/evaluate.h"
#include <vector>
#include <chrono>
#include <thread>
#include <atomic>
#include <mutex>

bool is_capture(Move move) {
    auto flag = static_cast<MOVE_FLAG>(move.get_flags());
    return flag == MOVE_FLAG::CAPTURE
        || flag == MOVE_FLAG::EP_CAPTURE
        || flag == MOVE_FLAG::KNIGHT_PROMO_CAPTURE
        || flag == MOVE_FLAG::BISHOP_PROMO_CAPTURE
        || flag == MOVE_FLAG::ROOK_PROMO_CAPTURE
        || flag == MOVE_FLAG::QUEEN_PROMO_CAPTURE;
}

Move Search::search(Position& pos, int time_ms) {
    Move best_move = Move::null();
    Timer timer = {
        .deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(time_ms),
    };

    int stable_move_count = 0;
    std::vector<Move> moves;
    Movegen::generate_pseudo_legal_moves(pos, pos.get_side(), moves);

    const size_t n = moves.size();
    std::mutex mtx;

    for(int depth = 1; depth <= DEPTH_MAX && !timer.stop; depth++) {
        int best_score = -SEARCH_BOUND;
        Move current_best_move = Move::null();
        std::atomic<size_t> move_idx = 0;

        auto searcher = [&]() {
            Position thread_pos = pos;

            while(1) {
                size_t i = move_idx.fetch_add(1);
                if(i >= n)
                    break;

                Move move = moves[i];

                if(!Movegen::make(thread_pos, move))
                    continue;

                int score = -alphaBeta(thread_pos, -SEARCH_BOUND, SEARCH_BOUND, depth - 1, 1, timer);

                Movegen::unmake(thread_pos, move);

                if(timer.stop)
                    break;

                std::lock_guard<std::mutex> lock(mtx);
                if(score > best_score) {
                    best_score = score;
                    current_best_move = move;
                }
            }
        };

        std::vector<std::thread> threads;
        for(int i = 0; i < 4; ++i) {
            threads.emplace_back(searcher);
        }

        for(auto& thread : threads) {
            if(thread.joinable())
                thread.join();
        }

        if(!current_best_move.is_null()) {
            if(best_move == current_best_move)
                stable_move_count++;
            else
                stable_move_count = 0;

            best_move = current_best_move;

            if(stable_move_count >= 2 && depth > 2)
                return best_move;
        }
    }

    return best_move;
}

int Search::quiesce(Position& pos, int alpha, int beta, Timer& timer){
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

            int score = -quiesce(pos, -beta, -alpha, timer);

            Movegen::unmake(pos, move);
            if(timer.stop || std::chrono::steady_clock::now() >= timer.deadline) {
                timer.stop = 1;
                return 0;
            }

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

int Search::alphaBeta(Position& pos, int alpha, int beta, int depth, int ply_from_root, Timer& timer) {
    if(depth == 0) {
        return quiesce(pos, alpha, beta, timer);
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

        int score = -alphaBeta(pos, -beta, -alpha, depth - 1, ply_from_root + 1, timer);

        Movegen::unmake(pos, move);

        if(timer.stop || std::chrono::steady_clock::now() >= timer.deadline) {
            timer.stop = 1;
            return 0;
        }

        if(score > best) {
            best = score;
            if(score > alpha)
                alpha = score;
        }

        if(score >= beta) {
            return best;
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
