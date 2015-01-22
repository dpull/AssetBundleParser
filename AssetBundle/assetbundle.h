#ifndef ASSETBUNDLE_H
#define ASSETBUNDLE_H

struct assetbundle* assetbundle_load(const char* filename);
bool assetbundle_check(struct assetbundle* bundle);
void assetbundle_destory(struct assetbundle* bundle);

#endif