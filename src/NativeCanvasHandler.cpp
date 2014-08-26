#include "NativeCanvasHandler.h"
#include "NativeSkia.h"
#include "NativeCanvas2DContext.h"
#include "NativeMacros.h"
#include "NativeContext.h"
#include <stdio.h>

#include <jsapi.h>
#include <js/GCAPI.h>

int NativeCanvasHandler::LastIdx = 0;

NativeCanvasHandler::NativeCanvasHandler(int width, int height,
    NativeContext *NativeCtx, bool lazyLoad) :
    m_Context(NULL), jsobj(NULL), jscx(NULL), left(0.0), top(0.0), a_left(0), a_top(0),
    right(0.0), bottom(0.0),
    m_Overflow(true),
    m_Parent(NULL), m_Children(NULL), m_Next(NULL),
    m_Prev(NULL), m_Last(NULL), nchildren(0), coordPosition(COORD_RELATIVE),
    visibility(CANVAS_VISIBILITY_VISIBLE),
    m_FlowMode(kFlowDoesntInteract),
    coordMode(kLeft_Coord | kTop_Coord),
    opacity(1.0),
    zoom(1.0),
    scaleX(1.0),
    scaleY(1.0),
    m_AllowNegativeScroll(false),
    m_NativeContext(NativeCtx),
    m_Pending(0),
    m_Loaded(!lazyLoad)
{
    /*
        TODO: thread safe
    */
    m_Identifier.idx = ++NativeCanvasHandler::LastIdx;

    asprintf(&m_Identifier.str, "%lld", m_Identifier.idx);

    m_NativeContext->m_CanvasList.set(m_Identifier.str, this);

    m_Width = native_max(width, 1);
    m_Height = native_max(height, 1);
    m_MaxHeight = 0;
    m_MaxWidth = 0;
    m_MinWidth = 1;
    m_MinHeight = 1;

    m_FluidHeight = false;
    m_FluidWidth = false;

    memset(&this->padding, 0, sizeof(this->padding));
    memset(&this->translate_s, 0, sizeof(this->translate_s));
    memset(&this->mousePosition, 0, sizeof(this->mousePosition));

    this->mousePosition.consumed = true;

    this->content.width = m_Width;
    this->content.height = m_Height;

    this->content._width = m_Width;
    this->content._height = m_Height;

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

void NativeCanvasHandler::setId(const char *str)
{
    if (!str) {
        return;
    }

    m_NativeContext->m_CanvasList.erase(m_Identifier.str);
    m_NativeContext->m_CanvasList.set(str, this);

    free(m_Identifier.str);
    m_Identifier.str = strdup(str);
}

void NativeCanvasHandler::translate(double x, double y)
{
    this->translate_s.x += x;
    this->translate_s.y += y;
}

bool NativeCanvasHandler::setMinWidth(int width)
{
    if (width < 1) width = 1;

    m_MinWidth = m_MaxWidth ? native_min(width, m_MaxWidth) : width;

    if (m_Width < m_MinWidth) {
        this->setWidth(m_MinWidth);
    }

    return true;
}

bool NativeCanvasHandler::setMinHeight(int height)
{
    if (height < 1) height = 1;

    m_MinHeight = m_MaxHeight ? native_min(height, m_MaxHeight) : height;

    if (m_Height < m_MinHeight) {
        this->setHeight(m_MinHeight);
    }

    return true;
}

bool NativeCanvasHandler::setMaxWidth(int width)
{
    if (width < 1) width = 1;
    
    m_MaxWidth = native_max(m_MinWidth, width);

    if (m_Width > m_MaxWidth) {
        this->setWidth(m_MaxWidth);
    }

    return true;
}

bool NativeCanvasHandler::setMaxHeight(int height)
{
    if (height < 1) height = 1;

    m_MaxHeight = native_max(m_MinHeight, height);

    if (m_Height > m_MaxHeight) {
        this->setHeight(m_MaxHeight);
    }

    return true;
}

bool NativeCanvasHandler::setWidth(int width, bool force)
{
    width = m_MaxWidth ? native_clamp(width, m_MinWidth, m_MaxWidth) : 
                           native_max(width, m_MinWidth);

    if (!force && !this->hasFixedWidth()) {
        return false;
    }

    if (m_Width == width) {
        return true;
    }

    m_Width = width;

    this->setPendingFlags(kPendingResizeWidth);

    updateChildrenSize(true, false);

    return true;
}

bool NativeCanvasHandler::setHeight(int height, bool force)
{
    if (!force && !this->hasFixedHeight()) {
        return false;
    }

    height = m_MaxHeight ? native_clamp(height, m_MinHeight, m_MaxHeight) : 
                           native_max(height, m_MinHeight);

    if (m_Height == height) {
        return true;
    }
    m_Height = height;

    this->setPendingFlags(kPendingResizeHeight);

    updateChildrenSize(false, true);

    return true;
}

void NativeCanvasHandler::setSize(int width, int height, bool redraw)
{

    height = m_MaxHeight ? native_clamp(height, m_MinHeight, m_MaxHeight) : 
                           native_max(height, m_MinHeight);

    width = m_MaxWidth ? native_clamp(width, m_MinWidth, m_MaxWidth) : 
                           native_max(width, m_MinWidth);

    if (m_Height == height && m_Width == width) {
        return;
    }

    m_Width = width;
    m_Height = height;

    this->setPendingFlags(kPendingResizeWidth | kPendingResizeHeight);

    updateChildrenSize(true, true);
}

void NativeCanvasHandler::deviceSetSize(int width, int height)
{
    if (m_Context) {
        m_Context->setSize(width + (this->padding.global * 2),
            height + (this->padding.global * 2));
    }

    NativeArgs arg;

    arg[0].set(width);
    arg[1].set(height);
    this->fireEvent<NativeCanvasHandler>(RESIZE_EVENT, arg);
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
        //NLOG("Update size of %p through parent", cur);
        cur->setSize(updateWidth ? cur->getWidth() : cur->m_Width,
            updateHeight ? cur->getHeight() : cur->m_Height);
    }
}

