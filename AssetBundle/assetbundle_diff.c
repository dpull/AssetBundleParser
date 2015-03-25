#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <fcntl.h>
#include <errno.h>
#include <assert.h>
#include <string.h>
#include "utils/platform.h"
#include "utils/debug_tree.h"
#include "utils/traversedir.h"
#include "tools.h"
#include "filemaping.h"
#include "object_class.h"
#include "assetfile_imp.h"
#include "assetfile.h"
#include "assetbundle.h"
#include "assetbundle_imp.h"
#include "assetbundle_diff.h"

#define MAX_PATH 				(512)
#define MAX_COMBINE_FILE_SIZE	(1024 * 1024 * 100)
#define MAX_SPLIT_FILE_COUNT	(100)
#define MAX_DIFF_RESERVED_SIZE  (1024 * 1024)
#define MAX_DIFF_INFO_SIZE      (1024)
#define DIFF_INFO_VERIFY        ("dpull@v1")

struct split_filemaping
{
    struct filemaping* filemaping;
    char* name;
    bool format;
    size_t offsets_count;
    size_t* offsets;
};

void split_filemaping_destory(struct split_filemaping* split_filemaping)
{
	if (split_filemaping->filemaping)
		filemaping_destory(split_filemaping->filemaping);

	if (split_filemaping->name)
		free(split_filemaping->name);

	if (split_filemaping->offsets)
		free(split_filemaping->offsets);

	free(split_filemaping);
}

struct split_filemaping* split_filemaping_create(const char* fullpath, const char* filename)
{
	struct split_filemaping* split_filemaping = (struct split_filemaping*)calloc(1, sizeof(*split_filemaping));
    const char* split0 = strstr(filename, ".split0");

    if (split0 == NULL) {
    	split_filemaping->filemaping = filemaping_create_readonly(fullpath);
    	if (!split_filemaping->filemaping) {
    		split_filemaping_destory(split_filemaping);
    		return NULL;
    	}
    	split_filemaping->name = strdup(filename);
    	split_filemaping->format = false;
    	split_filemaping->offsets_count = 0;

    	return split_filemaping;
    } else {
		char temp_file[MAX_PATH];
        strcpy(temp_file, "/tmp/split_filemaping.XXXXXX");
        mktemp(temp_file);

        split_filemaping->filemaping = filemaping_create_readwrite(temp_file, MAX_COMBINE_FILE_SIZE);
		if (!split_filemaping->filemaping) {
    		split_filemaping_destory(split_filemaping);
    		return NULL;
    	}

    	split_filemaping->name = strndup(filename, split0 - filename);
    	split_filemaping->format = true;
    	split_filemaping->offsets = (size_t*)calloc(MAX_SPLIT_FILE_COUNT, sizeof(*split_filemaping->offsets));

        unsigned char* data = filemaping_getdata(split_filemaping->filemaping);
        size_t offset = 0;

        for (int i = 0; i < MAX_SPLIT_FILE_COUNT; ++i) {
            char split_asset_path[MAX_PATH];
            size_t dir_length = strlen(fullpath) - strlen(filename);
            strncpy(split_asset_path, fullpath, dir_length);
            sprintf(split_asset_path + dir_length, "%s.split%d", split_filemaping->name, i);

            struct filemaping* filemaping = filemaping_create_readonly(split_asset_path);
            if (!filemaping)
                break;
            
            split_filemaping->offsets[i] = offset;
            split_filemaping->offsets_count++;

            unsigned char* split_data = filemaping_getdata(filemaping);
            size_t split_length = filemaping_getlength(filemaping);
            memcpy(data + offset, split_data, split_length);
            
            offset += split_length;
            filemaping_destory(filemaping);
        }

        return split_filemaping;
    }
}

struct assetfiles
{
    size_t max_count;
    size_t count;

    struct assetfile** assetfiles;

    struct assetbundle* assetbundle;
    size_t assetbundle_assetfile_count;
  
    struct split_filemaping** split_filemapings;
    unsigned char** filemaping_base;
};

struct assetfiles* assetfiles_create()
{
	struct assetfiles* assetfiles = (struct assetfiles*)calloc(1, sizeof(*assetfiles));
	return assetfiles;
}

