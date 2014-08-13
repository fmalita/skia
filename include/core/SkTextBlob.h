/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTextBlob_DEFINED
#define SkTextBlob_DEFINED

#include "SkPaint.h"
#include "SkRect.h"
#include "SkRefCnt.h"
#include "SkTDArray.h"

class SkBaseDevice;
class SkDraw;
class SkPoint;

class SK_API SkTextChunk {
public:
    static SkTextChunk* Create(const uint16_t* glyphs, size_t count, const SkPaint& paint,
                               const SkRect* bounds = NULL);
    static SkTextChunk* Create(const uint16_t* glyphs, size_t count, const SkScalar* pos,
                               const SkPaint& paint, const SkRect* bounds = NULL);
    static SkTextChunk* Create(const uint16_t* glyphs, size_t count, const SkPoint* pos,
                               const SkPaint& paint, const SkRect* bounds = NULL);

    ~SkTextChunk();

    void draw(SkBaseDevice*, const SkDraw&, const SkPaint&) const;

    const SkRect& bounds() const;

private:
    SkTextChunk(const uint16_t* glyphs, size_t count, const SkPaint& paint, const SkRect* bounds);
    SkTextChunk(const uint16_t* glyphs, size_t count, const SkScalar* pos, const SkPaint& paint,
                const SkRect* bounds);
    SkTextChunk(const uint16_t* glyphs, size_t count, const SkPoint* pos, const SkPaint& paint,
                const SkRect* bounds);

    void init(const uint16_t*, size_t, const SkRect*, const SkScalar*);

    friend class SkPictureRecord;

    size_t         fGlyphCount;
    uint16_t*      fGlyphs;
    SkScalar*      fPos;  // FIXME: merge glyphs/pos storage?

    SkPaint        fFont; // FIXME: SkFont

    mutable SkRect fBounds;
    mutable bool   fBoundsDirty   : 1;

    unsigned       fScalarsPerPos : 2;
};

class SK_API SkTextBlob : public SkRefCnt {
public:
    static SkTextBlob* Create(SkTextChunk* chunk);
    static SkTextBlob* Create(const SkTDArray<SkTextChunk*>& chunks);

    ~SkTextBlob();

    class SK_API Iter {
    public:
        Iter(const SkTextBlob* blob) : fBlob(blob), fIndex(0) { SkASSERT(blob); }
        const SkTextChunk* next() {
            return (fIndex < fBlob->fChunks.count()) ? fBlob->fChunks[fIndex++] : NULL;
        }

    private:
        const SkTextBlob* fBlob;
        int fIndex;
    };

private:
    SkTextBlob(const SkTDArray<SkTextChunk*>& chunks);

    SkTDArray<SkTextChunk*>  fChunks;
};

class SK_API SkTextBlobBuilder {
public:
    SkTextBlobBuilder();
    ~SkTextBlobBuilder();

    void addChunk(SkTextChunk* chunk);

    // FIXME: should we support glyph-wise ops? Not sure whether this is interesting
    // for clients at this point -- they likely have a better idea of chunking.
    /*
    void addGlyph(uint16_t glyph, const SkPaint& font, const SkRect* bounds = NULL);
    void addGlyph(uint16_t glyph, const SkScalar pos, const SkPaint& font,
                  const SkRect* bounds = NULL);
    void addGlyph(uint16_t glyph, const SkPoint pos, const SkPaint& font,
                  const SkRect* bounds = NULL);
    */

    const SkTextBlob* build();

private:

    SkTDArray<SkTextChunk*>  fChunks;
};

#endif // SkTextBlob_DEFINED
