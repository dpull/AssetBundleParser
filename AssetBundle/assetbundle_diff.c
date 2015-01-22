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
#include "assetbundle_imp.h"
#include "assetfile_diff.h"
#include "assetbundle_diff.h"

#define HASH_SIZE	(16)
extern void md5(const char* message, long len, char* output);

struct assetbundle_diff
{
	char src_hash[HASH_SIZE];
	char dst_hash[HASH_SIZE];

	struct assetbundle* bundle;
	struct assetfile_diff** file_diffs;
};

void assetbundle_md5(struct assetbundle* bundle, char* output)
{
    unsigned char* data = filemaping_getdata(bundle->filemaping);
    size_t length = filemaping_getlength(bundle->filemaping);
    
    md5((char*)data, (long)length, output);
}



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

	free(src_files);
    return diff;
}

bool assetbundle_diff_merge(const char* filename, struct assetbundle* from, struct assetbundle_diff* diff)
{
    size_t length = filemaping_getlength(diff->bundle->filemaping);
	struct filemaping* filemaping = filemaping_create_readwrite(filename, length);
    if (!filemaping)
        return false;

    bool result = false;
    unsigned char* data = filemaping_getdata(filemaping);
    size_t offset = 0;

    check_write_overlapping_zero_buffer(data, 0, length);
    
   	offset += assetbundle_header_save(&diff->bundle->header, data, offset);
   	offset += assetbundle_entryinfo_save(diff->bundle, data, offset);

	size_t src_files_count = from->entryinfo_count;
	struct assetfile** src_files = (struct assetfile**)malloc(sizeof(*src_files) * src_files_count);
	memset(src_files, 0, sizeof(*src_files) * src_files_count);
    
    for (size_t i = 0; i < src_files_count; ++i) {
        struct assetbundle_entryinfo* entryinfo = &from->entryinfo[i];
        src_files[i] = entryinfo->assetfile;
    }
    
 	for (size_t i = 0; i < diff->bundle->entryinfo_count; ++i) {
		struct assetbundle_entryinfo* entryinfo = &diff->bundle->entryinfo[i];
		size_t file_start = diff->bundle->header.header_size + entryinfo->offset;
		size_t file_offset = file_start;

		file_offset += assetfile_diff_savefile(entryinfo->assetfile, data, file_offset);
		assert(file_offset - file_start <= entryinfo->size);

		if (!assetfile_diff_merge(src_files, src_files_count, entryinfo->assetfile, diff->file_diffs[i], data, file_start, entryinfo->size))
            goto Exit0;
	}
    
    char diff_hash[HASH_SIZE];
    md5((char*)data, (long)length, diff_hash);
    if (memcmp(diff_hash, diff->dst_hash, sizeof(diff_hash)) != 0)
        goto Exit0;
    
    result = true;
Exit0:
	free(src_files);
    filemaping_destory(filemaping);
    return result;
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

struct assetbundle_diff* assetbundle_diff_load(const char* filename, struct assetbundle* to)
{
    struct filemaping* filemaping = filemaping_create_readonly(filename);
    if (!filemaping)
        return NULL;

    unsigned char* data = filemaping_getdata(filemaping);
    size_t length = filemaping_getlength(filemaping);
    char diff_hash[HASH_SIZE];

    md5((char*)(data + HASH_SIZE), (long)(length - HASH_SIZE), diff_hash);

    if (memcmp(diff_hash, data, sizeof(diff_hash)) != 0)
    	return NULL;

    struct assetbundle_diff* diff = (struct assetbundle_diff*)malloc(sizeof(*diff));
    memset(diff, 0, sizeof(*diff));
    diff->bundle = to;
    diff->bundle->filemaping = filemaping;

    size_t offset = HASH_SIZE;

    offset += write_buffer((unsigned char*)diff->src_hash, 0, data + offset, sizeof(diff->src_hash));
    offset += write_buffer((unsigned char*)diff->dst_hash, 0, data + offset, sizeof(diff->dst_hash));

    offset += assetbundle_header_load(&diff->bundle->header, data, offset);
   	offset += assetbundle_entryinfo_load(diff->bundle, data, offset);
    
    for (size_t i = 0; i < diff->bundle->entryinfo_count; ++i) {
        offset += assetfile_diff_loadfile(&diff->bundle->entryinfo[i].assetfile, data, offset);
    }
    
    diff->file_diffs = (struct assetfile_diff**)malloc(sizeof(*diff->file_diffs) * to->entryinfo_count);
    memset(diff->file_diffs, 0, sizeof(*diff->file_diffs) * to->entryinfo_count);
    
    for (size_t i = 0; i < diff->bundle->entryinfo_count; ++i) {
        offset += assetfile_diff_loaddiff(&diff->file_diffs[i], data, offset);
    }
    
    return diff;
}

bool assetbundle_diff_save(const char* filename, struct assetbundle_diff* diff)
{
    size_t length = filemaping_getlength(diff->bundle->filemaping);
    struct filemaping* filemaping = filemaping_create_readwrite(filename, length);
    if (!filemaping)
        return false;
    
    unsigned char* data = filemaping_getdata(filemaping);
    size_t offset = HASH_SIZE;
    
    check_write_overlapping_zero_buffer(data, 0, length);
    
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

    md5((char*)(data + HASH_SIZE), (long)(offset - HASH_SIZE), (char*)data);
    filemaping_destory(filemaping);
    filemaping_truncate(filename, offset);
    return true;
}

int assetbundle_diff(const char* assetbundle_from, const char* assetbundle_to, const char* assetbundle_diff)
{
	int result = ASSETBUNDLE_FAILED;
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

	result = ASSETBUNDLE_SUCCEED;
Exit0:
	if (diff)
		assetbundle_diff_destory(diff);
	
	if (to)
		assetbundle_destory(to);

	if (from)
		assetbundle_destory(from);
	return result;
}

struct assetbundle* assetbundle_safeload(const char* filename, const char* hashcheck)
{
	struct filemaping* filemaping = filemaping_create_readonly(filename);
	if (!filemaping)
		return NULL;

	unsigned char* data = filemaping_getdata(filemaping);
    size_t length = filemaping_getlength(filemaping);
    char hash[HASH_SIZE];
    
    md5((char*)data, (long)length, hash);

    if (memcmp(hash, hashcheck, sizeof(hash)) != 0) {
    	filemaping_destory(filemaping);
    	return NULL;
    }
    return assetbundle_load_filemaping(filemaping);
}

int assetbundle_merge(const char* assetbundle_from, const char* assetbundle_to, const char* assetbundle_diff)
{
	int result = ASSETBUNDLE_FAILED;
	struct assetbundle* from = NULL;
	struct assetbundle* to = NULL; 
	struct assetbundle_diff* diff = NULL;

	to = assetbundle_create();

	diff = assetbundle_diff_load(assetbundle_diff, to);
	if (!diff) {
		result = ASSETBUNDLE_DIFF_LOAD_FAILED;
		goto Exit0;
	}	

	from = assetbundle_safeload(assetbundle_from, diff->src_hash);
	if (!from) {
		result = ASSETBUNDLE_FROM_LOAD_FAILED;
		goto Exit0;
	}
    
    if (assetbundle_diff_merge(assetbundle_diff, from, diff)) {
        result = ASSETBUNDLE_FAILED;
        goto Exit0;
    }

	result = ASSETBUNDLE_SUCCEED;
Exit0:
	if (diff)
		assetbundle_diff_destory(diff);
	
	if (to)
		assetbundle_destory(to);

	if (from)
		assetbundle_destory(from);
	return result;
}