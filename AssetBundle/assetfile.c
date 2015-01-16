#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <errno.h>
#include <assert.h>
#include "assetfile.h"
#include "tools.h"

const size_t assetheader_reserved_size = 3;

struct assetheader
{
	size_t metadata_size;
	size_t file_size;
	int version_info;
	size_t data_offset;
	unsigned char endianness;
	unsigned char* reserved;
};

size_t assetheader_load(struct assetheader* header, unsigned char* data, size_t offset)
{
	size_t start = offset;

	offset += read_uint32(data, offset, &header->metadata_size, false);
	offset += read_uint32(data, offset, &header->file_size, false);
	offset += read_int32(data, offset, &header->version_info, false);
	offset += read_uint32(data, offset, &header->data_offset, false);

	header->endianness = 0;
	if (header->version_info >= 9){
		offset += read_byte(data, offset, &header->endianness);
		offset += read_buffer(data, offset, &header->reserved, assetheader_reserved_size);
	}

	return offset - start;
}

size_t assetheader_save(struct assetheader* header, unsigned char* data, size_t offset)
{
	size_t start = offset;

	offset += write_uint32(data, offset, header->metadata_size, false);
	offset += write_uint32(data, offset, header->file_size, false);
	offset += write_int32(data, offset, header->version_info, false);
	offset += write_uint32(data, offset, header->data_offset, false);

	if (header->version_info >= 9){
		offset += write_byte(data, offset, header->endianness);
		offset += write_buffer(data, offset, header->reserved, assetheader_reserved_size);
	}

	return offset - start;
}

struct assetfile
{
    struct assetheader header;
    char* unity_revision;
    int typetree_struct_attribute;
    size_t typetree_struct_count; // must == 0
    int typetree_padding;
    size_t objectinfo_struct_count;
    struct objectinfo* objectinfo_struct;
    size_t externals_struct_count;
    struct fileidentifier* externals_struct;
};

struct objectinfo
{
	int path_id;
	size_t offset;
	size_t length;
	int type_id;
	short class_id;
	short is_destroyed;
	unsigned char* buffer;
};

size_t objectinfo_struct_load(struct assetfile* file, unsigned char* data, size_t offset)
{
	size_t start = offset;

	offset += read_uint32(data, offset, &file->objectinfo_struct_count, true);
	file->objectinfo_struct = (struct objectinfo*)malloc(sizeof(*file->objectinfo_struct) * file->objectinfo_struct_count);

	for (size_t i = 0; i < file->objectinfo_struct_count; ++i) {
		struct objectinfo* objectinfo = &file->objectinfo_struct[i];

		offset += read_int32(data, offset, &objectinfo->path_id, true);
		offset += read_uint32(data, offset, &objectinfo->offset, true);
		offset += read_uint32(data, offset, &objectinfo->length, true);
		offset += read_int32(data, offset, &objectinfo->type_id, true);
		offset += read_int16(data, offset, &objectinfo->class_id, true);
		offset += read_int16(data, offset, &objectinfo->is_destroyed, true);
		objectinfo->buffer = NULL;

		assert(objectinfo->type_id == objectinfo->class_id || (objectinfo->class_id == 114 && objectinfo->type_id < 0));
	}
    
    return offset - start;
}

size_t objectinfo_struct_save(struct assetfile* file, unsigned char* data, size_t offset)
{
	size_t start = offset;

	offset += write_uint32(data, offset, file->objectinfo_struct_count, true);

	for (size_t i = 0; i < file->objectinfo_struct_count; ++i) {
		struct objectinfo* objectinfo = &file->objectinfo_struct[i];

		offset += write_int32(data, offset, objectinfo->path_id, true);
		offset += write_uint32(data, offset, objectinfo->offset, true);
		offset += write_uint32(data, offset, objectinfo->length, true);
		offset += write_int32(data, offset, objectinfo->type_id, true);
		offset += write_int16(data, offset, objectinfo->class_id, true);
		offset += write_int16(data, offset, objectinfo->is_destroyed, true);
	}
    
    return offset - start;
}

const size_t guid_size = 8;

struct fileidentifier
{		
	char* asset_path;
	unsigned char* guid;
	char* file_path;
	int type;
};

size_t externals_struct_load(struct assetfile* file, unsigned char* data, size_t offset)
{
	size_t start = offset;

	offset += read_uint32(data, offset, &file->externals_struct_count, true);
	file->externals_struct = (struct fileidentifier*)malloc(sizeof(*file->externals_struct) * file->externals_struct_count);

	for (size_t i = 0; i < file->externals_struct_count; ++i) {
		struct fileidentifier* fileidentifier = &file->externals_struct[i];

		offset += read_string(data, offset, &fileidentifier->asset_path);
		offset += read_buffer(data, offset, &fileidentifier->guid, guid_size);
		offset += read_string(data, offset, &fileidentifier->file_path);
		offset += read_int32(data, offset, &fileidentifier->type, true);
	}
    return offset - start;
}

size_t externals_struct_save(struct assetfile* file, unsigned char* data, size_t offset)
{
	size_t start = offset;

	offset += write_uint32(data, offset, file->externals_struct_count, true);

	for (size_t i = 0; i < file->externals_struct_count; ++i) {
		struct fileidentifier* fileidentifier = &file->externals_struct[i];

		offset += write_string(data, offset, fileidentifier->asset_path);
		offset += write_buffer(data, offset, fileidentifier->guid, guid_size);
		offset += write_string(data, offset, fileidentifier->file_path);
		offset += write_int32(data, offset, fileidentifier->type, true);
	}
    return offset - start;
}

struct assetfile* assetfile_load(unsigned char* data, size_t offset, size_t size)
{
	struct assetfile* file = (struct assetfile*)malloc(sizeof(struct assetfile));
	size_t start = offset;

	offset += assetheader_load(&file->header, data, offset);
    assert(offset - start <= size);

	if (file->header.version_info >= 7) {
		offset += read_string(data, offset, &file->unity_revision);
		offset += read_int32(data, offset, &file->typetree_struct_attribute, true);
	}

	offset += read_uint32(data, offset, &file->typetree_struct_count, true);
	assert(file->typetree_struct_count == 0);

	if (file->header.version_info >= 7) {
		offset += read_int32(data, offset, &file->typetree_padding, true);
	}

	offset += objectinfo_struct_load(file, data, offset);
    assert(offset - start <= size);

    offset += externals_struct_load(file, data, offset);
    assert(offset - start <= size);


	return file;
}

bool assetfile_save(struct assetfile* file, unsigned char* data, size_t offset, size_t size)
{
	//size_t start = offset;
	offset += assetheader_save(&file->header, data, offset);

	return true;
}

void assetfile_destory(struct assetfile* file)
{
    free(file->externals_struct);
    free(file->objectinfo_struct);
	free(file);
}
