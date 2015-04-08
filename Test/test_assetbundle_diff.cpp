
#include "gtest/gtest.h"

extern "C" {
    #include "utils/platform.h"
    #include "utils/traversedir.h"
    #include "filemapping.h"
    #include "assetbundle_diff.h"
}

class assetbundle_diff_test : public testing::Test
{
    
};

bool readfile_in_dir(unsigned char* buffer, const char* filename, size_t offset, size_t length, void* userdata)
{
    char* dir = (char*)userdata;
    char filename_buffer[512];
    snprintf(filename_buffer, sizeof(filename_buffer), "%s/%s", dir, filename);
    
    struct filemapping* filemapping = filemapping_create_readonly(filename_buffer);
    if (!filemapping)
        return false;
    size_t file_length = filemapping_getlength(filemapping);
    if (file_length < offset + length) {
        filemapping_destory(filemapping);
        return false;
    }
    
    unsigned char* data = filemapping_getdata(filemapping);
    memcpy(buffer, data + offset, length);
    filemapping_destory(filemapping);
    return true;
}

TEST_F(assetbundle_diff_test, test_diff_dir)
{
    ASSERT_TRUE(::assetbundle_diff("TestData/4.6_ios_player", NULL, "TestData/4.6_ios_scene_1", "TestData/diff") == ASSETBUNDLE_SUCCEED);
    ASSERT_TRUE(::assetbundle_merge(readfile_in_dir, (void*)"TestData/4.6_ios_player", NULL, "TestData/4.6_ios_scene_1_tmp", "TestData/diff") == ASSETBUNDLE_SUCCEED);
    
    struct filemapping* file1 = ::filemapping_create_readonly("TestData/4.6_ios_scene_1");
    ASSERT_TRUE(file1 != NULL);
    
    struct filemapping* file2 = ::filemapping_create_readonly("TestData/4.6_ios_scene_1_tmp");
    ASSERT_TRUE(file2 != NULL);
    
    ASSERT_TRUE(::filemapping_getlength(file1) == ::filemapping_getlength(file2));
    ASSERT_TRUE(memcmp(::filemapping_getdata(file1), ::filemapping_getdata(file2), ::filemapping_getlength(file1)) == 0);
    
    ::filemapping_destory(file1);
    ::filemapping_destory(file2);
}

TEST_F(assetbundle_diff_test, test_diff_file)
{
    ASSERT_TRUE(::assetbundle_diff(NULL, "TestData/4.6_ios_scene_1", "TestData/4.6_ios_scene_2", "TestData/diff") == ASSETBUNDLE_SUCCEED);
    ASSERT_TRUE(::assetbundle_merge(readfile_in_dir, NULL, "TestData/4.6_ios_scene_1", "TestData/4.6_ios_scene_2_tmp", "TestData/diff") == ASSETBUNDLE_SUCCEED);
    
    struct filemapping* file1 = ::filemapping_create_readonly("TestData/4.6_ios_scene_2");
    ASSERT_TRUE(file1 != NULL);
    
    struct filemapping* file2 = ::filemapping_create_readonly("TestData/4.6_ios_scene_2_tmp");
    ASSERT_TRUE(file2 != NULL);
    
    ASSERT_TRUE(::filemapping_getlength(file1) == ::filemapping_getlength(file2));
    ASSERT_TRUE(memcmp(::filemapping_getdata(file1), ::filemapping_getdata(file2), ::filemapping_getlength(file1)) == 0);
    
    ::filemapping_destory(file1);
    ::filemapping_destory(file2);
}
