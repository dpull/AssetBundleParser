#ifndef ASSETBUNDLE_DIFF_IMP_H
#define ASSETBUNDLE_DIFF_IMP_H

#define MAX_PATH                 (512)
#define MAX_COMBINE_FILE_SIZE    (1024 * 1024 * 100)
#define MAX_SPLIT_FILE_COUNT    (100)
#define MAX_DIFF_RESERVED_SIZE  (1024 * 1024)
#define MAX_DIFF_INFO_SIZE      (1024)
#define DIFF_INFO_VERIFY        ("dpull@v1")

struct split_filemapping
{
    struct filemapping* filemapping;
    char* name;
    bool format;
    size_t offsets_count;
    size_t* offsets;
};

struct assetfiles
{
    size_t max_count;
    size_t count;
    
    struct assetfile** assetfiles;
    
    struct assetbundle* assetbundle;
    size_t assetbundle_assetfile_count;
    
    struct split_filemapping** split_filemappings;
    unsigned char** filemapping_base;
};

struct assetbundle_diff
{
    struct filemapping* filemapping;
    
    size_t assetbundle_size;
    struct assetbundle* assetbundle;
    
    size_t assetfile_index_count;
    char** assetfile_index;
    
    unsigned char* buffer_begin;
    unsigned char* buffer_end;
    unsigned char* buffer_pos;
};

#pragma pack(1)
struct objectinfo_diff_info
{
    char verify[sizeof(DIFF_INFO_VERIFY)];
    uint16_t index;
    uint32_t offset;
};
#pragma pack()

#endif