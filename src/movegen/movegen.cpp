#include "movegen.h"
#include "board/board.h"
#include "move/move.h"
#include "tools/magics.h"
#include "movegen/magic_constants.h"
#include "transposition-table/zobrist.h"
#include <cstdint>
#include <vector>

namespace Movegen {
    DiagonalCache diag_cache{};
    HorizontalCache hori_cache{};
}

void Movegen::init() {
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

bool Movegen::is_sq_attacked_by_color(Position& pos, uint8_t sq, PIECE_C color) {
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
        auto diag_magic_idx = (diag_blocker_bb * Movegen::Constants::DIAGONAL_MAGICS[from_sq]) >> Movegen::diag_cache.shifts[from_sq];
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

template<Movegen::GenType type>
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

            if(from_bb & rank7) {
                if constexpr (type == Movegen::GenType::ALL) {
                    if(push_bb & empty) {
                        moves.push_back(Move::encode(sq, sq + 8, MOVE_FLAG::KNIGHT_PROMO));
                        moves.push_back(Move::encode(sq, sq + 8, MOVE_FLAG::BISHOP_PROMO));
                        moves.push_back(Move::encode(sq, sq + 8, MOVE_FLAG::ROOK_PROMO));
                        moves.push_back(Move::encode(sq, sq + 8, MOVE_FLAG::QUEEN_PROMO));
                    }
                }
                if(left_blocker_mask_bb & enemy_bb) {
                    moves.push_back(Move::encode(sq, sq + 7, MOVE_FLAG::KNIGHT_PROMO_CAPTURE));
                    moves.push_back(Move::encode(sq, sq + 7, MOVE_FLAG::BISHOP_PROMO_CAPTURE));
                    moves.push_back(Move::encode(sq, sq + 7, MOVE_FLAG::ROOK_PROMO_CAPTURE));
                    moves.push_back(Move::encode(sq, sq + 7, MOVE_FLAG::QUEEN_PROMO_CAPTURE));
                }
                if(right_blocker_mask_bb & enemy_bb) {
                    moves.push_back(Move::encode(sq, sq + 9, MOVE_FLAG::KNIGHT_PROMO_CAPTURE));
                    moves.push_back(Move::encode(sq, sq + 9, MOVE_FLAG::BISHOP_PROMO_CAPTURE));
                    moves.push_back(Move::encode(sq, sq + 9, MOVE_FLAG::ROOK_PROMO_CAPTURE));
                    moves.push_back(Move::encode(sq, sq + 9, MOVE_FLAG::QUEEN_PROMO_CAPTURE));
                }
            } else {
                if constexpr (type == Movegen::GenType::ALL) {
                    if(push_bb & empty) {
                        moves.push_back(Move::encode(sq, sq + 8, MOVE_FLAG::QUIET));
                        if((from_bb & rank2) && ((push_bb << 8) & empty)) {
                            moves.push_back(Move::encode(sq, sq + 16, MOVE_FLAG::DBL_P_PUSH));
                        }
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
            }
        } else {
            auto push_bb = from_bb >> 8;
            auto left_blocked_mask_bb = (from_bb & ~FILE_H) >> 7;
            auto right_blocked_mask_bb = (from_bb & ~FILE_A) >> 9;

            if(from_bb & rank2) {
                if constexpr (type == Movegen::GenType::ALL) {
                    if(push_bb & empty) {
                        moves.push_back(Move::encode(sq, sq - 8, MOVE_FLAG::KNIGHT_PROMO));
                        moves.push_back(Move::encode(sq, sq - 8, MOVE_FLAG::BISHOP_PROMO));
                        moves.push_back(Move::encode(sq, sq - 8, MOVE_FLAG::ROOK_PROMO));
                        moves.push_back(Move::encode(sq, sq - 8, MOVE_FLAG::QUEEN_PROMO));
                    }
                }
                if(left_blocked_mask_bb & enemy_bb) {
                    moves.push_back(Move::encode(sq, sq - 7, MOVE_FLAG::KNIGHT_PROMO_CAPTURE));
                    moves.push_back(Move::encode(sq, sq - 7, MOVE_FLAG::BISHOP_PROMO_CAPTURE));
                    moves.push_back(Move::encode(sq, sq - 7, MOVE_FLAG::ROOK_PROMO_CAPTURE));
                    moves.push_back(Move::encode(sq, sq - 7, MOVE_FLAG::QUEEN_PROMO_CAPTURE));
                }
                if(right_blocked_mask_bb & enemy_bb) {
                    moves.push_back(Move::encode(sq, sq - 9, MOVE_FLAG::KNIGHT_PROMO_CAPTURE));
                    moves.push_back(Move::encode(sq, sq - 9, MOVE_FLAG::BISHOP_PROMO_CAPTURE));
                    moves.push_back(Move::encode(sq, sq - 9, MOVE_FLAG::ROOK_PROMO_CAPTURE));
                    moves.push_back(Move::encode(sq, sq - 9, MOVE_FLAG::QUEEN_PROMO_CAPTURE));
                }
            } else {
                if constexpr (type == Movegen::GenType::ALL) {
                    if(push_bb & empty) {
                        moves.push_back(Move::encode(sq, sq - 8, MOVE_FLAG::QUIET)); 

                        if((from_bb & rank7) && ((push_bb >> 8) & empty)) {
                            moves.push_back(Move::encode(sq, sq - 16, MOVE_FLAG::DBL_P_PUSH));
                        }
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
            }
        }
    }
}

template<Movegen::GenType type>
void Movegen::generate_knight_moves(Position& pos, PIECE_C color, std::vector<Move>& moves) {
    auto bitboard = pos.pieces.get_pieces(color, PIECE_T::KNIGHT);
    auto empty_bb = ~pos.pieces.get_both_pieces();
    auto enemy = color == PIECE_C::WHITE ? PIECE_C::BLACK: PIECE_C::WHITE;
    auto enemy_bb = pos.pieces.get_colored_pieces(enemy);

    while(bitboard) {
        int sq = std::countr_zero(bitboard);
        bitboard &= bitboard - 1;

        for(auto off : KNIGHT_OFFSETS) {
            int to_sq = sq + off;
            if(to_sq < 0 || to_sq > 63) continue;
            if(!knight_guard(sq, to_sq)) continue;
            uint64_t to_bb = static_cast<uint64_t>(1) << to_sq;

            if constexpr (type == Movegen::GenType::ALL) {
                if(to_bb & empty_bb) {
                    moves.push_back(Move::encode(sq, to_sq, MOVE_FLAG::QUIET));
                }
            }
            if(to_bb & enemy_bb) {
                moves.push_back(Move::encode(sq, to_sq, MOVE_FLAG::CAPTURE));
            }
        }
    }
}

template<Movegen::GenType type>
void Movegen::generate_bishop_moves(Position& pos, PIECE_C color, std::vector<Move>& moves) {
    auto bitboard = pos.pieces.get_pieces(color, PIECE_T::BISHOP);
    auto empty_bb = ~pos.pieces.get_both_pieces();
    auto enemy = color == PIECE_C::WHITE ? PIECE_C::BLACK : PIECE_C::WHITE;
    auto enemy_bb = pos.pieces.get_colored_pieces(enemy);
    auto occupancy_bb = ~empty_bb;

    while(bitboard) {
        int sq = std::countr_zero(bitboard);
        bitboard &= bitboard - 1;
        auto blocker_bb = occupancy_bb & diag_cache.masks[sq];
        auto magic_idx = (blocker_bb * Movegen::Constants::DIAGONAL_MAGICS[sq]) >> diag_cache.shifts[sq];
        auto attack_bb = diag_cache.moves[sq][magic_idx];

        while(attack_bb) {
            int to_sq = std::countr_zero(attack_bb);
            auto to_bb = static_cast<uint64_t>(1) << to_sq;
            attack_bb &= attack_bb - 1;

            if constexpr (type == Movegen::GenType::ALL) {
                if(to_bb & empty_bb) {
                    moves.push_back(Move::encode(sq, to_sq, MOVE_FLAG::QUIET));
                }
            }

            if(to_bb & enemy_bb) {
                moves.push_back(Move::encode(sq, to_sq, MOVE_FLAG::CAPTURE));
            }
        }
    }
}

template<Movegen::GenType type>
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

            if constexpr (type == Movegen::GenType::ALL) {
                if(to_bb & empty_bb) {
                    moves.push_back(Move::encode(sq, to_sq, MOVE_FLAG::QUIET));
                }
            }

            if(to_bb & enemy_bb) {
                moves.push_back(Move::encode(sq, to_sq, MOVE_FLAG::CAPTURE));
            }
        }
    }
}

