#pragma once

#include <chrono>
#include <utility>

using state_change_point = std::chrono::steady_clock::time_point;

template <typename T>
class StateManager
{
public:
    StateManager(T initial_state, T error_state, int max_reset_count);
    ~StateManager() = default;

    StateManager(const StateManager&) = delete;
    StateManager& operator=(const StateManager&) = delete;
    StateManager(StateManager&&) noexcept = delete;
    StateManager& operator=(StateManager&&) noexcept = delete;

    T set_state(T);
    T reset_state();
    const std::pair<T, long> get_state() const;
private:
    T state;
    T error_state;
    int max_reset_count;
    int reset_count;
    state_change_point last_state_change;
};

template<typename T>
StateManager<T>::StateManager(T initial_state, T error_state, int max_reset_count)
    : state{ initial_state },
    error_state{ error_state },
    max_reset_count{ max_reset_count },
    reset_count{},
    last_state_change{ std::chrono::high_resolution_clock::now() }
{}

template<typename T>
T StateManager<T>::set_state(T new_state)
{
    if (new_state != state)
    {
        state = new_state;
        reset_count = 0;
        last_state_change = std::chrono::high_resolution_clock::now();
        return state;
    }
    else
    {
        return reset_state();
    }
}

template<typename T>
T StateManager<T>::reset_state()
{
    reset_count++;

    if (reset_count > max_reset_count)
    {
        set_state(error_state);
    }

    last_state_change = std::chrono::high_resolution_clock::now();

    return state;
}

template<typename T>
const std::pair<T, long> StateManager<T>::get_state() const
{
    auto now = std::chrono::high_resolution_clock::now();
    long elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(now - last_state_change).count();
    return { state, elapsed };
}
