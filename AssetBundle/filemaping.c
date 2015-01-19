#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#ifdef _MSC_VER
#else
#include <unistd.h>
#include <sys/mman.h>
#endif


#ifdef _MSC_VER
struct filemaping
{
	int a;
};

struct filemaping* filemaping_create_readonly(const char* file)
{
	return NULL;
}

struct filemaping* filemaping_create_readwrite(const char* file, size_t length)
{
	return NULL;
}

void filemaping_destory(struct filemaping* filemaping)
{

}

unsigned char* filemaping_getdata(struct filemaping* filemaping)
{
	return NULL;
}

size_t filemaping_getlength(struct filemaping* filemaping)
{
	return 0;
}
#else
struct filemaping
{
	unsigned char* data;
	size_t length;
};

struct filemaping* filemaping_create_readonly(const char* file)
{
	int fd = open(file, O_RDONLY);
	if (fd == -1)
		return NULL;

	struct stat stat;
	if (fstat(fd, &stat) == -1) {
		close(fd);
		return NULL;
	}

	unsigned char* data = (unsigned char*)mmap(0, stat.st_size, PROT_READ, MAP_SHARED, fd, 0);    
	if (data == MAP_FAILED) {
		close(fd);
		return NULL;
	}

	close(fd);

	struct filemaping* filemaping = (struct filemaping*)malloc(sizeof(*filemaping));
	memset(filemaping, 0, sizeof(*filemaping));

	filemaping->data = data;
	filemaping->length = stat.st_size;
	return filemaping;
}

struct filemaping* filemaping_create_readwrite(const char* file, size_t length)
{
	int fd = open(file, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
	if (fd == -1)
		return NULL;

	if (ftruncate(fd, length) != 0){
		close(fd);
		return NULL;
	}

	unsigned char* data = (unsigned char *)mmap(0, length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0); 
	if (data == MAP_FAILED) {
		close(fd);
		return NULL;
	}

	close(fd);

	struct filemaping* filemaping = (struct filemaping*)malloc(sizeof(*filemaping));
	memset(filemaping, 0, sizeof(*filemaping));
	
	filemaping->data = data;
	filemaping->length = length;
	return filemaping;
}

void filemaping_destory(struct filemaping* filemaping)
{
	munmap(filemaping->data, filemaping->length);
	free(filemaping);
}

unsigned char* filemaping_getdata(struct filemaping* filemaping)
{
	return filemaping->data;
}

size_t filemaping_getlength(struct filemaping* filemaping)
{
	return filemaping->length;
}
#endif