void assetfiles_destory(struct assetfiles* assetfiles)
{
	if (assetfiles->assetfiles) {
		for (size_t i = 0; i < assetfiles->count; ++i) {
			struct assetfile* assetfile = assetfiles->assetfiles[i];
			if (assetfile)
				assetfile_destory(assetfile);
		}
		free(assetfiles->assetfiles);
	}

    if (assetfiles->assetbundle) {
        assetbundle_destory(assetfiles->assetbundle);
    }

	if (assetfiles->split_filemapings) {
		for (size_t i = 0; i < assetfiles->count; ++i) {
			struct split_filemaping* split_filemaping = assetfiles->split_filemapings[i];
			if (split_filemaping)
				split_filemaping_destory(split_filemaping);
		}
		free(assetfiles->split_filemapings);
	}

    if (assetfiles->filemaping_base) {
        free(assetfiles->filemaping_base);
    }
	free(assetfiles);
}

void assetfiles_insert(struct assetfiles* assetfiles, struct assetfile* assetfile, struct split_filemaping* split_filemaping, unsigned char* filemaping_base)
{
	if (assetfiles->count == assetfiles->max_count) {
        assetfiles->max_count = 2 * ((assetfiles->max_count == 0) ? 100 : assetfiles->max_count);
        assetfiles->assetfiles = (struct assetfile**)realloc(assetfiles->assetfiles, assetfiles->max_count * sizeof(*assetfiles->assetfiles));
        assetfiles->split_filemapings = (struct split_filemaping**)realloc(assetfiles->split_filemapings, assetfiles->max_count * sizeof(*assetfiles->split_filemapings));
        assetfiles->filemaping_base = (unsigned char**)realloc(assetfiles->filemaping_base, assetfiles->max_count * sizeof(*assetfiles->filemaping_base));
    }

    assetfiles->assetfiles[assetfiles->count] = assetfile;
    assetfiles->split_filemapings[assetfiles->count] = split_filemaping;
    assetfiles->filemaping_base[assetfiles->count] = filemaping_base;
    assetfiles->count++;
}

bool assetfiles_loadfrom_assetbundle(struct assetfiles* assetfiles, const char* filename)
{
	struct assetbundle* assetbundle = assetbundle_load(filename);
	if (!assetbundle)
		return false;

	if (!assetbundle_check(assetbundle)) {
		assetbundle_destory(assetbundle);
		return false;
	}

	size_t count = assetbundle_assetfile_count(assetbundle);
    unsigned char* data = filemaping_getdata(assetbundle->filemaping);

	for (size_t i = 0; i < count; ++i)	{
		struct assetfile* assetfile = assetbundle_get_assetfile(assetbundle, i);
        assetfiles_insert(assetfiles, assetfile, NULL, data);
	}

    assetfiles->assetbundle = assetbundle;
	assetfiles->assetbundle_assetfile_count = count;
    return true;
}

bool load_assetfile_dir(const char* fullpath, const char* filename, void* userdata)
{
    struct split_filemaping* split_filemaping = split_filemaping_create(fullpath, filename);
    if (!split_filemaping)
        return true;

    unsigned char* data = filemaping_getdata(split_filemaping->filemaping);
    size_t length = filemaping_getlength(split_filemaping->filemaping);

    if (!is_assetfile(data, 0, length)) {
        split_filemaping_destory(split_filemaping);
        return true;
    }
    
    struct assetfile* assetfile = assetfile_load(data, 0, length);
    if (!assetfile) {
        split_filemaping_destory(split_filemaping);
        return true;
    }

    assetfiles_insert((struct assetfiles*)userdata, assetfile, split_filemaping, data);
    return true;
}

struct assetbundle_diff
{
    struct filemaping* filemaping;
    size_t assetbundle_size;
    struct assetbundle* assetbundle;
    size_t assetfile_index_count; 
    char** assetfile_index; 

    unsigned char* buffer_begin; 
    unsigned char* buffer_end; 
    unsigned char* buffer_pos; 
};

void assetbundle_diff_destory(struct assetbundle_diff* assetbundle_diff)
{
    if (assetbundle_diff->assetfile_index) 
        free(assetbundle_diff->assetfile_index);

    if (assetbundle_diff->assetbundle) 
        assetbundle_destory(assetbundle_diff->assetbundle);

    if (assetbundle_diff->filemaping)
        filemaping_destory(assetbundle_diff->filemaping);

    free(assetbundle_diff);
}

struct assetbundle_diff* assetbundle_diff_loaddata(unsigned char* data, size_t length)
{
    struct assetbundle_diff* assetbundle_diff = (struct assetbundle_diff*)calloc(1, sizeof(*assetbundle_diff));
    size_t offset = 0;
    offset += read_uint32(data, offset, &assetbundle_diff->assetbundle_size, false);
    
