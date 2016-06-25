#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <fcntl.h>
#include <errno.h>
#include <assert.h>
#include <string.h>
#include "utils/platform.h"
#include "tools.h"
#include "utils/debug_tree.h"
#include "filemapping.h"
#include "assetfile.h"
#include "assetfile_imp.h"
#include "object_class.h"

size_t assetheader_load(struct assetheader* header, unsigned char* data, size_t offset)
{
	size_t start = offset;

	offset += read_uint32(data, offset, &header->metadata_size, false);
	offset += read_uint32(data, offset, &header->file_size, false);
	offset += read_int32(data, offset, &header->version, false);
	offset += read_uint32(data, offset, &header->data_offset, false);

	header->endianness = 0;
	if (header->version >= 9){
		offset += read_byte(data, offset, &header->endianness);
		offset += read_buffer(data, offset, &header->reserved, ASSETHEADER_RESERVED_SIZE);
	}

	return offset - start;
}

size_t assetheader_save(struct assetheader* header, unsigned char* data, size_t offset)
{
	size_t start = offset;

	offset += write_uint32(data, offset, header->metadata_size, false);
	offset += write_uint32(data, offset, header->file_size, false);
	offset += write_int32(data, offset, header->version, false);
	offset += write_uint32(data, offset, header->data_offset, false);

	if (header->version >= 9){
		offset += write_byte(data, offset, header->endianness);
		offset += write_buffer(data, offset, header->reserved, ASSETHEADER_RESERVED_SIZE);
	}

	return offset - start;
}

size_t field_type_load(unsigned char* data, size_t start, struct field_type* field_type)
{
    size_t offset = start;
    
    offset += read_int16(data, offset, &field_type->version, true);
    offset += read_byte(data, offset, &field_type->tree_level);
    offset += read_byte(data, offset, &field_type->is_array);
    offset += read_int32(data, offset, &field_type->type_offset, true);
    offset += read_int32(data, offset, &field_type->name_offset, true);
    offset += read_int32(data, offset, &field_type->size, true);
    offset += read_int32(data, offset, &field_type->index, true);
    offset += read_int32(data, offset, &field_type->meta_flag, true);
    
    return offset - start;
}

size_t field_type_save(unsigned char* data, size_t start, struct field_type* field_type)
{
    size_t offset = start;
    
    offset += write_int16(data, offset, field_type->version, true);
    offset += write_byte(data, offset, field_type->tree_level);
    offset += write_byte(data, offset, field_type->is_array);
    offset += write_int32(data, offset, field_type->type_offset, true);
    offset += write_int32(data, offset, field_type->name_offset, true);
    offset += write_int32(data, offset, field_type->size, true);
    offset += write_int32(data, offset, field_type->index, true);
    offset += write_int32(data, offset, field_type->meta_flag, true);
    
    return offset - start;
}


size_t field_type_list_load(unsigned char* data, size_t start, struct field_type_list* field_type_list)
{
    size_t offset = start;
    offset += read_int32(data, offset, &field_type_list->num_fields, true);
    offset += read_int32(data, offset, &field_type_list->string_table_len, true);
    
    if (field_type_list->num_fields > 0) {
        field_type_list->field_type_nodes = (struct field_type*)calloc(field_type_list->num_fields, sizeof(*field_type_list->field_type_nodes));
        for (size_t i = 0; i < field_type_list->num_fields; ++i) {
            offset += field_type_load(data, offset, field_type_list->field_type_nodes + i);
        }
    }
    
    offset += read_buffer(data, offset, &field_type_list->type_name_table, field_type_list->string_table_len);
    return offset - start;
}

size_t field_type_list_save(unsigned char* data, size_t start, struct field_type_list* field_type_list)
{
    size_t offset = start;
    
    offset += write_int32(data, offset, field_type_list->num_fields, true);
    offset += write_int32(data, offset, field_type_list->string_table_len, true);
    
    for (size_t i = 0; i < field_type_list->num_fields; ++i) {
        offset += field_type_save(data, offset, field_type_list->field_type_nodes + i);
    }
    
    offset += write_buffer(data, offset, field_type_list->type_name_table, field_type_list->string_table_len);

    return offset - start;
}


