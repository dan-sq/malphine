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

std::string square_name(int square) {
    const char file = static_cast<char>('a' + (square % 8));
    const char rank = static_cast<char>('1' + (square / 8));
    return std::string{file, rank};
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

void debug_case(const std::string& group,
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
        std::cout << "PASS\n";
    } else {
        std::cout << "FAIL\n";
        print_difference(generated, expected);
    }
}

void init_movegen() {
    Movegen::init_diagonal_cache();
    Movegen::init_horizontal_cache();
}

int main() {
    init_movegen();

    debug_case("piece", "white pawn pushes and captures",
               "8/8/8/8/8/3p1p2/4P3/8 w - - 0 1",
               PIECE_C::WHITE,
               Movegen::generate_pawn_moves,
               {
                   "e2->d3x",
                   "e2->e3",
                   "e2->e4dbl",
                   "e2->f3x",
               });

    debug_case("piece", "white knight blocked by own piece",
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

    debug_case("piece", "white bishop magic moves",
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

    debug_case("piece", "white rook magic moves",
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

    debug_case("piece", "white queen combines bishop and rook moves",
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

    debug_case("piece", "white king on corner",
               "8/8/8/8/8/8/1p6/K7 w - - 0 1",
               PIECE_C::WHITE,
               Movegen::generate_king_moves,
               {
                   "a1->a2",
                   "a1->b1",
                   "a1->b2x",
               });

    debug_case("king safety", "king cannot move onto rook attack",
               "4r3/8/8/8/4K3/8/8/8 w - - 0 1",
               PIECE_C::WHITE,
               Movegen::generate_king_moves,
               {
                   "e4->d3",
                   "e4->d4",
                   "e4->d5",
                   "e4->e3",
                   "e4->f3",
                   "e4->f4",
                   "e4->f5",
               });

    debug_case("king safety", "king cannot move onto bishop attack",
               "7b/8/8/8/4K3/8/8/8 w - - 0 1",
               PIECE_C::WHITE,
               Movegen::generate_king_moves,
               {
                   "e4->d3",
                   "e4->d5",
                   "e4->e3",
                   "e4->f3",
                   "e4->f4",
                   "e4->f5",
               });

    debug_case("king safety", "king cannot move onto queen diagonal attack",
               "7q/8/8/8/4K3/8/8/8 w - - 0 1",
               PIECE_C::WHITE,
               Movegen::generate_king_moves,
               {
                   "e4->d3",
                   "e4->d5",
                   "e4->e3",
                   "e4->f3",
                   "e4->f4",
                   "e4->f5",
               });

    debug_case("king safety", "king cannot move onto knight attack",
               "8/8/5n2/8/4K3/8/8/8 w - - 0 1",
               PIECE_C::WHITE,
               Movegen::generate_king_moves,
               {
                   "e4->d3",
                   "e4->d4",
                   "e4->e3",
                   "e4->e5",
                   "e4->f3",
                   "e4->f4",
                   "e4->f5",
               });

    debug_case("king safety", "king cannot move onto black pawn attack from edge",
               "8/8/8/p7/8/1K6/8/8 w - - 0 1",
               PIECE_C::WHITE,
               Movegen::generate_king_moves,
               {
                   "b3->a2",
                   "b3->a3",
                   "b3->a4",
                   "b3->b2",
                   "b3->c2",
                   "b3->c3",
                   "b3->c4",
               });

    debug_case("king safety", "king cannot move next to enemy king",
               "8/8/8/4k3/8/4K3/8/8 w - - 0 1",
               PIECE_C::WHITE,
               Movegen::generate_king_moves,
               {
                   "e3->d2",
                   "e3->d3",
                   "e3->e2",
                   "e3->f2",
                   "e3->f3",
               });

    debug_case("castling", "white king side castle",
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

    debug_case("castling", "white queen side castle",
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

    debug_case("castling", "white can have both castle rights",
               "4k3/8/8/8/8/8/8/R3K2R w KQ - 0 1",
               PIECE_C::WHITE,
               Movegen::generate_king_moves,
               {
                   "e1->c1O-O-O",
                   "e1->d1",
                   "e1->d2",
                   "e1->e2",
                   "e1->f1",
                   "e1->f2",
                   "e1->g1O-O",
               });

    debug_case("castling", "white cannot castle through occupied squares",
               "4k3/8/8/8/8/8/8/R2BK1NR w KQ - 0 1",
               PIECE_C::WHITE,
               Movegen::generate_king_moves,
               {
                   "e1->d2",
                   "e1->e2",
                   "e1->f1",
                   "e1->f2",
               });

    debug_case("castling", "black king side castle",
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

    debug_case("castling", "rook generator should not create castling moves",
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

    debug_case("castling", "rook generator should ignore single castle right",
               "4k3/8/8/8/8/8/8/4K2R w K - 0 1",
               PIECE_C::WHITE,
               Movegen::generate_rook_moves,
               {
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

    debug_case("all", "all moves includes rooks and king",
               "8/8/8/8/8/8/8/R3K3 w - - 0 1",
               PIECE_C::WHITE,
               Movegen::generate_all_moves,
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

    debug_case("all", "all moves includes pawns and king captures",
               "8/8/8/8/8/8/Pp6/K7 w - - 0 1",
               PIECE_C::WHITE,
               Movegen::generate_all_moves,
               {
                   "a1->b1",
                   "a1->b2x",
                   "a2->a3",
                   "a2->a4dbl",
               });

    return 0;
}
