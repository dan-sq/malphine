#include "fen.h"
#include "board.h"
#include "transposition-table/zobrist.h"
#include <ranges>
#include <unordered_map>

enum class PARSER_STATE { POS, MV, CSTL, EP_SQ, HM, FM };

void load_fen(Position& pos, const std::string &fen) {
    pos.reset_hash();

    auto parts = fen | std::views::split(' ')
                        | std::views::transform([](auto part) {
                                return std::string(part.begin(), part.end());
                                });

    auto state = PARSER_STATE::POS;
    std::unordered_map<char, PIECE_T> symbol_to_piece_t = {
        { 'P', PIECE_T::PAWN },
        { 'N', PIECE_T::KNIGHT },
        { 'B', PIECE_T::BISHOP },
        { 'R', PIECE_T::ROOK },
        { 'Q', PIECE_T::QUEEN },
        { 'K', PIECE_T::KING },
        { 'p', PIECE_T::PAWN },
        { 'n', PIECE_T::KNIGHT },
        { 'b', PIECE_T::BISHOP },
        { 'r', PIECE_T::ROOK },
        { 'q', PIECE_T::QUEEN },
        { 'k', PIECE_T::KING },
    };

    for(auto part : parts) {
        switch(state) {
        case PARSER_STATE::POS: {
            int square = 56;
            for(char ch : part) {
                if(ch == '/') {
                    square -= 16;
                    continue;
                }
                if(std::isdigit(ch)) {
                    square += ch - '0';
                    continue;
                }
                auto piece = symbol_to_piece_t[ch];
                if(std::isupper(ch)) {
                    pos.set_piece(PIECE_C::WHITE, piece, square);
                    pos.xor_hash(Zobrist::white_keys[static_cast<int>(piece)][square]);
                } else {
                    pos.set_piece(PIECE_C::BLACK, piece, square);
                    pos.xor_hash(Zobrist::black_keys[static_cast<int>(piece)][square]);
                }

                square++;
            }

            state = PARSER_STATE::MV;
            break;
        }
        case PARSER_STATE::MV:
            if(part == "w") {
                pos.set_side(PIECE_C::WHITE);
            } else {
                pos.set_side(PIECE_C::BLACK);
                pos.xor_hash(Zobrist::side_key);
            }

            state = PARSER_STATE::CSTL;
            break;
        case PARSER_STATE::CSTL: {
            uint8_t perms = 0;
            for(char ch : part) {
                switch(ch) {
                case 'K':
                    perms |= static_cast<uint8_t>(CASTLE_PERM::WKC);
                    break;
                case 'Q':
                    perms |= static_cast<uint8_t>(CASTLE_PERM::WQC);
                    break;
                case 'k':
                    perms |= static_cast<uint8_t>(CASTLE_PERM::BKC);
                    break;
                case 'q':
                    perms |= static_cast<uint8_t>(CASTLE_PERM::BQC);
                    break;
                }
            }
            pos.set_castle(perms);
            pos.xor_hash(Zobrist::cstl_keys[perms]);

            state = PARSER_STATE::EP_SQ;
            break;
        }
        case PARSER_STATE::EP_SQ:
            if(part != "-") {
                int file = part[0] - 'a';
                int rank = part[1] - '1';
                pos.set_en_pas(rank * 8 + file);
                pos.xor_hash(Zobrist::ep_keys[file]);
            } else {
                pos.set_en_pas(64);
            }

            state = PARSER_STATE::HM;
            break;
        case PARSER_STATE::HM: {
            int hm = std::stoi(part);
            pos.set_half_moves(hm);

            state = PARSER_STATE::FM;
            break;
        }
        case PARSER_STATE::FM: {
            int fm = std::stoi(part);
            pos.set_full_moves(fm);

            return;
        }
        }
    }
}
