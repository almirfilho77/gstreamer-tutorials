#include <iostream>
#include <gst/gst.h>

int 
main(int argc, char* argv[])
{
    GstElement *pipeline, *source, *sink;

    // Exercise section of the tutorial
    GstElement *filter;

    GstBus *bus;
    GstMessage *msg;
    GstStateChangeReturn ret;

    /* Initialize GStreamer */
    gst_init (&argc, &argv);

    /* Create the elements */
    source = gst_element_factory_make("videotestsrc", "video_source");
    sink = gst_element_factory_make("autovideosink", "video_sink");

    // Exercise section of the tutorial
    filter = gst_element_factory_make("vertigotv", "video_filter");

    /* Create the empty pipeline */
    pipeline = gst_pipeline_new("test-pipeline");

    // Learning purposes: compare pointer addresses to after they are freed
    std::cout << "Pipeline pointer address: " << pipeline << std::endl;
    std::cout << "Source pointer address: " << source << std::endl;
    std::cout << "Filter pointer address: " << filter << std::endl;
    std::cout << "Sink pointer address: " << sink << std::endl;


    /* Build the pipeline */
    gst_bin_add_many(GST_BIN(pipeline), 
                            source,
                            filter, // Exercise section of the tutorial
                            sink,
                            NULL);
    
    if (gst_element_link(source, filter) != TRUE || 
        gst_element_link(filter, sink)   != TRUE
        )
    {
        g_printerr("Elements could not be linked");
        gst_object_unref(pipeline);
        return -1;
    }

    /* Modify the source's pattern property 
     * 
     * More info about the possible values of this property can be found at
     * this link: https://gstreamer.freedesktop.org/documentation/videotestsrc/index.html?gi-language=c#GstVideoTestSrcPattern
     * or using gst-inspect-1.0 tool
     */
    g_object_set(source, "pattern", 0, NULL);

    /* Start playing */
    ret = gst_element_set_state(pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) 
    {
        g_printerr ("Unable to set the pipeline to the playing state.\n");
        gst_object_unref (pipeline);
        return -1;
    }

    /* Wait until error or EOS */
    bus = gst_element_get_bus (pipeline);
    msg =
        gst_bus_timed_pop_filtered (bus, GST_CLOCK_TIME_NONE,
        static_cast<GstMessageType>(GST_MESSAGE_ERROR | GST_MESSAGE_EOS));

    /* Parse error msg */
    if (msg != NULL)
    {
        GError *err;
        gchar *debug_info;

        switch(GST_MESSAGE_TYPE (msg))
        {
            case GST_MESSAGE_ERROR:
                gst_message_parse_error(msg, &err, &debug_info);
                g_printerr("Error received from element %s: %s\n",
                            GST_OBJECT_NAME(msg->src),
                            err->message);
                g_printerr("Debugging information: %s\n",
                            debug_info ? debug_info : "none");
                g_clear_error(&err);
                g_free(debug_info);
                break;
            
            case GST_MESSAGE_EOS:
                g_print("End of stream.\n");
                break;
            
            default:
                /* We should not reach here because we only asked for ERRORs and EOS */
                g_printerr ("Unexpected message received.\n");
                break;
        }
        gst_message_unref(msg);
    }

    /* Free resources */
    gst_object_unref (bus);

    /* According to the tutorial:
     * "Setting the pipeline to the NULL state will make sure it frees any resources 
     * it has allocated (More about states in Basic tutorial 3: Dynamic pipelines). 
     * Finally, unreferencing the pipeline will destroy it, and all its contents."
     */ 
    gst_element_set_state (pipeline, GST_STATE_NULL);
    gst_object_unref (pipeline);

    return 0;
}