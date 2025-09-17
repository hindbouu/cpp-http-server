#include <iostream>
#include <string>
#include <sstream>
#include <thread>
#include <vector>
#include <map>
#include <functional>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <chrono>
#include <iomanip>

class SimpleHttpServer {
private:
    int port;
    int server_socket;
    bool running = false;
    
    // Route handler type
    using Handler = std::function<std::string(const std::string&)>;
    std::map<std::string, Handler> routes;
    
    std::string getCurrentTime() {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
        return ss.str();
    }
    
    std::string parseUrl(const std::string& request) {
        std::istringstream iss(request);
        std::string method, url, version;
        iss >> method >> url >> version;
        return url;
    }
    
    std::string createHttpResponse(const std::string& content, const std::string& contentType = "text/html") {
        std::ostringstream response;
        response << "HTTP/1.1 200 OK\r\n";
        response << "Content-Type: " << contentType << "\r\n";
        response << "Content-Length: " << content.length() << "\r\n";
        response << "Connection: close\r\n";
        response << "Access-Control-Allow-Origin: *\r\n";
        response << "\r\n";
        response << content;
        return response.str();
    }
    
    void handleClient(int client_socket) {
        char buffer[4096] = {0};
        ssize_t bytes_read = read(client_socket, buffer, sizeof(buffer) - 1);
        
        if (bytes_read > 0) {
            std::string request(buffer);
            std::string url = parseUrl(request);
            
            // Log the request
            std::cout << "[" << getCurrentTime() << "] GET " << url << std::endl;
            
            std::string response_content;
            std::string content_type = "text/html";
            
            // Route matching
            if (routes.find(url) != routes.end()) {
                response_content = routes[url](request);
                if (url.find("/api/") == 0) {
                    content_type = "application/json";
                }
            } else {
                response_content = "<h1>404 - Page Not Found</h1><p>The requested URL " + url + " was not found.</p>";
            }
            
            std::string http_response = createHttpResponse(response_content, content_type);
            send(client_socket, http_response.c_str(), http_response.length(), 0);
        }
        
        close(client_socket);
    }
    
public:
    explicit SimpleHttpServer(int p) : port(p) {
        setupRoutes();
    }
    
