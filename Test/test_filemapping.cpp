
#include "gtest/gtest.h"

extern "C" {
    #include "utils/platform.h"
    #include "filemapping.h"
}

class filemapping_test : public testing::Test
{
    
};

TEST_F(filemapping_test, test_read_write)
{
    const char* filename = "TestData/test_tmp";
    size_t file_length = 1024;
    struct filemapping* filemapping = ::filemapping_create_readwrite(filename, file_length);
    ASSERT_TRUE(filemapping);
    
    unsigned char* data = filemapping_getdata(filemapping);
    for (size_t i = 0; i < file_length; ++i) {
        data[i] = i * 550084 % 256;
    }
    
    ::filemapping_destory(filemapping);
    
    filemapping = ::filemapping_create_readonly(filename);
    ASSERT_TRUE(filemapping);
    ASSERT_EQ(file_length, ::filemapping_getlength(filemapping));
    
    data = filemapping_getdata(filemapping);
    for (size_t i = 0; i < file_length; ++i) {
        ASSERT_EQ(i * 550084 % 256, data[i]);
    }
    ::filemapping_destory(filemapping);
}

void test_file_size(const char* filename, size_t file_length)
{
    struct filemapping* filemapping = ::filemapping_create_readonly(filename);
    ASSERT_TRUE(filemapping);
    ASSERT_EQ(file_length, ::filemapping_getlength(filemapping));
    ::filemapping_destory(filemapping);
}

TEST_F(filemapping_test, test_readwrite_truncate)
{
	const char* filename = "TestData/test_tmp";
	size_t file_length = 1024;

	struct filemapping* filemapping = ::filemapping_create_readwrite(filename, file_length);
	ASSERT_TRUE(filemapping);
	::filemapping_destory(filemapping);

	test_file_size(filename, file_length);

	file_length /= 10;
	filemapping = ::filemapping_create_readwrite(filename, file_length);
	ASSERT_TRUE(filemapping);
	::filemapping_destory(filemapping);
	test_file_size(filename, file_length);

	file_length *= 100;
	filemapping = ::filemapping_create_readwrite(filename, file_length);
	ASSERT_TRUE(filemapping);
	::filemapping_destory(filemapping);
	test_file_size(filename, file_length);
}

TEST_F(filemapping_test, test_truncate)
{
    const char* filename = "TestData/test_tmp";
    size_t file_length = 1024;
    
    struct filemapping* filemapping = ::filemapping_create_readwrite(filename, file_length);
    ASSERT_TRUE(filemapping);
    ::filemapping_destory(filemapping);
    
    test_file_size(filename, file_length);
    
    file_length /= 10;
    filemapping_truncate(filename, file_length);
    test_file_size(filename, file_length);
    
    file_length *= 100;
    filemapping_truncate(filename, file_length);
    test_file_size(filename, file_length);
}