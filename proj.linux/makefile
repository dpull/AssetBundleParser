MAKE = make -f $@ debug_x64 debug_x86

# make platform's all projects
all:  template
	rm -rf x86
	rm -rf x86_64
	mkdir x86
	mkdir x86_64
	mv bin/libAssetBundleParser_x86d.so  x86/libAssetBundleParser.so 
	mv bin/libAssetBundleParser_x64d.so  x86_64/libAssetBundleParser.so 
.PHONY: all

clean:
	rm -rf build
	rm -rf bin
	rm -rf lib
.PHONY: clean

template:
	$(MAKE)
.PHONY: template
