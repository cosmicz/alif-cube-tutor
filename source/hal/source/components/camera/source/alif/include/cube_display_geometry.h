#ifndef CUBE_DISPLAY_GEOMETRY_H
#define CUBE_DISPLAY_GEOMETRY_H

#ifndef CUBE_DISPLAY_ROTATION
#define CUBE_DISPLAY_ROTATION 0
#endif

/* Camera pixels and overlays share LVGL's logical canvas. The LCD scanout
 * remains physical panel size; lv_port rotates the complete composition. */
#if CUBE_DISPLAY_ROTATION == 90 || CUBE_DISPLAY_ROTATION == 270
#define CUBE_PREVIEW_WIDTH RTE_PANEL_VACTIVE_LINE
#define CUBE_PREVIEW_HEIGHT RTE_PANEL_HACTIVE_TIME
#else
#define CUBE_PREVIEW_WIDTH RTE_PANEL_HACTIVE_TIME
#define CUBE_PREVIEW_HEIGHT RTE_PANEL_VACTIVE_LINE
#endif
#define CUBE_STATUS_Y ((CUBE_PREVIEW_HEIGHT - 240) / 2 + 270)

#endif
