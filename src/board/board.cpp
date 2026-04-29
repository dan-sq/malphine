#include "board.h"
#include <array>

void Position::set_side(PIECE_C s) {
    this->side = s;
}

void Position::set_castle(uint8_t cstl) {
    this->castle = cstl;
}

void Position::set_en_pas(uint8_t ep) {
    this->en_pas = ep;
}

uint8_t Position::get_en_pas() const {
    return this->en_pas;
}

void Position::set_half_ply(uint8_t hp) {
    this->half_ply = hp;
}

void Position::set_ply(uint8_t p) {
    this->ply = p;
}

uint64_t Pieces::get_pieces(PIECE_C color, PIECE_T type) const {
    return piece_board[static_cast<int>(color)][static_cast<int>(type)];
}

void Pieces::set_piece(PIECE_C color, PIECE_T type, int square) {
    this->piece_board[static_cast<int>(color)][static_cast<int>(type)] |= (static_cast<uint64_t>(1) << square);
}

void Position::set_piece(PIECE_C color, PIECE_T type, int square) {
    if(square <  0 or square > 63) return;
    this->pieces.set_piece(color, type, square);
}

uint64_t Pieces::get_colored_pieces(PIECE_C color) const {
    uint64_t res = 0;
    for(int i = 0; i < Pieces::NUM_PIECES; i++) {
        res |= piece_board[static_cast<int>(color)][i];
    }

    return res;
}

uint64_t Pieces::get_both_pieces() const {
    uint64_t res = 0;
    for(int i = 0; i < Pieces::NUM_PIECES; i++) {
        res |= piece_board[static_cast<int>(PIECE_C::WHITE)][i];
        res |= piece_board[static_cast<int>(PIECE_C::BLACK)][i];
    }

    return res;
}
