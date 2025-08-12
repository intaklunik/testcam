# TestCam

TestCam is a Linux kernel driver that implements a simple V4L2 virtual video capture device. The driver generates a horizontal text animation over a static background and exposes it to userspace as /dev/videoN.

The driver uses the *videobuf2* framework with *MMAP* streaming.

## Features
Currently TestCam supports only 1 format.  
- Resolution: 120x160  
- Pixel format: RGB565  
- Frame rate: 10 FPS

## Build
Build
```bash
make
```
Clean build files
```bash
make clean
```

## Load / Unload Module
TestCam requires preloaded modules:
```bash
sudo modprobe videobuf2_v4l2
sudo modprobe videobuf2_vmalloc
```
Load module:  
```bash
sudo insmod testcam.ko
```
Unload module:  
```bash
sudo rmmod testcam
```
Check registered video device:
```bash
ls /dev/video*
ls /sys/devices/virtual/video4linux
```

## Tested Environment
x86_64 Ubuntu 24.04.1 Kernel 6.14.0-27-generic

## Usage / Tests
- Display device information
```bash
v4l2-ctl -d /dev/videoN --all
```
- Start videostreaming using FFmpeg
```bash
ffplay -f v4l2 -framerate 10 -video_size 120x160 /dev/videoN
```
- Cheese
```bash
sudo cheese -d /dev/videoN
```
- Also tested on Teams and Telegram video calls.

## Notes
- Designed for testing V4L2 driver concepts, not for production use.  
- During development, parts of the *vivid* and *uvcvideo* drivers from the Linux kernel were used as references.  


