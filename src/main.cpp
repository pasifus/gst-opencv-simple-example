/*
    gst-opencv-simple-example
*/

#include <stdio.h>
#include <signal.h>
#include <unistd.h>

#include "Pipeline.h"
#include "Logger.h"

#define DEFAULT_DEVICE_LOCATION "/dev/video0"

GMainLoop* g_loop = NULL;

static void InterruptSignalHandler(int signal)
{
    LOG_INFO("Get Ctrl+C signal %d. Please wait...\n", signal);
    std::_Exit(EXIT_SUCCESS);
}

static std::string GetEnvAndCheck(const char* name, const char *defaultValue = "")
{
    LOG_ASSERT(name && strlen(name));
    const char* str = getenv(name);
    if(str == NULL)
    {
        LOG_INFO("Env: %s not set. U|sing Default value %s.", name, defaultValue);
        return defaultValue;
    }
    else if(!strlen(str))
    {
        LOG_INFO("Env: %s empty.", name);
        return defaultValue;
    }
    
    LOG_INFO("Env: %s set to %s", name, str);

    return str;
}

int main(int argc, char *argv[]) 
{
    (void)argc;
    (void)argv;

    signal(SIGINT, InterruptSignalHandler);

    LOG_INFO("Starting app...");

    // create loop
    g_loop = g_main_loop_new(NULL, FALSE);
    g_assert(g_loop);

    auto deviceLocation = GetEnvAndCheck("DEVICE_LOCATION", DEFAULT_DEVICE_LOCATION);
    Pipeline pipeline(deviceLocation);
    if(!pipeline.Open())
    {
        LOG_INFO("Failed open pipeline\n");
        return EXIT_FAILURE;
    }

    LOG_INFO("starting loop");
    g_main_loop_run(g_loop);
    LOG_INFO("stopped loop");

    g_main_loop_unref(g_loop);
    g_loop = NULL;

    LOG_INFO("Stopping app...");

    return EXIT_SUCCESS;
}
