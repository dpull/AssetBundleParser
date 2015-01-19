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

	struct assetbundle_diff* diff = assetbundle_diff(bundle1, bundle2);
    assert(diff);
    
    return 0;
}
