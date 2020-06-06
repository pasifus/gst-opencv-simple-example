#ifndef processPipeline_H
#define processPipeline_H

#include <string>

#include <gst/gst.h>
#include <gst/app/gstappsrc.h>
#include <gst/app/gstappsink.h>

class Pipeline
{
    public:
        Pipeline(const std::string& deviceLocation);
        virtual ~Pipeline();
        
        bool Open();
        void Close();
    private:
        bool CreateProcessPipeline();
        bool CreateDisplayPipeline(int width, int height);

        static gboolean MessageHandle(GstBus *bus, GstMessage *message, gpointer userData);
        static GstFlowReturn OnNewPreroll(GstAppSink *appSink, gpointer userData);
        static GstFlowReturn OnNewSample(GstAppSink *appSink, gpointer userData);
    private:
        std::string deviceLocation_;

        GstElement* processPipeline_ = NULL;
        GstElement* processSrc_ = NULL;
        GstElement* processConvert_ = NULL;
        GstElement* processFilter_ = NULL;
        GstElement* processSink_ = NULL;

        GstElement* displayPipeline_ = NULL;
        GstElement* displaySrc_ = NULL;
        GstElement* displayConvert_ = NULL;
        GstElement* displaySink_ = NULL;

        int64_t pipelineStartedAt_ = 0;
        bool initDisplayPipeline_ = false;
};

#endif /* processPipeline_H */