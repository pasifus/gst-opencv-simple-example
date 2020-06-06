# Example using gstreamer with opencv for c++ developer

1. Capture from a webcam device
2. Manipulate the image (rotate 180)
3. Visualize “latency” (ts + pts)
4. Render to display

Example also include:
- cmake build
- vscode container develop

## Run in docker with display support
docker run -it --net=host -e DISPLAY -v $HOME/.Xauthority:/root/.Xauthority -e DEVICE_LOCATION=/dev/video0 gst-opencv-simple-example

## Tested on platfor
- Linux (ubuntu 16/18)
- MacOS (develop only)