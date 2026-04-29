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

uint8_t Move::get_flags() const {
    uint8_t res = 0;
    res = (this->move & 0x000F);
    return res;
}


Move Move::encode(uint8_t from, uint8_t to, MOVE_FLAG flags) {
    if(to > 63) return null();
    if(from > 63) return null();
    uint16_t res = 0;
    res |= static_cast<uint16_t>(from) << 10;
    res |= static_cast<uint16_t>(to) << 4;
    res |= static_cast<uint16_t>(flags);

    return Move(res);
}
