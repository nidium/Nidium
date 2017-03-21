/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include "Graphics/CanvasHandler.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <js/GCAPI.h>

#include "Binding/JSCanvas2DContext.h"
#include "Interface/SystemInterface.h"
#include "Macros.h"

using Nidium::Core::Args;
using Nidium::Frontend::Context;
using Nidium::Frontend::InputEvent;
using Nidium::Frontend::InputTouch;
using Nidium::Frontend::InputHandler;
using Nidium::Graphics::CanvasHandler;
using Nidium::Binding::Canvas2DContext;
using Nidium::Interface::UIInterface;

namespace Nidium {
namespace Graphics {

CanvasHandler::CanvasHandler(int width,
                             int height,
                             Context *nctx,
                             bool lazyLoad)
    : m_Context(NULL), m_JsCx(nctx->getNJS()->getJSContext()), m_Left(0.0),
      m_Top(0.0), m_aLeft(0), m_aTop(0), m_Right(0.0), m_Bottom(0.0),
      m_Overflow(true), m_Parent(NULL), m_Children(NULL), m_Next(NULL),
      m_Prev(NULL), m_Last(NULL), m_Flags(0), m_nChildren(0),
      m_CoordPosition(COORD_RELATIVE), m_Visibility(CANVAS_VISIBILITY_VISIBLE),
      m_FlowMode(kFlowDoesntInteract), m_CoordMode(kLeft_Coord | kTop_Coord),
      m_Opacity(1.0), m_Zoom(1.0), m_ScaleX(1.0), m_ScaleY(1.0),
      m_AllowNegativeScroll(false), m_NidiumContext(nctx), m_Pending(0),
      m_Loaded(!lazyLoad), m_Cursor(UIInterface::ARROW)
{
    /*
        TODO: thread safe
    */
    static uint64_t g_LastIdx = 8;

    m_Identifier.idx = ++g_LastIdx;
    m_NidiumContext->m_CanvasListIdx.insert({m_Identifier.idx, this});
    m_Identifier.str = nullptr;

    m_Width     = nidium_max(width, 1);
    m_Height    = nidium_max(height, 1);
    m_MaxHeight = 0;
    m_MaxWidth  = 0;
    m_MinWidth  = 1;
    m_MinHeight = 1;

    m_FluidHeight = false;
    m_FluidWidth  = false;

    memset(&m_Margin, 0, sizeof(m_Margin));
    memset(&m_Padding, 0, sizeof(m_Padding));
    memset(&m_Translate_s, 0, sizeof(m_Translate_s));
    memset(&m_MousePosition, 0, sizeof(m_MousePosition));

    m_MousePosition.consumed = true;

    m_Content.width  = m_Width;
    m_Content.height = m_Height;

    m_Content._width  = m_Width;
    m_Content._height = m_Height;

    m_Content.scrollLeft = 0;
    m_Content.scrollTop  = 0;

    m_CoordMode = kLeft_Coord | kTop_Coord;
}

void CanvasHandler::setPositioning(CanvasHandler::COORD_POSITION mode)
{
    if (mode == COORD_INLINE) {
        mode = COORD_RELATIVE;
        m_FlowMode |= kFlowInlinePreviousSibling;
    } else if (mode == COORD_INLINEBREAK) {
        mode = COORD_RELATIVE;
        m_FlowMode |= kFlowBreakAndInlinePreviousSibling;
    } else {
        m_FlowMode &= ~kFlowBreakAndInlinePreviousSibling;
    }

    m_CoordPosition = mode;
    this->computeAbsolutePosition();
}

void CanvasHandler::setId(const char *str)
{
    if (!str) {
        return;
    }

    if (m_Identifier.str) {
        m_NidiumContext->m_CanvasList.erase(m_Identifier.str);
    }

    m_NidiumContext->m_CanvasList.set(str, this);

    if (m_Identifier.str) {
        free(m_Identifier.str);
    }

    m_Identifier.str = strdup(str);
}

void CanvasHandler::translate(double x, double y)
{
    m_Translate_s.x += x;
    m_Translate_s.y += y;
}

bool CanvasHandler::setMinWidth(int width)
{
    if (width < 1) width = 1;

    m_MinWidth = m_MaxWidth ? nidium_min(width, m_MaxWidth) : width;

    if (m_Width < m_MinWidth) {
        this->setWidth(m_MinWidth);
    }

    return true;
}

bool CanvasHandler::setMinHeight(int height)
{
    if (height < 1) height = 1;

    m_MinHeight = m_MaxHeight ? nidium_min(height, m_MaxHeight) : height;

    if (m_Height < m_MinHeight) {
        this->setHeight(m_MinHeight);
    }

    return true;
}

bool CanvasHandler::setMaxWidth(int width)
{
    if (width < 1) width = 1;

    m_MaxWidth = nidium_max(m_MinWidth, width);

    if (m_Width > m_MaxWidth) {
        this->setWidth(m_MaxWidth);
    }

    return true;
}

bool CanvasHandler::setMaxHeight(int height)
{
    if (height < 1) height = 1;

    m_MaxHeight = nidium_max(m_MinHeight, height);

    if (m_Height > m_MaxHeight) {
        this->setHeight(m_MaxHeight);
    }

    return true;
}

bool CanvasHandler::setWidth(int width, bool force)
{
    width = m_MaxWidth ? nidium_clamp(width, m_MinWidth, m_MaxWidth)
                       : nidium_max(width, m_MinWidth);

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

bool CanvasHandler::setHeight(int height, bool force)
{
    if (!force && !this->hasFixedHeight()) {
        return false;
    }

    height = m_MaxHeight ? nidium_clamp(height, m_MinHeight, m_MaxHeight)
                         : nidium_max(height, m_MinHeight);

    if (m_Height == height) {
        return true;
    }
    m_Height = height;

    this->setPendingFlags(kPendingResizeHeight);

    updateChildrenSize(false, true);

    return true;
}

void CanvasHandler::setSize(int width, int height, bool redraw)
{

    height = m_MaxHeight ? nidium_clamp(height, m_MinHeight, m_MaxHeight)
                         : nidium_max(height, m_MinHeight);

    width = m_MaxWidth ? nidium_clamp(width, m_MinWidth, m_MaxWidth)
                       : nidium_max(width, m_MinWidth);

    if (m_Height == height && m_Width == width) {
        return;
    }

    m_Width  = width;
    m_Height = height;

    this->setPendingFlags(kPendingResizeWidth | kPendingResizeHeight);

    updateChildrenSize(true, true);
}

void CanvasHandler::deviceSetSize(int width, int height)
{
    if (m_Context) {
        m_Context->setSize(width + (m_Padding.global * 2),
                           height + (m_Padding.global * 2));
    }

    Args arg;

    arg[0].set(width);
    arg[1].set(height);
    this->fireEvent<CanvasHandler>(RESIZE_EVENT, arg);
}


void CanvasHandler::updateChildrenSize(bool width, bool height)
{
    CanvasHandler *cur;

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
        // NUI_LOG("Update size of %p through parent", cur);
        cur->setSize(updateWidth ? cur->getWidth() : cur->m_Width,
                     updateHeight ? cur->getHeight() : cur->m_Height);
    }
}

void CanvasHandler::setPadding(int padding)
{
    if (padding < 0) padding = 0;

    if (padding == m_Padding.global) {
        return;
    }

    int tmppadding = m_Padding.global;

    m_Padding.global = padding;

    if (m_Context) {
        m_Context->translate(-tmppadding, -tmppadding);

        m_Context->setSize(m_Width + (m_Padding.global * 2),
                           m_Height + (m_Padding.global * 2));

        m_Context->translate(m_Padding.global, m_Padding.global);
    }
}

void CanvasHandler::setScrollLeft(int value)
{
    if (value < 0 && !m_AllowNegativeScroll) value = 0;

    m_Content.scrollLeft = value;
}

void CanvasHandler::setScrollTop(int value)
{
    if (value < 0 && !m_AllowNegativeScroll) value = 0;

    m_Content.scrollTop = value;
}

void CanvasHandler::bringToFront()
{
    if (!m_Parent) {
        return;
    }

    m_Parent->addChild(this, POSITION_FRONT);
}

void CanvasHandler::sendToBack()
{
    if (!m_Parent) {
        return;
    }

    m_Parent->addChild(this, POSITION_BACK);
}

void CanvasHandler::insertBefore(CanvasHandler *insert, CanvasHandler *ref)
{
    if (!ref || !insert) {
        this->addChild(insert, POSITION_FRONT);
        return;
    }
    if (ref->getParent() != this || ref == insert || insert == this) {
        return;
    }

    insert->removeFromParent(true);

    insert->m_Prev = ref->m_Prev;
    insert->m_Next = ref;

    if (ref->m_Prev) {
        ref->m_Prev->m_Next = insert;
        ref->m_Prev         = insert;
    } else {
        m_Children = insert;
    }

    insert->m_Parent = this;
    m_nChildren++;
}

void CanvasHandler::insertAfter(CanvasHandler *insert, CanvasHandler *ref)
{
    if (!ref) {
        this->addChild(insert, POSITION_FRONT);
        return;
    }

    this->insertBefore(insert, ref->m_Next);
}

void CanvasHandler::addChild(CanvasHandler *insert,
                             CanvasHandler::Position position)
{
    if (!insert || insert == this) {
        return;
    }
    /* Already belong to a parent? move it */
    insert->removeFromParent(true);

    switch (position) {
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
            m_Children     = insert;
            break;
    }

    insert->m_Parent = this;
    m_nChildren++;

    //Args arg;
    //insert->fireEventSync<CanvasHandler>(MOUNT_EVENT, arg);

}

void CanvasHandler::removeFromParent(bool willBeAdopted)
{
    if (!m_Parent) {
        return;
    }

    InputHandler *inputHandler = m_NidiumContext->getInputHandler();
    if (!willBeAdopted && inputHandler->getCurrentClickedHandler() == this) {
        inputHandler->setCurrentClickedHandler(nullptr);
    }

#if 0
    if (m_JsObj && JS::IsIncrementalBarrierNeeded(JS_GetRuntime(m_JsCx))) {
        printf("Barrier needed\n");
        //JS::IncrementalReferenceBarrier(m_JsObj);
    }
#endif
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

    m_Parent->m_nChildren--;
    m_Parent = NULL;
    m_Next   = NULL;
    m_Prev   = NULL;
#if 0
    Args arg;
    this->fireEventSync<CanvasHandler>(UNMOUNT_EVENT, arg);
#endif
}

void CanvasHandler::dispatchMouseEvents(LayerizeContext &layerContext)
{
    InputEvent *ev = m_NidiumContext->getInputHandler()->getEvents();
    if (ev == NULL) {
        return;
    }

    Rect actualRect;
    actualRect.m_fLeft   = m_aLeft - m_Padding.global;
    actualRect.m_fTop    = m_aTop - m_Padding.global;
    actualRect.m_fRight  = m_Width + m_aLeft;
    actualRect.m_fBottom = m_Height + m_aTop;

    if (layerContext.m_Clip) {

        if (!actualRect.intersect(layerContext.m_Clip->m_fLeft,
                                  layerContext.m_Clip->m_fTop,
                                  layerContext.m_Clip->m_fRight,
                                  layerContext.m_Clip->m_fBottom)) {

            return;
        }
    }

    ape_pool_list_t *evlist = NULL;

    for (; ev != NULL; ev = ev->m_Next) {
        if (ev->isInRect(actualRect)) {
            /*
                Increment depth (Nth canvas affected by this event)
            */
            ev->inc();

            if (!evlist) {
                evlist = ape_new_pool_list(0, 4);
            }

            InputEvent *dup = ev->dupWithHandler(this);

            ape_pool_push(evlist, dup);
        }
    }

    if (evlist) {
        ape_pool_push(&m_NidiumContext->m_CanvasEventsCanvas, evlist);
    }
}

void CanvasHandler::layerize(LayerizeContext &layerContext,
    std::vector<ComposeContext> &compList, bool draw)
{
    CanvasHandler *cur;
    Rect nclip;
    LayerSiblingContext *sctx = layerContext.m_SiblingCtx;

    if (m_Visibility == CANVAS_VISIBILITY_HIDDEN || m_Opacity == 0.0) {
        return;
    }
    int maxChildrenHeight = this->getHeight(),
        maxChildrenWidth  = this->getWidth();

    // double pzoom = this->zoom * azoom;
    double popacity = m_Opacity * layerContext.m_aOpacity;

    int tmpLeft;
    int tmpTop;

    if (m_CoordPosition == COORD_RELATIVE
        && (m_FlowMode & kFlowBreakAndInlinePreviousSibling)) {

        CanvasHandler *prev = getPrevInlineSibling();

        if (!prev) {
            m_Left = tmpLeft = m_Margin.left;
            m_Top = tmpTop = m_Margin.top;

        } else {
            int prevWidth = prev->m_Visibility == CANVAS_VISIBILITY_HIDDEN
                                ? 0
                                : prev->getWidth();

            m_Left = tmpLeft = (prev->m_Left + prevWidth + prev->m_Margin.right)
                               + m_Margin.left;
            m_Top = tmpTop = (prev->m_Top - prev->m_Margin.top) + m_Margin.top;

            if (m_Parent) {
                /*
                    Line break if :
                        - flow mode is kFlowBreakPreviousSibling (inline-break)
                   or
                        - Element would overflow-x its parent + parent doesn't
                   have a fluid width
                        - Element would overflow-x its parent + parent has a
                   fluid height but a
                   maxWidth
                */
                if ((m_FlowMode & kFlowBreakPreviousSibling)
                    || ((!m_Parent->isWidthFluid()
                         || (m_Parent->m_MaxWidth
                             && tmpLeft + this->getWidth()
                                    > m_Parent->m_MaxWidth))
                        && tmpLeft + this->getWidth() > m_Parent->getWidth())) {

                    sctx->m_MaxLineHeightPreviousLine = sctx->m_MaxLineHeight;
                    sctx->m_MaxLineHeight
                        = this->getHeight() + m_Margin.bottom + m_Margin.top;

                    tmpTop = m_Top = (prev->m_Top - prev->m_Margin.top)
                                     + sctx->m_MaxLineHeightPreviousLine
                                     + m_Margin.top;
                    tmpLeft = m_Left = m_Margin.left;
                }
            }
        }

        sctx->m_MaxLineHeight
            = nidium_max(this->getHeight() + m_Margin.bottom + m_Margin.top,
                         sctx->m_MaxLineHeight);

    } else {
        tmpLeft = this->getLeft();
        tmpTop  = this->getTop();
    }

    /*
        Fill the root layer with white
        This is the base surface on top of the window frame buffer
    */
    if (layerContext.m_Layer == NULL && m_Context) {
        layerContext.m_Layer = this;
    } else {
        bool willDraw = true;
        double cleft = 0.0, ctop = 0.0;

        if (m_CoordPosition != COORD_ABSOLUTE) {
            cleft = layerContext.m_pLeft;
            ctop  = layerContext.m_pTop;
        }

        /*
            Set the absolute position
        */
        m_aLeft = cleft + tmpLeft + m_Translate_s.x;
        m_aTop  = ctop + tmpTop + m_Translate_s.y;

        /*
            draw current context on top of the root layer
        */
        willDraw
            = (!layerContext.m_Clip || m_CoordPosition == COORD_ABSOLUTE
               || (layerContext.m_Clip->checkIntersect(
                      m_aLeft - m_Padding.global, m_aTop - m_Padding.global,
                      m_aLeft + m_Padding.global + this->getWidth(),
                      m_aTop + m_Padding.global + this->getHeight())));

        if (willDraw && !m_Loaded) {
            m_Loaded = true;
            this->checkLoaded();
        }

        if (draw && m_Context && willDraw) {

            ComposeContext compctx = {
                .handler  = this,
                .left     = m_aLeft - m_Padding.global,
                .top      = m_aTop - m_Padding.global,
                .opacity  = popacity,
                .zoom     = m_Zoom,
                .needClip = (m_CoordPosition != COORD_ABSOLUTE && layerContext.m_Clip),
                .clip     = layerContext.m_Clip ? *layerContext.m_Clip : Rect()
            };

            this->dispatchMouseEvents(layerContext);

            /* XXX: This could mutate the current state
               of the canvas since it enter the JS */

            if (m_NeedPaint) {
                m_NeedPaint = false;
                this->paint();
            }

            /*
                The JS callback could have removed
                the canvas from its parent
            */
            if (!m_Parent) {
                return;
            }

            compList.push_back(std::move(compctx));
        }
    }

    if (!m_Overflow) {
        if (layerContext.m_Clip == NULL) {
            layerContext.m_Clip            = &nclip;
            layerContext.m_Clip->m_fLeft   = m_aLeft;
            layerContext.m_Clip->m_fTop    = m_aTop;
            layerContext.m_Clip->m_fRight  = m_Width + m_aLeft;
            layerContext.m_Clip->m_fBottom = m_Height + m_aTop;
            /*
                if clip is not null, reduce it to intersect the current rect.
                /!\ clip->intersect changes "clip"
            */
        } else if (!layerContext.m_Clip->intersect(
                       m_aLeft, m_aTop, m_Width + m_aLeft, m_Height + m_aTop)
                   && (!m_FluidHeight || !m_FluidWidth)) {
            /* don't need to draw children (out of bounds) */
            return;
        }
    }

    if (m_nChildren) {
        Rect tmpClip;

        /* Save the clip */
        if (layerContext.m_Clip != NULL) {
            memcpy(&tmpClip, layerContext.m_Clip, sizeof(Rect));
        }
/* Occlusion culling */
#if 0
        CanvasHandler **culling = static_cast<CanvasHandler **>(malloc(
                                        sizeof(CanvasHandler *)
                                        * m_nChildren));

        Rect culRect;
        for (cur = last; cur != NULL; cur = cur->prev) {

        }
#endif
        struct LayerSiblingContext siblingctx;

        for (cur = m_Children; cur != NULL; cur = cur->m_Next) {
            int offsetLeft = 0, offsetTop = 0;
            if (cur->m_CoordPosition == COORD_RELATIVE) {
                offsetLeft = -m_Content.scrollLeft;
                offsetTop  = -m_Content.scrollTop;
            }

            struct LayerizeContext ctx
                = {.m_Layer = layerContext.m_Layer,
                   .m_pLeft = tmpLeft + m_Translate_s.x + layerContext.m_pLeft
                              + offsetLeft,
                   .m_pTop
                   = tmpTop + m_Translate_s.y + layerContext.m_pTop + offsetTop,
                   .m_aOpacity   = popacity,
                   .m_aZoom      = m_Zoom,
                   .m_Clip       = layerContext.m_Clip,
                   .m_SiblingCtx = &siblingctx };

            cur->layerize(ctx, compList, draw);

            /*
                Incrementaly check the bottom/right most children
                in order to compute the contentHeight/Width
            */
            if (cur->m_CoordPosition == COORD_RELATIVE
                && cur->m_Visibility == CANVAS_VISIBILITY_VISIBLE) {

                int actualChildrenHeightPlusTop
                    = cur->getTop() + (cur->m_Overflow ? cur->m_Content._height
                                                       : cur->getHeight());
                int actualChildrenWidthPlusLeft
                    = cur->getLeft() + (cur->m_Overflow ? cur->m_Content._width
                                                        : cur->getWidth());

                if (actualChildrenHeightPlusTop > maxChildrenHeight) {
                    maxChildrenHeight = actualChildrenHeightPlusTop;
                }
                if (actualChildrenWidthPlusLeft > maxChildrenWidth) {
                    maxChildrenWidth = actualChildrenWidthPlusLeft;
                }
            }
            /* restore the old clip (layerize could have altered it) */
            if (layerContext.m_Clip != NULL) {
                memcpy(layerContext.m_Clip, &tmpClip, sizeof(Rect));
            }
        }
    }
    if (m_Content._height != maxChildrenHeight) {
        m_Content._height = maxChildrenHeight;

        this->propertyChanged(kContentHeight_Changed);
    }
    if (m_Content._width != maxChildrenWidth) {
        m_Content._width = maxChildrenWidth;

        this->propertyChanged(kContentWidth_Changed);
    }

    /*
        Height is dynamic.
        It's automatically adjusted by the height of its content
    */
    if (m_FluidHeight) {
        int contentHeight = this->getContentHeight(true);

        int newHeight = m_MaxHeight ? nidium_clamp(contentHeight, m_MinHeight,
                                                   m_MaxHeight)
                                    : nidium_max(contentHeight, m_MinHeight);

        if (m_Height != newHeight) {
            this->setHeight(newHeight, true);
        }
    }

    if (m_FluidWidth) {
        int contentWidth = this->getContentWidth(true);

        int newWidth = m_MaxWidth
                           ? nidium_clamp(contentWidth, m_MinWidth, m_MaxWidth)
                           : nidium_max(contentWidth, m_MinWidth);

        if (m_Width != newWidth) {
            this->setWidth(newWidth, true);
        }
    }

    if (layerContext.m_Layer == this) {
        m_MousePosition.consumed = true;
        m_MousePosition.xrel     = 0;
        m_MousePosition.yrel     = 0;
    }
}

// {{{ Getters
int CanvasHandler::getContentWidth(bool inner)
{
    this->computeContentSize(NULL, NULL, inner);

    return m_Content.width;
}

int CanvasHandler::getContentHeight(bool inner)
{
    this->computeContentSize(NULL, NULL, inner);

    return m_Content.height;
}

/* TODO: optimize tail recursion? */
bool CanvasHandler::hasAFixedAncestor() const
{
    if (m_CoordPosition == COORD_FIXED) {
        return true;
    }
    return (m_Parent ? m_Parent->hasAFixedAncestor() : false);
}

/* Compute whether or the canvas is going to be drawn */
bool CanvasHandler::isDisplayed() const
{
    if (m_Visibility == CANVAS_VISIBILITY_HIDDEN) {
        return false;
    }

    return (m_Parent ? m_Parent->isDisplayed() : true);
}

void CanvasHandler::computeAbsolutePosition()
{
    if (m_CoordPosition == COORD_ABSOLUTE) {
        m_aTop  = this->getTop();
        m_aLeft = this->getLeft();
        return;
    }

    if (m_CoordPosition == COORD_RELATIVE
        && (m_FlowMode & kFlowBreakAndInlinePreviousSibling)) {

        if (m_Parent == NULL) {
            m_aTop = m_aLeft = 0;
            return;
        }

        CanvasHandler *elem, *prev = NULL;

        m_Parent->computeAbsolutePosition();

        double offset_x = m_Parent->m_aLeft - m_Parent->m_Content.scrollLeft,
               offset_y = m_Parent->m_aTop - m_Parent->m_Content.scrollTop;

        double maxLineHeightPreviousLine = 0, maxLineHeight = 0;

        for (elem = m_Parent->getFirstChild(); elem != NULL;
             elem = elem->m_Next) {

            if (!(elem->getFlowMode() & kFlowInlinePreviousSibling)) {
                continue;
            }

            if (prev) {
                int prevWidth = prev->m_Visibility == CANVAS_VISIBILITY_HIDDEN
                                    ? 0
                                    : prev->getWidth();

                elem->m_Left = (prev->m_Left + prevWidth + prev->m_Margin.right)
                               + elem->m_Margin.left;
                elem->m_Top
                    = (prev->m_Top - prev->m_Margin.top) + elem->m_Margin.top;

                if ((elem->m_FlowMode & kFlowBreakPreviousSibling)
                    || ((!m_Parent->isWidthFluid()
                         || (m_Parent->m_MaxWidth
                             && elem->m_Left + elem->getWidth()
                                    > m_Parent->m_MaxWidth))
                        && elem->m_Left + elem->getWidth()
                               > m_Parent->getWidth())) {

                    maxLineHeightPreviousLine = maxLineHeight;
                    maxLineHeight             = elem->getHeight() + elem->m_Margin.bottom
                                    + elem->m_Margin.top;

                    elem->m_Top = (prev->m_Top - prev->m_Margin.top)
                                  + maxLineHeightPreviousLine
                                  + elem->m_Margin.top;
                    elem->m_Left = elem->m_Margin.left;
                }
            } else {
                /* The first element is aligned to the parent's top-left */
                elem->m_Left = elem->m_Margin.left;
                elem->m_Top  = elem->m_Margin.top;
            }

            elem->m_aLeft = elem->m_Left + offset_x;
            elem->m_aTop  = elem->m_Top + offset_y;

            maxLineHeight = nidium_max(elem->getHeight() + elem->m_Margin.bottom
                                           + elem->m_Margin.top,
                                       maxLineHeight);

            if (elem == this) {
                break;
            }

            prev = elem;
        }

        return;
    }

    double ctop = this->getTopScrolled(), cleft = this->getLeftScrolled();

    CanvasHandler *cparent = m_Parent;

    while (cparent != NULL) {

        ctop += cparent->getTopScrolled();
        cleft += cparent->getLeftScrolled();

        if (cparent->m_CoordPosition == COORD_ABSOLUTE) {
            break;
        }

        cparent = cparent->getParent();
    }

    m_aTop  = ctop;
    m_aLeft = cleft;
}

bool CanvasHandler::isOutOfBound()
{
    if (!m_Parent) {
        return false;
    }

    CanvasHandler *cur;

    for (cur = m_Parent; cur != NULL; cur = cur->m_Parent) {
        if (!cur->m_Overflow) {

            cur->computeAbsolutePosition();
            this->computeAbsolutePosition();

            return (
                this->getLeft(true) + getWidth() <= cur->getLeft(true)
                || this->getTop(true) + getHeight() <= cur->getTop(true)
                || this->getLeft(true) >= cur->getLeft(true) + cur->getWidth()
                || this->getTop(true) >= cur->getTop(true) + cur->getHeight());
        }
    }

    return false;
}

Rect CanvasHandler::getViewport()
{
    CanvasHandler *cur = NULL;

    for (cur = m_Parent; cur != NULL; cur = cur->m_Parent) {
        if (!cur->m_Parent) break;

        if (!cur->m_Overflow) {

            cur->computeAbsolutePosition();

            Rect rect = { cur->getLeft(true), cur->getTop(true),
                          cur->getTop(true) + cur->getHeight(),
                          cur->getLeft(true) + cur->getWidth() };

            Rect prect = m_Parent->getViewport();

            rect.intersect(prect.m_fLeft, prect.m_fTop, prect.m_fRight,
                           prect.m_fBottom);

            return rect;
        }
    }
    if (!cur) cur = this;

    return { cur->getLeft(true), cur->getTop(true),
             cur->getTop(true) + cur->getHeight(),
             cur->getLeft(true) + cur->getWidth() };
}

Rect CanvasHandler::getVisibleRect()
{
    Rect vp = this->getViewport();
    this->computeAbsolutePosition();

    return {
        .m_fLeft
        = nidium_min(nidium_max(this->getLeft(true), vp.m_fLeft), vp.m_fRight),
        .m_fTop
        = nidium_min(nidium_max(this->getTop(true), vp.m_fTop), vp.m_fBottom),
        .m_fBottom = nidium_min(this->getTop(true) + getHeight(), vp.m_fBottom),
        .m_fRight  = nidium_min(this->getLeft(true) + getWidth(), vp.m_fRight)
    };
}

void CanvasHandler::computeContentSize(int *cWidth, int *cHeight, bool inner)
{
    CanvasHandler *cur;
    m_Content.width  = inner ? 0 : this->getWidth();
    m_Content.height = inner ? 0 : this->getHeight();

    /* don't go further if it doesn't overflow (and not the requested handler)
     */
    if (!m_Overflow && /*!m_FluidHeight && */ cWidth && cHeight) {
        *cWidth  = m_Content.width;
        *cHeight = m_Content.height;
        return;
    }

    for (cur = m_Children; cur != NULL; cur = cur->m_Next) {
        if (cur->m_CoordPosition == COORD_RELATIVE
            && cur->m_Visibility == CANVAS_VISIBILITY_VISIBLE) {

            int retWidth, retHeight;

            cur->computeContentSize(&retWidth, &retHeight,
                                    /*cur->m_FluidHeight*/ false);

            if (retWidth + cur->getLeft() > m_Content.width) {
                m_Content.width = retWidth + cur->getLeft();
            }
            if (retHeight + cur->getTop() > m_Content.height) {
                m_Content.height = retHeight + cur->getTop();
            }
        }
    }
    if (cWidth) *cWidth = m_Content.width;
    if (cHeight) *cHeight = m_Content.height;
}


bool CanvasHandler::isHidden() const
{
    return (m_Visibility == CANVAS_VISIBILITY_HIDDEN);
}

void CanvasHandler::getChildren(CanvasHandler **out) const
{
    CanvasHandler *cur;
    int i = 0;
    for (cur = m_Children; cur != NULL; cur = cur->m_Next) {
        out[i++] = cur;
    }
}

int CanvasHandler::getCursor()
{
    if (m_Cursor != UIInterface::ARROW) {
        return m_Cursor;
    }

    /* Inherit from parent when default */
    return m_Parent ? m_Parent->getCursor() : UIInterface::ARROW;
}
// }}}

// {{{ Setters
void CanvasHandler::setCursor(int cursor)
{
    m_Cursor = cursor;
    Nidium::Interface::__NidiumUI->setCursor(
        (UIInterface::CURSOR_TYPE) this->getCursor());
}

void CanvasHandler::setHidden(bool val)
{
    m_Visibility = (val ? CANVAS_VISIBILITY_HIDDEN : CANVAS_VISIBILITY_VISIBLE);
}

void CanvasHandler::setOpacity(double val)
{
    val = nidium_min(1, nidium_max(0, val));

    m_Opacity = val;
}

void CanvasHandler::setZoom(double zoom)
{
    m_Zoom = zoom;
}
// }}}

void CanvasHandler::setScale(double x, double y)
{
    this->recursiveScale(x, y, m_ScaleX, m_ScaleY);
    m_ScaleX = x;
    m_ScaleY = y;
}

void CanvasHandler::setContext(CanvasContext *context)
{
    m_Context = context;
    m_Context->translate(m_Padding.global, m_Padding.global);
}

bool CanvasHandler::setFluidHeight(bool val)
{
    m_FluidHeight = val;
    return true;
}

bool CanvasHandler::setFluidWidth(bool val)
{
    m_FluidWidth = val;
    return true;
}
// }}}

void CanvasHandler::recursiveScale(double x, double y, double oldX, double oldY)
{
    CanvasHandler *cur = this;

    if (!cur->m_Context) return;

    cur->m_Context->setScale(x, y, oldX, oldY);

    for (cur = m_Children; cur != NULL; cur = cur->m_Next) {
        cur->recursiveScale(x, y, oldX, oldY);
    }
}

int32_t CanvasHandler::countChildren() const
{
    return m_nChildren;
}

bool CanvasHandler::containsPoint(double x, double y) const
{
    return (x >= getLeft(true) && x <= getLeft(true) + m_Width
            && y >= getTop(true) && y <= getTop(true) + m_Height);
}

void CanvasHandler::unrootHierarchy()
{
#if 0
    CanvasHandler *cur;

    for (cur = children; cur != NULL; cur = cur->next) {
        cur->unrootHierarchy();
        if (cur->context && cur->context->m_JsObj && cur->context->m_JsCx) {
            JS::RemoveObjectRoot(cur->context->m_JsCx, &cur->context->m_JsObj);
        }
        if (cur->m_JsObj) {
            JS::RemoveObjectRoot(cur->m_JsCx, &cur->m_JsObj);
        }
        cur->m_JsObj = NULL;
        cur->context->m_JsObj = NULL;
    }
    children = NULL;
#endif
}

void CanvasHandler::_JobResize(void *arg)
{
    Args *args             = (Args *)arg;
    CanvasHandler *handler = static_cast<CanvasHandler *>(args[0][0].toPtr());

    int64_t height = args[0][1].toInt64();

    /*
        Force resize even if it hasn't a fixed height
    */
    handler->setHeight(height, true);

    delete args;
}

void CanvasHandler::setPendingFlags(int flags, bool append)
{
    if (!append) {
        m_Pending = 0;
    }

    m_Pending |= flags;

    if (m_Pending == 0) {
        m_NidiumContext->m_CanvasPendingJobs.erase((uint64_t) this);
        return;
    }
    if (!m_NidiumContext->m_CanvasPendingJobs.get((uint64_t) this)) {
        m_NidiumContext->m_CanvasPendingJobs.set((uint64_t) this, this);
    }
}

void CanvasHandler::execPending()
{
    if ((m_Pending & kPendingResizeHeight)
        || (m_Pending & kPendingResizeWidth)) {
        this->deviceSetSize(m_Width, m_Height);
    }

    this->setPendingFlags(0, false);
}

bool CanvasHandler::checkLoaded(bool async)
{
    if (m_Loaded) {
        Args arg;

        if (async) {
            this->fireEvent<CanvasHandler>(LOADED_EVENT, arg, true);

            return true;
        }

        this->fireEventSync<CanvasHandler>(LOADED_EVENT, arg);
        return true;
    }

    return false;
}

void CanvasHandler::paint()
{
    Args arg;
    this->fireEventSync<CanvasHandler>(PAINT_EVENT, arg);
}

void CanvasHandler::propertyChanged(EventsChangedProperty property)
{
    Args arg;
    arg[0].set(property);

    switch (property) {
        case kContentWidth_Changed:
            arg[1].set(m_Content._width);
            break;
        case kContentHeight_Changed:
            arg[1].set(m_Content._height);
            break;
        default:
            break;
    }

    this->fireEvent<CanvasHandler>(CHANGE_EVENT, arg, true);
}

// {{{ Events
void CanvasHandler::onDrag(InputEvent *ev, CanvasHandler *target, bool end)
{
    Args arg;

    if (!end) {
        arg[0].set((m_Flags & kDrag_Flag) == 0
                       ? InputEvent::kMouseDragStart_Type
                       : InputEvent::kMouseDrag_Type);
    } else {
        arg[0].set(InputEvent::kMouseDragEnd_Type);
    }

    arg[1].set(ev->m_x);
    arg[2].set(ev->m_y);
    arg[3].set(ev->m_data[0]);
    arg[4].set(ev->m_data[1]);
    arg[5].set(ev->m_x - m_aLeft); // layerX
    arg[6].set(ev->m_y - m_aTop);  // layerY
    arg[7].set(target);            // target

    if (!end && (m_Flags & kDrag_Flag) == 0) {
        m_Flags |= kDrag_Flag;
    }

    this->fireEvent<CanvasHandler>(CanvasHandler::MOUSE_EVENT, arg);

    if (!end) {
        arg[0].set(InputEvent::kMouseDragOver_Type);
        arg[7].set(this); // source

        target->fireEvent<CanvasHandler>(CanvasHandler::MOUSE_EVENT, arg);
    }
}

void CanvasHandler::onDrop(InputEvent *ev, CanvasHandler *drop)
{
    Args arg;
    arg[0].set(InputEvent::kMouseDrop_Type);
    arg[1].set(ev->m_x);
    arg[2].set(ev->m_y);
    arg[3].set((int64_t)0);
    arg[4].set((int64_t)0);
    arg[5].set(ev->m_x - m_aLeft); // layerX
    arg[6].set(ev->m_y - m_aTop);  // layerY
    arg[7].set(drop);

    this->fireEvent<CanvasHandler>(CanvasHandler::MOUSE_EVENT, arg);
}

void CanvasHandler::onMouseEvent(InputEvent *ev)
{
    InputHandler *inputHandler = m_NidiumContext->getInputHandler();
    CanvasHandler *underneath = this;

    if (InputEvent *tmpEvent = ev->getEventForNextCanvas()) {
        underneath = tmpEvent->m_Handler;
    }

    switch (ev->getType()) {
        case InputEvent::kTouchMove_Type:
            /*
                If the touchmove event is received on an handler outside of the
                origin handlers fire the touchmove event of the original handler.
            */
            if (!ev->getTouch()->hasOrigin(this)) {
                Args arg;
                this->onTouch(ev, arg, nullptr);
                ev->getTouch()->getTarget()->fireEvent<CanvasHandler>(CanvasHandler::TOUCH_EVENT, arg);
            }
            break;
        case InputEvent::kMouseClick_Type:
            if (ev->m_data[0] == 1) // left click
                inputHandler->setCurrentClickedHandler(this);
            break;
        case InputEvent::kMouseClickRelease_Type:
            if (ev->m_data[0] == 1) {
                CanvasHandler *drag;
                if ((drag = inputHandler->getCurrentClickedHandler())
                    && (drag->m_Flags & kDrag_Flag)) {

                    CanvasHandler *target = (drag == this) ? underneath : this;

                    drag->onDrag(ev, target, true);
                    target->onDrop(ev, drag);

                    drag->m_Flags &= ~kDrag_Flag;
                }
                inputHandler->setCurrentClickedHandler(NULL);
            }
            break;
        case InputEvent::kMouseMove_Type: {
            CanvasHandler *drag;
            if ((drag = inputHandler->getCurrentClickedHandler())) {

                drag->onDrag(ev, (this == drag) ? underneath : this);
            }
            break;
        }
        default:
            break;
    }

    Nidium::Interface::__NidiumUI->setCursor(
        (UIInterface::CURSOR_TYPE) this->getCursor());
}

void CanvasHandler::onTouch(InputEvent *ev, Args &args, CanvasHandler *handler)
{
    InputHandler *inputHandler = m_NidiumContext->getInputHandler();
    std::shared_ptr<InputTouch> touch = ev->getTouch();

    if (ev->getType() == InputEvent::kTouchStart_Type) {
        if (!inputHandler->getTouch(ev->getTouch()->getTouchID())) {
            inputHandler->addTouch(ev->getTouch());
        }
        touch->addOrigin(handler);
    } else if (ev->getType() == InputEvent::kTouchEnd_Type) {
        inputHandler->rmTouch(touch->getIdentifier());
    }

    inputHandler->addChangedTouch(touch);

    args[0].set(ev->getType());
    args[1].set(touch.get());

    /*
        Update touch coordinates when they are processed
    */
    touch->x    = ev->m_x;
    touch->y    = ev->m_y;
}

/*
    Called by Context whenever there are pending events on this canvas
    Currently only handle mouse & touch events.
*/
bool CanvasHandler::_handleEvent(InputEvent *ev)
{
    Events canvasEvent = MOUSE_EVENT;

    for (CanvasHandler *handler = this; handler != NULL;
         handler = handler->getParent()) {

        Args arg;

        switch (ev->getType()) {
            case InputEvent::kMouseMove_Type:
            case InputEvent::kMouseClick_Type:
            case InputEvent::kMouseClickRelease_Type:
            case InputEvent::kMouseDoubleClick_Type:
            case InputEvent::kMouseDragStart_Type:
            case InputEvent::kMouseDragEnd_Type:
            case InputEvent::kMouseDragOver_Type:
            case InputEvent::kMouseDrop_Type:
            case InputEvent::kMouseDrag_Type:
            case InputEvent::kMouseWheel_Type:
                arg[0].set(ev->getType());
                arg[1].set(ev->m_x);
                arg[2].set(ev->m_y);
                arg[3].set(ev->m_data[0]);     // xrel
                arg[4].set(ev->m_data[1]);     // yrel
                arg[5].set(ev->m_x - m_aLeft); // layerX
                arg[6].set(ev->m_y - m_aTop);  // layerY
                arg[7].set(this);              // target
                break;
            case InputEvent::kTouchStart_Type:
            case InputEvent::kTouchEnd_Type:
            case InputEvent::kTouchMove_Type:
                /*
                    If the handler isn't one of the handlers that
                    received the touchstart event ignore it.
                 */
                if (!ev->getTouch()->hasOrigin(handler)) {
                    continue;
                }

                canvasEvent = TOUCH_EVENT;

                this->onTouch(ev, arg, handler);
                break;
        }

        /* fireEvent returns false if a stopPropagation is detected */
        if (!handler->fireEvent<CanvasHandler>(canvasEvent, arg)) {
            break;
        }
    }

    this->onMouseEvent(ev);

    return true;
}
// }}}


CanvasHandler::~CanvasHandler()
{
    CanvasHandler *cur = m_Children, *cnext;

    removeFromParent();

    /* all children got orphaned :(*/
    while (cur != NULL) {
        // printf("Warning: a canvas got orphaned (%p)\n", cur);
        cnext = cur->m_Next;
        cur->removeFromParent();
        cur = cnext;
    }

    m_NidiumContext->m_CanvasListIdx.erase(m_Identifier.idx);

    if (m_Identifier.str) {
        m_NidiumContext->m_CanvasList.erase(m_Identifier.str);
        free(m_Identifier.str);
    }

    m_NidiumContext->m_CanvasPendingJobs.erase((uint64_t) this);
}

} // namespace Graphics
} // namespace Nidium
