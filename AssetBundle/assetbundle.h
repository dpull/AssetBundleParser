#ifndef ASSETBUNDLE_H
#define ASSETBUNDLE_H

EXTERN_API struct assetbundle* assetbundle_load(const char* filename);
EXTERN_API bool assetbundle_check(struct assetbundle* bundle);
EXTERN_API void assetbundle_destory(struct assetbundle* bundle);
EXTERN_API void assetbundle_print(struct assetbundle* bundle, struct debug_tree* root);

EXTERN_API size_t assetbundle_assetfile_count(struct assetbundle* bundle);
EXTERN_API struct assetfile*  assetbundle_get_assetfile(struct assetbundle* bundle, size_t index);
EXTERN_API struct assetbundle* assetbundle_load_data(unsigned char* data, size_t length);

#endif
