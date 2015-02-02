LibName=ios

xcodebuild -sdk iphoneos
xcodebuild -sdk iphonesimulator
lipo -create -output lib${LibName}.a build/Release-iphoneos/lib${LibName}.a build/Release-iphonesimulator/lib${LibName}.a