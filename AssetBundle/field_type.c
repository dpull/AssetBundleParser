#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <errno.h>
#include <assert.h>
#include <string.h>
#include "utils/platform.h"
#include "utils/debug_tree.h"
#include "tools.h"
#include "filemaping.h"
#include "field_type.h"


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
 	struct filemaping* filemaping;   
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
    struct filemaping* filemaping = filemaping_create_readonly(file);
    if (!filemaping)
        return NULL;

    struct field_type_db* field_type_db = (struct field_type_db*)calloc(1, sizeof(*field_type_db)); 
    field_type_db->filemaping = filemaping;

    unsigned char* data = filemaping_getdata(filemaping);
    size_t length = filemaping_getlength(filemaping);
    size_t offset = 0;

    offset += read_int32(data, offset, &field_type_db->version, false);
    assert(field_type_db->version == 1);

    offset += read_uint32(data, offset, &field_type_db->count, false);

    field_type_db->field_type_list = (struct field_type_list*)calloc(field_type_db->count, sizeof(*field_type_db->field_type_list));
    for (size_t i = 0; i < field_type_db->count; ++i) {
        offset += field_type_list_load(data, offset, length, field_type_db->field_type_list + i);
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

    if (field_type_db->filemaping)
        filemaping_destory(field_type_db->filemaping);

    free(field_type_db);
}

void field_type_list_print(struct field_type_list* field_type_list, struct debug_tree* root)
{
    struct debug_tree* debug_tree = debug_tree_create(root, "%s\t%s", field_type_list->field_type.type, field_type_list->field_type.name);
    for (size_t i = 0; i < field_type_list->children_count; ++i) {
        field_type_list_print(field_type_list->children_field_type_list + i, debug_tree);
    }
}

void field_type_db_print(struct field_type_db* field_type_db, struct debug_tree* root)
{
    for (size_t i = 0; i < field_type_db->count; ++i) {
        field_type_list_print(field_type_db->field_type_list + i, root);
    }
}