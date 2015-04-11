#ifndef assetfile_imp_h
#define assetfile_imp_h

#define ASSETHEADER_RESERVED_SIZE    (3)
struct assetheader
{
	size_t metadata_size;
	size_t file_size;
	int version_info;
	size_t data_offset;
	unsigned char endianness;
	unsigned char* reserved;
};

#define ASSETFILE_META_ALIGN    (4096)
#define ASSETFILE_ALIGN            (16)
struct assetfile
{
	struct assetheader header;
	char* unity_revision;
	int typetree_struct_attribute;
	size_t typetree_struct_count; // must == 0
    struct typetree* typetree_struct;
	int typetree_padding;
	size_t objectinfo_struct_count;
	struct objectinfo* objectinfo_struct;
	size_t externals_struct_count;
	struct fileidentifier* externals_struct;
	size_t align_data_length;
	unsigned char* align_data;
};

struct field_type
{
    char* type;
    char* name;
    int size;
    int index;
    int is_array;
    int version;
    int meta_flag;
};

struct field_type_list
{
    struct field_type field_type;
    int children_count;
    struct field_type_list* children_field_type_list;
};

struct typetree
{
    int class_id;
    struct field_type_list field_type_list;
};


#define OBJECTINFO_BUFFER_ALIGN        (8)
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

#define GUID_SIZE    (16)
struct fileidentifier
{
	char* asset_path;
	unsigned char* guid;
	char* file_path;
	int type;
};

size_t assetfile_objectinfo_count(struct assetfile* assetfile);
struct objectinfo* assetfile_get_objectinfo(struct assetfile* assetfile, size_t index);

#endif