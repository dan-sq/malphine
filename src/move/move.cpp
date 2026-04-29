#include "move.h"
#include <cstdint>

uint8_t Move::get_from() const {
    uint8_t res = 0;
    res = this->move >> 10;
    return res;
}

uint8_t Move::get_to() const {
    uint8_t res = 0;
    res = (this->move >> 4) & 0x003F;
    return res;
}

uint8_t Move::get_promoted() const {
    uint8_t res = 0;
    res = (this->move >> 2) & 0x0003;
    return res;
}

uint8_t Move::get_flags() const {
    uint8_t res = 0;
    res = (this->move & 0x0003);
    return res;
}

// 000000 000000 00 000000000000000000

Move Move::encode(uint8_t from, uint8_t to, uint8_t promoted, uint8_t flags) {
    if(to > 63) return null();
    if(from > 63) return null();
    uint32_t res = 0;
    res |= static_cast<uint32_t>(from) << 26;
    res |= static_cast<uint32_t>(to) << 20;
    res |= promoted << 18;
    res |= flags;

    return Move(res);
}
