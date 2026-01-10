#include "common/tcp_connection.h"
#include "common/rpc_protocol.h"
#include <iostream>
#include "logic/logic.h"
#include <string>
#include "common/game_state.h"
#include <fstream>
using namespace std ;
int main(int argc, char* argv[]) {
    string host = "127.0.0.1";
    int port = 8080;
    string agent_id = "backup_agent_id";
    string call_id ="1";
    if (argc > 1) {
        host = argv[1];
    }
    if (argc > 2) {
        port = stoi(argv[2]);
    }
    if (argc > 3) {
        agent_id = argv[3];
    }  
    

    
    // Create and connect the TCP client
    net::TcpConnection connection;
    connection.connect(host, port);
    cout << "Client running..." << (connection.isConnected())<< endl;

    // Register the agent
    string registermessage =  "{"
        "\"id\":\"" + call_id + "\","
        "\"type\":\"register_agent\","
        "\"agent_id\":\"" + agent_id + "\""
        "}";
    connection.sendMessage(registermessage);
    agent::SimpleAgent my_agent;

    // Main loop to handle server messages
    while (true)
    {
        string response = connection.receiveMessage();
        string id = rpc :: extractStringValue (response, "id");
        string type = rpc :: extractStringValue (response, "type");

        if (rpc::extractStringValue(response, "type") == "recieve_intel") {
            connection.sendMessage(rpc::void_response(id));    
        }
        else if (rpc::extractStringValue(response, "type") == "play_turn"){ 
            game::GameState game_state;
            agent :: SimpleAction redditben10; 
            try {
                game_state = rpc ::deserializeGameState(response); 
                int turn = game_state.current_turn;
                if (turn == 1) {
                    string team_name = "default_team";
                    for (auto& agents : game_state.agents) {
                        if (agents.id == agent_id) {
                            team_name = agents.team;
                            break;
                        }
                    }
                    my_agent.initialize(agent_id, team_name);
                }
                redditben10 = my_agent.processTurn(game_state);

            } catch (const exception& e) {
                cout << "Error deserializing game state: " << e.what() << endl;
            }
            if (redditben10.type == agent::SimpleActionType::move) {
                string direction_str = game::getStringFromDirection(redditben10.direction);
                ofstream meteorologicLog("/meteorologic.txt"); 

                connection.sendMessage(rpc::turn_response(id, "move_" + direction_str));
            }
            else if (redditben10.type == agent::SimpleActionType::attack) {
                string direction_str = game::getStringFromDirection(redditben10.direction);
                connection.sendMessage(rpc::turn_response(id, "attack_" + direction_str));
            }
            else if (redditben10.type == agent::SimpleActionType::defend) {
                string direction_str = game::getStringFromDirection(redditben10.direction);
                connection.sendMessage(rpc::turn_response(id, "defend_" + direction_str));
            }
            else if (redditben10.type == agent::SimpleActionType::send_message) {
                connection.sendMessage(rpc::turn_response(id, "send_message:" + redditben10.message));
            }    
        }
        else if (rpc::extractStringValue(response, "type") == "notify_game_over") {
            cout << "Game Over received. Exiting..." << endl;
            break;
        }
    }
}