    void setupRoutes() {
        // Home page
        routes["/"] = [](const std::string&) -> std::string {
            return R"(
<!DOCTYPE html>
<html>
<head>
    <title>Simple C++ HTTP Server</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 40px; background: #f5f5f5; }
        .container { background: white; padding: 30px; border-radius: 10px; box-shadow: 0 2px 10px rgba(0,0,0,0.1); }
        h1 { color: #333; border-bottom: 3px solid #007acc; padding-bottom: 10px; }
        .endpoint { background: #f8f9fa; padding: 15px; margin: 10px 0; border-radius: 5px; border-left: 4px solid #007acc; }
        a { color: #007acc; text-decoration: none; font-weight: bold; }
        a:hover { text-decoration: underline; }
        .success { color: #28a745; }
    </style>
</head>
<body>
    <div class="container">
        <h1>🚀 Simple C++ HTTP Server</h1>
        <p class="success">✅ Server is running successfully!</p>
        <p>This is a <strong>fully working HTTP server</strong> written in C++ from scratch.</p>
        
        <h2>Available Endpoints:</h2>
        
        <div class="endpoint">
            <strong>🏠 <a href="/">Home Page</a></strong><br>
            <code>GET /</code> - This page you're viewing now
        </div>
        
        <div class="endpoint">
            <strong>❤️ <a href="/api/health">Health Check</a></strong><br>
            <code>GET /api/health</code> - JSON health status
        </div>
        
        <div class="endpoint">
            <strong>👤 <a href="/api/user/123">User API</a></strong><br>
            <code>GET /api/user/123</code> - Get user information (try different numbers!)
        </div>
        
        <div class="endpoint">
            <strong>ℹ️ <a href="/about">About</a></strong><br>
            <code>GET /about</code> - Information about this server
        </div>
        
        <div class="endpoint">
            <strong>⏰ <a href="/time">Current Time</a></strong><br>
            <code>GET /time</code> - Get current server time
        </div>
        
        <h2>Features Demonstrated:</h2>
        <ul>
            <li>✅ Socket programming in C++</li>
            <li>✅ HTTP request parsing</li>
            <li>✅ Multi-threaded request handling</li>
            <li>✅ JSON API responses</li>
            <li>✅ HTML page serving</li>
            <li>✅ Request logging</li>
            <li>✅ CORS headers</li>
        </ul>
        
        <p><em>Check your terminal to see real-time request logs!</em></p>
    </div>
</body>
</html>
            )";
        };
        
        // Health check API
        routes["/api/health"] = [this](const std::string&) -> std::string {
            auto timestamp = std::chrono::system_clock::now().time_since_epoch().count();
            return R"({"status":"healthy","server":"Simple C++ HTTP Server","timestamp":)" + 
                   std::to_string(timestamp) + R"(,"uptime":"running"})";
        };
        
        // User API - simulates getting user data
        routes["/api/user/123"] = [](const std::string&) -> std::string {
            return R"({"id":123,"name":"John Doe","email":"john@example.com","active":true})";
        };
        
        routes["/api/user/456"] = [](const std::string&) -> std::string {
            return R"({"id":456,"name":"Jane Smith","email":"jane@example.com","active":true})";
        };
        
        routes["/api/user/789"] = [](const std::string&) -> std::string {
            return R"({"id":789,"name":"Bob Johnson","email":"bob@example.com","active":false})";
        };
        
        // About page
        routes["/about"] = [](const std::string&) -> std::string {
            return R"(
<!DOCTYPE html>
<html>
<head>
    <title>About - C++ HTTP Server</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 40px; background: #f5f5f5; }
        .container { background: white; padding: 30px; border-radius: 10px; box-shadow: 0 2px 10px rgba(0,0,0,0.1); }
        h1 { color: #333; }
        .code { background: #f8f9fa; padding: 10px; border-radius: 5px; font-family: monospace; }
    </style>
</head>
<body>
    <div class="container">
        <h1>📋 About This Server</h1>
        <p>This is a <strong>simple but fully functional HTTP server</strong> written in C++ from scratch.</p>
        
        <h2>Technical Details:</h2>
        <ul>
            <li><strong>Language:</strong> C++17</li>
            <li><strong>Socket API:</strong> POSIX sockets</li>
            <li><strong>Threading:</strong> std::thread for concurrent handling</li>
            <li><strong>HTTP:</strong> HTTP/1.1 protocol</li>
            <li><strong>Features:</strong> Routing, JSON APIs, static HTML</li>
        </ul>
        
        <h2>What It Demonstrates:</h2>
        <ul>
            <li>Low-level socket programming</li>
            <li>HTTP protocol implementation</li>
            <li>Multi-threaded server architecture</li>
            <li>Modern C++ practices</li>
            <li>Web API development</li>
        </ul>
        
        <p><a href="/">← Back to Home</a></p>
    </div>
</body>
</html>
            )";
        };
        
        // Time endpoint
        routes["/time"] = [this](const std::string&) -> std::string {
            return R"(
<!DOCTYPE html>
<html>
<head>
    <title>Server Time</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 40px; background: #f5f5f5; text-align: center; }
        .container { background: white; padding: 30px; border-radius: 10px; box-shadow: 0 2px 10px rgba(0,0,0,0.1); }
        .time { font-size: 2em; color: #007acc; font-weight: bold; }
    </style>
</head>
<body>
    <div class="container">
        <h1>⏰ Current Server Time</h1>
        <div class="time">)" + getCurrentTime() + R"(</div>
        <p>This page was generated dynamically by the C++ server!</p>
        <p><a href="/">← Back to Home</a></p>
    </div>
</body>
</html>
            )";
        };
    }
    
    void start() {
        // Create socket
        server_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (server_socket < 0) {
            throw std::runtime_error("Failed to create socket");
        }
        
        // Allow socket reuse
        int opt = 1;
        setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
        
        // Setup address
        struct sockaddr_in address;
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(port);
        
        // Bind socket
        if (bind(server_socket, (struct sockaddr*)&address, sizeof(address)) < 0) {
            throw std::runtime_error("Failed to bind to port " + std::to_string(port));
        }
        
        // Listen for connections
        if (listen(server_socket, 10) < 0) {
            throw std::runtime_error("Failed to listen on socket");
        }
        
        running = true;
        std::cout << "\n🚀 Simple C++ HTTP Server started!" << std::endl;
        std::cout << "📱 Open your browser to: http://localhost:" << port << std::endl;
        std::cout << "🔍 Watch this terminal for request logs..." << std::endl;
        std::cout << "⏹️  Press Ctrl+C to stop the server\n" << std::endl;
        
        // Accept connections
        while (running) {
            struct sockaddr_in client_address;
            socklen_t client_len = sizeof(client_address);
            
            int client_socket = accept(server_socket, (struct sockaddr*)&client_address, &client_len);
            
            if (client_socket >= 0) {
                // Handle each request in a separate thread
                std::thread client_thread(&SimpleHttpServer::handleClient, this, client_socket);
                client_thread.detach(); // Let thread run independently
            }
        }
    }
    
    ~SimpleHttpServer() {
        if (server_socket >= 0) {
            close(server_socket);
        }
    }
};

int main() {
    try {
        SimpleHttpServer server(8080);
        server.start();
    } catch (const std::exception& e) {
        std::cerr << "❌ Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
