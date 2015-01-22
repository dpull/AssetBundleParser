#ifndef assetbundle_h
#define assetbundle_h

struct assetbundle* assetbundle_load(const char* filename);
bool assetbundle_check(struct assetbundle* bundle);
void assetbundle_destory(struct assetbundle* bundle);

#endif