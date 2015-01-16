
struct assetfile* assetfile_load(unsigned char* data, size_t offset, size_t size);
bool assetfile_save(struct assetfile* file, unsigned char* data, size_t offset, size_t size);
void assetfile_destory(struct assetfile* file);


