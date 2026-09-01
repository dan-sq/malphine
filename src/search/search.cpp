#include "search/search.h"
#include "board/board.h"
#include "move/move.h"
#include "movegen/movegen.h"
#include "evaluate/evaluate.h"
#include <vector>
#include <chrono>
#include <iostream>
#include <algorithm>
#include <optional>
#include "transposition-table/table.h"

namespace Search {
    TranspositionTable table{};
}

int move_score(const Position& pos, Move move) {
    int result = 0;
    switch(static_cast<MOVE_FLAG>(move.get_flags())) {
        case MOVE_FLAG::KNIGHT_PROMO_CAPTURE: {
            PIECE_T attacker = PIECE_T::PAWN;
            PIECE_T attacked = pos.pieces.get_piece_on(move.get_to());

            result += 50 + Evaluate::PIECE_VALS[static_cast<int>(attacked)] * 10 - Evaluate::PIECE_VALS[static_cast<int>(attacker)];
            break;
        }
        case MOVE_FLAG::BISHOP_PROMO_CAPTURE: {
            PIECE_T attacker = PIECE_T::PAWN;
            PIECE_T attacked = pos.pieces.get_piece_on(move.get_to());

            result += 60 + Evaluate::PIECE_VALS[static_cast<int>(attacked)] * 10 - Evaluate::PIECE_VALS[static_cast<int>(attacker)];
            break;
        }
        case MOVE_FLAG::ROOK_PROMO_CAPTURE: {
            PIECE_T attacker = PIECE_T::PAWN;
            PIECE_T attacked = pos.pieces.get_piece_on(move.get_to());

            result += 70 + Evaluate::PIECE_VALS[static_cast<int>(attacked)] * 10 - Evaluate::PIECE_VALS[static_cast<int>(attacker)];
            break;
        }
        case MOVE_FLAG::QUEEN_PROMO_CAPTURE: {
            PIECE_T attacker = PIECE_T::PAWN;
            PIECE_T attacked = pos.pieces.get_piece_on(move.get_to());

            result += 80 + Evaluate::PIECE_VALS[static_cast<int>(attacked)] * 10 - Evaluate::PIECE_VALS[static_cast<int>(attacker)];
            break;
        }
        case MOVE_FLAG::EP_CAPTURE: {
            PIECE_T attacker = pos.pieces.get_piece_on(move.get_from());
            PIECE_T attacked = PIECE_T::PAWN;

            result += Evaluate::PIECE_VALS[static_cast<int>(attacked)] * 10 - Evaluate::PIECE_VALS[static_cast<int>(attacker)];
            break;
        }
        case MOVE_FLAG::CAPTURE: {
            PIECE_T attacker = pos.pieces.get_piece_on(move.get_from());
            PIECE_T attacked = pos.pieces.get_piece_on(move.get_to());

            result += Evaluate::PIECE_VALS[static_cast<int>(attacked)] * 10 - Evaluate::PIECE_VALS[static_cast<int>(attacker)];
            break;
        }
        case MOVE_FLAG::KNIGHT_PROMO:
            result += 75;
            break;
        case MOVE_FLAG::BISHOP_PROMO:
            result += 80;
            break;
        case MOVE_FLAG::ROOK_PROMO:
            result += 85;
            break;
        case MOVE_FLAG::QUEEN_PROMO:
            result += 90;
            break;
        case MOVE_FLAG::K_CSTL:
            result += 70;
            break;
        case MOVE_FLAG::Q_CSTL:
            result += 70;
            break;
        case MOVE_FLAG::DBL_P_PUSH:
        case MOVE_FLAG::QUIET:
            result += 65;
            break;
    }

    return result;
}

void order_moves(const Position& pos, std::vector<Move>& moves) {
    std::sort(moves.begin(), moves.end(), [&](const Move& a, const Move& b) {
            return move_score(pos, a) > move_score(pos, b);
            });
}

