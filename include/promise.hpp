#ifndef ASYNCPP_PROMISE_HPP
#define ASYNCPP_PROMISE_HPP

#include "unhandled_promise_rejection.hpp"

#include <exception>
#include <functional>
#include <iostream>
#include <mutex>
#include <string_view>
#include <thread>

template <typename T>
class Promise {
public:
    using ResolveFunction_t = std::function<void(const T&)>;
    using RejectFunction_t = std::function<void(std::string_view)>;
    using ExecutorFunction_t = std::function<void(ResolveFunction_t, RejectFunction_t)>;

    /*
     * Not 100% sure about the explicit keyword here as
     * we might want to implicitly convert executor to Promise?
     *
     * Also, I don't think users should be able to set start_now to false.
     * Need to change later
     */
    explicit Promise(ExecutorFunction_t executor_func, bool start_now = true);
    ~Promise() { clean_thread(); }

    Promise() = delete;
    Promise<T>(const Promise<T>&) = delete;
    Promise<T>& operator=(const Promise<T>&) = delete;

    Promise<T>& then(ResolveFunction_t on_resolve, RejectFunction_t on_reject = nullptr);
    Promise<T>& catch_error(RejectFunction_t on_reject) { return then(nullptr, on_reject); }

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

    ExecutorFunction_t m_executor_func;
    ResolveFunction_t m_on_resolve_callback;
    RejectFunction_t  m_on_reject_callback;

    void _resolve(const T& value);
    void _reject(std::string_view reason);

    void _handle_exception(std::string_view);
    
    void start_work();
    void clean_thread();
};

template <typename T>
Promise<T>::Promise(ExecutorFunction_t executor_func, bool start_now)
    : m_executor_func(executor_func)
{
    // m_executor_func = executor_func;
    if (start_now) start_work();
}

template <typename T>
void Promise<T>::_handle_exception(std::string_view message) {
    bool has_reject_callback;
    {
        std::lock_guard<std::mutex> lock(m_state_mutex);
        has_reject_callback = bool(m_on_reject_callback);
    }
    if (has_reject_callback) _reject(message);
    else {
        clean_thread();
        throw;
    }
}

template <typename T>
void Promise<T>::start_work() {
    m_thread = std::thread([this]() {
        try {
            m_executor_func(
                    [this](const T& value){ this->_resolve(value); },
                    [this](std::string_view reason){ this->_reject(reason); }
            );
        } catch (std::exception& e) {
            _handle_exception(e.what());
        } catch (...) {
            _handle_exception("Non-exception type thrown");
        }
    });
}

template <typename T>
void Promise<T>::clean_thread() {
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
void Promise<T>::_reject(std::string_view reason) {
    std::lock_guard<std::mutex> lock(m_state_mutex);
    if (m_state != State::pending) return;

    m_state = State::rejected;

    if (m_on_reject_callback) {
        m_on_reject_callback(reason);
        m_on_reject_callback = nullptr;
    } else throw UnhandledPromiseRejection(reason);
}

#endif // ASYNCPP_PROMISE_HPP
