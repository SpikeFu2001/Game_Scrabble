#include "board.h"

#include "board_square.h"
#include "exceptions.h"
#include "formatting.h"
#include <fstream>
#include <iomanip>

using namespace std;

bool Board::Position::operator==(const Board::Position& other) const {
    return this->row == other.row && this->column == other.column;
}

bool Board::Position::operator!=(const Board::Position& other) const {
    return this->row != other.row || this->column != other.column;
}

Board::Position Board::Position::translate(Direction direction) const { return this->translate(direction, 1); }

Board::Position Board::Position::translate(Direction direction, ssize_t distance) const {
    if (direction == Direction::DOWN) {
        return Board::Position(this->row + distance, this->column);
    } else {
        return Board::Position(this->row, this->column + distance);
    }
}

Board Board::read(const string& file_path) {
    ifstream file(file_path);
    if (!file) {
        throw FileException("cannot open board file!");
    }

    size_t rows;
    size_t columns;
    size_t starting_row;
    size_t starting_column;
    file >> rows >> columns >> starting_row >> starting_column;
    Board board(rows, columns, starting_row, starting_column);

    // TODO: complete implementation of reading in board from file here.
    char multiplier;
    board.squares.resize(rows);
    for (size_t i = 0; i < rows; i++) {
        for (size_t j = 0; j < columns; j++) {
            file >> multiplier;
            unsigned short letter_multiplier = 1;
            unsigned short word_multiplier = 1;
            if (multiplier == '2') {
                letter_multiplier = 2;
            } else if (multiplier == '3') {
                letter_multiplier = 3;
            } else if (multiplier == 'd') {
                word_multiplier = 2;
            } else if (multiplier == 't') {
                word_multiplier = 3;
            }
            BoardSquare add(letter_multiplier, word_multiplier);
            board.squares[i].push_back(add);
        }
    }

    return board;
}

size_t Board::get_move_index() const { return this->move_index; }

