#include "transposition-table/zobrist.h"
#include <random>

void Zobrist::init() {
    std::mt19937_64 gen(0x1111111111111111ULL);

    for (auto& piece : white_keys) {
        for (auto& key : piece) {
            key = gen();
        }
    }

    for (auto& piece : black_keys) {
        for (auto& key : piece) {
            key = gen();
        }
    }

    side_key = gen();

    for (auto& key : cstl_keys) {
        key = gen();
    }

    for (auto& key : ep_keys) {
        key = gen();
    }
}
