/*
 * Copyright 2024 Valve Corporation
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "libagx.h"

enum libagx_tess_partitioning {
   LIBAGX_TESS_PARTITIONING_FRACTIONAL_ODD,
   LIBAGX_TESS_PARTITIONING_FRACTIONAL_EVEN,
   LIBAGX_TESS_PARTITIONING_INTEGER,
};

enum libagx_tess_mode {
   /* Do not actually tessellate, just write the index counts */
   LIBAGX_TESS_MODE_COUNT,

   /* Tessellate using the count buffers to allocate indices */
   LIBAGX_TESS_MODE_WITH_COUNTS,
};

struct libagx_tess_point {
   uint32_t u;
   uint32_t v;
};
AGX_STATIC_ASSERT(sizeof(struct libagx_tess_point) == 8);

struct libagx_tess_args {
   /* Heap to allocate tessellator outputs in */
   GLOBAL(struct agx_geometry_state) heap;

   /* Patch coordinate buffer, indexed as:
    *
    *    coord_allocs[patch_ID] + vertex_in_patch
    */
   GLOBAL(struct libagx_tess_point) patch_coord_buffer;

   /* Per-patch index within the heap for the tess coords, written by the
    * tessellator based on the allocated memory.
    */
   GLOBAL(uint32_t) coord_allocs;

   /* Space for output draws from the tessellator. API draw calls. */
   GLOBAL(uint32_t) out_draws;

   /* Tessellation control shader output buffer. */
   GLOBAL(float) tcs_buffer;

   /* Count buffer. # of indices per patch written here, then prefix summed. */
   GLOBAL(uint32_t) counts;

   /* Allocated index buffer for all patches, if we're prefix summing counts */
   GLOBAL(uint32_t) index_buffer;

   /* Address of the tess eval invocation counter for implementing pipeline
    * statistics, if active. Zero if inactive. Incremented by tessellator.
    */
   GLOBAL(uint32_t) statistic;

   /* Address of the tess control invocation counter for implementing pipeline
    * statistics, if active. Zero if inactive. Incremented by indirect tess
    * setup kernel.
    */
   GLOBAL(uint32_t) tcs_statistic;

   /* For indirect draws with tessellation, the grid sizes. VS then TCS then
    * tess. Allocated by the CPU and written by the tessellation
    * setup indirect kernel.
    */
   GLOBAL(uint32_t) grids;

   /* For indirect draws, the output input assembly descriptor */
   GLOBAL(struct agx_ia_state) ia;

   /* For indirect draws, the indirect draw descriptor. */
   GLOBAL(uint32_t) indirect;

   /* For indirect draws, the allocation for the vertex buffer.
    *
    * TODO: We could move these fields to an indirect setup kernel, not sure if
    * it's worth it though...
    */
   GLOBAL(uint64_t) vertex_output_buffer_ptr;

   /* Yet more indirect draw garbage. I need a refactor. */
   uint64_t in_index_buffer;
   uint32_t in_index_buffer_range_el;
   uint32_t in_index_size_B;

   /* When geom+tess used together, the buffer containing TES outputs (executed
    * as a hardware compute shader).
    */
   uint64_t tes_buffer;

   /* For indirect draws, the bitfield of VS outputs */
   uint64_t vertex_outputs;

   /* Bitfield of TCS per-vertex outputs */
   uint64_t tcs_per_vertex_outputs;

   /* Default tess levels used in OpenGL when there is no TCS in the pipeline.
    * Unused in Vulkan and OpenGL ES.
    */
   float tess_level_outer_default[4];
   float tess_level_inner_default[2];

   /* Number of vertices in the input patch */
   uint32_t input_patch_size;

   /* Number of vertices in the TCS output patch */
   uint32_t output_patch_size;

   /* Number of patch constants written by TCS */
   uint32_t tcs_patch_constants;

   /* Number of input patches per instance of the VS/TCS */
   uint32_t patches_per_instance;

   /* Stride between tessellation facotrs in the TCS output buffer. */
   uint32_t tcs_stride_el;

   /* Number of patches being tessellated */
   uint32_t nr_patches;

   /* Partitioning and points mode. These affect per-patch setup code but not
    * the hot tessellation loop so we make them dynamic to reduce tessellator
    * variants.
    */
   enum libagx_tess_partitioning partitioning;
   uint32_t points_mode;

   /* When fed into a geometry shader, triangles should be counter-clockwise.
    * The tessellator always produces clockwise triangles, but we can swap
    * dynamically in the TES.
    */
   uint32_t ccw;
} PACKED;
AGX_STATIC_ASSERT(sizeof(struct libagx_tess_args) == 51 * 4);
