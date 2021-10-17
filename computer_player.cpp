
#include "computer_player.h"

#include <memory>
#include <string>

void ComputerPlayer::left_part(
        Board::Position anchor_pos,  // remain unchanged
        std::string partial_word,
        Move partial_move,
        std::shared_ptr<Dictionary::TrieNode> node,
        size_t limit,
        TileCollection& remaining_tiles,
        std::vector<Move>& legal_moves,  // remain unchanged
        const Board& board,
        const Dictionary& dictionary) const {
    // HW5: IMPLEMENT THIS
    // if limit=n, go through limit=n,n-1,n-2,...,0

    if (limit == 0) {  // base case, hit the anchor position
        extend_right(anchor_pos, partial_word, partial_move, node, remaining_tiles, legal_moves, board, dictionary);
        return;
        // call extend_right
    } else if (limit > 0) {
        for (auto pair : node->nexts) {
            try {
                TileKind placement = remaining_tiles.lookup_tile(pair.first);
                // have the next_letter in hand
                remaining_tiles.remove_tile(placement);  // place the tile on board
                Move new_move = partial_move;            // the starting position remains unchange
                new_move.tiles.push_back(placement);     // add the placed tile to the move.tiles
                left_part(
                        anchor_pos,
                        partial_word + pair.first,  // update the word
                        new_move,                   // update the move
                        pair.second,                // update the node
                        limit - 1,                  // update the distance from anchor
                        remaining_tiles,            // update the remaining tiles
                        legal_moves,
                        board,
                        dictionary);
                remaining_tiles.add_tile(placement);
                // undo the change
            } catch (std::exception& e) {
                // do not have any tile in next_letters
                // do nothing
            }
            try {
                TileKind blank_tile = remaining_tiles.lookup_tile('?');
                // have a blank tile on hand
                // place the blank tile as the next_letter
                remaining_tiles.remove_tile(blank_tile);
                blank_tile.assigned = pair.first;
                Move new_move = partial_move;
                new_move.tiles.push_back(blank_tile);
                left_part(
                        anchor_pos,
                        partial_word + pair.first,  // update the word
                        new_move,                   // update the move
                        pair.second,                // update the node
                        limit - 1,                  // update the distance from anchor
                        remaining_tiles,            // update the remaining tiles
                        legal_moves,
                        board,
                        dictionary);
                remaining_tiles.add_tile(blank_tile);
                // undo the change and backtrack to the next of *next_letters*
            } catch (std::exception& e) {
                // do not have any black tile on hand
                // do nothing
            }
        }
        // outside of the for loop of finding the letters in next_letters
        // when limit=n, covers moves start from n-1, n-2, n-3,..,1 also
        if (node == dictionary.get_root()) {
            // when no tile has been placed
            // we can look at the next square as starting point without anything has been placed
            Move new_move = partial_move;  // Kind Direction Tiles remain unchanged
            Board::Position previous_start(partial_move.row, partial_move.column);
            Board::Position new_start = previous_start.translate(partial_move.direction, 1);
            new_move.row = new_start.row;
            new_move.column = new_start.column;
            // update the position
            left_part(
                    anchor_pos,
                    "",
                    new_move,
                    dictionary.get_root(),
                    limit - 1,        // update the distance from anchor
                    remaining_tiles,  // remaining_tiles remain unchanged
                    legal_moves,
                    board,
                    dictionary);
        }
    }
}

