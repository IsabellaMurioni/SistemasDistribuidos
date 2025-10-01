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

game::GameState deserializeGameState(const std::string& json);

// Acá pueden armar todo lo relacionado con responder a las llamadas RPC
// y hacer la request de registro del agente.

} // namespace rpc