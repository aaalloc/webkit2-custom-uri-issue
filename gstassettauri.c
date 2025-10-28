#include "gstassettauri.h"

#include <gst/base/gstpushsrc.h>
#include <gst/check/gstcheck.h>
#include <gst/gst.h>

static GstURIType gst_asset_tauriuri_get_type(GType type)
{
    return GST_URI_SRC;
}

static const gchar *const *gst_asset_tauriuri_get_protocols(GType type)
{
    static const gchar *protocols[] = {ASSET_URI_STR, NULL};

    return protocols;
}

static gchar *gst_asset_tauriuri_get_uri(GstURIHandler *handler)
{
    return g_strdup_printf("%s://", ASSET_URI_STR);
}

static gboolean gst_asset_tauriuri_set_uri(GstURIHandler *handler,
                                           const gchar *uri, GError **error)
{
    GstTauriAsset *self = GST_ASSETS(handler);

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

static void gst_asset_tauriuri_handler_init(gpointer g_iface,
                                            gpointer iface_data)
{
    GstURIHandlerInterface *iface = (GstURIHandlerInterface *)g_iface;

    iface->get_type = gst_asset_tauriuri_get_type;
    iface->get_protocols = gst_asset_tauriuri_get_protocols;
    iface->get_uri = gst_asset_tauriuri_get_uri;
    iface->set_uri = gst_asset_tauriuri_set_uri;
}

static void gst_asset_tauriinit(GstTauriAsset *self)
{
    ///
}

static void gst_asset_taurifinalize(GObject *object)
{
    GstTauriAsset *self = GST_ASSETS(object);

    if (self->internal_src)
    {
        gst_bin_remove(GST_BIN(self), self->internal_src);
        self->internal_src = NULL;
    }
}

static void gst_asset_tauriclass_init(GstTauriAssetClass *self)
{
    GstElementClass *element_class = GST_ELEMENT_CLASS(self);
    GObjectClass *gobject_class = G_OBJECT_CLASS(self);

    gobject_class->finalize = gst_asset_taurifinalize;

    gst_element_class_set_static_metadata(
        element_class, "Assets plugins proxy Poc", "Example/FirstExample",
        "Goal to proxy custom URI", "Alexander Yanovskyy");
}

GType gst_asset_tauriget_type(void)
{
    static GType assets_type = 0;

    if (!assets_type)
    {
        static const GTypeInfo assets_info = {
            sizeof(GstTauriAssetClass),
            NULL, /* base_init */
            NULL, /* base_finalize */
            (GClassInitFunc)gst_asset_tauriclass_init,
            NULL, /* class_finalize */
            NULL, /* class_data */
            sizeof(GstTauriAsset),
            0, /* n_preallocs */
            (GInstanceInitFunc)gst_asset_tauriinit,
            NULL};

        static const GInterfaceInfo uri_hdlr_info = {
            gst_asset_tauriuri_handler_init, NULL, NULL};

        assets_type = g_type_register_static(GST_TYPE_BIN, "GstTauriAsset",
                                             &assets_info, 0);
        g_type_add_interface_static(assets_type, GST_TYPE_URI_HANDLER,
                                    &uri_hdlr_info);
    }

    return assets_type;
}

#define PACKAGE "_asset_tauri_"
#define VERSION "1.0"
GST_PLUGIN_DEFINE(GST_VERSION_MAJOR, GST_VERSION_MINOR, assettauri,
                  "Plugin proxy PoC", plugin_init, VERSION, "LGPL",
                  "gst_asset_tauri_package", "http://gstreamer.net/")