#pragma once

#include <cstdint>

class Move {
    private:
        uint32_t move;
        Move(uint16_t move) : move(move) {}

    public:
        Move() : move(0) {}
        
        uint8_t get_from() const;
        uint8_t get_to() const ;
        uint8_t get_promoted() const;
        uint8_t get_flags() const;
        static Move encode(uint8_t from, uint8_t to, uint8_t promoted, uint8_t flags);
        static Move null() { return Move(0); }
        bool is_null() const { return move == 0; }
};
