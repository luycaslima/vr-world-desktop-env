# World OS - Vr Desktop Environment

Custom Desktop Environment for my ARM OS for Custom VR set

Needs to adapt [rlxr](https://github.com/caszuu/rlxr/tree/main) to support native_mode , linux and monado


VR OS Architecture & Hardware Considerations

When migrating away from a general-purpose OS (like Raspberry Pi OS Lite) to a dedicated VR environment, pay attention to these core pillars:

Input & Sensor Fusion (IMUs)
Head tracking is the most critical component. A standard Linux kernel polling an I2C sensor often introduces micro-stutters (jitter) that lead straight to motion sickness.

    Instead of raw IMUs like the MPU6050, use sensors with hardware-level sensor fusion (like the BNO085) to offload the quaternion math from your SBC.

    For even better deterministic timing, you can offload the 1000Hz+ sensor polling to a dedicated microcontroller (like an ESP32 or RP2040). The MCU handles the raw I2C polling and feeds clean, filtered rotational data to your Pi/Radxa via a high-speed SPI or Serial connection.

Kernel & Latency (Motion-to-Photon)
Standard Linux task schedulers try to be fair to all processes, which is bad for VR. If a background task preempts your rendering thread for even 5 milliseconds, the user will feel it.

    You will want to patch your SBC's kernel with PREEMPT_RT (Real-Time). This allows you to set your Raylib thread to a real-time priority, ensuring your rendering and tracking loops are never interrupted by system noise [1.2.3].

Compositing "Apps" Inside the Environment
You mentioned running apps inside your Raylib VR environment. Because you are taking exclusive control of the screen via DRM, standard Linux apps (like a web browser or terminal) cannot draw to the screen.

    You will need to implement a mechanism to render external applications to off-screen textures (e.g., using Wayland protocols via wlroots, or rendering embedded browser instances like CEF/Ultralight directly to Raylib RenderTexture2D targets) and then map those textures onto 3D quads in your VR space.

Stripping the OS (Buildroot / Yocto)
When you are ready to drop Raspberry Pi OS Lite, look into Buildroot. It allows you to compile an absolutely minimal Linux image from source. You can configure it to include only the Linux kernel, the DRM/Mesa graphics drivers, and your executable. A well-configured Buildroot image can boot your hardware directly into your Raylib compositor in under 5 seconds


### OPENXR Support

If you just use Raylib's built-in 3D camera capabilities with two viewports, you are effectively just strapping a monitor to your face. True VR requires asynchronous reprojection (to prevent nausea when frames drop), predictive 6DOF head tracking, and—most importantly—barrel distortion and chromatic aberration correction to counteract the physical curvature of the headset lenses. OpenXR runtimes handle all of this complex math for you.

Here is what you need to know about setting up rlxr, the Linux OpenXR stack, and how overlay sessions fit into your "VR OS" architecture.
1. The Linux OpenXR Stack: Enter Monado

On Windows, SteamVR or the Oculus App acts as the OpenXR runtime. On Linux, the open-source standard runtime is Monado [1.2.1].

Because you are building a custom OS and bypassing a desktop environment (PLATFORM=DRM), the architecture looks like this:

    Monado (The Compositor): Monado takes exclusive control of your Pi/Radxa HDMI output via DRM/KMS [1.2.1, 1.2.2]. It reads the hardware sensors, applies the lens distortion, and pushes frames to the screen.

    OpenXR Loader (libopenxr_loader.so): This sits between your app and Monado [1.2.4].

    Your Raylib App (via rlxr): Your app renders the 3D scene (using EGL/OpenGL ES) and hands the framebuffers to Monado via the OpenXR API.

2. The Catch with rlxr on Linux (and how to fix it)

The caszuu/rlxr project is a great single-header wrapper [1.1.1], but if you look at its documentation, there is a major catch for your specific use case: it currently only supports Linux via Xlib (X11) [1.1.1].

Because you are compiling Raylib for native DRM, you don't have X11. If you try to compile rlxr as-is, it will fail looking for X11 graphics bindings.

To make rlxr work on your bare-metal setup, you will need to open rlxr.h and patch the graphics binding initialization. You must change the Linux graphics binding from XR_USE_PLATFORM_XLIB_KHR to XR_USE_PLATFORM_EGL_KHR. Raylib's DRM mode uses EGL to manage its OpenGL context, so you must pass that EGL context to OpenXR so Monado can access your rendered frames.

Setup Requirements for Linux/EndeavourOS:

    Install the Monado runtime (monado or monado-git on Arch/EndeavourOS).

    Install the OpenXR SDK loader (openxr).

    In your CMake file, you will use find_package(OpenXR REQUIRED) and link OpenXR::openxr_loader to your target, alongside rlxr.


## Raspberry Setup

ssh connection 
```sh
ssh l22l@192.168.*.*
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
ssh-copy-id -i ~/.ssh/id_ed25519.pub user@192.168.*.*

```

If crashes use this command to kill
```
ssh pi@192.168.1.50 "killall -9 game; fbset -depth 8; fbset -depth 16"
```