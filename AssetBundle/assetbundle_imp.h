#ifndef assetbundle_imp_h
#define assetbundle_imp_h

// for assetbundle.c and assetbundle_diff.c

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

size_t assetbundle_header_load(struct assetbundle_header* header, unsigned char* data, size_t offset);
size_t assetbundle_header_save(struct assetbundle_header* header, unsigned char* data, size_t offset);

struct assetbundle_entryinfo
{
	char* name;
	size_t offset;
	size_t size;
	struct assetfile* assetfile;
};

#define ASSETBUNDLE_ENTRYINFO_ALIGN		(4)
struct assetbundle
{
	struct assetbundle_header header;
	size_t entryinfo_count;
	struct assetbundle_entryinfo* entryinfo;
    size_t align_data_length;
    unsigned char* align_data;

	struct filemaping* filemaping;
};

struct assetbundle* assetbundle_load_data(unsigned char* data, size_t length);

// will delete!!!
struct assetbundle* assetbundle_create();
struct assetbundle* assetbundle_load_filemaping(struct filemaping* filemaping);

#endif