PlaceResult Board::test_place(const Move& move) const {
    // TODO: complete implementation here
    if (this->in_bounds_and_has_tile(Position(move.row, move.column))) {  // the start of a move is occupied
        return PlaceResult("Your start point should be an empty square!");
    }
    PlaceResult temp(std::vector<std::string>(0), 0);
    int NumberOfTiles = move.tiles.size();
    int NumberOfPlacements = 0;
    size_t PlaceRow = move.row;
    size_t PlaceCol = move.column;
    while (NumberOfPlacements < NumberOfTiles) {  // check for every tile
        Position target(PlaceRow, PlaceCol);
        if (PlaceRow >= 0 && PlaceRow < this->rows && PlaceCol >= 0 && PlaceCol < this->columns) {
            if (!this->in_bounds_and_has_tile(target)) {  // the target square does not have a tile
                string word_generated(1, move.tiles[NumberOfPlacements].letter);
                int word_score = 0;
                int word_multi = 1;
                if (move.tiles[NumberOfPlacements].letter == '?') {
                    word_generated = move.tiles[NumberOfPlacements].assigned;
                }
                word_multi *= this->at(target).word_multiplier;
                word_score += move.tiles[NumberOfPlacements].points * this->at(target).letter_multiplier;
                if (move.direction == Direction::ACROSS) {  // if the direction of placement is horizontal
                    if (PlaceRow - 1 >= 0) {
                        size_t PlaceRow_up = PlaceRow - 1;
                        while (this->in_bounds_and_has_tile(Position(PlaceRow_up, PlaceCol))
                               && PlaceRow_up >= 0) {  // if there are letters at top
                            char add = this->at(Position(PlaceRow_up, PlaceCol)).get_tile_kind().letter;
                            if (add == '?') {
                                add = this->at(Position(PlaceRow_up, PlaceCol)).get_tile_kind().assigned;
                            }
                            word_generated = add + word_generated;  // make the letter the head of the word
                            word_score += this->at(Position(PlaceRow_up, PlaceCol)).get_tile_kind().points;
                            PlaceRow_up--;
                        }
                    }
                    if (PlaceRow + 1 < this->rows) {
                        size_t PlaceRow_down = PlaceRow + 1;
                        while (this->in_bounds_and_has_tile(Position(PlaceRow_down, PlaceCol))
                               && PlaceRow_down <= (this->rows - 1)) {  // if there are letters at bottom
                            char add = this->at(Position(PlaceRow_down, PlaceCol)).get_tile_kind().letter;
                            if (add == '?') {
                                add = this->at(Position(PlaceRow_down, PlaceCol)).get_tile_kind().assigned;
                            }
                            word_generated = word_generated + add;  // make the letter the tail of the word
                            word_score += this->at(Position(PlaceRow_down, PlaceCol)).get_tile_kind().points;
                            PlaceRow_down++;
                        }
                    }
                    if (word_generated.size() > 1) {  // if a word is generated
                        word_score *= word_multi;
                        temp.points += word_score;
                        temp.words.push_back(word_generated);  // then push it to the list of generated words
                    }
                } else if (move.direction == Direction::DOWN) {  // if the direction of placement is vertical
                    size_t PlaceCol_left = PlaceCol - 1;
                    if (PlaceCol - 1 >= 0) {
                        while (this->in_bounds_and_has_tile(Position(PlaceRow, PlaceCol_left))
                               && PlaceCol_left >= 0) {  // if there are letters at right-hand side
                            char add = this->at(Position(PlaceRow, PlaceCol_left)).get_tile_kind().letter;
                            if (add == '?') {
                                add = this->at(Position(PlaceRow, PlaceCol_left)).get_tile_kind().assigned;
                            }
                            word_generated = add + word_generated;  // make the letter the head of the word
                            word_score += this->at(Position(PlaceRow, PlaceCol_left)).get_tile_kind().points;
                            PlaceCol_left--;
                        }
                    }
                    if (PlaceCol + 1 < this->columns) {
                        size_t PlaceCol_Right = PlaceCol + 1;
                        while (this->in_bounds_and_has_tile(
                                Position(PlaceRow, PlaceCol_Right))) {  // if there are letters at left-hand side
                            char add = this->at(Position(PlaceRow, PlaceCol_Right)).get_tile_kind().letter;
                            if (add == '?') {
                                add = this->at(Position(PlaceRow, PlaceCol_Right)).get_tile_kind().assigned;
                            }
                            word_generated = word_generated + add;  // make the letter the tail of the word
                            word_score += this->at(Position(PlaceRow, PlaceCol_Right)).get_tile_kind().points;
                            PlaceCol_Right++;
                        }
                    }
                    if (word_generated.size() > 1) {  // if a word is generated
                        word_score *= word_multi;
                        temp.points += word_score;
                        temp.words.push_back(word_generated);  // then push it to the list of generated words
                    }
                }
                NumberOfPlacements++;  // increment the number of placed tiles
            }
            // increment the row/column based on the direction
            if (move.direction == Direction::ACROSS) {
                PlaceCol++;
            } else if (move.direction == Direction::DOWN) {
                PlaceRow++;
            }
        } else {
            break;
        }
        // finish the loop when tiles are used up or the tiles go out of the board
    }
    if (NumberOfPlacements != NumberOfTiles) {
        return PlaceResult("The tiles goes out of the board!");
    }
    // then we push the word generated by the direction of placement to the PlaceResult
    string word_generated = "";
    size_t Row = move.row;
    size_t Col = move.column;
    int word_score = 0;
    int word_multi = 1;
    if (move.direction == Direction::ACROSS) {
        size_t Col_left = Col - 1;
        while (this->in_bounds_and_has_tile(Position(Row, Col_left))) {
            char add = this->at(Position(Row, Col_left)).get_tile_kind().letter;
            if (add == '?') {
                add = this->at(Position(Row, Col_left)).get_tile_kind().assigned;
            }
            word_generated = add + word_generated;
            word_score += this->at(Position(Row, Col_left)).get_tile_kind().points;
            Col_left -= 1;
        }
    } else if (move.direction == Direction::DOWN) {
        size_t Row_up = Row - 1;
        while (this->in_bounds_and_has_tile(Position(Row_up, Col))) {
            char add = this->at(Position(Row_up, Col)).get_tile_kind().letter;
            if (add == '?') {
                add = this->at(Position(Row_up, Col)).get_tile_kind().assigned;
            }
            word_generated = add + word_generated;
            word_score += this->at(Position(Row_up, Col)).get_tile_kind().points;
            Row_up -= 1;
        }
    }
    int i = 0;
    while (i < move.tiles.size()) {
        Position target(Row, Col);
        if (move.direction == Direction::ACROSS) {
            if (this->in_bounds_and_has_tile(target)) {
                if (this->at(target).get_tile_kind().letter != '?') {
                    word_generated += this->at(target).get_tile_kind().letter;
                } else {
                    word_generated += this->at(target).get_tile_kind().assigned;
                }
                word_score += this->at(target).get_tile_kind().points;
                Col++;
            } else {
                if (move.tiles[i].letter != '?') {
                    word_generated += move.tiles[i].letter;
                } else {
                    word_generated += move.tiles[i].assigned;
                }
                word_multi *= this->at(target).word_multiplier;
                word_score += move.tiles[i].points * this->at(target).letter_multiplier;
                i++;
                Col++;
            }
        } else if (move.direction == Direction::DOWN) {
            if (this->in_bounds_and_has_tile(target)) {
                if (this->at(target).get_tile_kind().letter != '?') {
                    word_generated += this->at(target).get_tile_kind().letter;
                } else {
                    word_generated += this->at(target).get_tile_kind().assigned;
                }
                word_score += this->at(target).get_tile_kind().points;
                Row++;
            } else {
                if (move.tiles[i].letter != '?') {
                    word_generated += move.tiles[i].letter;
                } else {
                    word_generated += move.tiles[i].assigned;
                }
                word_multi *= this->at(target).word_multiplier;
                word_score += move.tiles[i].points * this->at(target).letter_multiplier;
                i++;
                Row++;
            }
        }
    }
    while (this->in_bounds_and_has_tile(Position(Row, Col))) {
        if (this->at(Position(Row, Col)).get_tile_kind().letter != '?') {
            word_generated += this->at(Position(Row, Col)).get_tile_kind().letter;
        } else {
            word_generated += this->at(Position(Row, Col)).get_tile_kind().letter;
        }
        word_score += this->at(Position(Row, Col)).get_tile_kind().points;
        if (move.direction == Direction::ACROSS) {
            Col++;
        } else if (move.direction == Direction::DOWN) {
            Row++;
        }
    }
    if (word_generated.size() > 1) {
        word_score *= word_multi;
        temp.points += word_score;
        temp.words.push_back(word_generated);
    }
    if (this->get_move_index() == 0) {
        bool covers = false;
        size_t row = move.row;
        size_t col = move.column;
        for (auto it = move.tiles.begin(); it != move.tiles.end(); it++) {
            if (row == start.row && col == start.column) {
                covers = true;
                break;
            }
            if (move.direction == Direction::ACROSS) {
                col++;
            } else if (move.direction == Direction::DOWN) {
                row++;
            }
        }
        if (!covers) {
            return PlaceResult("The first move must cover the start square!");
        }
    } else {
        // check there are adjecent letters
        bool adjecent = false;
        size_t row = move.row;
        size_t col = move.column;
        for (auto it = move.tiles.begin(); it != move.tiles.end(); it++) {
            if (!this->in_bounds_and_has_tile(Position(row, col))) {
                if (this->in_bounds_and_has_tile(Position(row - 1, col))
                    || this->in_bounds_and_has_tile(Position(row + 1, col))
                    || this->in_bounds_and_has_tile(Position(row, col - 1))
                    || this->in_bounds_and_has_tile(Position(row, col + 1))) {
                    adjecent = true;
                    break;
                }
            } else {
                adjecent = true;
                break;
            }
            if (move.direction == Direction::ACROSS) {
                col++;
            } else if (move.direction == Direction::DOWN) {
                row++;
            }
        }
        if (!adjecent) {
            return PlaceResult("You must have at least one adjecent tile!");
        }
    }
    return temp;
}

