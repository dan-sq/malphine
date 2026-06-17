#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include "uci/uci.h"
#include "board/fen.h"
#include "move/move.h"
#include "movegen/movegen.h"
#include "search/search.h"

PIECE_T ch_to_piece(char ch) {
    switch(ch) {
        case 'n': return PIECE_T::KNIGHT;
        case 'b': return PIECE_T::BISHOP;
        case 'r': return PIECE_T::ROOK;
        case 'q': return PIECE_T::QUEEN;
        default: return PIECE_T::NONE;
    }
}

Move uci_to_move(Position& pos, std::string str) {
    if(str.size() < 4)
        return Move::null();
    std::vector<Move> moves;
    Movegen::generate_pseudo_legal_moves(pos, pos.get_side(), moves);
    uint8_t from_rank, from_file, to_rank, to_file;
    from_file = str[0] - 'a';
    from_rank = str[1] - '1';
    to_file = str[2] - 'a';
    to_rank = str[3] - '1';
    PIECE_T promo = str.size() == 5 ? ch_to_piece(str[4]) : PIECE_T::NONE;

    for(auto move : moves) {
        if (move.get_from() != from_rank * 8 + from_file)
            continue;
        if (move.get_to() != to_rank * 8 + to_file)
            continue;
        auto flags = move.get_flags();
        switch (static_cast<MOVE_FLAG>(flags)) {
            case MOVE_FLAG::KNIGHT_PROMO:
                if (promo != PIECE_T::KNIGHT)
                    continue;
                break;
            case MOVE_FLAG::BISHOP_PROMO:
                if (promo != PIECE_T::BISHOP)
                    continue;
                break;
            case MOVE_FLAG::ROOK_PROMO:
                if (promo != PIECE_T::ROOK)
                    continue;
                break;
            case MOVE_FLAG::QUEEN_PROMO:
                if (promo != PIECE_T::QUEEN)
                    continue;
                break;
            case MOVE_FLAG::KNIGHT_PROMO_CAPTURE:
                if (promo != PIECE_T::KNIGHT)
                    continue;
                break;
            case MOVE_FLAG::BISHOP_PROMO_CAPTURE:
                if (promo != PIECE_T::BISHOP)
                    continue;
                break;
            case MOVE_FLAG::ROOK_PROMO_CAPTURE:
                if (promo != PIECE_T::ROOK)
                    continue;
                break;
            case MOVE_FLAG::QUEEN_PROMO_CAPTURE:
                if (promo != PIECE_T::QUEEN)
                    continue;
                break;
            default:
                if(promo != PIECE_T::NONE)
                    continue;
                break;
        }

        if(!Movegen::make(pos, move))
            continue;

        Movegen::unmake(pos, move);

        return move;
    }

    return Move::null();
}

std::string move_to_uci(Move move) {
    std::string res;
    uint8_t from_sq = move.get_from();
    uint8_t to_sq = move.get_to();
    res += 'a' + (from_sq % 8);
    res += '1' + (from_sq / 8);
    res += 'a' + (to_sq % 8);
    res += '1' + (to_sq / 8);
    auto flags = move.get_flags();
    switch(static_cast<MOVE_FLAG>(flags)) {
        case MOVE_FLAG::KNIGHT_PROMO:
            res += 'n';
            break;
        case MOVE_FLAG::BISHOP_PROMO:
            res += 'b';
            break;
        case MOVE_FLAG::ROOK_PROMO:
            res += 'r';
            break;
        case MOVE_FLAG::QUEEN_PROMO:
            res += 'q';
            break;
        case MOVE_FLAG::KNIGHT_PROMO_CAPTURE:
            res += 'n';
            break;
        case MOVE_FLAG::BISHOP_PROMO_CAPTURE:
            res += 'b';
            break;
        case MOVE_FLAG::ROOK_PROMO_CAPTURE:
            res += 'r';
            break;
        case MOVE_FLAG::QUEEN_PROMO_CAPTURE:
            res += 'q';
            break;
        default:
            return res;
    }

    return res;
}

void UCI::loop() {
    Position pos;
    std::string input;

    while(std::getline(std::cin, input)) {
        std::istringstream stream(input);
        std::string cmd;
        std::string tok;
        stream >> cmd;

        if(cmd == "uci") {
            std::cout << "id name Malphine\n";
            std::cout << "id author Daniel Squair\n";
            std::cout << "uciok\n";
        } else if(cmd == "isready") {
            std::cout << "readyok\n";
        } else if(cmd == "ucinewgame") {
            pos = Position{};
            load_fen(pos, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
        } else if(cmd == "position") {
            stream >> tok;
            if(tok == "startpos") {
                pos = Position{};
                load_fen(pos, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

                if(stream >> tok && tok == "moves") {
                    std::string uci_move;
                    while(stream >> uci_move) {
                        Move engine_move = uci_to_move(pos, uci_move);
                        if (!engine_move.is_null())
                            Movegen::make(pos, engine_move);
                    }
                }
            }

            if(tok == "fen") {
                std::string fen, board, side, castle, ep, hm, fm;
                stream >> board >> side >> castle >> ep >> hm >> fm;
                fen = board + " " + side + " " + castle + " " + ep + " " + hm + " " + fm;
                pos = Position{};
                load_fen(pos, fen);

                if(stream >> tok && tok == "moves") {
                    std::string uci_move;
                    while(stream >> uci_move) {
                        Move engine_move = uci_to_move(pos, uci_move);
                        if (!engine_move.is_null())
                            Movegen::make(pos, engine_move);
                    }
                }
            }
        } else if (cmd == "go") {
            int depth = 3;
            while (stream >> tok) {
                if (tok == "depth") {
                    stream >> depth;
                    break;
                }
            }

            Move best = Search::search(pos, depth);
            std::cout << "bestmove " << move_to_uci(best) << "\n";
        } else if (cmd == "quit") {
            break;
        }
    }
}