Move Search::search(Position& pos, int time_ms) {
    Move best_move = Move::null();
    Move hash_move = Move::null();
    Context context = {
        .deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(time_ms),
    };

    auto entry = table.get_entry(pos.get_zobrist_hash());
    if (entry.has_value()) {
        hash_move = entry->get_best();
    }
    std::vector<Move> moves;
    Movegen::generate_pseudo_legal_moves<Movegen::GenType::ALL>(pos, pos.get_side(), moves);
    order_moves(pos, moves);
    if (!hash_move.is_null()) {
        auto it = std::find(moves.begin(), moves.end(), hash_move);
            if (it != moves.end())
                std::iter_swap(moves.begin(), it);
    }

    for(int depth = 1; depth <= DEPTH_MAX && !context.stop; depth++) {
        int best_score = -SEARCH_BOUND;
        int alpha = -SEARCH_BOUND;
        int beta = SEARCH_BOUND;
        Move current_best_move = Move::null();

        if (!best_move.is_null()) {
            auto it = std::find(moves.begin(), moves.end(), best_move);
            if (it != moves.end())
                std::iter_swap(moves.begin(), it);
        }

        for (auto& move : moves) {
            if(!Movegen::make(pos, move))
                continue;

            int score = -alphaBeta(pos, -beta, -alpha, depth - 1, 1, context);

            Movegen::unmake(pos, move);

            if(context.stop) {
                break;
            }

            if(score > best_score) {
                best_score = score;
                current_best_move = move;

                if (score > alpha) {
                    alpha = score;
                }
            }
        }

        if (context.stop) {
            return best_move;
        }

        if(!current_best_move.is_null()) {
            best_move = current_best_move;
        }
        std::cout << "info depth " << depth << " score cp " << best_score << "\n";
    }

    return best_move;
}

int Search::quiesce(Position& pos, int alpha, int beta, Context& context){
    int eval = pos.get_side() == PIECE_C::WHITE ? Evaluate::eval(pos) : -Evaluate::eval(pos);

    if(eval >= beta)
        return eval;

    if(eval > alpha)
        alpha = eval;

    std::vector<Move> moves;
    Movegen::generate_pseudo_legal_moves<Movegen::GenType::CAPTURES>(pos, pos.get_side(), moves);
    order_moves(pos, moves);
    for(auto& move : moves) {
        if(!Movegen::make(pos, move))
            continue;

        int score = -quiesce(pos, -beta, -alpha, context);

        Movegen::unmake(pos, move);
        if(context.stop || std::chrono::steady_clock::now() >= context.deadline) {
            context.stop = 1;
            return 0;
        }

        if(score >= beta)
            return score;

        if(score > eval)
            eval = score;

        if(score > alpha)
            alpha = score;
    }

    return eval;
}

int Search::alphaBeta(Position& pos, int alpha, int beta, int depth, int ply_from_root, Context& context) {
    if(depth == 0) {
        return quiesce(pos, alpha, beta, context);
    }

    auto entry = table.get_entry(pos.get_zobrist_hash());
    auto hash_move = Move::null();
    if (entry.has_value()) {
        auto transposition = entry.value();
        hash_move = transposition.get_best();
        int t_score = transposition.get_score();
        if (transposition.get_depth() >= depth) {
            switch(transposition.get_kind()) {
                case NodeKind::EXACT:
                    return t_score;
                case NodeKind::UPPER_BOUND:
                    if (t_score < alpha) {
                        return t_score;
                    }
                    break;
                case NodeKind::LOWER_BOUND:
                    if (t_score >= beta) {
                        return t_score;
                    }
                    break;
            }
        }
    }
    int original_alpha = alpha;
    int best = -SEARCH_BOUND;
    Move best_move = Move::null();
    std::vector<Move> moves;
    Movegen::generate_pseudo_legal_moves<Movegen::GenType::ALL>(pos, pos.get_side(), moves);
    order_moves(pos, moves);
    if (!hash_move.is_null()) {
        auto it = std::find(moves.begin(), moves.end(), hash_move);
            if (it != moves.end())
                std::iter_swap(moves.begin(), it);
    }

    bool no_legal_moves = 1;

    for(auto& move : moves) {
        if(!Movegen::make(pos, move)) {
            continue;
        }

        no_legal_moves = 0;

        int score = -alphaBeta(pos, -beta, -alpha, depth - 1, ply_from_root + 1, context);

        Movegen::unmake(pos, move);

        if(context.stop || std::chrono::steady_clock::now() >= context.deadline) {
            context.stop = 1;
            return 0;
        }

        if(score > best) {
            best = score;
            best_move = move;
            if(score > alpha)
                alpha = score;
        }

        if(score >= beta) {
            TableEntry new_entry{ pos.get_zobrist_hash(), move, depth, best, NodeKind::LOWER_BOUND };
            table.set_entry(new_entry, pos.get_zobrist_hash());
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

    NodeKind kind;
    if (best <= original_alpha) {
        kind = NodeKind::UPPER_BOUND;
    } else {
        kind = NodeKind::EXACT;
    }
    TableEntry new_entry{ pos.get_zobrist_hash(), best_move, depth, best, kind};
    table.set_entry(new_entry, pos.get_zobrist_hash());

    return best;
}

void Search::clear_table() {
    table.clear();
}
