#include "movegen.h"
#include "board/board.h"
#include "move/move.h"
#include "tools/magics.h"
#include "movegen/magic_constants.h"
#include <cstdint>
#include <vector>
#include <bit>

void Movegen::init_diagonal_cache() {
    for(int sq = 0; sq < 64; sq++) {
        auto moves_bb = Magics::sq_to_diag_move_bb(sq);
        auto ones_count = std::popcount(moves_bb);
        auto table_size = static_cast<uint64_t>(1) << ones_count;
        auto shift = 64 - ones_count;

        Movegen::diag_cache.masks[sq] = moves_bb;
        Movegen::diag_cache.shifts[sq] = shift;

        uint64_t blocker_bb, attack_bb;
        for(uint64_t idx = 0; idx < table_size; idx++) {
            blocker_bb = Magics::bb_from_idx(idx, moves_bb);
            attack_bb = Magics::generate_diagonal_moves(blocker_bb, sq);
            auto magic_idx = (blocker_bb * Movegen::Constants::DIAGONAL_MAGICS[sq]) >> Movegen::diag_cache.shifts[sq];
            Movegen::diag_cache.moves[sq][magic_idx] = attack_bb;
        }
    }
}

uint8_t distance_from_files(uint8_t to, uint8_t from) {
    return std::abs(to - from);
}

uint8_t distance_from_ranks(uint8_t to, uint8_t from) {
    return std::abs(to - from);
}

bool knight_guard(uint8_t sq, uint8_t to_sq) {
    auto file_diff = distance_from_files(sq % 8, to_sq % 8);
    auto rank_diff = distance_from_ranks(sq / 8, to_sq / 8);
    return (file_diff == 2 && (rank_diff == 1)) || (file_diff == 1 && (rank_diff == 2));
}

bool king_guard(uint8_t sq, uint8_t to_sq) {
      auto file_diff = distance_from_files(sq % 8, to_sq % 8);
      auto rank_diff = distance_from_ranks(sq / 8, to_sq / 8);

      return file_diff <= 1 && rank_diff <= 1 && (file_diff != 0 || rank_diff != 0);
}

