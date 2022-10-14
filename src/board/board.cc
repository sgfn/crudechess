#include <cstdio>

#include <chrono>
#include <fstream>
#include <iostream>

#include <vector>
#include <map>
#include <set>

#include <string>
#include <sstream>

#include "strfuns.hh"

#include "board.hh"
#include "fen.hh"


void Board::setup() {
    const std::string fen { FEN_INIT };
    set_fen(fen);
}

int Board::alg_to_num(const std::string& coords_str) const {
    if (coords_str.length() != 2) {
        return -1;
    }
    const char rank_ch = coords_str[1];
    const char file_ch = coords_str[0] < 97 ? coords_str[0]+32 : coords_str[0];
    const int rank = std::atoi(&rank_ch);
    const int file = file_ch - 97;
    return (rank - 1) * 8 + file;
}

std::string Board::num_to_alg(const int sq_num) const {
    if (sq_num == -1) {
        return "-";
    }
    const char rank = sq_num / 8 + 1 + 48;
    const char file = sq_num % 8 + 97;
    std::string s;
    s.push_back(file);
    s.push_back(rank);
    return s;
}

void Board::clear_board() {
    for (auto& sq : _chessboard) {
        sq.clear();
    }

    _to_move = '-';
    _castling_rights = 0;
    _ep_square = -1;
    _halfmove_clock = 0;
    _fullmove_counter = 1;

    _white_pieces.clear();
    _black_pieces.clear();

    _w_king_sq = -1;
    _b_king_sq = -1;

    _pseudolegal_move_targets.clear();
    _legal_moves.clear();
}

bool Board::position_legal() const {
    return true;
}

void Board::set_fen(const std::string& fen) {
    const std::string fen_stripped = Strfuns::strip_copy(fen);
    if (!Fen::fen_valid(fen_stripped)) {
        LOG_WARNING("Invalid FEN: %s", fen_stripped.c_str());
        return;
    }

    clear_board();

    std::stringstream fen_stream(fen);
    std::string field;
    int field_num = 0;
    while (std::getline(fen_stream, field, ' ')) {
        ++field_num;
        if (field_num == Fen::kPiecePositions) {
            int row = 0;
            int col = 0;
            for (auto ch : field) {
                if (ch == '/') {
                    col = 0;
                    ++row;
                } else if (ch >= 49 && ch <= 56) { // ch is a number
                    col += ch-48;
                } else { // ch is a letter
                    bool white = (ch >= 65 && ch <= 90);
                    if (white) {
                        ch += 32;
                    }

                    int sq_num = (7-row)*8+col;
                    _chessboard[sq_num].set(white ? 'w' : 'b', ch);
                    if (ch == 'k') {
                        update_king_internal(white ? 'w' : 'b', sq_num);
                    }

                    update_piece_sets_internal(white ? 'w' : 'b', -1, sq_num);
                    ++col;
                }
            }
        } else if (field_num == Fen::kPlayerToMove) {
            _to_move = field.at(0);
        } else if (field_num == Fen::kCastlingRights) {
            for (const auto ch : field) {
                switch (ch) {
                    case 'K':   _castling_rights += 8; break;
                    case 'Q':   _castling_rights += 4; break;
                    case 'k':   _castling_rights += 2; break;
                    case 'q':   _castling_rights += 1; break;
                }
            }
        } else if (field_num == Fen::kEpSquare) {
            if (field.at(0) != '-') {
                _ep_square = alg_to_num(field);
            } else {
                _ep_square = -1;
            }
        } else if (field_num == Fen::kHalfmoveClock) {
            _halfmove_clock = std::stoi(field);
        } else if (field_num == Fen::kFullmoveCounter) {
            _fullmove_counter = std::stoi(field);
        }
    }

    // this->print();

    // if (!position_legal()) {
    //     std::cerr << "Illegal position" << std::endl;
    //     clear_board();
    //     return;
    // }

    get_legal_moves();

    detect_game_end();
}


