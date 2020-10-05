# Freenect Green Screen
**Status:** In development :smile:

## About
Freenect Green Screen is a utility to use your Kinect device's (Gen1) depth sensors capabilities to only show the image of the person in front of it, simulating a green screen effect behind them. It captures the image with the Kinect VGA webcam and writes the edited frame in the virtual webcam provided by v4l2loopback module.

**As it makes use of Linux features, it's not portable to other platforms in its current state, as far as I know.**

### Current and planned features
- [X] **Virtual webcam:** create a virtual device seen as a webcam by your OS with the Kinect's VGA image;
- [ ] **Green Screen:** replace your background with an image;
- [ ] **Facial recognition:** to keep user's face always in frame, when possible;
- [ ] **Runtime controls:** adjust Kinect's angle, background image and turn on/off facial recognition while it's running;
- [ ] **Improved Green Screen:** replace your background with a video/gif;

## How to use it
### Requirements
- [libfreenect](https://github.com/OpenKinect/libfreenect)
- [OpenCV](https://opencv.org/)
- [v4l2loopback](https://github.com/umlaeute/v4l2loopback)

### Build
In the repository's root folder, run:
```shell
$ cmake . -DCMAKE_BUILD_TYPE=Release -Bbuild
$ cmake --build build/
```

### Running
In the repository's root folder, run:
```shell
$ build/bin/freenect-green-screen -h
```

## License
Freenect Green Screen is released under the [MIT License](https://github.com/mrtheduts/FreenectGreenScreen/blob/main/LICENSE).
