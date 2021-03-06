/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkScaledImageCache_DEFINED
#define SkScaledImageCache_DEFINED

#include "SkBitmap.h"

class SkDiscardableMemory;
class SkMipMap;

/**
 *  Cache object for bitmaps (with possible scale in X Y as part of the key).
 *
 *  Multiple caches can be instantiated, but each instance is not implicitly
 *  thread-safe, so if a given instance is to be shared across threads, the
 *  caller must manage the access itself (e.g. via a mutex).
 *
 *  As a convenience, a global instance is also defined, which can be safely
 *  access across threads via the static methods (e.g. FindAndLock, etc.).
 */
class SkScaledImageCache {
public:
    struct ID;

    struct Key {
        // Call this to access your private contents. Must not use the address after calling init()
        void* writableContents() { return this + 1; }

        // must call this after your private data has been written.
        // length must be a multiple of 4
        void init(size_t length);

        // This is only valid after having called init().
        uint32_t hash() const { return fHash; }

        bool operator==(const Key& other) const {
            const uint32_t* a = this->as32();
            const uint32_t* b = other.as32();
            for (int i = 0; i < fCount32; ++i) {
                if (a[i] != b[i]) {
                    return false;
                }
            }
            return true;
        }

        // delete using sk_free
        Key* clone() const;

    private:
        // store fCount32 first, so we don't consider it in operator<
        int32_t  fCount32;  // 2 + user contents count32
        uint32_t fHash;
        /* uint32_t fContents32[] */

        const uint32_t* as32() const { return (const uint32_t*)this; }
        const uint32_t* as32SkipCount() const { return this->as32() + 1; }
    };

    /**
     *  Returns a locked/pinned SkDiscardableMemory instance for the specified
     *  number of bytes, or NULL on failure.
     */
    typedef SkDiscardableMemory* (*DiscardableFactory)(size_t bytes);

    /*
     *  The following static methods are thread-safe wrappers around a global
     *  instance of this cache.
     */

    static ID* FindAndLock(const Key&, SkBitmap* result);
    static ID* AddAndLock(const Key&, const SkBitmap& result);
    
    static ID* FindAndLock(const Key&, const SkMipMap** result);
    static ID* AddAndLock(const Key&, const SkMipMap* result);
    
    static void Unlock(ID*);

    static size_t GetTotalBytesUsed();
    static size_t GetTotalByteLimit();
    static size_t SetTotalByteLimit(size_t newLimit);

    static size_t SetSingleAllocationByteLimit(size_t);
    static size_t GetSingleAllocationByteLimit();

    static SkBitmap::Allocator* GetAllocator();

    /**
     *  Call SkDebugf() with diagnostic information about the state of the cache
     */
    static void Dump();

    ///////////////////////////////////////////////////////////////////////////

    /**
     *  Construct the cache to call DiscardableFactory when it
     *  allocates memory for the pixels. In this mode, the cache has
     *  not explicit budget, and so methods like getTotalBytesUsed()
     *  and getTotalByteLimit() will return 0, and setTotalByteLimit
     *  will ignore its argument and return 0.
     */
    SkScaledImageCache(DiscardableFactory);

    /**
     *  Construct the cache, allocating memory with malloc, and respect the
     *  byteLimit, purging automatically when a new image is added to the cache
     *  that pushes the total bytesUsed over the limit. Note: The limit can be
     *  changed at runtime with setTotalByteLimit.
     */
    explicit SkScaledImageCache(size_t byteLimit);
    ~SkScaledImageCache();

    /**
     *  Search the cache for a matching key. If found, return its bitmap and return its ID pointer.
     *  Use the returned ID to unlock the cache when you are done using outBitmap.
     *
     *  If a match is not found, outBitmap will be unmodifed, and NULL will be returned.
     */
    ID* findAndLock(const Key& key, SkBitmap* outBitmap);
    ID* findAndLock(const Key& key, const SkMipMap** returnedMipMap);

    /**
     *  To add a new bitmap (or mipMap) to the cache, call
     *  AddAndLock. Use the returned ptr to unlock the cache when you
     *  are done using scaled.
     *
     *  Use (generationID, width, and height) or (original, scaleX,
     *  scaleY) or (original) as a search key
     */
    ID* addAndLock(const Key&, const SkBitmap& bitmap);
    ID* addAndLock(const Key&, const SkMipMap* mipMap);

    /**
     *  Given a non-null ID ptr returned by either findAndLock or addAndLock,
     *  this releases the associated resources to be available to be purged
     *  if needed. After this, the cached bitmap should no longer be
     *  referenced by the caller.
     */
    void unlock(ID*);

    size_t getTotalBytesUsed() const { return fTotalBytesUsed; }
    size_t getTotalByteLimit() const { return fTotalByteLimit; }

    /**
     *  This is respected by SkBitmapProcState::possiblyScaleImage.
     *  0 is no maximum at all; this is the default.
     *  setSingleAllocationByteLimit() returns the previous value.
     */
    size_t setSingleAllocationByteLimit(size_t maximumAllocationSize);
    size_t getSingleAllocationByteLimit() const;
    /**
     *  Set the maximum number of bytes available to this cache. If the current
     *  cache exceeds this new value, it will be purged to try to fit within
     *  this new limit.
     */
    size_t setTotalByteLimit(size_t newLimit);

    SkBitmap::Allocator* allocator() const { return fAllocator; };

    /**
     *  Call SkDebugf() with diagnostic information about the state of the cache
     */
    void dump() const;

public:
    struct Rec;
private:
    Rec*    fHead;
    Rec*    fTail;

    class Hash;
    Hash*   fHash;

    DiscardableFactory  fDiscardableFactory;
    // the allocator is NULL or one that matches discardables
    SkBitmap::Allocator* fAllocator;

    size_t  fTotalBytesUsed;
    size_t  fTotalByteLimit;
    size_t  fSingleAllocationByteLimit;
    int     fCount;

    Rec* findAndLock(const Key& key);
    ID* addAndLock(Rec* rec);

    void purgeRec(Rec*);
    void purgeAsNeeded();

    // linklist management
    void moveToHead(Rec*);
    void addToHead(Rec*);
    void detach(Rec*);

    void init();    // called by constructors

#ifdef SK_DEBUG
    void validate() const;
#else
    void validate() const {}
#endif
};
#endif