bool Board::is_in_check() const {
    const char k_colour = _to_move;
    const int k_row = (k_colour == 'w') ? _w_king_sq / 8 : _b_king_sq / 8;
    const int k_col = (k_colour == 'w') ? _w_king_sq % 8 : _b_king_sq % 8;

    // pawn checks
    int pawn_move = (k_colour == 'w') ? 1 : -1;
    std::vector<std::pair<int, int>> pawn_moves { {pawn_move, -1}, {pawn_move, 1} };
    for (const auto [mv_row, mv_col] : pawn_moves) {
        int atk_row = k_row + mv_row;
        int atk_col = k_col + mv_col;
        if (0 <= atk_row && atk_row <= 7 && 0 <= atk_col && atk_col <= 7) {
            const auto& atk_sq = _chessboard[atk_row*8 + atk_col];
            if (atk_sq.colour() != k_colour && atk_sq.piece() == 'p') {
                return true;
            }
        }
    }

    // king checks
    std::vector<std::pair<int, int>> king_moves { {1, 1}, {1, 0}, {1, -1}, {0, 1}, {0, -1}, {-1, 1}, {-1, 0}, {-1, -1} };
    for (const auto [mv_row, mv_col] : king_moves) {
        int atk_row = k_row + mv_row;
        int atk_col = k_col + mv_col;
        if (0 <= atk_row && atk_row <= 7 && 0 <= atk_col && atk_col <= 7) {
            const auto& atk_sq = _chessboard[atk_row*8 + atk_col];
            if (atk_sq.colour() != k_colour && atk_sq.piece() == 'k') {
                return true;
            }
        }
    }

    // knight checks
    std::vector<std::pair<int, int>> knight_moves { {1, 2}, {1, -2}, {-1, 2}, {-1, -2}, {2, 1}, {2, -1}, {-2, 1}, {-2, -1} };
    for (const auto [mv_row, mv_col] : knight_moves) {
        int atk_row = k_row + mv_row;
        int atk_col = k_col + mv_col;
        if (0 <= atk_row && atk_row <= 7 && 0 <= atk_col && atk_col <= 7) {
            const auto& atk_sq = _chessboard[atk_row*8 + atk_col];
            if (atk_sq.colour() != k_colour && atk_sq.piece() == 'n') {
                return true;
            }
        }
    }

    // ray piece checks
    const std::string brq = "brq";
    std::vector<std::tuple<int, int, char>> brq_moves { {-1, -1, 'r'}, {-1, 1, 'r'}, {1, -1, 'r'}, {1, 1, 'r'}, {-1, 0, 'b'}, {0, -1, 'b'}, {1, 0, 'b'}, {0, 1, 'b'} };
    for (const auto [mv_row, mv_col, notp_ch] : brq_moves) {
        int atk_row = k_row + mv_row;
        int atk_col = k_col + mv_col;
        while (0 <= atk_row && atk_row <= 7 && 0 <= atk_col && atk_col <= 7) {
            const auto& atk_sq = _chessboard[atk_row*8 + atk_col];
            if (atk_sq.colour() != k_colour && brq.find(atk_sq.piece()) != std::string::npos && atk_sq.piece() != notp_ch) {
                return true;
            } else if (atk_sq.colour() != 'e') {
                break;
            }
            atk_row += mv_row;
            atk_col += mv_col;
        }
    }

    return false;
}


