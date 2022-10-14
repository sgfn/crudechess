#include <regex>
#include <sstream>

#include "board_types.hh"
#include "fen.hh"

#include "log.hh"


/**
 * @brief Performs the first bunch of FEN validity checks. Does not check whether
 * the position denoted by a given FEN is legal.
 * 
 * @param fen FEN string
 * @retval true - FEN validation first stage passed
 * @retval false - otherwise
 */
bool Fen::fen_valid(const std::string& fen) {
    if (!std::regex_match(fen, std::regex(kFenRegex))) {
        LOG_TRACE("FEN `%s' invalid: does not match regex", fen.c_str());
        return false;
    }

    std::stringstream fen_stream(fen);
    std::string field;
    int field_num = 0;
    while (std::getline(fen_stream, field, ' ')) {
        ++field_num;
        if (field_num == kPiecePositions) {
            int col = 0;
            for (const auto ch : field) {
                if (ch == '/') {
                    if (col != 8) {
                        LOG_TRACE("FEN `%s' invalid: not enough columns", fen.c_str());
                        return false;
                    }
                    col = 0;
                } else if (ch >= 49 && ch <= 56) { // ch is a number
                    col += ch-48;
                } else { // ch is a letter
                    ++col;
                }

                if (col > 8) {
                    LOG_TRACE("FEN `%s' invalid: too many columns", fen.c_str());
                    return false;
                }
            }
        } else if (field_num == kCastlingRights) {
            char last_letter = 0;
            for (const auto ch : field) {
                if (last_letter >= ch) {
                    LOG_TRACE("FEN `%s' invalid: malformed castling rights", fen.c_str());
                    return false;
                }
                last_letter = ch;
            }
        }
    }

    return true;
}
