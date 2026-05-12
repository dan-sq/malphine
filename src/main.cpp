#include "board/board.h"
#include "board/fen.h"
#include "move/move.h"
#include "movegen/movegen.h"

#include <cstdint>
#include <iostream>
#include <set>
#include <string>
#include <vector>

using MovegenFn = void (*)(Position&, PIECE_C, std::vector<Move>&);

struct DebugStats {
    int passed = 0;
    int failed = 0;
};

std::string square_name(int square) {
    const char file = static_cast<char>('a' + (square % 8));
    const char rank = static_cast<char>('1' + (square / 8));
    return std::string{file, rank};
}

uint8_t square_from_name(const std::string& square) {
    const int file = square[0] - 'a';
    const int rank = square[1] - '1';
    return static_cast<uint8_t>(rank * 8 + file);
}

const char* flag_suffix(uint8_t flag) {
    switch(static_cast<MOVE_FLAG>(flag)) {
    case MOVE_FLAG::CAPTURE:
        return "x";
    case MOVE_FLAG::EP_CAPTURE:
        return "ep";
    case MOVE_FLAG::DBL_P_PUSH:
        return "dbl";
    case MOVE_FLAG::K_CSTL:
        return "O-O";
    case MOVE_FLAG::Q_CSTL:
        return "O-O-O";
    case MOVE_FLAG::KNIGHT_PROMO:
        return "=N";
    case MOVE_FLAG::BISHOP_PROMO:
        return "=B";
    case MOVE_FLAG::ROOK_PROMO:
        return "=R";
    case MOVE_FLAG::QUEEN_PROMO:
        return "=Q";
    case MOVE_FLAG::KNIGHT_PROMO_CAPTURE:
        return "x=N";
    case MOVE_FLAG::BISHOP_PROMO_CAPTURE:
        return "x=B";
    case MOVE_FLAG::ROOK_PROMO_CAPTURE:
        return "x=R";
    case MOVE_FLAG::QUEEN_PROMO_CAPTURE:
        return "x=Q";
    default:
        return "";
    }
}

std::string move_name(const Move& move) {
    return square_name(move.get_from()) + "->" + square_name(move.get_to()) +
           flag_suffix(move.get_flags());
}

std::set<std::string> collect_move_names(const std::vector<Move>& moves) {
    std::set<std::string> names;
    for(const auto& move : moves) {
        names.insert(move_name(move));
    }
    return names;
}

uint64_t collect_to_bitboard(const std::vector<Move>& moves) {
    uint64_t bb = 0;
    for(const auto& move : moves) {
        bb |= static_cast<uint64_t>(1) << move.get_to();
    }
    return bb;
}

void print_bitboard(uint64_t bb) {
    for(int rank = 7; rank >= 0; rank--) {
        std::cout << rank + 1 << "  ";
        for(int file = 0; file < 8; file++) {
            const int square = rank * 8 + file;
            std::cout << (((bb >> square) & 1ULL) ? "1 " : ". ");
        }
        std::cout << '\n';
    }
    std::cout << "\n   a b c d e f g h\n";
}

void print_moves(const std::set<std::string>& moves) {
    if(moves.empty()) {
        std::cout << "  (no moves)\n";
        return;
    }

    for(const auto& move : moves) {
        std::cout << "  " << move << '\n';
    }
}

void print_difference(const std::set<std::string>& generated,
                      const std::set<std::string>& expected) {
    bool printed_header = false;
    for(const auto& move : expected) {
        if(!generated.contains(move)) {
            if(!printed_header) {
                std::cout << "Missing:\n";
                printed_header = true;
            }
            std::cout << "  " << move << '\n';
        }
    }

    printed_header = false;
    for(const auto& move : generated) {
        if(!expected.contains(move)) {
            if(!printed_header) {
                std::cout << "Extra:\n";
                printed_header = true;
            }
            std::cout << "  " << move << '\n';
        }
    }
}

