#include "NativeCanvasHandler.h"
#include "NativeSkia.h"
#include "NativeCanvas2DContext.h"
#include <stdio.h>

#include <jsapi.h>
#include <js/GCAPI.h>

#define native_min(val1, val2)  ((val1 > val2) ? (val2) : (val1))
#define native_max(val1, val2)  ((val1 < val2) ? (val2) : (val1))

NativeCanvasHandler::NativeCanvasHandler(int width, int height) :
    m_Context(NULL), jsobj(NULL), jscx(NULL), left(0.0), top(0.0), a_left(0), a_top(0),
    right(0.0), bottom(0.0),
    overflow(true),
    parent(NULL), children(NULL), next(NULL),
    prev(NULL), last(NULL), nchildren(0), coordPosition(COORD_RELATIVE),
    visibility(CANVAS_VISIBILITY_VISIBLE),
    coordMode(kLeft_Coord | kTop_Coord),
    opacity(1.0),
    zoom(1.0),
    scaleX(1.0),
    scaleY(1.0)
{
    this->width = native_max(width, 2);
    this->height = native_max(height, 2);

    memset(&this->padding, 0, sizeof(this->padding));
    memset(&this->translate_s, 0, sizeof(this->translate_s));
    memset(&this->mousePosition, 0, sizeof(this->mousePosition));

    this->mousePosition.consumed = true;

    this->content.width = width;
    this->content.height = height;
    this->content.scrollLeft = 0;
    this->content.scrollTop = 0;

    this->coordMode = kLeft_Coord | kTop_Coord;
}

void NativeCanvasHandler::setPositioning(NativeCanvasHandler::COORD_POSITION mode)
{
    coordPosition = mode;
    this->computeAbsolutePosition();
}

void NativeCanvasHandler::translate(double x, double y)
{
    this->translate_s.x += x;
    this->translate_s.y += y;
}

bool NativeCanvasHandler::setWidth(int width)
{
    if (width < 1) width = 1;

    if (!this->hasFixedWidth()) {
        return false;
    }

    this->width = width;

    if (m_Context) {
        m_Context->setSize(this->width + (this->padding.global * 2),
            this->height + (this->padding.global * 2));
    }

    updateChildrenSize(true, false);

    return true;
}

bool NativeCanvasHandler::setHeight(int height)
{
    if (height < 1) height = 1;

    if (!this->hasFixedHeight()) {
        return false;
    }

    this->height = height;

    if (m_Context) {
        m_Context->setSize(this->width + (this->padding.global * 2),
            this->height + (this->padding.global * 2));
    }

    updateChildrenSize(false, true);

    return true;
}

void NativeCanvasHandler::setSize(int width, int height)
{
    this->width = width;
    this->height = height;

    if (m_Context) {
        m_Context->setSize(this->width + (this->padding.global * 2),
            this->height + (this->padding.global * 2));
    }

    updateChildrenSize(true, true);
}

void NativeCanvasHandler::updateChildrenSize(bool width, bool height)
{
    NativeCanvasHandler *cur;

    for (cur = children; cur != NULL; cur = cur->next) {
        bool updateWidth = false, updateHeight = false;

        if (width && !cur->hasFixedWidth()) {
            updateWidth = true;
        }
        if (height && !cur->hasFixedHeight()) {
            updateHeight = true;
        }

        if (!updateHeight && !updateWidth) {
            continue;
        }

        cur->setSize(updateWidth ? cur->getWidth() : cur->width,
            updateHeight ? cur->getHeight() : cur->height);
    }
}

void NativeCanvasHandler::setPadding(int padding)
{
    if (padding < 0) padding = 0;

    int tmppadding = this->padding.global;

    this->padding.global = padding;

    if (m_Context) {
        m_Context->translate(-tmppadding, -tmppadding);
     
        m_Context->setSize(this->width + (this->padding.global * 2),
            this->height + (this->padding.global * 2));

        m_Context->translate(this->padding.global, this->padding.global);
    }
}

void NativeCanvasHandler::setScrollLeft(int value)
{
    if (value < 0) value = 0;

    this->content.scrollLeft = value;
}

void NativeCanvasHandler::setScrollTop(int value)
{
    if (value < 0) value = 0;

    this->content.scrollTop = value;
}

