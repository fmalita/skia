
#include "SkTextBlob.h"

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

const SkTextBlob* SkTextBlob::Create(SkTextChunk *chunk) {
    SkTDArray<SkTextChunk*> chunks;
    chunks.setReserve(1);
    *chunks.append() = chunk;
    return SkNEW_ARGS(SkTextBlob, (chunks));
}

SkTextBlob::SkTextBlob(SkTDArray<SkTextChunk*>& chunks)
    : fChunks(chunks) {
}

SkTextBlobBuilder::SkTextBlobBuilder() {
    reset();
}

SkTextBlobBuilder::~SkTextBlobBuilder() {
    // Free any unused chunks.
    for (int i = 0; i < fChunks.count(); ++i) {
        sk_free(fChunks[i]);
    }
}

void SkTextBlobBuilder::reset() {
    fChunks.rewind();
    resetPendingChunk();
}

const SkTextBlob* SkTextBlobBuilder::build() {
    commitPendingChunk();

    const SkTextBlob* blob = SkNEW_ARGS(SkTextBlob, (fChunks));
    reset();

    return blob;
}

void SkTextBlobBuilder::addChunk(SkTextChunk* chunk) {
    // FIXME: attempt to merge?
    commitPendingChunk();

    *fChunks.append() = chunk;
}

void SkTextBlobBuilder::addGlyph(uint16_t glyph, const SkPaint& font, const SkRect* bounds) {
    convertPendingPositioning(SkTextChunk::kNone_Positioning);

    // FIXME: more granular font check
    if (fPendingFont != font) {
        commitPendingChunk();
        fPendingFont = font;
    }

    updatePendingBounds(bounds);
    *fPendingGlyphs.append() = glyph;
}

void SkTextBlobBuilder::addGlyph(uint16_t glyph, const SkScalar pos, const SkPaint& font,
                                 const SkRect* bounds) {
    convertPendingPositioning(SkTextChunk::kScalar_Positioning);

    // FIXME: more granular font check
    if (fPendingFont != font) {
        commitPendingChunk();
        fPendingFont = font;
    }

    updatePendingBounds(bounds);

    *fPendingGlyphs.append() = glyph;
    *fPendingHPos.append() = pos;

    SkASSERT(fPendingGlyphs.count() == fPendingHPos.count());
}

void SkTextBlobBuilder::addGlyph(uint16_t glyph, const SkPoint pos, const SkPaint& font,
                                 const SkRect* bounds) {
    convertPendingPositioning(SkTextChunk::kPoint_Positioning);

    // FIXME: more granular font check
    if (fPendingFont != font) {
        commitPendingChunk();
        fPendingFont = font;
    }

    updatePendingBounds(bounds);

    *fPendingGlyphs.append() = glyph;
    *fPendingPos.append() = pos;

    SkASSERT(fPendingGlyphs.count() == fPendingPos.count());
}

void SkTextBlobBuilder::updatePendingBounds(const SkRect* bounds) {
    fPendingBoundsValid &= (NULL != bounds);
    if (fPendingBoundsValid) {
        fPendingBounds.join(*bounds);
    }
}

void SkTextBlobBuilder::convertPendingPositioning(SkTextChunk::Positioning positioning) {
    if (fPendingPositioning == positioning) {
        return;
    }

    // FIXME: actually convert
    commitPendingChunk();

    fPendingPositioning = positioning;
}

void SkTextBlobBuilder::commitPendingChunk() {
    if (!hasPending()) {
        SkASSERT(fPendingPos.isEmpty());
        SkASSERT(fPendingHPos.isEmpty());
        SkASSERT(fPendingBounds.isEmpty());
        SkASSERT(fPendingBoundsValid);
        return;
    }

    SkTextChunk* chunk = NULL;
    const SkRect* chunkBounds = fPendingBoundsValid ? &fPendingBounds : NULL;

    switch (fPendingPositioning) {
    case SkTextChunk::kNone_Positioning:
        SkASSERT(fPendingPos.isEmpty());
        SkASSERT(fPendingHPos.isEmpty());
        chunk = SkTextChunk::Create(fPendingGlyphs.begin(), fPendingGlyphs.count(),
                                    fPendingFont, chunkBounds);
        break;
    case SkTextChunk::kScalar_Positioning:
        SkASSERT(fPendingPos.isEmpty());
        chunk = SkTextChunk::Create(fPendingGlyphs.begin(), fPendingGlyphs.count(),
                                    fPendingHPos.begin(), fPendingFont, chunkBounds);
        break;
    case SkTextChunk::kPoint_Positioning:
        SkASSERT(fPendingHPos.isEmpty());
        chunk = SkTextChunk::Create(fPendingGlyphs.begin(), fPendingGlyphs.count(),
                                    fPendingPos.begin(), fPendingFont, chunkBounds);
        break;
    default:
        SkFAIL("unhandled positioning value");
    }

    SkASSERT(NULL != chunk);
    *fChunks.append() = chunk;

    resetPendingChunk();
}

void SkTextBlobBuilder::resetPendingChunk() {
    fPendingGlyphs.rewind();
    fPendingHPos.rewind();
    fPendingPos.rewind();

    fPendingPositioning = SkTextChunk::kNone_Positioning;
    fPendingBounds.setEmpty();
    fPendingBoundsValid = true;
}