void debug_move_case(DebugStats& stats,
                     const std::string& group,
                     const std::string& label,
                     const std::string& fen,
                     PIECE_C color,
                     MovegenFn generate,
                     const std::set<std::string>& expected) {
    Position pos;
    load_fen(pos, fen);

    std::vector<Move> moves;
    generate(pos, color, moves);

    const auto generated = collect_move_names(moves);
    const bool passed = generated == expected;

    std::cout << "\n[" << group << "] " << label << '\n';
    std::cout << "FEN: " << fen << '\n';
    std::cout << "Generated moves (" << generated.size() << "):\n";
    print_moves(generated);
    std::cout << "Destination bitboard:\n";
    print_bitboard(collect_to_bitboard(moves));

    if(passed) {
        stats.passed++;
        std::cout << "PASS\n";
    } else {
        stats.failed++;
        std::cout << "FAIL\n";
        print_difference(generated, expected);
    }
}

void debug_attack_case(DebugStats& stats,
                       const std::string& label,
                       const std::string& fen,
                       const std::string& square,
                       PIECE_C attacking_color,
                       bool expected) {
    Position pos;
    load_fen(pos, fen);

    const bool attacked = Movegen::is_sq_attacked_by_color(pos, square_from_name(square), attacking_color);
    const bool passed = attacked == expected;

    std::cout << "\n[attack] " << label << '\n';
    std::cout << "FEN: " << fen << '\n';
    std::cout << square << " attacked by "
              << (attacking_color == PIECE_C::WHITE ? "white" : "black")
              << ": " << (attacked ? "true" : "false") << '\n';

    if(passed) {
        stats.passed++;
        std::cout << "PASS\n";
    } else {
        stats.failed++;
        std::cout << "FAIL expected " << (expected ? "true" : "false") << '\n';
    }
}

void init_movegen() {
    Movegen::init_diagonal_cache();
    Movegen::init_horizontal_cache();
}

void run_piece_move_tests(DebugStats& stats) {
    debug_move_case(stats, "piece", "white pawn pushes and captures",
                    "8/8/8/8/8/3p1p2/4P3/8 w - - 0 1",
                    PIECE_C::WHITE,
                    Movegen::generate_pawn_moves,
                    {
                        "e2->d3x",
                        "e2->e3",
                        "e2->e4dbl",
                        "e2->f3x",
                    });

    debug_move_case(stats, "piece", "black pawn pushes and captures",
                    "8/4p3/3P1P2/8/8/8/8/8 b - - 0 1",
                    PIECE_C::BLACK,
                    Movegen::generate_pawn_moves,
                    {
                        "e7->d6x",
                        "e7->e5dbl",
                        "e7->e6",
                        "e7->f6x",
                    });

    debug_move_case(stats, "piece", "white pawn en passant",
                    "8/8/8/3pP3/8/8/8/8 w - d6 0 1",
                    PIECE_C::WHITE,
                    Movegen::generate_pawn_moves,
                    {
                        "e5->d6ep",
                        "e5->e6",
                    });

    debug_move_case(stats, "piece", "white promotion push and captures",
                    "2n1r3/1P6/8/8/8/8/8/8 w - - 0 1",
                    PIECE_C::WHITE,
                    Movegen::generate_pawn_moves,
                    {
                        "b7->b8=B",
                        "b7->b8=N",
                        "b7->b8=Q",
                        "b7->b8=R",
                        "b7->c8x=B",
                        "b7->c8x=N",
                        "b7->c8x=Q",
                        "b7->c8x=R",
                    });

    debug_move_case(stats, "piece", "white knight blocked by own piece",
                    "8/8/8/1p3P2/3N4/8/8/8 w - - 0 1",
                    PIECE_C::WHITE,
                    Movegen::generate_knight_moves,
                    {
                        "d4->b3",
                        "d4->b5x",
                        "d4->c2",
                        "d4->c6",
                        "d4->e2",
                        "d4->e6",
                        "d4->f3",
                    });

    debug_move_case(stats, "piece", "white bishop magic moves",
                    "8/8/2p3P1/8/4B3/8/6p1/8 w - - 0 1",
                    PIECE_C::WHITE,
                    Movegen::generate_bishop_moves,
                    {
                        "e4->b1",
                        "e4->c2",
                        "e4->c6x",
                        "e4->d3",
                        "e4->d5",
                        "e4->f3",
                        "e4->f5",
                        "e4->g2x",
                    });

    debug_move_case(stats, "piece", "white rook magic moves",
                    "8/8/4P3/8/1P2R1p1/8/4p3/8 w - - 0 1",
                    PIECE_C::WHITE,
                    Movegen::generate_rook_moves,
                    {
                        "e4->c4",
                        "e4->d4",
                        "e4->e2x",
                        "e4->e3",
                        "e4->e5",
                        "e4->f4",
                        "e4->g4x",
                    });

    debug_move_case(stats, "piece", "white queen combines bishop and rook moves",
                    "8/8/2p1P1P1/8/1P2Q1p1/8/4p1p1/8 w - - 0 1",
                    PIECE_C::WHITE,
                    Movegen::generate_queen_moves,
                    {
                        "e4->b1",
                        "e4->c2",
                        "e4->c4",
                        "e4->c6x",
                        "e4->d3",
                        "e4->d4",
                        "e4->d5",
                        "e4->e2x",
                        "e4->e3",
                        "e4->e5",
                        "e4->f3",
                        "e4->f4",
                        "e4->f5",
                        "e4->g2x",
                        "e4->g4x",
                    });

    debug_move_case(stats, "piece", "white king on corner pseudo-legal moves",
                    "8/8/8/8/8/8/1p6/K7 w - - 0 1",
                    PIECE_C::WHITE,
                    Movegen::generate_king_moves,
                    {
                        "a1->a2",
                        "a1->b1",
                        "a1->b2x",
                    });
}

