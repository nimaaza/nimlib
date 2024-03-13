#include <gtest/gtest.h>

#include "../src/state_manager.h"

enum States { START, OK, CON_ERROR, SOME_STATE, ANOTHER_STATE };

TEST(StateManagerTest_Construction, InitialState_SetToStart)
{
    StateManager<States> sm{States::START, States::CON_ERROR, 2 };

    EXPECT_EQ(sm.get_state().first, States::START);
}

TEST(StateManagerTest_SetState, StateChange)
{
    StateManager<States> sm{States::START, States::CON_ERROR, 2 };
    sm.set_state(States::SOME_STATE);

    EXPECT_EQ(sm.get_state().first, States::SOME_STATE);
}

TEST(StateManagerTest_SetState, ResetCountZeroedOnStateChange)
{
    StateManager<States> sm{States::START, States::CON_ERROR, 2 };

    sm.set_state(States::SOME_STATE);
    sm.set_state(States::SOME_STATE);
    sm.set_state(States::SOME_STATE);
    auto state1 = sm.get_state().first;

    sm.set_state(States::ANOTHER_STATE);
    sm.set_state(States::ANOTHER_STATE);
    sm.set_state(States::ANOTHER_STATE);
    auto state2 = sm.get_state().first;

    sm.set_state(States::ANOTHER_STATE);
    auto state3 = sm.get_state().first;

    EXPECT_EQ(state1, States::SOME_STATE);
    EXPECT_EQ(state2, States::ANOTHER_STATE);
    EXPECT_EQ(state3, States::CON_ERROR);
}

TEST(StateManagerTest_SetState, StateNotNew)
{
    StateManager<States> sm{States::START, States::CON_ERROR, 3 };
    sm.set_state(States::SOME_STATE);

    // The following will be considered state resets and are allowed
    // the first three times.
    sm.set_state(States::SOME_STATE);
    sm.set_state(States::SOME_STATE);
    sm.set_state(States::SOME_STATE);
    auto state_before_limit = sm.get_state().first;
    sm.set_state(States::SOME_STATE);
    auto state_after_limit = sm.get_state().first;

    EXPECT_EQ(state_before_limit, States::SOME_STATE);
    EXPECT_EQ(state_after_limit, States::CON_ERROR);
}

TEST(StateManagerTest_ResetState, StateShouldNotChange)
{
    StateManager<States> sm{States::START, States::CON_ERROR, 2 };

    auto state_before = sm.get_state().first;
    sm.reset_state();
    auto state_after = sm.get_state().first;

    EXPECT_EQ(state_before, state_after);
}

TEST(StateManagerTest_ResetState, MaxResetCountReached)
{
    StateManager<States> sm{States::START, States::CON_ERROR, 4 };

    sm.set_state(States::SOME_STATE);
    sm.reset_state();
    sm.reset_state();
    sm.reset_state();
    sm.reset_state();
    auto state_before_reached = sm.get_state().first;
    sm.reset_state();
    auto state_after_reached = sm.get_state().first;

    EXPECT_EQ(state_before_reached, States::SOME_STATE);
    EXPECT_EQ(state_after_reached, States::CON_ERROR);
}
