#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <fcntl.h>
#include <errno.h>
#include <assert.h>
#include <string.h>
#include <stdint.h>
#include "utils/platform.h"
#include "utils/debug_tree.h"
#include "utils/traversedir.h"
#include "tools.h"
#include "filemapping.h"
#include "object_class.h"
#include "assetfile_imp.h"
#include "assetfile.h"
#include "assetbundle.h"
#include "assetbundle_imp.h"
#include "assetbundle_diff.h"
#include "assetbundle_diff_imp.h"

#define SPLIT_FILE_FROMAT ("%s.split%d")

void split_filemapping_destory(struct split_filemapping* split_filemapping)
{
	if (split_filemapping->filemapping)
		filemapping_destory(split_filemapping->filemapping);

	if (split_filemapping->name)
		free(split_filemapping->name);

	if (split_filemapping->offsets)
		free(split_filemapping->offsets);

	free(split_filemapping);
}

struct split_filemapping* split_filemapping_create(const char* fullpath, const char* filename)
{
	struct split_filemapping* split_filemapping = (struct split_filemapping*)calloc(1, sizeof(*split_filemapping));
	const char* split0 = strstr(filename, ".split0");

	if (split0 == NULL) {
		split_filemapping->filemapping = filemapping_create_readonly(fullpath);
		if (!split_filemapping->filemapping) {
			split_filemapping_destory(split_filemapping);
			return NULL;
		}
		split_filemapping->name = strdup(filename);
		split_filemapping->offsets_count = 0;

		return split_filemapping;
	}
	else {
		char temp_file[MAX_PATH];
		strcpy(temp_file, "/tmp/split_filemapping.XXXXXX");
		mktemp(temp_file);

		split_filemapping->filemapping = filemapping_create_readwrite(temp_file, MAX_COMBINE_FILE_SIZE);
		if (!split_filemapping->filemapping) {
			split_filemapping_destory(split_filemapping);
			return NULL;
		}

		split_filemapping->name = strndup(filename, split0 - filename);
		split_filemapping->offsets = (size_t*)calloc(MAX_SPLIT_FILE_COUNT, sizeof(*split_filemapping->offsets));

		unsigned char* data = filemapping_getdata(split_filemapping->filemapping);
		size_t offset = 0;

		for (int i = 0; i < MAX_SPLIT_FILE_COUNT; ++i) {
			char split_asset_path[MAX_PATH];
			size_t dir_length = strlen(fullpath) - strlen(filename);
			strncpy(split_asset_path, fullpath, dir_length);
			sprintf(split_asset_path + dir_length, SPLIT_FILE_FROMAT, split_filemapping->name, i);

			struct filemapping* filemapping = filemapping_create_readonly(split_asset_path);
			if (!filemapping)
				break;

			split_filemapping->offsets[i] = offset;
			split_filemapping->offsets_count++;

			unsigned char* split_data = filemapping_getdata(filemapping);
			size_t split_length = filemapping_getlength(filemapping);
			memcpy(data + offset, split_data, split_length);

			offset += split_length;
			filemapping_destory(filemapping);
		}

		return split_filemapping;
	}
}

struct assetfiles* assetfiles_create()
{
	struct assetfiles* assetfiles = (struct assetfiles*)calloc(1, sizeof(*assetfiles));
	return assetfiles;
}

void assetfiles_destory(struct assetfiles* assetfiles)
{
	if (assetfiles->assetfiles) {
		for (size_t i = assetfiles->assetbundle_assetfile_count; i < assetfiles->count; ++i) {
			struct assetfile* assetfile = assetfiles->assetfiles[i];
			if (assetfile)
				assetfile_destory(assetfile);
		}
		free(assetfiles->assetfiles);
	}

	if (assetfiles->assetbundle) {
		assetbundle_destory(assetfiles->assetbundle);
	}

	if (assetfiles->split_filemappings) {
		for (size_t i = 0; i < assetfiles->count; ++i) {
			struct split_filemapping* split_filemapping = assetfiles->split_filemappings[i];
			if (split_filemapping)
				split_filemapping_destory(split_filemapping);
		}
		free(assetfiles->split_filemappings);
	}

	if (assetfiles->filemapping_base) {
		free(assetfiles->filemapping_base);
	}
    
	free(assetfiles);
}

