#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <errno.h>
#include <assert.h>
#include "assetbundle.h"
#include "assetfile.h"
#include "tools.h"

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
	char* version_player;
	char* version_engine;
	size_t minimum_streamed_bytes;
	int header_size;
	int number_of_levels_to_download;
	size_t level_byte_end_count;
	struct level_info* level_byte_end;
	size_t complete_file_size;
	size_t data_header_size;
	unsigned char compressed; // form codingnow.com
};

size_t assetbundle_header_load(struct assetbundle_header* header, unsigned char* data, size_t offset)
{
	size_t start = offset;

	offset += read_string(data, offset, &header->signature);
	offset += read_int32(data, offset, &header->format, false);
	offset += read_string(data, offset, &header->version_player);
	offset += read_string(data, offset, &header->version_engine);
	offset += read_uint32(data, offset, &header->minimum_streamed_bytes, false);
	offset += read_int32(data, offset, &header->header_size, false);
	offset += read_int32(data, offset, &header->number_of_levels_to_download, false);
	offset += read_uint32(data, offset, &header->level_byte_end_count, false);
	offset += read_buffer(data, offset, (unsigned char**)&header->level_byte_end, sizeof(*header->level_byte_end) * header->level_byte_end_count);

	header->complete_file_size = 0;
	if (header->format >= 2) {
		offset += read_uint32(data, offset, &header->complete_file_size, false);
	}

	header->data_header_size = 0;
	if (header->format >= 3) {
		offset += read_uint32(data, offset, &header->data_header_size, false);
	}
	
	offset += read_byte(data, offset, &header->compressed);

    return offset - start;
}

size_t assetbundle_header_save(struct assetbundle_header* header, unsigned char* data, size_t offset)
{
	size_t start = offset;

	offset += write_string(data, offset, header->signature);
	offset += write_int32(data, offset, header->format, false);
	offset += write_string(data, offset, header->version_player);
	offset += write_string(data, offset, header->version_engine);
	offset += write_uint32(data, offset, header->minimum_streamed_bytes, false);
	offset += write_int32(data, offset, header->header_size, false);
	offset += write_int32(data, offset, header->number_of_levels_to_download, false);
	offset += write_uint32(data, offset, header->level_byte_end_count, false);
	offset += write_buffer(data, offset, (unsigned char*)header->level_byte_end, sizeof(*header->level_byte_end) * header->level_byte_end_count);

	if (header->format >= 2) {
		offset += write_uint32(data, offset, header->complete_file_size, false);
	}

	if (header->format >= 3) {
		offset += write_uint32(data, offset, header->data_header_size, false);
	}

	offset += write_byte(data, offset, header->compressed);

    return offset - start;
}

struct assetbundle_entryinfo
{
	char* name;
	size_t offset;
	size_t size;
	struct assetfile* assetfile;
};

struct assetbundle
{
	struct assetbundle_header header;
	size_t entryinfo_count;
	struct assetbundle_entryinfo* entryinfo;

	unsigned char* data;
	size_t length;
};

size_t assetbundle_entryinfo_load(struct assetbundle* bundle, unsigned char* data, size_t offset)
{
	size_t start = offset;
	offset += read_uint32(data, offset, &bundle->entryinfo_count, false);

	bundle->entryinfo = (struct assetbundle_entryinfo*)malloc(sizeof(*bundle->entryinfo) * bundle->entryinfo_count);
	for (size_t i = 0; i < bundle->entryinfo_count; ++i) {
		struct assetbundle_entryinfo* entryinfo = &bundle->entryinfo[i];
		offset += read_string(data, offset, &entryinfo->name);
		offset += read_uint32(data, offset, &entryinfo->offset, false);
		offset += read_uint32(data, offset, &entryinfo->size, false);

		size_t file_offset = bundle->header.header_size + entryinfo->offset;
		assert(file_offset + entryinfo->size <= bundle->length);

		entryinfo->assetfile = assetfile_load(data, file_offset, entryinfo->size);
	}
    return offset - start;
}

size_t assetbundle_entryinfo_save(struct assetbundle* bundle, unsigned char* data, size_t offset)
{
	size_t start = offset;
	offset += write_uint32(data, offset, bundle->entryinfo_count, false);

	for (size_t i = 0; i < bundle->entryinfo_count; ++i) {
		struct assetbundle_entryinfo* entryinfo = &bundle->entryinfo[i];
		offset += write_string(data, offset, entryinfo->name);
		offset += write_uint32(data, offset, entryinfo->offset, false);
		offset += write_uint32(data, offset, entryinfo->size, false);

		size_t file_offset = bundle->header.header_size + entryinfo->offset;
        bool ret = assetfile_save(entryinfo->assetfile, data, file_offset, entryinfo->size);
        assert(ret);
	}

    return offset - start;
}

void assetbundle_entryinfo_destory(struct assetbundle* bundle)
{
	for (size_t i = 0; i < bundle->entryinfo_count; ++i) {
		struct assetbundle_entryinfo* entryinfo = &bundle->entryinfo[i];
		assetfile_destory(entryinfo->assetfile);
	}
	free(bundle->entryinfo);	
}

struct assetbundle* assetbundle_load(char* file)
{
    int fd = open(file, O_RDONLY);
    if (fd == -1)
		return NULL;
    
    struct stat stat;
    if (fstat(fd, &stat) == -1) {
        close(fd);
        return NULL;
    }

	unsigned char* data = (unsigned char *)mmap(0, stat.st_size, PROT_READ, MAP_SHARED, fd, 0);    
	if (data == MAP_FAILED) {
        close(fd);
        return NULL;
	}

    close(fd);
    
    struct assetbundle* bundle = (struct assetbundle*)malloc(sizeof(struct assetbundle));
    bundle->data = data;
    bundle->length = stat.st_size;

    size_t offset = 0;
   	offset += assetbundle_header_load(&bundle->header, data, offset);
   	offset += assetbundle_entryinfo_load(bundle, data, offset);

	return bundle;
}

bool assetbundle_save(struct assetbundle* bundle, char* file)
{
	int fd = open(file, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    if (fd == -1)
        return false;
	
    if (ftruncate(fd, bundle->length) != 0){
        close(fd);
        return false;
    }
    
	unsigned char* data = (unsigned char *)mmap(0, bundle->length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0); 
	if (data == MAP_FAILED) {
		close(fd);
        return false;
	}
    
    close(fd);

    size_t offset = 0;
   	offset += assetbundle_header_save(&bundle->header, data, offset);
   	offset += assetbundle_entryinfo_save(bundle, data, offset);
    
    int j = 0;
    printf("begin\n");
   	for (size_t i = 0; i < bundle->length; ++i) {
		if (data[i] != bundle->data[i]) {
			printf("Error %d:[%hho]\t[%hho]\n", i, data[i], bundle->data[i]);
            j++;
		}
   	}
    
    printf("end:%d\n", j);

    munmap(data, bundle->length);
    return true;
}

void assetbundle_destory(struct assetbundle* bundle)
{
	assetbundle_entryinfo_destory(bundle);
	munmap(bundle->data, bundle->length);
	free(bundle);
}

