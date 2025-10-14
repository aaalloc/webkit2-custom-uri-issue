#include <gst/check/gstcheck.h>
#include <gst/base/gstpushsrc.h>
#include <gst/gst.h>

// https://gstreamer.freedesktop.org/documentation/tutorials/basic/dynamic-pipelines.html?gi-language=c

#define ASSET_URI_STR "assets"

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

  // convert "assets://file.mp4" → "file:///opt/app/assets/file.mp4"
  const gchar *filename = uri + strlen("assets://");
  gchar *real_uri = g_strdup_printf("file:///opt/app/assets/%s", filename);
  g_object_set(self, "location", real_uri, NULL);
  g_free(real_uri);
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
  GstBuffer *buf;
  GstMapInfo map;
  guint w = 64, h = 64;
  guint size;

  size = w * h * 3 / 2;
  buf = gst_buffer_new_and_alloc (size);
  gst_buffer_map (buf, &map, GST_MAP_WRITE);
  memset (map.data, 76, w * h);
  memset (map.data + (w * h), 85, (w * h) / 4);
  memset (map.data + (w * h) + ((w * h) / 4), 255, (w * h) / 4);
  gst_buffer_unmap (buf, &map);

  *p_buf = buf;
  return GST_FLOW_OK;
}

static GstCaps *
gst_assets_video_src_get_caps (GstBaseSrc * src, GstCaps * filter)
{
  guint w = 64, h = 64;
  return gst_caps_new_simple ("video/x-raw", "format", G_TYPE_STRING,
      "I420", "width", G_TYPE_INT, w, "height",
      G_TYPE_INT, h, "framerate", GST_TYPE_FRACTION, 1, 1, NULL);
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
  if (!gst_element_register (plugin, "redvideosrc", GST_RANK_PRIMARY,
          gst_assets_video_src_get_type ())) {
    return FALSE;
  }
  return TRUE;
}


#define PACKAGE "RedVideoSrc"

GST_PLUGIN_DEFINE
    (GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    redvideosrc,
    "Red Video Source",
    plugin_init,
    "1.0", "LGPL", "GStreamer", "http://gstreamer.net/")



int main(int argc, char *argv[]) {
    GstElement *pipeline;
    GError *error = NULL;

    gst_init(&argc, &argv);

    if (!gst_plugin_register_static(GST_VERSION_MAJOR, GST_VERSION_MINOR,
    "redvideosrc", "Red Video Source", plugin_init,
    "1.0", "LGPL", "GStreamer", "http://gstreamer.net/", "source"))
    {
        g_printerr("Failed to register redvideosrc plugin\n");
        return -1;
    }

    if (argc != 2) {
        g_printerr("Usage: %s assets://filename\n", argv[0]);
        return -1;
    }

    gchar *uri = argv[1];
    pipeline = gst_element_make_from_uri(GST_URI_SRC, uri, "source", &error);
    if (!pipeline) {
        g_printerr("Could not create source from URI: %s\n", error->message);
        g_clear_error(&error);
        return -1;
    }

    gchar *pipeline_desc = g_strdup_printf("uridecodebin uri=%s ! autovideosink", uri);
    pipeline = gst_parse_launch(pipeline_desc, &error);
    if (!pipeline) {
        g_printerr("Pipeline creation failed: %s\n", error->message);
        g_clear_error(&error);
        return -1;
    }
    g_free(pipeline_desc);

    gst_element_set_state(pipeline, GST_STATE_PLAYING);
    g_print("Playing %s\n", uri);

    GstBus *bus = gst_element_get_bus(pipeline);
    GstMessage *msg = gst_bus_timed_pop_filtered(bus, GST_CLOCK_TIME_NONE,
                                                 GST_MESSAGE_ERROR | GST_MESSAGE_EOS);

    if (msg != NULL)
        gst_message_unref(msg);

    gst_object_unref(bus);
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);

    return 0;
}