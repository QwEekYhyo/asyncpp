#ifndef ASYNCPP_PROMISE_HPP
#define ASYNCPP_PROMISE_HPP

#include "unhandled_promise_rejection.hpp"

#include <functional>
#include <iostream>
#include <mutex>
#include <thread>

template <typename T>
class Promise {
public:
    using ResolveFunction_t = std::function<void(const T&)>;
    using RejectFunction_t = std::function<void()>;
    using ExecutorFunction_t = std::function<void(ResolveFunction_t, RejectFunction_t)>;

    /*
     * Not 100% sure about the explicit keyword here as
     * we might want to implicitly convert executor to Promise?
     */
    explicit Promise<T>(ExecutorFunction_t executor_func);
    ~Promise();

    Promise<T>(const Promise<T>&) = delete;
    Promise<T>& operator=(const Promise<T>&) = delete;

    Promise<T>& then(ResolveFunction_t on_resolve, RejectFunction_t on_reject = nullptr);

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
    std::mutex m_state_mutex;
    T m_value;
    std::thread m_thread;

    ResolveFunction_t m_on_resolve_callback;
    RejectFunction_t  m_on_reject_callback;

    void _resolve(const T& value);
    void _reject();
};

template <typename T>
Promise<T>::Promise(ExecutorFunction_t executor_func) {
    m_thread = std::thread([executor_func, this]() {
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
Promise<T>& Promise<T>::then(ResolveFunction_t on_resolve, RejectFunction_t on_reject) {
    std::lock_guard<std::mutex> lock(m_state_mutex);

    if (m_state == State::fulfilled)
        on_resolve(m_value);
    else if (m_state == State::pending) {
        m_on_resolve_callback = on_resolve;
        m_on_reject_callback = on_reject;
    }

    return *this;
}

template <typename T>
void Promise<T>::_resolve(const T& value) {
    std::lock_guard<std::mutex> lock(m_state_mutex);
    if (m_state != State::pending) return;

    m_state = State::fulfilled;
    m_value = value;

    if (m_on_resolve_callback) {
        m_on_resolve_callback(m_value);
        m_on_resolve_callback = nullptr;
    }
}

template <typename T>
void Promise<T>::_reject() {
    std::lock_guard<std::mutex> lock(m_state_mutex);
    if (m_state != State::pending) return;

    m_state = State::rejected;

    if (m_on_reject_callback) {
        m_on_reject_callback();
        m_on_reject_callback = nullptr;
    } else throw UnhandledPromiseRejection("Some reason");
}

#endif // ASYNCPP_PROMISE_HPP