void assetfiles_insert(struct assetfiles* assetfiles, struct assetfile* assetfile, struct split_filemapping* split_filemapping, unsigned char* filemapping_base)
{
	if (assetfiles->count == assetfiles->max_count) {
		assetfiles->max_count = 2 * ((assetfiles->max_count == 0) ? 100 : assetfiles->max_count);
		assetfiles->assetfiles = (struct assetfile**)realloc(assetfiles->assetfiles, assetfiles->max_count * sizeof(*assetfiles->assetfiles));
		assetfiles->split_filemappings = (struct split_filemapping**)realloc(assetfiles->split_filemappings, assetfiles->max_count * sizeof(*assetfiles->split_filemappings));
		assetfiles->filemapping_base = (unsigned char**)realloc(assetfiles->filemapping_base, assetfiles->max_count * sizeof(*assetfiles->filemapping_base));
	}

	assetfiles->assetfiles[assetfiles->count] = assetfile;
	assetfiles->split_filemappings[assetfiles->count] = split_filemapping;
	assetfiles->filemapping_base[assetfiles->count] = filemapping_base;
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
	unsigned char* data = filemapping_getdata(assetbundle->filemapping); // bad code,

	for (size_t i = 0; i < count; ++i)    {
		struct assetfile* assetfile = assetbundle_get_assetfile(assetbundle, i);
		assetfiles_insert(assetfiles, assetfile, NULL, data);
	}

	assetfiles->assetbundle = assetbundle;
	assetfiles->assetbundle_assetfile_count = count;
	return true;
}

bool load_assetfile_dir(const char* fullpath, const char* filename, void* userdata)
{
    struct assetfiles* assetfiles = (struct assetfiles*)userdata;
    size_t dir_root_length = *(size_t*)assetfiles->userdata;
    
	struct split_filemapping* split_filemapping = split_filemapping_create(fullpath, fullpath + dir_root_length);
	if (!split_filemapping)
		return true;

	unsigned char* data = filemapping_getdata(split_filemapping->filemapping);
	size_t length = filemapping_getlength(split_filemapping->filemapping);

	if (!is_assetfile(data, 0, length)) {
		split_filemapping_destory(split_filemapping);
		return true;
	}

	struct assetfile* assetfile = assetfile_load(data, 0, length);
	if (!assetfile) {
		split_filemapping_destory(split_filemapping);
		return true;
	}

	assetfiles_insert(assetfiles, assetfile, split_filemapping, data);
	return true;
}

void assetbundle_diff_destory(struct assetbundle_diff* assetbundle_diff)
{
	if (assetbundle_diff->assetfile_index)
		free(assetbundle_diff->assetfile_index);

	if (assetbundle_diff->assetbundle)
		assetbundle_destory(assetbundle_diff->assetbundle);

	if (assetbundle_diff->filemapping)
		filemapping_destory(assetbundle_diff->filemapping);

	free(assetbundle_diff);
}

struct assetbundle_diff* assetbundle_diff_loaddata(unsigned char* data, size_t length, bool is_load_assetbundle)
{
	struct assetbundle_diff* assetbundle_diff = (struct assetbundle_diff*)calloc(1, sizeof(*assetbundle_diff));
	size_t offset = 0;

	offset += read_uint32(data, offset, &assetbundle_diff->assetbundle_size, false);

	if (is_load_assetbundle) {
		assetbundle_diff->assetbundle = assetbundle_load_data(data + offset, length - offset);
		if (!assetbundle_diff->assetbundle) {
			assetbundle_diff_destory(assetbundle_diff);
			return NULL;
		}
	}

	offset += assetbundle_diff->assetbundle_size;

	offset += read_uint32(data, offset, &assetbundle_diff->assetfile_index_count, false);
	assetbundle_diff->assetfile_index = (char**)malloc(assetbundle_diff->assetfile_index_count * sizeof(*assetbundle_diff->assetfile_index));
	for (size_t i = 0; i < assetbundle_diff->assetfile_index_count; ++i) {
		offset += read_string(data, offset, assetbundle_diff->assetfile_index + i);
	}

	assetbundle_diff->buffer_begin = data;
	assetbundle_diff->buffer_end = data + length;
	assetbundle_diff->buffer_pos = data + offset;
	return assetbundle_diff;
}

struct assetbundle_diff* assetbundle_diff_load(const char* filename, bool is_load_assetbundle)
{
	struct filemapping* filemapping = filemapping_create_readonly(filename);
	if (!filemapping)
		return NULL;

