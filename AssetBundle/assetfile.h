#ifndef ASSETFILE_H
#define ASSETFILE_H

API_EXTERN struct assetfile* assetfile_load(unsigned char* data, size_t start, size_t size);
API_EXTERN bool assetfile_save(struct assetfile* file, unsigned char* data, size_t start, size_t size);
API_EXTERN void assetfile_destory(struct assetfile* file);
API_EXTERN void assetfile_print(struct assetfile* file, struct debug_tree* root);

#endif
