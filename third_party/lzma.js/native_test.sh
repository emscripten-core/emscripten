./lzma-native < lzma-native > lzma-native.lz
./lzma-native -d < lzma-native.lz > lzma-native.post
ls -al lzma-native*
diff lzma-native lzma-native.post