PlaceResult Board::place(const Move& move) {
    // TODO: Complete implementation here
    PlaceResult result = test_place(move);
    if (result.valid) {
        move_index++;
        size_t PlaceRow = move.row;
        size_t PlaceCol = move.column;
        int i = 0;
        while (i < move.tiles.size()) {
            if (move.direction == Direction::ACROSS) {
                if (this->in_bounds_and_has_tile(Position(PlaceRow, PlaceCol))) {
                    PlaceCol++;
                } else {
                    this->at(Position(PlaceRow, PlaceCol)).set_tile_kind(move.tiles[i]);
                    i++;
                    PlaceCol++;
                }
            } else if (move.direction == Direction::DOWN) {
                if (this->in_bounds_and_has_tile(Position(PlaceRow, PlaceCol))) {
                    PlaceRow++;
                } else {
                    this->at(Position(PlaceRow, PlaceCol)).set_tile_kind(move.tiles[i]);
                    i++;
                    PlaceRow++;
                }
            }
        }
    }
    return result;
}

// The rest of this file is provided for you. No need to make changes.

BoardSquare& Board::at(const Board::Position& position) { return this->squares.at(position.row).at(position.column); }

const BoardSquare& Board::at(const Board::Position& position) const {
    return this->squares.at(position.row).at(position.column);
}

