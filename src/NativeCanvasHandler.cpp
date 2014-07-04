#include "NativeCanvasHandler.h"
#include "NativeSkia.h"
#include "NativeCanvas2DContext.h"
#include "NativeMacros.h"
#include <stdio.h>

#include <jsapi.h>
#include <js/GCAPI.h>

#define native_min(val1, val2)  ((val1 > val2) ? (val2) : (val1))
#define native_max(val1, val2)  ((val1 < val2) ? (val2) : (val1))

NativeCanvasHandler::NativeCanvasHandler(int width, int height) :
    m_Context(NULL), jsobj(NULL), jscx(NULL), left(0.0), top(0.0), a_left(0), a_top(0),
    right(0.0), bottom(0.0),
    overflow(true),
    m_Parent(NULL), m_Children(NULL), m_Next(NULL),
    m_Prev(NULL), m_Last(NULL), nchildren(0), coordPosition(COORD_RELATIVE),
    visibility(CANVAS_VISIBILITY_VISIBLE),
    m_FlowMode(kFlowDoesntInteract),
    coordMode(kLeft_Coord | kTop_Coord),
    opacity(1.0),
    zoom(1.0),
    scaleX(1.0),
    scaleY(1.0),
    m_AllowNegativeScroll(false)
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
    if (mode == COORD_INLINE) {
        mode = COORD_RELATIVE;
        m_FlowMode |= kFlowInlinePreviousSibling;
    } else {
        m_FlowMode &= ~kFlowInlinePreviousSibling;
    }

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

    if (this->width == width) {
        return true;
    }

    this->width = width;

    if (m_Context) {
        m_Context->setSize(this->width + (this->padding.global * 2),
            this->height + (this->padding.global * 2));
    }

    NativeArgs arg;

    arg[0].set(this->width);
    arg[1].set(this->height);

    this->fireEvent<NativeCanvasHandler>(RESIZE_EVENT, arg);

    updateChildrenSize(true, false);

    return true;
}

bool NativeCanvasHandler::setHeight(int height)
{
    if (height < 1) height = 1;

    if (!this->hasFixedHeight()) {
        return false;
    }
    if (this->height == height) {
        return true;
    }
    this->height = height;

    if (m_Context) {
        m_Context->setSize(this->width + (this->padding.global * 2),
            this->height + (this->padding.global * 2));
    }

    NativeArgs arg;

    arg[0].set(this->width);
    arg[1].set(this->height);

    this->fireEvent<NativeCanvasHandler>(RESIZE_EVENT, arg);

    updateChildrenSize(false, true);

    return true;
}

void NativeCanvasHandler::setSize(int width, int height, bool redraw)
{
    if (height < 1) height = 1;
    if (width < 1) width = 1;

    if (this->height == height && this->width == width) {
        return;
    }

    this->width = width;
    this->height = height;

    if (m_Context) {
        m_Context->setSize(this->width + (this->padding.global * 2),
            this->height + (this->padding.global * 2), redraw);
    }

    NativeArgs arg;

    arg[0].set(width);
    arg[1].set(height);

    this->fireEvent<NativeCanvasHandler>(RESIZE_EVENT, arg);

    updateChildrenSize(true, true);
}

