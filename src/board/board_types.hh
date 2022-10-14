#pragma once

#include <map>
#include <unordered_map>
#include <unordered_set>
#include <vector>

// Piece types
// Every square has a value coded as such:
// CLR  RAY  RLIKE  BLIKE  OTHER  |  SUM  MEANING
// ---  ---  -----  -----  -----  |  ---  -------
//   0    0      0      0      0  |    0  empty square
//   0    0      0      0      1  |    1  black pawn
//   0    0      0      1      1  |    3  black knight
//   0    0      1      1      0  |    6  black king
//   0    1      0      1      0  |   10  black bishop
//   0    1      1      0      0  |   12  black rook
//   0    1      1      1      0  |   14  black queen
//   1    0      0      0      1  |   17  white pawn
//   1    0      0      1      1  |   19  white knight
//   1    0      1      1      0  |   22  white king
//   1    1      0      1      0  |   26  white bishop
//   1    1      1      0      0  |   28  white rook
//   1    1      1      1      0  |   30  white queen
enum piece_type_t {
    kPieceNone   = 0b00000,
    kPiecePawn   = 0b00001,
    kPieceKnight = 0b00011,
    kPieceKing   = 0b00110,
    kPieceBishop = 0b01010,
    kPieceRook   = 0b01100,
    kPieceQueen  = 0b01110
};

using square_val_t = uint8_t;
static constexpr square_val_t kSquareWhitePieceBit { 0b10000 };
static constexpr square_val_t kSquareRayPieceBit { 0b01000 };

enum move_type_t {
    kMoveUp = 0,
    kMoveLeft,
    kMoveDown,
    kMoveRight,
    kMoveUpLeft,
    kMoveUpRight,
    kMoveDownLeft,
    kMoveDownRight,
    kMoveUpLeftKnight,
    kMoveUpRightKnight,
    kMoveDownLeftKnight,
    kMoveDownRightKnight,
    kMoveLeftUpKnight,
    kMoveLeftDownKnight,
    kMoveRightUpKnight,
    kMoveRightDownKnight,
    // Always at the end
    kMoveTypeCount
};


using board_t         = std::array<square_val_t, 64>;

using sq_num_t        = int8_t;
using sq_num_vector_t = std::vector<sq_num_t>;
using sq_num_uset_t   = std::unordered_set<sq_num_t>;
using sq_row_col_t    = std::pair<int, int>;

using move_t          = std::pair<sq_num_t, sq_num_t>;
using move_vector_t   = std::vector<move_t>;
using move_umap_t     = std::unordered_map<sq_num_t, sq_num_vector_t>;

using move_type_vector_t = std::vector<move_type_t>;

using piece_type_vector_t = std::vector<piece_type_t>;
using piece_type_uset_t = std::unordered_set<piece_type_t>;


static const std::map<char, piece_type_t> piece_type_map {
    { 'p', kPiecePawn },
    { 'b', kPieceBishop },
    { 'n', kPieceKnight },
    { 'r', kPieceRook },
    { 'q', kPieceQueen },
    { 'k', kPieceKing }
};


static const move_type_vector_t moves_template_bishoplike {
    kMoveUpLeft, kMoveUpRight, kMoveDownLeft, kMoveDownRight
};

static const move_type_vector_t moves_template_knightlike {
    kMoveUpLeftKnight, kMoveUpRightKnight, kMoveLeftUpKnight, kMoveLeftDownKnight, kMoveRightUpKnight, kMoveRightDownKnight, kMoveDownLeftKnight, kMoveDownRightKnight
};

static const move_type_vector_t moves_template_rooklike {
    kMoveUp, kMoveRight, kMoveLeft, kMoveDown
};

static constexpr sq_num_t   kInvalidSquare { -1 };
static constexpr int        kInvalidRowCol { -1 };
