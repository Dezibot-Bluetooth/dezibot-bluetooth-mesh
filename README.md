# Dezibot Bluetooth Mesh

ESP32 Bluetooth Mesh (BLE Mesh) project for DeziBot devices.

This repository contains:
- An ESP-IDF application (`src/`) built with the Arduino component.
- The `dezibot_bluetooth_mesh` component (`components/dezibot_bluetooth_mesh/`) providing the mesh stack wrapper APIs.
- Ready-to-copy examples (`examples/`) that you can drop into `src/`.

## Installation

Prerequisites:
```shell
mkdir ~/esp
cd ~/esp
git clone -b v5.5.1 --recursive https://github.com/espressif/esp-idf.git
cd ~/esp/esp-idf
./install.sh

# in the terminal where you need the esp-idf
. $HOME/esp/esp-idf/export.sh
```

## Scripts

Before you run any scripts or `idf.py` commands, set up the environment using:

```shell
. $HOME/esp/esp-idf/export.sh
```

Helper scripts live in `scripts/` and use a dedicated build directory (`cmake-build-idf/`).

```sh
./scripts/clean  # fullclean + reconfigure
./scripts/build  # build
./scripts/flash  # flash + monitor
```

Notes:
- If you need to specify a serial port, run `idf.py` directly:
  `idf.py -B cmake-build-idf -p /dev/ttyUSB0 flash monitor`
- To generate API docs, run: `doxygen`
  - to view the generated docs, open `docs/html/index.html` in a web browser

## Run An Example

Examples are stored in `examples/`. To run one, copy its files into `src/` and build/flash.

Client-only example:
```sh
cp examples/ClientOnly/ClientOnly.cpp src/main.cpp
./scripts/build
./scripts/flash
```

Server-only example:
```sh
cp examples/ServerOnly/ServerOnly.cpp src/main.cpp
cp examples/ServerOnly/server_composition.* src/
./scripts/build
./scripts/flash
```
