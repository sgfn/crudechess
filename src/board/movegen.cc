#include "board.hh"


void Board::get_pseudolegal_moves_from_sq(const int sq_num) {
    const auto& from_sq = _chessboard[sq_num];
    const int from_row = sq_num / 8;
    const int from_col = sq_num % 8;

    _pseudolegal_move_targets.clear();
    if (from_sq.colour() != _to_move) {
        return;
    }

    std::vector<std::pair<int, int>> all_moves;


    if (from_sq.piece() == 'p') {
        const int start_row = from_sq.colour() == 'w' ? 1 : 6;
        const int pawn_move = from_sq.colour() == 'w' ? 1 : -1;
        // std move
        all_moves.push_back(std::make_pair(pawn_move, 0));
        // first pawn move
        if (from_row == start_row) {
            all_moves.push_back(std::make_pair(2*pawn_move, 0));
        }
        // captures
        std::vector<std::pair<int, int>> all_captures { { pawn_move, -1 }, { pawn_move, 1 } };

        for (const auto [mv_row, mv_col] : all_moves) {
            int dest_row = from_row + mv_row;
            int dest_col = from_col + mv_col;
            // -Wmaybe-redundant
            if (0 <= dest_row && dest_row <= 7 && 0 <= dest_col && dest_col <= 7) {
                const int dest_num = dest_row*8 + dest_col;
                const auto& to_sq = _chessboard[dest_num];
                // cannot advance onto occupied
                if (to_sq.colour() != 'e') {
                    break;
                }
                _pseudolegal_move_targets.push_back(dest_num);
            }
        }

        for (const auto [mv_row, mv_col] : all_captures) {
            int dest_row = from_row + mv_row;
            int dest_col = from_col + mv_col;
            // -Wmaybe-redundant
            if (0 <= dest_row && dest_row <= 7 && 0 <= dest_col && dest_col <= 7) {
                const int dest_num = dest_row*8 + dest_col;
                const auto& to_sq = _chessboard[dest_num];
                // standard capture, en passant
                if ((to_sq.colour() != 'e' && to_sq.colour() != from_sq.colour()) ||
                    (to_sq.colour() == 'e' && _ep_square == dest_num)) {
                    _pseudolegal_move_targets.push_back(dest_num);
                }
            }
        }
    } else if (from_sq.piece() == 'k' || from_sq.piece() == 'n') {
        if (from_sq.piece() == 'k') {
            all_moves = { {1, 1}, {1, 0}, {1, -1}, {0, 1}, {0, -1}, {-1, 1}, {-1, 0}, {-1, -1} };

            // castling
            int tmp_iic_val = -1;
            int cs_kingside_mask = (from_sq.colour() == 'w') ? 8 : 2;
            auto& b = _chessboard;
            // kingside
            if (_castling_rights & cs_kingside_mask) {
                if (b[sq_num+1].colour() == 'e' && b[sq_num+2].colour() == 'e') {
                    // test whether king in check
                    tmp_iic_val = is_in_check();
                    if (tmp_iic_val == 0) {
                        _pseudolegal_move_targets.push_back(sq_num + 2);
                    }
                }
            }
            // queenside
            if (_castling_rights & (cs_kingside_mask>>1)) {
                if (b[sq_num-1].colour() == 'e' && b[sq_num-2].colour() == 'e' && b[sq_num-3].colour() == 'e') {
                    // test whether king in check
                    if (tmp_iic_val == -1) {
                        tmp_iic_val = is_in_check();
                    }
                    if (tmp_iic_val == 0) {
                        _pseudolegal_move_targets.push_back(sq_num - 2);
                    }
                }
            }
        } else {
            all_moves = { {1, 2}, {1, -2}, {-1, 2}, {-1, -2}, {2, 1}, {2, -1}, {-2, 1}, {-2, -1} };
        }

        for (const auto [mv_row, mv_col] : all_moves) {
            int dest_row = from_row + mv_row;
            int dest_col = from_col + mv_col;
            if (0 <= dest_row && dest_row <= 7 && 0 <= dest_col && dest_col <= 7) {
                const int dest_num = dest_row*8 + dest_col;
                const auto& to_sq = _chessboard[dest_num];
                if (to_sq.colour() != from_sq.colour()) {
                    _pseudolegal_move_targets.push_back(dest_num);
                }
            }
        }
    } else {
        if (from_sq.piece() != 'r') {
            all_moves.push_back(std::make_pair(-1, -1));
            all_moves.push_back(std::make_pair(-1, 1));
            all_moves.push_back(std::make_pair(1, -1));
            all_moves.push_back(std::make_pair(1, 1));
        }
        if (from_sq.piece() != 'b') {
            all_moves.push_back(std::make_pair(-1, 0));
            all_moves.push_back(std::make_pair(0, -1));
            all_moves.push_back(std::make_pair(1, 0));
            all_moves.push_back(std::make_pair(0, 1));
        }

        for (const auto [mv_row, mv_col] : all_moves) {
            int dest_row = from_row + mv_row;
            int dest_col = from_col + mv_col;
            while (0 <= dest_row && dest_row <= 7 && 0 <= dest_col && dest_col <= 7) {
                const int dest_num = dest_row*8 + dest_col;
                const auto& to_sq = _chessboard[dest_num];
                if (to_sq.colour() == 'e') {
                    _pseudolegal_move_targets.push_back(dest_num);
                } else {
                    if (to_sq.colour() != from_sq.colour()) {
                        _pseudolegal_move_targets.push_back(dest_num);
                    }
                    break;
                }
                dest_row += mv_row;
                dest_col += mv_col;
            }
        }
    }
}


void Board::get_legal_moves() {
    _legal_moves.clear();
    const auto& piece_set = (_to_move == 'w') ? _white_pieces : _black_pieces;
    for (const auto from_num : piece_set) {
        const auto& from_sq = _chessboard[from_num];
        const char from_clr = from_sq.colour();
        const char from_piece = from_sq.piece();

        // printf("get legal moves from square %s\n    board before:\n", num_to_alg(from_num).c_str());
        // this->print();
        get_pseudolegal_moves_from_sq(from_num);
        // printf("    plegal targets: ");
        // for (const auto mv : pseudolegal_moves) {
            // printf("%s, ", num_to_alg(mv).c_str());
        // }
        // printf("\n");

        for (const auto to_num : _pseudolegal_move_targets) {
            const auto& to_sq = _chessboard[to_num];
            const char to_piece = to_sq.piece();

            // castling - checking king's passthrough square
            if (from_piece == 'k' && std::abs(to_num - from_num) == 2) {
                // determine side
                int cs_dir = to_num > from_num ? 1 : -1;

                move_piece_internal(from_num, from_num + cs_dir);
                // illegal if king in check
                if (is_in_check()) {
                    move_piece_internal(from_num + cs_dir, from_num);
                    continue;
                }
                // unmove the piece, then proceed as normal
                move_piece_internal(from_num + cs_dir, from_num);
            }
            move_piece_internal(from_num, to_num);
            if (!is_in_check()) {
                _legal_moves.push_back(std::make_pair(from_num, to_num));
            }
            unmove_piece_internal(from_num, to_num, from_clr, from_piece, to_piece);
        }

        // printf("    board after:\n");
        // this->print();
    }
}
