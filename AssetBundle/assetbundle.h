#ifndef assetbundle_h
#define assetbundle_h

struct assetbundle* assetbundle_load(const char* filename);
bool assetbundle_check(struct assetbundle* bundle);
void assetbundle_destory(struct assetbundle* bundle);

int assetbundle_diff(const char* assetbundle_from, const char* assetbundle_to, const char* assetbundle_diff);
int assetbundle_merge(const char* assetbundle_from, const char* assetbundle_to, const char* assetbundle_diff);

#endif