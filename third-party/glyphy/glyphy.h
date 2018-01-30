/*
 * Copyright 2012 Google, Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Google Author(s): Behdad Esfahbod, Maysum Panju
 */

#ifndef GLYPHY_H
#define GLYPHY_H

#include <dali/devel-api/text-abstraction/text-abstraction-definitions.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int glyphy_bool_t;

typedef struct {
  double x;
  double y;
} glyphy_point_t;

/*
 * Geometry extents
 */

typedef struct {
  double min_x;
  double min_y;
  double max_x;
  double max_y;
} glyphy_extents_t;

void
glyphy_extents_clear (glyphy_extents_t *extents);

glyphy_bool_t
glyphy_extents_is_empty (const glyphy_extents_t *extents);

void
glyphy_extents_add (glyphy_extents_t     *extents,
                    const glyphy_point_t *p);

void
glyphy_extents_extend (glyphy_extents_t       *extents,
                       const glyphy_extents_t *other);

glyphy_bool_t
glyphy_extents_includes (const glyphy_extents_t *extents,
                         const glyphy_point_t   *p);

void
glyphy_extents_scale (glyphy_extents_t *extents,
                      double            x_scale,
                      double            y_scale);

/*
 * Circular arcs
 */

typedef struct {
  glyphy_point_t p0;
  glyphy_point_t p1;
  double d;
} glyphy_arc_t;

/*
 * Approximate outlines with multiple arcs
 */

typedef struct {
  glyphy_point_t p;
  double d;
} glyphy_arc_endpoint_t;

typedef glyphy_bool_t (*glyphy_arc_endpoint_accumulator_callback_t) (glyphy_arc_endpoint_t *endpoint,
                                                                     void                  *user_data);

typedef struct glyphy_arc_accumulator_t glyphy_arc_accumulator_t;

glyphy_arc_accumulator_t *
glyphy_arc_accumulator_create (void);

void
glyphy_arc_accumulator_destroy (glyphy_arc_accumulator_t *acc);

void
glyphy_arc_accumulator_reset (glyphy_arc_accumulator_t *acc);

/* Configure accumulator */

void
glyphy_arc_accumulator_set_tolerance (glyphy_arc_accumulator_t *acc,
                                      double                    tolerance);

double
glyphy_arc_accumulator_get_tolerance (glyphy_arc_accumulator_t *acc);

void
glyphy_arc_accumulator_set_callback (glyphy_arc_accumulator_t *acc,
                                     glyphy_arc_endpoint_accumulator_callback_t callback,
                                     void                     *user_data);

void
glyphy_arc_accumulator_get_callback (glyphy_arc_accumulator_t  *acc,
                                     glyphy_arc_endpoint_accumulator_callback_t *callback,
                                     void                     **user_data);

/* Accumulation results */

double
glyphy_arc_accumulator_get_error (glyphy_arc_accumulator_t *acc);

glyphy_bool_t
glyphy_arc_accumulator_successful (glyphy_arc_accumulator_t *acc);


/* Accumulate */

void
glyphy_arc_accumulator_move_to (glyphy_arc_accumulator_t *acc,
                                const glyphy_point_t *p0);

void
glyphy_arc_accumulator_line_to (glyphy_arc_accumulator_t *acc,
                                const glyphy_point_t *p1);

void
glyphy_arc_accumulator_conic_to (glyphy_arc_accumulator_t *acc,
                                 const glyphy_point_t *p1,
                                 const glyphy_point_t *p2);

void
glyphy_arc_accumulator_cubic_to (glyphy_arc_accumulator_t *acc,
                                 const glyphy_point_t *p1,
                                 const glyphy_point_t *p2,
                                 const glyphy_point_t *p3);

void
glyphy_arc_accumulator_arc_to (glyphy_arc_accumulator_t *acc,
                               const glyphy_point_t *p1,
                               double                d);

void
glyphy_arc_accumulator_close_path (glyphy_arc_accumulator_t *acc);

void
glyphy_arc_list_extents (const glyphy_arc_endpoint_t *endpoints,
                         unsigned int                 num_endpoints,
                         glyphy_extents_t            *extents);

/*
 * Modify outlines for proper consumption
 */

void
glyphy_outline_reverse (glyphy_arc_endpoint_t *endpoints,
                        unsigned int           num_endpoints);

/* Returns true if outline was modified */
glyphy_bool_t
glyphy_outline_winding_from_even_odd (glyphy_arc_endpoint_t *endpoints,
                                      unsigned int           num_endpoints,
                                      glyphy_bool_t          inverse);

/*
 * Encode an arc outline into binary blob for fast SDF calculation
 */

typedef Dali::TextAbstraction::VectorBlob glyphy_rgba_t;

glyphy_bool_t
glyphy_arc_list_encode_blob (const glyphy_arc_endpoint_t *endpoints,
                             unsigned int                 num_endpoints,
                             glyphy_rgba_t               *blob,
                             unsigned int                 blob_size,
                             double                       faraway,
                             double                       avg_fetch_desired,
                             double                      *avg_fetch_achieved,
                             unsigned int                *output_len,
                             unsigned int                *nominal_width,  /* 6bit */
                             unsigned int                *nominal_height, /* 6bit */
                             glyphy_extents_t            *extents);

/*
 * Calculate signed-distance-field from (encoded) arc list
 */

double
glyphy_sdf_from_arc_list (const glyphy_arc_endpoint_t *endpoints,
                          unsigned int                 num_endpoints,
                          const glyphy_point_t        *p,
                          glyphy_point_t              *closest_p /* may be NULL; TBD not implemented yet */);

double
glyphy_sdf_from_blob (const glyphy_rgba_t  *blob,
                      unsigned int          nominal_width,
                      unsigned int          nominal_height,
                      const glyphy_point_t *p,
                      glyphy_point_t       *closest_p /* may be NULL; TBD not implemented yet */);

#ifdef __cplusplus
}
#endif

#endif /* GLYPHY_H */