void NativeCanvasHandler::updateChildrenSize(bool width, bool height)
{
    NativeCanvasHandler *cur;

    for (cur = m_Children; cur != NULL; cur = cur->m_Next) {
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
        NLOG("Update size of %p through parent", cur);
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
    if (value < 0 && !m_AllowNegativeScroll) value = 0;

    this->content.scrollLeft = value;
}

void NativeCanvasHandler::setScrollTop(int value)
{
    if (value < 0 && !m_AllowNegativeScroll) value = 0;

    this->content.scrollTop = value;
}

void NativeCanvasHandler::bringToFront()
{
    if (!m_Parent) {
        return;
    }

    m_Parent->addChild(this, POSITION_FRONT);
}

void NativeCanvasHandler::sendToBack()
{
    if (!m_Parent) {
        return;
    }

    m_Parent->addChild(this, POSITION_BACK);
}

void NativeCanvasHandler::addChild(NativeCanvasHandler *insert,
    NativeCanvasHandler::Position position)
{
    /* Already belong to a parent? move it */
    insert->removeFromParent();

    switch(position) {
        case POSITION_FRONT:
            if (!m_Children) {
                m_Children = insert;
            }
            insert->m_Next = NULL;
            insert->m_Prev = m_Last;

            if (m_Last) {
                m_Last->m_Next = insert;
            }

            m_Last = insert;
            break;
        case POSITION_BACK:
            if (!m_Last) {
                m_Last = insert;
            }
            if (m_Children) {
                m_Children->m_Prev = insert;
            }
            insert->m_Next = m_Children;
            insert->m_Prev = NULL;
            m_Children = insert;
            break;
    }
    
    insert->m_Parent = this;
    this->nchildren++;
}

void NativeCanvasHandler::removeFromParent()
{   
    if (!m_Parent) {
        return;
    }

    if (this->jsobj && JS::IsIncrementalBarrierNeeded(JS_GetRuntime(this->jscx))) {
        JS::IncrementalReferenceBarrier(this->jsobj, JSTRACE_OBJECT);
    }
    
    if (m_Parent->m_Children == this) {
        m_Parent->m_Children = m_Next;
    }

    if (m_Parent->m_Last == this) {
        m_Parent->m_Last = m_Prev;
    }

    if (m_Prev) {
        m_Prev->m_Next = m_Next;
    }
    if (m_Next) {
        m_Next->m_Prev = m_Prev;
    }

    m_Parent->nchildren--;
    m_Parent = NULL;
    m_Next = NULL;
    m_Prev = NULL;

}

void NativeCanvasHandler::dispatchMouseEvents(NativeCanvasHandler *layer)
{
    if (!layer->mousePosition.consumed) {
        //printf("Mouse event : %dx%d\n", layer->mousePosition.x, layer->mousePosition.y);
    }
}

void NativeCanvasHandler::layerize(NativeLayerizeContext &layerContext)
{
    NativeCanvasHandler *cur;
    NativeRect nclip;
    NativeLayerSiblingContext *sctx = layerContext.siblingCtx;

    if (visibility == CANVAS_VISIBILITY_HIDDEN || opacity == 0.0) {
        return;
    }

    //double pzoom = this->zoom * azoom;
    double popacity = this->opacity * layerContext.aopacity;

    int tmpLeft;
    int tmpTop;

    if (this->coordPosition == COORD_RELATIVE && this->m_FlowMode & kFlowInlinePreviousSibling) {
        NativeCanvasHandler *prev = getPrevInlineSibling();

        if (!prev) {
            tmpLeft = tmpTop = 0;
        } else {
            tmpLeft = prev->a_left + prev->getWidth();
            tmpTop = prev->a_top;

            if (m_Parent) {
                /* New "line" */
                if (tmpLeft + this->getWidth() > m_Parent->a_left + m_Parent->getWidth()) {
                    sctx->maxLineHeightPreviousLine = sctx->maxLineHeight;
                    sctx->maxLineHeight = this->getHeight();

                    tmpTop = prev->a_top + sctx->maxLineHeightPreviousLine;
                    tmpLeft = 0;
                }
            }
        }
        if (this->getHeight() > sctx->maxLineHeight) {
            sctx->maxLineHeight = this->getHeight();
        }
    } else {
        tmpLeft = this->getLeft();
        tmpTop = this->getTop();
    }

    /*
        Fill the root layer with white
        This is the base surface on top of the window frame buffer
    */
    if (layerContext.layer == NULL && m_Context) {
        layerContext.layer = this;
        m_Context->clear(0xFFFFFFFF);
        m_Context->flush();
    } else {
        double cleft = 0.0, ctop = 0.0;

        if (coordPosition != COORD_ABSOLUTE) {
            cleft = layerContext.pleft;
            ctop = layerContext.ptop;
        }

        /*
            Set the absolute position
        */
        this->a_left = cleft + tmpLeft + this->translate_s.x;
        this->a_top = ctop + tmpTop + this->translate_s.y;


        /*
            Dispatch current mouse position.
        */

        this->dispatchMouseEvents(layerContext.layer);

        /*
            draw current context on top of the root layer
        */

        if (m_Context) {
            /*
                Not visible. Don't call composeWith()
            */
            if (!layerContext.clip || coordPosition == COORD_ABSOLUTE ||
              (layerContext.clip->checkIntersect(
                this->a_left - this->padding.global,
                this->a_top - this->padding.global,
                this->a_left + this->padding.global + this->getWidth(),
                this->a_top + this->padding.global + this->getHeight()))) {
            
                this->m_Context->composeWith((NativeCanvas2DContext *)layerContext.layer->m_Context,
                    this->a_left - this->padding.global, 
                    this->a_top - this->padding.global, popacity, zoom,
                    (coordPosition == COORD_ABSOLUTE) ? NULL : layerContext.clip);
            }
        }
    }

    if (!this->overflow) {
        if (layerContext.clip == NULL) {
            layerContext.clip = &nclip;
            layerContext.clip->fLeft = this->a_left;
            layerContext.clip->fTop = this->a_top;
            layerContext.clip->fRight = this->width + this->a_left;
            layerContext.clip->fBottom = this->height + this->a_top;
            /*
                if clip is not null, reduce it to intersect the current rect.
                /!\ clip->intersect changes "clip"
            */
        } else if (!layerContext.clip->intersect(this->a_left, this->a_top,
                    this->width + this->a_left, this->height + this->a_top)) {
            /* don't need to draw children (out of bounds) */
            return;
        }
    }

    if (nchildren) {
        NativeRect tmpClip;

        /* Save the clip */
        if (layerContext.clip != NULL) {
            memcpy(&tmpClip, layerContext.clip, sizeof(NativeRect));
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
        struct NativeLayerSiblingContext siblingctx;

        for (cur = m_Children; cur != NULL; cur = cur->m_Next) {
            int offsetLeft = 0, offsetTop = 0;
            if (cur->coordPosition == COORD_RELATIVE) {
                offsetLeft = -this->content.scrollLeft;
                offsetTop  = -this->content.scrollTop;
            }

            struct NativeLayerizeContext ctx = {
                .layer = layerContext.layer,
                .pleft = tmpLeft + this->translate_s.x + layerContext.pleft + offsetLeft,
                .ptop  = tmpTop + this->translate_s.y + layerContext.ptop + offsetTop,
                .aopacity = popacity,
                .azoom = zoom,
                .clip = layerContext.clip,
                .siblingCtx = &siblingctx
            };

            cur->layerize(ctx);

            /* restore the old clip (layerize could have altered it) */
            if (layerContext.clip != NULL) {
                memcpy(layerContext.clip, &tmpClip, sizeof(NativeRect));
            }
        }
    }

    if (layerContext.layer == this) {
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
    return (m_Parent ? m_Parent->hasAFixedAncestor() : false);
}

/* Compute whether or the canvas is going to be drawn */
bool NativeCanvasHandler::isDisplayed() const
{
    if (visibility == CANVAS_VISIBILITY_HIDDEN) {
        return false;
    }

    return (m_Parent ? m_Parent->isDisplayed() : true);
}

void NativeCanvasHandler::computeAbsolutePosition()
{
    if (this->coordPosition == COORD_ABSOLUTE) {
        this->a_top = this->getTop();
        this->a_left = this->getLeft();
        return;
    }

    if (this->coordPosition == COORD_RELATIVE &&
        m_FlowMode & kFlowInlinePreviousSibling) {
        this->left = 0;
        this->top = 0;
        NativeCanvasHandler *prev = getPrevInlineSibling();
        if (prev) {
            double ctop = this->getTopScrolled(), cleft = this->getLeftScrolled();

            prev->computeAbsolutePosition();

            this->a_top = ctop + prev->a_top;
            this->a_left = cleft + prev->a_left + prev->getWidth();

            return;
        }
    }

    double ctop = this->getTopScrolled(), cleft = this->getLeftScrolled();

    NativeCanvasHandler *cparent = m_Parent;

    while (cparent != NULL) {

        ctop += cparent->getTopScrolled();
        cleft += cparent->getLeftScrolled();

        if (cparent->coordPosition == COORD_ABSOLUTE) {
            break;
        }

        cparent = cparent->getParent();
    }

    this->a_top = ctop;
    this->a_left = cleft;

}

bool NativeCanvasHandler::isOutOfBound()
{
    if (!m_Parent) {
        return false;
    }

    NativeCanvasHandler *cur;

    for (cur = m_Parent; cur != NULL; cur = cur->m_Parent) {
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
    NativeCanvasHandler *cur = NULL;

    for (cur = m_Parent; cur != NULL; cur = cur->m_Parent) {
        if (!cur->m_Parent) break;

        if (!cur->overflow) {
            
            cur->computeAbsolutePosition();

            return {
                cur->getLeft(true),
                cur->getTop(true),
                cur->getTop(true)+cur->height,
                cur->getLeft(true)+cur->width
            };
        }
    }
    if (!cur) cur = this;

    return {
        cur->getLeft(),
        cur->getTop(),
        cur->getTop()+cur->height,
        cur->getLeft()+cur->width};
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

    for (cur = m_Children; cur != NULL; cur = cur->m_Next) {
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
    for (cur = m_Children; cur != NULL; cur = cur->m_Next) {
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

    for (cur = m_Children; cur != NULL; cur = cur->m_Next) {
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
    NativeCanvasHandler *cur = m_Children, *cnext;

    removeFromParent();

    /* all children got orphaned :( */
    while(cur != NULL) {
        //printf("Warning: a canvas got orphaned (%p)\n", cur);
        cnext = cur->m_Next;
        cur->removeFromParent();
        cur = cnext;
    }
}
