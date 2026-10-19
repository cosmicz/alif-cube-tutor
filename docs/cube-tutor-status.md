# Cube Tutor: build and validation handoff

Recorded October 18, 2026. This distinguishes source integration, programming
evidence and hardware acceptance; they are not interchangeable.

## Firmware identities

| Revision | Change | Evidence |
| --- | --- | --- |
| `2c2e56e5` | Bounding boxes, face memory and observation prompts; no 3D wireframe | Flashed; framebuffer inspected; saved profile verified |
| `96eea1ed` | Require three identical reads before remembering a face | Build succeeded; flashed through the logged gateway |
| `c7e9187b` | Add bounded white-cross next-move planner | Build succeeded; flashed through the logged gateway; host planner tests supplied |

Latest recorded firmware source: `c7e9187bcecd6ce59198e365b76c970607a22bc3`.
AXF SHA-256: `da4f21a386af15e526f72fbecbeeb040b8d21b07c998fc02fad4b61ace25afde`.

Rollback bounding-box AXF SHA-256:
`e97290ad2187b72944c37248ec01b61d4314bd9c514e13d3ab0e8bef0b069894`.

The table calibration was accepted by the operator. An earlier demo build
passed a physical unplug/replug check. The latest planner's six-face guidance
has not been independently acceptance-tested on the board in this closeout.
No firmware was flashed as part of README/merge cleanup.

## Model artifacts

| Artifact | Bytes | Role |
| --- | ---: | --- |
| `last.pt` | 6,458,090 | Training checkpoint, 3,138,003 parameters, one `cube_face` class |
| `last_int8.tflite` | 3,434,485 | Quantized detector before Vela |
| `last_int8_vela.tflite` | 3,033,904 | Deployed TFLM/Ethos-U85 detector |
| Separate sticker-centroid `.pte` | 14,480 | ExecuTorch experiment, not this firmware's colour reader |

Deployed detector SHA-256:
`34c208766b526c42746ad4c5b87afb0cb39e0b77e86f6a2e8bdaa50a05ba60f7`.

Input is white-balanced RGB, float32 NCHW `[1,3,192,192]`, normalized to 0–1.
Output `[1,5,756]` holds normalized box centres, widths, heights and scores.
The exported model has quantize, one `ethos-u` custom graph and dequantize
operators. Its SRAM estimate is 771.515625 KiB; this excludes other application
buffers. Vela's **11.87 ms / 84.2 inferences per second is a compiler estimate**,
not measured end-to-end FPS. The colour-read diagnostic counter can increment
once per selected face, so it cannot by itself establish camera FPS.

## Building for the board

Use the upstream [build guide](sections/building.md) for the toolchain,
submodule setup and resource generation. The demo was built with GNU Arm
13.3.Rel1 and the following configuration:

- Alif AppKit E8, high-performance Cortex-M55, Ethos-U85-256.
- `alif_object_detection` use case, TensorFlow Lite Micro runtime.
- OV5675 camera, console UART4, `ROTATE_DISPLAY=90`.
- LVGL software rendering; camera rotation and mirroring are separate from
  the display rotation.
- The custom Vela detector above, not the inherited human-face model.

Custom model weights, generated model source and firmware binaries are not
distributed in this repository. The public source alone does not reproduce
the demo artifact without those inputs; no one-command board build is claimed.

**Preserve the recovered boot configuration.** On the demo E8, file-backed
application LOADs must stay in `[0x80200000, 0x8057c000)`. The final 16 KiB
is reserved for boot metadata. The latest image ends at `0x805659c0`, leaving
91,712 bytes before that reservation. Do not erase the application/boot area
or assume a successful debugger load establishes a working cold boot.

## Tests and remaining checks

Run `bash tests/run_cube_tests.sh`; add `SANITIZE=1` for ASan/UBSan. The suite
covers camera mapping, four display rotations, button debounce, colour
sampling, face selection, observation prompts, stable memory and cross search
over 20 deterministic scrambles. It needs only a host C11/C++17 toolchain.
This is the complete standalone Cube Tutor suite, not the upstream MLEK
FVP/model integration suite, which needs additional resources.

The old single-face hint test asserted removed turn/alignment text. It now
checks the actual observation-only contract and rejects inferred turn claims.
The planner is tested separately because it consumes all six remembered faces.

### Known colour defect—not an accuracy pass

The retained raw table fixture has reference labels `BYRGRYRRR`; the shipped
classifier returns `BYRGOYRRR`. Its table-specific orange gate labels a
confirmed red sample as orange. The test now explicitly characterizes that
one mismatch **while retaining the original reference labels**. Passing the
host suite does not mean this colour defect has been corrected. Gloss,
lighting changes and red/orange overlap still need a broader labelled dataset.

### Planner validation still needed

The planner assumes correctly oriented, mutually consistent face reads. It
undoes the preview mirror and searches up to five turns to home another
white-cross edge without losing already-home edges. The current scan does
not enforce complete cube legality or verify that a physical turn occurred.
Old face reads can remain after a move. Worst-case search latency under the
display lock and a full six-face hardware rehearsal remain follow-up work.

## Branch disposition

Stable-memory and next-move source are integrated with their host tests.
The separate ExecuTorch runner, experimental localisers and calibration-NVM
work remain separate; merging them would replace runtime paths or introduce
unaccepted storage/layout changes. Private recordings, credentials and
flash-service endpoints are not published.
