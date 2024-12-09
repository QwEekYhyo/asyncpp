#ifndef ASYNCPP_PROMISE_HPP
#define ASYNCPP_PROMISE_HPP

#include <functional>
#include <iostream>
#include <thread>

template <typename T>
class Promise {
public:
    using ResolveFunction_t = std::function<void(const T&)>;
    using RejectFunction_t = std::function<void()>;
    using ExecutorFunction_t = std::function<void(ResolveFunction_t, RejectFunction_t)>;

    Promise<T>(ExecutorFunction_t executor_func);
    ~Promise();

    Promise<T>(const Promise<T>&) = delete;
    Promise<T>& operator=(const Promise<T>&) = delete;

    void debug() const {
        const char* str = m_state == State::pending ? "pending" : (m_state == State::fulfilled ? "fulfilled" : "rejected");
        std::cout << "value: " << m_value << ", state: "
        << str << std::endl;
    }

private:
    enum class State {
        pending,
        fulfilled,
        rejected
    };

    State m_state{State::pending};
    T m_value;
    std::thread m_thread;

    void _resolve(const T& value);
    void _reject();
};

template <typename T>
Promise<T>::Promise(ExecutorFunction_t executor_func) {
    m_thread = std::thread([&executor_func, this]() {
        executor_func(
                [this](const T& value){ this->_resolve(value); },
                [this](){ this->_reject(); }
        );
    });
}

template <typename T>
Promise<T>::~Promise() {
    if (m_thread.joinable())
        m_thread.detach();
}

template <typename T>
void Promise<T>::_resolve(const T& value) {
    if (m_state == State::pending) {
        m_value = value;
        m_state = State::fulfilled;
    }
}

template <typename T>
void Promise<T>::_reject() {
    if (m_state == State::pending)
        m_state = State::rejected;
}

#endif // ASYNCPP_PROMISE_HPP
