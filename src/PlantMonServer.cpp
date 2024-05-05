#include <fstream>
#include <iostream>
#include "crow.h"

using namespace std;
using namespace crow;

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

int main() {
    // Crow app instance
    SimpleApp app;

    // Endpoint for pushing sensor data
    CROW_ROUTE(app, "/push")
    .methods("POST"_method)
    ([](const request& req) {
        // Get JSON data from the request body
        json::rvalue json = crow::json::load(req.body);
        if (!json) {
            return response(400, "Invalid JSON format");
        }
        // Append sensor data to the text file
        appendSensorDataToFile(json::wvalue(json).dump());
        
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

    // Run the server on port 8080
    app.port(8080).multithreaded().run();

    return 0;
}

