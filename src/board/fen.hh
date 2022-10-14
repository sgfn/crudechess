#pragma once

#include <string>

#include "board_types.hh"

namespace Fen {
    static constexpr auto kFenInitial { "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1" };
    static constexpr auto kFenRegex { "^([pbnrqkPBNRQK1-8]+/){7}[pbnrqkPBNRQK1-8]+ (w|b) (-|[KQkq]+) (-|[a-h]{1}[36]{1})( (0|[1-9][0-9]*) [1-9][0-9]*)?$" };
    enum fen_field_t {
        kPiecePositions = 1,
        kPlayerToMove,
        kCastlingRights,
        kEpSquare,
        kHalfmoveClock,
        kFullmoveCounter,
    };

    inline sq_num_t get_sq_num_fen_unsafe(const int fen_row, const int fen_col) {
        return ((7-fen_row)<<3) + fen_col;
    }

    bool fen_valid(const std::string& fen);
}
