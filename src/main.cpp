#include "board/board.h"
#include "board/fen.h"
#include "movegen/movegen.h"
#include <iostream>
#include <cstdint>
#include <vector>
#include <bit>
#include <set>

std::string square_name(int square);

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

uint64_t debug_bishop_blocker_mask(uint8_t sq) {
    int diagonals[4][2] = {
        { 1, 1 },
        { 1, -1 },
        {-1, 1 },
        { -1, -1 }
    };

    uint64_t blocker_mask_bb = 0;
    for(int i = 0; i < 4; i++) {
        int file = sq % 8 + diagonals[i][0];
        int rank = sq / 8 + diagonals[i][1];
        while((file > 0 && file < 7) && (rank > 0 && rank < 7)) {
            blocker_mask_bb |= static_cast<uint64_t>(1) << (8 * rank + file);
            file += diagonals[i][0];
            rank += diagonals[i][1];
        }
    }

    return blocker_mask_bb;
}

void debug_bishop_magic_helpers(uint8_t sq) {
    uint64_t blocker_mask_bb = debug_bishop_blocker_mask(sq);
    auto ones = std::popcount(blocker_mask_bb);
    uint64_t size = static_cast<uint64_t>(1) << ones;

    std::cout << "DEBUG bishop square: " << square_name(sq)
              << " (" << static_cast<int>(sq) << ")\n\n";

    std::cout << "blocker_mask_bb, relevant bits = " << ones
              << ", possible blocker boards = " << size << "\n";
    debug_bitboard(blocker_mask_bb);
    std::cout << '\n';

    
}

void debug_init_bishop_magics() {
    std::cout << "Initializing bishop magics...\n" << std::flush;
    Movegen::init_diagonal_cache();
    std::cout << "Finished initializing bishop magics.\n\n";
}

void debug_init_rook_magics() {
    std::cout << "Initializing rook magics...\n" << std::flush;
    Movegen::init_horizontal_cache();
    std::cout << "Finished initializing rook magics.\n\n";
}

void print_bishop(Position& pos, PIECE_C color, const char* label) {
    std::cout << label << " BISHOPS\n\n";
    debug_bitboard(pos.pieces.get_pieces(color, PIECE_T::BISHOP));
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

void print_bishop_moves(Position& pos, PIECE_C color, const char* label) {
    std::vector<Move> moves;
    Movegen::generate_bishop_moves(pos, color, moves);

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

void debug_bishop_case(
    const char* label,
    const std::string& fen,
    PIECE_C color,
    const std::set<std::string>& expected_moves
) {
    Position pos;
    load_fen(pos, fen);

    std::vector<Move> moves;
    Movegen::generate_bishop_moves(pos, color, moves);

    std::set<std::string> actual_moves;
    uint64_t actual_moves_bb = 0;
    for(const auto& move : moves) {
        std::string move_text = square_name(move.get_from()) + "->" + square_name(move.get_to());
        if(move.get_flags() == static_cast<uint8_t>(MOVE_FLAG::CAPTURE)) {
            move_text += "x";
        }
        actual_moves.insert(move_text);
        actual_moves_bb |= static_cast<uint64_t>(1) << move.get_to();
    }

    std::cout << "BISHOP DEBUG: " << label << '\n';
    std::cout << "FEN: " << fen << '\n';
    std::cout << "expected (" << expected_moves.size() << ")\n";
    for(const auto& move : expected_moves) {
        std::cout << "  " << move << '\n';
    }

    std::cout << "actual (" << actual_moves.size() << ")\n";
    for(const auto& move : actual_moves) {
        std::cout << "  " << move << '\n';
    }

    std::cout << "actual move bitboard\n";
    debug_bitboard(actual_moves_bb);

    bool ok = actual_moves == expected_moves;
    std::cout << (ok ? "PASS" : "FAIL") << "\n\n";
}

void debug_rook_case(
    const char* label,
    const std::string& fen,
    PIECE_C color,
    const std::set<std::string>& expected_moves
) {
    Position pos;
    load_fen(pos, fen);

    std::vector<Move> moves;
    Movegen::generate_rook_moves(pos, color, moves);

    std::set<std::string> actual_moves;
    uint64_t actual_moves_bb = 0;
    for(const auto& move : moves) {
        std::string move_text = square_name(move.get_from()) + "->" + square_name(move.get_to());
        if(move.get_flags() == static_cast<uint8_t>(MOVE_FLAG::CAPTURE)) {
            move_text += "x";
        }
        actual_moves.insert(move_text);
        actual_moves_bb |= static_cast<uint64_t>(1) << move.get_to();
    }

    std::cout << "ROOK DEBUG: " << label << '\n';
    std::cout << "FEN: " << fen << '\n';
    std::cout << "expected (" << expected_moves.size() << ")\n";
    for(const auto& move : expected_moves) {
        std::cout << "  " << move << '\n';
    }

    std::cout << "actual (" << actual_moves.size() << ")\n";
    for(const auto& move : actual_moves) {
        std::cout << "  " << move << '\n';
    }

    std::cout << "actual move bitboard\n";
    debug_bitboard(actual_moves_bb);

    bool ok = actual_moves == expected_moves;
    std::cout << (ok ? "PASS" : "FAIL") << "\n\n";
}

int main() {
    Position pos;
    auto fen = "8/8/8/4B3/8/8/8/8 w - - 0 1";
    load_fen(pos, fen);

    debug_init_bishop_magics();
    debug_init_rook_magics();
   

    debug_rook_case(
        "empty board, rook on e5",
        "8/8/8/4R3/8/8/8/8 w - - 0 1",
        PIECE_C::WHITE,
        {
            "e5->e1", "e5->e2", "e5->e3", "e5->e4",
            "e5->e6", "e5->e7", "e5->e8",
            "e5->a5", "e5->b5", "e5->c5", "e5->d5",
            "e5->f5", "e5->g5", "e5->h5"
        }
    );

    debug_rook_case(
        "rook on e4, own blocker e6 and b4, captures e2 and g4",
        "8/8/4P3/8/1P2R1p1/8/4p3/8 w - - 0 1",
        PIECE_C::WHITE,
        {
            "e4->e3", "e4->e2x",
            "e4->e5",
            "e4->c4", "e4->d4",
            "e4->f4", "e4->g4x"
        }
    );

    //debug_bishop_magic_helpers(28); // e4

    print_bishop(pos, PIECE_C::WHITE, "WHITE");
    //print_knights(pos, PIECE_C::BLACK, "BLACK");

    print_bishop_moves(pos, PIECE_C::WHITE, "WHITE");
    //print_bishop_moves(pos, PIECE_C::BLACK, "BLACK");

    return 0;
}
