#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <fcntl.h>
#include <errno.h>
#include <assert.h>
#include <string.h>
#include "tools.h"
#include "assetfile.h"
#include "assetfile_imp.h"

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

bool assetfile_loadobjects(struct assetfile* file, unsigned char* data, size_t file_offset, size_t file_size)
{
    size_t max_offset = file_offset + file_size;
    
    for (size_t i = 0; i < file->objectinfo_struct_count; ++i) {
        struct objectinfo* objectinfo = &file->objectinfo_struct[i];
        
        size_t buffer_offset = file_offset + file->header.data_offset + objectinfo->offset;
        assert((buffer_offset - file_offset) % objectinfo_buffer_align == 0);
        
        buffer_offset += read_buffer(data, buffer_offset, &objectinfo->buffer, objectinfo->length);
        objectinfo->align_data_length = (buffer_offset - file_offset) % objectinfo_buffer_align;
        if (objectinfo->align_data_length != 0) {
            objectinfo->align_data_length = objectinfo_buffer_align - objectinfo->align_data_length;
            if (objectinfo->align_data_length + buffer_offset > max_offset)
                objectinfo->align_data_length = max_offset - buffer_offset;
            
            buffer_offset += read_buffer(data, buffer_offset, &objectinfo->align_data, objectinfo->align_data_length);
        }
    }
    
    return true;
}

bool assetfile_saveobjects(struct assetfile* file, unsigned char* data, size_t file_offset)
{
    for (size_t i = 0; i < file->objectinfo_struct_count; ++i) {
        struct objectinfo* objectinfo = &file->objectinfo_struct[i];
        
        size_t buffer_offset = file_offset + file->header.data_offset + objectinfo->offset;
        buffer_offset += write_buffer(data, buffer_offset, objectinfo->buffer, objectinfo->length);
        
        if (objectinfo->align_data_length != 0) {
            buffer_offset += write_buffer(data, buffer_offset, objectinfo->align_data, objectinfo->align_data_length);
        }
    }
    
    return true;
}

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

size_t assetmeta_load(struct assetfile* file, unsigned char* data, size_t offset, size_t file_offset)
{
	size_t start = offset;

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
    offset += externals_struct_load(file, data, offset);
    
    if (offset - file_offset < assetfile_meta_align) {
        file->align_data_length = assetfile_meta_align - (offset - file_offset);
    } else {
        file->align_data_length = (offset - file_offset) % assetfile_align;
        if (file->align_data_length != 0) {
            file->align_data_length = assetfile_align - file->align_data_length;
        }
    }
    
    if (file->align_data_length != 0) {
        offset += read_buffer(data, offset, &file->align_data, file->align_data_length);
    }
	
    return offset - start;
}

size_t assetmeta_save(struct assetfile* file, unsigned char* data, size_t offset)
{
	size_t start = offset;

	if (file->header.version_info >= 7) {
		offset += write_string(data, offset, file->unity_revision);
		offset += write_int32(data, offset, file->typetree_struct_attribute, true);
	}

	offset += write_uint32(data, offset, file->typetree_struct_count, true);

	if (file->header.version_info >= 7) {
		offset += write_int32(data, offset, file->typetree_padding, true);
	}

	offset += objectinfo_struct_save(file, data, offset);
    offset += externals_struct_save(file, data, offset);
    
    if (file->align_data_length != 0) {
        offset += write_buffer(data, offset, file->align_data, file->align_data_length);
    }
	
    return offset - start;
}

struct assetfile* assetfile_load(unsigned char* data, size_t start, size_t size)
{	
	size_t offset = start;
	struct assetfile* file = (struct assetfile*)malloc(sizeof(*file));
	memset(file, 0, sizeof(*file));

	offset += assetheader_load(&file->header, data, offset);
    assert(offset - start <= size);

    offset += assetmeta_load(file, data, offset, start);
    assert(offset - start <= size);
    
    if (!assetfile_loadobjects(file, data, start, size)) {
    	assetfile_destory(file);
    	return NULL;
    }
    
    return file;
}

bool assetfile_save(struct assetfile* file, unsigned char* data, size_t start, size_t size)
{
	size_t offset = start;

	offset += assetheader_save(&file->header, data, offset);
    offset += assetmeta_save(file, data, offset);
    
	return assetfile_saveobjects(file, data, start);
}

void assetfile_destory(struct assetfile* file)
{
	if (file->externals_struct)
    	free(file->externals_struct);

	if (file->objectinfo_struct) 
    	free(file->objectinfo_struct);

	free(file);
}
