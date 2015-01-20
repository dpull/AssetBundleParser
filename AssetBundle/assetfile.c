#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <fcntl.h>
#include <errno.h>
#include <assert.h>
#include <string.h>
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

const int assetfile_meta_align = 4096;
const int assetfile_align = 16;

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
    size_t align_data_length;
    unsigned char* align_data;
};

const int objectinfo_buffer_align = 8;
struct objectinfo
{
	int path_id;
	size_t offset;
	size_t length;
	int type_id;
	short class_id;
	short is_destroyed;
    
	unsigned char* buffer;
    size_t align_data_length;
    unsigned char* align_data;
};

size_t objectinfo_struct_load(struct assetfile* file, unsigned char* data, size_t offset)
{
	size_t start = offset;

	offset += read_uint32(data, offset, &file->objectinfo_struct_count, true);
	file->objectinfo_struct = (struct objectinfo*)malloc(sizeof(*file->objectinfo_struct) * file->objectinfo_struct_count);
	memset(file->objectinfo_struct, 0, sizeof(*file->objectinfo_struct) * file->objectinfo_struct_count);

	for (size_t i = 0; i < file->objectinfo_struct_count; ++i) {
		struct objectinfo* objectinfo = &file->objectinfo_struct[i];

		offset += read_int32(data, offset, &objectinfo->path_id, true);
		offset += read_uint32(data, offset, &objectinfo->offset, true);
		offset += read_uint32(data, offset, &objectinfo->length, true);
		offset += read_int32(data, offset, &objectinfo->type_id, true);
		offset += read_int16(data, offset, &objectinfo->class_id, true);
		offset += read_int16(data, offset, &objectinfo->is_destroyed, true);

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

bool objectinfos_load(struct assetfile* file, unsigned char* data, size_t buffer_base_offset)
{
	for (size_t i = 0; i < file->objectinfo_struct_count; ++i) {
		struct objectinfo* objectinfo = &file->objectinfo_struct[i];

		size_t buffer_offset = buffer_base_offset + file->header.data_offset + objectinfo->offset;
        assert((buffer_offset - buffer_base_offset) % objectinfo_buffer_align == 0);

		buffer_offset += read_buffer(data, buffer_offset, &objectinfo->buffer, objectinfo->length);        
        objectinfo->align_data_length = buffer_offset % objectinfo_buffer_align;
        if (objectinfo->align_data_length != 0) {
            objectinfo->align_data_length = objectinfo_buffer_align - objectinfo->align_data_length;
            buffer_offset += read_buffer(data, buffer_offset, &objectinfo->align_data, objectinfo->align_data_length);
        }        
	}
    
    return true;
}

size_t objectinfos_save(struct assetfile* file, unsigned char* data, size_t buffer_base_offset)
{
	for (size_t i = 0; i < file->objectinfo_struct_count; ++i) {
		struct objectinfo* objectinfo = &file->objectinfo_struct[i];

		size_t buffer_offset = buffer_base_offset + file->header.data_offset + objectinfo->offset;
		buffer_offset += write_buffer(data, buffer_offset, objectinfo->buffer, objectinfo->length);
        
        if (objectinfo->align_data_length != 0) {
            buffer_offset += write_buffer(data, buffer_offset, objectinfo->align_data, objectinfo->align_data_length);
        }
	}
    
    return true;
}

const size_t guid_size = 16;

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
	memset(file->externals_struct, 0, sizeof(*file->externals_struct) * file->externals_struct_count);

	for (size_t i = 0; i < file->externals_struct_count; ++i) {
		struct fileidentifier* fileidentifier = &file->externals_struct[i];
        
        if (file->header.version_info > 5) {
            offset += read_string(data, offset, &fileidentifier->asset_path);
        }
		offset += read_buffer(data, offset, &fileidentifier->guid, guid_size);
		offset += read_int32(data, offset, &fileidentifier->type, true);
		offset += read_string(data, offset, &fileidentifier->file_path);
	}
    return offset - start;
}

size_t externals_struct_save(struct assetfile* file, unsigned char* data, size_t offset)
{
	size_t start = offset;

	offset += write_uint32(data, offset, file->externals_struct_count, true);

	for (size_t i = 0; i < file->externals_struct_count; ++i) {
		struct fileidentifier* fileidentifier = &file->externals_struct[i];
        
        if (file->header.version_info > 5) {
            offset += write_string(data, offset, fileidentifier->asset_path);
        } 
		offset += write_buffer(data, offset, fileidentifier->guid, guid_size);
		offset += write_int32(data, offset, fileidentifier->type, true);
		offset += write_string(data, offset, fileidentifier->file_path);
	}
    return offset - start;
}

struct assetfile* assetfile_load(unsigned char* data, size_t offset, size_t size)
{	
	size_t start = offset;
	struct assetfile* file = (struct assetfile*)malloc(sizeof(*file));
	memset(file, 0, sizeof(*file));

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
    
    if (offset < assetfile_meta_align) {
        file->align_data_length = assetfile_meta_align - offset;
    } else {
        file->align_data_length = offset % assetfile_align;
        if (file->align_data_length != 0) {
            file->align_data_length = assetfile_align - file->align_data_length;
        }
    }
    
    if (file->align_data_length != 0) {
        offset += read_buffer(data, offset, &file->align_data, file->align_data_length);
    }

    bool ret = objectinfos_load(file, data, start);
    assert(ret);

	return file;
}

bool assetfile_save(struct assetfile* file, unsigned char* data, size_t offset, size_t size)
{
	size_t start = offset;
	offset += assetheader_save(&file->header, data, offset);

	if (file->header.version_info >= 7) {
		offset += write_string(data, offset, file->unity_revision);
		offset += write_int32(data, offset, file->typetree_struct_attribute, true);
	}

	offset += write_uint32(data, offset, file->typetree_struct_count, true);

	if (file->header.version_info >= 7) {
		offset += write_int32(data, offset, file->typetree_padding, true);
	}

	offset += objectinfo_struct_save(file, data, offset);
    assert(offset - start <= size);

    offset += externals_struct_save(file, data, offset);
    assert(offset - start <= size);
    
    if (file->align_data_length != 0) {
        offset += write_buffer(data, offset, file->align_data, file->align_data_length);
    }

    bool ret = objectinfos_save(file, data, start);
    assert(ret);

	return true;
}

void assetfile_destory(struct assetfile* file)
{
	if (file->externals_struct)
    	free(file->externals_struct);

	if (file->objectinfo_struct) 
    	free(file->objectinfo_struct);

	free(file);
}

struct objectinfo* objectinfo_findsame(struct assetfile* file, struct objectinfo* objectinfo) 
{
	for (size_t i = 0; i < file->objectinfo_struct_count; ++i) {
		struct objectinfo* cur_objectinfo = &file->objectinfo_struct[i];
		if (cur_objectinfo->length == objectinfo->length && memcmp(cur_objectinfo->buffer, objectinfo->buffer, cur_objectinfo->length) == 0)
			return cur_objectinfo;
	}
	return NULL;
}

char* objectinfo_getname(struct objectinfo* objectinfo)
{
	int name_len;
    size_t name_offset = 0;

    name_offset += read_int32(objectinfo->buffer, name_offset, &name_len, true);
    assert(name_len >= 0);
    assert(name_len <= objectinfo->length);
    
    char* name = NULL;
    if (name_len > 0) {
        name = malloc(name_len + 1);
        memcpy(name, objectinfo->buffer + name_offset, name_len);
        name[name_len] = '\0';
    }

    return name;
}

struct objectinfo_modify
{
	int dst_path_id;
	unsigned char* buffer;
	size_t length;
};

struct objectinfo_same
{
	int dst_path_id;
	size_t src_files_index;
	int src_path_id;
};

struct assetfile_diff
{
	size_t objectinfo_modify_count;
	struct objectinfo_modify* objectinfo_modify;

	size_t objectinfo_same_count;
	struct objectinfo_same* objectinfo_same;
};

struct assetfile_diff* assetfile_diff(struct assetfile** src_files, size_t src_files_count, struct assetfile* dst_file)
{
	struct assetfile_diff* diff = (struct assetfile_diff*)malloc(sizeof(*diff));

	diff->objectinfo_modify_count = 0;
	diff->objectinfo_modify = (struct objectinfo_modify*)malloc(sizeof(*diff->objectinfo_modify) * dst_file->objectinfo_struct_count);
	diff->objectinfo_same_count = 0;
	diff->objectinfo_same = (struct objectinfo_same*)malloc(sizeof(*diff->objectinfo_same) * dst_file->objectinfo_struct_count);

	for (size_t i = 0; i < dst_file->objectinfo_struct_count; ++i) {
		struct objectinfo* dst_objectinfo = &dst_file->objectinfo_struct[i];
		size_t src_files_index = 0;
		struct objectinfo* src_objectinfo = NULL;

		for (size_t j = 0; j < src_files_count; ++j) {
			struct assetfile* src_file = src_files[j];
            
			src_objectinfo = objectinfo_findsame(src_file, dst_objectinfo);
			if (src_objectinfo) {
                assert(memcmp(dst_objectinfo->buffer, src_objectinfo->buffer, dst_objectinfo->length + dst_objectinfo->align_data_length) == 0);

                src_files_index = j;
				break;
			}
		}

		
		if (src_objectinfo) {
			struct objectinfo_same* objectinfo_same = &diff->objectinfo_same[diff->objectinfo_same_count];
			diff->objectinfo_same_count++;

			objectinfo_same->dst_path_id = dst_objectinfo->path_id;
			objectinfo_same->src_files_index = src_files_index;
			objectinfo_same->dst_path_id = src_objectinfo->path_id;
		} else {
			struct objectinfo_modify* objectinfo_modify = &diff->objectinfo_modify[diff->objectinfo_modify_count];
			diff->objectinfo_modify_count++;

			objectinfo_modify->dst_path_id = dst_objectinfo->path_id;
			objectinfo_modify->buffer = dst_objectinfo->buffer;
			objectinfo_modify->length = dst_objectinfo->length + dst_objectinfo->align_data_length;
		}
	}
    
    return diff;
}

void assetfile_diff_destory(struct assetfile_diff* diff)
{
    free(diff->objectinfo_modify);
    free(diff->objectinfo_same);
    free(diff);
}
