/*
 * GStreamer
 * Copyright (C) 2010 Thiago Santos <thiago.sousa.santos@collabora.co.uk>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Alternatively, the contents of this file may be used under the
 * GNU Lesser General Public License Version 2.1 (the "LGPL"), in
 * which case the following provisions apply instead of the ones
 * mentioned above:
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/*
 * Thanks to Jerry Huxtable <http://www.jhlabs.com> work on its java
 * image editor and filters. The algorithms here were extracted from
 * his code.
 */


#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gst/gst.h>
#include <math.h>

#include "gsttwirl.h"

GST_DEBUG_CATEGORY_STATIC (gst_twirl_debug);
#define GST_CAT_DEFAULT gst_twirl_debug

enum
{
  PROP_0,
  PROP_ANGLE
};

#define DEFAULT_ANGLE 0

GST_BOILERPLATE (GstTwirl, gst_twirl, GstCircleGeometricTransform,
    GST_TYPE_CIRCLE_GEOMETRIC_TRANSFORM);

static void
gst_twirl_set_property (GObject * object, guint prop_id, const GValue * value,
    GParamSpec * pspec)
{
  GstTwirl *twirl;

  twirl = GST_TWIRL_CAST (object);

  switch (prop_id) {
    case PROP_ANGLE:
      twirl->angle = g_value_get_double (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gst_twirl_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec)
{
  GstTwirl *twirl;

  twirl = GST_TWIRL_CAST (object);

  switch (prop_id) {
    case PROP_ANGLE:
      g_value_set_double (value, twirl->angle);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

/* Clean up */
static void
gst_twirl_finalize (GObject * obj)
{
  G_OBJECT_CLASS (parent_class)->finalize (obj);
}

/* GObject vmethod implementations */

static void
gst_twirl_base_init (gpointer gclass)
{
  GstElementClass *element_class = GST_ELEMENT_CLASS (gclass);

  gst_element_class_set_details_simple (element_class,
      "twirl",
      "Transform/Effect/Video",
      "Applies 'twirl' geometric transform to the image",
      "Thiago Santos<thiago.sousa.santos@collabora.co.uk>");
}

static gboolean
twirl_map (GstGeometricTransform * gt, gint x, gint y, gdouble * in_x,
    gdouble * in_y)
{
  GstCircleGeometricTransform *cgt = GST_CIRCLE_GEOMETRIC_TRANSFORM_CAST (gt);
  GstTwirl *twirl = GST_TWIRL_CAST (gt);
  gdouble distance;
  gdouble dx, dy;

  dx = x - cgt->precalc_x_center;
  dy = y - cgt->precalc_y_center;
  distance = dx * dx + dy * dy;

  if (distance > cgt->precalc_radius2) {
    *in_x = x;
    *in_y = y;
  } else {
    gdouble d = sqrt (distance);
    gdouble a = atan2 (dy, dx) + twirl->angle * (cgt->radius - d) / cgt->radius;

    *in_x = cgt->precalc_x_center + d * cos (a);
    *in_y = cgt->precalc_y_center + d * sin (a);

    *in_x = CLAMP (*in_x, 0, gt->width - 1);
    *in_y = CLAMP (*in_y, 0, gt->height - 1);
  }

  GST_DEBUG_OBJECT (twirl, "Inversely mapped %d %d into %lf %lf",
      x, y, *in_x, *in_y);

  return TRUE;
}

static void
gst_twirl_class_init (GstTwirlClass * klass)
{
  GObjectClass *gobject_class;
  GstGeometricTransformClass *gstgt_class;

  gobject_class = (GObjectClass *) klass;
  gstgt_class = (GstGeometricTransformClass *) klass;

  parent_class = g_type_class_peek_parent (klass);

  gobject_class->finalize = GST_DEBUG_FUNCPTR (gst_twirl_finalize);
  gobject_class->set_property = GST_DEBUG_FUNCPTR (gst_twirl_set_property);
  gobject_class->get_property = GST_DEBUG_FUNCPTR (gst_twirl_get_property);

  g_object_class_install_property (gobject_class, PROP_ANGLE,
      g_param_spec_double ("angle", "angle",
          "angle in radians of the twirl effect",
          -G_MAXDOUBLE, G_MAXDOUBLE, DEFAULT_ANGLE,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  gstgt_class->map_func = twirl_map;
}

static void
gst_twirl_init (GstTwirl * filter, GstTwirlClass * gclass)
{
  filter->angle = DEFAULT_ANGLE;
}

gboolean
gst_twirl_plugin_init (GstPlugin * plugin)
{
  GST_DEBUG_CATEGORY_INIT (gst_twirl_debug, "twirl", 0, "twirl");

  return gst_element_register (plugin, "twirl", GST_RANK_NONE, GST_TYPE_TWIRL);
}
