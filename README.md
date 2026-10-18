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

Accepted firmware source: `cae1a9bf392177d4660361ae6ff4a87ba74f1e18`.
AXF SHA-256:
`42607084c72c512379d3eaef14bcb9f7f7c93ec6c84cc2de7b4dc794bdb46fc9`.

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
