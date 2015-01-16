#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "tools.h"

size_t read_string(unsigned char* data, size_t offset, char** value)
{
    size_t len = strlen((char*)data + offset) + 1;
    return read_buffer(data, offset, (unsigned char**)value, len);
}

size_t write_string(unsigned char* data, size_t offset, char* value)
{
    size_t len = strlen(value) + 1;
    return write_buffer(data, offset, (unsigned char*)value, len);
}

size_t read_int16(unsigned char* data, size_t offset, short* value, bool littleEndian)
{
    if (!littleEndian)
        *value = (short)((data[offset] << 8) | data[offset + 1]);
    else
        *value = (short)((data[offset + 1] << 8) | data[offset]);
    return 2;
}

size_t write_int16(unsigned char* data, size_t offset, short value, bool littleEndian)
{
    if (!littleEndian) {
        data[offset + 1] = (unsigned char)value;
        data[offset] = (unsigned char)(value >> 8);
    } else {
        data[offset] = (unsigned char)value;
        data[offset + 1] = (unsigned char)(value >> 8);
    }
    return 2;
}

size_t read_int32(unsigned char* data, size_t offset, int* value, bool littleEndian)
{
    if (!littleEndian)
        *value = (data[offset] << 24) | (data[offset + 1] << 16) | (data[offset + 2] << 8) | data[offset + 3];
    else
        *value = (data[offset + 3] << 24) | (data[offset + 2] << 16) | (data[offset + 1] << 8) | data[offset];
    return 4;
}

size_t write_int32(unsigned char* data, size_t offset, int value, bool littleEndian)
{
    if (!littleEndian) {
        data[offset + 3] = (unsigned char)value;
        data[offset + 2] = (unsigned char)(value >> 8);
        data[offset + 1] = (unsigned char)(value >> 16);
        data[offset] = (unsigned char)(value >> 24);
    } else {
        data[offset] = (unsigned char)value;
        data[offset + 1] = (unsigned char)(value >> 8);
        data[offset + 2] = (unsigned char)(value >> 16);
        data[offset + 3] = (unsigned char)(value >> 24);
    }
    return 4;
}

size_t read_uint32(unsigned char* data, size_t offset, size_t* value, bool littleEndian)
{
    if (!littleEndian)
        *value = (size_t)((data[offset] << 24) | (data[offset + 1] << 16) | (data[offset + 2] << 8) | data[offset + 3]);
    else
        *value = (size_t)((data[offset + 3] << 24) | (data[offset + 2] << 16) | (data[offset + 1] << 8) | data[offset]);
    return 4;
}

size_t write_uint32(unsigned char* data, size_t offset, size_t value, bool littleEndian)
{
    if (!littleEndian) {
        data[offset + 3] = (unsigned char)value;
        data[offset + 2] = (unsigned char)(value >> 8);
        data[offset + 1] = (unsigned char)(value >> 16);
        data[offset] = (unsigned char)(value >> 24);
    } else {
        data[offset] = (unsigned char)value;
        data[offset + 1] = (unsigned char)(value >> 8);
        data[offset + 2] = (unsigned char)(value >> 16);
        data[offset + 3] = (unsigned char)(value >> 24);
    }
    return 4;
}

size_t read_byte(unsigned char* data, size_t offset, unsigned char* value)
{
    *value = data[offset];
    return 1;
}

size_t write_byte(unsigned char* data, size_t offset, unsigned char value)
{
    data[offset] = value;
    return 1;
}

size_t read_buffer(unsigned char* data, size_t offset, unsigned char** value, size_t size)
{
    *value = data + offset;
    return size;
}

size_t write_buffer(unsigned char* data, size_t offset, unsigned char* value, size_t size)
{
    memcpy(data + offset, value, size);
    return size;
}
