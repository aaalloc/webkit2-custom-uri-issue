#pragma once

#include <gst/check/gstcheck.h>
#include <gst/base/gstpushsrc.h>
#include <gst/gst.h>


#define GST_TYPE_ASSETS (gst_assets_get_type())
#define GST_ASSETS(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_ASSETS,GstAssets))
#define GST_ASSETS_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_ASSETS,GstAssetsClass))
#define GST_IS_ASSETS(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_ASSETS))
#define GST_IS_ASSETS_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_ASSETS))


typedef struct _GstAssets
{
    GstElement parent_instance;
    GstElement *pipeline, *sink;
    GstElement *dec;
    GstPad *sinkpad;
    GstBus *bus;
    GMainLoop *loop;
} GstAssets;

typedef struct _GstAssetsClass
{
  GstElementClass parent_class;
} GstAssetsClass;


GType gst_assets_get_type (void);

GST_ELEMENT_REGISTER_DECLARE(assets)

#define ASSET_URI_STR "assets"

static void
gst_assets_set_property (GObject *object,
                         guint prop_id,
                         const GValue *value,
                         GParamSpec *pspec)
{
  GstAssets *self = GST_ASSETS (object);

  switch (prop_id) {
    case 1: /* main-loop */
        self->loop = g_value_get_pointer (value);
        break;
    case 2:
        /* main-pipeline */
        self->pipeline = g_value_get_pointer (value);
        break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

/* Called when someone uses g_object_get() */
static void
gst_assets_get_property (GObject *object,
                         guint prop_id,
                         GValue *value,
                         GParamSpec *pspec)
{
  GstAssets *self = GST_ASSETS (object);

  switch (prop_id) {
    case 1:
        g_value_set_pointer (value, self->loop);
        break;
    case 2:
        g_value_set_pointer(value, self->pipeline);
        break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}



// static GstURIType
// gst_assets_video_src_uri_get_type (GType type)
// {
//   return GST_URI_SRC;
// }

// static const gchar *const *
// gst_assets_video_src_uri_get_protocols (GType type)
// {
//   static const gchar *protocols[] = { ASSET_URI_STR, NULL };

//   return protocols;
// }

// static gchar *
// gst_assets_video_src_uri_get_uri (GstURIHandler * handler)
// {
//   return g_strdup_printf ("%s://", ASSET_URI_STR);
// }