template<Movegen::GenType type>
void Movegen::generate_queen_moves(Position& pos, PIECE_C color, std::vector<Move>& moves){
    auto bitboard = pos.pieces.get_pieces(color, PIECE_T::QUEEN);
    auto empty_bb = ~pos.pieces.get_both_pieces();
    auto enemy = color == PIECE_C::WHITE ? PIECE_C::BLACK : PIECE_C::WHITE;
    auto enemy_bb = pos.pieces.get_colored_pieces(enemy);
    auto occupancy_bb = ~empty_bb;

    while(bitboard) {
        int sq = std::countr_zero(bitboard);
        bitboard &= bitboard - 1;
        auto diag_blocker_bb = occupancy_bb & diag_cache.masks[sq];
        auto hori_blocker_bb = occupancy_bb & hori_cache.masks[sq];
        auto diag_magic_idx = (diag_blocker_bb * Movegen::Constants::DIAGONAL_MAGICS[sq]) >> diag_cache.shifts[sq];
        auto hori_magic_idx = (hori_blocker_bb * Movegen::Constants::HORIZONTAL_MAGICS[sq]) >> hori_cache.shifts[sq];
        auto attack_bb = hori_cache.moves[sq][hori_magic_idx] | diag_cache.moves[sq][diag_magic_idx];

        while(attack_bb) {
            int to_sq = std::countr_zero(attack_bb);
            auto to_bb = static_cast<uint64_t>(1) << to_sq;
            attack_bb &= attack_bb - 1;

            if constexpr (type == Movegen::GenType::ALL) {
                if(to_bb & empty_bb) {
                    moves.push_back(Move::encode(sq, to_sq, MOVE_FLAG::QUIET));
                }
            }

            if(to_bb & enemy_bb) {
                moves.push_back(Move::encode(sq, to_sq, MOVE_FLAG::CAPTURE));
            }
        }
    }
}

