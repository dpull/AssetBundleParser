//
//  main.c
//  AssetBundle
//
//  Created by dpull on 15/1/15.
//  Copyright (c) 2015年 dpull. All rights reserved.
//

#include <stdio.h>
#include <stdbool.h>
#include "assetbundle.h"
#include <assert.h>

int main(int argc, const char * argv[]) {
	bool ret;

    struct assetbundle* bundle = assetbundle_load("/Users/dpull/Documents/AssetBundle/AssetBundle/test.unity3d");
    assert(bundle);

    ret = assetbundle_check(bundle);
    assert(ret);

    struct assetbundle* bundle1 = assetbundle_load("/Users/dpull/Documents/AssetBundle/AssetBundle/test1.unity3d");
    assert(bundle1);

    ret = assetbundle_check(bundle1);
    assert(ret);

    struct assetbundle* bundle2 = assetbundle_load("/Users/dpull/Documents/AssetBundle/AssetBundle/test2.unity3d");
    assert(bundle2);

    ret = assetbundle_check(bundle2);
    assert(ret);

    struct assetbundle* bundle3 = assetbundle_load("/Users/dpull/Documents/AssetBundle/AssetBundle/test3.unity3d");
    assert(bundle3);

    ret = assetbundle_check(bundle3);
    assert(ret);    
/*
    printf("cmp test1, test2. change 1(txt file), add 4 files(1 prefab)\n");
	struct assetbundle_diff* diff1 = assetbundle_diff(bundle1, bundle2);
    assetbundle_diff_save("/Users/dpull/Documents/AssetBundle/AssetBundle/diff_1to2", diff1);
    assetbundle_diff_destory(diff1);

    printf("cmp test2, test3. change 1(prefab)\n");
    struct assetbundle_diff* diff2 = assetbundle_diff(bundle2, bundle3);
    assetbundle_diff_save("/Users/dpull/Documents/AssetBundle/AssetBundle/diff_2to3", diff2);
    assetbundle_diff_destory(diff2);

    struct assetbundle_diff* diff3 = assetbundle_diff_load("/Users/dpull/Documents/AssetBundle/AssetBundle/diff_1to2");
    struct assetbundle_diff* diff4 = assetbundle_diff_load("/Users/dpull/Documents/AssetBundle/AssetBundle/diff_2to3");
*/
    
    return 0;
}
