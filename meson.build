project('gtk-browser', 'c')

# Option pour choisir quel exécutable compiler
executable_option = get_option('executable')

# Dépendances communes
gtkdep = dependency('gtk+-3.0')
webgtkdep = dependency('webkit2gtk-4.1', method: 'pkg-config')
gstreamerdep = dependency('gstreamer-1.0', method: 'pkg-config')

# Compiler l'exécutable approprié selon l'option
if executable_option == 'browse'
  executable('browse', 'browser.c', dependencies : [gtkdep, webgtkdep], install : true)
elif executable_option == 'gst_custom_uri'
  executable('gst_custom_uri', 'gst_custom_uri.c', dependencies : [gstreamerdep], install : true)
elif executable_option == 'both'
  executable('browse', 'browser.c', dependencies : [gtkdep, webgtkdep], install : true)
  executable('gst_custom_uri', 'gst_custom_uri.c', dependencies : [gstreamerdep], install : true)
endif

