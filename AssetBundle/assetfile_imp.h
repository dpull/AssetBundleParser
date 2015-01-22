#ifndef assetfile_imp_h
#define assetfile_imp_h

#define assetheader_reserved_size 3
struct assetheader
{
	size_t metadata_size;
	size_t file_size;
	int version_info;
	size_t data_offset;
	unsigned char endianness;
	unsigned char* reserved;
};

size_t assetheader_load(struct assetheader* header, unsigned char* data, size_t offset);
size_t assetheader_save(struct assetheader* header, unsigned char* data, size_t offset);

#define assetfile_meta_align 4096
#define assetfile_align 16
struct assetfile
{
    struct assetheader header;
    char* unity_revision;
    int typetree_struct_attribute;
    size_t typetree_struct_count; // must == 0
    int typetree_padding;
    size_t objectinfo_struct_count;
    struct objectinfo* objectinfo_struct;
    size_t externals_struct_count;
    struct fileidentifier* externals_struct;
    size_t align_data_length;
    unsigned char* align_data;
};

#define objectinfo_buffer_align 8
struct objectinfo
{
	int path_id;
	size_t offset;
	size_t length;
	int type_id;
	short class_id;
	short is_destroyed;
    
	unsigned char* buffer;
    size_t align_data_length;
    unsigned char* align_data;
};

#define guid_size 16
struct fileidentifier
{		
	char* asset_path;
	unsigned char* guid;
	char* file_path;
	int type;
};

size_t assetmeta_load(struct assetfile* file, unsigned char* data, size_t offset, size_t file_offset);
size_t assetmeta_save(struct assetfile* file, unsigned char* data, size_t offset);

#endif