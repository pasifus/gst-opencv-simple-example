# Example using gstreamer with opencv for c++ developer

1. Capture from a webcam device
2. Manipulate the image (rotate 180)
3. Visualize “latency” (ts + pts)
4. Render to display

Example also include:
- cmake build
- vscode container develop

## Run in docker with display support
docker run -it --net=host -e DISPLAY -v $HOME/.Xauthority:/root/.Xauthority --device=/dev/dri -e DEVICE_LOCATION=/dev/video0 gst-opencv-simple-example bash

# desting x-display support
gst-launch-1.0 videotestsrc ! autovideosink

## Tested on platfor
- Linux (ubuntu 16/18)
- MacOS (develop only)
