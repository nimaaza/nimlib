#include <gtest/gtest.h>

#include "../../src/utils/state_manager.h"

#include <unordered_map>
#include <ctime>

using nimlib::Server::Utils::StateManager;

enum States { START, OK, ERROR_STATE, SOME_STATE, ANOTHER_STATE };

std::unordered_map<States, std::vector<States>> transitions
{
    {States::START, {States::OK}},
    {States::OK, {States::SOME_STATE, States::ANOTHER_STATE, States::START}},
    {States::SOME_STATE, {States::ANOTHER_STATE, States::SOME_STATE}},
    {States::ANOTHER_STATE, {States::ANOTHER_STATE}},
};

const std::unordered_map<States, int> empty_max_reset_counts{};

const std::unordered_map<States, long> empty_time_outs{};

// TODO: timeouts and max_reset_count should have no effect when in error state.

TEST(StateManagerTest_Construction, InitialState_SetToStart)
{
    StateManager<States> sm{ States::START, States::ERROR_STATE, transitions, empty_max_reset_counts, empty_time_outs };

    EXPECT_EQ(sm.get_state(), States::START);
}

TEST(StateManagerTest_Transitions, StatesTransitToErrorWhenTimedOut)
{
    std::unordered_map<States, long> time_outs{ {States::SOME_STATE, 10} };
    StateManager<States> sm{ States::START, States::ERROR_STATE, transitions, {}, time_outs };
    sm.set_state(States::SOME_STATE);

    // Wait 10 milliseconds.
    timespec t1, t2;
    t1.tv_sec = 0L;
    t1.tv_nsec = 10'000'000L;
    if (nanosleep(&t1, &t2) < 0)
    {
        // TODO: failed to sleep for some reason and will cause the test to fail
    }

    EXPECT_FALSE(sm.can_transition_to(States::ANOTHER_STATE));
    EXPECT_EQ(sm.get_state(), States::ERROR_STATE);
}

TEST(StateManagerTest_Transitions, AllStatesTransitToError)
{
    StateManager<States> sm{ States::START, States::ERROR_STATE, transitions, empty_max_reset_counts, empty_time_outs };

    sm.set_state(States::START);
    EXPECT_TRUE(sm.can_transition_to(States::ERROR_STATE));

    sm.set_state(States::SOME_STATE);
    EXPECT_TRUE(sm.can_transition_to(States::ERROR_STATE));

    sm.set_state(States::ANOTHER_STATE);
    EXPECT_TRUE(sm.can_transition_to(States::ERROR_STATE));

    sm.set_state(States::ERROR_STATE);
    EXPECT_TRUE(sm.can_transition_to(States::ERROR_STATE));
}

TEST(StateManagerTest_Transitions, TransitionsCheck)
{
    StateManager<States> sm{ States::START, States::ERROR_STATE, transitions, empty_max_reset_counts, empty_time_outs };

    EXPECT_FALSE(sm.can_transition_to(States::START));
    EXPECT_TRUE(sm.can_transition_to(States::OK));
    EXPECT_FALSE(sm.can_transition_to(States::SOME_STATE));
    EXPECT_FALSE(sm.can_transition_to(States::ANOTHER_STATE));

    sm.set_state(States::OK);
    EXPECT_TRUE(sm.can_transition_to(States::START));
    EXPECT_FALSE(sm.can_transition_to(States::OK));
    EXPECT_TRUE(sm.can_transition_to(States::SOME_STATE));
    EXPECT_TRUE(sm.can_transition_to(States::ANOTHER_STATE));

    sm.set_state(States::SOME_STATE);
    EXPECT_FALSE(sm.can_transition_to(States::START));
    EXPECT_FALSE(sm.can_transition_to(States::OK));
    EXPECT_TRUE(sm.can_transition_to(States::ANOTHER_STATE));
    EXPECT_TRUE(sm.can_transition_to(States::SOME_STATE));

    sm.set_state(States::ANOTHER_STATE);
    EXPECT_FALSE(sm.can_transition_to(States::START));
    EXPECT_FALSE(sm.can_transition_to(States::OK));
    EXPECT_TRUE(sm.can_transition_to(States::ANOTHER_STATE));
    EXPECT_FALSE(sm.can_transition_to(States::SOME_STATE));
}

