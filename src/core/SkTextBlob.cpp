#include "SkTextBlob.h"

#include "SkCanvas.h"
#include "SkPaint.h"
#include "SkPoint.h"
#include "SkTypes.h"

SkTextBlob* SkTextBlob::Create(const void* text, size_t textLen, const SkPaint& paint,
    const SkRect* knownBounds) {
    if (NULL == text || 0 == textLen) {
        return NULL;
    }

    return SkNEW_ARGS(SkTextBlob, (text, textLen, paint, knownBounds));
}

SkTextBlob* SkTextBlob::Create(const void* text, size_t textLen, const SkPaint& paint,
    const SkScalar pos[], const SkRect* knownBounds) {
    if (NULL == text || 0 == textLen) {
        return NULL;
    }

    SkASSERT(pos);
    return SkNEW_ARGS(SkTextBlob, (text, textLen, paint, pos, knownBounds));
}

SkTextBlob* SkTextBlob::Create(const void* text, size_t textLen, const SkPaint& paint,
    const SkPoint pos[], const SkRect* knownBounds) {
    if (NULL == text || 0 == textLen) {
        return NULL;
    }

    SkASSERT(pos);
    return SkNEW_ARGS(SkTextBlob, (text, textLen, paint, pos, knownBounds));
}

SkTextBlob::SkTextBlob(const void* text, size_t textLen, const SkPaint& paint,
    const SkRect* knownBounds)
    : fPaint(paint)
    , fPositioning(kNone_Positioning) {

    init_common(text, textLen, paint, knownBounds);
}

SkTextBlob::SkTextBlob(const void* text, size_t textLen, const SkPaint& paint,
    const SkScalar pos[], const SkRect* knownBounds)
    : fPaint(paint)
    , fPositioning(kScalar_Positioning) {

    init_common(text, textLen, paint, knownBounds);

    size_t posSize = sizeof(SkScalar) * paint.textToGlyphs(text, textLen, NULL);
    SkASSERT(posSize > 0);

    fHPos = static_cast<SkScalar*>(sk_malloc_throw(posSize));
    memcpy(fHPos, pos, posSize);
}

SkTextBlob::SkTextBlob(const void* text, size_t textLen, const SkPaint& paint,
    const SkPoint pos[], const SkRect* knownBounds)
    : fPaint(paint)
    , fPositioning(kPoint_Positioning) {

    init_common(text, textLen, paint, knownBounds);

    size_t posSize = sizeof(SkPoint) * paint.textToGlyphs(text, textLen, NULL);
    SkASSERT(posSize > 0);

    fPos = static_cast<SkPoint*>(sk_malloc_throw(posSize));
    memcpy(fPos, pos, posSize);
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

const SkRect& SkTextBlob::bounds() const {
    if (!fCachedBoundsValid) {
        fPaint.measureText(fText, fTextLen, &fCachedBounds);
        fCachedBoundsValid = true;
    }

    return fCachedBounds;
}

void SkTextBlob::draw(SkCanvas* canvas, const SkPaint& paint) const {
    SkASSERT(canvas);
    SkASSERT(fText);
    SkASSERT(fTextLen);

    switch (fPositioning) {
    case kNone_Positioning:
        canvas->drawText(fText, fTextLen, 0, 0, paint);
        break;
    case kScalar_Positioning:
        canvas->drawPosTextH(fText, fTextLen, fHPos, 0, paint);
        break;
    case kPoint_Positioning:
        canvas->drawPosText(fText, fTextLen, fPos, paint);
        break;
    default:
        SkFAIL("unhandled positioning value");
    }
}
