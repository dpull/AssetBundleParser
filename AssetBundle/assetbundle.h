
struct assetbundle* assetbundle_load(char* file);
bool assetbundle_check(struct assetbundle* bundle);
void assetbundle_destory(struct assetbundle* bundle);

struct assetbundle_diff* assetbundle_diff(struct assetbundle* src, struct assetbundle* dst);
void assetbundle_diff_destory(struct assetbundle_diff* diff);