bool Board::is_in_bounds(const Board::Position& position) const {
    return position.row < this->rows && position.column < this->columns;
}

bool Board::in_bounds_and_has_tile(const Position& position) const {
    return is_in_bounds(position) && at(position).has_tile();
}

char Board::letter_at(Position p) const {
    if (this->at(p).get_tile_kind().letter != '?') {
        return this->at(p).get_tile_kind().letter;
    } else {
        return this->at(p).get_tile_kind().assigned;
    }
}

bool Board::is_anchor_spot(Position p) const {
    if (is_in_bounds(p) && !this->at(p).has_tile()) {
        if (p == start) {
            return true;
        }
        if (this->in_bounds_and_has_tile(Position(p.row, p.column - 1))) {
            return true;
        }
        if (this->in_bounds_and_has_tile(Position(p.row, p.column + 1))) {
            return true;
        }
        if (this->in_bounds_and_has_tile(Position(p.row+1, p.column))) {
            return true;
        }
        if (this->in_bounds_and_has_tile(Position(p.row-1, p.column ))) {
            return true;
        }
    }
    return false;
}

std::vector<Board::Anchor> Board::get_anchors() const {
    vector<Board::Anchor> result;
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < columns; j++) {
            Position current(i, j);
            if (is_anchor_spot(current)) {
                //one for across
                size_t limit_cross = 0;
                size_t left_column = j-1;
                while (this->is_in_bounds(Position(i, left_column))) {
                    if (!this->at(Position(i, left_column)).has_tile() && !is_anchor_spot(Position(i, left_column))) {
                        limit_cross++;
                        left_column--;
                    } else {
                        break;
                    }
                }
                result.push_back(Anchor(current, Direction::ACROSS, limit_cross));
                // one for down
                size_t limit_up = 0;
                size_t up_row = i - 1;
                while (this->is_in_bounds(Position(up_row, j))) {
                    if (!this->at(Position(up_row, j)).has_tile() && !is_anchor_spot(Position(up_row, j))) {
                        limit_up++;
                        up_row--;
                    } else {
                        break;
                    }
                }
                result.push_back(Anchor(current, Direction::DOWN, limit_up));
            }
        }
    }
    return result;
}

