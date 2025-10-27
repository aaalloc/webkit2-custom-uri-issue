#pragma once

#include <gst/base/gstpushsrc.h>
#include <gst/check/gstcheck.h>
#include <gst/gst.h>

GType gst_assets_get_type(void);

#define GST_TYPE_ASSETS (gst_assets_get_type())
#define GST_ASSETS(obj)                                                        \
    (G_TYPE_CHECK_INSTANCE_CAST((obj), GST_TYPE_ASSETS, GstAssets))
#define GST_ASSETS_CLASS(klass)                                                \
    (G_TYPE_CHECK_CLASS_CAST((klass), GST_TYPE_ASSETS, GstAssetsClass))
#define GST_IS_ASSETS(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), GST_TYPE_ASSETS))
#define GST_IS_ASSETS_CLASS(klass)                                             \
    (G_TYPE_CHECK_CLASS_TYPE((klass), GST_TYPE_ASSETS))

typedef struct _GstAssets
{
    GstBin parent_instance;
    GstElement *internal_src;
} GstAssets;

typedef struct _GstAssetsClass
{
    GstBinClass parent_class;
} GstAssetsClass;

GST_ELEMENT_REGISTER_DECLARE(assets)

#define ASSET_URI_STR "assets"

// static void gst_assets_set_property(GObject *object, guint prop_id,
//                                     const GValue *value, GParamSpec *pspec)
// {
//     GstAssets *self = GST_ASSETS(object);
//     switch (prop_id)
//     {
//     case 1:
//         self->url_str = g_value_get_pointer(value);
//         break;
//     default:
//         G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
//         break;
//     }
// }

// static void gst_assets_get_property(GObject *object, guint prop_id,
//                                     GValue *value, GParamSpec *pspec)
// {
//     GstAssets *self = GST_ASSETS(object);

//     switch (prop_id)
//     {
//     case 1:
//         g_value_set_pointer(value, self->url_str);
//         break;
//     default:
//         G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
//         break;
//     }
// }

static GstURIType gst_assets_uri_get_type(GType type) { return GST_URI_SRC; }

static const gchar *const *gst_assets_uri_get_protocols(GType type)
{
    static const gchar *protocols[] = {ASSET_URI_STR, NULL};

    return protocols;
}

static gchar *gst_assets_uri_get_uri(GstURIHandler *handler)
{
    return g_strdup_printf("%s://", ASSET_URI_STR);
}

#include <stdio.h>

static gboolean gst_assets_uri_set_uri(GstURIHandler *handler, const gchar *uri,
                                       GError **error)
{
    GstAssets *self = GST_ASSETS(handler);

    gchar *path = gst_uri_get_location(uri);
    if (!path)
    {
        g_set_error(error, GST_URI_ERROR, GST_URI_ERROR_BAD_URI,
                    "Could not get location from URI: %s", uri);
        return FALSE;
    }
    else if (!g_file_test(path, G_FILE_TEST_EXISTS))
    {
        g_set_error(error, GST_URI_ERROR, G_FILE_ERROR_NOENT,
                    "File does not exist: %s", path);
        g_free(path);
        return FALSE;
    }

    // Build internal pipeline (filesrc)
    if (self->internal_src)
        gst_bin_remove(GST_BIN(self), self->internal_src);

    self->internal_src = gst_element_factory_make("filesrc", NULL);
    g_object_set(self->internal_src, "location", path, NULL);
    gst_bin_add(GST_BIN(self), self->internal_src);

    // Expose src pad
    GstPad *srcpad = gst_element_get_static_pad(self->internal_src, "src");
    GstPad *ghostpad = gst_ghost_pad_new("src", srcpad);
    gst_pad_set_active(ghostpad, TRUE);
    gst_element_add_pad(GST_ELEMENT(self), ghostpad);
    gst_object_unref(srcpad);

    g_free(path);
    return TRUE;
}

static void gst_assets_uri_handler_init(gpointer g_iface, gpointer iface_data)
{
    GstURIHandlerInterface *iface = (GstURIHandlerInterface *)g_iface;

    iface->get_type = gst_assets_uri_get_type;
    iface->get_protocols = gst_assets_uri_get_protocols;
    iface->get_uri = gst_assets_uri_get_uri;
    iface->set_uri = gst_assets_uri_set_uri;
}

static void gst_assets_init(GstAssets *self)
{
    ///
}

static void gst_assets_finalize(GObject *object)
{
    GstAssets *self = GST_ASSETS(object);

    if (self->internal_src)
    {
        gst_bin_remove(GST_BIN(self), self->internal_src);
        self->internal_src = NULL;
    }
}

static void gst_assets_class_init(GstAssetsClass *self)
{
    GstElementClass *element_class = GST_ELEMENT_CLASS(self);
    GObjectClass *gobject_class = G_OBJECT_CLASS(self);

    // gobject_class->set_property = gst_assets_set_property;
    // gobject_class->get_property = gst_assets_get_property;
    gobject_class->finalize = gst_assets_finalize;

    // g_object_class_install_property(
    //     gobject_class, 1,
    //     g_param_spec_pointer("uri", "URI",
    //                          "External or internal URI used by the plugin",
    //                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

    gst_element_class_set_static_metadata(
        element_class, "Assets plugins proxy Poc", "Example/FirstExample",
        "Goal to proxy custom URI", "Alexander Yanovskyy");
}

GType gst_assets_get_type(void)
{
    static GType assets_type = 0;

    if (!assets_type)
    {
        static const GTypeInfo assets_info = {
            sizeof(GstAssetsClass),
            NULL, /* base_init */
            NULL, /* base_finalize */
            (GClassInitFunc)gst_assets_class_init,
            NULL, /* class_finalize */
            NULL, /* class_data */
            sizeof(GstAssets),
            0, /* n_preallocs */
            (GInstanceInitFunc)gst_assets_init,
            NULL};

        static const GInterfaceInfo uri_hdlr_info = {
            gst_assets_uri_handler_init, NULL, NULL};

        assets_type =
            g_type_register_static(GST_TYPE_BIN, "GstAssets", &assets_info, 0);
        g_type_add_interface_static(assets_type, GST_TYPE_URI_HANDLER,
                                    &uri_hdlr_info);
    }

    return assets_type;
}