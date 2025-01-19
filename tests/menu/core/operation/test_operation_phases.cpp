#include <gtest/gtest.h>

extern "C" {
#include "users/steve973/menu/core/operation/menu_operation.h"
#include "users/steve973/menu/core/operation/operation_types.h"
#include "users/steve973/menu/core/state/menu_state.h"
#include "users/steve973/menu/core/structure/menu_item.h"
}

class MenuOperationPhasesTest : public ::testing::Test {
protected:
    void SetUp() override {
        static const operation_config_t test_operation = {
            .action = "test_action",
            .precondition = NULL,
            .inputs = NULL,
            .input_count = 0,
            .confirm = NULL,
            .result = NULL,
            .postcondition = NULL,
            .precondition_display = NULL,
            .input_display = NULL,
            .confirm_display = NULL,
            .action_display = NULL,
            .result_display = NULL,
            .postcondition_display = NULL
        };

        static const menu_item_t test_item = {
            .label = "Test Item",
            .label_short = "Test",
            .icon = NULL,
            .shortcut = NULL,
            .help_text = NULL,
            .type = MENU_TYPE_ACTION,
            .operation = test_operation,
            .conditions = {0},
            .children = NULL,
            .child_count = 0,
            .screen_content = NULL,
            .display = NULL
        };

        item = &test_item;
    }

    const menu_item_t* item;
};

// Test that operation starts correctly with choice_made initialized to -1
TEST_F(MenuOperationPhasesTest, StartInitializesChoiceMadeToNegativeOne) {
    start_operation(item);
    EXPECT_EQ(get_operation_context()->choice_made, -1);
}

// Test that operation starts in PHASE_NONE
TEST_F(MenuOperationPhasesTest, StartInitializesPhaseToNone) {
    start_operation(item);
    EXPECT_EQ(get_current_operation_phase(), OPERATION_PHASE_NONE);
}

// Test that execute_operation advances from NONE to first required phase
TEST_F(MenuOperationPhasesTest, ExecuteAdvancesFromNoneToAction) {
    start_operation(item);
    execute_operation();
    // Since our test item has no precondition/input/confirm, should go straight to ACTION
    EXPECT_EQ(get_current_operation_phase(), OPERATION_PHASE_ACTION);
}

// Test that operation is considered in progress until complete
TEST_F(MenuOperationPhasesTest, OperationInProgressUntilComplete) {
    start_operation(item);
    EXPECT_TRUE(is_operation_in_progress());

    // Force operation to complete
    cancel_operation(false);
    EXPECT_FALSE(is_operation_in_progress());
}

// Test cancellation
TEST_F(MenuOperationPhasesTest, CancellationSetsCorrectState) {
    start_operation(item);
    cancel_operation(false);
    EXPECT_EQ(get_current_operation_phase(), OPERATION_PHASE_COMPLETE);
    EXPECT_FALSE(is_operation_in_progress());
}
