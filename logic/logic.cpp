#include "logic.h"
#include "common/game_state.h"


namespace agent {
    SimpleAgent::SimpleAgent() : health(100) {}

void SimpleAgent::initialize(const std::string& id, const std::string& team_name) {
    agent_id = id;
    team = team_name;
}

SimpleAction SimpleAgent::processTurn(const game::GameState& game_state) {
    updateSelfState(game_state);
    updateMemory(game_state);
    return decideSimpleAction(game_state);
}

void SimpleAgent::receiveMessage(const std::string& message) {
    if (message.find("ENEMY:") == 0) {
        size_t pos = message.find(':');
        std::string coords = message.substr(pos + 1);
        size_t comma = coords.find(',');
        int x = std::stoi(coords.substr(0, comma));
        int y = std::stoi(coords.substr(comma + 1));
        last_known_enemy = game::Position(x, y);
    }
}

// Implementación de métodos privados

void SimpleAgent::updateSelfState(const game::GameState& game_state) {
    for (const auto& agent : game_state.agents) {
        if (agent.id == agent_id && agent.is_alive) {
            current_position = agent.position;
            health = agent.hp;
            break;
        }
    }
}

void SimpleAgent::updateMemory(const game::GameState& game_state) {
    known_enemy_positions.clear();
    known_ally_positions.clear();
    
    for (const auto& agent : game_state.agents) {
        if (agent.is_alive) {
            if (agent.team != team) {
                known_enemy_positions.push_back(agent.position);
            } else if (agent.id != agent_id) {
                known_ally_positions.push_back(agent.position);
            }
        }
    }
}

SimpleAction SimpleAgent::decideSimpleAction(const game::GameState& game_state) {
    // 1. Si hay enemigo adyacente, ATACAR
    auto adjacent_enemy = findAdjacentEnemy();
    if (adjacent_enemy.first) {
        return createAttackAction(adjacent_enemy.second);
    }
    
    // 2. Si salud baja, DEFENDER o ESCAPAR
    if (health < LOW_HEALTH) {
        return handleLowHealth(game_state);
    }
    
    // 3. Si sabemos donde hay enemigos, MOVERSE hacia ellos
    if (!known_enemy_positions.empty()) {
        game::Position nearest_enemy = findNearestEnemy();
        return moveTowards(nearest_enemy, game_state);
    }
    
    // 4. Si no hay enemigos visibles, MOVERSE hacia base enemiga
    game::Position enemy_base = findEnemyBasePosition(game_state);
    return moveTowards(enemy_base, game_state);
}

std::pair<bool, game::Position> SimpleAgent::findAdjacentEnemy() {
    for (const auto& enemy_pos : known_enemy_positions) {
        if (getDistance(current_position, enemy_pos) <= ATTACK_RANGE) {
            return {true, enemy_pos};
        }
    }
    return {false, game::Position()};
}

SimpleAction SimpleAgent::handleLowHealth(const game::GameState& game_state) {
    // Si hay enemigos cerca, defender
    if (!known_enemy_positions.empty()) {
        game::Position nearest_enemy = findNearestEnemy();
        if (getDistance(current_position, nearest_enemy) <= 2) {
            return SimpleAction(SimpleActionType::defend);
        }
    }
    
    // Si no, moverse hacia base propia para curarse
    game::Position own_base = findOwnBasePosition(game_state);
    return moveTowards(own_base, game_state);
}
SimpleAction SimpleAgent::moveTowards(const game::Position& target, const game::GameState& game_state) {
    int dx = target.x - current_position.x;
    int dy = target.y - current_position.y;
    
    game::Direction best_dir = game::Direction::NORTH;
    double best_score = -1000000; // Valor inicial muy bajo
    
    std::vector<game::Direction> directions = {
        game::Direction::NORTH, game::Direction::SOUTH,
        game::Direction::EAST, game::Direction::WEST
    };
    
    for (auto dir : directions) {
        game::Position new_pos = current_position;
        
        // CALCULAR NUEVA POSICIÓN MANUALMENTE
        switch(dir) {
            case game::Direction::NORTH: 
                new_pos.y -= 1; 
                break;
            case game::Direction::SOUTH: 
                new_pos.y += 1; 
                break;
            case game::Direction::EAST: 
                new_pos.x += 1; 
                break;
            case game::Direction::WEST: 
                new_pos.x -= 1; 
                break;
        }
        
        if (isValidPosition(new_pos, game_state)) {
            double new_dist = getDistance(new_pos, target);
            double score = -new_dist; // Más negativo = mejor (más cerca)
            
            // Evitar enemigos si la salud es baja
            if (health < LOW_HEALTH) {
                for (const auto& enemy_pos : known_enemy_positions) {
                    double enemy_dist = getDistance(new_pos, enemy_pos);
                    score += enemy_dist * 2; // Premiar distancia de enemigos
                }
            }
            
            if (score > best_score) {
                best_score = score;
                best_dir = dir;
            }
        }
    }
    
    // Si encontramos un movimiento válido
    if (best_score > -1000000) {
        return SimpleAction(SimpleActionType::move, best_dir);
    }
    
    // Si no podemos movernos, defender
    return SimpleAction(SimpleActionType::defend);
}

SimpleAction SimpleAgent::createAttackAction(const game::Position& enemy_pos) {
    // Atacar en dirección al enemigo más cercano
    int dx = enemy_pos.x - current_position.x;
    int dy = enemy_pos.y - current_position.y;
    
    game::Direction attack_dir;
    
    if (std::abs(dx) > std::abs(dy)) {
        attack_dir = (dx > 0) ? game::Direction::EAST : game::Direction::WEST;
    } else {
        attack_dir = (dy > 0) ? game::Direction::SOUTH : game::Direction::NORTH;
    }
    
    return SimpleAction(SimpleActionType::attack, attack_dir);
}

game::Position SimpleAgent::findNearestEnemy() {
    if (known_enemy_positions.empty()) {
        return current_position; // Fallback
    }
    
    game::Position nearest = known_enemy_positions[0];
    double min_dist = getDistance(current_position, nearest);
    
    for (const auto& enemy_pos : known_enemy_positions) {
        double dist = getDistance(current_position, enemy_pos);
        if (dist < min_dist) {
            min_dist = dist;
            nearest = enemy_pos;
        }
    }
    
    return nearest;
}

// Métodos de utilidad

double SimpleAgent::getDistance(const game::Position& a, const game::Position& b) {
    return std::sqrt(std::pow(a.x - b.x, 2) + std::pow(a.y - b.y, 2));
}

bool SimpleAgent::isValidPosition(const game::Position& pos, const game::GameState& game_state) {
    if (pos.x < 0 || pos.y < 0 || 
        pos.x >= game_state.config.map_width || 
        pos.y >= game_state.config.map_height) {
        return false;
    }
    
    for (const auto& agent : game_state.agents) {
        if (agent.is_alive && agent.position == pos) {
            return false;
        }
    }
    
    return true;
}

game::Position SimpleAgent::findOwnBasePosition(const game::GameState& game_state) {
    for (const auto& base : game_state.bases) {
        if (base.team == team) {
            return base.position;
        }
    }
    return game::Position(0, 0);
}

game::Position SimpleAgent::findEnemyBasePosition(const game::GameState& game_state) {
    for (const auto& base : game_state.bases) {
        if (base.team != team) {
            return base.position;
        }
    }
    return game::Position(game_state.config.map_width - 1, game_state.config.map_height - 1);
}

} 