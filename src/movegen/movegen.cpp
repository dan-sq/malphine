#include "movegen.h"
#include "move/move.h"
#include <cstdint>
#include <vector>

void Movegen::generate_pawn_moves(Position& pos, PIECE_C color, std::vector<Move>& moves) {
    auto bitboard = pos.pieces.get_pieces(color, PIECE_T::PAWN);
    auto empty = ~pos.pieces.get_both_pieces();
    auto enemy = color == PIECE_C::WHITE ? PIECE_C::BLACK: PIECE_C::WHITE;
    auto enemy_bb = pos.pieces.get_colored_pieces(enemy);

    while(bitboard) {
        int sq = __builtin_ctzll(bitboard);
        bitboard &= bitboard - 1;
        uint64_t from_bb = static_cast<uint64_t>(1) << sq;
        uint64_t en_pas_bb = pos.get_en_pas() < 64 ? static_cast<uint64_t>(1) << pos.get_en_pas() : 0;
        if(color == PIECE_C::WHITE) {
            auto push_bb = from_bb << 8;
            auto left_attack_bb = (from_bb & ~FILE_A) << 7;
            auto right_attack_bb = (from_bb & ~FILE_H) << 9;
            if(push_bb & empty) {
                moves.push_back(Move::encode(sq, sq + 8, MOVE_FLAG::QUIET)); 

                if((from_bb & rank2) && ((push_bb << 8) & empty)) {
                    moves.push_back(Move::encode(sq, sq + 16, MOVE_FLAG::DBL_P_PUSH));
                }
            }
            if(left_attack_bb & enemy_bb) {
                moves.push_back(Move::encode(sq, sq + 7, MOVE_FLAG::CAPTURE));
            }
            if(right_attack_bb & enemy_bb) {
                moves.push_back(Move::encode(sq, sq + 9, MOVE_FLAG::CAPTURE));
            }
            if(left_attack_bb & en_pas_bb) {
                moves.push_back(Move::encode(sq, sq + 7, MOVE_FLAG::EP_CAPTURE));
            }
            if(right_attack_bb & en_pas_bb) {
                moves.push_back(Move::encode(sq, sq + 9, MOVE_FLAG::EP_CAPTURE));
            }
            if(from_bb & rank7 && push_bb & empty) {
                moves.push_back(Move::encode(sq, sq + 8, MOVE_FLAG::KNIGHT_PROMO));
                moves.push_back(Move::encode(sq, sq + 8, MOVE_FLAG::BISHOP_PROMO));
                moves.push_back(Move::encode(sq, sq + 8, MOVE_FLAG::ROOK_PROMO));
                moves.push_back(Move::encode(sq, sq + 8, MOVE_FLAG::QUEEN_PROMO));
            }
            if(from_bb & rank7 && left_attack_bb & enemy_bb) {
                moves.push_back(Move::encode(sq, sq + 7, MOVE_FLAG::KNIGHT_PROMO_CAPTURE));
                moves.push_back(Move::encode(sq, sq + 7, MOVE_FLAG::BISHOP_PROMO_CAPTURE));
                moves.push_back(Move::encode(sq, sq + 7, MOVE_FLAG::ROOK_PROMO_CAPTURE));
                moves.push_back(Move::encode(sq, sq + 7, MOVE_FLAG::QUEEN_PROMO_CAPTURE));
            }
            if(from_bb & rank7 && right_attack_bb & enemy_bb) {
                moves.push_back(Move::encode(sq, sq + 9, MOVE_FLAG::KNIGHT_PROMO_CAPTURE));
                moves.push_back(Move::encode(sq, sq + 9, MOVE_FLAG::BISHOP_PROMO_CAPTURE));
                moves.push_back(Move::encode(sq, sq + 9, MOVE_FLAG::ROOK_PROMO_CAPTURE));
                moves.push_back(Move::encode(sq, sq + 9, MOVE_FLAG::QUEEN_PROMO_CAPTURE));
            }
        } else {
            auto push_bb = from_bb >> 8;
            auto left_attack_bb = (from_bb & ~FILE_H) >> 7;
            auto right_attack_bb = (from_bb & ~FILE_A) >> 9;
            if(push_bb & empty) {
                moves.push_back(Move::encode(sq, sq - 8, MOVE_FLAG::QUIET)); 

                if((from_bb & rank2) && ((push_bb >> 8) & empty)) {
                    moves.push_back(Move::encode(sq, sq - 16, MOVE_FLAG::DBL_P_PUSH));
                }
            }
            if(left_attack_bb & enemy_bb) {
                moves.push_back(Move::encode(sq, sq - 7, MOVE_FLAG::CAPTURE));
            }
            if(right_attack_bb & enemy_bb) {
                moves.push_back(Move::encode(sq, sq - 9, MOVE_FLAG::CAPTURE));
            }
            if(left_attack_bb & en_pas_bb) {
                moves.push_back(Move::encode(sq, sq - 7, MOVE_FLAG::EP_CAPTURE));
            }
            if(right_attack_bb & en_pas_bb) {
                moves.push_back(Move::encode(sq, sq - 9, MOVE_FLAG::EP_CAPTURE));
            }
            if(from_bb & rank2 && push_bb & empty) {
                moves.push_back(Move::encode(sq, sq - 8, MOVE_FLAG::KNIGHT_PROMO));
                moves.push_back(Move::encode(sq, sq - 8, MOVE_FLAG::BISHOP_PROMO));
                moves.push_back(Move::encode(sq, sq - 8, MOVE_FLAG::ROOK_PROMO));
                moves.push_back(Move::encode(sq, sq - 8, MOVE_FLAG::QUEEN_PROMO));
            }
            if(from_bb & rank2 && left_attack_bb & enemy_bb) {
                moves.push_back(Move::encode(sq, sq - 7, MOVE_FLAG::KNIGHT_PROMO_CAPTURE));
                moves.push_back(Move::encode(sq, sq - 7, MOVE_FLAG::BISHOP_PROMO_CAPTURE));
                moves.push_back(Move::encode(sq, sq - 7, MOVE_FLAG::ROOK_PROMO_CAPTURE));
                moves.push_back(Move::encode(sq, sq - 7, MOVE_FLAG::QUEEN_PROMO_CAPTURE));
            }
            if(from_bb & rank2 && right_attack_bb & enemy_bb) {
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
    auto empty = ~pos.pieces.get_both_pieces();
    auto enemy = color == PIECE_C::WHITE ? PIECE_C::BLACK: PIECE_C::WHITE;
    auto enemy_bb = pos.pieces.get_colored_pieces(enemy);

    while(bitboard) {
        int sq = __builtin_ctzll(bitboard);
        bitboard &= bitboard - 1;
        uint64_t from_bb = static_cast<uint64_t>(1) << sq;

        if(color == PIECE_C::WHITE) {

        } else {
        }

    }
}

void Movegen::generate_bishop_moves(Position& pos, PIECE_C color, std::vector<Move>& moves);
void Movegen::generate_rook_moves(Position& pos, PIECE_C color, std::vector<Move>& moves);
void Movegen::generate_queen_moves(Position& pos, PIECE_C color, std::vector<Move>& moves);
void Movegen::generate_king_moves(Position& pos, PIECE_C color, std::vector<Move>& moves);
void Movegen::generate_all_moves(Position& pos, PIECE_C color, std::vector<Move>& moves);
