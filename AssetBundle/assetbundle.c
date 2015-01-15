#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include "AssetBundle.h"

size_t read_string(unsigned char* data, size_t offset, char** value)
{
    size_t start = offset;
    *value = data + offset;
    while (data[offset] != '\0')
        offset++;
    return offset - start + 1;
}

size_t write_string(unsigned char* data, size_t offset, char* value)
{
    size_t index = 0;
    while (value[index] != '\0')
    {
        data[offset + index] = value[index];
        index++;
    }
    data[offset + index] = '\0';
    return index + 1;
}

size_t read_int16(unsigned char* data, size_t offset, short* value, bool littleEndian)
{
    if (!littleEndian)
        *value = (short)((data[offset] << 8) | data[offset + 1]);
    else
        *value = (short)((data[offset + 1] << 8) | data[offset]);
    return 2;
}

size_t write_int16(unsigned char* data, size_t offset, short value, bool littleEndian)
{
    if (!littleEndian)
    {
        data[offset + 1] = (unsigned char)value;
        data[offset] = (unsigned char)(value >> 8);
    }
    else
    {
        data[offset] = (unsigned char)value;
        data[offset + 1] = (unsigned char)(value >> 8);
    }
    return 2;
}

size_t read_int32(unsigned char* data, size_t offset, int* value, bool littleEndian)
{
    if (!littleEndian)
        *value = (data[offset] << 24) | (data[offset + 1] << 16) | (data[offset + 2] << 8) | data[offset + 3];
    else
        *value = (data[offset + 3] << 24) | (data[offset + 2] << 16) | (data[offset + 1] << 8) | data[offset];
    return 4;
}

size_t write_int32(unsigned char* data, size_t offset, int value, bool littleEndian)
{
    if (!littleEndian)
    {
        data[offset + 3] = (unsigned char)value;
        data[offset + 2] = (unsigned char)(value >> 8);
        data[offset + 1] = (unsigned char)(value >> 16);
        data[offset] = (unsigned char)(value >> 24);
    }
    else
    {
        data[offset] = (unsigned char)value;
        data[offset + 1] = (unsigned char)(value >> 8);
        data[offset + 2] = (unsigned char)(value >> 16);
        data[offset + 3] = (unsigned char)(value >> 24);
    }
    return 4;
}

size_t read_uint32(unsigned char* data, size_t offset, size_t* value, bool littleEndian)
{
    if (!littleEndian)
        *value = (size_t)((data[offset] << 24) | (data[offset + 1] << 16) | (data[offset + 2] << 8) | data[offset + 3]);
    else
        *value = (size_t)((data[offset + 3] << 24) | (data[offset + 2] << 16) | (data[offset + 1] << 8) | data[offset]);
    return 4;
}

size_t write_uint32(unsigned char* data, size_t offset, uint value, bool littleEndian)
{
    if (!littleEndian)
    {
        data[offset + 3] = (unsigned char)value;
        data[offset + 2] = (unsigned char)(value >> 8);
        data[offset + 1] = (unsigned char)(value >> 16);
        data[offset] = (unsigned char)(value >> 24);
    }
    else
    {
        data[offset] = (unsigned char)value;
        data[offset + 1] = (unsigned char)(value >> 8);
        data[offset + 2] = (unsigned char)(value >> 16);
        data[offset + 3] = (unsigned char)(value >> 24);
    }
    return 4;
}

size_t read_byte(unsigned char* data, size_t offset, unsigned char* value)
{
    *value = data[offset];
    return 1;
}

size_t write_byte(unsigned char* data, size_t offset, unsigned char value)
{
    data[offset] = value;
    return 1;
}

size_t read_buffer(unsigned char* data, size_t offset, unsigned char** value, size_t size)
{
    *value = data + offset;
    return size;
}

size_t write_buffer(unsigned char* data, size_t offset, unsigned char* value, size_t size)
{
    memcpy(data + offset, value, size);
    value = data + offset;
    return size;
}

#pragma pack(1)
struct level_info
{
    int pack_size;
	int uncompressed_size;
};
#pragma pack()

struct assetbundle_header
{
	char* signature;
	int format;
	char* versionPlayer;
	char* versionEngine;
	size_t minimumStreamedBytes;
	int headerSize;
	int numberOfLevelsToDownload;
	size_t levelByteEndCount;
	struct level_info* levelByteEnd;
	size_t completeFileSize;
	size_t dataHeaderSize;
	bool compressed; // form codingnow.com
};

size_t assetbundle_header_load(struct assetbundle_header* header, unsigned char* data, size_t offset)
{
	size_t start = offset;
	offset += read_string(data, offset, &header->signature);
	offset += read_int32(data, offset, &header->format, false);
	offset += read_string(data, offset, &header->versionPlayer);
	offset += read_string(data, offset, &header->versionEngine);
	offset += read_uint32(data, offset, &header->minimumStreamedBytes, false);
	offset += read_int32(data, offset, &header->headerSize, false);
	offset += read_int32(data, offset, &header->numberOfLevelsToDownload, false);
	offset += read_uint32(data, offset, &header->levelByteEndCount, false);
	offset += read_buffer(data, offset, (unsigned char**)&header->levelByteEnd, sizeof(header->levelByteEnd[0]) * header->levelByteEndCount);

    return offset - start;
}

struct assetbundle
{
	struct assetbundle_header header;
};

struct assetbundle* _assetbundle_load(unsigned char* data, size_t length)
{
    struct assetbundle* bundle = (struct assetbundle*)malloc(sizeof(struct assetbundle));
    size_t offset = 0;
    assetbundle_header_load(&bundle->header, data, offset);
    return NULL;
}

struct assetbundle* assetbundle_load(char* file)
{
    int fd = open(file, O_RDONLY);
    if (fd == -1)
		return NULL;
    
    struct stat stat;
    if (fstat(fd, &stat) == -1)
    {
        close(fd);
        return NULL;
    }

	struct assetbundle* asset = NULL;
	unsigned char* data = (unsigned char *)mmap(0, stat.st_size, PROT_READ, MAP_SHARED, fd, 0);
	if (data)
	{
		asset = _assetbundle_load(data, stat.st_size);
	}

	close(fd);
	return asset;
}

bool assetbundle_save(struct assetbundle* bundle, char* file)
{
    return  false;
}


