#include "../include/promise.hpp"

#include <chrono>
#include <thread>

int main() {
    Promise<int> promise([](Promise<int>::ResolveFunction_t resolve, Promise<int>::RejectFunction_t reject) {
        std::cout << "trying to resolve\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        reject();
        resolve(69);
    });

    promise.then(
            [](const int& res){
                std::cout << "Promise resolved with value: " << res << std::endl;
            }
    )
    .catch_error(
            [](){
                std::cout << "Promise was rejected\n";
            }
    );

    std::this_thread::sleep_for(std::chrono::milliseconds(600));
    promise.debug();

    return 0;
}
