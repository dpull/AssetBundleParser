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
#include "assetfile.h"
#include "assetfile_imp.h"

struct objectinfo_modify
{
	int topath_id;
	unsigned char* buffer;
	size_t length;
};

struct objectinfo_same
{
	int topath_id;
	size_t align_data_length;
    unsigned char* align_data;
    
	size_t fromfiles_index;
	size_t fromobject_index;
};

struct assetfile_diff
{
	size_t objectinfo_modify_count;
	struct objectinfo_modify* objectinfo_modify;

	size_t objectinfo_same_count;
	struct objectinfo_same* objectinfo_same;
};

extern char* objectinfo_getname(struct objectinfo* objectinfo);

int objectinfo_findsame(struct assetfile* file, struct objectinfo* objectinfo) 
{
    char* search_name = objectinfo_getname(objectinfo);
	for (int i = 0; i < (int)file->objectinfo_struct_count; ++i) {
		struct objectinfo* cur_objectinfo = &file->objectinfo_struct[i];
		if (cur_objectinfo->type_id == objectinfo->type_id && cur_objectinfo->length == objectinfo->length && memcmp(cur_objectinfo->buffer, objectinfo->buffer, cur_objectinfo->length) == 0)
            return i;
        
        char* cur_name = objectinfo_getname(cur_objectinfo);
        if (cur_objectinfo->type_id == objectinfo->type_id && search_name[0] && cur_objectinfo->type_id == 83)
        {
            if (strstr(search_name, "name_xing") == NULL)
                continue;
            
            if (strcmp(search_name, cur_name) != 0)
                continue;
            
            
            if (cur_objectinfo->length == objectinfo->length)
            {
                size_t len = strlen(cur_name);
                len = (len + 3) / 4 * 4;
                
                printf("type_id \t %d \t %s \t %u\n", cur_objectinfo->type_id, cur_name, cur_objectinfo->length);
                size_t print_count = 0;
                for (int i = 0; i < cur_objectinfo->length; ++i) {
                    if (objectinfo->buffer[i] != cur_objectinfo->buffer[i]) {
                        print_count ++;
                        if (print_count > 20) {
                            printf("....\n");
                            break;
                        }
                        printf("%d \t %hhd \t %hhd\n", (int)i, objectinfo->buffer[i], cur_objectinfo->buffer[i]);
                    }
                }
            }
            else
            {
                printf("diff size type_id \t %d \t %s \t %u \t %u\n", cur_objectinfo->type_id, cur_name, cur_objectinfo->length, objectinfo->length);
            }
        }
        free(cur_name);
	}
    free(search_name);         
	return -1;
}

struct assetfile_diff* assetfile_diff(struct assetfile** fromfiles, size_t fromfiles_count, struct assetfile* tofile)
{
	struct assetfile_diff* diff = (struct assetfile_diff*)malloc(sizeof(*diff));

	diff->objectinfo_modify_count = 0;
	diff->objectinfo_modify = (struct objectinfo_modify*)malloc(sizeof(*diff->objectinfo_modify) * tofile->objectinfo_struct_count);
	diff->objectinfo_same_count = 0;
	diff->objectinfo_same = (struct objectinfo_same*)malloc(sizeof(*diff->objectinfo_same) * tofile->objectinfo_struct_count);

	for (size_t i = 0; i < tofile->objectinfo_struct_count; ++i) {
		struct objectinfo* to_objectinfo = &tofile->objectinfo_struct[i];
		size_t fromfiles_index = 0;
		int fromobject_index = -1;

		for (size_t j = 0; j < fromfiles_count; ++j) {
			struct assetfile* fromfile = fromfiles[j];
            
			fromobject_index = objectinfo_findsame(fromfile, to_objectinfo);
			if (fromobject_index >= 0) {                
				struct objectinfo* from_objectinfo = &fromfile->objectinfo_struct[fromobject_index];
                assert(memcmp(to_objectinfo->buffer, from_objectinfo->buffer, to_objectinfo->length + to_objectinfo->align_data_length) == 0);
                fromfiles_index = j;
				break;
			}
		}
		
		if (fromobject_index >= 0) {
			struct objectinfo_same* objectinfo_same = &diff->objectinfo_same[diff->objectinfo_same_count];
			diff->objectinfo_same_count++;

			objectinfo_same->topath_id = to_objectinfo->path_id;
			objectinfo_same->align_data_length = to_objectinfo->align_data_length;
            objectinfo_same->align_data = to_objectinfo->align_data;
			objectinfo_same->fromfiles_index = fromfiles_index;
			objectinfo_same->fromobject_index = fromobject_index;
            
            char* name = objectinfo_getname(to_objectinfo);
           // printf("same \t type_id \t %d \t %s \t %u\n", to_objectinfo->type_id, name, to_objectinfo->length);
            free(name);
            
		} else {
			struct objectinfo_modify* objectinfo_modify = &diff->objectinfo_modify[diff->objectinfo_modify_count];
			diff->objectinfo_modify_count++;

			objectinfo_modify->topath_id = to_objectinfo->path_id;
			objectinfo_modify->buffer = to_objectinfo->buffer;
			objectinfo_modify->length = to_objectinfo->length + to_objectinfo->align_data_length;
            
            char* name = objectinfo_getname(to_objectinfo);
           // printf("diff \t type_id \t %d \t %s \t %u\n", to_objectinfo->type_id, name, to_objectinfo->length);
            free(name);
		}
	}
    
