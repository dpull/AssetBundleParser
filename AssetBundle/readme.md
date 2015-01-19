# AssetBundle #

AssetBundle是Unity3d assetbundle文件的差异比较及合并工具。
该程序代码并不健壮，如果拿错误格式的文件传入会造成程序崩溃，因为assetbundle的格式是非公开的，当该程序解析出错时，我希望其能明确告知。

目前只支持非压缩的AssetBundle的包，因为既然选择了这种更新方式，压缩没有意义，反而会降低保证客户端的读取效率，所以做包时需要增加：`BuildOptions.UncompressedAssetBundle` 。

目前只支持移动端，因为TypeTree相关的数据在移动端上没有，故而也并没有解析。即目前只测试过支持 BuildTarget.iPhone 和 BuildTarget.Android，这对我当前的项目支持是足够的。


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


 