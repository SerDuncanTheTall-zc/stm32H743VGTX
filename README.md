# STM32H743VGTX High-Performance Projects

![STM32H7](https://img.shields.io/badge/MCU-STM32H743VGT6-blue.svg)
![Clock](https://img.shields.io/badge/Clock-480MHz-red.svg)
![Compiler](https://img.shields.io/badge/Compiler-GCC%20(CubeIDE)-orange.svg)
![License](https://img.shields.io/badge/License-MIT-green.svg)

本项目致力于探索 **STM32H743VGT6** (Cortex-M7) 的极致性能，涵盖了从底层驱动适配到高性能实时任务处理的完整实践。

## 📌 设计架构：分支即项目

为了保持代码库的纯净与模块化，本项目采用 **“单功能单分支”** 的开发模式：
* **`master` 分支**：作为项目的中央导航枢纽与索引。
* **各 Feature 分支**：包含完整的独立工程配置、驱动源码及演示文档。

---

## 📦 已发布模块 (Featured Branches)

### 🔧 driver/sg90-hcsr04
**雷达式超声波避障系统**。模拟雷达扫描逻辑，通过舵机带动传感器进行 180° 环境探测。

* **技术要点**：
    * 基于 **TIM PWM** 驱动 SG90 伺服电机。
    * 利用 **输入捕获 (Input Capture)** 实现 HC-SR04 高精度脉冲测距。
    * 逻辑层与硬件层高度解耦，便于移植。

* **快速拉取代码**：
    ```bash
    git clone -b driver/sg90-hcsr04 --single-branch [https://github.com/SerDuncanTheTall-zc/stm32H743VGTX.git](https://github.com/SerDuncanTheTall-zc/stm32H743VGTX.git)
    ```

* **演示视频**：
    * [📺 YouTube Video](https://youtu.be/yUmKvmhgVbc?si=SUmeEauqsGHOwMXn)
    * [📺 Bilibili 演示](https://www.bilibili.com/video/BV14YAEzWEvY/)

---

### 📷 硬件展示 (Hardware Setup)

<div align="center">
  <img src="./doc/sg90_Ultrasonic.jpg" width="45%" alt="Hardware Setup 1">
  <img src="./doc/stm32H743VGTX.jpg" width="45%" alt="Hardware Setup 2">
</div>

---

## 🚀 路线图 (Future Roadmap)

后续将陆续迁移并开源以下模块：

- [ ] **master**：高性能 DCMI + DMA 图像采集与 LCD 实时渲染。


---

## ⚠️ 技术说明 (Technical Notes)

基于 **STM32H7** 系列的高性能特性，在编译和使用本项目时请注意：

1.  **开发环境**：推荐使用 **STM32CubeIDE** (GCC Toolchain)。
2.  **内存管理**：务必关注 `Linker Script (.ld)` 中的内存分配，尤其是 **DTCM/SRAM** 的布局。
3.  **缓存一致性**：由于开启了 **D-Cache**，在处理 DMA（如 DCMI 或 ADC）数据时，请务必使用 `SCB_InvalidateDCache_by_Addr` 确保数据一致性。
4.  **MPU 配置**：外设缓冲区（Buffer）所在的内存区域需正确配置 MPU 属性（如 Non-cacheable）。

---

## 👤 作者 (Author)

**zhangchao (jojochao)**

---
© 2026 zhangchao. Licensed under the MIT License.