    return diff;
}

bool assetfile_diff_merge(struct assetfile** fromfiles, size_t fromfiles_count, struct assetfile* tofile, struct assetfile_diff* diff, unsigned char* data, size_t start, size_t size)
{
	struct objectinfo_same* objectinfo_same = diff->objectinfo_same;
	struct objectinfo_modify* objectinfo_modify = diff->objectinfo_modify;

	for (size_t i = 0; i < tofile->objectinfo_struct_count; ++i) {
        struct objectinfo* objectinfo = &tofile->objectinfo_struct[i];
        size_t buffer_offset = start + tofile->header.data_offset + objectinfo->offset;
    	assert(buffer_offset - start <= size);

        if (objectinfo_same->topath_id == objectinfo->path_id) {
        	assert(objectinfo_same - diff->objectinfo_same < (int)diff->objectinfo_same_count);
        	assert(objectinfo_same->fromfiles_index < fromfiles_count);

			struct assetfile* fromfile = fromfiles[objectinfo_same->fromfiles_index];
			assert(objectinfo_same->fromobject_index < fromfile->objectinfo_struct_count);

			struct objectinfo* from_objectinfo = &fromfile->objectinfo_struct[objectinfo_same->fromobject_index];
            assert(from_objectinfo->length == objectinfo->length);
            
      		buffer_offset += write_buffer(data, buffer_offset, from_objectinfo->buffer, from_objectinfo->length);
    		assert(buffer_offset - start <= size);
            
            buffer_offset += write_buffer(data, buffer_offset, objectinfo_same->align_data, objectinfo_same->align_data_length);
            assert(buffer_offset - start <= size);

        	objectinfo_same++;
        } else if (objectinfo_modify->topath_id == objectinfo->path_id) {
        	assert(objectinfo_modify - diff->objectinfo_modify < (int)diff->objectinfo_modify_count);

        	buffer_offset += write_buffer(data, buffer_offset, objectinfo_modify->buffer, objectinfo_modify->length);
    		assert(buffer_offset - start <= size);

        	objectinfo_modify++;
        } else {
        	assert(false);
        	return false;
        }
    }
    
    return true;
}

void assetfile_diff_destory(struct assetfile_diff* diff)
{
    free(diff->objectinfo_modify);
    free(diff->objectinfo_same);
    free(diff);
}

size_t assetfile_diff_loadfile(struct assetfile** retfile, unsigned char* data, size_t offset)
{
    struct assetfile* file = (struct assetfile*)malloc(sizeof(*file));
    memset(file, 0, sizeof(*file));
    
	size_t start = offset;
	offset += assetheader_load(&file->header, data, offset);
    offset += assetmeta_load(file, data, offset, start);
    
    *retfile = file;
    return offset - start;
}

size_t assetfile_diff_savefile(struct assetfile* file, unsigned char* data, size_t offset)
{
	size_t start = offset;
	offset += assetheader_save(&file->header, data, offset);
    offset += assetmeta_save(file, data, offset);

    return offset - start;
}

