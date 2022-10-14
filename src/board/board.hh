#pragma once

#include <string>

#include <map>
#include <set>
#include <unordered_set>
#include <vector>

#include "log.hh"

#include "board_types.hh"

#define FEN_INIT "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"


struct move_record_t {
    int from_num;
    int to_num;
    char from_piece;
    char to_piece;
    char move_type;
    uint8_t castling_rights;
    int ep_square;
    int halfmove_clock;
    int fullmove_counter;
    std::vector<std::pair<int, int>> moves_cache;
};


class Square {
public:
    void clear() { _colour = 'e'; _piece = 'e'; }
    void set(const char c, const char p) { _colour = c; _piece = p; }
    void set_colour(const char c) { _colour = c; }
    void set_piece(const char p) { _piece = p; }
    char colour() const { return _colour; }
    char piece() const { return _piece; }
    char print() const {
        if (_colour == 'e') {
            return ' ';
        } else {
            return (_colour == 'w') ? _piece-32 : _piece;
        }
    }

private:
    char _colour = 'e';
    char _piece = 'e';
};


class Board {
public:
    Board() {
        // Square::init_arrays();
        _white_pieces.reserve(16);
        _black_pieces.reserve(16);

        _pseudolegal_move_targets.reserve(27);
        _legal_moves.reserve(218);
        setup();
    }

    void set_fen(const std::string& fen);
    int64_t perft(const int depth);
    void print(std::set<int>& highlit_squares) const;
    void print() const {
        std::set<int> hsq;
        print(hsq);
    }
    void interactive_mode();

private:
    Square _chessboard[64];
    char _to_move = '-';
    uint8_t _castling_rights;
    int _ep_square = -1;
    int _halfmove_clock = -1;
    int _fullmove_counter = -1;
    std::unordered_set<int> _white_pieces;
    std::unordered_set<int> _black_pieces;
    std::vector<move_record_t> _move_history;
    int _w_king_sq = -1;
    int _b_king_sq = -1;

    std::vector<int> _pseudolegal_move_targets;
    std::vector<std::pair<int, int>> _legal_moves;


    // board_t         chessboard;
    // piece_colour_t  player_to_move;
    // uint8_t         castling_rights;
    // sq_num_t        ep_square;
    // uint16_t        halfmove_clock;
    // uint16_t        fullmove_counter;
    // piece_map_t     white_piece_map;
    // piece_map_t     black_piece_map;
    // sq_num_vector_t pseudolegal_move_targets;
    // sq_num_vector_t legal_move_targets;
    // move_vector_t   legal_moves;
    // move_umap_t     legal_moves_map;
    // piece_id_t      white_king_id;
    // piece_id_t      black_king_id;

private:
    bool position_legal() const;
    std::string get_move_str(const int move_from, const int move_to, const char promote_to) const;
    std::string get_move_str(const int move_from, const int move_to) const;
    void clear_board();
    void setup();
    int alg_to_num(const std::string& coords_str) const;
    std::string num_to_alg(const int sq_num) const;
    void get_pseudolegal_moves_from_sq(const int sq_num);
    bool is_in_check() const;
    void show_legal_moves(const int sq_num) const;
    void show_piece_positions(const char colour) const;
    void get_legal_moves();

    void make_move(const int from_num, const int to_num, const char promote_to, const bool perft_mode);
    void make_move(const int from_num, const int to_num, const char promote_to);
    void unmake_move();

    int detect_game_end(const bool verbose) const;
    int detect_game_end() const;

    std::map<std::string, int64_t> divide(const int depth);

    void add_piece_internal(const char colour, const char piece, const int sq_num);
    std::pair<int, char> move_piece_internal(const int from_num, int to_num, const char promote_to = 'q', const bool update_lists = false);
    void unmove_piece_internal(const int from_num, int to_num, const char from_colour, const char from_piece, const char to_piece, const bool update_lists = false);
    void update_king_internal(const char colour, const int sq_num);
    void update_piece_sets_internal(const char colour, const int sq_from, const int sq_to);
};

static constexpr auto kCrudechessWelcomeString { "crudechess - interactive board" };
static constexpr auto kCrudechessHelpString {
"Available commands:\n"
"    q             - quit\n"
"    h             - print this message\n"
"    b             - show board\n"
"    f             - setup starting position\n"
"    f <FEN>       - setup position denoted by FEN\n"
"    f get         - get FEN of current position (NOT IMPLEMENTED)\n"
"    l             - print all legal moves from current position\n"
"    l <square>    - show legal moves from given square on board\n"
"    m <from> <to> - move a piece (move must be legal)\n"
"    u             - unmake last move\n"
"    p <depth>     - run perft from current position \n"
"    d <depth>     - run divide from current position \n"
"    c             - debug: is player to move in check\n"
"    s <b|w>       - debug: show piece positions"
};
