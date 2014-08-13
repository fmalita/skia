/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTextBlob.h"

#include "SkCanvas.h"

SkTextChunk* SkTextChunk::Create(const uint16_t* glyphs, size_t count, const SkPaint& paint,
                                 const SkRect* bounds) {
    if (NULL == glyphs || 0 == count) {
        return NULL;
    }

    return SkNEW_ARGS(SkTextChunk, (glyphs, count, paint, bounds));
}

SkTextChunk* SkTextChunk::Create(const uint16_t* glyphs, size_t count, const SkScalar* pos,
                                 const SkPaint& paint, const SkRect* bounds) {
    if (NULL == glyphs || NULL == pos || 0 == count) {
        return NULL;
    }

    return SkNEW_ARGS(SkTextChunk, (glyphs, count, pos, paint, bounds));
}

SkTextChunk* SkTextChunk::Create(const uint16_t* glyphs, size_t count, const SkPoint* pos,
                                 const SkPaint& paint, const SkRect* bounds) {
    if (NULL == glyphs || NULL == pos || 0 == count) {
        return NULL;
    }

    return SkNEW_ARGS(SkTextChunk, (glyphs, count, pos, paint, bounds));
}

SkTextChunk::SkTextChunk(const uint16_t *glyphs, size_t count, const SkPaint &paint,
                         const SkRect* bounds)
    : fFont(paint)
    , fPositioning(kNone_Positioning) {

    SkASSERT(SkPaint::kGlyphID_TextEncoding == paint.getTextEncoding());
    init(glyphs, count, bounds);
}

SkTextChunk::SkTextChunk(const uint16_t *glyphs, size_t count, const SkScalar* pos,
                         const SkPaint &paint, const SkRect* bounds)
    : fFont(paint)
    , fPositioning(kScalar_Positioning) {

    SkASSERT(SkPaint::kGlyphID_TextEncoding == paint.getTextEncoding());
    init(glyphs, count, bounds);

    SkASSERT(NULL != pos);
    size_t storageSize = sizeof(SkScalar) * count;
    fPosH = static_cast<SkScalar*>(sk_malloc_throw(storageSize));
    memcpy(fPosH, pos, storageSize);
}

SkTextChunk::SkTextChunk(const uint16_t *glyphs, size_t count, const SkPoint* pos,
                         const SkPaint &paint, const SkRect* bounds)
    : fFont(paint)
    , fPositioning(kPoint_Positioning) {

    SkASSERT(SkPaint::kGlyphID_TextEncoding == paint.getTextEncoding());
    init(glyphs, count, bounds);

    SkASSERT(NULL != pos);
    size_t storageSize = sizeof(SkPoint) * count;
    fPos = static_cast<SkPoint*>(sk_malloc_throw(storageSize));
    memcpy(fPos, pos, storageSize);
}

SkTextChunk::~SkTextChunk() {
    sk_free(fGlyphs);

    if (kNone_Positioning != fPositioning) {
        SkASSERT(NULL != fPos);
        sk_free(fPos);
    }
}

void SkTextChunk::init(const uint16_t* glyphs, size_t count, const SkRect* bounds) {
    SkASSERT(glyphs);
    SkASSERT(count > 0);

    size_t storageSize = sizeof(*glyphs) * count;

    fGlyphCount = count;
    fGlyphs = static_cast<uint16_t*>(sk_malloc_throw(storageSize));
    memcpy(fGlyphs, glyphs, storageSize);

    fBoundsDirty = NULL == bounds;
    if (!fBoundsDirty) {
        fBounds = *bounds;
    }
}

const SkRect& SkTextChunk::bounds() const {
    if (fBoundsDirty) {
        fFont.measureText(fGlyphs, fGlyphCount, &fBounds);
        fBoundsDirty = false;
    }

    return fBounds;
}

void SkTextChunk::draw(SkCanvas *canvas, const SkPaint& paint) const {
    SkASSERT(NULL != canvas);
    SkASSERT(SkPaint::kGlyphID_TextEncoding == paint.getTextEncoding());

    size_t length = sizeof(uint16_t) * fGlyphCount;
    switch (fPositioning) {
    case kNone_Positioning:
        canvas->drawText(fGlyphs, length, 0, 0, paint);
        break;
    case kScalar_Positioning:
        canvas->drawPosTextH(fGlyphs, length, fPosH, 0, paint);
        break;
    case kPoint_Positioning:
        canvas->drawPosText(fGlyphs, length, fPos, paint);
        break;
    default:
        SkFAIL("unhandled positioning value");
    }
}

SkTextBlob* SkTextBlob::Create(SkTextChunk *chunk) {
    SkTDArray<SkTextChunk*> chunks;
    chunks.setReserve(1);
    *chunks.append() = chunk;
    return SkNEW_ARGS(SkTextBlob, (chunks));
}

SkTextBlob::SkTextBlob(SkTDArray<SkTextChunk*>& chunks)
    : fChunks(chunks) {
}
SkTextBlob::~SkTextBlob() {
    for (int i = 0; i < fChunks.count(); ++i) {
        SkDELETE(fChunks[i]);
    }
}

void SkTextBlob::draw(SkCanvas* canvas, const SkPaint& paint) const {
    for (int i = 0; i < fChunks.count(); ++i) {
        fChunks[i]->draw(canvas, paint);
    }
}

SkTextBlobBuilder::SkTextBlobBuilder() {
    reset();
}

SkTextBlobBuilder::~SkTextBlobBuilder() {
    // Free any unused chunks.
    for (int i = 0; i < fChunks.count(); ++i) {
        SkDELETE(fChunks[i]);
    }
}

void SkTextBlobBuilder::reset() {
    fChunks.rewind();
}

const SkTextBlob* SkTextBlobBuilder::build() {
    const SkTextBlob* blob = SkNEW_ARGS(SkTextBlob, (fChunks));
    reset();

    return blob;
}

void SkTextBlobBuilder::addChunk(SkTextChunk* chunk) {
    *fChunks.append() = chunk;
}


