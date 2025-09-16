# tmini_plus_emulator
Serial LiDAR emulator (T-mini Plus)

## Requirements
- CMake >= 3.10 and a C++ compiler
- socat (for virtual serial port)
- ydlidar_ros2_driver (for ROS2 node)

## Build
```bash
git clone https://github.com/Ar-Ray-code/tmini_plus_emulator.git
mkdir -p tmini_plus_emulator/linux/build
cd tmini_plus_emulator/linux/build

cmake ..
make -j$(nproc)
./tmini_plus_emulator
```

## Usage

### Terminal 1
Setup a virtual serial port pair

```bash
socat -d -d pty,raw,echo=0,link=/tmp/ttyEMUL pty,raw,echo=0,link=/tmp/ttyROS
```

### Terminal 2 (Emulator side)

Run the emulator ([example_data.txt](data/example_data.txt) is provided)

```bash
cd <path to build directory>
./tmini_plus_emulator /tmp/ttyEMUL ../../data/example_data.txt

## If you want to stream the data repeatedly, use the --stream option
# ./tmini_plus_emulator /tmp/ttyEMUL ./tmini_plus_emulator/data/example_data.txt --stream
```

### Terminal 3 (YDLidar ROS side)

Setup the [ydlidar_ros](https://github.com/YDLIDAR/ydlidar_ros2_driver) repo and run the ROS2 node

[lidar_params.yaml](config/lidar_params.yaml) is provided, make sure to set the `port` parameter to `/tmp/ttyROS`

```bash
source <path to your ros2 workspace>/install/setup.bash
ros2 launch ydlidar_ros2_driver ydlidar_launch.py params_file:=<path to lidar_params.yaml>
```

### Terminal 4 (View the data)
Open RViz2 and add a LaserScan display, setting the topic to `/scan`

```bash
source /opt/ros/<ros2_distro>/setup.bash

rviz2
```
<img width="885" height="572" alt="Screenshot from 2025-09-16 18-45-19" src="https://github.com/user-attachments/assets/1053105b-7d19-450f-9854-ebad7b881f96" />


## References

[YDLDIAR T-MINI PLUS DEVELOPMENT MANUAL](https://cdn.soselectronic.com/productdata/1f/ec/c83edf83/t-mini-plus.pdf)


