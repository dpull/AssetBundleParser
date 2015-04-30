#ifndef ASSETBUNDLE_H
#define ASSETBUNDLE_H

API_EXTERN struct assetbundle* assetbundle_load(const char* filename);
API_EXTERN bool assetbundle_check(struct assetbundle* bundle);
API_EXTERN void assetbundle_destroy(struct assetbundle* bundle);
API_EXTERN void assetbundle_print(struct assetbundle* bundle, struct debug_tree* root);

API_EXTERN size_t assetbundle_assetfile_count(struct assetbundle* bundle);
API_EXTERN struct assetfile*  assetbundle_get_assetfile(struct assetbundle* bundle, size_t index);
API_EXTERN struct assetbundle* assetbundle_load_data(unsigned char* data, size_t length);

#endif