bool is_sq_attacked_by_color(Position& pos, uint8_t sq, PIECE_C color) {
    auto sq_bb = static_cast<uint64_t>(1) << sq;
    auto occupancy_bb = pos.pieces.get_both_pieces();
    auto pawns_bb = pos.pieces.get_pieces(color, PIECE_T::PAWN);
    auto knights_bb = pos.pieces.get_pieces(color, PIECE_T::KNIGHT);
    auto bishops_bb = pos.pieces.get_pieces(color, PIECE_T::BISHOP);
    auto rooks_bb = pos.pieces.get_pieces(color, PIECE_T::ROOK);
    auto queens_bb = pos.pieces.get_pieces(color, PIECE_T::QUEEN);
    auto kings_bb = pos.pieces.get_pieces(color, PIECE_T::KING);

    if(color == PIECE_C::WHITE) {
        auto pawn_attacks_bb = (pawns_bb & ~Movegen::FILE_A) << 7  | (pawns_bb & ~Movegen::FILE_H) << 9;
        if(pawn_attacks_bb & sq_bb) return true;
    } else {
        auto pawn_attacks_bb = (pawns_bb & ~Movegen::FILE_H) >> 7  | (pawns_bb & ~Movegen::FILE_A) >> 9;
        if(pawn_attacks_bb & sq_bb) return true;
    }

    while(knights_bb) {
        auto from_sq = std::countr_zero(knights_bb);
        knights_bb &= knights_bb - 1;
        for(auto off : Movegen::KNIGHT_OFFSETS) {
            int to_sq = from_sq + off;
            if(to_sq > 63 || to_sq < 0) continue;
            if(!knight_guard(from_sq, to_sq)) continue;

            if(to_sq == sq) return true;
        }
    }

    while(bishops_bb) {
        int from_sq = std::countr_zero(bishops_bb);
        bishops_bb &= bishops_bb - 1;
        auto blocker_bb = occupancy_bb & Movegen::diag_cache.masks[from_sq];
        auto magic_idx = (blocker_bb * Movegen::Constants::DIAGONAL_MAGICS[from_sq]) >> Movegen::diag_cache.shifts[from_sq];
        auto attack_bb = Movegen::diag_cache.moves[from_sq][magic_idx];
        if(attack_bb & sq_bb) return true;
    }

    while(rooks_bb) {
        int from_sq = std::countr_zero(rooks_bb);
        rooks_bb &= rooks_bb - 1;
        auto blocker_bb = occupancy_bb & Movegen::hori_cache.masks[from_sq];
        auto magic_idx = (blocker_bb * Movegen::Constants::HORIZONTAL_MAGICS[from_sq]) >> Movegen::hori_cache.shifts[from_sq];
        auto attack_bb = Movegen::hori_cache.moves[from_sq][magic_idx];
        if(attack_bb & sq_bb) return true;
    }

    while(queens_bb) {
        int from_sq = std::countr_zero(queens_bb);
        queens_bb &= queens_bb - 1;
        auto diag_blocker_bb = occupancy_bb & Movegen::diag_cache.masks[from_sq];
        auto hori_blocker_bb = occupancy_bb & Movegen::hori_cache.masks[from_sq];
        auto diag_magic_idx = (hori_blocker_bb * Movegen::Constants::DIAGONAL_MAGICS[from_sq]) >> Movegen::diag_cache.shifts[from_sq];
        auto hori_magic_idx = (hori_blocker_bb * Movegen::Constants::HORIZONTAL_MAGICS[from_sq]) >> Movegen::hori_cache.shifts[from_sq];
        auto attack_bb = Movegen::diag_cache.moves[from_sq][diag_magic_idx] | Movegen::hori_cache.moves[from_sq][hori_magic_idx];
        if(attack_bb & sq_bb) return true;
    }

    while(kings_bb) {
        int from_sq = std::countr_zero(kings_bb);
        kings_bb &= kings_bb - 1;

        for(auto off : Movegen::KING_OFFSETS) {
            int to_sq = from_sq + off;
            if(to_sq < 0 || to_sq > 63) continue;
            if(!king_guard(from_sq, to_sq)) continue;
            if(to_sq == sq) return true;
        }
    }

    return false;
}

void Movegen::init_horizontal_cache() {
    for(int sq = 0; sq < 64; sq++) {
        auto moves_bb = Magics::sq_to_hori_move_bb(sq);
        auto ones_count = std::popcount(moves_bb);
        auto table_size = static_cast<uint64_t>(1) << ones_count;
        auto shift = 64 - ones_count;

        Movegen::hori_cache.masks[sq] = moves_bb;
        Movegen::hori_cache.shifts[sq] = shift;

        uint64_t blocker_bb, attack_bb;
        for(uint64_t idx = 0; idx < table_size; idx++) {
            blocker_bb = Magics::bb_from_idx(idx, moves_bb);
            attack_bb = Magics::generate_horizontal_moves(blocker_bb, sq);
            auto magic_idx = (blocker_bb * Movegen::Constants::HORIZONTAL_MAGICS[sq]) >> Movegen::hori_cache.shifts[sq];
            Movegen::hori_cache.moves[sq][magic_idx] = attack_bb;
        }
    }
}

