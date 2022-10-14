#include <gtest/gtest.h>

#include "fen.hh"


TEST(FenValidTest, ValidFenAOK) {
    EXPECT_EQ(Fen::fen_valid("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq -"), true);
    EXPECT_EQ(Fen::fen_valid("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"), true);
    EXPECT_EQ(Fen::fen_valid("8/8/8/k5QK/8/8/8/8 b - -"), true);
}

TEST(FenValidTest, ValidFenLegalPositionIncorrectProperty) {
    EXPECT_EQ(Fen::fen_valid("8/8/k7/8/8/8/8/7K w KQ -"), true); // incorrect castling rights
    EXPECT_EQ(Fen::fen_valid("8/8/k7/8/8/8/8/7K w - a3"), true); // incorrect EP square
    EXPECT_EQ(Fen::fen_valid("8/8/8/p7/8/3PP3/8/K6k w - a6"), true); // EP available on square, but not possible
    EXPECT_EQ(Fen::fen_valid("8/8/8/pP6/8/8/8/K6k w - a6 21 37"), true); // EP available, thus pawn moved, thus incorrect halfmove counter
    EXPECT_EQ(Fen::fen_valid("8/11111111/8/8/8/8/8/k6K b - -"), true); // can be shortened
}

TEST(FenValidTest, ValidFenIllegalPosition) {
    EXPECT_EQ(Fen::fen_valid("8/8/8/8/8/8/8/8 w - -"), true); // not enough kings
    EXPECT_EQ(Fen::fen_valid("8/K5K1/8/8/8/8/k1k5/8 w - -"), true); // too many kings
    EXPECT_EQ(Fen::fen_valid("8/K5K1/8/8/8/8/8/8 w - -"), true); // both
    EXPECT_EQ(Fen::fen_valid("8/kK6/8/8/8/8/8/8 w - -"), true); // kings adjacent
    EXPECT_EQ(Fen::fen_valid("pPpPpPpP/8/8/k6K/8/8/8/PpPpPpPp w - -"), true); // pawns on edge ranks
    EXPECT_EQ(Fen::fen_valid("8/8/8/k5QK/8/8/8/8 w - -"), true); // player not to move in check
}

TEST(FenValidTest, InvalidFen) {
    EXPECT_EQ(Fen::fen_valid(""), false);
    EXPECT_EQ(Fen::fen_valid("Most certainly not a FEN."), false);
    EXPECT_EQ(Fen::fen_valid("9/8/7/6/0/4/3/2 b - -"), false);
    EXPECT_EQ(Fen::fen_valid(" rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - "), false);
    EXPECT_EQ(Fen::fen_valid("rrrrrrrrrrrrrrrrrr/8/8/8/8/8/8/k6K b - -"), false);
    EXPECT_EQ(Fen::fen_valid("8/8/k6K b - -"), false);
    EXPECT_EQ(Fen::fen_valid("8/12345/8/8/8/8/8/k6K b - -"), false);
    EXPECT_EQ(Fen::fen_valid("8/1p/8/8/8/8/8/k6K b - -"), false);
    EXPECT_EQ(Fen::fen_valid("8/8/8/8/8/8/8/k6K q - -"), false);
    EXPECT_EQ(Fen::fen_valid("8/8/8/8/8/8/8/k6K w FFFF -"), false);
    EXPECT_EQ(Fen::fen_valid("8/8/8/8/8/8/8/k6K w QQ -"), false);
    EXPECT_EQ(Fen::fen_valid("8/8/8/8/8/8/8/k6K w qk -"), false);
    EXPECT_EQ(Fen::fen_valid("8/8/8/8/8/8/8/k6K w - x0"), false);
    EXPECT_EQ(Fen::fen_valid("8/8/8/8/8/8/8/k6K w - a8"), false);
    EXPECT_EQ(Fen::fen_valid("8/8/8/8/8/8/8/k6K w - - -5 -10"), false);
    EXPECT_EQ(Fen::fen_valid("8/8/8/8/8/8/8/k6K w - - 00 3"), false);
    EXPECT_EQ(Fen::fen_valid("8/8/8/8/8/8/8/k6K w - - 0 0"), false);
}