void NativeCanvasHandler::bringToFront()
{
    if (!this->parent) {
        return;
    }

    this->parent->addChild(this, POSITION_FRONT);
}

void NativeCanvasHandler::sendToBack()
{
    if (!this->parent) {
        return;
    }

    this->parent->addChild(this, POSITION_BACK);
}

void NativeCanvasHandler::addChild(NativeCanvasHandler *insert,
    NativeCanvasHandler::Position position)
{
    /* Already belong to a parent? move it */
    insert->removeFromParent();

    switch(position) {
        case POSITION_FRONT:
            if (!children) {
                children = insert;
            }
            insert->next = NULL;
            insert->prev = last;

            if (last) {
                last->next = insert;
            }

            last = insert;
            break;
        case POSITION_BACK:
            if (!last) {
                last = insert;
            }
            if (children) {
                children->prev = insert;
            }
            insert->next = children;
            insert->prev = NULL;
            children = insert;
            break;
    }
    
    insert->parent = this;
    this->nchildren++;
}

void NativeCanvasHandler::removeFromParent()
{   
    if (!parent) {
        return;
    }

    if (this->jsobj && JS::IsIncrementalBarrierNeeded(JS_GetRuntime(this->jscx))) {
        printf("Reference barrier\n");
        JS::IncrementalReferenceBarrier(this->jsobj, JSTRACE_OBJECT);
    }
    
    if (parent->children == this) {
        parent->children = next;
    }

    if (parent->last == this) {
        parent->last = prev;
    }

    if (prev) {
        prev->next = next;
    }
    if (next) {
        next->prev = prev;
    }

    parent->nchildren--;
    parent = NULL;
    next = NULL;
    prev = NULL;

}

void NativeCanvasHandler::dispatchMouseEvents(NativeCanvasHandler *layer)
{
    if (!layer->mousePosition.consumed) {
        //printf("Mouse event : %dx%d\n", layer->mousePosition.x, layer->mousePosition.y);
    }
}

void NativeCanvasHandler::layerize(NativeCanvasHandler *layer,
    double pleft, double ptop, double aopacity, double azoom, NativeRect *clip)
{
    NativeCanvasHandler *cur;
    NativeRect nclip;

    if (visibility == CANVAS_VISIBILITY_HIDDEN || opacity == 0.0) {
        return;
    }

    //double pzoom = this->zoom * azoom;
    double popacity = this->opacity * aopacity;
    /*
        Fill the root layer with white
        This is the base surface on top of the window frame buffer
    */
    int tmpLeft = this->getLeft();
    int tmpTop = this->getTop();

    if (layer == NULL && m_Context) {
        layer = this;
        m_Context->clear(0xFFFFFFFF);
    } else {
        double cleft = 0.0, ctop = 0.0;

        if (coordPosition != COORD_ABSOLUTE) {
            cleft = pleft;
            ctop = ptop;
        }

        /*
            Set the absolute position
        */
        this->a_left = cleft + tmpLeft + this->translate_s.x;
        this->a_top = ctop + tmpTop + this->translate_s.y;


        /*
            Dispatch current mouse position.
        */

        this->dispatchMouseEvents(layer);

        /*
            draw current context on top of the root layer
        */

        if (m_Context) {
            this->m_Context->composeWith((NativeCanvas2DContext *)layer->m_Context,
                this->a_left - this->padding.global, 
                this->a_top - this->padding.global, popacity, zoom,
                (coordPosition == COORD_ABSOLUTE) ? NULL : clip);
        }
    }

    if (!this->overflow) {

        if (clip == NULL) {
            clip = &nclip;
            clip->fLeft = this->a_left;
            clip->fTop = this->a_top;
            clip->fRight = this->width + this->a_left;
            clip->fBottom = this->height + this->a_top;
            /* if clip is not null, reduce it to intersect the current rect */
        } else if (!clip->intersect(this->a_left, this->a_top,
                    this->width + this->a_left, this->height + this->a_top)) {
            /* don't need to draw children (out of bounds) */
            return;
        }
    }

    if (nchildren) {
        NativeRect tmpClip;

        /* Save the clip */
        if (clip != NULL) {
            memcpy(&tmpClip, clip, sizeof(NativeRect));
        }
        /* Occlusion culling */
#if 0
        NativeCanvasHandler **culling = (NativeCanvasHandler **)malloc(
                                        sizeof(NativeCanvasHandler *)
                                        * nchildren);

        NativeRect culRect;
        for (cur = last; cur != NULL; cur = cur->prev) {
            
        }
#endif
        for (cur = children; cur != NULL; cur = cur->next) {
            int offsetLeft = 0, offsetTop = 0;
            if (cur->coordPosition == COORD_RELATIVE) {
                offsetLeft = -this->content.scrollLeft;
                offsetTop  = -this->content.scrollTop;
            }
            cur->layerize(layer,
                    tmpLeft + this->translate_s.x + pleft + offsetLeft,
                    tmpTop + this->translate_s.y + ptop + offsetTop,
                    popacity, zoom, clip);

            /* restore the old clip (layerize could have altered it) */
            if (clip != NULL) {
                memcpy(clip, &tmpClip, sizeof(NativeRect));
            }
        }
    }

    if (layer == this) {
        this->mousePosition.consumed = true;
    }
}

