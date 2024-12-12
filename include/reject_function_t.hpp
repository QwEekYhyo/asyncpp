#ifndef ASYNCPP_REJECT_FUNCTION_T_HPP
#define ASYNCPP_REJECT_FUNCTION_T_HPP

#include <cstddef>
#include <functional>
#include <string>

/*
 * I was thinking about using this wrapper to allow call to reject()
 * with no reason and this class would add a default reason
 * but I scratched the idea
 */
class RejectFunction_t final {
private:
    using func_t = std::function<void(const std::string&)>;
    func_t m_func;

public:
    void operator()(const std::string& str = "unspecified reason") {
        m_func(str);
    }

    explicit operator bool() const {
        return bool(m_func);
    }

    RejectFunction_t(std::nullptr_t) : m_func(nullptr) {}

    RejectFunction_t& operator=(std::nullptr_t) noexcept {
        m_func = nullptr;
        return *this;
    }
};

#endif // ASYNCPP_REJECT_FUNCTION_T_HPP
