# AssetBundle #

`AssetBundle` 是 Unity3d assetbundle文件的差异比较及合并工具。

`assetbundle_load` 的代码并不健壮，如果拿错误格式的文件传入会造成程序崩溃，因为assetbundle的格式是非公开的，当该程序解析出错时，我希望其能方便调试，另外代码简单，方便对照 `disunity` 的代码来查找问题。
差异文件中会包含新旧assetbundle文件的md5，只有在文件校验成功时，才会调用 `assetbundle_load` ，所以无需担心在差异合并时会造成客户端闪退的情况，但是在生成差异工具是会存在崩溃的问题，当程序崩溃或 `assetbundle_check` 失败时，说明该工具不支持了。

目前只支持非压缩的 `assetbundle` 的包，因为既然选择了这种更新方式，压缩没有意义，反而会降低保证客户端的读取效率，所以做包时需要增加：`BuildOptions.UncompressedAssetBundle` 或 `BuildAssetBundleOptions.UncompressedAssetBundle`。

目前只支持移动端，因为TypeTree相关的数据在移动端上没有，故而也并没有解析。对于非移动端可以考虑加上 `BuildAssetBundleOptions.DisableWriteTypeTree` ，这对我当前的项目支持是足够的， 在下一个版本可能会加上这个支持。

## 使用流程 ##
### 生成差异：###

1. 调用assetbundle_load分析包
1. 调用assetbundle_check检查支持
1. 调用assetbundle_diff生成差异文件

差异并没有经过压缩，请在传输

## 接口说明 ##
assetbundle_load 
描述 解析assetbundle文件
参数 assetbundle文件路径
返回 assetbundle指针

assetbundle_check
描述 检查assetbundle文件是否支持，会申请大量内存，只用于差异比较时检查。
返回 true:二进制检查成功

#打包代码示例#

	BuildPipeline.BuildStreamedSceneAssetBundle(
		new string[]{"Assets/Client.unity", "Assets/Art/Maps/map001/map001.unity"}, 
		"Assets/test1.unity3d", 
		BuildTarget.iPhone,
		BuildOptions.UncompressedAssetBundle);

	Object oo1 = AssetDatabase.LoadMainAssetAtPath("Assets/UI/Panels/Action.prefab");
	Object oo2 = AssetDatabase.LoadMainAssetAtPath("Assets/UI/Panels/Bag.prefab");
	Object oo3 = AssetDatabase.LoadMainAssetAtPath("Assets/UI/Panels/Fight.prefab");
	Object oo4 = AssetDatabase.LoadMainAssetAtPath("Assets/UI/Panels/Fight.prefab");
	BuildPipeline.BuildAssetBundle(
		oo1, 
		new Object[]{oo2, oo3, oo4}, 
		"Assets/test2.unity3d",
		BuildAssetBundleOptions.CollectDependencies | BuildAssetBundleOptions.CompleteAssets| BuildAssetBundleOptions.UncompressedAssetBundle | BuildAssetBundleOptions.DisableWriteTypeTree, 
		BuildTarget.iPhone);	

 