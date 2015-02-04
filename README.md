# License:  #
Released under MIT license.

It's forbidden to sell Unity AssetStore plugins based on this source. Because I'll write a plugin to sell. :)

# AssetBundleParser #
`AssetBundleParser` is a tool to compare and merge Unity assetbundle.

`AssetBundleParser` isn't robust. If it's used to load a file of wrong format, it'll crash. There are two reasons:

1. The format of assetbundle is private. The code is kept simple, so that if it fails to parse a file of right format and crashes, it'll be ease to compare the source code with `disunity` to find locate the bug.
1. It's easy to verify if a file is of right format by using md5, it's unnecessary to add too much error-check and recovery code which only complicates the source.

At current only uncompressed `assetbundle` is supported. This is because  Lzma compression speed is very slow. So when create a bundle, options like `BuildOptions.UncompressedAssetBundle` or `BuildAssetBundleOptions.UncompressedAssetBundle` shall be used.

It only supports mobile platforms. TypeTree related data isn't available on mobile, so it's not parsed. The support for TypeTree maybe added later for non-mobile platforms.

## Usage ##
1. Call `assetbundle_diff` to generate a diff file.
1. Call `assetbundle_merge` to merge diff file.

*The diff file isn't compressed. Compress it before sending to network if needed.*

## Interface ##
assetbundle_load
Description: Parse assetbundle file
Param: assetbundle: File path
Return: A pointer to assetbundle

assetbundle_check
Description: Check if an assetbundle is supported. It's used only when comparing difference. It may use much memory.
Return: true: The assetbundle is in good status.

assetbundle_diff
Description: Generate diff file
Return: 0: Success; None-zero: error code.

assetbundle_merge
Description: Merge diff files.
Return: 0: Success; None-zero: error code

# Sample to show how to create bundle. #

	BuildPipeline.BuildStreamedSceneAssetBundle(
		new string[]{"Assets/Client.unity", "Assets/Art/Maps/map001/map001.unity"}, 
		"Assets/scenes.unity3d", 
		BuildTarget.iPhone,
		BuildOptions.UncompressedAssetBundle);

	BuildPipeline.BuildAssetBundle(
		o1, 
		new Object[]{o2, o3, o4}, 
		"Assets/res.unity3d",
		BuildAssetBundleOptions.CollectDependencies | BuildAssetBundleOptions.CompleteAssets| BuildAssetBundleOptions.UncompressedAssetBundle | BuildAssetBundleOptions.DisableWriteTypeTree, 
		BuildTarget.iPhone);	

# Sample to show how to create and merge diff files. #
	
	// "scenes_v1.unity3d", "scenes_v2.unity3d" must exist. Create "v1to2.diff"
	assetbundle_diff("scenes_v1.unity3d", "scenes_v2.unity3d", "v1to2.diff"); 

	// "scenes_v1.unity3d", "v1to2.diff" must exist，create "scenes_v2.unity3d"
	// The order of arguments is a little weird. Any suggestions?
	assetbundle_merge("scenes_v1.unity3d", "scenes_v2.unity3d", "v1to2.diff");


# 开源协议 #
采用MIT开源许可证。
不想每个文件都写上，所以放在这儿了。

补充：

不允许出售基于此代码的Unity AssetStore插件，因为我准备写个插件卖 ：）

# AssetBundleParser #

`AssetBundleParser` 是 Unity assetbundle文件的差异比较及合并工具。

`AssetBundleParser` 的代码并不健壮，如果拿错误格式的文件传入会造成程序崩溃，原因有二：

1. assetbundle的格式是非公开的，保持代码简单，可以方便和 `disunity` 对照代码查找问题，正确格式解析有问题可以快速暴露。
1. 文件的正确性可以通过验证md5等方式来保证， 没必要太增加代码复杂度. 合并差异前，会检查文件diff文件，旧assetbundle文件的md5是否一致，合并后也会检查新assetbundle文件的md5是否一致

目前只支持非压缩的 `assetbundle` 的包，因为Lzma压缩速度很慢，差异合并后再压缩整个过程就太久了，所以做包时需要增加：`BuildOptions.UncompressedAssetBundle` 或 `BuildAssetBundleOptions.UncompressedAssetBundle`。

目前只支持移动端，因为TypeTree相关的数据在移动端上没有，故而也并没有解析。对于非移动端可以考虑加上 `BuildAssetBundleOptions.DisableWriteTypeTree` ，这对我当前的项目支持是足够的， 在下一个版本可能会加上这个支持。

## 使用流程 ##
1. 调用 `assetbundle_diff` 生成差异文件
1. 调用 `assetbundle_merge` 合并差异文件

*差异并没有经过压缩，请在网路传输前进行压缩。*

## 接口说明 ##
assetbundle_load 
描述 解析assetbundle文件
参数 assetbundle文件路径
返回 assetbundle指针

assetbundle_check
描述 检查assetbundle文件是否支持，会申请大量内存，只用于差异比较时检查。
返回 true:二进制检查成功

assetbundle_diff
描述 生成差异文件
返回 0 成功，非0各种错误码

assetbundle_merge
描述 合并差异文件
返回 0 成功，非0各种错误码

# 打包代码示例 #

	BuildPipeline.BuildStreamedSceneAssetBundle(
		new string[]{"Assets/Client.unity", "Assets/Art/Maps/map001/map001.unity"}, 
		"Assets/scenes.unity3d", 
		BuildTarget.iPhone,
		BuildOptions.UncompressedAssetBundle);

	BuildPipeline.BuildAssetBundle(
		o1, 
		new Object[]{o2, o3, o4}, 
		"Assets/res.unity3d",
		BuildAssetBundleOptions.CollectDependencies | BuildAssetBundleOptions.CompleteAssets| BuildAssetBundleOptions.UncompressedAssetBundle | BuildAssetBundleOptions.DisableWriteTypeTree, 
		BuildTarget.iPhone);	

# 生成差异及合并代码示例 #
	
	// "scenes_v1.unity3d", "scenes_v2.unity3d" 必须存在，生成"v1to2.diff"
	assetbundle_diff("scenes_v1.unity3d", "scenes_v2.unity3d", "v1to2.diff"); 

	// "scenes_v1.unity3d", "v1to2.diff" 必须存在，生成 "scenes_v2.unity3d"
	// 该接口参数顺序有点诡异，大家是否有建议？
	assetbundle_merge("scenes_v1.unity3d", "scenes_v2.unity3d", "v1to2.diff");


 