int NativeCanvasHandler::getContentWidth()
{
    this->computeContentSize(NULL, NULL);

    return this->content.width;
}

int NativeCanvasHandler::getContentHeight()
{
    this->computeContentSize(NULL, NULL);

    return this->content.height;
}

/* TODO: optimize tail recursion? */
bool NativeCanvasHandler::hasAFixedAncestor() const
{
    if (coordPosition == COORD_FIXED) {
        return true;
    }
    return (parent ? parent->hasAFixedAncestor() : false);
}

/* Compute whether or the canvas is going to be drawn */
bool NativeCanvasHandler::isDisplayed() const
{
    if (visibility == CANVAS_VISIBILITY_HIDDEN) {
        return false;
    }

    return (parent ? parent->isDisplayed() : true);
}

void NativeCanvasHandler::computeAbsolutePosition()
{
    if (this->coordPosition == COORD_ABSOLUTE) {
        this->a_top = this->getTop();
        this->a_left = this->getLeft();
        return;
    }

    double ctop = this->getTopScrolled(), cleft = this->getLeftScrolled();

    NativeCanvasHandler *cparent = this->parent;

    while (cparent != NULL) {

        ctop += cparent->getTopScrolled();
        cleft += cparent->getLeftScrolled();

        if (cparent->coordPosition == COORD_ABSOLUTE) {
            break;
        }

        cparent = cparent->parent;
    }

    this->a_top = ctop;
    this->a_left = cleft;

}

bool NativeCanvasHandler::isOutOfBound()
{
    if (!this->parent) {
        return false;
    }

    NativeCanvasHandler *cur;

    for (cur = this->parent; cur != NULL; cur = cur->parent) {
        if (!cur->overflow) {
            
            cur->computeAbsolutePosition();
            this->computeAbsolutePosition();

            return (this->getLeft(true)+this->width <= cur->getLeft(true) ||
                this->getTop(true)+this->height <= cur->getTop(true)
                || this->getLeft(true) >= cur->getLeft(true) + cur->width ||
                this->getTop(true) >= cur->getTop(true) + cur->height);
        }
    }

    return false;
}

NativeRect NativeCanvasHandler::getViewport()
{
    NativeCanvasHandler *cur;

    for (cur = this->parent; cur != NULL; cur = cur->parent) {
        if (!cur->parent) break;

        if (!cur->overflow) {
            
            cur->computeAbsolutePosition();

            return {cur->getLeft(true), cur->getTop(true),
                cur->getTop(true)+cur->height, cur->getLeft(true)+cur->width};
        }
    }
    if (!cur) cur = this;

    return {cur->getLeft(), cur->getTop(),
        cur->getTop()+cur->height, cur->getLeft()+cur->width};
}

NativeRect NativeCanvasHandler::getVisibleRect()
{
    NativeRect vp = this->getViewport();
    this->computeAbsolutePosition();

    return {
        .fLeft   = native_min(native_max(this->getLeft(true), vp.fLeft), vp.fRight),
        .fTop    = native_min(native_max(this->getTop(true), vp.fTop), vp.fBottom),
        .fBottom = native_min(this->getTop(true)+this->height, vp.fBottom),
        .fRight  = native_min(this->getLeft(true)+this->width, vp.fRight)
    };
}

