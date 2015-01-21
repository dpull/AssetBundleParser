#ifndef assetbundle_h
#define assetbundle_h

struct assetbundle* assetbundle_load(char* filename);
bool assetbundle_check(struct assetbundle* bundle);
void assetbundle_destory(struct assetbundle* bundle);

struct assetbundle_diff* assetbundle_diff(struct assetbundle* src, struct assetbundle* dst);
void assetbundle_diff_destory(struct assetbundle_diff* diff);
struct assetbundle_diff* assetbundle_diff_load(char* filename);
bool assetbundle_diff_save(char* filename, struct assetbundle_diff* diff);


#endif