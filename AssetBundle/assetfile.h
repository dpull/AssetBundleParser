#ifndef ASSETFILE_H
#define ASSETFILE_H

EXTERN_API struct assetfile* assetfile_load(unsigned char* data, size_t start, size_t size);
EXTERN_API bool assetfile_save(struct assetfile* file, unsigned char* data, size_t start, size_t size);
EXTERN_API void assetfile_destory(struct assetfile* file);
EXTERN_API void assetfile_print(struct assetfile* file, struct debug_tree* root);

#endif
