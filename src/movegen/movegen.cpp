#include "movegen.h"
#include "board/board.h"
#include "move/move.h"
#include <cstdint>
#include <vector>
#include <bit>
#include <random>

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
            auto left_blocker_mask_bb = (from_bb & ~FILE_H) >> 7;
            auto right_blocker_mask_bb = (from_bb & ~FILE_A) >> 9;
            if(push_bb & empty) {
                moves.push_back(Move::encode(sq, sq - 8, MOVE_FLAG::QUIET)); 

                if((from_bb & rank2) && ((push_bb >> 8) & empty)) {
                    moves.push_back(Move::encode(sq, sq - 16, MOVE_FLAG::DBL_P_PUSH));
                }
            }
            if(left_blocker_mask_bb & enemy_bb) {
                moves.push_back(Move::encode(sq, sq - 7, MOVE_FLAG::CAPTURE));
            }
            if(right_blocker_mask_bb & enemy_bb) {
                moves.push_back(Move::encode(sq, sq - 9, MOVE_FLAG::CAPTURE));
            }
            if(left_blocker_mask_bb & en_pas_bb) {
                moves.push_back(Move::encode(sq, sq - 7, MOVE_FLAG::EP_CAPTURE));
            }
            if(right_blocker_mask_bb & en_pas_bb) {
                moves.push_back(Move::encode(sq, sq - 9, MOVE_FLAG::EP_CAPTURE));
            }
            if(from_bb & rank2 && push_bb & empty) {
                moves.push_back(Move::encode(sq, sq - 8, MOVE_FLAG::KNIGHT_PROMO));
                moves.push_back(Move::encode(sq, sq - 8, MOVE_FLAG::BISHOP_PROMO));
                moves.push_back(Move::encode(sq, sq - 8, MOVE_FLAG::ROOK_PROMO));
                moves.push_back(Move::encode(sq, sq - 8, MOVE_FLAG::QUEEN_PROMO));
            }
            if(from_bb & rank2 && left_blocker_mask_bb & enemy_bb) {
                moves.push_back(Move::encode(sq, sq - 7, MOVE_FLAG::KNIGHT_PROMO_CAPTURE));
                moves.push_back(Move::encode(sq, sq - 7, MOVE_FLAG::BISHOP_PROMO_CAPTURE));
                moves.push_back(Move::encode(sq, sq - 7, MOVE_FLAG::ROOK_PROMO_CAPTURE));
                moves.push_back(Move::encode(sq, sq - 7, MOVE_FLAG::QUEEN_PROMO_CAPTURE));
            }
            if(from_bb & rank2 && right_blocker_mask_bb & enemy_bb) {
                moves.push_back(Move::encode(sq, sq - 9, MOVE_FLAG::KNIGHT_PROMO_CAPTURE));
                moves.push_back(Move::encode(sq, sq - 9, MOVE_FLAG::BISHOP_PROMO_CAPTURE));
                moves.push_back(Move::encode(sq, sq - 9, MOVE_FLAG::ROOK_PROMO_CAPTURE));
                moves.push_back(Move::encode(sq, sq - 9, MOVE_FLAG::QUEEN_PROMO_CAPTURE));
            }
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
    if((distance_from_files(sq % 8, to_sq % 8) == 2 && (distance_from_ranks(sq / 8, to_sq / 8) == 1))
            || (distance_from_files(sq % 8, to_sq % 8) == 1 && (distance_from_ranks(sq / 8, to_sq / 8) == 2))) {
        return 1;
    } else {
        return 0;
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

        for(int i = 0; i < 8; i++) {
            int to_sq = sq + KNIGHT_OFFSETS[i];
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

uint64_t Movegen::blocked_bishop_attacks(uint64_t bb, uint8_t sq) {
        int diagonals[4][2] = {
            { 1, 1 },
            { 1, -1 },
            {-1, 1 },
            { -1, -1 }
        };

        uint64_t res = 0;
        for(int i = 0; i < 4; i++) {
            int file = sq % 8 + diagonals[i][0];
            int rank = sq / 8 + diagonals[i][1];
            while((file > -1 && file < 8) && (rank > -1 && rank < 8)) {
                uint64_t to_bb = static_cast<uint64_t>(1) << (8 * rank + file);
                res |= to_bb;
                if(bb & to_bb) {
                    break;
                }
                file += diagonals[i][0];
                rank += diagonals[i][1];
            }
        }

        return res;
}

uint64_t Movegen::bb_from_idx(uint64_t idx, uint64_t bb) {
    uint64_t blocker_bb = 0;
    uint64_t bit_index = 0;

    while (bb) {
        int sq = std::countr_zero(bb);
        bb &= bb - 1;

        if (idx & (static_cast<uint64_t>(1) << bit_index)) {
            blocker_bb |= static_cast<uint64_t>(1) << sq;
        }

        bit_index++;
    }

    return blocker_bb;
}

uint64_t bishop_blocker_mask_bb(uint8_t sq) {
    int diagonals[4][2] = {
        { 1, 1 },
        { 1, -1 },
        {-1, 1 },
        { -1, -1 }
    };
    uint64_t blocker_mask_bb = 0;
    for(int i = 0; i < 4; i++) {
        int file = sq % 8 + diagonals[i][0];
        int rank = sq / 8 + diagonals[i][1];
        while((file > 0 && file < 7) && (rank > 0 && rank < 7)) {
            blocker_mask_bb |= static_cast<uint64_t>(1) << (8 * rank + file);
            file += diagonals[i][0];
            rank += diagonals[i][1];
        }
    }
    return blocker_mask_bb;
}

uint64_t generate_magic_number(uint8_t sq) {
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> dis(0, std::numeric_limits<std::uint64_t>::max());
    auto blocker_mask_bb = bishop_blocker_mask_bb(sq);
    Movegen::bishop_masks[sq] = blocker_mask_bb;
    auto ones_count = std::popcount(blocker_mask_bb);
    auto table_size = static_cast<uint64_t>(1) << ones_count;
    auto shift = 64 - ones_count;
    Movegen::bishop_shifts[sq] = shift;
    uint64_t magic = 0;

    while(1) {
        magic = dis(gen) & dis(gen) & dis(gen);
        std::array<uint64_t, 512> lookup = {};
        bool filled[512] = {};
        bool failed = 0;
        for(uint64_t idx = 0; idx < table_size; idx++) {
            auto blocker_bb = Movegen::bb_from_idx(idx, blocker_mask_bb);
            auto attack_bb = Movegen::blocked_bishop_attacks(blocker_bb, sq);
            uint64_t magic_idx = (blocker_bb * magic) >> shift;
            if(!filled[magic_idx]) {
                filled[magic_idx] = 1;
                lookup[magic_idx] = attack_bb;
            } else if(lookup[magic_idx] != attack_bb){
                failed = 1;
                break;
            }
            if(failed) break;
        }
        if(!failed) {
            Movegen::bishop_attacks[sq] = lookup;
            break;
        }
    }

    return magic;
}

void Movegen::init_bishop_magics() {
    for(int i = 0; i < 64; i++) {
        auto magic = generate_magic_number(i);
        bishop_magics[i] = magic;
    }
}

void Movegen::generate_bishop_moves(Position& pos, PIECE_C color, std::vector<Move>& moves) {
    auto bitboard = pos.pieces.get_pieces(color, PIECE_T::BISHOP);
    auto empty_bb = ~pos.pieces.get_both_pieces();
    auto us = color == PIECE_C::WHITE ? PIECE_C::WHITE : PIECE_C::BLACK;
    auto enemy = color == PIECE_C::WHITE ? PIECE_C::BLACK : PIECE_C::WHITE;
    auto enemy_bb = pos.pieces.get_colored_pieces(enemy);
    auto occupancy_bb = ~empty_bb;

    while(bitboard) {
        int sq = std::countr_zero(bitboard);
        bitboard &= bitboard - 1;
        uint64_t from_bb = static_cast<uint64_t>(1) << sq;
        auto blocker_bb = occupancy_bb & bishop_masks[sq];
        auto magic_idx = (blocker_bb * bishop_magics[sq]) >> bishop_shifts[sq];
        auto attack_bb = bishop_attacks[sq][magic_idx];
        attack_bb &= ~pos.pieces.get_colored_pieces(us);

        while(attack_bb) {
            int to_sq = std::countr_zero(attack_bb);
            auto to_bb = static_cast<uint64_t>(1) << to_sq;
            attack_bb &= attack_bb- 1;

            if(to_bb & empty_bb) {
                moves.push_back(Move::encode(sq, to_sq, MOVE_FLAG::QUIET));
            }

            if(to_bb & enemy_bb) {
                moves.push_back(Move::encode(sq, to_sq, MOVE_FLAG::CAPTURE));
            }
        }
    }
}
void Movegen::generate_rook_moves(Position& pos, PIECE_C color, std::vector<Move>& moves);
void Movegen::generate_queen_moves(Position& pos, PIECE_C color, std::vector<Move>& moves);
void Movegen::generate_king_moves(Position& pos, PIECE_C color, std::vector<Move>& moves);
void Movegen::generate_all_moves(Position& pos, PIECE_C color, std::vector<Move>& moves);
