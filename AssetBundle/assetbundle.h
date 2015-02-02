#ifndef ASSETBUNDLE_H
#define ASSETBUNDLE_H

EXTERN_API struct assetbundle* assetbundle_load(const char* filename);
EXTERN_API bool assetbundle_check(struct assetbundle* bundle);
EXTERN_API void assetbundle_destory(struct assetbundle* bundle);
EXTERN_API void assetbundle_print(struct assetbundle* bundle, struct debug_tree* root);
#endif