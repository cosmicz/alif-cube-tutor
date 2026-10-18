# Cube Tutor on Alif E8

A camera-grounded Rubik's Cube tutor: hold up a scrambled cube, and the board
locates a visible face, reads its nine sticker colours and displays
observation-based prompts alongside a live, mirrored preview. No solved
starting position or cloud connection is required.

## How it works

1. The **OV5675 camera** captures RGB frames; the preview is rotated, cropped
   and mirrored for natural interaction.
2. A **YOLOv8 cube-face detector** processes white-balanced 192×192 RGB input.
   PyTorch/Ultralytics training, INT8 export and Arm Vela compilation target
   the **Ethos-U85**, using **TensorFlow Lite Micro** in the final firmware.
3. A **C colour reader on the Cortex-M55** samples a 3×3 grid within each
   selected face box, applies white balance and classifies sticker colours.
4. **LVGL** displays the preview, boxes, colour letters and short prompts
   about visible white-edge stickers. Uncertain colours are rejected.

The entire live perception-and-feedback loop runs on the board. The laptop
is used for training, flashing and diagnostics, not live inference.

## Read the code

- [Inference and display loop](source/app/use_case/alif_object_detection/src/UseCaseHandler.cc)
- [Face selection and suppression](source/app/use_case/alif_object_detection/include/CubeFaceMultiSelect.hpp)
- [Sticker reader and colour classification](source/hal/source/components/camera/source/alif/source/cube_preview.h)
- [Camera adapter and saved calibration](source/hal/source/components/camera/source/alif/source/hal_camera_alif.c)
- [Observation prompts](source/app/use_case/alif_object_detection/include/cube_cross_hint.h)
- [Regression tests](tests/cube_preview)

## Demo status

Live camera preview, full-face detection and calibrated sticker recognition
were demonstrated on the E8. The operator accepted the table calibration on
October 18, 2026. That named profile is compiled into startup; later
button-triggered recalibration is runtime-only.

Current firmware source: `2c2e56e593c71afd88446dfa6621015f8463218b`.
AXF SHA-256:
`e97290ad2187b72944c37248ec01b61d4314bd9c514e13d3ab0e8bef0b069894`.

The current display uses ordinary detection bounding boxes, colour labels
and face memory. The experimental 3D wireframe and arrow overlay was removed;
the detector and saved table calibration are unchanged.

This is a perception-and-feedback prototype, **not a complete cube solver**.
Six-face state reconstruction and visual move verification are next.
Multi-face decoding is implemented but not acceptance-demonstrated on
hardware. Colour boundaries are tuned to the demonstration table; no
lighting-independent accuracy claim is made.

Known host-test failures in the accepted source: the older raw-colour
fixture expects `BYRGRYRRR` but the table-specific classifier returns
`BYRGOYRRR`; the cross-hint test still expects removed move/alignment claims.
These are not presented as passing regressions. Publishing this repository
does not change the accepted board firmware.

We also exercised a **separate ExecuTorch sticker-classification prototype**
on the E8, but its colour accuracy was not accepted. The final detector in
this repository uses **TFLM, not ExecuTorch**. The separate runner is not
included here.

## Building and provenance

This application builds on [npow/alif-mlek-yolo](https://github.com/npow/alif-mlek-yolo)
and the Arm/Alif ML Embedded Evaluation Kit. Existing history, source
notices and licences are preserved; see [License.txt](License.txt),
[Apache licence](LICENSE_APACHE_2.0.txt) and dependency-specific terms.

The [MLEK guide](ML_Embedded_Evaluation_Kit.md) describes toolchain and
dependency setup. Our demo uses OV5675, UART4, display rotation 90°,
Ethos-U85-256 and LVGL software rendering. The inherited camera guide
describes the original human-face model, **not the Cube Tutor model**.

Custom model weights and firmware binaries are not yet included: this is a
source repository, not a ready-to-flash package. Do not substitute the
base project's default human-face model. Private camera recordings and
flashing credentials are excluded.

Hackathon submission: [Arm-Examples/CMSIS-Executorch PR #37](https://github.com/Arm-Examples/CMSIS-Executorch/pull/37).
