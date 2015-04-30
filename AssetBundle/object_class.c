#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdbool.h>
#include "utils/platform.h"
#include "tools.h"
#include "object_class.h"

char* objectinfo_getname(unsigned char* data, size_t start, size_t size)
{
	int name_len;
	size_t offset = start;
	offset += read_int32(data, offset, &name_len, true);
	if (name_len >= 0 && name_len <= (int)size)
		return strndup((char*)data + offset, name_len);
	return strdup("");
}

struct object_class_audioclip
{
	int name_len;
	unsigned char* name;
	int format;
	int type;
	unsigned char is_3d;
	unsigned char use_hardware;
	int stream;
	int data_size;
	unsigned char* data;
};

struct object_class_audioclip* object_class_audioclip_load(unsigned char* data, size_t start, size_t size)
{
	size_t offset = start;
	struct object_class_audioclip* object_class = (struct object_class_audioclip*)calloc(1, sizeof(*object_class));

	offset += read_int32(data, offset, &object_class->name_len, true);
	if (object_class->name_len < 0 || object_class->name_len >(int)size) {
		free(object_class);
		return NULL;
	}
	offset += read_buffer(data, offset, &object_class->name, object_class->name_len);
	offset = (offset + 3) / 4 * 4;

	offset += read_int32(data, offset, &object_class->format, true);
	offset += read_int32(data, offset, &object_class->type, true);
	offset += read_byte(data, offset, &object_class->is_3d);
	offset += read_byte(data, offset, &object_class->use_hardware);

	offset = (offset + 3) / 4 * 4;

	offset += read_int32(data, offset, &object_class->stream, true);
	offset += read_int32(data, offset, &object_class->data_size, true);
	offset += read_buffer(data, offset, &object_class->data, object_class->data_size);

	return object_class;
}

void object_class_audioclip_destroy(struct object_class_audioclip* object_class)
{
	if (object_class)
		free(object_class);
}

bool is_assetfile(unsigned char* data, size_t start, size_t size)
{
	if (size < 23)
		return false;

	if (strncmp((char*)data + start + 20, "4.", sizeof("4.") - 1) == 0)
		return true;

	if (strncmp((char*)data + start + 20, "5.", sizeof("5.") - 1) == 0)
		return true;

	return false;
}

bool is_assetbundle(unsigned char* data)
{
	return (strncmp((char*)data, "Unity", sizeof("Unity") - 1) == 0);
}