size_t assetfile_diff_loaddiff(struct assetfile_diff** retdiff, unsigned char* data, size_t offset)
{
    size_t start = offset;
    struct assetfile_diff* diff = (struct assetfile_diff*)malloc(sizeof(*diff));
    
	offset += read_uint32(data, offset, &diff->objectinfo_modify_count, true);
	diff->objectinfo_modify = (struct objectinfo_modify*)malloc(sizeof(*diff->objectinfo_modify) * diff->objectinfo_modify_count);

	for (size_t i = 0; i < diff->objectinfo_modify_count; ++i) {
		struct objectinfo_modify* objectinfo_modify = &diff->objectinfo_modify[i];

        offset += read_int32(data, offset, &objectinfo_modify->topath_id, true);
		offset += read_uint32(data, offset, &objectinfo_modify->length, true);
		offset += read_buffer(data, offset, &objectinfo_modify->buffer, objectinfo_modify->length);
	}

	offset += read_uint32(data, offset, &diff->objectinfo_same_count, true);
	diff->objectinfo_same = (struct objectinfo_same*)malloc(sizeof(*diff->objectinfo_same) * diff->objectinfo_same_count);

	for (size_t i = 0; i < diff->objectinfo_same_count; ++i) {
		struct objectinfo_same* objectinfo_same = &diff->objectinfo_same[i];

        offset += read_int32(data, offset, &objectinfo_same->topath_id, true);
		offset += read_uint32(data, offset, &objectinfo_same->align_data_length, true);
        offset += read_buffer(data, offset, &objectinfo_same->align_data, objectinfo_same->align_data_length);
		offset += read_uint32(data, offset, &objectinfo_same->fromfiles_index, true);
		offset += read_uint32(data, offset, &objectinfo_same->fromobject_index, true);
	}
    
    *retdiff = diff;
    return offset - start;
}

size_t assetfile_diff_savediff(struct assetfile_diff* diff, unsigned char* data, size_t offset)
{
	size_t start = offset;

	offset += write_uint32(data, offset, diff->objectinfo_modify_count, true);
	for (size_t i = 0; i < diff->objectinfo_modify_count; ++i) {
		struct objectinfo_modify* objectinfo_modify = &diff->objectinfo_modify[i];

        offset += write_int32(data, offset, objectinfo_modify->topath_id, true);
		offset += write_uint32(data, offset, objectinfo_modify->length, true);
		offset += write_buffer(data, offset, objectinfo_modify->buffer, objectinfo_modify->length);
	}

	offset += write_uint32(data, offset, diff->objectinfo_same_count, true);
	for (size_t i = 0; i < diff->objectinfo_same_count; ++i) {
		struct objectinfo_same* objectinfo_same = &diff->objectinfo_same[i];
        
        offset += write_int32(data, offset, objectinfo_same->topath_id, true);
		offset += write_uint32(data, offset, objectinfo_same->align_data_length, true);
        offset += write_buffer(data, offset, objectinfo_same->align_data, objectinfo_same->align_data_length);
		offset += write_uint32(data, offset, objectinfo_same->fromfiles_index, true);
        offset += write_uint32(data, offset, objectinfo_same->fromobject_index, true);
	}

    return offset - start;	
}

void assetfile_diff_print(struct assetfile_diff* diff, struct debug_tree* root)
{
    struct debug_tree* assetfile_diff = debug_tree_create(root, "assetfile_diff");
	struct debug_tree* objectinfo_modify_count = debug_tree_create(assetfile_diff, "objectinfo_modify_count:%d", diff->objectinfo_modify_count);
	for (size_t i = 0; i < diff->objectinfo_modify_count; ++i) {
		struct objectinfo_modify* objectinfo_modify = &diff->objectinfo_modify[i];

		struct debug_tree* debug_tree = debug_tree_create(objectinfo_modify_count, "topath_id:%d", objectinfo_modify->topath_id);
		debug_tree_create(debug_tree, "length:%u", objectinfo_modify->length);
	}

	struct debug_tree* objectinfo_same_count = debug_tree_create(root, "objectinfo_same_count:%d", diff->objectinfo_same_count);	
	for (size_t i = 0; i < diff->objectinfo_same_count; ++i) {
		struct objectinfo_same* objectinfo_same = &diff->objectinfo_same[i];
        
       	struct debug_tree* debug_tree = debug_tree_create(objectinfo_same_count, "topath_id:%d", objectinfo_same->topath_id);
		debug_tree_create(debug_tree, "fromfiles_index:%u", objectinfo_same->fromfiles_index);
		debug_tree_create(debug_tree, "fromobject_index:%u", objectinfo_same->fromobject_index);
	}
}