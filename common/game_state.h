// common/game_state.h
// Defines the structures and enums for the game state 
#pragma once
#include <string>
#include <vector>

namespace game {

// Position on the map
struct Position {
    int x, y;
    
    Position(int x = 0, int y = 0) : x(x), y(y) {}
    
    bool operator==(const Position& other) const {
        return x == other.x && y == other.y;
    }
};

// Direction for movement and facing
enum class Direction {
    NORTH, SOUTH, EAST, WEST
};

// Helper functions for Direction
inline Position getDirectionOffset(Direction dir) {
    switch(dir) {
        case Direction::NORTH: return Position(0, -1);
        case Direction::SOUTH: return Position(0, 1);
        case Direction::EAST: return Position(1, 0);
        case Direction::WEST: return Position(-1, 0);
        default: return Position(0, 0);
    }
}

inline Direction getOppositeDirection(Direction dir) {
    switch(dir) {
        case Direction::NORTH: return Direction::SOUTH;
        case Direction::SOUTH: return Direction::NORTH;
        case Direction::EAST: return Direction::WEST;
        case Direction::WEST: return Direction::EAST;
        default: return Direction::NORTH;
    }
}

inline Direction getDirectionFromString(const std::string& dirStr) {
    if (dirStr == "north") return Direction::NORTH;
    if (dirStr == "south") return Direction::SOUTH;
    if (dirStr == "east") return Direction::EAST;
    if (dirStr == "west") return Direction::WEST;
    return Direction::NORTH; // default
}

inline std::string getStringFromDirection(Direction dir) {
    switch(dir) {
        case Direction::NORTH: return "north";
        case Direction::SOUTH: return "south";
        case Direction::EAST: return "east";
        case Direction::WEST: return "west";
        default: return "south";
    }
}

// Agent state
struct Agent {
    std::string id;
    std::string team;
    Position position;
    Direction facing;
    int hp;
    int max_hp;
    bool is_alive;
    
    Agent(const std::string& id, const std::string& team, Position pos, int hp = 100)
        : id(id), team(team), position(pos), facing(Direction::NORTH), hp(hp), max_hp(hp), is_alive(true) {}
};

// Base structure
struct Base {
    std::string team;
    Position position;
    int hp;
    int max_hp;
    bool is_destroyed;
    
    Base(const std::string& team, Position pos, int hp = 500)
        : team(team), position(pos), hp(hp), max_hp(hp), is_destroyed(false) {}
};

// Game configuration
struct GameConfig {
    int map_width;
    int map_height;
    int max_turns;
    
    GameConfig() : map_width(20), map_height(20), max_turns(500) {}
};

// Game state
struct GameState {
    std::vector<Agent> agents;
    std::vector<Base> bases;
    int current_turn;
    bool game_over;
    std::string winner;
    GameConfig config;
    
    GameState() : current_turn(0), game_over(false) {}
};

} // namespace game