#pragma once

#include <chrono>
#include <utility>
#include <unordered_map>
#include <vector>

namespace nimlib::Server::Utils
{
    using state_change_point = std::chrono::steady_clock::time_point;

    template <typename T>
    class StateManager
    {
    public:
        StateManager(
            T initial_state,
            T error_state,
            const std::unordered_map<T, std::vector<T>>& state_transition_map,
            const std::unordered_map<T, int>& max_reset_counts,
            const std::unordered_map<T, long>& msec_time_outs
        );
        ~StateManager() = default;

        StateManager(const StateManager&) = delete;
        StateManager& operator=(const StateManager&) = delete;
        StateManager(StateManager&&) noexcept = delete;
        StateManager& operator=(StateManager&&) noexcept = delete;

        bool can_transition_to(T next_state) const;
        T set_state(T);
        T reset_state();
        void clear();
        const T get_state();
        bool starting() const;
        bool in_error() const;

    private:
        bool timed_out() const;

    private:
        T state;
        const T initial_state;
        const T error_state;
        const std::unordered_map<T, std::vector<T>>& transition_map;
        const std::unordered_map<T, long>& msec_time_outs;
        const std::unordered_map<T, int>& max_reset_counts;
        int reset_count;
        state_change_point last_state_change;
    };

    template<typename T>
    StateManager<T>::StateManager(
        T initial_state,
        T error_state,
        const std::unordered_map<T, std::vector<T>>& state_transition_map,
        const std::unordered_map<T, int>& max_reset_counts,
        const std::unordered_map<T, long>& msec_time_outs
    ) :
        state{ initial_state },
        initial_state{ initial_state },
        error_state{ error_state },
        transition_map{ state_transition_map },
        max_reset_counts{ max_reset_counts },
        reset_count{},
        msec_time_outs{ msec_time_outs },
        last_state_change{ std::chrono::steady_clock::now() }
    {}

    template<typename T>
    bool StateManager<T>::can_transition_to(T next_state) const
    {
        if (timed_out())
        {
            return false;
        }
        else if (next_state == error_state)
        {
            return true;
        }
        else if (auto it = transition_map.find(state); it != transition_map.end())
        {
            const auto& next_states = it->second;
            for (auto s : next_states)
            {
                if (s == next_state) return true;
            }
        }

        return false;
    }

    template<typename T>
    T StateManager<T>::set_state(T new_state)
    {
        if (state != error_state)
        {
            if (timed_out())
            {
                state = error_state;
            }
            else if (new_state != state)
            {
                state = can_transition_to(new_state) ? new_state : error_state;
                reset_count = 0;
            }
            else
            {
                reset_state();
            }
        }

        last_state_change = std::chrono::steady_clock::now();
        return state;
    }

    template<typename T>
    T StateManager<T>::reset_state()
    {
        // When in error state or when the current state has timed out, no reason
        // to continue with reset_state().
        if (state == error_state || timed_out())
        {
            state = error_state;
        }
        else
        {
            reset_count++;

            if (auto it = max_reset_counts.find(state); it != max_reset_counts.end())
            {
                if (reset_count > it->second) state = error_state;
            }
        }

        last_state_change = std::chrono::steady_clock::now();
        return state;
    }

    template<typename T>
    void StateManager<T>::clear()
    {
        state = initial_state;
        reset_count = 0;
        last_state_change = std::chrono::steady_clock::now();
    }

    template<typename T>
    const T StateManager<T>::get_state()
    {
        if (timed_out()) set_state(error_state);
        return state;
    }

    template<typename T>
    bool StateManager<T>::starting() const { return get_state() == initial_state; }

    template<typename T>
    bool StateManager<T>::in_error() const { return get_state() == error_state; }

    template<typename T>
    bool StateManager<T>::timed_out() const
    {
        // When no timeout has been defined, the return value is always false.
        if (msec_time_outs.empty()) return false;

        // When in error state, timeout makes no sense.
        if (state == error_state) return false;

        using namespace std::chrono;

        if (auto it = msec_time_outs.find(state); it != msec_time_outs.end())
        {
            auto diff = duration_cast<milliseconds>(steady_clock::now() - last_state_change).count();
            return diff >= it->second;
        }
        else
        {
            return false;
        }
    }
};

