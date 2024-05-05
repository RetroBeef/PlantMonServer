#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include "crow.h"

using namespace std;
using namespace crow;

struct Config {
    std::string crtPath;
    std::string keyPath;
    std::vector<std::string> acceptedTokens;
};

std::string loadfile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + filename);
    }

    std::ostringstream oss;
    oss << file.rdbuf();
    return oss.str();
}

bool parseConfig(const std::string& configFile, Config& config) {
    crow::json::rvalue root;

    try {
        root = crow::json::load(loadfile(configFile));
    } catch (std::exception& e) {
        std::cerr << "Failed to parse config file: " << e.what() << std::endl;
        return false;
    }

    if (!root.has("crtPath") || root["crtPath"].t() != crow::json::type::String) {
        std::cerr << "Missing or invalid crtPath in config" << std::endl;
        return false;
    }
    config.crtPath = std::move(root["crtPath"].s());

    if (!root.has("keyPath") || root["keyPath"].t() != crow::json::type::String) {
        std::cerr << "Missing or invalid keyPath in config" << std::endl;
        return false;
    }
    config.keyPath = std::move(root["keyPath"].s());

    if (!root.has("acceptedTokens")) {
        std::cerr << "Missing acceptedTokens in config" << std::endl;
        return false;
    }
    if (root["acceptedTokens"].t() != crow::json::type::List) {
        std::cerr << "acceptedTokens must be a list in config" << std::endl;
        return false;
    }
    for (const auto& token : root["acceptedTokens"]) {
        if (token.t() != crow::json::type::String) {
            std::cerr << "Invalid token in acceptedTokens" << std::endl;
            return false;
        }
        config.acceptedTokens.push_back(std::move(token.s()));
    }

    return true;
}

// Function to append sensor data to a text file
void appendSensorDataToFile(const std::string& data) {
    std::ofstream file("sensor_data.txt", std::ios_base::app);
    if (file.is_open()) {
        file << data << endl;
        file.close();
    } else {
        cerr << "Error: Unable to open file for appending sensor data." << endl;
    }
}

int main(int argc, char* argv[]) {
    Config config;
    if (parseConfig("config.json", config)) {
        std::cout << "Configuration loaded successfully:" << std::endl;
        std::cout << "CRT Path: " << config.crtPath << std::endl;
        std::cout << "Key Path: " << config.keyPath << std::endl;
        std::cout << "Accepted Tokens:" << std::endl;
        for (const auto& token : config.acceptedTokens) {
            std::cout << "- " << token << std::endl;
        }
    } else {
        std::cerr << "Failed to load configuration" << std::endl;
        return 1;
    }
    // Crow app instance
    SimpleApp app;

    app.ssl_file(config.crtPath, config.keyPath);

    // Endpoint for pushing sensor data
    CROW_ROUTE(app, "/push")
    .methods("POST"_method)
    ([&config](const request& req) {
        std::string token = req.get_header_value("token");
        if(!token.size()) return response(401);
        bool found = false;
        for(const auto& t : config.acceptedTokens){
            if(!t.compare(token)){
                found = true;
                break;
            }
        }
        if(!found) return response(401);
        // Get JSON data from the request body
        json::rvalue json = crow::json::load(req.body);
        if (!json) {
            return response(400, "Invalid JSON format");
        }
        // Append sensor data to the text file
        std::string item = json::wvalue(json).dump();
        appendSensorDataToFile(item);
        std::cout << "appending " << item << std::endl;
        
        return response(200);
    });

    // Endpoint for pulling sensor data
    CROW_ROUTE(app, "/pull")
    .methods("GET"_method)
    ([]() {
        std::ifstream file("sensor_data.txt");
        if (file.is_open()) {
            std::stringstream buffer;
            buffer << file.rdbuf();
            return response(200, buffer.str());
        } else {
            return response(500, "Error: Unable to open file for reading sensor data.");
        }
    });

    // Run the server
    app.port(44300).multithreaded().run();

    return 0;
}