    assetbundle_diff->assetbundle = assetbundle_load_data(data + offset, length - offset);    
    if (!assetbundle_diff->assetbundle || !assetbundle_check(assetbundle_diff->assetbundle)) {
        assetbundle_diff_destory(assetbundle_diff);
        return NULL;
    }
    offset += assetbundle_diff->assetbundle_size;

    offset += read_uint32(data, offset, &assetbundle_diff->assetfile_index_count, false);

    assetbundle_diff->assetfile_index = (char**)malloc(assetbundle_diff->assetfile_index_count * sizeof(*assetbundle_diff->assetfile_index));
    for (int i = 0; i < assetbundle_diff->assetfile_index_count; ++i) {
        offset += read_string(data, offset, assetbundle_diff->assetfile_index + i);
    }

    assetbundle_diff->buffer_begin = data;
    assetbundle_diff->buffer_end = data + length;
    assetbundle_diff->buffer_pos = data + offset;
    return assetbundle_diff;
}

struct assetbundle_diff* assetbundle_diff_create(const char* src, const char* dst)
{
    struct filemaping* src_filemaping = filemaping_create_readonly(src);
    if (!src_filemaping)
        return NULL;

    unsigned char* src_data = filemaping_getdata(src_filemaping);
    size_t src_length = filemaping_getlength(src_filemaping);

    struct filemaping* dst_filemaping = filemaping_create_readwrite(dst, src_length + MAX_DIFF_RESERVED_SIZE);
    if (!dst_filemaping) {
        filemaping_destory(src_filemaping);
        return NULL;    
    }

    unsigned char* dst_data = filemaping_getdata(dst_filemaping);
    size_t dst_length = filemaping_getlength(dst_filemaping);

    size_t offset = 0;
    offset += write_uint32(dst_data, offset, src_length, false);
    offset += write_buffer(dst_data, offset, src_data, src_length);
    offset += write_uint32(dst_data, offset, 0, false);

    struct assetbundle_diff* assetbundle_diff = assetbundle_diff_loaddata(dst_data, dst_length);
    if (!assetbundle_diff) {
        filemaping_destory(src_filemaping);
        filemaping_destory(dst_filemaping);
        return NULL;
    }
    assetbundle_diff->filemaping = dst_filemaping;
    filemaping_destory(src_filemaping);
    return assetbundle_diff;
} 

struct objectinfo* objectinfo_find(struct objectinfo* objectinfo, struct assetfile* assetfile) 
{
    size_t obj_count = assetfile_objectinfo_count(assetfile);
    for (size_t i = 0; i < obj_count; ++i) {
        struct objectinfo* objectinfo_check = assetfile_get_objectinfo(assetfile, i);
        if (objectinfo_check->type_id != objectinfo->type_id || objectinfo_check->length != objectinfo->length)
            continue;
        
        if (memcmp(objectinfo_check->buffer, objectinfo->buffer, objectinfo_check->length) == 0) {
            return objectinfo_check;
        }
    }
    return NULL;
}

size_t assetbundle_diff_insert(struct assetbundle_diff* assetbundle_diff, char* filename)
{
    for (size_t i = 0; i < assetbundle_diff->assetfile_index_count; ++i) {
        char* assetfile_index = assetbundle_diff->assetfile_index[i]; 
        if (strcmp(assetfile_index, filename) == 0)
            return i + 1;
    }

    unsigned char* buffer_pos = assetbundle_diff->buffer_pos;
    assetbundle_diff->buffer_pos += write_string(buffer_pos, 0, filename);
    assert(assetbundle_diff->buffer_pos < assetbundle_diff->buffer_end);

    assetbundle_diff->assetfile_index_count++;
    assetbundle_diff->assetfile_index = realloc(assetbundle_diff->assetfile_index, assetbundle_diff->assetfile_index_count * sizeof(*assetbundle_diff->assetfile_index));
    
    read_string(buffer_pos, 0, assetbundle_diff->assetfile_index + assetbundle_diff->assetfile_index_count - 1);
    return assetbundle_diff->assetfile_index_count;
}

struct objectinfo_diff_info
{
    char* verify;
    size_t index;
    size_t offset;
};

