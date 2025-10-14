project('gtk-browser', 'c')

executable_option = get_option('executable')

if executable_option == 'browse'
  gtkdep = dependency('gtk+-3.0')
  webgtkdep = dependency('webkit2gtk-4.1', method: 'pkg-config')
  executable('browse', 'browser.c', dependencies : [gtkdep, webgtkdep], install : true)
else
  gstreamerdep = dependency('gstreamer-1.0', method: 'pkg-config')
  gstreamer_base_dep = dependency('gstreamer-base-1.0', method: 'pkg-config')
  executable('gst_custom_uri', 'gst_custom_uri.c', dependencies : [gstreamerdep, gstreamer_base_dep], install : true)
endif

