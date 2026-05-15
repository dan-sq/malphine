#include "board.h"
#include <array>

void Position::set_side(PIECE_C s) {
    this->side = s;
}

PIECE_C Position::get_side() {
    return this->side;
}

PIECE_C Position::fetch_and_change_side() {
    if(this->side == PIECE_C::WHITE) {
        set_side(PIECE_C::BLACK);
        return PIECE_C::WHITE;
    } else {
        set_side(PIECE_C::WHITE);
        return PIECE_C::BLACK;
    }
}

void Position::set_castle(uint8_t cstl) {
    this->castle = cstl;
}

uint8_t Position::get_castle() {
    return this->castle;
}

void Position::set_en_pas(uint8_t ep) {
    this->en_pas = ep;
}

uint8_t Position::get_en_pas() const {
    return this->en_pas;
}

void Position::set_half_moves(uint8_t hp) {
    this->half_moves = hp;
}

uint8_t Position::get_half_moves() {
    return this->half_moves;
}

void Position::set_ply(uint8_t p) {
    this->ply = p;
}

uint8_t Position::get_ply() {
    return this->ply;
}

void Position::set_full_moves(uint8_t fm) {
    this->full_moves = fm;
}

uint8_t Position::get_full_moves() {
    return this->full_moves;
}

uint64_t Pieces::get_pieces(PIECE_C color, PIECE_T type) const {
    return piece_board[static_cast<int>(color)][static_cast<int>(type)];
}

PIECE_T Pieces::get_piece_on(uint8_t sq) {
    auto to_bb = static_cast<uint64_t>(1) << sq;
    for(int c = 0; c < NUM_COLORS; c++) {
        for(int p = 0; p < NUM_PIECES; p++) {
            auto pieces = this->get_pieces(static_cast<PIECE_C>(c), static_cast<PIECE_T>(p));
            if (pieces & to_bb) return static_cast<PIECE_T>(p);
        }
    }

    return PIECE_T::NONE;
}

Undo Position::fetch_and_pop_undos() {
    auto u = this->undos.back();
    this->undos.pop_back();

    return u;
}

void Position::append_to_undos(Undo undo) {
    this->undos.push_back(undo);
}

void Pieces::set_piece(PIECE_C color, PIECE_T type, int square) {
    this->piece_board[static_cast<int>(color)][static_cast<int>(type)] |= (static_cast<uint64_t>(1) << square);
}

void Pieces::remove_piece(PIECE_C color, PIECE_T type, int square) {
    this->piece_board[static_cast<int>(color)][static_cast<int>(type)] &= ~(static_cast<uint64_t>(1) << square);
}

void Position::remove_piece(PIECE_C color, PIECE_T type, int square) {
    if(square < 0 || square > 63) return;
    this->pieces.remove_piece(color, type, square);
}

void Position::set_piece(PIECE_C color, PIECE_T type, int square) {
    if(square <  0 || square > 63) return;
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
