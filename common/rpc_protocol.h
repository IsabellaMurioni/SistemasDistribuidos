// common/rpc_protocol.h
// Declare the functions and structures for the rpc_protocol.cpp
#pragma once
#include <string>
#include <memory>
#include "game_state.h"
#include "tcp_connection.h"
#include <thread>
#include <unordered_map>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <chrono>
#include <future>

namespace rpc {

// Helper functions for JSON parsing
std::string escapeJson(const std::string& str);
std::string extractStringValue(const std::string& json, const std::string& key);
int extractIntValue(const std::string& json, const std::string& key);
bool extractBoolValue(const std::string& json, const std::string& key);
std::string void_response(const std::string& id);
std::string turn_response(const std::string& id, const std::string& action);
std::string register_message(const std::string& id, const std::string& agent_id);
game::GameState deserializeGameState(const std::string& json);

// Acá pueden armar todo lo relacionado con responder a las llamadas RPC
// y hacer la request de registro del agente.

} // namespace rpc