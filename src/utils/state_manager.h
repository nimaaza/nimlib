#pragma once

#include <chrono>
#include <utility>
#include <unordered_map>
#include <vector>

using state_change_point = std::chrono::steady_clock::time_point;

template <typename T>
class StateManager
{
public:
    StateManager(
        T initial_state,
        T error_state,
        const std::unordered_map<T, std::vector<T>>& state_transition_map,
        int max_reset_count
    );
    ~StateManager() = default;

    StateManager(const StateManager&) = delete;
    StateManager& operator=(const StateManager&) = delete;
    StateManager(StateManager&&) noexcept = delete;
    StateManager& operator=(StateManager&&) noexcept = delete;

    bool ready_to_transition(T next_state) const;
    T set_state(T);
    T reset_state();
    const std::pair<T, long> get_state_pair() const;
    const T get_state() const;
    bool starting() const;
    bool in_error() const;

private:
    T state;
    const T initial_state;
    const T error_state;
    const std::unordered_map<T, std::vector<T>>& state_transition_map;
    const int max_reset_count;
    int reset_count;
    state_change_point last_state_change;
};

template<typename T>
StateManager<T>::StateManager(
    T initial_state,
    T error_state,
    const std::unordered_map<T, std::vector<T>>& state_transition_map,
    int max_reset_count
) :
    state{ initial_state },
    initial_state{ initial_state },
    error_state{ error_state },
    state_transition_map{ state_transition_map },
    max_reset_count{ max_reset_count },
    reset_count{},
    last_state_change{ std::chrono::steady_clock::now() }
{}

template<typename T>
bool StateManager<T>::ready_to_transition(T next_state) const
{
    if (next_state == error_state) return true;

    auto next_states = state_transition_map.find(state)->second;
    for (auto s : next_states)
    {
        if (s == next_state) return true;
    }

    return false;
}

template<typename T>
T StateManager<T>::set_state(T new_state)
{
    if (new_state != state)
    {
        state = new_state;
        reset_count = 0;
        last_state_change = std::chrono::steady_clock::now();
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

    last_state_change = std::chrono::steady_clock::now();

    return state;
}

template<typename T>
const std::pair<T, long> StateManager<T>::get_state_pair() const
{
    auto now = std::chrono::steady_clock::now();
    long elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(now - last_state_change).count();
    return { state, elapsed };
}

template<typename T>
const T StateManager<T>::get_state() const { return state; }

template<typename T>
bool StateManager<T>::starting() const { return state == initial_state; }

template<typename T>
bool StateManager<T>::in_error() const { return state == error_state; }
