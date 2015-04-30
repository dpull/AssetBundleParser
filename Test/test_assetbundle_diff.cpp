
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

bool API_CALLBACK readfile_in_dir(unsigned char* buffer, const char* filename, size_t offset, size_t length, void* userdata)
{
    char* dir = (char*)userdata;
    char filename_buffer[512];
    snprintf(filename_buffer, sizeof(filename_buffer), "%s/%s", dir, filename);
    
    struct filemapping* filemapping = filemapping_create_readonly(filename_buffer);
    if (!filemapping)
        return false;
    size_t file_length = filemapping_getlength(filemapping);
    if (file_length < offset + length) {
        filemapping_destroy(filemapping);
        return false;
    }
    
    unsigned char* data = filemapping_getdata(filemapping);
    memcpy(buffer, data + offset, length);
    filemapping_destroy(filemapping);
    return true;
}

TEST_F(assetbundle_diff_test, test_diff_dir)
{
    ASSERT_EQ(ASSETBUNDLE_SUCCEED, ::assetbundle_diff("TestData/4.6_ios_player", NULL, "TestData/4.6_ios_scene_1", "TestData/diff"));
    ASSERT_EQ(ASSETBUNDLE_SUCCEED, ::assetbundle_merge(readfile_in_dir, (void*)"TestData/4.6_ios_player", NULL, "TestData/4.6_ios_scene_1_tmp", "TestData/diff"));
    
    struct filemapping* file1 = ::filemapping_create_readonly("TestData/4.6_ios_scene_1");
    ASSERT_TRUE(file1);
    
    struct filemapping* file2 = ::filemapping_create_readonly("TestData/4.6_ios_scene_1_tmp");
    ASSERT_TRUE(file2);
    
    ASSERT_EQ(::filemapping_getlength(file1), ::filemapping_getlength(file2));
    ASSERT_EQ(0, memcmp(::filemapping_getdata(file1), ::filemapping_getdata(file2), ::filemapping_getlength(file1)));
    
    ::filemapping_destroy(file1);
    ::filemapping_destroy(file2);
}

TEST_F(assetbundle_diff_test, test_diff_file)
{
    ASSERT_EQ(ASSETBUNDLE_SUCCEED, ::assetbundle_diff(NULL, "TestData/4.6_ios_scene_1", "TestData/4.6_ios_scene_2", "TestData/diff"));
    ASSERT_EQ(ASSETBUNDLE_SUCCEED, ::assetbundle_merge(readfile_in_dir, NULL, "TestData/4.6_ios_scene_1", "TestData/4.6_ios_scene_2_tmp", "TestData/diff"));
    
    struct filemapping* file1 = ::filemapping_create_readonly("TestData/4.6_ios_scene_2");
    ASSERT_TRUE(file1);
    
    struct filemapping* file2 = ::filemapping_create_readonly("TestData/4.6_ios_scene_2_tmp");
    ASSERT_TRUE(file2);
    
    ASSERT_EQ(::filemapping_getlength(file1), ::filemapping_getlength(file2));
    ASSERT_EQ(0, memcmp(::filemapping_getdata(file1), ::filemapping_getdata(file2), ::filemapping_getlength(file1)));
    
    ::filemapping_destroy(file1);
    ::filemapping_destroy(file2);
}

TEST_F(assetbundle_diff_test, hero_crash_20150423)
{
    ASSERT_EQ(ASSETBUNDLE_SUCCEED, ::assetbundle_diff("TestData/iPhone/Client/Payload/ProductName.app/Data", "TestData/iPhone/Patch/20150420_112809.asset", "TestData/iPhone/Patch/20150420_163057.asset", "TestData/iPhone/tmp.diff"));
    ASSERT_EQ(ASSETBUNDLE_SUCCEED, ::assetbundle_merge(readfile_in_dir, (void*)"TestData/iPhone/Client/Payload/ProductName.app/Data", "TestData/iPhone/Patch/20150420_112809.asset", "TestData/iPhone/Patch/20150420_163057.tmp", "TestData/iPhone/tmp.diff"));
    
    struct filemapping* file1 = ::filemapping_create_readonly("TestData/iPhone/Patch/20150420_163057.asset");
    ASSERT_TRUE(file1);
    
    struct filemapping* file2 = ::filemapping_create_readonly("TestData/iPhone/Patch/20150420_163057.tmp");
    ASSERT_TRUE(file2);
    
    ASSERT_EQ(::filemapping_getlength(file1), ::filemapping_getlength(file2));
    ASSERT_EQ(0, memcmp(::filemapping_getdata(file1), ::filemapping_getdata(file2), ::filemapping_getlength(file1)));
    
    ::filemapping_destroy(file1);
    ::filemapping_destroy(file2);
}

