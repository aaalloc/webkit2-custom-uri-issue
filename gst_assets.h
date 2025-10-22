#pragma once

#include <gst/check/gstcheck.h>
#include <gst/base/gstpushsrc.h>
#include <gst/gst.h>

GType gst_assets_get_type(void);

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



static GstURIType
gst_assets_uri_get_type (GType type)
{
  return GST_URI_SRC;
}

static const gchar *const *
gst_assets_uri_get_protocols (GType type)
{
  static const gchar *protocols[] = { ASSET_URI_STR, NULL };

  return protocols;
}

static gchar *
gst_assets_uri_get_uri (GstURIHandler * handler)
{
  return g_strdup_printf ("%s://", ASSET_URI_STR);
}


static gboolean
gst_assets_uri_set_uri (GstURIHandler * handler, const gchar * uri, GError ** error)
{
  GstElement *self = GST_ELEMENT(handler);

  if (!uri || !g_str_has_prefix(uri, "assets://"))
    return FALSE;

  return TRUE;
}

static void
gst_assets_uri_handler_init (gpointer g_iface, gpointer iface_data)
{
  GstURIHandlerInterface *iface = (GstURIHandlerInterface *) g_iface;

  iface->get_type = gst_assets_uri_get_type;
  iface->get_protocols = gst_assets_uri_get_protocols;
  iface->get_uri = gst_assets_uri_get_uri;
  iface->set_uri = gst_assets_uri_set_uri;
}


static void
gst_assets_init(GstAssets * self)
{
  // gst_assets_src_init_type (GST_TYPE_ASSETS);
}

static void
gst_assets_class_init (GstAssetsClass * self)
{
  GstElementClass *element_class = GST_ELEMENT_CLASS (self);
  GObjectClass *gobject_class = G_OBJECT_CLASS (self);

  gobject_class->set_property = gst_assets_set_property;
  gobject_class->get_property = gst_assets_get_property;

  g_object_class_install_property (gobject_class, 1,
    g_param_spec_pointer ("main-loop",
                          "Main Loop",
                          "External or internal GMainLoop used by the plugin",
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));


  g_object_class_install_property (gobject_class, 2,
    g_param_spec_pointer ("main-pipeline",
                          "Main Pipeline",
                          "External or internal GstPipeline used by the plugin",
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));


  gst_element_class_set_static_metadata (element_class,
    "Assets plugins proxy Poc",
    "Example/FirstExample",
    "Goal to proxy custom URI",
    "Alexander Yanovskyy");

}


GType 
gst_assets_get_type(void) 
{
  static GType assets_type = 0;

  if (!assets_type) {
    static const GTypeInfo assets_info = {
      sizeof (GstAssetsClass),
      NULL,   /* base_init */
      NULL,   /* base_finalize */
      (GClassInitFunc) gst_assets_class_init,
      NULL,   /* class_finalize */
      NULL,   /* class_data */
      sizeof (GstAssets),
      0,      /* n_preallocs */
      (GInstanceInitFunc) gst_assets_init,
      NULL
    };

    static const GInterfaceInfo uri_hdlr_info = {
      gst_assets_uri_handler_init, NULL, NULL
    };

    assets_type = g_type_register_static (GST_TYPE_ELEMENT,
        "GstAssets", &assets_info, 0);
    g_type_add_interface_static (assets_type, GST_TYPE_URI_HANDLER,
        &uri_hdlr_info);  

  }

  return assets_type;
}