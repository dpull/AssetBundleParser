//
//  main.c
//  AssetBundle
//
//  Created by dpull on 15/1/15.
//  Copyright (c) 2015年 dpull. All rights reserved.
//
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include "utils/platform.h"
#include "utils/debug_tree.h"
#include "assetbundle.h"
#include "assetbundle_diff.h"
#include "assetfile.h"
#include "filemapping.h"
#include "field_type.h"
#include "traversedir.h"
#include "assetbundle_diff.h"

int main(int argc, const char * argv[])
{
    int ret;
    
    ret = assetbundle_diff(
                           "/Users/dpull/Documents/AssetBundle/Untitled",
                           NULL,
                           "/Users/dpull/Documents/AssetBundle/Untitled.asset",
                           "/Users/dpull/Documents/AssetBundle/diff.asset");
    assert(ret == 0);
    
    assetbundle_diff_print("/Users/dpull/Documents/AssetBundle/diff.asset");
    
    
/*
    struct field_type_db* db = field_type_db_load("/Users/dpull/Documents/AssetBundle/types.dat");
    struct debug_tree* root = debug_tree_create(NULL, "*");
    field_type_db_print(db, root);
    // debug_tree_print(root, stdout);
    field_type_db_destory(db);

    struct file_asset* file_asset = (struct file_asset*)malloc(sizeof(*file_asset));
    memset(file_asset, 0, sizeof(*file_asset));
    
    traversedir("/Users/dpull/Documents/AssetBundle/Untitled", load_assetfile, file_asset, true);
    
    struct assetbundle* p1 = assetbundle_load("/Users/dpull/Documents/AssetBundle/Untitled.asset");
    assert(p1);

        
   
    root = debug_tree_create(NULL, "*");
    assetbundle_print(p1, root);

    FILE* debug1 = fopen("/Users/dpull/Documents/AssetBundle/debug1.txt", "w+");
    debug_tree_print(root, debug1);

    debug_tree_destory(root);


    root = debug_tree_create(NULL, "*");

    for (int i = 0; i < file_asset->count; ++i)
    {
        struct debug_tree* file = debug_tree_create(root, "%s", file_asset->names[i]);
        assetfile_print(file_asset->assets[i], file);
    }
    
    FILE* debug2 = fopen("/Users/dpull/Documents/AssetBundle/debug2.txt", "w+");
    debug_tree_print(root, debug2);
    debug_tree_destory(root);

    int ret;
 
    ret = assetbundle_diff(
        "D:\\OneDrive\\Code\\assetbundleparser\\Untitled1.asset",
        "D:\\OneDrive\\Code\\assetbundleparser\\Untitled2.asset",
        "D:\\OneDrive\\Code\\assetbundleparser\\diff.asset"
    );
    assert(ret == 0);
    
    ret = assetbundle_merge(
        "D:\\OneDrive\\Code\\assetbundleparser\\Untitled1.asset",
        "D:\\OneDrive\\Code\\assetbundleparser\\Untitled3.asset",
        "D:\\OneDrive\\Code\\assetbundleparser\\diff.asset"
    );
    assert(ret == 0);
*/
    
    return 0;
}