TEST(StateManagerTest_Transitions, UnspecifiedTransition)
{
    enum OtherStates { O_START, O_OK, O_ERROR_STATE, O_SOME_STATE, O_ANOTHER_STATE, UNSPEC_TRANSITION };

    std::unordered_map<OtherStates, std::vector<OtherStates>> other_transitions
    {
        {OtherStates::O_START, {OtherStates::O_OK}},
        {OtherStates::O_OK, {OtherStates::O_SOME_STATE, OtherStates::O_ANOTHER_STATE, OtherStates::O_START}},
        {OtherStates::O_SOME_STATE, {OtherStates::O_ANOTHER_STATE, OtherStates::O_SOME_STATE}},
        {OtherStates::O_ANOTHER_STATE, {}},
    };
    std::unordered_map<OtherStates, int> other_empty_max_reset_counts{};
    std::unordered_map<OtherStates, long> other_empty_time_outs{};

    StateManager<OtherStates> sm
    {
        OtherStates::O_START,
        OtherStates::O_ERROR_STATE,
        other_transitions,
        other_empty_max_reset_counts,
        other_empty_time_outs
    };
    sm.set_state(OtherStates::UNSPEC_TRANSITION);

    EXPECT_FALSE(sm.can_transition_to(OtherStates::UNSPEC_TRANSITION));
}

TEST(StateManagerTest_SetState, NotPossibleWhenInError)
{
    StateManager<States> sm{ States::START, States::ERROR_STATE, transitions, empty_max_reset_counts, empty_time_outs };
    sm.set_state(States::ERROR_STATE);
    sm.set_state(States::ANOTHER_STATE);

    EXPECT_EQ(sm.get_state(), States::ERROR_STATE);
}

TEST(StateManagerTest_SetState, StateChangeWhenCanTransitionTo)
{
    StateManager<States> sm{ States::START, States::ERROR_STATE, transitions, empty_max_reset_counts, empty_time_outs };
    // From START cannot transition to ANOTHER_STATE.
    sm.set_state(States::OK);

    EXPECT_EQ(sm.get_state(), States::OK);
}

TEST(StateManagerTest_SetState, ErrorWhenCannotTransitionTo)
{
    StateManager<States> sm{ States::START, States::ERROR_STATE, transitions, empty_max_reset_counts, empty_time_outs };
    // From START cannot transition to ANOTHER_STATE.
    sm.set_state(States::ANOTHER_STATE);

    EXPECT_EQ(sm.get_state(), States::ERROR_STATE);
}

TEST(StateManagerTest_SetState, WhenTimedOut)
{
    std::unordered_map<States, long> time_outs{ {States::SOME_STATE, 10} };
    StateManager<States> sm{ States::START, States::ERROR_STATE, transitions, {}, time_outs };
    sm.set_state(States::SOME_STATE);

    // Wait 10 milliseconds.
    timespec t1, t2;
    t1.tv_sec = 0L;
    t1.tv_nsec = 10'000'000L;
    if (nanosleep(&t1, &t2) < 0)
    {
        // TODO: failed to sleep for some reason and will cause the test to fail
    }

    sm.set_state(States::ANOTHER_STATE);

    EXPECT_EQ(sm.get_state(), States::ERROR_STATE);
}

