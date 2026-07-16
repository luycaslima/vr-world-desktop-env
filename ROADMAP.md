# ROADMAP


## Steps of development


### 1. Initial Phase - Os Prototyping 
- [ ] Desktop Mode mode Application (Initial Tests)
  - [X] Change raylib to sdl3 for better support 
    - [ ] Adapt functions to work as raylib style/idiomatic as groundbase
      - [ ] Load Shader,Load Mesh, Draw in batch, Do Perspective Camera
    - [ ] ECS
    - [ ] EventBus
    - [ ] 3D Rendering
      - [ ] Shader Batch Pipeline
    - [ ] Physics Integration
  - [X] OpenXR + Monado + Wivrn Support (Debug Purporses with the Oculus quest 1)
    - [ ] Insert Xfvb method to open applications VR 
    - [ ] Wayland Server to open appls in VR


### 2. Physical Device creation
- Hardware Setup 
  - Screen , glasses, lenses ,IMU, cameras , microcontrollers
  - 3d Printing
  - **MAIN** reference: [link](https://github.com/dmcke5/DIY_VR) 
  - IMU support [link](https://github.com/LiquidCGS/FastIMU) 
  - Controllers

### 3. Arm based Setup and support

- Obtain Radxa Dragon Q6A (8gb preferably)
  
- Embedded System Support
  - OpenXR Support Compilation
  - Os Optimization


- Native mode Application (Bare metal to the OS)
  - OpenXR + Monado Support
  - Wayland Server Support




