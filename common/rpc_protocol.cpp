// common/rpc_protocol.cpp
// Implements the RPC protocol for communication between the game engine and agents
#include "rpc_protocol.h"
#include <sstream>
#include <unordered_map>
#include <iostream>
#include <thread>
#include <chrono>
#include <future>

namespace rpc {

#include <string>

std::string void_response(const std::string& id) {
    return std::string("{") +
        "\"id\":\"" + id + "\"," +
        "\"status\":\"ok\"" +
    "}";
}

std::string turn_response(const std::string& id, const std::string& action) {
    return std::string("{") +
        "\"id\":\"" + id + "\"," +
        "\"action\":\"" + action + "\"" +
    "}";
}


std::string register_message(const std::string& id, const std::string& agent_id) {
    return std::string("{") +
        "\"id\":\"" + id + "\"," +
        "\"type\":\"register_agent\"," +
        "\"agent_id\":\"" + agent_id + "\"" +
    "}";
}
    
        //  Basic JSON utilities
std::string escapeJson(const std::string& str) {
    std::string result;
    for (char c : str) {
        switch (c) {
            case '"': result += "\\\""; break;
            case '\\': result += "\\\\"; break;
            case '\n': result += "\\n"; break;
            case '\r': result += "\\r"; break;
            case '\t': result += "\\t"; break;
            default: result += c; break;
        }
    }
    return result;
}

std::string extractStringValue(const std::string& json, const std::string& key) {
    std::string search = "\"" + key + "\":\"";
    size_t start = json.find(search);
    if (start == std::string::npos) return "";
    start += search.length();
    size_t end = json.find("\"", start);
    if (end == std::string::npos) return "";
    return json.substr(start, end - start);
}

bool extractBoolValue(const std::string& json, const std::string& key) {
    std::string search = "\"" + key + "\":";
    size_t start = json.find(search);
    if (start == std::string::npos) return false;
    start += search.length();
    return json.substr(start, 4) == "true";
}

int extractIntValue(const std::string& json, const std::string& key) {
    std::string search = "\"" + key + "\":";
    size_t start = json.find(search);
    if (start == std::string::npos) return 0;
    start += search.length();
    size_t end = json.find_first_of(",}", start);
    if (end == std::string::npos) return 0;
    return std::stoi(json.substr(start, end - start));
}

// GameState deserialization implementation
game::Position deserializePosition(const std::string& json) {
    int x = extractIntValue(json, "x");
    int y = extractIntValue(json, "y");
    return game::Position(x, y);
}

game::Agent deserializeAgent(const std::string& json) {
    std::string id = extractStringValue(json, "id");
    std::string team = extractStringValue(json, "team");
    
    // Extract position object
    std::string pos_search = "\"position\":";
    size_t pos_start = json.find(pos_search);
    if (pos_start != std::string::npos) {
        pos_start += pos_search.length();
        size_t pos_end = json.find("}", pos_start);
        if (pos_end != std::string::npos) {
            std::string pos_json = json.substr(pos_start, pos_end - pos_start + 1);
            game::Position position = deserializePosition(pos_json);
            
            std::string facing_str = extractStringValue(json, "facing");
            game::Direction facing = game::getDirectionFromString(facing_str);
            int hp = extractIntValue(json, "hp");
            int max_hp = extractIntValue(json, "max_hp");
            bool is_alive = extractBoolValue(json, "is_alive");
            
            game::Agent agent(id, team, position, max_hp);
            agent.facing = facing;
            agent.hp = hp;
            agent.is_alive = is_alive;
            return agent;
        }
    }
    
    // Fallback - create agent with default values
    return game::Agent(id, team, game::Position(0, 0), 100);
}

game::Base deserializeBase(const std::string& json) {
    std::string team = extractStringValue(json, "team");
    
    // Extract position object
    std::string pos_search = "\"position\":";
    size_t pos_start = json.find(pos_search);
    game::Position position(0, 0);
    if (pos_start != std::string::npos) {
        pos_start += pos_search.length();
        size_t pos_end = json.find("}", pos_start);
        if (pos_end != std::string::npos) {
            std::string pos_json = json.substr(pos_start, pos_end - pos_start + 1);
            position = deserializePosition(pos_json);
        }
    }
    
    int hp = extractIntValue(json, "hp");
    int max_hp = extractIntValue(json, "max_hp");
    bool is_destroyed = extractBoolValue(json, "is_destroyed");
    
    game::Base base(team, position, max_hp);
    base.hp = hp;
    base.is_destroyed = is_destroyed;
    return base;
}

game::GameConfig deserializeGameConfig(const std::string& json) {
    game::GameConfig config;
    config.map_width = extractIntValue(json, "map_width");
    config.map_height = extractIntValue(json, "map_height");
    config.max_turns = extractIntValue(json, "max_turns");
    return config;
}

game::GameState deserializeGameState(const std::string& json) {
    game::GameState state;
    
    // Deserialize agents array
    std::string agents_search = "\"agents\":[";
    size_t agents_start = json.find(agents_search);
    if (agents_start != std::string::npos) {
        agents_start += agents_search.length();
        size_t agents_end = agents_start;
        int bracket_count = 1;
        bool in_string = false;
        bool escaped = false;
        
        for (size_t i = agents_start; i < json.length() && bracket_count > 0; ++i) {
            char c = json[i];
            
            if (escaped) {
                escaped = false;
                continue;
            }
            if (c == '\\') {
                escaped = true;
                continue;
            }
            if (c == '"') {
                in_string = !in_string;
                continue;
            }
            
            if (!in_string) {
                if (c == '[') bracket_count++;
                else if (c == ']') bracket_count--;
            }
            
            agents_end = i;
        }
        
        std::string agents_json = json.substr(agents_start, agents_end - agents_start);
        
        // Parse individual agent objects
        size_t pos = 0;
        int brace_count = 0;
        size_t obj_start = 0;
        in_string = false;
        escaped = false;
        
        for (size_t i = 0; i <= agents_json.length(); ++i) {
            if (i == agents_json.length() || 
                (!in_string && brace_count == 0 && (agents_json[i] == ',' || i == agents_json.length()))) {
                
                if (i > obj_start) {
                    std::string agent_json = agents_json.substr(obj_start, i - obj_start);
                    if (!agent_json.empty() && agent_json != ",") {
                        state.agents.push_back(deserializeAgent(agent_json));
                    }
                }
                
                // Skip comma and whitespace
                while (i + 1 < agents_json.length() && 
                       (agents_json[i + 1] == ',' || std::isspace(agents_json[i + 1]))) {
                    i++;
                }
                obj_start = i + 1;
                continue;
            }
            
            char c = agents_json[i];
            
            if (escaped) {
                escaped = false;
                continue;
            }
            if (c == '\\') {
                escaped = true;
                continue;
            }
            if (c == '"') {
                in_string = !in_string;
                continue;
            }
            
            if (!in_string) {
                if (c == '{') brace_count++;
                else if (c == '}') brace_count--;
            }
        }
    }
    
    // Similar parsing for bases array
    std::string bases_search = "\"bases\":[";
    size_t bases_start = json.find(bases_search);
    if (bases_start != std::string::npos) {
        bases_start += bases_search.length();
        size_t bases_end = bases_start;
        int bracket_count = 1;
        bool in_string = false;
        bool escaped = false;
        
        for (size_t i = bases_start; i < json.length() && bracket_count > 0; ++i) {
            char c = json[i];
            
            if (escaped) {
                escaped = false;
                continue;
            }
            if (c == '\\') {
                escaped = true;
                continue;
            }
            if (c == '"') {
                in_string = !in_string;
                continue;
            }
            
            if (!in_string) {
                if (c == '[') bracket_count++;
                else if (c == ']') bracket_count--;
            }
            
            bases_end = i;
        }
        
        std::string bases_json = json.substr(bases_start, bases_end - bases_start);
        
        // Parse individual base objects
        size_t pos = 0;
        int brace_count = 0;
        size_t obj_start = 0;
        in_string = false;
        escaped = false;
        
        for (size_t i = 0; i <= bases_json.length(); ++i) {
            if (i == bases_json.length() || 
                (!in_string && brace_count == 0 && (bases_json[i] == ',' || i == bases_json.length()))) {
                
                if (i > obj_start) {
                    std::string base_json = bases_json.substr(obj_start, i - obj_start);
                    if (!base_json.empty() && base_json != ",") {
                        state.bases.push_back(deserializeBase(base_json));
                    }
                }
                
                // Skip comma and whitespace
                while (i + 1 < bases_json.length() && 
                       (bases_json[i + 1] == ',' || std::isspace(bases_json[i + 1]))) {
                    i++;
                }
                obj_start = i + 1;
                continue;
            }
            
            char c = bases_json[i];
            
            if (escaped) {
                escaped = false;
                continue;
            }
            if (c == '\\') {
                escaped = true;
                continue;
            }
            if (c == '"') {
                in_string = !in_string;
                continue;
            }
            
            if (!in_string) {
                if (c == '{') brace_count++;
                else if (c == '}') brace_count--;
            }
        }
    }
    
    // Parse config object
    std::string config_search = "\"config\":";
    size_t config_start = json.find(config_search);
    if (config_start != std::string::npos) {
        config_start += config_search.length();
        size_t config_end = json.find("}", config_start);
        if (config_end != std::string::npos) {
            std::string config_json = json.substr(config_start, config_end - config_start + 1);
            state.config = deserializeGameConfig(config_json);
        }
    }
    
    // Parse simple fields
    state.current_turn = extractIntValue(json, "current_turn");
    state.game_over = extractBoolValue(json, "game_over");
    state.winner = extractStringValue(json, "winner");
    
    return state;
}

} // namespace rpc