void Board::print(ostream& out) const {
    // Draw horizontal number labels
    for (size_t i = 0; i < BOARD_TOP_MARGIN - 2; ++i) {
        out << std::endl;
    }
    out << FG_COLOR_LABEL << repeat(SPACE, BOARD_LEFT_MARGIN);
    const size_t right_number_space = (SQUARE_OUTER_WIDTH - 3) / 2;
    const size_t left_number_space = (SQUARE_OUTER_WIDTH - 3) - right_number_space;
    for (size_t column = 0; column < this->columns; ++column) {
        out << repeat(SPACE, left_number_space) << std::setw(2) << column + 1 << repeat(SPACE, right_number_space);
    }
    out << std::endl;

    // Draw top line
    out << repeat(SPACE, BOARD_LEFT_MARGIN);
    print_horizontal(this->columns, L_TOP_LEFT, T_DOWN, L_TOP_RIGHT, out);
    out << endl;

    // Draw inner board
    for (size_t row = 0; row < this->rows; ++row) {
        if (row > 0) {
            out << repeat(SPACE, BOARD_LEFT_MARGIN);
            print_horizontal(this->columns, T_RIGHT, PLUS, T_LEFT, out);
            out << endl;
        }

        // Draw insides of squares
        for (size_t line = 0; line < SQUARE_INNER_HEIGHT; ++line) {
            out << FG_COLOR_LABEL << BG_COLOR_OUTSIDE_BOARD;

            // Output column number of left padding
            if (line == 1) {
                out << repeat(SPACE, BOARD_LEFT_MARGIN - 3);
                out << std::setw(2) << row + 1;
                out << SPACE;
            } else {
                out << repeat(SPACE, BOARD_LEFT_MARGIN);
            }

            // Iterate columns
            for (size_t column = 0; column < this->columns; ++column) {
                out << FG_COLOR_LINE << BG_COLOR_NORMAL_SQUARE << I_VERTICAL;
                const BoardSquare& square = this->squares.at(row).at(column);
                bool is_start = this->start.row == row && this->start.column == column;

                // Figure out background color
                if (square.word_multiplier == 2) {
                    out << BG_COLOR_WORD_MULTIPLIER_2X;
                } else if (square.word_multiplier == 3) {
                    out << BG_COLOR_WORD_MULTIPLIER_3X;
                } else if (square.letter_multiplier == 2) {
                    out << BG_COLOR_LETTER_MULTIPLIER_2X;
                } else if (square.letter_multiplier == 3) {
                    out << BG_COLOR_LETTER_MULTIPLIER_3X;
                } else if (is_start) {
                    out << BG_COLOR_START_SQUARE;
                }

                // Text
                if (line == 0 && is_start) {
                    out << "  \u2605  ";
                } else if (line == 0 && square.word_multiplier > 1) {
                    out << FG_COLOR_MULTIPLIER << repeat(SPACE, SQUARE_INNER_WIDTH - 2) << 'W' << std::setw(1)
                        << square.word_multiplier;
                } else if (line == 0 && square.letter_multiplier > 1) {
                    out << FG_COLOR_MULTIPLIER << repeat(SPACE, SQUARE_INNER_WIDTH - 2) << 'L' << std::setw(1)
                        << square.letter_multiplier;
                } else if (line == 1 && square.has_tile()) {
                    char l = square.get_tile_kind().letter == TileKind::BLANK_LETTER ? square.get_tile_kind().assigned
                                                                                     : ' ';
                    out << repeat(SPACE, 2) << FG_COLOR_LETTER << square.get_tile_kind().letter << l
                        << repeat(SPACE, 1);
                } else if (line == SQUARE_INNER_HEIGHT - 1 && square.has_tile()) {
                    out << repeat(SPACE, SQUARE_INNER_WIDTH - 1) << FG_COLOR_SCORE << square.get_points();
                } else {
                    out << repeat(SPACE, SQUARE_INNER_WIDTH);
                }
            }

            // Add vertical line
            out << FG_COLOR_LINE << BG_COLOR_NORMAL_SQUARE << I_VERTICAL << BG_COLOR_OUTSIDE_BOARD << std::endl;
        }
    }

    // Draw bottom line
    out << repeat(SPACE, BOARD_LEFT_MARGIN);
    print_horizontal(this->columns, L_BOTTOM_LEFT, T_UP, L_BOTTOM_RIGHT, out);
    out << endl << rang::style::reset << std::endl;
}