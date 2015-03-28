#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdbool.h>
#ifdef _MSC_VER
#include <Windows.h>
#else
#include <unistd.h>
#include <sys/mman.h>
#endif
#include "utils/platform.h"
#include "filemapping.h"

#ifdef _MSC_VER
struct filemapping
{
	unsigned char* data;
	size_t length;
};

struct filemapping* filemapping_create_readonly(const char* file)
{
	HANDLE handle;
	DWORD length;
	HANDLE mapping;
	PVOID data;

	handle = CreateFileA(file, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (handle == INVALID_HANDLE_VALUE)
		return NULL;

	length = GetFileSize(handle, NULL);

	mapping = CreateFileMappingA(handle, NULL, PAGE_READWRITE, 0, 0, NULL);
	CloseHandle(handle);

	if (!mapping)
		return NULL;

	data = MapViewOfFile(mapping, FILE_MAP_READ, 0, 0, 0);
	CloseHandle(mapping);

	if (!data)
		return NULL;

	struct filemapping* filemapping = (struct filemapping*)malloc(sizeof(*filemapping));
	memset(filemapping, 0, sizeof(*filemapping));

	filemapping->data = (unsigned char*)data;
	filemapping->length = (size_t)length;

	return filemapping;
}

struct filemapping* filemapping_create_readwrite(const char* file, size_t length)
{
	HANDLE handle;
	HANDLE mapping;
	PVOID data;

	handle = CreateFileA(file, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (handle == INVALID_HANDLE_VALUE)
		return NULL;

	mapping = CreateFileMappingA(handle, NULL, PAGE_READWRITE, 0, length, NULL);
	CloseHandle(handle);

	if (!mapping)
		return NULL;

	data = MapViewOfFile(mapping, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
	CloseHandle(mapping);

	if (!data)
		return NULL;

	struct filemapping* filemapping = (struct filemapping*)malloc(sizeof(*filemapping));
	memset(filemapping, 0, sizeof(*filemapping));

	filemapping->data = (unsigned char*)data;
	filemapping->length = length;

	return filemapping;
}

void filemapping_destory(struct filemapping* filemapping)
{
	UnmapViewOfFile(filemapping->data);
	free(filemapping);
}

unsigned char* filemapping_getdata(struct filemapping* filemapping)
{
	return filemapping->data;
}

size_t filemapping_getlength(struct filemapping* filemapping)
{
	return filemapping->length;
}

bool filemapping_truncate(const char* file, size_t length)
{
	BOOL ret = false;
	HANDLE handle = CreateFileA(file, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (handle == INVALID_HANDLE_VALUE)
		return false;

	if (SetFilePointer(handle, length, NULL, FILE_BEGIN))
		ret = SetEndOfFile(handle);

	CloseHandle(handle);
	return ret;
}
#else
struct filemapping
{
	unsigned char* data;
	size_t length;
};

struct filemapping* filemapping_create_readonly(const char* file)
{
	int fd = open(file, O_RDONLY);
	if (fd == -1)
		return NULL;

	struct stat stat;
	if (fstat(fd, &stat) == -1) {
		close(fd);
		return NULL;
	}

	unsigned char* data = (unsigned char*)mmap(0, (size_t)stat.st_size, PROT_READ, MAP_SHARED, fd, 0);
	if (data == MAP_FAILED) {
		close(fd);
		return NULL;
	}

	close(fd);

	struct filemapping* filemapping = (struct filemapping*)malloc(sizeof(*filemapping));
	memset(filemapping, 0, sizeof(*filemapping));

	filemapping->data = data;
	filemapping->length = (size_t)stat.st_size;
	return filemapping;
}

struct filemapping* filemapping_create_readwrite(const char* file, size_t length)
{
	int fd = open(file, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
	if (fd == -1)
		return NULL;

	if (length == 0) {
		struct stat stat;
		if (fstat(fd, &stat) == -1) {
			close(fd);
			return NULL;
		}    
		length = (size_t)stat.st_size;    
	} else {
		if (ftruncate(fd, length) != 0){
			close(fd);
			return NULL;
		}
	}

	unsigned char* data = (unsigned char *)mmap(0, length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0); 
	if (data == MAP_FAILED) {
		close(fd);
		return NULL;
	}

	close(fd);

	struct filemapping* filemapping = (struct filemapping*)malloc(sizeof(*filemapping));
	memset(filemapping, 0, sizeof(*filemapping));

	filemapping->data = data;
	filemapping->length = length;
	return filemapping;
}

void filemapping_destory(struct filemapping* filemapping)
{
	munmap(filemapping->data, filemapping->length);
	free(filemapping);
}

unsigned char* filemapping_getdata(struct filemapping* filemapping)
{
	return filemapping->data;
}

size_t filemapping_getlength(struct filemapping* filemapping)
{
	return filemapping->length;
}

bool filemapping_truncate(const char* file, size_t length)
{
	return truncate(file, length);
}
#endif
