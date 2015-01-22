#ifndef ASSETBUNDLE_H
#define ASSETBUNDLE_H

struct assetbundle* assetbundle_load(const char* filename);
bool assetbundle_check(struct assetbundle* bundle);
void assetbundle_destory(struct assetbundle* bundle);
void assetbundle_print(struct assetbundle* bundle, struct debug_tree* root);
#endif