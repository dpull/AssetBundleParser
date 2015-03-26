#ifndef TOOLS_H
#define TOOLS_H

size_t read_buffer(unsigned char* data, size_t offset, unsigned char** value, size_t size);
size_t write_buffer(unsigned char* data, size_t offset, unsigned char* value, size_t size);

size_t read_string(unsigned char* data, size_t offset, char** value);
size_t write_string(unsigned char* data, size_t offset, char* value);

size_t read_int16(unsigned char* data, size_t offset, short* value, bool littleEndian);
size_t write_int16(unsigned char* data, size_t offset, short value, bool littleEndian);

size_t read_uint16(unsigned char* data, size_t offset, unsigned short* value, bool littleEndian);
size_t write_uint16(unsigned char* data, size_t offset, unsigned short value, bool littleEndian);

size_t read_int32(unsigned char* data, size_t offset, int* value, bool littleEndian);
size_t write_int32(unsigned char* data, size_t offset, int value, bool littleEndian);

size_t read_uint32(unsigned char* data, size_t offset, size_t* value, bool littleEndian);
size_t write_uint32(unsigned char* data, size_t offset, size_t value, bool littleEndian);

size_t read_byte(unsigned char* data, size_t offset, unsigned char* value);
size_t write_byte(unsigned char* data, size_t offset, unsigned char value);

void check_write_overlapping_zero_buffer(unsigned char* data, size_t offset, size_t size);
#endif