void ComputerPlayer::extend_right(
        Board::Position square,
        std::string partial_word,
        Move partial_move,
        std::shared_ptr<Dictionary::TrieNode> node,
        TileCollection& remaining_tiles,
        std::vector<Move>& legal_moves,
        const Board& board,
        const Dictionary& dictionary) const {
    // HW5: IMPLEMENT THIS
    // trigger the extend_right function when reaching the anchor
    if (dictionary.is_word(partial_word)) {
        PlaceResult move_result = board.test_place(partial_move);
        bool valid_words = true;
        for (std::string letters : move_result.words) {
            if (!dictionary.is_word(letters)) {
                valid_words = false;
            }
        }
        if (move_result.valid && valid_words) {
            legal_moves.push_back(partial_move);
        }
        // do not return and end the function
        // because valid word can be prefix of other words
    }
    if (board.in_bounds_and_has_tile(square)) {
        if (node->nexts.find(board.letter_at(square)) != node->nexts.end()) {
            node = node->nexts.find(board.letter_at(square))->second;
            // if the letter on the square can be the next letter
            // update the node
            partial_word = partial_word + board.letter_at(square);
            // add the letter on board into the partial word
            // do not update anything in partial_move
            square = square.translate(partial_move.direction, 1);
            // update the square
            extend_right(square, partial_word, partial_move, node, remaining_tiles, legal_moves, board, dictionary);
        } else {
            return;
        }
    } else if (board.is_in_bounds(square)) {
        // the square is unoccupied
        for (auto pair : node->nexts) {
            try {
                TileKind placement = remaining_tiles.lookup_tile(pair.first);
                // have the next_letter in hand
                remaining_tiles.remove_tile(placement);  // place the tile on board
                Move new_move = partial_move;            // the starting position remains unchange
                new_move.tiles.push_back(placement);     // add the placed tile to the move.tiles
                extend_right(
                        square.translate(partial_move.direction, 1),
                        partial_word + pair.first,
                        new_move,
                        pair.second,
                        remaining_tiles,
                        legal_moves,
                        board,
                        dictionary);
                remaining_tiles.add_tile(placement);  // undo the change 
            } catch (std::exception& e) {
                // do nothing
            }
            try {
                TileKind blank_tile = remaining_tiles.lookup_tile('?');
                // have the next_letter in hand
                remaining_tiles.remove_tile(blank_tile);  // place the tile on board
                blank_tile.assigned = pair.first;
                Move new_move = partial_move;            // the starting position remains unchange
                new_move.tiles.push_back(blank_tile);     // add the placed tile to the move.tiles
                extend_right(
                        square.translate(partial_move.direction, 1),
                        partial_word + pair.first,
                        new_move,
                        pair.second,
                        remaining_tiles,
                        legal_moves,
                        board,
                        dictionary);
                remaining_tiles.add_tile(blank_tile);  // undo the change and backtrack to the next of *next_letters*
            } catch (std::exception& e) {
                // do nothing
            }
        }
    } else {
        // not in bound
        return;
    }
}

Move ComputerPlayer::get_move(const Board& board, const Dictionary& dictionary) const {
    std::vector<Move> legal_moves;
    std::vector<Board::Anchor> anchors = board.get_anchors();
    // HW5: IMPLEMENT THIS
    for (auto cur_anchor : anchors) {
        if (cur_anchor.limit > 0) {
            Board::Position start_from
                    = cur_anchor.position.translate(cur_anchor.direction, (-1) * (ssize_t)cur_anchor.limit);
            std::vector<TileKind> empty_tiles;
            Move initial(empty_tiles, start_from.row, start_from.column, cur_anchor.direction);
            std::string empty_word = "";
            TileCollection tiles_in_cp = this->tiles;
            left_part(
                    cur_anchor.position,    // the postition of the anchor
                    empty_word,             // an empty word
                    initial,                // the initial move with no tiles and starts from *limits away from anchor*
                    dictionary.get_root(),  // the root of dictionary
                    cur_anchor.limit,       // limit
                    tiles_in_cp,            // the tiles in hand of computer player
                    legal_moves,
                    board,
                    dictionary);
        } else if (cur_anchor.limit == 0) {
            // do not call left_part
            // if there are precceding tiles, add those up and extend_right
            Board::Position pre = cur_anchor.position.translate(cur_anchor.direction, -1);
            std::string partial_word = "";
            std::vector<TileKind> empty_tiles;
            TileCollection tiles_in_cp = this->tiles;
            Move extend(empty_tiles, cur_anchor.position.row, cur_anchor.position.column, cur_anchor.direction);
            auto new_node = dictionary.get_root();
            while (board.in_bounds_and_has_tile(pre)) {
                // there are precceding tiles
                partial_word = board.letter_at(pre) + partial_word;
                pre = pre.translate(cur_anchor.direction, -1);
            }
            for (char letter : partial_word) {
                // update the node
                new_node = new_node->nexts.find(letter)->second;
            }
            extend_right(
                    cur_anchor.position, partial_word, extend, new_node, tiles_in_cp, legal_moves, board, dictionary);
        }
    }
    return get_best_move(legal_moves, board, dictionary);
}

Move ComputerPlayer::get_best_move(
        std::vector<Move> legal_moves, const Board& board, const Dictionary& dictionary) const {
    Move best_move = Move();  // Pass if no move found
    int max = 0;
    // HW5: IMPLEMENT THIS
    for (Move cur : legal_moves) {
        PlaceResult cur_result = board.test_place(cur);
        int cur_score = cur_result.points;
        if (cur.tiles.size() == hand)
            cur_score += 50;
        if (cur_score > max) {
            best_move = cur;
            max = cur_score;
        }
    }
    return best_move;
}