	unsigned char* data = filemapping_getdata(filemapping);
	size_t length = filemapping_getlength(filemapping);

	struct assetbundle_diff* assetbundle_diff = assetbundle_diff_loaddata(data, length, is_load_assetbundle);
	if (!assetbundle_diff) {
		filemapping_destory(filemapping);
		return NULL;
	}
	assetbundle_diff->filemapping = filemapping;
	return assetbundle_diff;
}

bool assertbundle_checkfile(const char* filename)
{
	struct assetbundle* assetbundle = assetbundle_load(filename);
	if (!assetbundle)
		return false;

	bool ret = assetbundle_check(assetbundle);
	assetbundle_destory(assetbundle);
	return ret;
}

struct assetbundle_diff* assetbundle_diff_create(const char* src, const char* dst)
{
	if (!assertbundle_checkfile(src))
		return NULL;

	struct filemapping* src_filemapping = filemapping_create_readonly(src);
	if (!src_filemapping)
		return NULL;

	unsigned char* src_data = filemapping_getdata(src_filemapping);
	size_t src_length = filemapping_getlength(src_filemapping);

	struct filemapping* dst_filemapping = filemapping_create_readwrite(dst, src_length + MAX_DIFF_RESERVED_SIZE);
	if (!dst_filemapping) {
		filemapping_destory(src_filemapping);
		return NULL;
	}

	unsigned char* dst_data = filemapping_getdata(dst_filemapping);
	size_t dst_length = filemapping_getlength(dst_filemapping);
	memset(dst_data, 0, dst_length);

	size_t offset = 0;
	offset += write_uint32(dst_data, offset, src_length, false);
	offset += write_buffer(dst_data, offset, src_data, src_length);
	offset += write_uint32(dst_data, offset, 0, false);

	struct assetbundle_diff* assetbundle_diff = assetbundle_diff_loaddata(dst_data, dst_length, true);
	if (!assetbundle_diff) {
		filemapping_destory(src_filemapping);
		filemapping_destory(dst_filemapping);
		return NULL;
	}
	assetbundle_diff->filemapping = dst_filemapping;
	filemapping_destory(src_filemapping);
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
	memset(assetbundle_diff->buffer_begin + 4 + assetbundle_diff->assetbundle_size, 0, 4);
	write_uint32(assetbundle_diff->buffer_begin, 4 + assetbundle_diff->assetbundle_size, assetbundle_diff->assetfile_index_count, false);

	assetbundle_diff->assetfile_index = realloc(assetbundle_diff->assetfile_index, assetbundle_diff->assetfile_index_count * sizeof(*assetbundle_diff->assetfile_index));

	read_string(buffer_pos, 0, assetbundle_diff->assetfile_index + assetbundle_diff->assetfile_index_count - 1);
	return assetbundle_diff->assetfile_index_count;
}

void objectinfo_diff(struct objectinfo* objectinfo, struct assetbundle_diff* assetbundle_diff, struct assetfiles* assetfiles)
{
	struct objectinfo* objectinfo_same = NULL;
	size_t assetfile_index = 0;

	for (size_t i = 0; i < assetfiles->count; ++i) {
		objectinfo_same = objectinfo_find(objectinfo, assetfiles->assetfiles[i]);
		if (objectinfo_same) {
			assetfile_index = i;
			break;
		}
	}

	if (!objectinfo_same)
		return;

	if (objectinfo->length <= sizeof(struct objectinfo_diff_info))
		return;

	struct objectinfo_diff_info objectinfo_diff_info;
	memcpy(objectinfo_diff_info.verify, DIFF_INFO_VERIFY, sizeof(DIFF_INFO_VERIFY));

	struct split_filemapping* split_filemapping = assetfiles->split_filemappings[assetfile_index];
	unsigned char* filemapping_base = assetfiles->filemapping_base[assetfile_index];

	objectinfo_diff_info.index = 0;
	objectinfo_diff_info.offset = (uint32_t)(objectinfo_same->buffer - filemapping_base);

	if (split_filemapping && split_filemapping->offsets) {
		int split_index = 0;
		for (int i = (int)split_filemapping->offsets_count - 1; i >= 0; --i) {
			if (objectinfo_diff_info.offset > split_filemapping->offsets[i]) {
				split_index = i;
				objectinfo_diff_info.offset -= split_filemapping->offsets[i];
				break;
			}
		}

		char split_asset_path[MAX_PATH];
		sprintf(split_asset_path, SPLIT_FILE_FROMAT, split_filemapping->name, split_index);

		objectinfo_diff_info.index = assetbundle_diff_insert(assetbundle_diff, split_asset_path);;
	}
	else if (split_filemapping) {
		objectinfo_diff_info.index = assetbundle_diff_insert(assetbundle_diff, split_filemapping->name);
	}

	memset(objectinfo->buffer, 0, objectinfo->length);
	memcpy(objectinfo->buffer, &objectinfo_diff_info, sizeof(objectinfo_diff_info));
}

