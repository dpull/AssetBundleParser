# License:  #
Released under MIT license.

It's forbidden to sell Unity AssetStore plugins based on this source. Because I'll write a plugin to sell. :)

# AssetBundleParser #
`AssetBundleParser` is a tool to compare and merge Unity assetbundle.

`AssetBundleParser` isn't robust. If it's used to load a file of wrong format, it'll crash. There are two reasons:

1. The format of assetbundle is private. The code is kept simple, so that if it fails to parse a file of right format and crashes, it'll be ease to compare the source code with `disunity` to find locate the bug.
1. It's easy to verify if a file is of right format by using md5, it's unnecessary to add too much error-check and recovery code which only complicates the source.

At current only uncompressed `assetbundle` is supported. This is because  Lzma compression speed is very slow. So when create a bundle, options like `BuildOptions.UncompressedAssetBundle` or `BuildAssetBundleOptions.UncompressedAssetBundle` shall be used.

# Compatibility #

	Unity 				| Support
	---  				| --- 		
	4.6 				| √ 
	5.0 				| ×		

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