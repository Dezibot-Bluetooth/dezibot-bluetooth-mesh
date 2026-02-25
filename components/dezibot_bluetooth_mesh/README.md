# Dezibot Bluetooth Mesh Component

ESP-IDF component that implements the DeziBot Bluetooth Mesh (BLE Mesh) stack and exposes an Arduino-style C++ wrapper.

## What It Contains

- Core mesh logic split into submodules:
  - `common/` (shared includes + init helpers)
  - `client/` (client models / sending messages)
  - `server/` (server models / receiving messages + event dispatch)
  - `provisioner/` (optional: provisioner role)
- Public wrapper API:
  - `DeziBotMesh` (Arduino-style C++ wrapper): `components/dezibot_bluetooth_mesh/DeziBotMesh.h`
- Support headers used by the wrapper:
  - C/C++ boundary header to call C implementation from C++: `components/dezibot_bluetooth_mesh/ble_mesh_bridge.h`

## Wrapper APIs

### Arduino C++: `DeziBotMesh`

Main class: `DeziBotMesh` (header: `DeziBotMesh.h`)

Minimal client example:
```cpp
#include <Arduino.h>
#include <DeziBotMesh.h>

DeziBotMesh mesh;

void setup() {
  mesh.init();
  mesh.beginClient();

  // broadcast to all nodes
  mesh.sendOnOff(true, 0xFFFF);
}

void loop() {
  delay(1000);
}
```

Minimal server example (logs incoming OnOff Set):
```cpp
#include <Arduino.h>
#include <DeziBotMesh.h>
#include <server/server_events.h>

DeziBotMesh mesh;

static void onEvt(const mesh_server_evt_t *evt) {
  if (!evt) return;
  if (evt->type == MESH_EVT_ONOFF_SET) {
    // evt->onoff_set.onoff is 0/1
  }
}

void setup() {
  mesh.init();
  mesh.beginServer(onEvt);
}

void loop() {
  delay(10);
}
```

### C/C++ Boundary: `ble_mesh_bridge.h`

`ble_mesh_bridge.h` is used to make the C implementation callable from the C++ wrapper.
It is primarily an internal detail used by `DeziBotMesh` to invoke the underlying C functions.
