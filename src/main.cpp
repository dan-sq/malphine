#include "board/board.h"
#include "board/fen.h"
#include "movegen/movegen.h"
#include <iostream>
#include <cstdint>
#include <vector>

void debug_bitboard(uint64_t bitboard) {
    for(int rank = 7; rank >= 0; rank--) {
        for(int file = 0; file < 8; file++) {
            int square = rank * 8 + file;
            if((bitboard >> square & 1) == 1) {
                std::cout << "1";
            } else {
                std::cout << "0";
            }
        }
        std::cout << std::endl;
    }
}

void print_pawns(Position& pos, PIECE_C color, const char* label) {
    std::cout << label << " PAWNS\n\n";
    debug_bitboard(pos.pieces.get_pieces(color, PIECE_T::PAWN));
    std::cout << '\n';
}
std::string square_name(int square) {
    const char file = static_cast<char>('a' + (square % 8));
    const char rank = static_cast<char>('1' + (square / 8));

    return {file, rank};
}

const char* promotion_name(uint8_t promoted) {
    switch (promoted) {
        case 0: return "knight";
        case 1: return "bishop";
        case 2: return "rook";
        case 3: return "queen";
        default: return "unknown";
    }
}

void print_pawn_moves(Position& pos, PIECE_C color, const char* label) {
    std::vector<Move> moves;
    Movegen::generate_pawn_moves(pos, color, moves);

    std::cout << label << " MOVES (" << moves.size() << ")\n";

    for (const auto& move : moves) {
        uint8_t from = move.get_from();
        uint8_t to = move.get_to();

        std::cout
            << square_name(from) << " -> " << square_name(to)
            << "  (" << static_cast<int>(from)
            << " -> " << static_cast<int>(to) << ")";

        if (move.get_flags() == static_cast<int>(PAWN_FLAG::PROMOTION)) {
            std::cout
                << " promotion="
                << promotion_name(move.get_promoted());
        } else if (move.get_flags() == static_cast<int>(PAWN_FLAG::EN_PAS)) {
            std::cout << " en passant";
        }

        std::cout << '\n';
    }

    std::cout << '\n';
}

int main() {
    Position pos;
    std::vector<Move> white_moves, black_moves;
    auto fen = "8/8/8/3pP3/8/8/8/8 w - d6 0 1";
    load_fen(pos, fen);
    print_pawns(pos, PIECE_C::WHITE, "WHITE");
    print_pawns(pos, PIECE_C::BLACK, "BLACK");

    print_pawn_moves(pos, PIECE_C::WHITE, "WHITE");

    return 0;
}
