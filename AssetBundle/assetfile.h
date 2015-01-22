#ifndef assetfile_h
#define assetfile_h

struct assetfile* assetfile_load(unsigned char* data, size_t start, size_t size);
bool assetfile_save(struct assetfile* file, unsigned char* data, size_t start, size_t size);
void assetfile_destory(struct assetfile* file);

/***********************************************************************************************
									diff and merge
***********************************************************************************************/


#endif