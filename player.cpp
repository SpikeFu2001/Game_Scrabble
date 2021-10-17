#include "player.h"

using namespace std;

// TODO: implement member functions

// Adds points to player's score
void Player::add_points(size_t points) { this->points += points; }

// Subtracts points from player's score
void Player::subtract_points(size_t points) { this->points -= points; }

size_t Player::get_points() const { return this->points; }

const std::string& Player::get_name() const { return this->name; }

// Returns the number of tiles in a player's hand.
size_t Player::count_tiles() const { return this->tiles.count_tiles(); }

// Removes tiles from player's hand.
void Player::remove_tiles(const std::vector<TileKind>& tiles) {
    for (auto it = tiles.begin(); it != tiles.end(); it++) {
        this->tiles.remove_tile(*it);
    }
}

// Adds tiles to player's hand.
void Player::add_tiles(const std::vector<TileKind>& tiles) {
    for (auto it = tiles.begin(); it != tiles.end(); it++) {
        this->tiles.add_tile(*it);
    }
}

// Checks if player has a matching tile.
bool Player::has_tile(TileKind tile) {
    try{
        this->tiles.lookup_tile(tile.letter);
        return true;
    } catch (out_of_range& e) {
        return false;
    }
}

// Returns the total points of all tiles in the players hand.
unsigned int Player::get_hand_value() const { return this->tiles.total_points(); }

size_t Player::get_hand_size() const { return this->hand_size; }