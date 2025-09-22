Needs webkit2 and gtk header

```
# PKG_CONFIG_PATH=/home/yanovskyy/Documents/webkit/WebKit/build/Source/WebKit:$PKG_CONFIG_PATH meson setup build
$ meson compile -C build 
$ GST_DEBUG=2 WEBKIT_DISABLE_DMABUF_RENDERER=1 ./build/browse /home/yanovskyy/Documents/projects/gtkbrowser/1900.mp3
```
If whitescreen
```
GST_DEBUG=2 WEBKIT_DISABLE_COMPOSITING_MODE=1 ./browse ../1900.mp3 
```