void field_type_list_destroy(struct field_type_list* field_type_list)
{
    if (field_type_list->field_type_nodes)
        free(field_type_list->field_type_nodes);
}

size_t typetree_struct_load(struct assetfile* file, unsigned char* data, size_t offset)
{
    size_t start = offset;
    offset += read_string(data, offset, &file->typetree_struct_revision);
    offset += read_int32(data, offset, &file->typetree_struct_attribute, true);
    offset += read_byte(data, offset, &file->typetree_struct_embedded);
    offset += read_uint32(data, offset, &file->typetree_struct_count, true);
    
    if (file->typetree_struct_count > 0) {
        file->typetree_struct = (struct typetree*)calloc(file->typetree_struct_count, sizeof(*file->typetree_struct));
        for (size_t i = 0; i < file->typetree_struct_count; ++i) {
            struct typetree* typetree = &file->typetree_struct[i];
            offset += read_int32(data, offset, &typetree->class_id, true);
            if (typetree->class_id < 0) {
                offset += read_buffer(data, offset, &typetree->script_id, UNITY_HASH_128);
            }
            offset += read_buffer(data, offset, &typetree->oldtype_hash, UNITY_HASH_128);
            
            if (file->typetree_struct_embedded)
                offset += field_type_list_load(data, offset, &typetree->field_type_list);
        }
    }
    
    if (file->header.version >= 7) {
        offset += read_int32(data, offset, &file->typetree_padding, true);
    }
    
    return offset - start;
}

size_t typetree_struct_save(struct assetfile* file, unsigned char* data, size_t offset)
{
    size_t start = offset;
    
    if (file->header.version >= 7) {
        offset += write_string(data, offset, file->typetree_struct_revision);
        offset += write_int32(data, offset, file->typetree_struct_attribute, true);
    }
    
    offset += write_uint32(data, offset, file->typetree_struct_count, true);
    for (size_t i = 0; i < file->typetree_struct_count; ++i) {
        struct typetree* typetree = &file->typetree_struct[i];
        offset += write_int32(data, offset, typetree->class_id, true);
        offset += field_type_list_save(data, offset, &typetree->field_type_list);
    }
    
    if (file->header.version >= 7) {
        offset += write_int32(data, offset, file->typetree_padding, true);
    }
    
    return offset - start;
}

void typetree_struct_destroy(struct assetfile* file)
{
    for (size_t i = 0; i < file->typetree_struct_count; ++i) {
        struct typetree* typetree = &file->typetree_struct[i];
        field_type_list_destroy(&typetree->field_type_list);
    }
    
    if (file->typetree_struct)
        free(file->typetree_struct);
}

