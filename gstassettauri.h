#pragma once

#include <gst/base/gstpushsrc.h>
#include <gst/check/gstcheck.h>
#include <gst/gst.h>

GType gst_asset_tauriget_type(void);

#define GST_TYPE_ASSETS (gst_asset_tauriget_type())
#define GST_ASSETS(obj)                                                        \
    (G_TYPE_CHECK_INSTANCE_CAST((obj), GST_TYPE_ASSETS, GstTauriAsset))
#define gst_asset_tauriCLASS(klass)                                            \
    (G_TYPE_CHECK_CLASS_CAST((klass), GST_TYPE_ASSETS, GstTauriAssetClass))
#define GST_IS_ASSETS(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), GST_TYPE_ASSETS))
#define GST_IS_ASSETS_CLASS(klass)                                             \
    (G_TYPE_CHECK_CLASS_TYPE((klass), GST_TYPE_ASSETS))

typedef struct _GstAssets
{
    GstBin parent_instance;
    GstElement *internal_src;
} GstTauriAsset;

typedef struct _GstAssetsClass
{
    GstBinClass parent_class;
} GstTauriAssetClass;

#define ASSET_URI_STR "asset"

static gboolean plugin_init(GstPlugin *plugin)
{
    return gst_element_register(plugin, "assettauri", GST_RANK_NONE,
                                GST_TYPE_ASSETS);
}
