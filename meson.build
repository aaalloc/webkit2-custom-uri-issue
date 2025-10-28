project('gtk-browser', 'c')

executable_option = get_option('executable')

if executable_option == 'browse'
  gtkdep = dependency('gtk+-3.0')
  webgtkdep = dependency('webkit2gtk-4.1', method: 'pkg-config')
  executable('browse', 'browser.c', dependencies : [gtkdep, webgtkdep], install : true)
else
  gstreamerdep = dependency('gstreamer-1.0', method: 'pkg-config')
  gstreamer_base_dep = dependency('gstreamer-base-1.0', method: 'pkg-config')

  gst_assets_inc = include_directories('.')
  gst_assets = shared_library(
    'gstassettauri',
    'gstassettauri.c',
    include_directories: gst_assets_inc,
    dependencies: [gstreamerdep, gstreamer_base_dep],
    install: true
  )

  executable(
    'gst_custom_uri',
    'gst_custom_uri.c',
    dependencies: [gstreamerdep, gstreamer_base_dep],
    link_with: gst_assets,
    include_directories: gst_assets_inc,
    install: false
  )
endif