void Board::make_move(const int from_num, const int to_num, const char promote_to, const bool perft_mode) {
    if (!perft_mode) {
        bool legal = false;
        for (const auto& [from_mv, to_mv] : _legal_moves) {
            if (from_mv == from_num && to_mv == to_num) {
                legal = true;
                break;
            }
        }
        if (!legal) {
            std::cout << "Illegal move\n"; // add more data
            return;
        }
    }

    // store board properties
    const uint8_t cs_rt = _castling_rights;
    const int ep_sq = _ep_square;
    const int hm_cl = _halfmove_clock;
    const int fm_ct = _fullmove_counter;

    auto& from_sq = _chessboard[from_num];
    auto& to_sq = _chessboard[to_num];
    const char from_colour = from_sq.colour();
    const char from_piece = from_sq.piece();
    const char to_piece = to_sq.piece();

    // detecting loss of castling rights
    int mask = (from_sq.colour() == 'w') ? 3 : 12;
    int mask_diff = (from_sq.colour() == 'w') ? 4 : 1;
    // king has moved
    if ((from_num == 4 || from_num == 60) && from_piece == 'k') {
        _castling_rights &= mask;
    }
    // rook has moved
    else if ((from_num == 0 || from_num == 7 || from_num == 56 || from_num == 63) && from_piece == 'r') {
        if ((from_num == 7 && from_colour == 'w') || (from_num == 63 && from_colour == 'b')) {
            _castling_rights &= mask + mask_diff;
        } else if ((from_num == 0 && from_colour == 'w') || (from_num == 56 && from_colour == 'b')) {
            _castling_rights &= mask + (mask_diff<<1);
        }
    }
    // rook was captured
    else if ((to_num == 0 || to_num == 7 || to_num == 56 || to_num == 63) && to_piece == 'r') {
        // capturing rook removes rights for opponent
        mask = 15 - mask;
        mask_diff = 5 - mask_diff;
        if ((to_num == 7 && from_colour == 'b') || (to_num == 63 && from_colour == 'w')) {
            _castling_rights &= mask + mask_diff;
        } else if ((to_num == 0 && from_colour == 'b') || (to_num == 56 && from_colour == 'w')) {
            _castling_rights &= mask + (mask_diff<<1);
        }
    }

    // move and check whether to reset halfmove clock
    const auto rval = move_piece_internal(from_num, to_num, promote_to, true);
    if (rval.first > 0) {
        _halfmove_clock = 0;
    } else {
        _halfmove_clock += 1;
    }

    if (_to_move == 'b') {
        _fullmove_counter += 1;
    }

    _to_move = (_to_move == 'w') ? 'b' : 'w';

    // update list of previous moves
    move_record_t move_data {from_num, to_num, from_piece, to_piece, rval.second, cs_rt, ep_sq, hm_cl, fm_ct, _legal_moves};
    _move_history.push_back(move_data);

    // detect ep in next ply
    if (from_piece == 'p') {
        if (to_num - from_num == 16) {
            _ep_square = to_num - 8;
        } else if (to_num - from_num == -16) {
            _ep_square = to_num + 8;
        } else {
            _ep_square = -1;
        }
    } else {
        _ep_square = -1;
    }

    // update legal moves
    get_legal_moves();

    // detect, handle end
    if (!perft_mode) {
        detect_game_end();
    }
}

void Board::make_move(const int from_num, const int to_num, const char promote_to) {
    make_move(from_num, to_num, promote_to, false);
}

void Board::unmake_move() {
    if (_move_history.size() == 0) {
        std::cout << "Nothing to unmake\n";
        return;
    }
    // unpack move data
    const auto move_data = _move_history[_move_history.size()-1];
    _move_history.pop_back();

    // reinstate board properties
    _castling_rights = move_data.castling_rights;
    _ep_square = move_data.ep_square;
    _halfmove_clock = move_data.halfmove_clock;
    _fullmove_counter = move_data.fullmove_counter;

    const char has_moved = _to_move == 'b' ? 'w' : 'b';
    // unmake the move
    unmove_piece_internal(move_data.from_num, move_data.to_num, has_moved, move_data.from_piece, move_data.to_piece, true);

    _to_move = has_moved;
    _legal_moves = move_data.moves_cache;
}

int Board::detect_game_end(const bool verbose) const {
    if (_legal_moves.size() == 0) {
        if (is_in_check()) {
            if (verbose) {
                printf("Checkmate. %s wins", _to_move == 'b' ? "White" : "Black");
            }
            return 1;
        } else {
            if (verbose) {
                printf("Stalemate");
            }
            return 2;
        }
    }
    return 0;
}

int Board::detect_game_end() const {
    return detect_game_end(false);
}

int64_t Board::perft(const int depth) {
    const char promotion_targets[4] {'q', 'r', 'b', 'n'};

    if (depth < 0) {
        return -1;
    }
    if (depth == 0) {
        return 1;
    }
    if (depth == 1) {
        int64_t counter = 0;
        for (const auto [move_from, move_to] : _legal_moves) {
            const auto& from_sq = _chessboard[move_from];
            const char from_colour = from_sq.colour();
            const int move_rank = move_from / 8;
            if (from_sq.piece() == 'p' && ((move_rank == 6 && from_colour == 'w') || (move_rank == 1 && from_colour == 'b'))) {
                counter += 3;
            }
            counter += 1;
        }
        return counter;
    }

    int64_t leaf_nodes = 0;
    std::vector<std::pair<int, int>> legals { _legal_moves };
    for (const auto [move_from, move_to] : legals) {
        if (_chessboard[move_from].piece() == 'p' && (move_to / 8 == 0 || move_to / 8 == 7)) {
            for (const auto promote_to : promotion_targets) {
                make_move(move_from, move_to, promote_to, true);
                leaf_nodes += perft(depth-1);
                unmake_move();
            }
        } else {
            make_move(move_from, move_to, 'q', true);
            leaf_nodes += perft(depth-1);
            unmake_move();
        }
    }

    return leaf_nodes;
}

