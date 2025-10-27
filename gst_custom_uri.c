#include "gst_assets.h"
#include <gst/base/gstpushsrc.h>
#include <gst/check/gstcheck.h>
#include <gst/gst.h>

// https://gstreamer.freedesktop.org/documentation/tutorials/basic/dynamic-pipelines.html?gi-language=c

// G_DEFINE_TYPE(GstAssets, gst_assets, GST_TYPE_ELEMENT);

GST_ELEMENT_REGISTER_DEFINE(assets, "assets", GST_RANK_NONE, GST_TYPE_ASSETS);

static gboolean plugin_init(GstPlugin *plugin)
{
    return GST_ELEMENT_REGISTER(assets, plugin);
}

#define PACKAGE "AssetsPoC"
#define VERSION "1.0"
GST_PLUGIN_DEFINE(GST_VERSION_MAJOR, GST_VERSION_MINOR, assets,
                  "Plugin proxy PoC", plugin_init, VERSION, "LGPL", "GStreamer",
                  "http://gstreamer.net/")

static void cb_pad_added(GstElement *dec, GstPad *pad, gpointer data)
{
    GstElement *ssink = (GstElement *)data;
    GstCaps *caps;
    GstStructure *str;
    const gchar *name;
    GstPadTemplate *templ;
    GstElementClass *klass;

    /* check media type */
    caps = gst_pad_query_caps(pad, NULL);
    str = gst_caps_get_structure(caps, 0);
    name = gst_structure_get_name(str);

    klass = GST_ELEMENT_GET_CLASS(ssink);

    if (g_str_has_prefix(name, "audio"))
    {
        templ = gst_element_class_get_pad_template(klass, "audio_sink");
    }
    else if (g_str_has_prefix(name, "video"))
    {
        templ = gst_element_class_get_pad_template(klass, "video_sink");
    }
    else if (g_str_has_prefix(name, "text"))
    {
        templ = gst_element_class_get_pad_template(klass, "text_sink");
    }
    else
    {
        templ = NULL;
    }

    if (templ)
    {
        GstPad *sinkpad;

        sinkpad = gst_element_request_pad(ssink, templ, NULL, NULL);

        if (!gst_pad_is_linked(sinkpad))
            gst_pad_link(pad, sinkpad);

        gst_object_unref(sinkpad);
    }

    gst_clear_caps(&caps);
}

static gboolean my_bus_callback(GstBus *bus, GstMessage *message, gpointer data)
{
    GMainLoop *loop = (GMainLoop *)data;
    GError *err;
    gchar *debug;
    switch (GST_MESSAGE_TYPE(message))
    {
    case GST_MESSAGE_ERROR:
    {
        gst_message_parse_error(message, &err, &debug);
        g_print("Error: %s\n", err->message);
        g_error_free(err);
        g_free(debug);

        g_main_loop_quit(loop);
        break;
    }
    case GST_MESSAGE_WARNING:
    {
        gst_message_parse_warning(message, &err, &debug);
        g_print("Warning: %s\n", err->message);
        g_error_free(err);
        g_free(debug);
        break;
    }
    case GST_MESSAGE_EOS:
        /* end-of-stream */
        g_main_loop_quit(loop);
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

gint main(gint argc, gchar *argv[])
{

    gst_init(&argc, &argv);

    if (!gst_plugin_register_static(GST_VERSION_MAJOR, GST_VERSION_MINOR,
                                    "assetsrc", "Red Video Source", plugin_init,
                                    "1.0", "LGPL", "GStreamer",
                                    "http://gstreamer.net/", "source"))
    {
        g_printerr("Failed to register assetsrc plugin\n");
        return -1;
    }

    if (argc != 2)
    {
        g_print("Usage: %s <uri>\n", argv[0]);
        return -1;
    }

    GMainLoop *loop = g_main_loop_new(NULL, FALSE);

    if (argc != 2)
    {
        g_print("Usage: %s <uri>\n", argv[0]);
        return -1;
    }

    GstElement *pipeline = gst_pipeline_new("pipeline");
    GstElement *sink = gst_element_factory_make("playsink", "sink");

    GstBus *bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline));
    gst_bus_add_watch(bus, my_bus_callback, loop);
    gst_object_unref(bus);

    GstElement *uridecodebin =
        gst_element_factory_make("uridecodebin", "source");
    g_object_set(G_OBJECT(uridecodebin), "uri", argv[1], NULL);
    g_signal_connect(uridecodebin, "pad-added", G_CALLBACK(cb_pad_added), sink);

    gst_bin_add_many(GST_BIN(pipeline), uridecodebin, sink, NULL);

    gst_element_set_state(pipeline, GST_STATE_PLAYING);
    g_main_loop_run(loop);

    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(GST_OBJECT(pipeline));
    return 0;
}

// https://gstreamer.freedesktop.org/documentation/application-development/highlevel/playback-components.html?gi-language=c