TEST(StateManagerTest_SetState, WhenMoreThanMaxResetCount)
{
    std::unordered_map<States, int> max_reset_counts{ {States::SOME_STATE, 5} };
    StateManager<States> sm{ States::START, States::ERROR_STATE, transitions, max_reset_counts, empty_time_outs };
    // State is set to OK so other transitions are possible.
    sm.set_state(States::OK);
    sm.set_state(States::SOME_STATE);

    auto state_before = sm.get_state();
    // Set state more than five times
    sm.set_state(States::SOME_STATE); // 1
    sm.set_state(States::SOME_STATE); // 2
    sm.set_state(States::SOME_STATE); // 3
    sm.set_state(States::SOME_STATE); // 4
    sm.set_state(States::SOME_STATE); // 5
    sm.set_state(States::SOME_STATE);
    auto state_after = sm.get_state();

    EXPECT_EQ(state_before, States::SOME_STATE);
    EXPECT_EQ(state_after, States::ERROR_STATE);
}

TEST(StateManagerTest_SetState, ResetCountZeroedOnStateTransition)
{
    std::unordered_map<States, int> max_reset_counts
    {
        {States::SOME_STATE, 2},
        {States::ANOTHER_STATE, 3}
    };

    StateManager<States> sm{ States::START, States::ERROR_STATE, transitions, max_reset_counts, empty_time_outs };
    // State is set to OK so other transitions are possible.
    sm.set_state(States::OK);

    sm.set_state(States::SOME_STATE);
    sm.set_state(States::SOME_STATE);
    sm.set_state(States::SOME_STATE);
    auto state1 = sm.get_state();

    sm.set_state(States::ANOTHER_STATE);
    sm.set_state(States::ANOTHER_STATE);
    sm.set_state(States::ANOTHER_STATE);
    sm.set_state(States::ANOTHER_STATE);
    auto state2 = sm.get_state();

    sm.set_state(States::ANOTHER_STATE);
    auto state3 = sm.get_state();

    EXPECT_EQ(state1, States::SOME_STATE);
    EXPECT_EQ(state2, States::ANOTHER_STATE);
    EXPECT_EQ(state3, States::ERROR_STATE);
}

TEST(StateManagerTest_SetState, StateNotNew)
{
    std::unordered_map<States, int> max_reset_counts
    {
        {States::SOME_STATE, 3},
    };

    StateManager<States> sm{ States::START, States::ERROR_STATE, transitions, max_reset_counts, empty_time_outs };
    // State is set to OK so other transitions are possible.
    sm.set_state(States::OK);
    sm.set_state(States::SOME_STATE);

    // The following will be considered state resets and are allowed
    // the first three times.
    sm.set_state(States::SOME_STATE);
    sm.set_state(States::SOME_STATE);
    sm.set_state(States::SOME_STATE);
    auto state_before_limit = sm.get_state();
    sm.set_state(States::SOME_STATE);
    auto state_after_limit = sm.get_state();

    EXPECT_EQ(state_before_limit, States::SOME_STATE);
    EXPECT_EQ(state_after_limit, States::ERROR_STATE);
}

TEST(StateManagerTest_ResetState, StateShouldNotChangeUponReset)
{
    StateManager<States> sm{ States::START, States::ERROR_STATE, transitions, empty_max_reset_counts, empty_time_outs };

    auto state_before = sm.get_state();
    sm.reset_state();
    auto state_after = sm.get_state();

    EXPECT_EQ(state_before, state_after);
}

TEST(StateManagerTest_ResetState, MaxResetCountReached)
{
    std::unordered_map<States, int> max_reset_counts
    {
        {States::SOME_STATE, 4},
    };

    StateManager<States> sm{ States::START, States::ERROR_STATE, transitions, max_reset_counts, empty_time_outs };
    // State is set to OK so other transitions are possible.
    sm.set_state(States::OK);

    sm.set_state(States::SOME_STATE);
    sm.reset_state();
    sm.reset_state();
    sm.reset_state();
    sm.reset_state();
    auto state_before_reached = sm.get_state();
    sm.reset_state();
    auto state_after_reached = sm.get_state();

    EXPECT_EQ(state_before_reached, States::SOME_STATE);
    EXPECT_EQ(state_after_reached, States::ERROR_STATE);
}