void assetbundle_diff_process(struct assetbundle_diff* assetbundle_diff, struct assetfiles* assetfiles)
{
	size_t file_count = assetbundle_assetfile_count(assetbundle_diff->assetbundle);
	for (size_t i = 0; i < file_count; ++i)  {
		struct assetfile* assetfile = assetbundle_get_assetfile(assetbundle_diff->assetbundle, i);
		size_t obj_count = assetfile_objectinfo_count(assetfile);
		for (size_t j = 0; j < obj_count; ++j) {
			struct objectinfo* objectinfo = assetfile_get_objectinfo(assetfile, j);
			objectinfo_diff(objectinfo, assetbundle_diff, assetfiles);
		}
	}
}

bool get_objectinfo_diff_info(unsigned char* buffer, size_t length, struct objectinfo_diff_info* ret_objectinfo_diff_info)
{
    if (length < sizeof(*ret_objectinfo_diff_info))
        return false;
    
    if (strcmp((char*)buffer, DIFF_INFO_VERIFY) != 0)
        return false;
    
    memcpy(ret_objectinfo_diff_info, buffer, sizeof(*ret_objectinfo_diff_info));
    return true;
}

EXTERN_API errno_t assetbundle_diff(const char* dir, const char* from, const char* to, const char* diff)
{
	struct assetfiles* assetfiles = assetfiles_create();
    
	if (from && !assetfiles_loadfrom_assetbundle(assetfiles, from)) {
		assetfiles_destory(assetfiles);
		return ASSETBUNDLE_FROM_LOAD_FAILED;
	}

	if (dir) {
        size_t dir_length = strlen(dir);
        if (dir_length > 0 && dir[dir_length - 1] != '/' && dir[dir_length - 1] != '\\')
            dir_length++;
        
        assetfiles->userdata = &dir_length;
		traversedir(dir, load_assetfile_dir, assetfiles, true);
        assetfiles->userdata = NULL;
	}

	struct assetbundle_diff* assetbundle_diff = assetbundle_diff_create(to, diff);
	if (!assetbundle_diff) {
		assetfiles_destory(assetfiles);
		return ASSETBUNDLE_DIFF_CREATE_FAILED;
	}

	assetbundle_diff_process(assetbundle_diff, assetfiles);

	size_t assetbundle_diff_length = assetbundle_diff->buffer_pos - assetbundle_diff->buffer_begin;
	assetbundle_diff_destory(assetbundle_diff);
	assetfiles_destory(assetfiles);

	filemapping_truncate(diff, assetbundle_diff_length);

	return ASSETBUNDLE_SUCCEED;
}

