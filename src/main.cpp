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

void print_knights(Position& pos, PIECE_C color, const char* label) {
    std::cout << label << " KNIGHTS\n\n";
    debug_bitboard(pos.pieces.get_pieces(color, PIECE_T::KNIGHT));
    std::cout << '\n';
}
std::string square_name(int square) {
    const char file = static_cast<char>('a' + (square % 8));
    const char rank = static_cast<char>('1' + (square / 8));

    return {file, rank};
}

const char* move_flag_name(uint8_t flag) {
    switch (static_cast<MOVE_FLAG>(flag)) {
        case MOVE_FLAG::QUIET: return "quiet";
        case MOVE_FLAG::DBL_P_PUSH: return "double pawn push";
        case MOVE_FLAG::K_CSTL: return "king castle";
        case MOVE_FLAG::Q_CSTL: return "queen castle";
        case MOVE_FLAG::CAPTURE: return "capture";
        case MOVE_FLAG::EP_CAPTURE: return "en passant";
        case MOVE_FLAG::KNIGHT_PROMO: return "promotion=knight";
        case MOVE_FLAG::BISHOP_PROMO: return "promotion=bishop";
        case MOVE_FLAG::ROOK_PROMO: return "promotion=rook";
        case MOVE_FLAG::QUEEN_PROMO: return "promotion=queen";
        case MOVE_FLAG::KNIGHT_PROMO_CAPTURE: return "capture promotion=knight";
        case MOVE_FLAG::BISHOP_PROMO_CAPTURE: return "capture promotion=bishop";
        case MOVE_FLAG::ROOK_PROMO_CAPTURE: return "capture promotion=rook";
        case MOVE_FLAG::QUEEN_PROMO_CAPTURE: return "capture promotion=queen";
        default: return "unknown";
    }
}

void print_knight_moves(Position& pos, PIECE_C color, const char* label) {
    std::vector<Move> moves;
    Movegen::generate_knight_moves(pos, color, moves);

    std::cout << label << " MOVES (" << moves.size() << ")\n";

    for (const auto& move : moves) {
        uint8_t from = move.get_from();
        uint8_t to = move.get_to();

        std::cout
            << square_name(from) << " -> " << square_name(to)
            << "  (" << static_cast<int>(from)
            << " -> " << static_cast<int>(to) << ")";

        const uint8_t flag = move.get_flags();
        if (flag != static_cast<uint8_t>(MOVE_FLAG::QUIET)) {
            std::cout << " " << move_flag_name(flag);
        }

        std::cout << '\n';
    }

    std::cout << '\n';
}

int main() {
    Position pos;
    auto fen = "8/8/8/8/8/8/8/N7 w - - 0 1";
    load_fen(pos, fen);
    print_knights(pos, PIECE_C::WHITE, "WHITE");
    print_knights(pos, PIECE_C::BLACK, "BLACK");

    print_knight_moves(pos, PIECE_C::WHITE, "WHITE");

    return 0;
}