TEST(StateManagerTest_ResetState, NoEffectWhenMaxResetCountNotPresent)
{
    std::unordered_map<States, int> max_reset_counts
    {
        {States::ANOTHER_STATE, 4},
    };

    StateManager<States> sm{ States::START, States::ERROR_STATE, transitions, max_reset_counts, empty_time_outs };
    // State is set to OK so other transitions are possible.
    sm.set_state(States::OK);

    for (int i = 0; i < 100; i++)
    {
        sm.set_state(States::SOME_STATE);
    }

    EXPECT_EQ(sm.get_state(), States::SOME_STATE);

    sm.set_state(States::ANOTHER_STATE);

    // This new state can be set up to 4 times without problem.
    for (int i = 0; i < 4; i++)
    {
        sm.set_state(States::ANOTHER_STATE);
        EXPECT_EQ(sm.get_state(), States::ANOTHER_STATE);
    }

    // The 5th time results to an error state.
    sm.set_state(States::ANOTHER_STATE);
    EXPECT_EQ(sm.get_state(), States::ERROR_STATE);

}

TEST(StateManagerTest_ResetState, NotEffectiveIfTimedOut)
{
    std::unordered_map<States, long> time_outs
    {
        { States::OK, 10 },
    };

    StateManager<States> sm{ States::START, States::ERROR_STATE, transitions, {}, time_outs };
    sm.set_state(States::OK);

    EXPECT_EQ(sm.get_state(), States::OK);

    // Wait 10 milliseconds.
    timespec t1, t2;
    t1.tv_sec = 0L;
    t1.tv_nsec = 10'000'000L;
    if (nanosleep(&t1, &t2) < 0)
    {
        // TODO: failed to sleep for some reason and will cause the test to fail
    }

    sm.reset_state();

    EXPECT_EQ(sm.get_state(), States::ERROR_STATE);
}

TEST(StateManagerTest_TimeOuts, InErrorAfterTimeOut)
{
    std::unordered_map<States, long> time_outs
    {
        { States::OK, 10 },
        { States::ANOTHER_STATE, 1 }
    };

    {
        StateManager<States> sm_1{ States::START, States::ERROR_STATE, transitions, {}, time_outs };
        sm_1.set_state(States::OK);

        // Wait 10 milliseconds.
        timespec t1, t2;
        t1.tv_sec = 0L;
        t1.tv_nsec = 10'000'000L;
        if (nanosleep(&t1, &t2) < 0)
        {
            // TODO: failed to sleep for some reason and will cause the test to fail
        }

        EXPECT_EQ(sm_1.get_state(), States::ERROR_STATE);
    }

    {
        StateManager<States> sm_2{ States::START, States::ERROR_STATE, transitions, {}, time_outs };
        sm_2.set_state(States::ANOTHER_STATE);

        // Wait 1 millisecond.
        timespec t1, t2;
        t1.tv_sec = 0L;
        t1.tv_nsec = 1'000'000L;
        if (nanosleep(&t1, &t2) < 0)
        {
            // TODO: failed to sleep for some reason and will cause the test to fail
        }

        EXPECT_EQ(sm_2.get_state(), States::ERROR_STATE);
    }
}

TEST(StateManagerTest_TimeOuts, NoTimeOut)
{
    std::unordered_map<States, long> time_outs
    {
        { States::OK, 10 },
        { States::ANOTHER_STATE, 1 }
    };

    StateManager<States> sm{ States::START, States::ERROR_STATE, transitions, {}, time_outs };
    // State is set to OK so other transitions are possible.
    sm.set_state(States::OK);

    sm.set_state(States::SOME_STATE);

    // Wait long enough. The state States::SOME_STATE has no timeout, so state should
    // end up in an error state no matter how long has passed.
    timespec t1, t2;
    t1.tv_sec = 0L;
    t1.tv_nsec = 1'000'000'000L;
    if (nanosleep(&t1, &t2) < 0)
    {
        // TODO: failed to sleep for some reason and will cause the test to fail
    }

    EXPECT_EQ(sm.get_state(), States::SOME_STATE);
}
