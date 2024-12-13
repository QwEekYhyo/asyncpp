#ifndef ASYNCPP_UNHANDLED_PROMISE_REJECTION_HPP
#define ASYNCPP_UNHANDLED_PROMISE_REJECTION_HPP

#include <stdexcept>
#include <string_view>

class UnhandledPromiseRejection : public std::runtime_error {
public:
    explicit UnhandledPromiseRejection(const std::string& message)
        : std::runtime_error(message) {}

    explicit UnhandledPromiseRejection(const char* message)
        : std::runtime_error(message) {}

    explicit UnhandledPromiseRejection(std::string_view message)
        : std::runtime_error(std::string(message)) {}
};

#endif // ASYNCPP_UNHANDLED_PROMISE_REJECTION_HPP