EXTERN_API void assetbundle_diff_print(const char* filename)
{
	struct assetbundle_diff* assetbundle_diff = assetbundle_diff_load(filename, true);
	if (!assetbundle_diff)
		return;

	struct debug_tree* root = debug_tree_create(NULL, "*");

	struct debug_tree* fileindex = debug_tree_create(root, "fileindex count:%hu", assetbundle_diff->assetfile_index_count);
	for (size_t i = 0; i < assetbundle_diff->assetfile_index_count; ++i) {
		debug_tree_create(fileindex, "index:%u\t%s", i + 1, assetbundle_diff->assetfile_index[i]);
	}

	struct debug_tree* unchangelist = debug_tree_create(root, "unchange list");
	struct debug_tree* changelist = debug_tree_create(root, "changed list");
    struct objectinfo_diff_info objectinfo_diff_info;

	size_t file_count = assetbundle_assetfile_count(assetbundle_diff->assetbundle);
	for (size_t i = 0; i < file_count; ++i)  {
		struct assetfile* assetfile = assetbundle_get_assetfile(assetbundle_diff->assetbundle, i);
		size_t obj_count = assetfile_objectinfo_count(assetfile);

		for (size_t j = 0; j < obj_count; ++j) {
			struct objectinfo* objectinfo = assetfile_get_objectinfo(assetfile, j);
			if (get_objectinfo_diff_info(objectinfo->buffer, objectinfo->length, &objectinfo_diff_info)) {
				debug_tree_create(unchangelist, "path_id:%d\tindex:%hu\toffset:%u", objectinfo->path_id, objectinfo_diff_info.index, objectinfo_diff_info.offset);
			}
			else {
				char* objectinfo_name = objectinfo_getname(objectinfo->buffer, 0, objectinfo->length);
				debug_tree_create(changelist, "path_id:%d\tname:%s", objectinfo->path_id, objectinfo_name);
				free(objectinfo_name);
			}
		}
	}

	struct debug_tree* assetbundle = debug_tree_create(root, "assetbundle size:%u", assetbundle_diff->assetbundle_size);
	assetbundle_print(assetbundle_diff->assetbundle, assetbundle);

	debug_tree_print(root, stdout);
	debug_tree_destory(root);
	assetbundle_diff_destory(assetbundle_diff);
}

EXTERN_API errno_t assetbundle_merge(readfile_callback* fn_readfile, void* userdata, const char* from, const char* to, const char* diff)
{
	struct filemapping* filemapping_from = NULL;
	unsigned char* data_from = NULL;
	size_t data_from_length = 0;

	if (from) {
		filemapping_from = filemapping_create_readonly(from);
		if (!filemapping_from) {
			return ASSETBUNDLE_FROM_LOAD_FAILED;
		}
		data_from = filemapping_getdata(filemapping_from);
		data_from_length = filemapping_getlength(filemapping_from);
	}

	struct assetbundle_diff* assetbundle_diff = assetbundle_diff_load(diff, false);
	if (!assetbundle_diff) {
		filemapping_destory(filemapping_from);
		return ASSETBUNDLE_DIFF_LOAD_FAILED;
	}

	struct filemapping* filemapping_to = filemapping_create_readwrite(to, assetbundle_diff->assetbundle_size);
    if (!filemapping_to) {
        assetbundle_diff_destory(assetbundle_diff);
        filemapping_destory(filemapping_from);
        return ASSETBUNDLE_TO_CREATE_FAILED;
    }
        
	unsigned char* data_to = filemapping_getdata(filemapping_to);
	memcpy(data_to, assetbundle_diff->buffer_begin + sizeof(uint32_t), assetbundle_diff->assetbundle_size);

	struct assetbundle* assetbundle_to = assetbundle_load_data(data_to, assetbundle_diff->assetbundle_size);
    
    struct objectinfo_diff_info objectinfo_diff_info;
	bool merge_error = false;
	size_t file_count = assetbundle_assetfile_count(assetbundle_to);
	for (size_t i = 0; i < file_count; ++i)  {
		struct assetfile* assetfile = assetbundle_get_assetfile(assetbundle_to, i);
		size_t obj_count = assetfile_objectinfo_count(assetfile);

		for (size_t j = 0; j < obj_count; ++j) {
			struct objectinfo* objectinfo = assetfile_get_objectinfo(assetfile, j);
            if (!get_objectinfo_diff_info(objectinfo->buffer, objectinfo->length, &objectinfo_diff_info))
				continue;

			if (objectinfo_diff_info.index == 0) {
				assert(objectinfo_diff_info.offset + objectinfo->length < data_from_length);
				memcpy(objectinfo->buffer, data_from + objectinfo_diff_info.offset, objectinfo->length);
				continue;
			}

			assert(objectinfo_diff_info.index <= assetbundle_diff->assetfile_index_count);
			char* assetfile_name = assetbundle_diff->assetfile_index[objectinfo_diff_info.index - 1];
			if (!fn_readfile(objectinfo->buffer, assetfile_name, objectinfo_diff_info.offset, objectinfo->length, userdata)) {
				merge_error = true;
				break;
			}
		}
	}

	assetbundle_diff_destory(assetbundle_diff);
	assetbundle_destory(assetbundle_to);
	filemapping_destory(filemapping_to);
    if (filemapping_from)
        filemapping_destory(filemapping_from);
	return merge_error ? ASSETBUNDLE_FAILED : ASSETBUNDLE_SUCCEED;
}