# World OS - Vr Desktop Environment

Custom Desktop Environment for my ARM OS for Custom VR set


### Linux Execution and Testing

Requires OpenXR, Monado and Wivrn, xfvb setted up

For debbuging create a .desktop file so Wivrn can find the build easily and execute it

add also `export XR_RUNTIME_JSON=/usr/share/openxr/1/openxr_wivrn.json` in your .bashrc so every session will find it 

```
[Desktop Entry]
Type=Application
Name=Raylib VR Test Build
Exec=~/vr-world-desktop-env/build/desktop/game
Icon=applications-games
Terminal=false
Categories=X-WiVRn-VR;Game; #The first tag is important for Wivrn to find
```

>To the Intellisense find the SDL3/SDL_openxr.h tell clangd to restart the server 


---

### VR OS Architecture & Hardware Considerations

Core pillars for the project:

Initially thought for Raspberry pi 4, migrating to a Radxa Dragon Q6A

Input & Sensor Fusion (IMUs)
Head tracking is the most critical component. A standard Linux kernel polling an I2C sensor often introduces micro-stutters (jitter) that lead straight to motion sickness.

    Instead of raw IMUs like the MPU6050, use sensors with hardware-level sensor fusion (like the BNO085) to offload the quaternion math from the SBC.

    For even better deterministic timing, it can offload the 1000Hz+ sensor polling to a dedicated microcontroller (like an ESP32 or RP2040). The MCU handles the raw I2C polling and feeds clean, filtered rotational data to your Pi/Radxa via a high-speed SPI or Serial connection.

Kernel & Latency (Motion-to-Photon)
Standard Linux task schedulers try to be fair to all processes, which is bad for VR. If a background task preempts the rendering thread for even 5 milliseconds, the user will feel it.

    Patch the SBC's kernel with PREEMPT_RT (Real-Time). This allows  to set the Application thread to a real-time priority, ensuring the rendering and tracking loops are never interrupted by system noise [1.2.3].

Compositing "Apps" Inside the Environment
For running apps inside VR environment. As It needs taking exclusive control of the screen via DRM, standard Linux apps (like a web browser or terminal) cannot draw to the screen.

    Implement a mechanism to render external applications to off-screen textures (e.g., using Wayland protocols via wlroots, or rendering embedded browser instances like CEF/Ultralight directly to Raylib RenderTexture2D targets) and then map those textures onto 3D quads in your VR space.

Stripping the OS (Buildroot / Yocto)
After dropping Raspberry Pi OS Lite, use Buildroot. It allows to compile an absolutely minimal Linux image from source. Configure it to include only the Linux kernel, the DRM/Mesa graphics drivers, and  executable. A well-configured Buildroot image can boot the hardware directly into the App compositor in under 5 seconds (hopefully)

--- 
### OPENXR Support

> Migration made from Raylib rlxr to SDL3 with full OpenXR support


## SBC/Raspberry/Radxa Setup

ssh connection 
```sh
ssh user@192.*.*.*
```

Allow video input on a OS Lite

```sh
# Add your user account to the video and input hardware groups
sudo usermod -aG video,input $USER

# Ensure your package list has the necessary system display development files
sudo apt update
sudo apt install libdrm-dev libgbm-dev libegl1-mesa-dev libgles2-mesa-dev
```

Setup the passthrough compilation

- Generate a SshKey without password
```
ssh-keygen -t ed25519 -C "vr-development-key"
```

- Copy to the raspberry pi
```
ssh-copy-id -i ~/.ssh/id_ed25519.pub user@192.*.*.*

```

If crashes use this command to kill
```
ssh pi@192.*.*.* "killall -9 game; fbset -depth 8; fbset -depth 16"
```
