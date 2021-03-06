/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkOpContour_DEFINED
#define SkOpContour_DEFINED

#include "SkOpSegment.h"
#include "SkTDArray.h"
#include "SkTSort.h"

class SkChunkAlloc;
class SkPathWriter;

class SkOpContour {
public:
    SkOpContour() {
        reset();
    }

    ~SkOpContour() {
        if (fNext) {
            fNext->~SkOpContour();
        }
    }

    bool operator<(const SkOpContour& rh) const {
        return fBounds.fTop == rh.fBounds.fTop
                ? fBounds.fLeft < rh.fBounds.fLeft
                : fBounds.fTop < rh.fBounds.fTop;
    }

    void addCubic(SkPoint pts[4], SkChunkAlloc* allocator) {
        appendSegment(allocator).addCubic(pts, this);
    }

    void addCurve(SkPath::Verb verb, const SkPoint pts[4], SkChunkAlloc* allocator);

    void addLine(SkPoint pts[2], SkChunkAlloc* allocator) {
        appendSegment(allocator).addLine(pts, this);
    }

    void addQuad(SkPoint pts[3], SkChunkAlloc* allocator) {
        appendSegment(allocator).addQuad(pts, this);
    }

    void align() {
        SkASSERT(fCount > 0);
        SkOpSegment* segment = &fHead;
        do {
            segment->align();
        } while ((segment = segment->next()));
    }

    SkOpSegment& appendSegment(SkChunkAlloc* allocator) {
        SkOpSegment* result = fCount++
                ? SkOpTAllocator<SkOpSegment>::Allocate(allocator) : &fHead;
        result->setPrev(fTail);
        if (fTail) {
            fTail->setNext(result);
        }
        fTail = result;
        return *result;
    }

    SkOpContour* appendContour(SkChunkAlloc* allocator) {
        SkOpContour* contour = SkOpTAllocator<SkOpContour>::New(allocator);
        contour->setNext(NULL);
        SkOpContour* prev = this;
        SkOpContour* next;
        while ((next = prev->next())) {
            prev = next;
        }
        prev->setNext(contour);
        return contour;
    }
    
    const SkPathOpsBounds& bounds() const {
        return fBounds;
    }

    void calcAngles(SkChunkAlloc* allocator) {
        SkASSERT(fCount > 0);
        SkOpSegment* segment = &fHead;
        do {
            segment->calcAngles(allocator);
        } while ((segment = segment->next()));
    }

    void complete() {
        setBounds();
    }

    int count() const {
        return fCount;
    }

    int debugID() const {
        return PATH_OPS_DEBUG_RELEASE(fID, -1);
    }

    int debugIndent() const {
        return PATH_OPS_DEBUG_RELEASE(fIndent, 0);
    }

#if DEBUG_ACTIVE_SPANS
    void debugShowActiveSpans() {
        SkOpSegment* segment = &fHead;
        do {
            segment->debugShowActiveSpans();
        } while ((segment = segment->next()));
    }
#endif

    const SkOpAngle* debugAngle(int id) const {
        return PATH_OPS_DEBUG_RELEASE(globalState()->debugAngle(id), NULL);
    }

    SkOpContour* debugContour(int id) {
        return PATH_OPS_DEBUG_RELEASE(globalState()->debugContour(id), NULL);
    }

    const SkOpPtT* debugPtT(int id) const {
        return PATH_OPS_DEBUG_RELEASE(globalState()->debugPtT(id), NULL);
    }

    const SkOpSegment* debugSegment(int id) const {
        return PATH_OPS_DEBUG_RELEASE(globalState()->debugSegment(id), NULL);
    }

    const SkOpSpanBase* debugSpan(int id) const {
        return PATH_OPS_DEBUG_RELEASE(globalState()->debugSpan(id), NULL);
    }

    SkOpGlobalState* globalState() const {
        return fState; 
    }

    void debugValidate() const {
#if DEBUG_VALIDATE
        const SkOpSegment* segment = &fHead;
        const SkOpSegment* prior = NULL;
        do {
            segment->debugValidate();
            SkASSERT(segment->prev() == prior);
            prior = segment;
        } while ((segment = segment->next()));
        SkASSERT(prior == fTail);
#endif
    }

    bool done() const {
        return fDone;
    }

    void dump();
    void dumpAll();
    void dumpAngles() const;
    void dumpPt(int ) const;
    void dumpPts() const;
    void dumpPtsX() const;
    void dumpSegment(int ) const;
    void dumpSegments(SkPathOp op) const;
    void dumpSpan(int ) const;
    void dumpSpans() const;

    const SkPoint& end() const {
        return fTail->pts()[SkPathOpsVerbToPoints(fTail->verb())];
    }

    SkOpSegment* first() {
        SkASSERT(fCount > 0);
        return &fHead;
    }

    const SkOpSegment* first() const {
        SkASSERT(fCount > 0);
        return &fHead;
    }

