#pragma once

#include <cstdint>

enum class MOVE_FLAG { QUIET = 0, DBL_P_PUSH = 1, K_CSTL = 2, Q_CSTL = 3,
    CAPTURE = 4, EP_CAPTURE = 5, KNIGHT_PROMO = 8,
    BISHOP_PROMO = 9, ROOK_PROMO = 10, QUEEN_PROMO = 11, KNIGHT_PROMO_CAPTURE = 12,
    BISHOP_PROMO_CAPTURE = 13, ROOK_PROMO_CAPTURE = 14, QUEEN_PROMO_CAPTURE = 15
};

class Move {
    private:
        uint16_t move;
        Move(uint16_t move) : move(move) {}

    public:
        Move() : move(0) {}
        uint8_t get_from() const;
        uint8_t get_to() const;
        uint8_t get_flags() const;
        static Move encode(uint8_t from, uint8_t to, MOVE_FLAG flags);
        static Move null() { return Move(0); }
        bool is_null() const { return this->move == 0; }
        bool operator==(const Move mv) {
            return move == mv.move;
        }
};