std::string Board::get_move_str(const int move_from, const int move_to) const {
    std::string s = num_to_alg(move_from) + num_to_alg(move_to);
    return s;
}

std::string Board::get_move_str(const int move_from, const int move_to, const char promote_to) const {
    std::string s = get_move_str(move_from, move_to);
    s.push_back(promote_to - 32);
    return s;
}

std::map<std::string, int64_t> Board::divide(const int depth) {
    const char promotion_targets[4] {'q', 'r', 'b', 'n'};
    std::map<std::string, int64_t> leaf_nodes_dict;
    if (depth < 2) {
        perft(depth);
    } else {
        std::vector<std::pair<int, int>> legals { _legal_moves };
        for (const auto [move_from, move_to] : legals) {
            if (_chessboard[move_from].piece() == 'p' && (move_to / 8 == 0 || move_to / 8 == 7)) {
                for (const auto promote_to : promotion_targets) {
                    make_move(move_from, move_to, promote_to, true);
                    leaf_nodes_dict[get_move_str(move_from, move_to, promote_to)] = perft(depth-1);
                    unmake_move();
                }
            } else {
                make_move(move_from, move_to, 'q', true);
                leaf_nodes_dict[get_move_str(move_from, move_to)] = perft(depth-1);
                unmake_move();
            }
        }
    }
    return leaf_nodes_dict;
}

void Board::interactive_mode() {
    std::cout << kCrudechessWelcomeString << std::endl;
    bool active = true;
    std::string input, cmd, args;
    size_t sep_pos;
    while (active) {
        std::cout << "> ";
        std::getline(std::cin, input);
        sep_pos = input.find(' ');
        cmd = input.substr(0, sep_pos);
        if (sep_pos == std::string::npos) {
            args = "";
        } else {
            args = input.substr(sep_pos+1, std::string::npos);
        }
        if (cmd=="q" || cmd=="qqq" || cmd=="quit" || cmd=="exit") {
            active = false;
        }
        else if (cmd=="h" || cmd=="help") {
            std::cout << kCrudechessHelpString << std::endl;
        }
        else if (cmd=="b" || cmd=="board") {
            this->print();
        }
        else if (cmd=="c" || cmd=="iic" || cmd=="check") {
            std::cout << (is_in_check() ? "In check" : "Not in check") << std::endl;
        }
        else if (cmd=="f" || cmd=="fen") {
            if (input.substr(sep_pos+1, 3) == "get") {
                // std::cout << get_fen() << std::endl;
                // todo: add get_fen()
            }
            else {
                if (sep_pos == std::string::npos) {
                    std::string finit = FEN_INIT;
                    set_fen(finit);
                } else {
                    set_fen(args);
                }
            }
        }
        else if (cmd=="l" || cmd=="slm" || cmd=="legal") {
            if (args.size()) {
                show_legal_moves(alg_to_num(args));
            } else {
                for (const auto& [from_move, to_move] : _legal_moves) {
                    printf("%s%s, ", num_to_alg(from_move).c_str(), num_to_alg(to_move).c_str());
                }
                printf("\n");
            }
        }
        else if (cmd=="s" || cmd=="spp" || cmd=="pieces") {
            show_piece_positions(args[0]);
        }
        else if (cmd=="m" || cmd=="mv" || cmd=="move") {
            const int from_num = alg_to_num(args.substr(0, 2));
            const int to_num = alg_to_num(args.substr(3, 2));
            const char promote_to = (args.size() > 5) ? args[6] : 'q';
            make_move(from_num, to_num, promote_to);
        }
        else if (cmd=="u" || cmd=="um" || cmd=="undo" || cmd=="unmove") {
            unmake_move();
        }
        else if (cmd=="p" || cmd=="perft") {
            auto s_tm = std::chrono::high_resolution_clock::now();
            const int nodes = this->perft(std::stoi(args));
            auto e_tm = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::milli> t_tm = e_tm-s_tm;
            std::cout << "Nodes: " << nodes << " \tTime: " << t_tm.count();
            std::cout << " ms" << std::endl;
        }
        else if (cmd=="d" || cmd=="divide") {
            auto s_tm = std::chrono::high_resolution_clock::now();
            const auto nodes_dict = divide(std::stoi(args));
            auto e_tm = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::milli> t_tm = e_tm-s_tm;
            for (const auto& [move, count] : nodes_dict) {
                printf("%s: %ld\n", move.c_str(), count);
            }
            std::cout << "Time: " << t_tm.count() << " ms" << std::endl;
        }
        else {
            std::cout << "Unknown command: `" << cmd << "'" << std::endl;
        }
    }
}