void run_attack_tests(DebugStats& stats) {
    debug_attack_case(stats, "rook attacks along open file",
                      "4r3/8/8/8/4K3/8/8/8 w - - 0 1",
                      "e4", PIECE_C::BLACK, true);

    debug_attack_case(stats, "rook attack blocked by piece",
                      "4r3/8/4p3/8/4K3/8/8/8 w - - 0 1",
                      "e4", PIECE_C::BLACK, false);

    debug_attack_case(stats, "bishop attacks diagonal",
                      "8/7b/8/8/4K3/8/8/8 w - - 0 1",
                      "e4", PIECE_C::BLACK, true);

    debug_attack_case(stats, "knight attacks target",
                      "8/8/5n2/8/4K3/8/8/8 w - - 0 1",
                      "e4", PIECE_C::BLACK, true);

    debug_attack_case(stats, "black pawn attacks downward",
                      "8/8/8/3p4/4K3/8/8/8 w - - 0 1",
                      "e4", PIECE_C::BLACK, true);

    debug_attack_case(stats, "king attacks adjacent square",
                      "8/8/8/4k3/4K3/8/8/8 w - - 0 1",
                      "e4", PIECE_C::BLACK, true);
}

void run_castling_tests(DebugStats& stats) {
    debug_move_case(stats, "castling", "white king side castle",
                    "4k3/8/8/8/8/8/8/4K2R w K - 0 1",
                    PIECE_C::WHITE,
                    Movegen::generate_king_moves,
                    {
                        "e1->d1",
                        "e1->d2",
                        "e1->e2",
                        "e1->f1",
                        "e1->f2",
                        "e1->g1O-O",
                    });

    debug_move_case(stats, "castling", "white queen side castle",
                    "4k3/8/8/8/8/8/8/R3K3 w Q - 0 1",
                    PIECE_C::WHITE,
                    Movegen::generate_king_moves,
                    {
                        "e1->c1O-O-O",
                        "e1->d1",
                        "e1->d2",
                        "e1->e2",
                        "e1->f1",
                        "e1->f2",
                    });

    debug_move_case(stats, "castling", "black king side castle",
                    "4k2r/8/8/8/8/8/8/4K3 b k - 0 1",
                    PIECE_C::BLACK,
                    Movegen::generate_king_moves,
                    {
                        "e8->d7",
                        "e8->d8",
                        "e8->e7",
                        "e8->f7",
                        "e8->f8",
                        "e8->g8O-O",
                    });

    debug_move_case(stats, "castling", "black queen side castle",
                    "r3k3/8/8/8/8/8/8/4K3 b q - 0 1",
                    PIECE_C::BLACK,
                    Movegen::generate_king_moves,
                    {
                        "e8->c8O-O-O",
                        "e8->d7",
                        "e8->d8",
                        "e8->e7",
                        "e8->f7",
                        "e8->f8",
                    });

    debug_move_case(stats, "castling", "white cannot castle through attacked square",
                    "k4r2/8/8/8/8/8/8/4K2R w K - 0 1",
                    PIECE_C::WHITE,
                    Movegen::generate_king_moves,
                    {
                        "e1->d1",
                        "e1->d2",
                        "e1->e2",
                        "e1->f1",
                        "e1->f2",
                    });

    debug_move_case(stats, "castling", "white cannot castle through occupied squares",
                    "4k3/8/8/8/8/8/8/R2BK1NR w KQ - 0 1",
                    PIECE_C::WHITE,
                    Movegen::generate_king_moves,
                    {
                        "e1->d2",
                        "e1->e2",
                        "e1->f1",
                        "e1->f2",
                    });

    debug_move_case(stats, "castling", "rook generator should not create castling moves",
                    "4k3/8/8/8/8/8/8/R3K2R w KQ - 0 1",
                    PIECE_C::WHITE,
                    Movegen::generate_rook_moves,
                    {
                        "a1->a2",
                        "a1->a3",
                        "a1->a4",
                        "a1->a5",
                        "a1->a6",
                        "a1->a7",
                        "a1->a8",
                        "a1->b1",
                        "a1->c1",
                        "a1->d1",
                        "h1->f1",
                        "h1->g1",
                        "h1->h2",
                        "h1->h3",
                        "h1->h4",
                        "h1->h5",
                        "h1->h6",
                        "h1->h7",
                        "h1->h8",
                    });
}

