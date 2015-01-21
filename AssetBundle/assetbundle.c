#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <errno.h>
#include <assert.h>
#include <string.h>
#include "assetfile.h"
#include "tools.h"
#include "filemaping.h"
#include "assetbundle.h"

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
	unsigned char compressed; 
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

const int assetbundle_entryinfo_align = 4;

struct assetbundle
{
	struct assetbundle_header header;
	size_t entryinfo_count;
	struct assetbundle_entryinfo* entryinfo;
    size_t align_data_length;
    unsigned char* align_data;

	struct filemaping* filemaping;
};

size_t assetbundle_entryinfo_load(struct assetbundle* bundle, unsigned char* data, size_t offset)
{
	size_t start = offset;
	offset += read_uint32(data, offset, &bundle->entryinfo_count, false);

	bundle->entryinfo = (struct assetbundle_entryinfo*)malloc(sizeof(*bundle->entryinfo) * bundle->entryinfo_count);
	memset(bundle->entryinfo, 0, sizeof(*bundle->entryinfo) * bundle->entryinfo_count);

	for (size_t i = 0; i < bundle->entryinfo_count; ++i) {
		struct assetbundle_entryinfo* entryinfo = &bundle->entryinfo[i];
		offset += read_string(data, offset, &entryinfo->name);
		offset += read_uint32(data, offset, &entryinfo->offset, false);
		offset += read_uint32(data, offset, &entryinfo->size, false);
	}
    
    bundle->align_data_length = offset % assetbundle_entryinfo_align;
    if (bundle->align_data_length != 0) {
        bundle->align_data_length = assetbundle_entryinfo_align - bundle->align_data_length;
        offset += read_buffer(data, offset, &bundle->align_data, bundle->align_data_length);
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
	}
    
    if (bundle->align_data_length != 0) {
        offset += write_buffer(data, offset, bundle->align_data, bundle->align_data_length);
    }
    return offset - start;
}

bool assetfiles_load(struct assetbundle* bundle, unsigned char* data)
{
	for (size_t i = 0; i < bundle->entryinfo_count; ++i) {
		struct assetbundle_entryinfo* entryinfo = &bundle->entryinfo[i];
		size_t file_offset = bundle->header.header_size + entryinfo->offset;
		assert(file_offset + entryinfo->size <= filemaping_getlength(bundle->filemaping));

		entryinfo->assetfile = assetfile_load(data, file_offset, entryinfo->size);
    	assert(entryinfo->assetfile);
	} 
	return true;
}

bool assetfiles_save(struct assetbundle* bundle, unsigned char* data)
{
	for (size_t i = 0; i < bundle->entryinfo_count; ++i) {
		struct assetbundle_entryinfo* entryinfo = &bundle->entryinfo[i];
		size_t file_offset = bundle->header.header_size + entryinfo->offset;

        bool ret = assetfile_save(entryinfo->assetfile, data, file_offset, entryinfo->size);
        assert(ret);
	} 
	return true;
}

void assetbundle_entryinfo_destory(struct assetbundle* bundle)
{
	for (size_t i = 0; i < bundle->entryinfo_count; ++i) {
		struct assetbundle_entryinfo* entryinfo = &bundle->entryinfo[i];
		if (entryinfo->assetfile)
			assetfile_destory(entryinfo->assetfile);
	}
	free(bundle->entryinfo);	
}

struct assetbundle* assetbundle_load_filemaping(struct filemaping* filemaping)
{
    struct assetbundle* bundle = (struct assetbundle*)malloc(sizeof(*bundle));
    memset(bundle, 0, sizeof(*bundle));
    
    bundle->filemaping = filemaping;
    
    unsigned char* data = filemaping_getdata(filemaping);
    size_t offset = 0;
   	offset += assetbundle_header_load(&bundle->header, data, offset);
    assert(strcmp(bundle->header.signature, "UnityRaw") == 0); // only support uncompressed assetbundle
    
   	offset += assetbundle_entryinfo_load(bundle, data, offset);

  	bool ret = assetfiles_load(bundle, data);
    assert(ret);

    return bundle;
}

struct assetbundle* assetbundle_load(const char* filename)
{
	struct filemaping* filemaping = filemaping_create_readonly(filename);
	if (!filemaping)
		return NULL;