void Board::add_piece_internal(const char colour, const char piece, const int sq_num) {
    auto& to_sq = _chessboard[sq_num];
    if (piece == 'k') {
        update_king_internal(colour, sq_num);
    }

    to_sq.set(colour, piece);
}

std::pair<int, char> Board::move_piece_internal(const int from_num, int to_num, const char promote_to, const bool update_lists) {
    std::pair<int, char> rval {0, 's'};
    auto& from_sq = _chessboard[from_num];
    const char from_colour = from_sq.colour();
    const char their_colour = from_colour == 'w' ? 'b' : 'w';

    if (from_colour == 'e') {
        std::cout << "Square empty\n"; // more data?
        return rval;
    }

    if (from_sq.piece() == 'k') {
        update_king_internal(from_colour, to_num);
        // castling
        if ((from_num == 4 || from_num == 60) && std::abs(to_num - from_num) == 2) {
            rval.second = 'c';
            const int rook_from = (to_num > from_num) ? from_num + 3 : from_num - 4;
            const int rook_to = (to_num > from_num) ? from_num + 1 : from_num - 1;
            _chessboard[rook_to].set(from_colour, 'r');
            _chessboard[rook_from].clear();

            if (update_lists) {
                update_piece_sets_internal(from_colour, rook_from, rook_to);
            }
        }
    }

    // standard capture
    if (to_num != -1) {
        if (_chessboard[to_num].colour() == their_colour) {
            rval.first = 2;
            if (update_lists) {
                update_piece_sets_internal(their_colour, to_num, -1);
            }
        }
    }

    if (from_sq.piece() == 'p') {
        // promotions
        const int pr_row = from_colour == 'w' ? 7 : 0;
        if (to_num / 8 == pr_row) {
            rval.second = promote_to;
            add_piece_internal(from_colour, promote_to, to_num);
            if (update_lists) {
                update_piece_sets_internal(from_colour, from_num, to_num);
            }
            to_num = -1;
        }
        // en passant
        else if (to_num == _ep_square) {
            rval.second = 'e';
            const int ep_pawn_sq = to_num > from_num ? to_num - 8 : to_num + 8;
            _chessboard[ep_pawn_sq].clear();
            if (update_lists) {
                update_piece_sets_internal(their_colour, ep_pawn_sq, -1);
            }
        }

        rval.first = 1;
    }

    // actually move the piece
    if (to_num != -1) {
        _chessboard[to_num].set(from_colour, from_sq.piece());
    }
    from_sq.clear();

    if (update_lists) {
        if (rval.second != 'q' && rval.second != 'r' && rval.second != 'b' && rval.second != 'n') {
            update_piece_sets_internal(from_colour, from_num, to_num);
        }
    }

    return rval;
}