void Movegen::generate_pawn_moves(Position& pos, PIECE_C color, std::vector<Move>& moves) {
    auto bitboard = pos.pieces.get_pieces(color, PIECE_T::PAWN);
    auto empty = ~pos.pieces.get_both_pieces();
    auto enemy = color == PIECE_C::WHITE ? PIECE_C::BLACK: PIECE_C::WHITE;
    auto enemy_bb = pos.pieces.get_colored_pieces(enemy);

    while(bitboard) {
        int sq = std::countr_zero(bitboard);
        bitboard &= bitboard - 1;
        uint64_t from_bb = static_cast<uint64_t>(1) << sq;
        uint64_t en_pas_bb = pos.get_en_pas() < 64 ? static_cast<uint64_t>(1) << pos.get_en_pas() : 0;
        if(color == PIECE_C::WHITE) {
            auto push_bb = from_bb << 8;
            auto left_blocker_mask_bb = (from_bb & ~FILE_A) << 7;
            auto right_blocker_mask_bb = (from_bb & ~FILE_H) << 9;
            if(push_bb & empty) {
                moves.push_back(Move::encode(sq, sq + 8, MOVE_FLAG::QUIET)); 

                if((from_bb & rank2) && ((push_bb << 8) & empty)) {
                    moves.push_back(Move::encode(sq, sq + 16, MOVE_FLAG::DBL_P_PUSH));
                }
            }
            if(left_blocker_mask_bb & enemy_bb) {
                moves.push_back(Move::encode(sq, sq + 7, MOVE_FLAG::CAPTURE));
            }
            if(right_blocker_mask_bb & enemy_bb) {
                moves.push_back(Move::encode(sq, sq + 9, MOVE_FLAG::CAPTURE));
            }
            if(left_blocker_mask_bb & en_pas_bb) {
                moves.push_back(Move::encode(sq, sq + 7, MOVE_FLAG::EP_CAPTURE));
            }
            if(right_blocker_mask_bb & en_pas_bb) {
                moves.push_back(Move::encode(sq, sq + 9, MOVE_FLAG::EP_CAPTURE));
            }
            if(from_bb & rank7 && push_bb & empty) {
                moves.push_back(Move::encode(sq, sq + 8, MOVE_FLAG::KNIGHT_PROMO));
                moves.push_back(Move::encode(sq, sq + 8, MOVE_FLAG::BISHOP_PROMO));
                moves.push_back(Move::encode(sq, sq + 8, MOVE_FLAG::ROOK_PROMO));
                moves.push_back(Move::encode(sq, sq + 8, MOVE_FLAG::QUEEN_PROMO));
            }
            if(from_bb & rank7 && left_blocker_mask_bb & enemy_bb) {
                moves.push_back(Move::encode(sq, sq + 7, MOVE_FLAG::KNIGHT_PROMO_CAPTURE));
                moves.push_back(Move::encode(sq, sq + 7, MOVE_FLAG::BISHOP_PROMO_CAPTURE));
                moves.push_back(Move::encode(sq, sq + 7, MOVE_FLAG::ROOK_PROMO_CAPTURE));
                moves.push_back(Move::encode(sq, sq + 7, MOVE_FLAG::QUEEN_PROMO_CAPTURE));
            }
            if(from_bb & rank7 && right_blocker_mask_bb & enemy_bb) {
                moves.push_back(Move::encode(sq, sq + 9, MOVE_FLAG::KNIGHT_PROMO_CAPTURE));
                moves.push_back(Move::encode(sq, sq + 9, MOVE_FLAG::BISHOP_PROMO_CAPTURE));
                moves.push_back(Move::encode(sq, sq + 9, MOVE_FLAG::ROOK_PROMO_CAPTURE));
                moves.push_back(Move::encode(sq, sq + 9, MOVE_FLAG::QUEEN_PROMO_CAPTURE));
            }
        } else {
            auto push_bb = from_bb >> 8;
            auto left_blocked_mask_bb = (from_bb & ~FILE_H) >> 7;
            auto right_blocked_mask_bb = (from_bb & ~FILE_A) >> 9;
            if(push_bb & empty) {
                moves.push_back(Move::encode(sq, sq - 8, MOVE_FLAG::QUIET)); 

                if((from_bb & rank2) && ((push_bb >> 8) & empty)) {
                    moves.push_back(Move::encode(sq, sq - 16, MOVE_FLAG::DBL_P_PUSH));
                }
            }
            if(left_blocked_mask_bb & enemy_bb) {
                moves.push_back(Move::encode(sq, sq - 7, MOVE_FLAG::CAPTURE));
            }
            if(right_blocked_mask_bb & enemy_bb) {
                moves.push_back(Move::encode(sq, sq - 9, MOVE_FLAG::CAPTURE));
            }
            if(left_blocked_mask_bb & en_pas_bb) {
                moves.push_back(Move::encode(sq, sq - 7, MOVE_FLAG::EP_CAPTURE));
            }
            if(right_blocked_mask_bb & en_pas_bb) {
                moves.push_back(Move::encode(sq, sq - 9, MOVE_FLAG::EP_CAPTURE));
            }
            if(from_bb & rank2 && push_bb & empty) {
                moves.push_back(Move::encode(sq, sq - 8, MOVE_FLAG::KNIGHT_PROMO));
                moves.push_back(Move::encode(sq, sq - 8, MOVE_FLAG::BISHOP_PROMO));
                moves.push_back(Move::encode(sq, sq - 8, MOVE_FLAG::ROOK_PROMO));
                moves.push_back(Move::encode(sq, sq - 8, MOVE_FLAG::QUEEN_PROMO));
            }
            if(from_bb & rank2 && left_blocked_mask_bb & enemy_bb) {
                moves.push_back(Move::encode(sq, sq - 7, MOVE_FLAG::KNIGHT_PROMO_CAPTURE));
                moves.push_back(Move::encode(sq, sq - 7, MOVE_FLAG::BISHOP_PROMO_CAPTURE));
                moves.push_back(Move::encode(sq, sq - 7, MOVE_FLAG::ROOK_PROMO_CAPTURE));
                moves.push_back(Move::encode(sq, sq - 7, MOVE_FLAG::QUEEN_PROMO_CAPTURE));
            }
            if(from_bb & rank2 && right_blocked_mask_bb & enemy_bb) {
                moves.push_back(Move::encode(sq, sq - 9, MOVE_FLAG::KNIGHT_PROMO_CAPTURE));
                moves.push_back(Move::encode(sq, sq - 9, MOVE_FLAG::BISHOP_PROMO_CAPTURE));
                moves.push_back(Move::encode(sq, sq - 9, MOVE_FLAG::ROOK_PROMO_CAPTURE));
                moves.push_back(Move::encode(sq, sq - 9, MOVE_FLAG::QUEEN_PROMO_CAPTURE));
            }
        }
    }
}

