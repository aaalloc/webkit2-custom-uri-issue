#include <gst/check/gstcheck.h>
#include <gst/base/gstpushsrc.h>
#include <gst/gst.h>

// https://gstreamer.freedesktop.org/documentation/tutorials/basic/dynamic-pipelines.html?gi-language=c

#define ASSET_URI_STR "assets"

GstElement *pipeline, *sink;


static GstURIType
gst_assets_video_src_uri_get_type (GType type)
{
  return GST_URI_SRC;
}

static const gchar *const *
gst_assets_video_src_uri_get_protocols (GType type)
{
  static const gchar *protocols[] = { ASSET_URI_STR, NULL };

  return protocols;
}

static gchar *
gst_assets_video_src_uri_get_uri (GstURIHandler * handler)
{
  return g_strdup_printf ("%s://", ASSET_URI_STR);
}

// static gboolean
// gst_assets_video_src_uri_set_uri (GstURIHandler * handler, const gchar * uri,
//     GError ** error)
// {
//   return (uri != NULL && g_str_has_prefix (uri, ASSET_URI_STR));
// }

static gboolean
gst_assets_video_src_uri_set_uri (GstURIHandler * handler, const gchar * uri, GError ** error)
{
  GstElement *self = GST_ELEMENT(handler);

  if (!uri || !g_str_has_prefix(uri, "assets://"))
    return FALSE;

  return TRUE;
}


static void
gst_assets_video_src_uri_handler_init (gpointer g_iface, gpointer iface_data)
{
  GstURIHandlerInterface *iface = (GstURIHandlerInterface *) g_iface;

  iface->get_type = gst_assets_video_src_uri_get_type;
  iface->get_protocols = gst_assets_video_src_uri_get_protocols;
  iface->get_uri = gst_assets_video_src_uri_get_uri;
  iface->set_uri = gst_assets_video_src_uri_set_uri;
}

static void
gst_assets_video_src_init_type (GType type)
{
  static const GInterfaceInfo uri_hdlr_info = {
    gst_assets_video_src_uri_handler_init, NULL, NULL
  };

  g_type_add_interface_static (type, GST_TYPE_URI_HANDLER, &uri_hdlr_info);
}

typedef GstPushSrc GstRedVideoSrc;
typedef GstPushSrcClass GstRedVideoSrcClass;

G_DEFINE_TYPE_WITH_CODE (GstRedVideoSrc, gst_assets_video_src,
    GST_TYPE_PUSH_SRC, gst_assets_video_src_init_type (g_define_type_id));

static GstFlowReturn
gst_assets_video_src_create (GstPushSrc * src, GstBuffer ** p_buf)
{
  // TODO: goal here is just get buffer from pad
  return GST_FLOW_OK;
}

static GstCaps *
gst_assets_video_src_get_caps (GstBaseSrc * src, GstCaps * filter)
{
  return gst_caps_new_any();
}

static void
gst_assets_video_src_class_init (GstRedVideoSrcClass * klass)
{
  GstPushSrcClass *pushsrc_class = GST_PUSH_SRC_CLASS (klass);
  GstBaseSrcClass *basesrc_class = GST_BASE_SRC_CLASS (klass);
  static GstStaticPadTemplate src_templ = GST_STATIC_PAD_TEMPLATE ("src",
      GST_PAD_SRC, GST_PAD_ALWAYS,
      GST_STATIC_CAPS ("video/x-raw, format=(string)I420")
      );
  GstElementClass *element_class = GST_ELEMENT_CLASS (klass);

  gst_element_class_add_static_pad_template (element_class, &src_templ);
  gst_element_class_set_metadata (element_class,
      "Red Video Src", "Source/Video", "yep", "me");

  pushsrc_class->create = gst_assets_video_src_create;
  basesrc_class->get_caps = gst_assets_video_src_get_caps;
}

static void
gst_assets_video_src_init (GstRedVideoSrc * src)
{
}

static gboolean
plugin_init (GstPlugin * plugin)
{
  if (!gst_element_register (plugin, "assetsrc", GST_RANK_PRIMARY,
          gst_assets_video_src_get_type ())) {
    return FALSE;
  }
  return TRUE;
}


#define PACKAGE "CustomUriProxy"