void Board::unmove_piece_internal(const int from_num, const int to_num, const char from_colour, const char from_piece, const char to_piece, const bool update_lists) {
    auto& from_sq = _chessboard[from_num];
    auto& to_sq = _chessboard[to_num];

    const char their_colour = from_colour == 'b' ? 'w' : 'b';

    // std capture
    if (to_piece != 'e') {
        from_sq.set(from_colour, from_piece);
        to_sq.set(their_colour, to_piece);
        if (update_lists) {
            update_piece_sets_internal(their_colour, -1, to_num);
        }
    }
    // en passant or not a capture (maybe castling)
    else {
        // ep
        if (from_piece == 'p' && to_num == _ep_square) {
            const int ep_pawn_sq = to_num > from_num ? to_num - 8 : to_num + 8;
            _chessboard[ep_pawn_sq].set(their_colour, 'p');
            if (update_lists) {
                update_piece_sets_internal(their_colour, -1, ep_pawn_sq);
            }
        }
        // castling
        else if (from_piece == 'k' && std::abs(to_num - from_num) == 2) {
            const int rook_from = (to_num > from_num) ? from_num + 3 : from_num - 4;
            const int rook_to = (to_num > from_num) ? from_num + 1 : from_num - 1;

            _chessboard[rook_to].clear();
            _chessboard[rook_from].set(from_colour, 'r');
            if (update_lists) {
                update_piece_sets_internal(from_colour, rook_to, rook_from);
            }
        }

        move_piece_internal(to_num, from_num, 'q', false);
    }
    if (update_lists) {
        update_piece_sets_internal(from_colour, to_num, from_num);
    }

    // cleanup after unmaking a promotion
    if (from_piece != from_sq.piece()) {
        from_sq.set_piece(from_piece);
    }

    // upd king
    if (from_piece == 'k') {
        update_king_internal(from_colour, from_num);
    }
}

void Board::update_king_internal(const char colour, const int sq_num) {
    if (colour == 'b') {
        _b_king_sq = sq_num;
    } else {
        _w_king_sq = sq_num;
    }
}

void Board::update_piece_sets_internal(const char colour, const int sq_from, const int sq_to) {
    auto& p_set = colour == 'b' ? _black_pieces : _white_pieces;

    if (sq_to != -1) {
        p_set.insert(sq_to);
    }
    if (sq_from != -1) {
        p_set.erase(sq_from);
    }
}

void load_and_run_tests(const std::string& test_file_path, const int max_depth) {
    Board b;
    std::ifstream file;
    file.open(test_file_path);
    std::string line;
    printf("Testing at depth %d\n", max_depth);
    printf("No.      FEN                               Result    Passed    Delta      Time\n");
    printf("-----    ------------------------------    ------    ------    -------    ---------\n");
    int fail = 0;
    int pass = 0;
    int test_no = 0;
    bool failed = false;
    int64_t result = 0;
    int64_t expected = 0;
    const auto start_time = std::chrono::high_resolution_clock::now();
    std::chrono::system_clock::time_point s_tm, e_tm;
    std::chrono::duration<double, std::milli> t_tm;
    while (std::getline(file, line)) {
        std::stringstream linestream(line);
        std::string field;
        int i = 0;
        failed = false;
        while (std::getline(linestream, field, ',')) {
            if (field[0] == '#') {
                break;
            }
            if (i == 0) {
                s_tm = std::chrono::high_resolution_clock::now();
                b.set_fen(field);
                std::string fen_output_string = "";
                if (field.size() > 30) {
                    fen_output_string = field.substr(0, 12) + "[...]" + field.substr(field.size()-13);
                } else {
                    fen_output_string = field;
                }
                printf("%5d    %-30s    ", ++test_no, fen_output_string.c_str());
            } else {
                result = b.perft(i);
                expected = std::stoi(field);
                if (result == expected) {
                    printf(".");
                    fflush(stdout);
                } else {
                    failed = true;
                    printf("F");
                }
            }
            if (failed || i == max_depth) {
                e_tm = std::chrono::high_resolution_clock::now();
                t_tm = e_tm-s_tm;
                const std::string padding(6-i, ' ');
                const std::string delta_str = ((result > expected) ? "+" : "") + std::to_string(result-expected);
                printf("%s    %d/%d       %-7s    %.2lf ms\n", padding.c_str(), failed ? i-1 : i, max_depth, delta_str.c_str(), t_tm.count());
                if (failed) {
                    ++fail;
                    // printf("[ FAIL ] Test %d at depth %d - expected %d, got %d (delta: %s%d)\n", test_no, i, expected, result, result-expected>0 ? "+" : "", result-expected);
                } else {
                    ++pass;
                }
                break;
            }
            ++i;
        }
    }
    e_tm = std::chrono::high_resolution_clock::now();
    t_tm = e_tm-start_time;
    printf("\n%d/%d tests passed (time: %.2lf ms)\n", pass, test_no, t_tm.count());
    file.close();
}

#ifndef GTEST_UT
int main(int argc, char* argv[]) {
    if (argc > 2) {
        load_and_run_tests(argv[1], atoi(argv[2]));
    } else {
        Board board;
        board.interactive_mode();
    }
    return 0;
}
#endif
