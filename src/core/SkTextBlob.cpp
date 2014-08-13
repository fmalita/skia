/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTextBlob.h"

#include "SkDevice.h"

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
    : fPos(NULL)
    , fFont(paint)
    , fScalarsPerPos(0) {

    SkASSERT(SkPaint::kGlyphID_TextEncoding == paint.getTextEncoding());
    init(glyphs, count, bounds, NULL);
}

SkTextChunk::SkTextChunk(const uint16_t *glyphs, size_t count, const SkScalar* pos,
                         const SkPaint &paint, const SkRect* bounds)
    : fPos(NULL)
    , fFont(paint)
    , fScalarsPerPos(1) {

    SkASSERT(SkPaint::kGlyphID_TextEncoding == paint.getTextEncoding());
    init(glyphs, count, bounds, pos);
}

SkTextChunk::SkTextChunk(const uint16_t *glyphs, size_t count, const SkPoint* pos,
                         const SkPaint &paint, const SkRect* bounds)
    : fPos(NULL)
    , fFont(paint)
    , fScalarsPerPos(2) {

    SkASSERT(SkPaint::kGlyphID_TextEncoding == paint.getTextEncoding());
    init(glyphs, count, bounds, (const SkScalar*)pos);
}

SkTextChunk::~SkTextChunk() {
    sk_free(fGlyphs);
    sk_free(fPos);
}

void SkTextChunk::init(const uint16_t* glyphs, size_t count, const SkRect* bounds,
                       const SkScalar* pos) {
    SkASSERT(glyphs);
    SkASSERT(count > 0);

    size_t storageSize = sizeof(uint16_t) * count;

    fGlyphCount = count;
    fGlyphs = static_cast<uint16_t*>(sk_malloc_throw(storageSize));
    memcpy(fGlyphs, glyphs, storageSize);

    fBoundsDirty = NULL == bounds;
    if (!fBoundsDirty) {
        fBounds = *bounds;
    }

    if (NULL != pos) {
        size_t posStorageSize = sizeof(SkScalar) * fScalarsPerPos * count;
        fPos = static_cast<SkScalar*>(sk_malloc_throw(posStorageSize));
        memcpy(fPos, pos, posStorageSize);
    }
}

const SkRect& SkTextChunk::bounds() const {
    if (fBoundsDirty) {
        fFont.measureText(fGlyphs, fGlyphCount, &fBounds);
        fBoundsDirty = false;
    }

    return fBounds;
}

void SkTextChunk::draw(SkBaseDevice* device, const SkDraw& draw, const SkPaint& paint) const {
    SkASSERT(NULL != device);
    SkASSERT(SkPaint::kGlyphID_TextEncoding == paint.getTextEncoding());

    size_t length = sizeof(uint16_t) * fGlyphCount;
    if (NULL != fPos) {
        SkASSERT(1 == fScalarsPerPos || 2 == fScalarsPerPos);
        device->drawPosText(draw, fGlyphs, length, fPos, 0, fScalarsPerPos, paint);
    } else {
        SkASSERT(0 == fScalarsPerPos);
        device->drawText(draw, fGlyphs, length, 0, 0, paint);
    }
}

SkTextBlob* SkTextBlob::Create(SkTextChunk *chunk) {
    SkTDArray<SkTextChunk*> chunks;
    chunks.setReserve(1);
    *chunks.append() = chunk;
    return SkNEW_ARGS(SkTextBlob, (chunks));
}

SkTextBlob* SkTextBlob::Create(const SkTDArray<SkTextChunk*>& chunks) {
    return SkNEW_ARGS(SkTextBlob, (chunks));
}

SkTextBlob::SkTextBlob(const SkTDArray<SkTextChunk*>& chunks)
    : fChunks(chunks) {
}

SkTextBlob::~SkTextBlob() {
    Iter it(this);
    while (const SkTextChunk* chunk = it.next()) {
        SkDELETE(chunk);
    }
}

SkTextBlobBuilder::SkTextBlobBuilder() {
}

SkTextBlobBuilder::~SkTextBlobBuilder() {
    // unused chunks.
    for (int i = 0; i < fChunks.count(); ++i) {
        SkDELETE(fChunks[i]);
    }
}

const SkTextBlob* SkTextBlobBuilder::build() {
    const SkTextBlob* blob = SkTextBlob::Create(fChunks);
    fChunks.rewind();
    return blob;
}

void SkTextBlobBuilder::addChunk(SkTextChunk* chunk) {
    *fChunks.append() = chunk;
}


