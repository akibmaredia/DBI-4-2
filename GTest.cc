
#include "Statistics.h"
#include <gtest/gtest.h>


TEST(AddRelFail, Run) {
    Statistics file;
    ASSERT_EQ(file.AddRel(NULL,0),0);
}

TEST(AddRelSuccess, Run) {
    Statistics file;
    ASSERT_EQ(file.AddRel("lineitem",0),1);
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}