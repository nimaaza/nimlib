#include <gtest/gtest.h>

#include "../../src/utils/state_manager.h"

enum States { START, OK, ERROR_STATE, SOME_STATE, ANOTHER_STATE };

std::unordered_map<States, std::vector<States>> transitions
{
    {States::START, {States::OK}},
    {States::OK, {States::SOME_STATE, States::ANOTHER_STATE, States::START}},
    {States::SOME_STATE, {States::ANOTHER_STATE, States::SOME_STATE}},
    {States::ANOTHER_STATE, {}},
};

TEST(StateManagerTest_Transitions, AllStatesTransitToError)
{
    StateManager<States> sm{ States::START, States::ERROR_STATE, transitions, 2 };

    sm.set_state(States::START);
    EXPECT_TRUE(sm.ready_to_transition(States::ERROR_STATE));

    sm.set_state(States::SOME_STATE);
    EXPECT_TRUE(sm.ready_to_transition(States::ERROR_STATE));

    sm.set_state(States::ANOTHER_STATE);
    EXPECT_TRUE(sm.ready_to_transition(States::ERROR_STATE));

    sm.set_state(States::ERROR_STATE);
    EXPECT_TRUE(sm.ready_to_transition(States::ERROR_STATE));
}

TEST(StateManagerTest_Transitions, TransitionsCheck)
{
    StateManager<States> sm{ States::START, States::ERROR_STATE, transitions, 2 };

    sm.set_state(States::START);
    EXPECT_FALSE(sm.ready_to_transition(States::START));
    EXPECT_TRUE(sm.ready_to_transition(States::OK));
    EXPECT_FALSE(sm.ready_to_transition(States::SOME_STATE));
    EXPECT_FALSE(sm.ready_to_transition(States::ANOTHER_STATE));

    sm.set_state(States::OK);
    EXPECT_TRUE(sm.ready_to_transition(States::START));
    EXPECT_FALSE(sm.ready_to_transition(States::OK));
    EXPECT_TRUE(sm.ready_to_transition(States::SOME_STATE));
    EXPECT_TRUE(sm.ready_to_transition(States::ANOTHER_STATE));

    sm.set_state(States::SOME_STATE);
    EXPECT_FALSE(sm.ready_to_transition(States::START));
    EXPECT_FALSE(sm.ready_to_transition(States::OK));
    EXPECT_TRUE(sm.ready_to_transition(States::ANOTHER_STATE));
    EXPECT_TRUE(sm.ready_to_transition(States::SOME_STATE));

    sm.set_state(States::ANOTHER_STATE);
    EXPECT_FALSE(sm.ready_to_transition(States::START));
    EXPECT_FALSE(sm.ready_to_transition(States::OK));
    EXPECT_FALSE(sm.ready_to_transition(States::ANOTHER_STATE));
    EXPECT_FALSE(sm.ready_to_transition(States::SOME_STATE));
}

TEST(StateManagerTest_Construction, InitialState_SetToStart)
{
    StateManager<States> sm{ States::START, States::ERROR_STATE, transitions, 2 };

    EXPECT_EQ(sm.get_state(), States::START);
}

TEST(StateManagerTest_SetState, StateChange)
{
    StateManager<States> sm{ States::START, States::ERROR_STATE, transitions, 2 };
    sm.set_state(States::SOME_STATE);

    EXPECT_EQ(sm.get_state(), States::SOME_STATE);
}

TEST(StateManagerTest_SetState, ResetCountZeroedOnStateChange)
{
    StateManager<States> sm{ States::START, States::ERROR_STATE, transitions, 2 };

    sm.set_state(States::SOME_STATE);
    sm.set_state(States::SOME_STATE);
    sm.set_state(States::SOME_STATE);
    auto state1 = sm.get_state();

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
    StateManager<States> sm{ States::START, States::ERROR_STATE, transitions, 3 };
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

TEST(StateManagerTest_ResetState, StateShouldNotChange)
{
    StateManager<States> sm{ States::START, States::ERROR_STATE, transitions, 2 };

    auto state_before = sm.get_state();
    sm.reset_state();
    auto state_after = sm.get_state();

    EXPECT_EQ(state_before, state_after);
}

TEST(StateManagerTest_ResetState, MaxResetCountReached)
{
    StateManager<States> sm{ States::START, States::ERROR_STATE, transitions, 4 };

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
