#include <iostream>

#include "board.hh"

#define CLR_ESC "\x1b[0m"
#define CLR_L_B "\x1b[0;30;46m"
#define CLR_L_W "\x1b[0;37;46m"
#define CLR_D_B "\x1b[0;30;44m"
#define CLR_D_W "\x1b[0;37;44m"
#define CLR_H_B "\x1b[0;30;45m"
#define CLR_H_W "\x1b[0;37;45m"


void Board::print(std::set<int>& highlit_squares) const {
    std::string s = "";
    bool sq_light = true;
    for (int sq_row = 7; sq_row > -1; sq_row--) {
        for (int sq_col = 0; sq_col < 8; sq_col++) {
            std::string clr;
            const auto& sq = _chessboard[sq_row*8 + sq_col];
            if (highlit_squares.count(sq_row*8+sq_col)) {
                clr = (sq.colour() == 'w') ? CLR_H_W : CLR_H_B;
            } else if (sq_light) {
                clr = (sq.colour() == 'w') ? CLR_L_W : CLR_L_B;
            } else {
                clr = (sq.colour() == 'w') ? CLR_D_W : CLR_D_B;
            }
            s.append(clr + sq.print() + ' ' + CLR_ESC);
            sq_light = !sq_light;
        }
        s.append(std::to_string(sq_row + 1) + '\n');
        sq_light = !sq_light;
    }
    s.append(" A B C D E F G H\n");
    std::cout << s;
}


void Board::show_legal_moves(const int sq_num) const {
    std::set<int> sq_set;
    for (const auto [fr, to] : _legal_moves) {
        if (fr == sq_num) {
            sq_set.insert(to);
        }
    }
    print(sq_set);
}


void Board::show_piece_positions(const char colour) const {
    std::set<int> sq_set;
    const auto& piece_set = (colour == 'w') ? _white_pieces : _black_pieces;
    for (const auto sq_num : piece_set) {
        sq_set.insert(sq_num);
    }
    print(sq_set);
}
