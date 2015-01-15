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
    // insert code here...
    if (assetbundle_load("/Users/dpull/Desktop/AssetBundle/AssetBundle/test.unity3d"))
    {
        printf("Hello, World!\n");
    }
    printf("Hello, World!\n");
    return 0;
}