    void indentDump() {
        PATH_OPS_DEBUG_CODE(fIndent += 2);
    }

    void init(SkOpGlobalState* globalState, bool operand, bool isXor) {
        fState = globalState;
        fOperand = operand;
        fXor = isXor;
    }

    bool isXor() const {
        return fXor;
    }

    void missingCoincidence(SkOpCoincidence* coincidences, SkChunkAlloc* allocator) {
        SkASSERT(fCount > 0);
        SkOpSegment* segment = &fHead;
        do {
            if (fState->angleCoincidence()) {
                segment->checkAngleCoin(coincidences, allocator);
            } else {
                segment->missingCoincidence(coincidences, allocator);
            }
        } while ((segment = segment->next()));
    }

    bool moveNearby() {
        SkASSERT(fCount > 0);
        SkOpSegment* segment = &fHead;
        do {
            if (!segment->moveNearby()) {
                return false;
            }
        } while ((segment = segment->next()));
        return true;
    }

    SkOpContour* next() {
        return fNext;
    }

    const SkOpContour* next() const {
        return fNext;
    }

    SkOpSegment* nonVerticalSegment(SkOpSpanBase** start, SkOpSpanBase** end);

    bool operand() const {
        return fOperand;
    }

    bool oppXor() const {
        return fOppXor;
    }

    void outdentDump() {
        PATH_OPS_DEBUG_CODE(fIndent -= 2);
    }

    void remove(SkOpContour* contour) {
        if (contour == this) {
            SkASSERT(fCount == 0);
            return;
        }
        SkASSERT(contour->fNext == NULL);
        SkOpContour* prev = this;
        SkOpContour* next;
        while ((next = prev->next()) != contour) {
            SkASSERT(next);
            prev = next;
        }
        SkASSERT(prev);
        prev->setNext(NULL);
    }

    void reset() {
        fTail = NULL;
        fNext = NULL;
        fCount = 0;
        fDone = false;
        SkDEBUGCODE(fBounds.set(SK_ScalarMax, SK_ScalarMax, SK_ScalarMin, SK_ScalarMin));
        SkDEBUGCODE(fFirstSorted = -1);
        PATH_OPS_DEBUG_CODE(fIndent = 0);
    }

    void setBounds() {
        SkASSERT(fCount > 0);
        const SkOpSegment* segment = &fHead;
        fBounds = segment->bounds();
        while ((segment = segment->next())) {
            fBounds.add(segment->bounds());
        }
    }

    void setGlobalState(SkOpGlobalState* state) {
        fState = state;
    }

    void setNext(SkOpContour* contour) {
//        SkASSERT(!fNext == !!contour);
        fNext = contour;
    }

    void setOperand(bool isOp) {
        fOperand = isOp;
    }

    void setOppXor(bool isOppXor) {
        fOppXor = isOppXor;
    }

    void setXor(bool isXor) {
        fXor = isXor;
    }

    SkPath::Verb simplifyCubic(SkPoint pts[4]);

    void sortAngles() {
        SkASSERT(fCount > 0);
        SkOpSegment* segment = &fHead;
        do {
            segment->sortAngles();
        } while ((segment = segment->next()));
    }

    void sortSegments() {
        SkOpSegment* segment = &fHead;
        do {
            *fSortedSegments.append() = segment;
        } while ((segment = segment->next()));
        SkTQSort<SkOpSegment>(fSortedSegments.begin(), fSortedSegments.end() - 1);
        fFirstSorted = 0;
    }

    const SkPoint& start() const {
        return fHead.pts()[0];
    }

    void toPartialBackward(SkPathWriter* path) const {
        const SkOpSegment* segment = fTail;
        do {
            segment->addCurveTo(segment->tail(), segment->head(), path, true);
        } while ((segment = segment->prev()));
    }

    void toPartialForward(SkPathWriter* path) const {
        const SkOpSegment* segment = &fHead;
        do {
            segment->addCurveTo(segment->head(), segment->tail(), path, true);
        } while ((segment = segment->next()));
    }

    void toPath(SkPathWriter* path) const;
    void topSortableSegment(const SkPoint& topLeft, SkPoint* bestXY, SkOpSegment** topStart);
    SkOpSegment* undoneSegment(SkOpSpanBase** startPtr, SkOpSpanBase** endPtr);

private:
    SkOpGlobalState* fState;
    SkOpSegment fHead;
    SkOpSegment* fTail;
    SkOpContour* fNext;
    SkTDArray<SkOpSegment*> fSortedSegments;  // set by find top segment
    SkPathOpsBounds fBounds;
    int fCount;
    int fFirstSorted;
    bool fDone;  // set by find top segment
    bool fOperand;  // true for the second argument to a binary operator
    bool fXor;  // set if original path had even-odd fill
    bool fOppXor;  // set if opposite path had even-odd fill
    PATH_OPS_DEBUG_CODE(int fID);
    PATH_OPS_DEBUG_CODE(int fIndent);
};

#endif
