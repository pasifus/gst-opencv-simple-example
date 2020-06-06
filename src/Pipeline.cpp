#include <fstream>
#include <iomanip>
#include <chrono>
#include <memory>

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>

#include "Pipeline.h"
#include "Logger.h"

Pipeline::Pipeline(const std::string& deviceLocation) :
    deviceLocation_(deviceLocation)
{

}

Pipeline::~Pipeline()
{
    Close();
}

bool Pipeline::Open()
{
    LOG_INFO("Opening pipeline");

    if(!gst_is_initialized())
    {
        gst_init(NULL, NULL);
        gchar* version = gst_version_string();
        LOG_ASSERT(version);
        
        LOG_INFO("Gstreamer version: %s", version);
        g_free(version);
    }

    if(!CreateProcessPipeline())
    {
        LOG_ERROR("Failed  CreateProcessPipeline");
        return false;
    }

    pipelineStartedAt_ = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

    return true;
}

bool Pipeline::CreateProcessPipeline()
{
    LOG_DEBUG("Pipeline::CreateProcessPipeline");

    processPipeline_ = gst_pipeline_new(NULL);
    LOG_ASSERT(processPipeline_);

    // message handler
    GstBus *bus = gst_pipeline_get_bus(GST_PIPELINE(processPipeline_));
    LOG_ASSERT(bus);
   
	gst_bus_add_signal_watch(bus);
	LOG_ASSERT(g_signal_connect(G_OBJECT(bus), "message", G_CALLBACK(MessageHandle), this));
   
	gst_object_unref(GST_OBJECT(bus));

    // testing: videotestsrc
    processSrc_ = gst_element_factory_make("v4l2src", "src");
    LOG_ASSERT(processSrc_);

    processConvert_ = gst_element_factory_make("videoconvert", "convert");
    LOG_ASSERT(processConvert_);

    processFilter_ = gst_element_factory_make("capsfilter", "filter");
    LOG_ASSERT(processFilter_);

    processSink_ = gst_element_factory_make("appsink", "sink");
    LOG_ASSERT(processSink_);

    // params
    g_object_set(processSrc_, "device", deviceLocation_.c_str(), NULL);

    // filter - caps
    GstCaps *caps = gst_caps_from_string("video/x-raw, format=I420");
    LOG_ASSERT(caps); 
    g_object_set(G_OBJECT(processFilter_), "caps", caps, NULL);
    gst_caps_unref(caps);

    // limit sink buffer
    g_object_set(processSink_, "max-buffers", 1, NULL);

    // add signal
    gst_app_sink_set_emit_signals((GstAppSink*)processSink_, TRUE);
    LOG_ASSERT(g_signal_connect(processSink_, "new-preroll", G_CALLBACK(OnNewPreroll), this));
    LOG_ASSERT(g_signal_connect(processSink_, "new-sample", G_CALLBACK(OnNewSample), this));

    // link elements
    gst_bin_add_many(GST_BIN(processPipeline_), processSrc_, processConvert_, processFilter_, processSink_, NULL);
    if (!gst_element_link_many(processSrc_, processConvert_, processFilter_, processSink_, NULL))
    {
        LOG_ERROR("Failed link");
        return false;
    }

    // switch to play
    if(gst_element_set_state(processPipeline_, GST_STATE_PLAYING) == GST_STATE_CHANGE_FAILURE)
    {
        LOG_ERROR("Failed  play");
        return false;
    }

    return true;
}

