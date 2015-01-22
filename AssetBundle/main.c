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
#include "assetbundle_diff.h"
#include <assert.h>
#include "debug_tree.h"

int main(int argc, const char * argv[]) {
    struct debug_tree* root = debug_tree_create(NULL, "*");
    struct debug_tree* node1 = debug_tree_create(root, "%s", "node1");
    struct debug_tree* node2 = debug_tree_create(root, "%s", "node2");
    struct debug_tree* node1_1 = debug_tree_create(node1, "%s", "node1_1");
    struct debug_tree* node1_2 = debug_tree_create(node1, "%s", "node1_2");
    debug_tree_print(root);
    
	int ret;

    struct assetbundle* bundle = assetbundle_load("/Users/dpull/Documents/AssetBundle/AssetBundle/test.unity3d");
    assert(bundle);

    ret = (int)assetbundle_check(bundle);
    assert(ret);

    ret = assetbundle_diff(
        "/Users/dpull/Documents/AssetBundle/AssetBundle/test1.unity3d",
        "/Users/dpull/Documents/AssetBundle/AssetBundle/test2.unity3d",
        "/Users/dpull/Documents/AssetBundle/AssetBundle/diff_1to2"
    );
    assert(ret == 0);
    
    ret = assetbundle_merge(
        "/Users/dpull/Documents/AssetBundle/AssetBundle/test1.unity3d",
        "/Users/dpull/Documents/AssetBundle/AssetBundle/test2_1.unity3d",
        "/Users/dpull/Documents/AssetBundle/AssetBundle/diff_1to2"
    );
    assert(ret == 0);
    
    ret = assetbundle_diff(
        "/Users/dpull/Documents/AssetBundle/AssetBundle/test2.unity3d",
        "/Users/dpull/Documents/AssetBundle/AssetBundle/test3.unity3d",
        "/Users/dpull/Documents/AssetBundle/AssetBundle/diff_2to3"
    );
    assert(ret == 0);
    
    ret = assetbundle_merge(
        "/Users/dpull/Documents/AssetBundle/AssetBundle/test2.unity3d",
        "/Users/dpull/Documents/AssetBundle/AssetBundle/test3_1.unity3d",
        "/Users/dpull/Documents/AssetBundle/AssetBundle/diff_2to3"
    );
    assert(ret == 0);
    return 0;
}