template<Movegen::GenType type>
void Movegen::generate_king_moves(Position& pos, PIECE_C color, std::vector<Move>& moves){
    auto bitboard = pos.pieces.get_pieces(color, PIECE_T::KING);
    auto empty_bb = ~pos.pieces.get_both_pieces();
    auto enemy = color == PIECE_C::WHITE ? PIECE_C::BLACK : PIECE_C::WHITE;
    auto enemy_bb = pos.pieces.get_colored_pieces(enemy);
    auto cstl_flag = pos.get_castle();
    auto rooks_bb = pos.pieces.get_pieces(color, PIECE_T::ROOK);
    if constexpr (type == Movegen::GenType::ALL) {
        if(color == PIECE_C::WHITE) {
            if(cstl_flag & static_cast<uint8_t>(CASTLE_PERM::WKC)) {
                auto cstl_mask_bb = static_cast<uint64_t>(1) << 5 | static_cast<uint64_t>(1) << 6;
                if(!is_sq_attacked_by_color(pos, 4, PIECE_C::BLACK)
                        && !is_sq_attacked_by_color(pos, 5, PIECE_C::BLACK)
                        && !is_sq_attacked_by_color(pos, 6, PIECE_C::BLACK)
                        && (cstl_mask_bb & empty_bb) == cstl_mask_bb
                        && (bitboard & static_cast<uint64_t>(1) << 4)
                        && (rooks_bb & static_cast<uint64_t>(1) << 7)) {
                    moves.push_back(Move::encode(4, 6, MOVE_FLAG::K_CSTL));
                }
            }
            if(cstl_flag & static_cast<uint8_t>(CASTLE_PERM::WQC)) {
                auto cstl_mask_bb = static_cast<uint64_t>(1) << 3 | static_cast<uint64_t>(1) << 2 | static_cast<uint64_t>(1) << 1;
                    if(!is_sq_attacked_by_color(pos, 4, PIECE_C::BLACK)
                        && !is_sq_attacked_by_color(pos, 3, PIECE_C::BLACK)
                        && !is_sq_attacked_by_color(pos, 2, PIECE_C::BLACK)
                        && (cstl_mask_bb & empty_bb) == cstl_mask_bb
                        && (bitboard & static_cast<uint64_t>(1) << 4)
                        && (rooks_bb & 1)) {
                    moves.push_back(Move::encode(4, 2, MOVE_FLAG::Q_CSTL));
                }
            }
        } else {
            if(cstl_flag & static_cast<uint8_t>(CASTLE_PERM::BKC)) {
                auto cstl_mask_bb = static_cast<uint64_t>(1) << 61 | static_cast<uint64_t>(1) << 62;
                if(!is_sq_attacked_by_color(pos, 60, PIECE_C::WHITE)
                        && !is_sq_attacked_by_color(pos, 61, PIECE_C::WHITE)
                        && !is_sq_attacked_by_color(pos, 62, PIECE_C::WHITE)
                        && (cstl_mask_bb & empty_bb) == cstl_mask_bb
                        && (bitboard & static_cast<uint64_t>(1) << 60)
                        && (rooks_bb & static_cast<uint64_t>(1) << 63)) {
                            moves.push_back(Move::encode(60, 62, MOVE_FLAG::K_CSTL));
                }
            }
            if(cstl_flag & static_cast<uint8_t>(CASTLE_PERM::BQC)) {
                auto cstl_mask_bb = static_cast<uint64_t>(1) << 57 | static_cast<uint64_t>(1) << 58 | static_cast<uint64_t>(1) << 59;
                if(!is_sq_attacked_by_color(pos, 60, PIECE_C::WHITE)
                        && !is_sq_attacked_by_color(pos, 59, PIECE_C::WHITE)
                        && !is_sq_attacked_by_color(pos, 58, PIECE_C::WHITE)
                        && (cstl_mask_bb & empty_bb) == cstl_mask_bb
                        && (bitboard & static_cast<uint64_t>(1) << 60)
                        && (rooks_bb & static_cast<uint64_t>(1) << 56)) {
                            moves.push_back(Move::encode(60, 58, MOVE_FLAG::Q_CSTL));
                }
            }
        }
    }

    while(bitboard) {
        int sq = std::countr_zero(bitboard);
        bitboard &= bitboard - 1;

        for(auto off : KING_OFFSETS) {
            int to_sq = sq + off;
            if(to_sq < 0 || to_sq > 63) continue;
            if(!king_guard(sq, to_sq)) continue;
            auto to_bb = static_cast<uint64_t>(1) << to_sq;
            
            if constexpr (type == Movegen::GenType::ALL) {
                if(to_bb & empty_bb) {
                    moves.push_back(Move::encode(sq, to_sq, MOVE_FLAG::QUIET));
                }
            }

            if(to_bb & enemy_bb) {
                moves.push_back(Move::encode(sq, to_sq, MOVE_FLAG::CAPTURE));
            }

        }
    }
}

template<Movegen::GenType type>
void Movegen::generate_pseudo_legal_moves(Position& pos, PIECE_C color, std::vector<Move>& moves) {
    generate_pawn_moves<type>(pos, color, moves);
    generate_knight_moves<type>(pos, color, moves);
    generate_bishop_moves<type>(pos, color, moves);
    generate_rook_moves<type>(pos, color, moves);
    generate_queen_moves<type>(pos, color, moves);
    generate_king_moves<type>(pos, color, moves);
}

uint64_t Movegen::perft(Position& pos, uint64_t depth) {
    uint64_t nodes = 0;
    if(depth == 0) return static_cast<uint64_t>(1);

    std::vector<Move> moves;
    generate_pseudo_legal_moves<Movegen::GenType::ALL>(pos, pos.get_side(), moves);

    for(auto& move : moves) {
        if(make(pos, move)) {
            nodes += perft(pos, depth - 1);
            unmake(pos, move);
        }
    }

    return nodes;
}

