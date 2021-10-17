#include "human_player.h"

#include "exceptions.h"
#include "formatting.h"
#include "move.h"
#include "place_result.h"
#include "rang.h"
#include "tile_kind.h"
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <vector>

using namespace std;

// This method is fully implemented.
inline string& to_upper(string& str) {
    transform(str.begin(), str.end(), str.begin(), ::toupper);
    return str;
}

Move HumanPlayer::get_move(const Board& board, const Dictionary& dictionary) const {
    // TODO: begin your implementation here.
    this->print_hand(cout);
    cout << "Now take your time and then Enter your Move:" << endl;
    string move_string;
    getline(cin, move_string);
    move_string = to_upper(move_string);
    Move move_generated = parse_move(move_string);
    if (move_generated.kind == MoveKind::PLACE) {
        PlaceResult placementResult = board.test_place(move_generated);
        if (placementResult.valid) {
            std::vector<std::string> words = placementResult.words;
            for (std::vector<std::string>::iterator it = words.begin(); it != words.end(); it++) {
                cout << (*it) << endl;
                if (!dictionary.is_word(*it)) {
                    throw MoveException("This move generated words that are not in the dictionary!");
                }
            }
        } else {
            throw MoveException("Your placement is invalid!");
        }
    }
    return move_generated;
}

vector<TileKind> HumanPlayer::parse_tiles(string& letters, string& move_string) const {
    vector<TileKind> result;
    stringstream input(letters);
    char letter;
    while (input >> letter) {
        if (letter != '?' || move_string == "EXCHANGE") {
            TileKind remove = this->tiles.lookup_tile(letter);
            result.push_back(remove);
        } else if (letter == '?' && move_string == "PLACE") {
            TileKind remove = this->tiles.lookup_tile(letter);
            input >> letter;
            TileKind assign(remove.letter, remove.points, letter);
            result.push_back(assign);
        }
    }
    return result;
}

Move HumanPlayer::parse_move(string& move_string) const {
    stringstream input(move_string);
    string command;
    input >> command;
    if (command == "EXCHANGE") {
        string letters;
        input >> letters;
        vector<TileKind> tiles_generated = parse_tiles(letters, command);
        return Move(tiles_generated);
    } else if (command == "PASS") {
        return Move();
    } else if (command == "PLACE") {
        char direction;
        int row;
        int column;
        string letters;
        input >> direction >> row >> column >> letters;
        vector<TileKind> tiles_generated = parse_tiles(letters, command);
        if (direction == '-') {
            return Move(tiles_generated, row-1, column-1, Direction::ACROSS);
        } else if (direction == '|') {
            return Move(tiles_generated, row-1, column-1, Direction::DOWN);
        } else {
            throw CommandException("Direction must be vertical or horizontal!");
        }
    } else {
        throw CommandException("Hey that is an invalid Move kind!");
    }
}

// This function is fully implemented.
void HumanPlayer::print_hand(ostream& out) const {
    const size_t tile_count = tiles.count_tiles();
    const size_t empty_tile_count = this->get_hand_size() - tile_count;
    const size_t empty_tile_width = empty_tile_count * (SQUARE_OUTER_WIDTH - 1);

    for (size_t i = 0; i < HAND_TOP_MARGIN - 2; ++i) {
        out << endl;
    }

    out << repeat(SPACE, HAND_LEFT_MARGIN) << FG_COLOR_HEADING << "Your Hand: " << endl << endl;

    // Draw top line
    out << repeat(SPACE, HAND_LEFT_MARGIN) << FG_COLOR_LINE << BG_COLOR_NORMAL_SQUARE;
    print_horizontal(tile_count, L_TOP_LEFT, T_DOWN, L_TOP_RIGHT, out);
    out << repeat(SPACE, empty_tile_width) << BG_COLOR_OUTSIDE_BOARD << endl;

    // Draw middle 3 lines
    for (size_t line = 0; line < SQUARE_INNER_HEIGHT; ++line) {
        out << FG_COLOR_LABEL << BG_COLOR_OUTSIDE_BOARD << repeat(SPACE, HAND_LEFT_MARGIN);
        for (auto it = tiles.cbegin(); it != tiles.cend(); ++it) {
            out << FG_COLOR_LINE << BG_COLOR_NORMAL_SQUARE << I_VERTICAL << BG_COLOR_PLAYER_HAND;

            // Print letter
            if (line == 1) {
                out << repeat(SPACE, 2) << FG_COLOR_LETTER << (char)toupper(it->letter) << repeat(SPACE, 2);

                // Print score in bottom right
            } else if (line == SQUARE_INNER_HEIGHT - 1) {
                out << FG_COLOR_SCORE << repeat(SPACE, SQUARE_INNER_WIDTH - 2) << setw(2) << it->points;

            } else {
                out << repeat(SPACE, SQUARE_INNER_WIDTH);
            }
        }
        if (tiles.count_tiles() > 0) {
            out << FG_COLOR_LINE << BG_COLOR_NORMAL_SQUARE << I_VERTICAL;
            out << repeat(SPACE, empty_tile_width) << BG_COLOR_OUTSIDE_BOARD << endl;
        }
    }

    // Draw bottom line
    out << repeat(SPACE, HAND_LEFT_MARGIN) << FG_COLOR_LINE << BG_COLOR_NORMAL_SQUARE;
    print_horizontal(tile_count, L_BOTTOM_LEFT, T_UP, L_BOTTOM_RIGHT, out);
    out << repeat(SPACE, empty_tile_width) << rang::style::reset << endl;
}

