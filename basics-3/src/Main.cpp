#include <iostream>
#include <gst/gst.h>

/* Structure to contain all our information, so we can pass it to CBs */
typedef struct _CustomData {
    GstElement *pipeline;
    GstElement *source;
    GstElement *video_convert;
    GstElement *audio_convert;
    GstElement *resample;
    GstElement *video_sink;
    GstElement *audio_sink;
} CustomData;

/* Handler for the pad-added signal */
static void pad_added_handler(GstElement *src, GstPad *pad, CustomData *data);
static bool check_error_elements_created(CustomData &data);
static void retrive_pad_info(GstPad *pad);

int 
main(int argc, char* argv[])
{
    CustomData data;
    GstBus *bus;
    GstMessage *msg;
    GstStateChangeReturn ret;
    gboolean terminate = FALSE;

    /* Initialize GStreamer */
    std::cout << "Init gst\n";
    gst_init(&argc, &argv);

    /* Create the elements */
    std::cout << "Create elements\n";
    data.source = gst_element_factory_make("uridecodebin", "source");
    data.video_convert = gst_element_factory_make("videoconvert", "video_convert");
    data.audio_convert = gst_element_factory_make("audioconvert", "audio_convert");
    data.resample = gst_element_factory_make("audioresample", "resample");
    data.video_sink = gst_element_factory_make("autovideosink", "video_sink");
    data.audio_sink = gst_element_factory_make("autoaudiosink", "audio_sink");

    /* Create the empty pipeline */
    std::cout << "Create the empty pipeline\n";
    data.pipeline = gst_pipeline_new("test-pipeline");

    /* Test if everything went well with element creation */
    std::cout << "Test if everything went well with element creation\n";
    if (check_error_elements_created(data) == TRUE)
        return -1;

    /* Build the pipeline. We are NOT linking the source at this
     * point. We will do it later. 
     */
    std::cout << "Build the pipeline without link\n";
    gst_bin_add_many(GST_BIN(data.pipeline), 
                    data.source,
                    data.video_convert,
                    data.video_sink,
                    data.audio_convert, 
                    data.resample,
                    data.audio_sink,
                    NULL);

    gboolean retval = gst_element_link_many(data.video_convert, data.video_sink, NULL);
    if (retval == FALSE)
    {
        g_printerr("Elements in video chain could not be linked.\n");
        gst_object_unref(data.pipeline);
        return -1;
    }

    retval = gst_element_link_many(data.audio_convert, data.resample, data.audio_sink, NULL);
    if (retval == FALSE)
    {
        g_printerr("Elements audio chain could not be linked.\n");
        gst_object_unref(data.pipeline);
        return -1;
    }
    

    /* Set the URI to play */
    g_object_set(data.source, "uri", "https://www.freedesktop.org/software/gstreamer-sdk/data/media/sintel_trailer-480p.webm", NULL);
    
    /* Connect to the pad-added signal */
    g_signal_connect(data.source, "pad-added", G_CALLBACK(pad_added_handler), &data);

    /* Start playing */
    ret = gst_element_set_state(data.pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE)
    {
        g_printerr("Unable to set the pipeline to the playing state.\n");
        gst_object_unref(data.pipeline);
        return -1;
    }

    /* Listen to the bus */
    bus = gst_element_get_bus(data.pipeline);
    do 
    {
        msg = gst_bus_timed_pop_filtered(bus, GST_CLOCK_TIME_NONE,
            (GstMessageType)(GST_MESSAGE_STATE_CHANGED | GST_MESSAGE_ERROR | GST_MESSAGE_EOS));
        
        /* Parse message */
        if (msg != NULL)
        {
            GError *err;
            gchar *debug_info;

            switch (GST_MESSAGE_TYPE(msg)) 
            {
                case GST_MESSAGE_ERROR:
                gst_message_parse_error(msg, &err, &debug_info);
                g_printerr("Error received from element %s: %s\n", GST_OBJECT_NAME(msg->src), err->message);
                g_printerr("Debugging information: %s\n", debug_info ? debug_info : "none");
                g_clear_error(&err);
                g_free(debug_info);
                terminate = TRUE;
                break;

                case GST_MESSAGE_EOS:
                g_print("End-Of-Stream reached.\n");
                terminate = TRUE;
                break;

                case GST_MESSAGE_STATE_CHANGED:
                /* We are only interested in state-changed messages from the pipeline */
                if (GST_MESSAGE_SRC(msg) == GST_OBJECT(data.pipeline)) {
                    GstState old_state, new_state, pending_state;
                    gst_message_parse_state_changed(msg, &old_state, &new_state, &pending_state);
                    g_print("Pipeline state changed from %s to %s:\n",
                        gst_element_state_get_name(old_state), gst_element_state_get_name(new_state));
                }
                break;

                default:
                /* We should not reach here */
                g_printerr("Unexpected message received.\n");
                break;
            }
            gst_message_unref(msg);
        }
    } while (!terminate);

    /* Free resources */
    gst_object_unref(bus);
    gst_element_set_state(data.pipeline, GST_STATE_NULL);
    gst_object_unref(data.pipeline);
    return 0;
}