void Movegen::generate_knight_moves(Position& pos, PIECE_C color, std::vector<Move>& moves) {
    auto bitboard = pos.pieces.get_pieces(color, PIECE_T::KNIGHT);
    auto empty_bb = ~pos.pieces.get_both_pieces();
    auto enemy = color == PIECE_C::WHITE ? PIECE_C::BLACK: PIECE_C::WHITE;
    auto enemy_bb = pos.pieces.get_colored_pieces(enemy);

    while(bitboard) {
        int sq = std::countr_zero(bitboard);
        bitboard &= bitboard - 1;
        uint64_t from_bb = static_cast<uint64_t>(1) << sq;

        for(auto off : KNIGHT_OFFSETS) {
            int to_sq = sq + off;
            if(to_sq < 0 || to_sq > 63) continue;
            if(!knight_guard(sq, to_sq)) continue;
            uint64_t to_bb = static_cast<uint64_t>(1) << to_sq;
            if(to_bb & empty_bb) {
                moves.push_back(Move::encode(sq, to_sq, MOVE_FLAG::QUIET));
            }
            if(to_bb & enemy_bb) {
                moves.push_back(Move::encode(sq, to_sq, MOVE_FLAG::CAPTURE));
            }
        }
    }
}

void Movegen::generate_bishop_moves(Position& pos, PIECE_C color, std::vector<Move>& moves) {
    auto bitboard = pos.pieces.get_pieces(color, PIECE_T::BISHOP);
    auto empty_bb = ~pos.pieces.get_both_pieces();
    auto enemy = color == PIECE_C::WHITE ? PIECE_C::BLACK : PIECE_C::WHITE;
    auto enemy_bb = pos.pieces.get_colored_pieces(enemy);
    auto occupancy_bb = ~empty_bb;

    while(bitboard) {
        int sq = std::countr_zero(bitboard);
        bitboard &= bitboard - 1;
        uint64_t from_bb = static_cast<uint64_t>(1) << sq;
        auto blocker_bb = occupancy_bb & diag_cache.masks[sq];
        auto magic_idx = (blocker_bb * Movegen::Constants::DIAGONAL_MAGICS[sq]) >> diag_cache.shifts[sq];
        auto attack_bb = diag_cache.moves[sq][magic_idx];

        while(attack_bb) {
            int to_sq = std::countr_zero(attack_bb);
            auto to_bb = static_cast<uint64_t>(1) << to_sq;
            attack_bb &= attack_bb - 1;

            if(to_bb & empty_bb) {
                moves.push_back(Move::encode(sq, to_sq, MOVE_FLAG::QUIET));
            }

            if(to_bb & enemy_bb) {
                moves.push_back(Move::encode(sq, to_sq, MOVE_FLAG::CAPTURE));
            }
        }
    }
}
void Movegen::generate_rook_moves(Position& pos, PIECE_C color, std::vector<Move>& moves){
    auto bitboard = pos.pieces.get_pieces(color, PIECE_T::ROOK);
    auto empty_bb = ~pos.pieces.get_both_pieces();
    auto enemy = color == PIECE_C::WHITE ? PIECE_C::BLACK : PIECE_C::WHITE;
    auto enemy_bb = pos.pieces.get_colored_pieces(enemy);
    auto occupancy_bb = ~empty_bb;

    while(bitboard) {
        int sq = std::countr_zero(bitboard);
        bitboard &= bitboard - 1;
        auto blocker_bb = occupancy_bb & hori_cache.masks[sq];
        auto magic_idx = (blocker_bb * Movegen::Constants::HORIZONTAL_MAGICS[sq]) >> hori_cache.shifts[sq];
        auto attack_bb = hori_cache.moves[sq][magic_idx];

        while(attack_bb) {
            int to_sq = std::countr_zero(attack_bb);
            auto to_bb = static_cast<uint64_t>(1) << to_sq;
            attack_bb &= attack_bb - 1;

            if(to_bb & empty_bb) {
                moves.push_back(Move::encode(sq, to_sq, MOVE_FLAG::QUIET));
            }

            if(to_bb & enemy_bb) {
                moves.push_back(Move::encode(sq, to_sq, MOVE_FLAG::CAPTURE));
            }
        }
    }
}
void Movegen::generate_queen_moves(Position& pos, PIECE_C color, std::vector<Move>& moves){
    auto bitboard = pos.pieces.get_pieces(color, PIECE_T::QUEEN);
    auto empty_bb = ~pos.pieces.get_both_pieces();
    auto enemy = color == PIECE_C::WHITE ? PIECE_C::BLACK : PIECE_C::WHITE;
    auto enemy_bb = pos.pieces.get_colored_pieces(enemy);
    auto occupancy_bb = ~empty_bb;

    while(bitboard) {
        int sq = std::countr_zero(bitboard);
        bitboard &= bitboard - 1;
        uint64_t from_bb = static_cast<uint64_t>(1) << sq;
        auto diag_blocker_bb = occupancy_bb & diag_cache.masks[sq];
        auto hori_blocker_bb = occupancy_bb & hori_cache.masks[sq];
        auto diag_magic_idx = (diag_blocker_bb * Movegen::Constants::DIAGONAL_MAGICS[sq]) >> diag_cache.shifts[sq];
        auto hori_magic_idx = (hori_blocker_bb * Movegen::Constants::HORIZONTAL_MAGICS[sq]) >> hori_cache.shifts[sq];
        auto attack_bb = hori_cache.moves[sq][hori_magic_idx] | diag_cache.moves[sq][diag_magic_idx];

        while(attack_bb) {
            int to_sq = std::countr_zero(attack_bb);
            auto to_bb = static_cast<uint64_t>(1) << to_sq;
            attack_bb &= attack_bb - 1;

            if(to_bb & empty_bb) {
                moves.push_back(Move::encode(sq, to_sq, MOVE_FLAG::QUIET));
            }

            if(to_bb & enemy_bb) {
                moves.push_back(Move::encode(sq, to_sq, MOVE_FLAG::CAPTURE));
            }
        }
    }
}
void Movegen::generate_king_moves(Position& pos, PIECE_C color, std::vector<Move>& moves){
    auto bitboard = pos.pieces.get_pieces(color, PIECE_T::KING);
    auto empty_bb = ~pos.pieces.get_both_pieces();
    auto enemy = color == PIECE_C::WHITE ? PIECE_C::BLACK : PIECE_C::WHITE;
    auto enemy_bb = pos.pieces.get_colored_pieces(enemy);
    auto cstl_flag = pos.get_castle();
    if(cstl_flag & static_cast<uint8_t>(CASTLE_PERM::WKC)) {
        auto cstl_mask_bb = static_cast<uint64_t>(1) << 5 | static_cast<uint64_t>(1) << 6;
        if(!is_sq_attacked_by_color(pos, 4, PIECE_C::BLACK)
                && !is_sq_attacked_by_color(pos, 5, PIECE_C::BLACK)
                && !is_sq_attacked_by_color(pos, 6, PIECE_C::BLACK)
                && (cstl_mask_bb & empty_bb) == cstl_mask_bb) {
            moves.push_back(Move::encode(4, 6, MOVE_FLAG::K_CSTL));
        }
    }
    if(cstl_flag & static_cast<uint8_t>(CASTLE_PERM::WQC)) {
        auto cstl_mask_bb = static_cast<uint64_t>(1) << 3 | static_cast<uint64_t>(1) << 2;
        if(!is_sq_attacked_by_color(pos, 4, PIECE_C::BLACK)
                && !is_sq_attacked_by_color(pos, 3, PIECE_C::BLACK)
                && !is_sq_attacked_by_color(pos, 2, PIECE_C::BLACK)
                && (cstl_mask_bb & empty_bb) == cstl_mask_bb) {
            moves.push_back(Move::encode(4, 2, MOVE_FLAG::Q_CSTL));
        }
    }
    if(cstl_flag & static_cast<uint8_t>(CASTLE_PERM::BKC)) {
        auto cstl_mask_bb = static_cast<uint64_t>(1) << 61 | static_cast<uint64_t>(1) << 62;
        if(!is_sq_attacked_by_color(pos, 60, PIECE_C::WHITE)
                && !is_sq_attacked_by_color(pos, 61, PIECE_C::WHITE)
                && !is_sq_attacked_by_color(pos, 62, PIECE_C::WHITE)
                && (cstl_mask_bb & empty_bb) == cstl_mask_bb) {
            moves.push_back(Move::encode(60, 62, MOVE_FLAG::K_CSTL));
        }
    }
    if(cstl_flag & static_cast<uint8_t>(CASTLE_PERM::BQC)) {
        auto cstl_mask_bb = static_cast<uint64_t>(1) << 59 | static_cast<uint64_t>(1) << 58;
        if(!is_sq_attacked_by_color(pos, 60, PIECE_C::WHITE)
                && !is_sq_attacked_by_color(pos, 59, PIECE_C::WHITE)
                && !is_sq_attacked_by_color(pos, 58, PIECE_C::WHITE)
                && (cstl_mask_bb & empty_bb) == cstl_mask_bb) {
            moves.push_back(Move::encode(60, 58, MOVE_FLAG::Q_CSTL));
        }
    }

    while(bitboard) {
        int sq = std::countr_zero(bitboard);
        bitboard &= bitboard - 1;

        for(auto off : KING_OFFSETS) {
            int to_sq = sq + off;
            if(to_sq < 0 || to_sq > 63) continue;
            if(!king_guard(sq, to_sq)) continue;
            if(is_sq_attacked_by_color(pos, to_sq, enemy)) continue;
            auto to_bb = static_cast<uint64_t>(1) << to_sq;

            if(to_bb & empty_bb) {
                moves.push_back(Move::encode(sq, to_sq, MOVE_FLAG::QUIET));
            }

            if(to_bb & enemy_bb) {
                moves.push_back(Move::encode(sq, to_sq, MOVE_FLAG::CAPTURE));
            }

        }
    }
}

void Movegen::generate_all_moves(Position& pos, PIECE_C color, std::vector<Move>& moves) {
    generate_pawn_moves(pos, color, moves);
    generate_knight_moves(pos, color, moves);
    generate_bishop_moves(pos, color, moves);
    generate_rook_moves(pos, color, moves);
    generate_queen_moves(pos, color, moves);
    generate_king_moves(pos, color, moves);
}
