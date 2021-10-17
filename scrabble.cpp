#include "scrabble.h"

#include "formatting.h"
#include <iomanip>
#include <iostream>
#include <map>

using namespace std;

// Given to you. this does not need to be changed
Scrabble::Scrabble(const ScrabbleConfig& config)
        : hand_size(config.hand_size),
          minimum_word_length(config.minimum_word_length),
          tile_bag(TileBag::read(config.tile_bag_file_path, config.seed)),
          board(Board::read(config.board_file_path)),
          dictionary(Dictionary::read(config.dictionary_file_path)) {}

// Game Loop should cycle through players and get and execute that players move
// until the game is over.
void Scrabble::game_loop() {
    bool end_of_game = false;
    int pass = 0;
    while (!end_of_game) {
        for (int i = 0; i < players.size(); i++) {
            bool valid = false;
            board.print(cout);
            cout << "Player " << this->players.at(i)->get_name() << endl;
            while (!valid) {
                if (pass == this->number_of_human) {
                    end_of_game = true;
                    break;
                }
                try {
                    Move move_generated = this->players.at(i)->get_move(board, dictionary);
                    int points = 0;
                    if (move_generated.kind == MoveKind::PASS && this->players.at(i)->is_human()) {
                        pass++;
                    } else if (move_generated.kind == MoveKind::EXCHANGE) {
                        if (this->players.at(i)->is_human()) {
                            pass = 0;
                        }
                        this->players.at(i)->remove_tiles(move_generated.tiles);
                        for (auto it = move_generated.tiles.begin(); it != move_generated.tiles.end(); it++) {
                            tile_bag.add_tile(*it);
                        }
                        this->players.at(i)->add_tiles(tile_bag.remove_random_tiles(move_generated.tiles.size()));
                    } else if (move_generated.kind == MoveKind::PLACE) {
                        if (this->players.at(i)->is_human()){
                            pass = 0;
                        }
                        PlaceResult result = board.place(move_generated);
                        this->players.at(i)->remove_tiles(move_generated.tiles);
                        points = result.points;
                        if (move_generated.tiles.size() == hand_size) {
                            points += 50;
                        }
                        this->players.at(i)->add_points(points);
                        if (move_generated.tiles.size() <= tile_bag.count_tiles()) {
                            this->players.at(i)->add_tiles(tile_bag.remove_random_tiles(move_generated.tiles.size()));
                        } else {
                            this->players.at(i)->add_tiles(tile_bag.remove_random_tiles(tile_bag.count_tiles()));
                        }
                    }
                    valid = true;
                    cout << "You gained " << SCORE_COLOR << points << rang::style::reset << " points!" << endl;
                    cout << "Your current score: " << SCORE_COLOR << this->players.at(i)->get_points()
                         << rang::style::reset << endl;
                    cout << endl << "Press [enter] to continue.";
                    cin.ignore();
                } catch (MoveException& e) {
                    cout << endl << "That move was invalid, think about a new one:";
                    cout << endl << "Press [enter] to continue.";
                    cin.ignore();
                } catch (CommandException& e) {
                    cout << endl << "That command was invalid, think about a new one:";
                    cout << endl << "Press [enter] to continue.";
                    cin.ignore();
                }
                if (this->players.at(i)->count_tiles() == 0) {
                    end_of_game = true;
                    break;
                }
            }
        }
    }
    // TODO: implement this.
    // Useful cout expressions with fancy colors. Expressions in curly braces, indicate values you supply.
    // cout << "You gained " << SCORE_COLOR << {points} << rang::style::reset << " points!" << endl;
    // cout << "Your current score: " << SCORE_COLOR << {points} << rang::style::reset << endl;
    // cout << endl << "Press [enter] to continue.";
}

// Performs final score subtraction. Players lose points for each tile in their
// hand. The player who cleared their hand receives all the points lost by the
// other players.
void Scrabble::final_subtraction(vector<shared_ptr<Player>>& plrs) {
    for (int i = 0; i < plrs.size(); i++) {
        if (plrs.at(i)->count_tiles() == 0) {
            for (int j = 0; j < plrs.size(); j++) {
                plrs.at(i)->add_points(plrs.at(j)->get_hand_value());
            }
        } else {
            if (plrs.at(i)->get_points() < plrs.at(i)->get_hand_value()) {
                plrs.at(i)->subtract_points(plrs.at(i)->get_points());
            } else {
                plrs.at(i)->subtract_points(plrs.at(i)->get_hand_value());
            }
        }
    }
}

// You should not need to change this function.
void Scrabble::print_result() {
    // Determine highest score
    size_t max_points = 0;
    for (auto player : this->players) {
        if (player->get_points() > max_points) {
            max_points = player->get_points();
        }
    }

    // Determine the winner(s) indexes
    vector<shared_ptr<Player>> winners;
    for (auto player : this->players) {
        if (player->get_points() >= max_points) {
            winners.push_back(player);
        }
    }

    cout << (winners.size() == 1 ? "Winner:" : "Winners: ");
    for (auto player : winners) {
        cout << SPACE << PLAYER_NAME_COLOR << player->get_name();
    }
    cout << rang::style::reset << endl;

    // now print score table
    cout << "Scores: " << endl;
    cout << "---------------------------------" << endl;

    // Justify all integers printed to have the same amount of character as the high score, left-padding with spaces
    cout << setw(static_cast<uint32_t>(floor(log10(max_points) + 1)));

    for (auto player : this->players) {
        cout << SCORE_COLOR << player->get_points() << rang::style::reset << " | " << PLAYER_NAME_COLOR
             << player->get_name() << rang::style::reset << endl;
    }
}

void Scrabble::add_players() {
    cout << "Enter the number of players:" << endl;
    int number_players;
    cin >> number_players;
    cin.ignore();
    for (int i = 0; i < number_players; i++) {
        string name;
        string is_computer;
        bool is_human = true;
        cout << "Enter the name of player" << i+1 << " :" << endl;
        getline(cin, name);
        cout << "Is " << name << " a computer? (y/n)" << endl;
        getline(cin, is_computer);
        if (is_computer == "y") {
            this->players.push_back(make_shared<ComputerPlayer>(name, hand_size));
            this->players[i]->add_tiles(tile_bag.remove_random_tiles(hand_size));
            cout << "Computer Player " << name << " joins the game" << endl;
        } else if (is_computer == "n") {
            this->players.push_back(make_shared<HumanPlayer>(name, hand_size));
            this->players[i]->add_tiles(tile_bag.remove_random_tiles(hand_size));
            this->number_of_human++;
            cout << "Human Player " << name << " joins the game" << endl;
        } else {
            this->players.push_back(make_shared<HumanPlayer>(name, hand_size));
            this->players[i]->add_tiles(tile_bag.remove_random_tiles(hand_size));
            this->number_of_human++;
            cout << "That is an invalid input, but by default " << name;
            cout << " is a human player and now joins the game." << endl;
        }
    }
}

// You should not need to change this.
void Scrabble::main() {
    add_players();
    game_loop();
    final_subtraction(this->players);
    print_result();
}