void run_pseudo_legal_tests(DebugStats& stats) {
    debug_move_case(stats, "pseudo", "all pseudo-legal moves includes rooks and king",
                    "8/8/8/8/8/8/8/R3K3 w - - 0 1",
                    PIECE_C::WHITE,
                    Movegen::generate_pseudo_legal_moves,
                    {
                        "a1->a2",
                        "a1->a3",
                        "a1->a4",
                        "a1->a5",
                        "a1->a6",
                        "a1->a7",
                        "a1->a8",
                        "a1->b1",
                        "a1->c1",
                        "a1->d1",
                        "e1->d1",
                        "e1->d2",
                        "e1->e2",
                        "e1->f1",
                        "e1->f2",
                    });

    debug_move_case(stats, "pseudo", "pinned rook still moves in pseudo-legal generation",
                    "4r3/8/8/8/8/8/4R3/4K3 w - - 0 1",
                    PIECE_C::WHITE,
                    Movegen::generate_rook_moves,
                    {
                        "e2->a2",
                        "e2->b2",
                        "e2->c2",
                        "e2->d2",
                        "e2->e3",
                        "e2->e4",
                        "e2->e5",
                        "e2->e6",
                        "e2->e7",
                        "e2->e8x",
                        "e2->f2",
                        "e2->g2",
                        "e2->h2",
                    });

    debug_move_case(stats, "pseudo", "king moves are pseudo-legal before make/unmake filtering",
                    "4r3/8/8/8/4K3/8/8/8 w - - 0 1",
                    PIECE_C::WHITE,
                    Movegen::generate_king_moves,
                    {
                        "e4->d3",
                        "e4->d4",
                        "e4->d5",
                        "e4->e3",
                        "e4->e5",
                        "e4->f3",
                        "e4->f4",
                        "e4->f5",
                    });
}

int main() {
    init_movegen();

    DebugStats stats;
    run_piece_move_tests(stats);
    run_attack_tests(stats);
    run_castling_tests(stats);
    run_pseudo_legal_tests(stats);

    std::cout << "\nSummary: " << stats.passed << " passed, "
              << stats.failed << " failed\n";

    return stats.failed == 0 ? 0 : 1;
}