    return assetbundle_load_filemaping(filemaping);
}

bool assetbundle_check(struct assetbundle* bundle)
{
	size_t length = filemaping_getlength(bundle->filemaping);
	unsigned char* dest_data = (unsigned char*)malloc(length);
    size_t offset = 0;

   	offset += assetbundle_header_save(&bundle->header, dest_data, offset);
   	offset += assetbundle_entryinfo_save(bundle, dest_data, offset);

  	bool ret = assetfiles_save(bundle, dest_data);
    assert(ret);
    
    int error_bytes = 0;
	unsigned char* src_data = filemaping_getdata(bundle->filemaping);

	for (size_t i = 0; i < length; ++i) {
		if (src_data[i] != dest_data[i]) {
			printf("offset %d:[%hho]\t[%hho]\n", (int)i, src_data[i], dest_data[i]);
            error_bytes++;
		}
   	}

   	free(dest_data);

   	if (error_bytes > 0) 
		printf("check failed, error bytes count:%d\n", error_bytes);
	else
		printf("check succeed\n");

    return error_bytes == 0;
}

void assetbundle_destory(struct assetbundle* bundle)
{
	assetbundle_entryinfo_destory(bundle);
	filemaping_destory(bundle->filemaping);
	free(bundle);
}

extern void md5(const char* message, long len, char* output);

void assetbundle_md5(struct assetbundle* bundle, char* output)
{
    unsigned char* data = filemaping_getdata(bundle->filemaping);
    size_t length = filemaping_getlength(bundle->filemaping);
    
    md5((char*)data, (long)length, output);
}

const int hash_size = 16;
struct assetbundle_diff
{
    char src_hash[hash_size];
    char dst_hash[hash_size];

	struct assetbundle* bundle;
    struct assetfile_diff** file_diffs;
};

struct assetbundle_diff* assetbundle_diff_create(struct assetbundle* from, struct assetbundle* to)
{
    struct assetbundle_diff* diff = (struct assetbundle_diff*)malloc(sizeof(*diff));
    
    assetbundle_md5(from, diff->src_hash);
    assetbundle_md5(to, diff->dst_hash);
    diff->bundle = to;
    diff->file_diffs = (struct assetfile_diff**)malloc(sizeof(*diff->file_diffs) * to->entryinfo_count);
    memset(diff->file_diffs, 0, sizeof(*diff->file_diffs) * to->entryinfo_count);
    
	size_t src_files_count = from->entryinfo_count;
	struct assetfile** src_files = (struct assetfile**)malloc(sizeof(*src_files) * src_files_count);
	memset(src_files, 0, sizeof(*src_files) * src_files_count);

	for (size_t i = 0; i < src_files_count; ++i) {
		struct assetbundle_entryinfo* entryinfo = &from->entryinfo[i];
		src_files[i] = entryinfo->assetfile;
	}
    
	for (size_t i = 0; i < to->entryinfo_count; ++i) {
		struct assetbundle_entryinfo* entryinfo = &to->entryinfo[i];
		diff->file_diffs[i] = assetfile_diff(src_files, src_files_count, entryinfo->assetfile);
        assert(diff->file_diffs[i]);
	}
    return diff;
}

void assetbundle_diff_destory(struct assetbundle_diff* diff)
{
    for (size_t i = 0; i < diff->bundle->entryinfo_count; ++i) {
        if (diff->file_diffs[i])
            assetfile_diff_destory(diff->file_diffs[i]);
    }
    free(diff->file_diffs);
    free(diff);
}

struct assetbundle_diff* assetbundle_diff_load(char* filename)
{
    struct filemaping* filemaping = filemaping_create_readonly(filename);
    if (!filemaping)
        return NULL;

    unsigned char* data = filemaping_getdata(filemaping);
    size_t length = filemaping_getlength(filemaping);
    char diff_hash[hash_size];

    md5((char*)(data + hash_size), (long)(length - hash_size), diff_hash);

    if (memcmp(diff_hash, data, sizeof(diff_hash)) != 0)
    	return NULL;

    struct assetbundle_diff* diff = (struct assetbundle_diff*)malloc(sizeof(*diff));
    size_t offset = hash_size;
    