size_t objectinfo_struct_load(struct assetfile* file, unsigned char* data, size_t offset)
{
	size_t start = offset;

	offset += read_uint32(data, offset, &file->objectinfo_struct_count, true);
    if (file->objectinfo_struct_count > 0) {
        file->objectinfo_struct = (struct objectinfo*)calloc(file->objectinfo_struct_count, sizeof(*file->objectinfo_struct));

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
		assert((buffer_offset - file_offset) % OBJECTINFO_BUFFER_ALIGN == 0);

		buffer_offset += read_buffer(data, buffer_offset, &objectinfo->buffer, objectinfo->length);
		objectinfo->align_data_length = (buffer_offset - file_offset) % OBJECTINFO_BUFFER_ALIGN;
		if (objectinfo->align_data_length != 0) {
			objectinfo->align_data_length = OBJECTINFO_BUFFER_ALIGN - objectinfo->align_data_length;
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
    if (file->objectinfo_struct_count > 0) {
        file->externals_struct = (struct fileidentifier*)calloc(file->externals_struct_count, sizeof(*file->externals_struct));
        
        for (size_t i = 0; i < file->externals_struct_count; ++i) {
            struct fileidentifier* fileidentifier = &file->externals_struct[i];
            
            if (file->header.version > 5) {
                offset += read_string(data, offset, &fileidentifier->asset_path);
            }
            offset += read_buffer(data, offset, &fileidentifier->guid, GUID_SIZE);
            offset += read_int32(data, offset, &fileidentifier->type, true);
            offset += read_string(data, offset, &fileidentifier->file_path);
        }
    }
	return offset - start;
}

size_t externals_struct_save(struct assetfile* file, unsigned char* data, size_t offset)
{
	size_t start = offset;

	offset += write_uint32(data, offset, file->externals_struct_count, true);

	for (size_t i = 0; i < file->externals_struct_count; ++i) {
		struct fileidentifier* fileidentifier = &file->externals_struct[i];

		if (file->header.version > 5) {
			offset += write_string(data, offset, fileidentifier->asset_path);
		}
		offset += write_buffer(data, offset, fileidentifier->guid, GUID_SIZE);
		offset += write_int32(data, offset, fileidentifier->type, true);
		offset += write_string(data, offset, fileidentifier->file_path);
	}
	return offset - start;
}

size_t assetmeta_load(struct assetfile* file, unsigned char* data, size_t offset, size_t file_offset)
{
	size_t start = offset;
    
    offset += typetree_struct_load(file, data, offset);
	offset += objectinfo_struct_load(file, data, offset);
	offset += externals_struct_load(file, data, offset);

	if (offset - file_offset < ASSETFILE_META_ALIGN) {
		file->align_data_length = ASSETFILE_META_ALIGN - (offset - file_offset);
	}
	else {
		file->align_data_length = (offset - file_offset) % ASSETFILE_ALIGN;
		if (file->align_data_length != 0) {
			file->align_data_length = ASSETFILE_ALIGN - file->align_data_length;
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

	offset += typetree_struct_save(file, data, offset);
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
	struct assetfile* file = (struct assetfile*)calloc(1, sizeof(*file));

	offset += assetheader_load(&file->header, data, offset);
	assert(offset - start <= size);

	offset += assetmeta_load(file, data, offset, start);
	assert(offset - start <= size);

	if (!assetfile_loadobjects(file, data, start, size)) {
		assetfile_destroy(file);
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

void assetfile_destroy(struct assetfile* file)
{
	if (file->externals_struct)
		free(file->externals_struct);

	if (file->objectinfo_struct)
		free(file->objectinfo_struct);
    
    typetree_struct_destroy(file);
    
	free(file);
}

void assetfile_print(struct assetfile* file, struct debug_tree* root)
{
	struct debug_tree* header = debug_tree_create(root, "header");

	debug_tree_create(header, "metadata_size:%u", file->header.metadata_size);
	debug_tree_create(header, "file_size:%u", file->header.file_size);
	debug_tree_create(header, "data_offset:%u", file->header.data_offset);

	struct debug_tree* objectinfo_struct = debug_tree_create(root, "objectinfo_struct:%u", file->objectinfo_struct_count);

	for (size_t i = 0; i < file->objectinfo_struct_count; ++i) {
		struct objectinfo* objectinfo = &file->objectinfo_struct[i];
		struct debug_tree* debug_tree = debug_tree_create(objectinfo_struct, "objectinfo path_id:%d, type_id:%d", objectinfo->path_id, objectinfo->type_id);

		char* objectinfo_name = objectinfo_getname(objectinfo->buffer, 0, objectinfo->length);
		if (objectinfo_name[0])
			debug_tree_create(debug_tree, "name:%s", objectinfo_name);
		free(objectinfo_name);

		debug_tree_create(debug_tree, "offset:%u", objectinfo->offset);
		debug_tree_create(debug_tree, "length:%u", objectinfo->length);
		debug_tree_create(debug_tree, "align_data_length:%u", objectinfo->align_data_length);
	}
}

size_t assetfile_objectinfo_count(struct assetfile* assetfile)
{
	return assetfile->objectinfo_struct_count;
}

struct objectinfo* assetfile_get_objectinfo(struct assetfile* assetfile, size_t index)
{
	return assetfile->objectinfo_struct + index;
}