bool Pipeline::CreateDisplayPipeline(int width, int height)
{
    LOG_DEBUG("Pipeline::CreateDisplayPipeline");

    displayPipeline_ = gst_pipeline_new(NULL);
    LOG_ASSERT(displayPipeline_);
        
    // message handler
    GstBus *bus = gst_pipeline_get_bus(GST_PIPELINE(displayPipeline_));
    LOG_ASSERT(bus);
   
	gst_bus_add_signal_watch(bus);
	LOG_ASSERT(g_signal_connect(G_OBJECT(bus), "message", G_CALLBACK(MessageHandle), this));
   
	gst_object_unref(GST_OBJECT(bus));

    displaySrc_ = gst_element_factory_make("appsrc", "src");
    LOG_ASSERT(displaySrc_);

    displayConvert_ = gst_element_factory_make("videoconvert", "convert");
    LOG_ASSERT(displayConvert_);

    displaySink_ = gst_element_factory_make("xvimagesink", "sink");
    LOG_ASSERT(displaySink_);

    // filter - caps
    GstCaps *caps = gst_caps_new_simple("video/x-raw",
                                        "format", G_TYPE_STRING, "I420",
                                        "width", G_TYPE_INT, width,
                                        "height", G_TYPE_INT, height,
                                        NULL);
    LOG_ASSERT(caps); 
    g_object_set(G_OBJECT(displaySrc_), "caps", caps, NULL);
    gst_caps_unref(caps);

    // link elements
    gst_bin_add_many(GST_BIN(displayPipeline_), displaySrc_, displayConvert_, displaySink_, NULL);
    if(!gst_element_link_many(displaySrc_, displayConvert_, displaySink_, NULL))
    {
        LOG_ERROR("Failed link");
        return false;
    }

    // switch to play
    if(gst_element_set_state(displayPipeline_, GST_STATE_PLAYING) == GST_STATE_CHANGE_FAILURE)
    {
        LOG_ERROR("Failed  play");
        return false;
    }

    return true;
}

void Pipeline::Close()
{
    LOG_INFO("Closing pipeline");
}

gboolean Pipeline::MessageHandle(GstBus *bus, GstMessage *message, gpointer userData)
{
    (void)bus;

    LOG_ASSERT(userData);
    Pipeline *self = (Pipeline*)userData;

    switch(GST_MESSAGE_TYPE(message)) 
    {
        case GST_MESSAGE_ERROR:
        {
            GError *err = NULL;
            gchar *debug = NULL;

            gst_message_parse_error(message, &err, &debug);

            if(err)
            {
                gchar *name = gst_object_get_path_string(message->src);
                LOG_ERROR("Pipeline::MessageHandle %s: %s", name, err->message);
                g_free(name);
                g_error_free(err);
            }
            
            if(debug)
            {
                LOG_ERROR("Pipeline::MessageHandle %s", debug);
                g_free(debug);
            }

            if(self->processPipeline_)
                gst_element_set_state(self->processPipeline_, GST_STATE_NULL); 
            if(self->displayPipeline_)
                gst_element_set_state(self->displayPipeline_, GST_STATE_NULL);  

            LOG_ASSERT(TRUE);
        }
        break;

        case GST_MESSAGE_WARNING:
        {
            GError *err = NULL;
            gchar *debug = NULL;

            gst_message_parse_warning(message, &err, &debug);

            if(err)
            {
                gchar *name = gst_object_get_path_string(message->src);
                LOG_WARNING("Pipeline::MessageHandle %s: %s", name, err->message);
                g_free(name);
                g_error_free(err);
            }

            if(debug)
            {
                LOG_WARNING("Pipeline::MessageHandle: %s", debug);
                g_free(debug);
            }
        }
        break;

        case GST_MESSAGE_EOS: 
        {
            LOG_WARNING("Pipeline::MessageHandle EOS");

            LOG_ASSERT(TRUE);
        }
        break;

        default:
            break;
    }

    return TRUE;
}

GstFlowReturn Pipeline::OnNewPreroll(GstAppSink *appSink, gpointer userData)
{
    // LOG_INFO("New preroll\n");

    LOG_ASSERT(userData);
    Pipeline *self = (Pipeline*)userData;

    GstSample* sample = gst_app_sink_pull_preroll(appSink);
    LOG_ASSERT(sample);
    GstCaps *caps = gst_sample_get_caps (sample);
    GstStructure *structure = gst_caps_get_structure (caps, 0);

    int width, height;
    gst_structure_get_int(structure, "width", &width);
    gst_structure_get_int(structure, "height", &height);
    gst_sample_unref(sample);

    LOG_INFO("Pipeline started with resolution (%dx%d)", width, height);

    if(!self->CreateDisplayPipeline(width, height))
    {
        LOG_ERROR("Failed  CreateDisplayPipeline");
        return GST_FLOW_ERROR;
    }

    return GST_FLOW_OK;
}