    offset += write_buffer((unsigned char*)diff->src_hash, 0, data + offset, sizeof(diff->src_hash));
    offset += write_buffer((unsigned char*)diff->dst_hash, 0, data + offset, sizeof(diff->dst_hash));
    
    offset += assetbundle_header_load(&diff->bundle->header, data, offset);
   	offset += assetbundle_entryinfo_load(diff->bundle, data, offset);
    
    for (size_t i = 0; i < diff->bundle->entryinfo_count; ++i) {
        assert (diff->bundle->entryinfo[i].assetfile);
        offset += assetfile_diff_savefile(diff->bundle->entryinfo[i].assetfile, data, offset);
    }
    
    for (size_t i = 0; i < diff->bundle->entryinfo_count; ++i) {
        assert (diff->file_diffs[i]);
        offset += assetfile_diff_savediff(diff->file_diffs[i], data, offset);
    }
    
    md5((char*)(data + hash_size), (long)(offset - hash_size), (char*)data);
    filemaping_destory(filemaping);
    filemaping_truncate(filename, offset);
    
    return diff;
}

bool assetbundle_diff_save(const char* filename, struct assetbundle_diff* diff)
{
    size_t length = filemaping_getlength(diff->bundle->filemaping);
    struct filemaping* filemaping = filemaping_create_readwrite(filename, length);
    if (!filemaping)
        return false;
    
    unsigned char* data = filemaping_getdata(filemaping);
    size_t offset = hash_size;
    
    offset += write_buffer(data, offset, (unsigned char*)diff->src_hash, sizeof(diff->src_hash));
    offset += write_buffer(data, offset, (unsigned char*)diff->dst_hash, sizeof(diff->dst_hash));
    
    offset += assetbundle_header_save(&diff->bundle->header, data, offset);
   	offset += assetbundle_entryinfo_save(diff->bundle, data, offset);
    
    for (size_t i = 0; i < diff->bundle->entryinfo_count; ++i) {
        assert (diff->bundle->entryinfo[i].assetfile);
        offset += assetfile_diff_savefile(diff->bundle->entryinfo[i].assetfile, data, offset);
    }
    
    for (size_t i = 0; i < diff->bundle->entryinfo_count; ++i) {
        assert (diff->file_diffs[i]);
        offset += assetfile_diff_savediff(diff->file_diffs[i], data, offset);
    }

    md5((char*)(data + hash_size), (long)(offset - hash_size), (char*)data);
    filemaping_destory(filemaping);
    filemaping_truncate(filename, offset);
    return true;
}

int assetbundle_diff(const char* assetbundle_from, const char* assetbundle_to, const char* assetbundle_diff)
{
	int result = ASSETBUNDLE_DIFF_FAILED;
	bool retcode = false;
	struct assetbundle* from = NULL;
	struct assetbundle* to = NULL;
	struct assetbundle_diff* diff = NULL;

	from = assetbundle_load(assetbundle_from);
	if (!from) {
		result = ASSETBUNDLE_FROM_LOAD_FAILED;
		goto Exit0;
	}

	retcode = assetbundle_check(from);
	if (!retcode) {
		result = ASSETBUNDLE_FROM_CHECK_FAILED;
		goto Exit0;
	}

	to = assetbundle_load(assetbundle_to);
	if (!to) {
		result = ASSETBUNDLE_TO_LOAD_FAILED;
		goto Exit0;
	}

	retcode = assetbundle_check(to);
	if (!retcode) {
		result = ASSETBUNDLE_TO_CHECK_FAILED;
		goto Exit0;
	}

	diff = assetbundle_diff_create(from, to);
	if (!diff) {
		result = ASSETBUNDLE_DIFF_CREATE_FAILED;
		goto Exit0;
	}
	
	retcode = assetbundle_diff_save(assetbundle_diff, diff);
	if (!retcode) {
		result = ASSETBUNDLE_DIFF_SAVE_FAILED;
		goto Exit0;
	}

	result = ASSETBUNDLE_DIFF_SUCCEED;
Exit0:
	if (diff)
		assetbundle_diff_destory(diff);
	
	if (to)
		assetbundle_destory(to);

	if (from)
		assetbundle_destory(from);
	return result;
}