/* This is the definition of the callback funcion for when the pad is added */
static void pad_added_handler(GstElement *src, GstPad *new_pad, CustomData *data)
{
    GstPad *sink_pad = NULL;
    GstPadLinkReturn ret;
    GstCaps *new_pad_caps = NULL;
    GstStructure *new_pad_struct = NULL;
    const gchar *new_pad_type = NULL;

    g_print("Received new pad '%s' from '%s': \n", GST_PAD_NAME(new_pad), GST_ELEMENT_NAME(src));

    /* Check the new pad's type and Attempt the link */
    new_pad_caps = gst_pad_get_current_caps(new_pad);
    new_pad_struct = gst_caps_get_structure(new_pad_caps, 0);
    new_pad_type = gst_structure_get_name(new_pad_struct);

    if (g_str_has_prefix(new_pad_type, "video/x-raw"))
    {
        std::cout << "Linking video pads\n";
        sink_pad = gst_element_get_compatible_pad(data->video_convert, new_pad, new_pad_caps);

        /* Try to get info on the compatible pad */
        retrive_pad_info(sink_pad);

        if (gst_pad_is_linked(sink_pad))
        {
            g_print("Video pad already linked. Ignoring...\n");
            goto exit;
        }
        ret = gst_pad_link(new_pad, sink_pad);
    }
    else if (g_str_has_prefix(new_pad_type, "audio/x-raw"))
    {
        std::cout << "Linking audio pads\n";
        sink_pad = gst_element_get_compatible_pad(data->audio_convert, new_pad, new_pad_caps);

        /* Try to get info on the compatible pad */
        retrive_pad_info(sink_pad);

        if (gst_pad_is_linked(sink_pad))
        {
            g_print("Audio pad already linked. Ignoring...\n");
            goto exit;
        }
        ret = gst_pad_link(new_pad, sink_pad);
    }
    else
    {
        g_print("It has type '%s' which is not necessary. Ignoring...\n", new_pad_type);
        goto exit;
    }

    if (GST_PAD_LINK_FAILED(ret))
    {
        g_print("Type is '%s' but link failed. \n", new_pad_type);
    }
    else
    {
        g_print("Link succeeded (type '%s'). \n", new_pad_type);
    }

exit:
    /* Unreference the new pad's caps, if we got them */
    if (new_pad_caps != NULL)
    {
        gst_caps_unref(new_pad_caps);
    }

    /* Unreference the sink pads */
    if (sink_pad != nullptr)
        gst_object_unref(sink_pad);
}

static bool check_error_elements_created(CustomData &data)
{
    bool err_detected = false;
    if (data.pipeline == nullptr)
    {
        g_printerr("Pipeline element could not be created. \n");
        err_detected = true;
    }
    if (data.source == nullptr)
    {
        g_printerr("Source element could not be created. \n");
        err_detected = true;
    }
    if (data.video_convert == nullptr)
    {
        g_printerr("Video convert element could not be created. \n");
        err_detected = true;
    }
    if (data.audio_convert == nullptr)
    {
        g_printerr("Audio convert element could not be created. \n");
        err_detected = true;
    }
    if (data.resample == nullptr)
    {
        g_printerr("Resample element could not be created. \n");
        err_detected = true;
    }
    if (data.video_sink == nullptr)
    {
        g_printerr("Video sink element could not be created. \n");
        err_detected = true;
    }
    if (data.audio_sink == nullptr)
    {
        g_printerr("Audio sink element could not be created. \n");
        err_detected = true;
    }
    return err_detected;
}

static void retrive_pad_info(GstPad *pad)
{
    GstCaps *pad_caps = nullptr;
    GstStructure *pad_struct = nullptr;
    const gchar *pad_type = nullptr;

    std::cout << "Trying to retrieve pad info about "<< GST_PAD_NAME(pad) << " pad\n";

    if (gst_pad_has_current_caps(pad))
    {
            pad_caps = gst_pad_get_current_caps(pad);
            pad_struct = gst_caps_get_structure(pad_caps, 0);
            pad_type = gst_structure_get_name(pad_struct);
            std::cout << "Compatible pad name is " << pad_type << "\n"; 
    }
    else
    {
        std::cout << "No current caps for "<< GST_PAD_NAME(pad) << " pad\n";
    }
}