GstFlowReturn Pipeline::OnNewSample(GstAppSink *appSink, gpointer userData)
{
    // LOG_INFO("New sample\n");

    LOG_ASSERT(userData);
    Pipeline *self = (Pipeline*)userData;

    GstSample* sample = gst_app_sink_pull_sample(appSink);
    LOG_ASSERT(sample);

    GstCaps *caps = gst_sample_get_caps(sample);
    GstStructure *structure = gst_caps_get_structure(caps, 0);
    int width, height;
    gst_structure_get_int(structure, "width", &width);
    gst_structure_get_int(structure, "height", &height);
    const gchar* format = gst_structure_get_string(structure, "format");
    LOG_ASSERT(format);
    LOG_ASSERT(!g_strcmp0(format, "I420"));

    GstBuffer* buffer = gst_sample_get_buffer(sample);
    LOG_ASSERT(buffer);

    auto pts = (int64_t)(GST_BUFFER_PTS(buffer) / 1000000);
// g_print("pts=%ld format=%s %dx%d\n", pts, format, width, height);

    // buffer map
    GstMapInfo info;
    LOG_ASSERT(gst_buffer_map(buffer, &info, GST_MAP_READ));

    // rotate 180
    cv::Mat yFrame(height, width, CV_8UC1, info.data);
    cv::flip(yFrame, yFrame, 0);
    cv::Mat uFrame(height / 2, width / 2, CV_8UC1, info.data + height * width);
    cv::flip(uFrame, uFrame, 0);
    cv::Mat vFrame(height / 2, width / 2, CV_8UC1, info.data + height * width + (height / 2) * width / 2);
    cv::flip(vFrame, vFrame, 0);

    // create test for machine timestamp
    cv::Mat frame(height * 3/2, width, CV_8UC1, (char*)info.data);
    std::string tsText = "ts: " + std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count());
    // {
    //     auto now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    //     std::stringstream ss;
    //     std::time_t temp = now/1000;
    //     std::tm* t = std::gmtime(&temp);
    //     ss << std::put_time(t, "%Y-%m-%d %I:%M:%S %p");
    //     tsText = "ts: " + ss.str();
    // }
    

    // crete test for frame pts
    std::string ptsText = "pts: " + std::to_string(pts + self->pipelineStartedAt_);
    // {
    //     std::stringstream ss;
    //     std::time_t temp = (pts + self->pipelineStartedAt_)/1000;
    //     std::tm* t = std::gmtime(&temp);
    //     ss << std::put_time(t, "%Y-%m-%d %I:%M:%S %p");
    //     ptsText = "pts: " + ss.str();
    // }

    cv::putText(frame,                                      // image
                tsText,                                     // text
                cv::Point(10, frame.rows / 3),              // position
                cv::FONT_HERSHEY_SIMPLEX,
                1.0,
                CV_RGB(0xf00, 0x00, 0xff),
                2);
    
    cv::putText(frame,                                      // image
                ptsText,                                    // text
                cv::Point(10, frame.rows / 2),              // position
                cv::FONT_HERSHEY_SIMPLEX,
                1.0,
                CV_RGB(0xf00, 0x00, 0xff),
                2);


    // send to displayPipeline
    GstFlowReturn ret;
    g_signal_emit_by_name(self->displaySrc_, "push-buffer", buffer, &ret);
    if(ret != GST_FLOW_OK) 
        LOG_ERROR("Failed push-buffer");

    gst_buffer_unmap(buffer, &info);
    gst_sample_unref(sample);

	return GST_FLOW_OK;
}