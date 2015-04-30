
#include "gtest/gtest.h"

extern "C" {
    #include "utils/platform.h"
    #include "assetbundle.h"
}

class assetbundle_test : public testing::Test
{
    
};

TEST_F(assetbundle_test, test_unity_4_6_ios)
{
    struct assetbundle* assetbundle = ::assetbundle_load("TestData/4.6_ios_scene_1");
    ASSERT_TRUE(assetbundle);
    
    ASSERT_TRUE(::assetbundle_check(assetbundle));
    
    ::assetbundle_destroy(assetbundle);
}

TEST_F(assetbundle_test, test_unity_4_6_osx)
{
    struct assetbundle* assetbundle = ::assetbundle_load("TestData/4.6_osx_scene_1");
    ASSERT_TRUE(assetbundle);
    
    ASSERT_TRUE(::assetbundle_check(assetbundle));
    
    ::assetbundle_destroy(assetbundle);
}

/*
not support unity 5.x
TEST_F(assetbundle_test, test_unity_5_0_ios)
{
    struct assetbundle* assetbundle = ::assetbundle_load("TestData/5.0_ios_assetbundle_1");
    ASSERT_TRUE(assetbundle);
    
    ASSERT_TRUE(::assetbundle_check(assetbundle));
    
    ::assetbundle_destroy(assetbundle);
}
*/