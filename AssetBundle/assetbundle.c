#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <errno.h>
#include <assert.h>
#include <string.h>
#include <stdint.h>
#include "utils/platform.h"
#include "utils/debug_tree.h"
#include "tools.h"
#include "filemapping.h"
#include "assetfile.h"
#include "assetbundle.h"
#include "assetbundle_imp.h"

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

	bundle->align_data_length = offset % ASSETBUNDLE_ENTRYINFO_ALIGN;
	if (bundle->align_data_length != 0) {
		bundle->align_data_length = ASSETBUNDLE_ENTRYINFO_ALIGN - bundle->align_data_length;
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

void assetbundle_entryinfo_destory(struct assetbundle* bundle)
{
	for (size_t i = 0; i < bundle->entryinfo_count; ++i) {
		struct assetbundle_entryinfo* entryinfo = &bundle->entryinfo[i];
		if (entryinfo->assetfile)
			assetfile_destory(entryinfo->assetfile);
	}
	free(bundle->entryinfo);
}

EXTERN_API struct assetbundle* assetbundle_load_data(unsigned char* data, size_t length)
{
	struct assetbundle* bundle = (struct assetbundle*)malloc(sizeof(*bundle));
	memset(bundle, 0, sizeof(*bundle));
	size_t offset = 0;

	offset += assetbundle_header_load(&bundle->header, data, offset);
	assert(strcmp(bundle->header.signature, "UnityRaw") == 0); // only support uncompressed assetbundle

	offset += assetbundle_entryinfo_load(bundle, data, offset);

	for (size_t i = 0; i < bundle->entryinfo_count; ++i) {
		struct assetbundle_entryinfo* entryinfo = &bundle->entryinfo[i];
		size_t file_offset = bundle->header.header_size + entryinfo->offset;
		assert(file_offset + entryinfo->size <= length);

		entryinfo->assetfile = assetfile_load(data, file_offset, entryinfo->size);
		assert(entryinfo->assetfile);
	}

	return bundle;
}

EXTERN_API struct assetbundle* assetbundle_load(const char* filename)
{
	struct filemapping* filemapping = filemapping_create_readonly(filename);
	if (!filemapping)
		return NULL;

	unsigned char* data = filemapping_getdata(filemapping);
	size_t length = filemapping_getlength(filemapping);

	struct assetbundle* bundle = assetbundle_load_data(data, length);
	bundle->filemapping = filemapping;
	return bundle;
}

EXTERN_API bool assetbundle_check(struct assetbundle* bundle)
{
	size_t length = filemapping_getlength(bundle->filemapping);
	unsigned char* dest_data = (unsigned char*)malloc(length);
	memset(dest_data, 0, length);

	size_t offset = 0;
	offset += assetbundle_header_save(&bundle->header, dest_data, offset);
	offset += assetbundle_entryinfo_save(bundle, dest_data, offset);

	for (size_t i = 0; i < bundle->entryinfo_count; ++i) {
		struct assetbundle_entryinfo* entryinfo = &bundle->entryinfo[i];
		size_t file_offset = bundle->header.header_size + entryinfo->offset;

		bool ret = assetfile_save(entryinfo->assetfile, dest_data, file_offset, entryinfo->size);
		assert(ret);
	}

	int error_bytes = 0;
	unsigned char* src_data = filemapping_getdata(bundle->filemapping);

	for (size_t i = 0; i < length; ++i) {
		if (src_data[i] != dest_data[i]) {
			printf("offset %d:[%hho]\t[%hho]\n", (int)i, src_data[i], dest_data[i]);
			error_bytes++;
		}
	}

	free(dest_data);

	if (error_bytes > 0)
		printf("check failed, error bytes count:%d\n", error_bytes);

	return error_bytes == 0;
}

EXTERN_API void assetbundle_destory(struct assetbundle* bundle)
{
	assetbundle_entryinfo_destory(bundle);
	if (bundle->filemapping)
		filemapping_destory(bundle->filemapping);
	free(bundle);
}

EXTERN_API void assetbundle_print(struct assetbundle* bundle, struct debug_tree* root)
{
	struct debug_tree* header = debug_tree_create(root, "header");

	debug_tree_create(header, "header_size:%d", bundle->header.header_size);
	debug_tree_create(header, "level_byte_end_count:%u", bundle->header.level_byte_end_count);
	debug_tree_create(header, "complete_file_size:%u", bundle->header.complete_file_size);
	debug_tree_create(header, "data_header_size:%u", bundle->header.data_header_size);


	for (size_t i = 0; i < bundle->entryinfo_count; ++i) {
		struct assetbundle_entryinfo* entryinfo = &bundle->entryinfo[i];
		struct debug_tree* debug_tree = debug_tree_create(root, "entryinfo:%s, offset:%u, size:%u", entryinfo->name, entryinfo->offset, entryinfo->size);
		if (entryinfo->assetfile)
			assetfile_print(entryinfo->assetfile, debug_tree);
	}
}

EXTERN_API size_t assetbundle_assetfile_count(struct assetbundle* bundle)
{
	return bundle->entryinfo_count;
}

EXTERN_API struct assetfile*  assetbundle_get_assetfile(struct assetbundle* bundle, size_t index)
{
	return bundle->entryinfo[index].assetfile;
}