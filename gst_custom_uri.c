#include <gst/check/gstcheck.h>
#include <gst/base/gstpushsrc.h>

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
  return g_printf ("%s://", ASSET_URI_STR);
}

static gboolean
gst_assets_video_src_uri_set_uri (GstURIHandler * handler, const gchar * uri,
    GError ** error)
{
  return (uri != NULL && g_str_has_prefix (uri, ASSET_URI_STR));
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



int main(int argc, char const *argv[])
{
    /* code */
    return 0;
}