GST_PLUGIN_DEFINE
    (GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    assetsrc,
    "Plugin proxy PoC",
    plugin_init,
    "1.0", "LGPL", "GStreamer", "http://gstreamer.net/")



static void
cb_pad_added (GstElement *dec,
          GstPad     *pad,
          gpointer    data)
{
  GstCaps *caps;
  GstStructure *str;
  const gchar *name;
  GstPadTemplate *templ;
  GstElementClass *klass;

  /* check media type */
  caps = gst_pad_query_caps (pad, NULL);
  str = gst_caps_get_structure (caps, 0);
  name = gst_structure_get_name (str);

  klass = GST_ELEMENT_GET_CLASS (sink);

  if (g_str_has_prefix (name, "audio")) {
    templ = gst_element_class_get_pad_template (klass, "audio_sink");
  } else if (g_str_has_prefix (name, "video")) {
    templ = gst_element_class_get_pad_template (klass, "video_sink");
  } else if (g_str_has_prefix (name, "text")) {
    templ = gst_element_class_get_pad_template (klass, "text_sink");
  } else {
    templ = NULL;
  }

  if (templ) {
    GstPad *sinkpad;

    sinkpad = gst_element_request_pad (sink, templ, NULL, NULL);

    if (!gst_pad_is_linked (sinkpad))
      gst_pad_link (pad, sinkpad);

    gst_object_unref (sinkpad);
  }

  gst_clear_caps (&caps);
}
static GMainLoop *loop;

static gboolean
my_bus_callback (GstBus * bus, GstMessage * message, gpointer data)
{
  // g_print ("Got %s message\n", GST_MESSAGE_TYPE_NAME (message));
  GError *err;
  gchar *debug;
  switch (GST_MESSAGE_TYPE (message)) {
    case GST_MESSAGE_ERROR:{
      gst_message_parse_error (message, &err, &debug);
      g_print ("Error: %s\n", err->message);
      g_error_free (err);
      g_free (debug);

      g_main_loop_quit (loop);
      break;
    }
    case GST_MESSAGE_WARNING:{
      gst_message_parse_warning (message, &err, &debug);
      g_print ("Warning: %s\n", err->message);
      g_error_free (err);
      g_free (debug);
      break;
    }
    case GST_MESSAGE_EOS:
      /* end-of-stream */
      g_main_loop_quit (loop);
      break;
    default:
      /* unhandled message */
      break;
  }

  /* we want to be notified again the next time there is a message
   * on the bus, so returning TRUE (FALSE means we want to stop watching
   * for messages on the bus and our callback should not be called again)
   */
  return TRUE;
}

gint
main (gint   argc,
      gchar *argv[])
{
  GstElement *dec;
  GstBus *bus;

  /* init GStreamer */
  gst_init (&argc, &argv);

  if (!gst_plugin_register_static(GST_VERSION_MAJOR, GST_VERSION_MINOR,
  "assetsrc", "Red Video Source", plugin_init,
  "1.0", "LGPL", "GStreamer", "http://gstreamer.net/", "source"))
  {
      g_printerr("Failed to register assetsrc plugin\n");
      return -1;
  }

  loop = g_main_loop_new (NULL, FALSE);

  /* make sure we have input */
  if (argc != 2) {
    g_print ("Usage: %s <uri>\n", argv[0]);
    return -1;
  }

  /* setup */
  pipeline = gst_pipeline_new ("pipeline");

  bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
  gst_bus_add_watch (bus, my_bus_callback, loop);
  gst_object_unref (bus);

  dec = gst_element_factory_make ("uridecodebin", "source");
  g_object_set (G_OBJECT (dec), "uri", argv[1], NULL);
  g_signal_connect (dec, "pad-added", G_CALLBACK (cb_pad_added), NULL);

  /* create audio output */
  sink = gst_element_factory_make ("playsink", "sink");
  gst_bin_add_many (GST_BIN (pipeline), dec, sink, NULL);

  /* run */
  gst_element_set_state (pipeline, GST_STATE_PLAYING);
  g_main_loop_run (loop);

  /* cleanup */
  gst_element_set_state (pipeline, GST_STATE_NULL);
  gst_object_unref (GST_OBJECT (pipeline));

  return 0;
}
