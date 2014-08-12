#include "SkTextBlob.h"

#include "SkPaint.h"
#include "SkPoint.h"
#include "SkTypes.h"

const SkTextBlob* SkTextBlob::Create(const void* text, size_t textLen, const SkPaint& paint,
    const SkRect* knownBounds) {
    return SkNEW_ARGS(SkTextBlob, (text, textLen, paint, knownBounds));
}

const SkTextBlob* SkTextBlob::Create(const void* text, size_t textLen, const SkPaint& paint,
    const SkScalar pos[], const SkRect* knownBounds) {
    return SkNEW_ARGS(SkTextBlob, (text, textLen, paint, pos, knownBounds));
}

SkTextBlob::SkTextBlob(const void* text, size_t textLen, const SkPaint& paint,
    const SkRect* knownBounds)
    : fPositioning(kNone_Positioning) {

    init_common(text, textLen, paint, knownBounds);
}

SkTextBlob::SkTextBlob(const void* text, size_t textLen, const SkPaint& paint,
    const SkScalar pos[], const SkRect* knownBounds)
    : fPositioning(kScalar_Positioning) {

    init_common(text, textLen, paint, knownBounds);

    size_t posSize = sizeof(SkScalar) * paint.textToGlyphs(text, textLen, NULL);
    fHPos = static_cast<SkScalar*>(sk_malloc_throw(posSize));
    memcpy(fHPos, pos, posSize);
}

void SkTextBlob::init_common(const void* text, size_t textLen, const SkPaint& paint,
    const SkRect* knownBounds) {

    fTextLen = textLen;
    fText = sk_malloc_throw(textLen);
    memcpy(fText, text, textLen);

    fCachedBoundsValid = NULL != knownBounds;
    if (fCachedBoundsValid) {
        fCachedBounds = *knownBounds;
    }
}

SkTextBlob::~SkTextBlob() {
    if (kNone_Positioning != fPositioning) {
        SkASSERT(NULL != fPos);
        sk_free((void*)fPos);
    }
}