void NativeCanvasHandler::computeContentSize(int *cWidth, int *cHeight)
{
    NativeCanvasHandler *cur;
    this->content.width = this->getWidth();
    this->content.height = this->getHeight();

    /* don't go further if it doesn't overflow (and not the requested handler) */
    if (!this->overflow && cWidth && cHeight) {
        *cWidth = this->content.width;
        *cHeight = this->content.height;
        return;
    }

    for (cur = children; cur != NULL; cur = cur->next) {
        if (cur->coordPosition == COORD_RELATIVE &&
            cur->visibility == CANVAS_VISIBILITY_VISIBLE) {
            
            int retWidth, retHeight;

            cur->computeContentSize(&retWidth, &retHeight);

            if (retWidth + cur->getLeft() > this->content.width) {
                this->content.width = retWidth + cur->getLeft();
            }
            if (retHeight + cur->getTop() > this->content.height) {
                this->content.height = retHeight + cur->getTop();
            }
        }
    }
    if (cWidth) *cWidth = this->content.width;
    if (cHeight) *cHeight = this->content.height;
}

bool NativeCanvasHandler::isHidden() const
{
    return (visibility == CANVAS_VISIBILITY_HIDDEN);
}

void NativeCanvasHandler::setHidden(bool val)
{
    visibility = (val ? CANVAS_VISIBILITY_HIDDEN : CANVAS_VISIBILITY_VISIBLE);
}

void NativeCanvasHandler::setOpacity(double val)
{
    if (val < 0.0 || val > 1.) {
        val = 1;
    }

    opacity = val;
}

void NativeCanvasHandler::setZoom(double zoom)
{
    this->zoom = zoom;
}

void NativeCanvasHandler::getChildren(NativeCanvasHandler **out) const
{
    NativeCanvasHandler *cur;
    int i = 0;
    for (cur = children; cur != NULL; cur = cur->next) {
        out[i++] = cur;
    }
}

void NativeCanvasHandler::setScale(double x, double y)
{
    this->recursiveScale(x, y, this->scaleX, this->scaleY);
    this->scaleX = x;
    this->scaleY = y;
}

void NativeCanvasHandler::setContext(NativeCanvasContext *context)
{
    this->m_Context = context;
    this->m_Context->translate(this->padding.global, this->padding.global);
}

void NativeCanvasHandler::recursiveScale(double x, double y,
    double oldX, double oldY)
{
    NativeCanvasHandler *cur = this;

    if (!cur->m_Context) return;
    
    cur->m_Context->setScale(x, y, oldX, oldY);

    for (cur = children; cur != NULL; cur = cur->next) {
        cur->recursiveScale(x, y, oldX, oldY);
    }
}

int32_t NativeCanvasHandler::countChildren() const
{
    return this->nchildren;
}

bool NativeCanvasHandler::containsPoint(double x, double y) const
{
    return (x >= getLeft(true) && x <= getLeft(true)+width &&
            y >= getTop(true) && y <= getTop(true)+height);
}

void NativeCanvasHandler::unrootHierarchy()
{
    #if 0
    NativeCanvasHandler *cur;

    for (cur = children; cur != NULL; cur = cur->next) {
        cur->unrootHierarchy();
        if (cur->context && cur->context->jsobj && cur->context->jscx) {
            JS_RemoveObjectRoot(cur->context->jscx, &cur->context->jsobj);
        }
        if (cur->jsobj) {
            JS_RemoveObjectRoot(cur->jscx, &cur->jsobj);
        }
        cur->jsobj = NULL;
        cur->context->jsobj = NULL;
    }
    children = NULL;
    #endif
}

NativeCanvasHandler::~NativeCanvasHandler()
{
    NativeCanvasHandler *cur = children, *cnext;

    removeFromParent();

    /* all children got orphaned :( */
    while(cur != NULL) {
        //printf("Warning: a canvas got orphaned (%p)\n", cur);
        cnext = cur->next;
        cur->removeFromParent();
        cur = cnext;
    }
}