void objectinfo_diff(struct objectinfo* objectinfo, struct assetbundle_diff* assetbundle_diff, struct assetfiles* assetfiles) 
{
    struct objectinfo* objectinfo_same = NULL;
    size_t assetfile_index = 0;

    for (size_t i = 0; i < assetfiles->count; ++i) {
        struct objectinfo* objectinfo_same = objectinfo_find(objectinfo, assetfiles->assetfiles[i]);
        if (objectinfo_same) {
            assetfile_index = i;
            break;
        }
    }

    if (!objectinfo_same)
        return;

    struct objectinfo_diff_info* objectinfo_diff_info = (struct objectinfo_diff_info*)calloc(1, sizeof(objectinfo_diff_info));
    objectinfo_diff_info->verify = DIFF_INFO_VERIFY;

    struct split_filemaping* split_filemaping = assetfiles->split_filemapings[assetfile_index];
    unsigned char* filemaping_base = assetfiles->filemaping_base[assetfile_index];
    size_t offset = objectinfo_same->buffer - filemaping_base;

    if (split_filemaping) {
        if (split_filemaping->format) {
            int split_index = 0;
            for (int i = (int)split_filemaping->offsets_count - 1; i >= 0; --i) {
                if (offset > split_filemaping->offsets[i]) {
                    split_index = i;
                    offset -= split_filemaping->offsets[i];
                }
            }

            char split_asset_path[MAX_PATH];
            sprintf(split_asset_path, "%s.split%d", split_filemaping->name, split_index);

            objectinfo_diff_info->index = assetbundle_diff_insert(assetbundle_diff, split_asset_path);;
            objectinfo_diff_info->offset = offset;
        } else {
            objectinfo_diff_info->index = assetbundle_diff_insert(assetbundle_diff, split_filemaping->name);
            objectinfo_diff_info->offset = offset;
        }
    } else {
        objectinfo_diff_info->index = 0;
        objectinfo_diff_info->offset = offset;
    }

    unsigned char buffer[MAX_DIFF_INFO_SIZE];
    size_t buffer_offset = 0;

    buffer_offset += write_string(buffer, buffer_offset, objectinfo_diff_info->verify);
    buffer_offset += write_uint32(buffer, buffer_offset, objectinfo_diff_info->index, false);
    buffer_offset += write_uint32(buffer, buffer_offset, objectinfo_diff_info->offset, false);

    if (offset < objectinfo->length) {
        memset(objectinfo->buffer, 0, objectinfo->length);
        memcpy(objectinfo->buffer, buffer, offset);
    }

    free(objectinfo_diff_info);
}

void assetbundle_diff_process(struct assetbundle_diff* assetbundle_diff, struct assetfiles* assetfiles)
{
    size_t file_count = assetbundle_assetfile_count(assetbundle_diff->assetbundle);
    for (size_t i = 0; i < file_count; ++i)  {
        struct assetfile* assetfile = assetbundle_get_assetfile(assetbundle_diff->assetbundle, i);
        size_t obj_count = assetfile_objectinfo_count(assetfile);        
        for (size_t j = 0; i < obj_count; ++j) {
            struct objectinfo* objectinfo = assetfile_get_objectinfo(assetfile, j);
            objectinfo_diff(objectinfo, assetbundle_diff, assetfiles);
        }
    }
}

EXTERN_API errno_t assetbundle_diff(const char* assetfile_dir, const char* assetbundle_from, const char* assetbundle_to, const char* assetbundle_diff_name)
{
    int result = ASSETBUNDLE_FAILED;
    struct assetfiles* assetfiles = assetfiles_create();

	if (assetbundle_from) {
        if (!assetfiles_loadfrom_assetbundle(assetfiles, assetbundle_from)) {
            result = ASSETBUNDLE_FROM_LOAD_FAILED;
            goto Exit0;
        }
	}

    if (assetfile_dir) {
        traversedir(assetfile_dir, load_assetfile_dir, assetfiles, true);
    }

    struct assetbundle_diff* assetbundle_diff = assetbundle_diff_create(assetbundle_to, assetbundle_diff_name);
    if (!assetbundle_diff) {
        result = ASSETBUNDLE_DIFF_CREATE_FAILED;
        goto Exit0;
    }

    assetbundle_diff_process(assetbundle_diff, assetfiles);

    size_t length = assetbundle_diff->buffer_end - assetbundle_diff->buffer_begin;
    assetbundle_diff_destory(assetbundle_diff);

    filemaping_truncate(assetbundle_diff_name, length);

    result = ASSETBUNDLE_SUCCEED;
Exit0:
    return result;    
}
