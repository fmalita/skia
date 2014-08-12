#ifndef SkTextBlob_DEFINED
#define SkTextBlob_DEFINED

#include "SkPaint.h"
#include "SkRect.h"
#include "SkRefCnt.h"
#include "SkScalar.h"

class SkPoint;

class SK_API SkTextBlob : public SkRefCnt {
public:
    static SkTextBlob* Create(const void* text, size_t textLen, const SkPaint& paint,
        const SkRect* knownBounds = NULL);
    static SkTextBlob* Create(const void* text, size_t textLen, const SkPaint& paint,
        const SkScalar pos[], const SkRect* knownBounds = NULL);
    static SkTextBlob* Create(const void* text, size_t textLen, const SkPaint& paint,
        const SkPoint pos[], const SkRect* knownBounds = NULL);

    ~SkTextBlob();

    void draw(SkCanvas*, const SkPaint&) const;
    const SkRect& bounds() const;

private:
    explicit SkTextBlob(const void* text, size_t textLen, const SkPaint& paint,
                        const SkRect* knownBounds);
    explicit SkTextBlob(const void* text, size_t textLen, const SkPaint& paint,
               const SkScalar pos[], const SkRect* knownBounds);
    explicit SkTextBlob(const void* text, size_t textLen, const SkPaint& paint,
               const SkPoint pos[], const SkRect* knownBounds);

    void init_common(const void* text, size_t textLen, const SkPaint& paint,
                     const SkRect* knownBounds);

    enum Positioning {
        kNone_Positioning,
        kScalar_Positioning,
        kPoint_Positioning
    };

    void*   fText;
    size_t  fTextLen;
    SkPaint fPaint;

    union {
        SkPoint*  fPos;
        SkScalar* fHPos;
    };

    mutable SkRect fCachedBounds;
    mutable bool   fCachedBoundsValid : 1;

    Positioning    fPositioning : 2;

};

#endif // SkTextBlob_DEFINED