bool Movegen::make(Position& pos, Move move) {
    if(move.is_null()) return 0;
    PIECE_C us = pos.get_side();
    PIECE_C enemy = us == PIECE_C::WHITE ? PIECE_C::BLACK : PIECE_C::WHITE;
    auto from_sq = move.get_from();
    auto to_sq = move.get_to();
    PIECE_T pt = pos.pieces.get_piece_on(from_sq);
    auto king_bb = pos.pieces.get_pieces(us, PIECE_T::KING);
    auto king_sq = std::countr_zero(king_bb);

    bool ep = 0;
    auto cstl_perms = pos.get_castle();
    auto half_moves = pos.get_half_moves();
    auto en_pas = pos.get_en_pas();

    uint64_t z_hash = pos.get_zobrist_hash();
    if (us == PIECE_C::WHITE) {
        pos.xor_hash(Zobrist::white_keys[static_cast<int>(pt)][from_sq]);
    } else if (us == PIECE_C::BLACK) {
        pos.xor_hash(Zobrist::black_keys[static_cast<int>(pt)][from_sq]);
    }
    pos.xor_hash(Zobrist::cstl_keys[cstl_perms]);
    if (en_pas < 64) {
        auto ep_file = en_pas & 7;
        pos.xor_hash(Zobrist::ep_keys[ep_file]);
    }

    Undo undo = {
        .side = us,
        .castle = cstl_perms,
        .en_pas = en_pas,
        .half_moves = half_moves,
        .ply = pos.get_ply(),
        .full_moves = pos.get_full_moves(),
        .zobrist_hash = z_hash
    };

    switch(static_cast<MOVE_FLAG>(move.get_flags())) {
        case MOVE_FLAG::QUIET:
            pos.pieces.remove_piece(us, pt, from_sq);
            pos.pieces.set_piece(us, pt, to_sq);

            if(pt == PIECE_T::KING)
                king_sq = std::countr_zero(pos.pieces.get_pieces(us, pt));

            if(us == PIECE_C::WHITE) {
                pos.xor_hash(Zobrist::white_keys[static_cast<int>(pt)][to_sq]);

                if(pt == PIECE_T::KING && from_sq == 4) cstl_perms &= ~(1 << 0 | 1 << 1);
                if(pt == PIECE_T::ROOK && from_sq == 7) cstl_perms &= ~(1 << 0);
                if(pt == PIECE_T::ROOK && from_sq == 0) cstl_perms &= ~(1 << 1);
            } else {
                pos.xor_hash(Zobrist::black_keys[static_cast<int>(pt)][to_sq]);

                if(pt == PIECE_T::KING && from_sq == 60) cstl_perms &= ~(1 << 2 | 1 << 3);
                if(pt == PIECE_T::ROOK && from_sq == 63) cstl_perms &= ~(1 << 2);
                if(pt == PIECE_T::ROOK && from_sq == 56) cstl_perms &= ~(1 << 3);
            }

            if(is_sq_attacked_by_color(pos, king_sq, enemy)) {
                pos.pieces.remove_piece(us, pt, to_sq);
                pos.pieces.set_piece(us, pt, from_sq);
                pos.set_hash(z_hash);

                return false;
            }

            if(pt != PIECE_T::PAWN)
                half_moves++;
            else
                half_moves = 0;

            break;
        case MOVE_FLAG::DBL_P_PUSH:
            pos.pieces.remove_piece(us, pt, from_sq);
            pos.pieces.set_piece(us, pt, to_sq);

            if (us == PIECE_C::WHITE) {
                pos.xor_hash(Zobrist::white_keys[static_cast<int>(pt)][to_sq]);
            } else if (us == PIECE_C::BLACK) {
                pos.xor_hash(Zobrist::black_keys[static_cast<int>(pt)][to_sq]);
            }

            if(is_sq_attacked_by_color(pos, king_sq, enemy)) {
                pos.pieces.remove_piece(us, pt, to_sq);
                pos.pieces.set_piece(us, pt, from_sq);
                pos.set_hash(z_hash);

                return false;
            }

            ep = 1;
            half_moves = 0;

            break;
        case MOVE_FLAG::K_CSTL:
            if(us == PIECE_C::WHITE) {
                pos.pieces.remove_piece(us, pt, from_sq);
                pos.pieces.set_piece(us, pt, to_sq);
                pos.xor_hash(Zobrist::white_keys[static_cast<int>(pt)][to_sq]);

                pos.pieces.remove_piece(us, PIECE_T::ROOK, 7);
                pos.pieces.set_piece(us, PIECE_T::ROOK, 5);
                pos.xor_hash(Zobrist::white_keys[static_cast<int>(PIECE_T::ROOK)][7]);
                pos.xor_hash(Zobrist::white_keys[static_cast<int>(PIECE_T::ROOK)][5]);
                cstl_perms &= ~(1 << 0 | 1 << 1);
            } else {
                pos.pieces.remove_piece(us, pt, from_sq);
                pos.pieces.set_piece(us, pt, to_sq);
                pos.xor_hash(Zobrist::black_keys[static_cast<int>(pt)][to_sq]);

                pos.pieces.remove_piece(us, PIECE_T::ROOK, 63);
                pos.pieces.set_piece(us, PIECE_T::ROOK, 61);
                pos.xor_hash(Zobrist::black_keys[static_cast<int>(PIECE_T::ROOK)][63]);
                pos.xor_hash(Zobrist::black_keys[static_cast<int>(PIECE_T::ROOK)][61]);
                cstl_perms &= ~(1 << 2 | 1 << 3);
            }

            half_moves++;

            break;
        case MOVE_FLAG::Q_CSTL:
            if(us == PIECE_C::WHITE) {
                pos.pieces.remove_piece(us, pt, from_sq);
                pos.pieces.set_piece(us, pt, to_sq);
                pos.xor_hash(Zobrist::white_keys[static_cast<int>(pt)][to_sq]);

                pos.pieces.remove_piece(us, PIECE_T::ROOK, 0);
                pos.pieces.set_piece(us, PIECE_T::ROOK, 3);
                pos.xor_hash(Zobrist::white_keys[static_cast<int>(PIECE_T::ROOK)][0]);
                pos.xor_hash(Zobrist::white_keys[static_cast<int>(PIECE_T::ROOK)][3]);
                cstl_perms &= ~(1 << 0 | 1 << 1);
            } else {
                pos.pieces.remove_piece(us, pt, from_sq);
                pos.pieces.set_piece(us, pt, to_sq);
                pos.xor_hash(Zobrist::black_keys[static_cast<int>(pt)][to_sq]);

                pos.pieces.remove_piece(us, PIECE_T::ROOK, 56);
                pos.pieces.set_piece(us, PIECE_T::ROOK, 59);
                pos.xor_hash(Zobrist::black_keys[static_cast<int>(PIECE_T::ROOK)][56]);
                pos.xor_hash(Zobrist::black_keys[static_cast<int>(PIECE_T::ROOK)][59]);
                cstl_perms &= ~(1 << 2 | 1 << 3);
            }

            half_moves++;

            break;
        case MOVE_FLAG::CAPTURE:
        {
            PIECE_T to_capture = pos.pieces.get_piece_on(to_sq);
            undo.captured = to_capture;
            pos.pieces.remove_piece(us, pt, from_sq);
            pos.pieces.remove_piece(enemy, to_capture, to_sq);
            pos.pieces.set_piece(us, pt, to_sq);
            if(pt == PIECE_T::KING)
                king_sq = std::countr_zero(pos.pieces.get_pieces(us, pt));

            if(us == PIECE_C::WHITE) {
                pos.xor_hash(Zobrist::white_keys[static_cast<int>(pt)][to_sq]);
                pos.xor_hash(Zobrist::black_keys[static_cast<int>(to_capture)][to_sq]);

                if(pt == PIECE_T::KING && from_sq == 4) cstl_perms &= ~(1 << 0 | 1 << 1);

                if(pt == PIECE_T::ROOK && from_sq == 7) cstl_perms &= ~(1 << 0);
                if(pt == PIECE_T::ROOK && from_sq == 0) cstl_perms &= ~(1 << 1);

                if(to_capture == PIECE_T::ROOK && to_sq == 63) cstl_perms &= ~(1 << 2);
                if(to_capture == PIECE_T::ROOK && to_sq == 56) cstl_perms &= ~(1 << 3);
            } else {
                pos.xor_hash(Zobrist::black_keys[static_cast<int>(pt)][to_sq]);
                pos.xor_hash(Zobrist::white_keys[static_cast<int>(to_capture)][to_sq]);

                if(pt == PIECE_T::KING && from_sq == 60) cstl_perms &= ~(1 << 2 | 1 << 3);

                if(pt == PIECE_T::ROOK && from_sq == 63) cstl_perms &= ~(1 << 2);
                if(pt == PIECE_T::ROOK && from_sq == 56) cstl_perms &= ~(1 << 3);

                if(to_capture == PIECE_T::ROOK && to_sq == 7) cstl_perms &= ~(1 << 0);
                if(to_capture == PIECE_T::ROOK && to_sq == 0) cstl_perms &= ~(1 << 1);
            }

            if(is_sq_attacked_by_color(pos, king_sq, enemy)) {
                pos.pieces.remove_piece(us, pt, to_sq);
                pos.pieces.set_piece(us, pt, from_sq);
                pos.pieces.set_piece(enemy, to_capture, to_sq);
                pos.set_hash(z_hash);

                return false;
            }

            half_moves = 0;

            break;
        }
        case MOVE_FLAG::EP_CAPTURE:
        {
            PIECE_T to_capture;
            if(us == PIECE_C::WHITE) {
                to_capture = pos.pieces.get_piece_on(to_sq - 8);
            } else {
                to_capture = pos.pieces.get_piece_on(to_sq + 8);
            }

            undo.captured = to_capture;

            pos.pieces.remove_piece(us, pt, from_sq);
            if(us == PIECE_C::WHITE) {
                pos.pieces.remove_piece(enemy, to_capture, to_sq - 8);
                pos.xor_hash(Zobrist::white_keys[static_cast<int>(pt)][to_sq]);
                pos.xor_hash(Zobrist::black_keys[static_cast<int>(to_capture)][to_sq - 8]);
            } else {
                pos.pieces.remove_piece(enemy, to_capture, to_sq + 8);
                pos.xor_hash(Zobrist::black_keys[static_cast<int>(pt)][to_sq]);
                pos.xor_hash(Zobrist::white_keys[static_cast<int>(to_capture)][to_sq + 8]);
            }

            pos.pieces.set_piece(us, pt, to_sq);

            if(is_sq_attacked_by_color(pos, king_sq, enemy)) {
                pos.pieces.remove_piece(us, pt, to_sq);
                pos.pieces.set_piece(us, pt, from_sq);

                if(us == PIECE_C::WHITE) {
                    pos.pieces.set_piece(enemy, to_capture, to_sq - 8);
                } else if (us == PIECE_C::BLACK) {
                    pos.pieces.set_piece(enemy, to_capture, to_sq + 8);
                }

                pos.set_hash(z_hash);

                return false;
            }

            half_moves = 0;

            break;
        }
        case MOVE_FLAG::KNIGHT_PROMO:
            pos.pieces.remove_piece(us, pt, from_sq);
            pos.pieces.set_piece(us, PIECE_T::KNIGHT, to_sq);

            if (us == PIECE_C::WHITE) {
                pos.xor_hash(Zobrist::white_keys[static_cast<int>(PIECE_T::KNIGHT)][to_sq]);
            } else if (us == PIECE_C::BLACK) {
                pos.xor_hash(Zobrist::black_keys[static_cast<int>(PIECE_T::KNIGHT)][to_sq]);
            }

            if(is_sq_attacked_by_color(pos, king_sq, enemy)) {
                pos.pieces.remove_piece(us, PIECE_T::KNIGHT, to_sq);
                pos.pieces.set_piece(us, pt, from_sq);
                pos.set_hash(z_hash);

                return false;
            }

            half_moves = 0;

            break;
        case MOVE_FLAG::BISHOP_PROMO:
            pos.pieces.remove_piece(us, pt, from_sq);
            pos.pieces.set_piece(us, PIECE_T::BISHOP, to_sq);

            if (us == PIECE_C::WHITE) {
                pos.xor_hash(Zobrist::white_keys[static_cast<int>(PIECE_T::BISHOP)][to_sq]);
            } else if (us == PIECE_C::BLACK) {
                pos.xor_hash(Zobrist::black_keys[static_cast<int>(PIECE_T::BISHOP)][to_sq]);
            }

            if(is_sq_attacked_by_color(pos, king_sq, enemy)) {
                pos.pieces.remove_piece(us, PIECE_T::BISHOP, to_sq);
                pos.pieces.set_piece(us, pt, from_sq);
                pos.set_hash(z_hash);

                return false;
            }

            half_moves = 0;

            break;
        case MOVE_FLAG::ROOK_PROMO:
            pos.pieces.remove_piece(us, pt, from_sq);
            pos.pieces.set_piece(us, PIECE_T::ROOK, to_sq);

            if (us == PIECE_C::WHITE) {
                pos.xor_hash(Zobrist::white_keys[static_cast<int>(PIECE_T::ROOK)][to_sq]);
            } else if (us == PIECE_C::BLACK) {
                pos.xor_hash(Zobrist::black_keys[static_cast<int>(PIECE_T::ROOK)][to_sq]);
            }

            if(is_sq_attacked_by_color(pos, king_sq, enemy)) {
                pos.pieces.remove_piece(us, PIECE_T::ROOK, to_sq);
                pos.pieces.set_piece(us, pt, from_sq);
                pos.set_hash(z_hash);

                return false;
            }

            half_moves = 0;

            break;
        case MOVE_FLAG::QUEEN_PROMO:
            pos.pieces.remove_piece(us, pt, from_sq);
            pos.pieces.set_piece(us, PIECE_T::QUEEN, to_sq);

            if (us == PIECE_C::WHITE) {
                pos.xor_hash(Zobrist::white_keys[static_cast<int>(PIECE_T::QUEEN)][to_sq]);
            } else if (us == PIECE_C::BLACK) {
                pos.xor_hash(Zobrist::black_keys[static_cast<int>(PIECE_T::QUEEN)][to_sq]);
            }

            if(is_sq_attacked_by_color(pos, king_sq, enemy)) {
                pos.pieces.remove_piece(us, PIECE_T::QUEEN, to_sq);
                pos.pieces.set_piece(us, pt, from_sq);
                pos.set_hash(z_hash);

                return false;
            }

            half_moves = 0;

            break;
        case MOVE_FLAG::KNIGHT_PROMO_CAPTURE:
        {
            PIECE_T to_capture = pos.pieces.get_piece_on(to_sq);
            undo.captured = to_capture;
            pos.pieces.remove_piece(us, pt, from_sq);
            pos.pieces.remove_piece(enemy, to_capture, to_sq);
            pos.pieces.set_piece(us, PIECE_T::KNIGHT, to_sq);

            if(us == PIECE_C::WHITE) {
                pos.xor_hash(Zobrist::black_keys[static_cast<int>(to_capture)][to_sq]);
                pos.xor_hash(Zobrist::white_keys[static_cast<int>(PIECE_T::KNIGHT)][to_sq]);

                if(to_capture == PIECE_T::ROOK && to_sq == 63) cstl_perms &= ~(1 << 2);
                if(to_capture == PIECE_T::ROOK && to_sq == 56) cstl_perms &= ~(1 << 3);
            } else if (us == PIECE_C::BLACK) {
                pos.xor_hash(Zobrist::white_keys[static_cast<int>(to_capture)][to_sq]);
                pos.xor_hash(Zobrist::black_keys[static_cast<int>(PIECE_T::KNIGHT)][to_sq]);

                if(to_capture == PIECE_T::ROOK && to_sq == 7) cstl_perms &= ~(1 << 0);
                if(to_capture == PIECE_T::ROOK && to_sq == 0) cstl_perms &= ~(1 << 1);
            }

            if(is_sq_attacked_by_color(pos, king_sq, enemy)) {
                pos.pieces.remove_piece(us, PIECE_T::KNIGHT, to_sq);
                pos.pieces.set_piece(enemy, to_capture, to_sq);
                pos.pieces.set_piece(us, pt, from_sq);
                pos.set_hash(z_hash);

                return false;
            }

            half_moves = 0;

            break;
        }
        case MOVE_FLAG::BISHOP_PROMO_CAPTURE:
        {
            PIECE_T to_capture = pos.pieces.get_piece_on(to_sq);
            undo.captured = to_capture;
            pos.pieces.remove_piece(us, pt, from_sq);
            pos.pieces.remove_piece(enemy, to_capture, to_sq);
            pos.pieces.set_piece(us, PIECE_T::BISHOP, to_sq);

            if(us == PIECE_C::WHITE) {
                pos.xor_hash(Zobrist::black_keys[static_cast<int>(to_capture)][to_sq]);
                pos.xor_hash(Zobrist::white_keys[static_cast<int>(PIECE_T::BISHOP)][to_sq]);

                if(to_capture == PIECE_T::ROOK && to_sq == 63) cstl_perms &= ~(1 << 2);
                if(to_capture == PIECE_T::ROOK && to_sq == 56) cstl_perms &= ~(1 << 3);
            } else {
                pos.xor_hash(Zobrist::white_keys[static_cast<int>(to_capture)][to_sq]);
                pos.xor_hash(Zobrist::black_keys[static_cast<int>(PIECE_T::BISHOP)][to_sq]);

                if(to_capture == PIECE_T::ROOK && to_sq == 7) cstl_perms &= ~(1 << 0);
                if(to_capture == PIECE_T::ROOK && to_sq == 0) cstl_perms &= ~(1 << 1);
            }

            if(is_sq_attacked_by_color(pos, king_sq, enemy)) {
                pos.pieces.remove_piece(us, PIECE_T::BISHOP, to_sq);
                pos.pieces.set_piece(enemy, to_capture, to_sq);
                pos.pieces.set_piece(us, pt, from_sq);
                pos.set_hash(z_hash);

                return false;
            }

            half_moves = 0;

            break;
        }
        case MOVE_FLAG::ROOK_PROMO_CAPTURE:
        {
            PIECE_T to_capture = pos.pieces.get_piece_on(to_sq);
            undo.captured = to_capture;
            pos.pieces.remove_piece(us, pt, from_sq);
            pos.pieces.remove_piece(enemy, to_capture, to_sq);
            pos.pieces.set_piece(us, PIECE_T::ROOK, to_sq);

            if(us == PIECE_C::WHITE) {
                pos.xor_hash(Zobrist::black_keys[static_cast<int>(to_capture)][to_sq]);
                pos.xor_hash(Zobrist::white_keys[static_cast<int>(PIECE_T::ROOK)][to_sq]);

                if(to_capture == PIECE_T::ROOK && to_sq == 63) cstl_perms &= ~(1 << 2);
                if(to_capture == PIECE_T::ROOK && to_sq == 56) cstl_perms &= ~(1 << 3);
            } else {
                pos.xor_hash(Zobrist::white_keys[static_cast<int>(to_capture)][to_sq]);
                pos.xor_hash(Zobrist::black_keys[static_cast<int>(PIECE_T::ROOK)][to_sq]);

                if(to_capture == PIECE_T::ROOK && to_sq == 7) cstl_perms &= ~(1 << 0);
                if(to_capture == PIECE_T::ROOK && to_sq == 0) cstl_perms &= ~(1 << 1);
            }

            if(is_sq_attacked_by_color(pos, king_sq, enemy)) {
                pos.pieces.remove_piece(us, PIECE_T::ROOK, to_sq);
                pos.pieces.set_piece(enemy, to_capture, to_sq);
                pos.pieces.set_piece(us, pt, from_sq);
                pos.set_hash(z_hash);

                return false;
            }

            half_moves = 0;

            break;
        }
        case MOVE_FLAG::QUEEN_PROMO_CAPTURE:
        {
            PIECE_T to_capture = pos.pieces.get_piece_on(to_sq);
            undo.captured = to_capture;
            pos.pieces.remove_piece(us, pt, from_sq);
            pos.pieces.remove_piece(enemy, to_capture, to_sq);
            pos.pieces.set_piece(us, PIECE_T::QUEEN, to_sq);

            if(us == PIECE_C::WHITE) {
                pos.xor_hash(Zobrist::black_keys[static_cast<int>(to_capture)][to_sq]);
                pos.xor_hash(Zobrist::white_keys[static_cast<int>(PIECE_T::QUEEN)][to_sq]);

                if(to_capture == PIECE_T::ROOK && to_sq == 63) cstl_perms &= ~(1 << 2);
                if(to_capture == PIECE_T::ROOK && to_sq == 56) cstl_perms &= ~(1 << 3);
            } else {
                pos.xor_hash(Zobrist::white_keys[static_cast<int>(to_capture)][to_sq]);
                pos.xor_hash(Zobrist::black_keys[static_cast<int>(PIECE_T::QUEEN)][to_sq]);

                if(to_capture == PIECE_T::ROOK && to_sq == 7) cstl_perms &= ~(1 << 0);
                if(to_capture == PIECE_T::ROOK && to_sq == 0) cstl_perms &= ~(1 << 1);
            }

            if(is_sq_attacked_by_color(pos, king_sq, enemy)) {
                pos.pieces.remove_piece(us, PIECE_T::QUEEN, to_sq);
                pos.pieces.set_piece(enemy, to_capture, to_sq);
                pos.pieces.set_piece(us, pt, from_sq);
                pos.set_hash(z_hash);

                return false;
            }

            half_moves = 0;

            break;
        }
        default:
            pos.set_hash(z_hash);

            return false;
    }

    if(ep) {
        if(us == PIECE_C::WHITE) {
            pos.set_en_pas(to_sq - 8);
        } else {
            pos.set_en_pas(to_sq + 8);
        }

        auto ep_file = pos.get_en_pas() & 7;
        pos.xor_hash(Zobrist::ep_keys[ep_file]);
    } else {
        pos.set_en_pas(64);
    }

    if(us == PIECE_C::BLACK)
        pos.set_full_moves(pos.get_full_moves() + 1);

    pos.set_ply(pos.get_ply() + 1);
    pos.set_half_moves(half_moves);
    pos.set_castle(cstl_perms);
    pos.append_to_undos(undo);
    pos.set_side(enemy);

    pos.xor_hash(Zobrist::side_key);
    pos.xor_hash(Zobrist::cstl_keys[cstl_perms]);

    return true;
}

