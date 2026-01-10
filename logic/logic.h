#pragma once
#include "common/game_state.h"
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>

namespace agent {

enum class SimpleActionType {
    move,
    attack, 
    defend,
    send_message
};

struct SimpleAction {
    SimpleActionType type;
    game::Direction direction;
    std::string message;
    
    SimpleAction(SimpleActionType t = SimpleActionType::defend, 
                 game::Direction d = game::Direction::NORTH,
                 const std::string& msg = "")
        : type(t), direction(d), message(msg) {}
};

class SimpleAgent {
private:
     
    game::Position getDirectionOffset(game::Direction dir) {
        switch(dir) {
            case game::Direction::NORTH: return game::Position(0, -1);  // Arriba
            case game::Direction::SOUTH: return game::Position(0, 1);   // Abajo
            case game::Direction::EAST: return game::Position(1, 0);    // Derecha
            case game::Direction::WEST: return game::Position(-1, 0);   // Izquierda
            default: return game::Position(0, 0);
        }
    }




    std::string agent_id;
    std::string team;
    game::Position current_position;
    int health;
    
    // Memoria simple
    std::vector<game::Position> known_enemy_positions;
    std::vector<game::Position> known_ally_positions;
    game::Position last_known_enemy;
    
    // Constantes
    const int ATTACK_RANGE = 1;
    const int LOW_HEALTH = 30;

    // Métodos privados
    void updateSelfState(const game::GameState& game_state);
    void updateMemory(const game::GameState& game_state);
    SimpleAction decideSimpleAction(const game::GameState& game_state);
    
    std::pair<bool, game::Position> findAdjacentEnemy();
    SimpleAction handleLowHealth(const game::GameState& game_state);
    SimpleAction moveTowards(const game::Position& target, const game::GameState& game_state);
    SimpleAction createAttackAction(const game::Position& enemy_pos);
    game::Position findNearestEnemy();
    
    // Métodos de utilidad
    double getDistance(const game::Position& a, const game::Position& b);
    bool isValidPosition(const game::Position& pos, const game::GameState& game_state);
    game::Position findOwnBasePosition(const game::GameState& game_state);
    game::Position findEnemyBasePosition(const game::GameState& game_state);

public:
    SimpleAgent();
    
    void initialize(const std::string& id, const std::string& team_name);
    SimpleAction processTurn(const game::GameState& game_state);
    void receiveMessage(const std::string& message);
    
    // Getters para testing
    std::string getAgentId() const { return agent_id; }
    std::string getTeam() const { return team; }
    game::Position getCurrentPosition() const { return current_position; }
    int getHealth() const { return health; }
};

} // namespace agent