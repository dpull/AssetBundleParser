
#include "gtest/gtest.h"

extern "C" {
    #include "utils/platform.h"
    #include "assetbundle.h"
}

class assetbundle_test : public testing::Test
{
    
};

TEST_F(assetbundle_test, test_unity_4_6)
{
    struct assetbundle* assetbundle = ::assetbundle_load("TestData/4.6_ios_scene_1");
    ASSERT_TRUE(assetbundle);
    
    ASSERT_TRUE(::assetbundle_check(assetbundle));
    
    ::assetbundle_destory(assetbundle);
}