void Movegen::unmake(Position& pos, Move move) {
    Undo undo = pos.fetch_and_pop_undos();
    pos.set_side(undo.side);
    pos.set_castle(undo.castle);
    pos.set_en_pas(undo.en_pas);
    pos.set_half_moves(undo.half_moves);
    pos.set_ply(undo.ply);
    pos.set_full_moves(undo.full_moves);
    pos.set_hash(undo.zobrist_hash);

    PIECE_C us = pos.get_side();
    PIECE_C enemy = us == PIECE_C::WHITE ? PIECE_C::BLACK : PIECE_C::WHITE;
    auto from_sq = move.get_from();
    auto to_sq = move.get_to();
    PIECE_T pt = pos.pieces.get_piece_on(to_sq);
    auto king_bb = pos.pieces.get_pieces(us, PIECE_T::KING);
    auto king_sq = std::countr_zero(king_bb);
    PIECE_T captured = undo.captured;

    switch(static_cast<MOVE_FLAG>(move.get_flags())) {
        case MOVE_FLAG::QUIET:
            pos.pieces.remove_piece(us, pt, to_sq);
            pos.pieces.set_piece(us, pt, from_sq);

            break;
        case MOVE_FLAG::DBL_P_PUSH:
            pos.pieces.remove_piece(us, pt, to_sq);
            pos.pieces.set_piece(us, pt, from_sq);

            break;
        case MOVE_FLAG::K_CSTL:
            pos.pieces.remove_piece(us, pt, to_sq);
            pos.pieces.set_piece(us, pt, from_sq);

            if(us == PIECE_C::WHITE) {
                pos.pieces.remove_piece(us, PIECE_T::ROOK, 5);
                pos.pieces.set_piece(us, PIECE_T::ROOK, 7);
            } else {
                pos.pieces.remove_piece(us, PIECE_T::ROOK, 61);
                pos.pieces.set_piece(us, PIECE_T::ROOK, 63);
            }

            break;
        case MOVE_FLAG::Q_CSTL:
            pos.pieces.remove_piece(us, pt, to_sq);
            pos.pieces.set_piece(us, pt, from_sq);

            if(us == PIECE_C::WHITE) {
                pos.pieces.remove_piece(us, PIECE_T::ROOK, 3);
                pos.pieces.set_piece(us, PIECE_T::ROOK, 0);
            } else {
                pos.pieces.remove_piece(us, PIECE_T::ROOK, 59);
                pos.pieces.set_piece(us, PIECE_T::ROOK, 56);
            }

            break;
        case MOVE_FLAG::CAPTURE:
            pos.pieces.remove_piece(us, pt, to_sq);
            pos.pieces.set_piece(enemy, captured, to_sq);
            pos.pieces.set_piece(us, pt, from_sq);

            break;
        case MOVE_FLAG::EP_CAPTURE:
            pos.pieces.remove_piece(us, pt, to_sq);
            if(us == PIECE_C::WHITE) {
                pos.pieces.set_piece(enemy, captured, to_sq - 8);
            } else {
                pos.pieces.set_piece(enemy, captured, to_sq + 8);
            }

            pos.pieces.set_piece(us, pt, from_sq);

            break;
        case MOVE_FLAG::KNIGHT_PROMO:
            pos.pieces.remove_piece(us, pt, to_sq);
            pos.pieces.set_piece(us, PIECE_T::PAWN, from_sq);

            break;
        case MOVE_FLAG::BISHOP_PROMO:
            pos.pieces.remove_piece(us, pt, to_sq);
            pos.pieces.set_piece(us, PIECE_T::PAWN, from_sq);

            break;
        case MOVE_FLAG::ROOK_PROMO:
            pos.pieces.remove_piece(us, pt, to_sq);
            pos.pieces.set_piece(us, PIECE_T::PAWN, from_sq);

            break;
        case MOVE_FLAG::QUEEN_PROMO:
            pos.pieces.remove_piece(us, pt, to_sq);
            pos.pieces.set_piece(us, PIECE_T::PAWN, from_sq);

            break;
        case MOVE_FLAG::KNIGHT_PROMO_CAPTURE:
            pos.pieces.remove_piece(us, pt, to_sq);
            pos.pieces.set_piece(us, PIECE_T::PAWN, from_sq);
            pos.pieces.set_piece(enemy, captured, to_sq);

            break;
        case MOVE_FLAG::BISHOP_PROMO_CAPTURE:
            pos.pieces.remove_piece(us, pt, to_sq);
            pos.pieces.set_piece(us, PIECE_T::PAWN, from_sq);
            pos.pieces.set_piece(enemy, captured, to_sq);

            break;
        case MOVE_FLAG::ROOK_PROMO_CAPTURE:
            pos.pieces.remove_piece(us, pt, to_sq);
            pos.pieces.set_piece(us, PIECE_T::PAWN, from_sq);
            pos.pieces.set_piece(enemy, captured, to_sq);

            break;
        case MOVE_FLAG::QUEEN_PROMO_CAPTURE:
            pos.pieces.remove_piece(us, pt, to_sq);
            pos.pieces.set_piece(us, PIECE_T::PAWN, from_sq);
            pos.pieces.set_piece(enemy, captured, to_sq);

            break;
        default:
            break;
    }
}

template void Movegen::generate_pseudo_legal_moves<Movegen::GenType::ALL>(Position &pos, PIECE_C color, std::vector<Move> &moves);
template void Movegen::generate_pseudo_legal_moves<Movegen::GenType::CAPTURES>(Position &pos, PIECE_C color, std::vector<Move> &moves);
