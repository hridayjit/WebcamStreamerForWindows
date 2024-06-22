#include <gst/gst.h>
// #include <gtk/gtk.h>
#include <gst/video/video.h>
// #include <opencv/cv.h>

typedef struct Elements {
  GstElementFactory *sourceFactory;
  GstElementFactory *filterFactory;
  GstElementFactory *convertFilterFactory;
  GstElementFactory *capsFilterFactory;
  GstElementFactory *sinkFactory;
  GstElement *pipeline;
  GstElement * source;
  GstElement *filter;
  GstElement *convertFilter;
  GstElement *capsFilter;
  GstElement *sink;
} elements;

static GstPadProbeReturn buffer_probe_callback (GstPad *pad, GstPadProbeInfo *info, gpointer user_data);

int webcamStream (int argc, char *argv[])
{
  GError *err = NULL;
  // GstElementFactory *factory;
  // GstElement * element;
  gchar *sourceName;
  gchar *filterName;
  gchar *sinkName;
  elements data;
  GstStateChangeReturn ret;
  GstBus *bus;
  GstMessage *msg;
  GMainLoop *loop;
  gboolean terminate = FALSE;
  GstPad *sinkpad;

  /* init GStreamer */
  // gst_init (&argc, &argv);
  gst_init_check(&argc, &argv, &err);

  /* create element, method #2 */
  //find_factory
  data.sourceFactory = gst_element_factory_find ("ksvideosrc");
  if (!data.sourceFactory) {
    g_print ("Failed to find factory of type 'ksvideosrc  '\n");
    return 1;
  }
  data.filterFactory = gst_element_factory_find ("gamma");
  if (!data.filterFactory) {
    g_print ("Failed to find factory of type 'gamma '\n");
    return 1;
  }
  data.convertFilterFactory = gst_element_factory_find ("videoconvert");
  if (!data.convertFilterFactory) {
    g_print ("Failed to find convert factory of type 'videoconvert '\n");
    return 1;
  }
  data.capsFilterFactory = gst_element_factory_find("capsfilter");
  if(!data.capsFilterFactory) {
    g_print("Failed to find factory of type 'capsfilter '\n");
    return 1;
  }
  data.sinkFactory = gst_element_factory_find("autovideosink");
  if(!data.sinkFactory) {
    g_print ("Failed to find factory of type 'autovideosink '\n");
    return 1;
  }

  //create_element_from_factory
  data.source = gst_element_factory_create (data.sourceFactory, "source");
  if (!data.source) {
    gst_object_unref(GST_OBJECT(data.sourceFactory));
    g_print ("Failed to create source element, even though its factory exists!\n");
    return 1;
  }
  data.filter = gst_element_factory_create (data.filterFactory, "filter");
  if (!data.filter) {
    gst_object_unref(GST_OBJECT(data.filterFactory));
    g_print ("Failed to create filter element, even though its factory exists!\n");
    return 1;
  }
  data.convertFilter = gst_element_factory_create (data.convertFilterFactory, "convertFilter");
  if (!data.convertFilter) {
    gst_object_unref(GST_OBJECT(data.convertFilterFactory));
    g_print ("Failed to create convert filter element, even though its factory exists!\n");
    return 1;
  }
  data.capsFilter = gst_element_factory_create(data.capsFilterFactory, "capsFilter");
  if(!data.capsFilter) {
    gst_object_unref(GST_OBJECT(data.capsFilterFactory));
    g_print("Failed to create caps filter element, even though its factory exists!\n");
    return 1;
  }
  data.convertFilter = gst_element_factory_create (data.convertFilterFactory, "convertFilter");
  if (!data.convertFilter) {
    gst_object_unref(GST_OBJECT(data.convertFilterFactory));
    g_print ("Failed to create convert filter element, even though its factory exists!\n");
    return 1;
  }
  data.sink = gst_element_factory_create(data.sinkFactory, "sink");
  if (!data.sink) {
    gst_object_unref(GST_OBJECT(data.sinkFactory));
    g_print ("Failed to create sink element, even though its factory exists!\n");
    return 1;
  }

  //recieving element names
  g_object_get(G_OBJECT(data.source), "name", &sourceName, NULL);
  g_object_get(G_OBJECT(data.filter), "name", &filterName, NULL);
  g_object_get(G_OBJECT(data.sink), "name", &sinkName, NULL);
  g_print("The name of the elements created are '%s', '%s', '%s'. \n", sourceName, filterName, sinkName);


  // create pipeline
  data.pipeline = gst_pipeline_new("my-pipeline");
  if(!data.pipeline) {
    g_free(sourceName);
    g_free(filterName);
    g_free(sinkName);
    gst_object_unref (GST_OBJECT (data.source));
    gst_object_unref (GST_OBJECT (data.filter));
    gst_object_unref (GST_OBJECT (data.convertFilter));
    gst_object_unref (GST_OBJECT (data.capsFilter));
    gst_object_unref (GST_OBJECT (data.sink));
     gst_object_unref (GST_OBJECT (data.sourceFactory));
    gst_object_unref (GST_OBJECT (data.filterFactory));
    gst_object_unref (GST_OBJECT (data.convertFilterFactory));
    gst_object_unref (GST_OBJECT (data.capsFilterFactory));
    gst_object_unref (GST_OBJECT (data.sinkFactory));
    g_print("The pipeline could not be created.\n");
    return 1;
  }

  // Set the gamma property
  g_object_set(data.filter, "gamma", 1.6, NULL);

  // Set the caps for RGB format
    GstCaps *caps = gst_caps_new_simple("video/x-raw",
                                        "format", G_TYPE_STRING, "RGB",
                                        NULL);
    g_object_set(data.capsFilter, "caps", caps, NULL);
    gst_caps_unref(caps);

  /* must add elements to pipeline before linking them */
  gst_bin_add_many (GST_BIN (data.pipeline), data.source, data.filter, data.convertFilter, data.capsFilter, data.sink, NULL);
 /* link */
  if (!gst_element_link_many (data.source, data.filter, data.convertFilter, data.capsFilter, data.sink, NULL)) {
    g_warning ("Failed to link elements!");
  }

  // Attach a probe to the sink pad to access frame data
  sinkpad = gst_element_get_static_pad (data.sink, "sink");
  if(!sinkpad) {
    g_printerr("Unable to get sink pad of sink.\n");
    gst_object_unref (data.pipeline);
    return 1;
  }
  gst_pad_add_probe (sinkpad, GST_PAD_PROBE_TYPE_BUFFER, (GstPadProbeCallback) buffer_probe_callback, NULL, NULL);
  gst_object_unref (sinkpad);

  /* Start playing */
  ret = gst_element_set_state (data.pipeline, GST_STATE_PLAYING);
  if (ret == GST_STATE_CHANGE_FAILURE) {
    g_printerr ("Unable to set the pipeline to the playing state.\n");
    gst_object_unref (data.pipeline);
    return 1;
  }

 /* Listen to the bus */
  bus = gst_element_get_bus (data.pipeline);
  do {
    msg = gst_bus_timed_pop_filtered (bus, GST_CLOCK_TIME_NONE,
        GST_MESSAGE_STATE_CHANGED | GST_MESSAGE_ERROR | GST_MESSAGE_EOS);

    /* Parse message */
    if (msg != NULL) {
    //   GError *err;
      gchar *debug_info;

      switch (GST_MESSAGE_TYPE (msg)) {
        case GST_MESSAGE_ERROR:
          gst_message_parse_error (msg, &err, &debug_info);
          g_printerr ("Error received from element %s: %s\n", GST_OBJECT_NAME (msg->src), err->message);
          g_printerr ("Debugging information: %s\n", debug_info ? debug_info : "none");
          g_clear_error (&err);
          g_free (debug_info);
          terminate = TRUE;
          break;
        case GST_MESSAGE_EOS:
          g_print ("End-Of-Stream reached.\n");
          terminate = TRUE;
          break;
        case GST_MESSAGE_STATE_CHANGED:
          /* We are only interested in state-changed messages from the pipeline */
          if (GST_MESSAGE_SRC (msg) == GST_OBJECT (data.pipeline)) {
            GstState old_state, new_state, pending_state;
            gst_message_parse_state_changed (msg, &old_state, &new_state, &pending_state);
            g_print ("Pipeline state changed from %s to %s:\n",
                gst_element_state_get_name (old_state), gst_element_state_get_name (new_state));
          }
          break;
        default:
          /* We should not reach here */
          g_printerr ("Unexpected message received.\n");
          break;
      }
      gst_message_unref (msg);
    }
  } while (!terminate);


  // // Create a main loop to handle GStreamer messages and callbacks
  //   loop = g_main_loop_new(NULL, FALSE);
  //   g_main_loop_run(loop);

  // g_main_loop_unref(loop);
  g_free(sourceName);
  g_free(filterName);
  g_free(sinkName);

  // gst_message_unref (msg);
  gst_object_unref (bus);
  gst_element_set_state (data.pipeline, GST_STATE_NULL);
  gst_object_unref(GST_OBJECT (data.pipeline));
  g_print("Closed Successfully");
  return 0;
}

