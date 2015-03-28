#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <errno.h>
#include <assert.h>
#include <string.h>
#include "utils/platform.h"
#include "utils/debug_tree.h"
#include "tools.h"
#include "filemapping.h"
#include "field_type.h"

// http://docs.unity3d.com/Manual/ClassIDReference.html
// https://github.com/ata4/disunity/blob/master/disunity-core/src/main/resources/types.dat

struct field_type
{
	char* type;
	char* name;
	int size;
	int index;
	int is_array;
	int version;
	int meta_flag;
};

struct field_type_list
{
	struct field_type field_type;
	size_t children_count;
	struct field_type_list* children_field_type_list;
};

struct field_type_db
{
	int version;
	size_t count;
	struct field_type_list* field_type_list;
	struct filemapping* filemapping;
	size_t versions_count;
	char** versions;

};

size_t field_type_load(unsigned char* data, size_t start, size_t length, struct field_type* field_type)
{
	size_t offset = start;

	offset += read_string(data, offset, &field_type->type);
	offset += read_string(data, offset, &field_type->name);
	offset += read_int32(data, offset, &field_type->size, false);
	offset += read_int32(data, offset, &field_type->index, false);
	offset += read_int32(data, offset, &field_type->is_array, false);
	offset += read_int32(data, offset, &field_type->version, false);
	offset += read_int32(data, offset, &field_type->meta_flag, false);

	return offset - start;
}

size_t field_type_list_load(unsigned char* data, size_t start, size_t length, struct field_type_list* field_type_list)
{
	size_t offset = start;

	offset += field_type_load(data, offset, length, &field_type_list->field_type);
	offset += read_uint32(data, offset, &field_type_list->children_count, false);

	if (field_type_list->children_count > 0) {
		field_type_list->children_field_type_list = (struct field_type_list*)calloc(field_type_list->children_count, sizeof(*field_type_list->children_field_type_list));
		for (size_t i = 0; i < field_type_list->children_count; ++i) {
			offset += field_type_list_load(data, offset, length, field_type_list->children_field_type_list + i);
		}
	}

	return offset - start;
}

void field_type_list_destory(struct field_type_list* field_type_list)
{
	for (size_t i = 0; i < field_type_list->children_count; ++i) {
		field_type_list_destory(field_type_list->children_field_type_list + i);
	}

	if (field_type_list->children_field_type_list)
		free(field_type_list->children_field_type_list);
}

struct field_type_db* field_type_db_load(const char* file)
{
	struct filemapping* filemapping = filemapping_create_readonly(file);
	if (!filemapping)
		return NULL;

	struct field_type_db* field_type_db = (struct field_type_db*)calloc(1, sizeof(*field_type_db));
	field_type_db->filemapping = filemapping;

	unsigned char* data = filemapping_getdata(filemapping);
	size_t length = filemapping_getlength(filemapping);
	size_t offset = 0;

	offset += read_int32(data, offset, &field_type_db->version, false);
	assert(field_type_db->version == 1);

	offset += read_uint32(data, offset, &field_type_db->count, false);

	field_type_db->field_type_list = (struct field_type_list*)calloc(field_type_db->count, sizeof(*field_type_db->field_type_list));
	for (size_t i = 0; i < field_type_db->count; ++i) {
		offset += field_type_list_load(data, offset, length, field_type_db->field_type_list + i);
	}

	offset += read_uint32(data, offset, &field_type_db->versions_count, false);
	field_type_db->versions = (char**)calloc(field_type_db->versions_count, sizeof(*field_type_db->versions));
	for (size_t i = 0; i < field_type_db->count; ++i) {
		offset += read_string(data, offset, field_type_db->versions + i);
	}
	return field_type_db;
}

void field_type_db_destory(struct field_type_db* field_type_db)
{
	for (size_t i = 0; i < field_type_db->count; ++i) {
		field_type_list_destory(field_type_db->field_type_list + i);
	}

	if (field_type_db->field_type_list)
		free(field_type_db->field_type_list);

	if (field_type_db->filemapping)
		filemapping_destory(field_type_db->filemapping);

	free(field_type_db);
}

void field_type_list_print(struct field_type_list* field_type_list, struct debug_tree* root)
{
	struct debug_tree* type_list = debug_tree_create(
		root,
		"%s\t%s\t[%d]%s\t%s",
		field_type_list->field_type.type,
		field_type_list->field_type.name,
		field_type_list->field_type.version,
		field_type_list->field_type.is_array ? "[array]" : "",
		(field_type_list->field_type.meta_flag & 0x4000) ? "[align]" : ""
		);
	for (size_t i = 0; i < field_type_list->children_count; ++i) {
		field_type_list_print(field_type_list->children_field_type_list + i, type_list);
	}
}

void field_type_db_print(struct field_type_db* field_type_db, struct debug_tree* root)
{
	for (size_t i = 0; i < field_type_db->count; ++i) {
		field_type_list_print(field_type_db->field_type_list + i, root);
	}

	struct debug_tree* versions = debug_tree_create(root, "versions");
	for (size_t i = 0; i < field_type_db->versions_count; ++i) {
		debug_tree_create(versions, "[%d]\t%s", i, field_type_db->versions[i]);
	}
}