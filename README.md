Needs webkit2 and gtk header

```
# PKG_CONFIG_PATH=../WebKit/build/Source/WebKit:$PKG_CONFIG_PATH: meson setup build
$ meson build
$ cd build
$ ninja
$ GST_DEBUG=2 ./browse ../1900.mp3
```
If whitescreen
```
GST_DEBUG=2 WEBKIT_DISABLE_COMPOSITING_MODE=1 ./browse ../1900.mp3 
```