static GstPadProbeReturn
buffer_probe_callback (GstPad *pad, GstPadProbeInfo *info, gpointer user_data) {
    GstBuffer *buffer = GST_PAD_PROBE_INFO_BUFFER (info);
    GstMapInfo map;
    
    if(gst_buffer_map (buffer, &map, GST_MAP_READ)) {
      GstCaps *caps = gst_pad_get_current_caps(pad);
      GstStructure *structure = gst_caps_get_structure(caps, 0);

      // Retrieve width and height from the caps
      gint width, height;
      const gchar *format;
      gst_structure_get_int(structure, "width", &width);
      gst_structure_get_int(structure, "height", &height);
      format = gst_structure_get_string(structure, "format");

      g_print("Received frame: width = %d, height = %d, size = %lu\n", width, height, map.size);

      if(g_strcmp0(format, "RGB") != 0) {
        g_printerr("Unexpected format: %s. Expected RGB.\n", format);
        gst_buffer_unmap(buffer, &map);
            gst_caps_unref(caps);
            return GST_PAD_PROBE_OK;
      }

      // Ensure the buffer size matches the expected size for the RGB format
        gsize expected_size = width * height * 3;
        if (map.size != expected_size) {
            g_printerr("Unexpected buffer size: %lu. Expected: %lu.\n", map.size, expected_size);
            gst_buffer_unmap(buffer, &map);
            gst_caps_unref(caps);
            return GST_PAD_PROBE_OK;
        }

        // // Convert GStreamer buffer to OpenCV Mat
        // Mat frame(height, width, CV_8UC3, map.data);

        // // Process the frame using OpenCV (for example, convert to grayscale)
        // Mat gray_frame;
        // cvtColor(frame, gray_frame, COLOR_RGB2GRAY);

        // // Display the frame using OpenCV
        // imshow("Webcam", gray_frame);
        // waitKey(1); // Required to keep the window open and process events


       // Access pixel data here (map.data) and process as needed
      guint8 r, g, b;

      guint8 *pixels = map.data;
      for (gint y = 0; y < height; y++) {
          for (gint x = 0; x < width; x++) {
              guint8 *pixel = pixels + (y * width + x) * 3;
              guint8 r = pixel[0];
              guint8 g = pixel[1];
              guint8 b = pixel[2];
              
              // Process the RGB pixel data as needed
              // For example, you can print the first pixel's RGB values
              if (x == 0 && y == 0) {
                  g_print("First pixel RGB: R=%d, G=%d, B=%d\n", r, g, b);
              }
          }
      }

      gst_buffer_unmap (buffer, &map);
      gst_caps_unref(caps);
    }
    return GST_PAD_PROBE_OK;
}