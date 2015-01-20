
struct assetfile* assetfile_load(unsigned char* data, size_t offset, size_t size);
bool assetfile_save(struct assetfile* file, unsigned char* data, size_t offset, size_t size);
void assetfile_destory(struct assetfile* file);

struct assetfile_diff* assetfile_diff(struct assetfile** src_files, size_t src_files_count, struct assetfile* dst_file);

void assetfile_diff_destory(struct assetfile_diff* diff);