#include "../include/promise.hpp"

#include <chrono>
#include <thread>

int main() {
    Promise<int> promise([](Promise<int>::ResolveFunction_t resolve, Promise<int>::RejectFunction_t reject) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        resolve(69);
    });

    promise.debug();
    std::this_thread::sleep_for(std::chrono::milliseconds(600));
    promise.debug();

    return 0;
}
