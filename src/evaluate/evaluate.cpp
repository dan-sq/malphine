#include "evaluate/evaluate.h"
#include "board/board.h"

uint8_t Evaluate::mirror_sq(uint8_t sq) {
    return sq ^ 56;
}

int Evaluate::eval(Position &pos) {
    int score = 0;

    for(int c = 0; c < 2; c++) {
        PIECE_C color = static_cast<PIECE_C>(c);
        for(int p = 0; p < 6; p++) {
            PIECE_T piece = static_cast<PIECE_T>(p);
            auto pieces = pos.pieces.get_pieces(color, piece);

            while(pieces) {
                uint8_t sq = std::countr_zero(pieces);
                if(static_cast<PIECE_C>(c) == PIECE_C::WHITE) {
                    score += PIECE_VALS[p] + PST[static_cast<int>(piece)][sq];
                } else {
                    score -= PIECE_VALS[p] + PST[static_cast<int>(piece)][mirror_sq(sq)];
                }

                pieces &= pieces - 1;
            }
        }
    }

    return score;
}
