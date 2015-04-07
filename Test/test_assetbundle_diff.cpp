
#include "gtest/gtest.h"

extern "C" {
#include "utils/platform.h"
#include "utils/debug_tree.h"
#include "assetbundle.h"
#include "assetbundle_diff.h"
#include "assetfile.h"
#include "filemapping.h"
#include "field_type.h"
#include "utils/traversedir.h"
#include "assetbundle_diff.h"
}

class assetbundle_diff_test : public testing::Test
{
    
};

TEST_F(assetbundle_diff_test, test_diff)
{
    int ret = assetbundle_diff("TestData/iPhone_Client", NULL, "TestData/iPhone_AssetBundle_1.asset", "TestData/diff.asset");
    ASSERT_TRUE(ret == 0);
}

