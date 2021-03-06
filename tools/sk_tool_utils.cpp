/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "sk_tool_utils.h"
#include "sk_tool_utils_flags.h"

#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkTestScalerContext.h"

DEFINE_bool(portableFonts, false, "Use portable fonts");
DEFINE_bool(resourceFonts, false, "Use resource fonts");

namespace sk_tool_utils {

const char* colortype_name(SkColorType ct) {
    switch (ct) {
        case kUnknown_SkColorType:      return "Unknown";
        case kAlpha_8_SkColorType:      return "Alpha_8";
        case kIndex_8_SkColorType:      return "Index_8";
        case kARGB_4444_SkColorType:    return "ARGB_4444";
        case kRGB_565_SkColorType:      return "RGB_565";
        case kRGBA_8888_SkColorType:    return "RGBA_8888";
        case kBGRA_8888_SkColorType:    return "BGRA_8888";
        default:
            SkASSERT(false);
            return "unexpected colortype";
    }
}

SkTypeface* create_portable_typeface(const char* name, SkTypeface::Style style) {
    SkTypeface* face;
    if (FLAGS_portableFonts) {
        face = create_font(name, style);
    } else if (FLAGS_resourceFonts) {
        face = resource_font(name, style);
    } else {
        face = SkTypeface::CreateFromName(name, style);
    }
    return face;
}

void set_portable_typeface(SkPaint* paint, const char* name, SkTypeface::Style style) {
    SkTypeface* face = create_portable_typeface(name, style);
    SkSafeUnref(paint->setTypeface(face));
}

void write_pixels(SkCanvas* canvas, const SkBitmap& bitmap, int x, int y,
                  SkColorType colorType, SkAlphaType alphaType) {
    SkBitmap tmp(bitmap);
    tmp.lockPixels();

    SkImageInfo info = tmp.info();
    info.fColorType = colorType;
    info.fAlphaType = alphaType;

    canvas->writePixels(info, tmp.getPixels(), tmp.rowBytes(), x, y);
}

}  // namespace sk_tool_utils