void NativeCanvasHandler::setPadding(int padding)
{
    if (padding < 0) padding = 0;
    
    if (padding == this->padding.global) {
        return;
    }

    int tmppadding = this->padding.global;

    this->padding.global = padding;

    if (m_Context) {
        m_Context->translate(-tmppadding, -tmppadding);
     
        m_Context->setSize(m_Width + (this->padding.global * 2),
            m_Height + (this->padding.global * 2));

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

void NativeCanvasHandler::insertBefore(NativeCanvasHandler *insert,
    NativeCanvasHandler *ref)
{
    if (!ref || !insert) {
        this->addChild(insert, POSITION_FRONT);
        return;
    }
    if (ref->getParent() != this || ref == insert || insert == this) {
        return;
    }

    insert->removeFromParent();

    insert->m_Prev = ref->m_Prev;
    insert->m_Next = ref;

    if (ref->m_Prev) {
        ref->m_Prev->m_Next = insert;
        ref->m_Prev = insert;
    } else {
        m_Children = insert;
    }

    insert->m_Parent = this;
    this->nchildren++;
}

void NativeCanvasHandler::insertAfter(NativeCanvasHandler *insert,
    NativeCanvasHandler *ref)
{
    if (!ref) {
        this->addChild(insert, POSITION_FRONT);
        return;
    }

    this->insertBefore(insert, ref->m_Next);
}

void NativeCanvasHandler::addChild(NativeCanvasHandler *insert,
    NativeCanvasHandler::Position position)
{
    if (!insert || insert == this) {
        return;
    }
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
    int maxChildrenHeight = this->getHeight(), maxChildrenWidth = this->getWidth();

    //double pzoom = this->zoom * azoom;
    double popacity = this->opacity * layerContext.aopacity;

    int tmpLeft;
    int tmpTop;
    bool willDraw = true;

    if (this->coordPosition == COORD_RELATIVE &&
        this->m_FlowMode & kFlowInlinePreviousSibling) {

        NativeCanvasHandler *prev = getPrevInlineSibling();

        if (!prev) {
            tmpLeft = tmpTop = 0;
            this->left = 0;
            this->top = 0;

        } else {
            int prevWidth = prev->visibility == CANVAS_VISIBILITY_HIDDEN ?
                                                    0 : prev->getWidth();

            this->left = tmpLeft = prev->left + prevWidth;
            this->top = tmpTop = prev->top;

            if (m_Parent) {
                /* New "line" */
                if (hasStaticRight() ||
                    tmpLeft + this->getWidth() > m_Parent->getWidth()) {

                    sctx->maxLineHeightPreviousLine = sctx->maxLineHeight;
                    sctx->maxLineHeight = this->getHeight();

                    tmpTop = this->top = prev->top + sctx->maxLineHeightPreviousLine;
                    tmpLeft = this->left = 0;
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

        willDraw = (!layerContext.clip || coordPosition == COORD_ABSOLUTE ||
              (layerContext.clip->checkIntersect(
                this->a_left - this->padding.global,
                this->a_top - this->padding.global,
                this->a_left + this->padding.global + this->getWidth(),
                this->a_top + this->padding.global + this->getHeight())));

        if (m_Context && willDraw) {
            /*
                Not visible. Don't call composeWith()
            */
            this->m_Context->composeWith((NativeCanvas2DContext *)layerContext.layer->m_Context,
                this->a_left - this->padding.global, 
                this->a_top - this->padding.global, popacity, zoom,
                (coordPosition == COORD_ABSOLUTE) ? NULL : layerContext.clip);
        }
    }

    if (!m_Overflow) {
        if (layerContext.clip == NULL) {
            layerContext.clip = &nclip;
            layerContext.clip->fLeft = this->a_left;
            layerContext.clip->fTop = this->a_top;
            layerContext.clip->fRight = m_Width + this->a_left;
            layerContext.clip->fBottom = m_Height + this->a_top;
            /*
                if clip is not null, reduce it to intersect the current rect.
                /!\ clip->intersect changes "clip"
            */
        } else if (!layerContext.clip->intersect(this->a_left, this->a_top,
                    m_Width + this->a_left, m_Height + this->a_top) && !m_FluidHeight) {
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

            /*
                Incrementaly check the bottom/right most children
                in order to compute the contentHeight/Width
            */
            if (cur->coordPosition == COORD_RELATIVE &&
                cur->visibility == CANVAS_VISIBILITY_VISIBLE) {

                int actualChildrenHeightPlusTop = cur->getTop() + (cur->m_Overflow ?
                                        cur->content._height : cur->getHeight());
                int actualChildrenWidthPlusLeft = cur->getLeft() + (cur->m_Overflow ?
                                        cur->content._width : cur->getWidth());

                if (actualChildrenHeightPlusTop > maxChildrenHeight) {
                    maxChildrenHeight = actualChildrenHeightPlusTop;
                }
                if (actualChildrenWidthPlusLeft > maxChildrenWidth) {
                    maxChildrenWidth = actualChildrenWidthPlusLeft;
                }
            }
            /* restore the old clip (layerize could have altered it) */
            if (layerContext.clip != NULL) {
                memcpy(layerContext.clip, &tmpClip, sizeof(NativeRect));
            }
        }
    }
    if (this->content._height != maxChildrenHeight) {
        this->content._height = maxChildrenHeight;

        this->propertyChanged(kContentHeight_Changed);
    }
    if (this->content._width != maxChildrenWidth) {
        this->content._width = maxChildrenWidth;

        this->propertyChanged(kContentWidth_Changed);
    }

    /*
        Height is dynamic.
        It's automatically adjusted by the height of its content
    */
    if (m_FluidHeight) {
        int contentHeight = this->getContentHeight(true);

        int newHeight = m_MaxHeight ? native_clamp(contentHeight, m_MinHeight, m_MaxHeight) : 
                           native_max(contentHeight, m_MinHeight);

        if (m_Height != newHeight) {
            this->setHeight(newHeight, true);
        }
    }
    
    if (!m_Loaded && willDraw) {
        m_Loaded = true;
        this->checkLoaded();
    }

    if (layerContext.layer == this) {
        this->mousePosition.consumed = true;
    }
}

int NativeCanvasHandler::getContentWidth(bool inner)
{
    this->computeContentSize(NULL, NULL, inner);

    return this->content.width;
}

int NativeCanvasHandler::getContentHeight(bool inner)
{
    this->computeContentSize(NULL, NULL, inner);

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

        if (m_Parent == NULL) {
            this->a_top = this->a_left = 0;
            return;
        }

        NativeCanvasHandler *elem, *prev = NULL;

        m_Parent->computeAbsolutePosition();

        double offset_x = m_Parent->a_left - m_Parent->content.scrollLeft,
               offset_y = m_Parent->a_top - m_Parent->content.scrollTop;

        double maxLineHeightPreviousLine = 0, maxLineHeight = 0;

        for (elem = m_Parent->getFirstChild(); elem != NULL;
            elem = elem->m_Next) {

            if (!(elem->getFlowMode() & kFlowInlinePreviousSibling)) {
                continue;
            }

            if (prev) {
                int prevWidth = prev->visibility == CANVAS_VISIBILITY_HIDDEN ?
                                                    0 : prev->getWidth(); 

                elem->left = prev->left + prevWidth;
                elem->top = prev->top;

                if (elem->left + elem->getWidth() >  m_Parent->getWidth()) {
                    maxLineHeightPreviousLine = maxLineHeight;
                    maxLineHeight = elem->getHeight();

                    elem->top = prev->top + maxLineHeightPreviousLine;
                    elem->left = 0;
                }
            } else {
                /* The first element is aligned to the parent's top-left */
                elem->left = 0;
                elem->top = 0;
            }

            elem->a_left = elem->left + offset_x;
            elem->a_top = elem->top + offset_y;

            if (elem->getHeight() > maxLineHeight) {
                maxLineHeight = elem->getHeight();
            }

            if (elem == this) {
                break;
            }

            prev = elem;
        }

        return;
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
        if (!cur->m_Overflow) {
            
            cur->computeAbsolutePosition();
            this->computeAbsolutePosition();

            return (this->getLeft(true)+m_Width <= cur->getLeft(true) ||
                this->getTop(true)+m_Height <= cur->getTop(true)
                || this->getLeft(true) >= cur->getLeft(true) + cur->m_Width ||
                this->getTop(true) >= cur->getTop(true) + cur->m_Height);
        }
    }

    return false;
}

NativeRect NativeCanvasHandler::getViewport()
{
    NativeCanvasHandler *cur = NULL;

    for (cur = m_Parent; cur != NULL; cur = cur->m_Parent) {
        if (!cur->m_Parent) break;

        if (!cur->m_Overflow) {
            
            cur->computeAbsolutePosition();

            return {
                cur->getLeft(true),
                cur->getTop(true),
                cur->getTop(true)+cur->m_Height,
                cur->getLeft(true)+cur->m_Width
            };
        }
    }
    if (!cur) cur = this;

    return {
        cur->getLeft(),
        cur->getTop(),
        cur->getTop()+cur->m_Height,
        cur->getLeft()+cur->m_Width};
}

NativeRect NativeCanvasHandler::getVisibleRect()
{
    NativeRect vp = this->getViewport();
    this->computeAbsolutePosition();

    return {
        .fLeft   = native_min(native_max(this->getLeft(true), vp.fLeft), vp.fRight),
        .fTop    = native_min(native_max(this->getTop(true), vp.fTop), vp.fBottom),
        .fBottom = native_min(this->getTop(true)+m_Height, vp.fBottom),
        .fRight  = native_min(this->getLeft(true)+m_Width, vp.fRight)
    };
}

void NativeCanvasHandler::computeContentSize(int *cWidth, int *cHeight, bool inner)
{
    NativeCanvasHandler *cur;
    this->content.width = inner ? 0 : this->getWidth();
    this->content.height = inner ? 0 : this->getHeight();

    /* don't go further if it doesn't overflow (and not the requested handler) */
    if (!m_Overflow && /*!m_FluidHeight && */cWidth && cHeight) {
        *cWidth = this->content.width;
        *cHeight = this->content.height;
        return;
    }

    for (cur = m_Children; cur != NULL; cur = cur->m_Next) {
        if (cur->coordPosition == COORD_RELATIVE &&
            cur->visibility == CANVAS_VISIBILITY_VISIBLE) {
            
            int retWidth, retHeight;

            cur->computeContentSize(&retWidth, &retHeight, /*cur->m_FluidHeight*/ false);

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

bool NativeCanvasHandler::setFluidHeight(bool val)
{
    m_FluidHeight = val;
    return true;
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
    return (x >= getLeft(true) && x <= getLeft(true)+m_Width &&
            y >= getTop(true) && y <= getTop(true)+m_Height);
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

void NativeCanvasHandler::_jobResize(void *arg)
{
    NativeArgs *args = (NativeArgs *)arg;
    NativeCanvasHandler *handler = (NativeCanvasHandler *)args[0][0].toPtr();

    int64_t height = args[0][1].toInt64();

    /*
        Force resize even if it hasn't a fixed height
    */
    handler->setHeight(height, true);

    delete args;
}

void NativeCanvasHandler::setPendingFlags(int flags, bool append)
{
    if (!append) {
        m_Pending = 0;
    }

    m_Pending |= flags;

    if (m_Pending == 0) {
        m_NativeContext->m_CanvasPendingJobs.erase((uint64_t)this);
        return;
    }
    if (!m_NativeContext->m_CanvasPendingJobs.get((uint64_t)this)) {
        m_NativeContext->m_CanvasPendingJobs.set((uint64_t)this, this);
    }
}

void NativeCanvasHandler::execPending()
{
    if (m_Pending & kPendingResizeHeight || m_Pending & kPendingResizeWidth) {
        this->deviceSetSize(m_Width, m_Height);
    }

    this->setPendingFlags(0, false);
}

bool NativeCanvasHandler::checkLoaded()
{
    if (m_Loaded) {
        NativeArgs arg;
        this->fireEvent<NativeCanvasHandler>(LOADED_EVENT, arg, true);        
        return true;
    }
    return false;
}

void NativeCanvasHandler::propertyChanged(EventsChangedProperty property)
{
    NativeArgs arg;
    arg[0].set(property);

    switch (property) {
        case kContentWidth_Changed:
            arg[1].set(content._width);
            break;
        case kContentHeight_Changed:
            arg[1].set(content._height);
            break;
        default:
            break;
    }

    this->fireEvent<NativeCanvasHandler>(CHANGE_EVENT, arg, true);
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

    free(m_Identifier.str);

    m_NativeContext->m_CanvasPendingJobs.erase((uint64_t)this);
}
