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

int main(int argc, const char * argv[]) {
    struct assetbundle* bundle = assetbundle_load("/Users/dpull/Documents/AssetBundle/AssetBundle/test.unity3d");
    if (bundle) {
    	assetbundle_save(bundle, "/Users/dpull/Documents/AssetBundle/AssetBundle/test1.unity3d");
    }
    return 0;
}
