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

using Nidium::Core::Args;
using Nidium::Frontend::Context;
using Nidium::Frontend::InputEvent;
using Nidium::Graphics::CanvasHandler;
using Nidium::Binding::Canvas2DContext;
using Nidium::Interface::UIInterface;

namespace Nidium {
namespace Graphics {

CanvasHandler::CanvasHandler(int width,
                             int height,
                             Context *nctx,
                             bool lazyLoad)
    : m_Context(NULL), m_JsCx(nctx->getNJS()->getJSContext()), m_Right(0.0), m_Bottom(0.0),
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

    p_Width     = nidium_max(width, 1);
    p_Height    = nidium_max(height, 1);

    m_FluidHeight = false;
    m_FluidWidth  = false;

    memset(&m_Margin, 0, sizeof(m_Margin));
    memset(&m_MousePosition, 0, sizeof(m_MousePosition));

    m_MousePosition.consumed = true;

    m_Content.width  = p_Width;
    m_Content.height = p_Height;

    m_Content._width  = p_Width;
    m_Content._height = p_Height;

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

void CanvasHandler::setPropMinWidth(int width)
{
    if (width < 1) width = 1;

    p_MinWidth = p_MaxWidth ? nidium_min(width, p_MaxWidth) : width;

    if (p_Width < p_MinWidth) {
        this->setWidth(p_MinWidth);
    }
}

void CanvasHandler::setPropMinHeight(int height)
{
    if (height < 1) height = 1;

    p_MinHeight = p_MaxHeight ? nidium_min(height, p_MaxHeight) : height;

    if (p_Height < p_MinHeight) {
        this->setHeight(p_MinHeight);
    }
}

void CanvasHandler::setPropMaxWidth(int width)
{
    if (width < 1) width = 1;

    p_MaxWidth = nidium_max(p_MinWidth, width);

    if (p_Width > p_MaxWidth) {
        this->setWidth(p_MaxWidth);
    }
}

void CanvasHandler::setPropMaxHeight(int height)
{
    if (height < 1) height = 1;

    p_MaxHeight = nidium_max(p_MinHeight, height);

    if (p_Height > p_MaxHeight) {
        this->setHeight(p_MaxHeight);
    }
}

bool CanvasHandler::setWidth(int width, bool force)
{
    width = p_MaxWidth ? nidium_clamp(width, p_MinWidth, p_MaxWidth)
                       : nidium_max(width, p_MinWidth);

    if (!force && !this->hasFixedWidth()) {
        return false;
    }

    if (p_Width == width) {
        return true;
    }

    p_Width = width;

    this->setPendingFlags(kPendingResizeWidth);

    updateChildrenSize(true, false);

    return true;
}

bool CanvasHandler::setHeight(int height, bool force)
{
    if (!force && !this->hasFixedHeight()) {
        return false;
    }

    height = p_MaxHeight ? nidium_clamp(height, p_MinHeight, p_MaxHeight)
                         : nidium_max(height, p_MinHeight);

    if (p_Height == height) {
        return true;
    }
    p_Height = height;

    this->setPendingFlags(kPendingResizeHeight);

    updateChildrenSize(false, true);

    return true;
}

void CanvasHandler::setSize(int width, int height, bool redraw)
{

    height = p_MaxHeight ? nidium_clamp(height, p_MinHeight, p_MaxHeight)
                         : nidium_max(height, p_MinHeight);

    width = p_MaxWidth ? nidium_clamp(width, p_MinWidth, p_MaxWidth)
                       : nidium_max(width, p_MinWidth);

    if (p_Height == height && p_Width == width) {
        return;
    }

    p_Width  = width;
    p_Height = height;

    this->setPendingFlags(kPendingResizeWidth | kPendingResizeHeight);

    updateChildrenSize(true, true);
}

void CanvasHandler::deviceSetSize(int width, int height)
{
    if (m_Context) {
        m_Context->setSize(width + (p_Coating * 2),
                           height + (p_Coating * 2));
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
        // ndm_printf("Update size of %p through parent", cur);
        cur->setSize(updateWidth ? cur->getPropWidth() : cur->p_Width,
                     updateHeight ? cur->getPropHeight() : cur->p_Height);
    }
}

void CanvasHandler::setPropCoating(unsigned int coating)
{
    if (coating == p_Coating) {
        return;
    }

    int tmppadding = p_Coating;

    p_Coating = coating;

    if (m_Context) {
        m_Context->translate(-tmppadding, -tmppadding);

        m_Context->setSize(p_Width + (p_Coating * 2),
                           p_Height + (p_Coating * 2));

        m_Context->translate(p_Coating, p_Coating);
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

    if (!willBeAdopted && m_NidiumContext->getCurrentClickedHandler() == this) {
        m_NidiumContext->setCurrentClickedHandler(NULL);
    }

#if 0
    if (m_JsObj && JS::IsIncrementalBarrierNeeded(JS_GetRuntime(m_JsCx))) {
        ndm_log(NDM_LOG_DEBUG, "CanvasHandler", "Barrier needed");
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

/*
    dispatch all unprocessed mouse event to |this| canvas.
    This is called for every drawn canvas at every frame
*/
void CanvasHandler::dispatchMouseEvents(LayerizeContext &layerContext)
{
    InputEvent *ev = m_NidiumContext->getInputHandler()->getEvents();

    if (ev == NULL) {
        return;
    }

    Rect actualRect;
    actualRect.m_fLeft   = p_Left.getAlternativeValue() - p_Coating;
    actualRect.m_fTop    = p_Top.getAlternativeValue() - p_Coating;
    actualRect.m_fRight  = p_Width + p_Left.getAlternativeValue();
    actualRect.m_fBottom = p_Height + p_Top.getAlternativeValue();

    if (layerContext.m_Clip) {

        if (!actualRect.intersect(layerContext.m_Clip->m_fLeft,
                                  layerContext.m_Clip->m_fTop,
                                  layerContext.m_Clip->m_fRight,
                                  layerContext.m_Clip->m_fBottom)) {

            return;
        }
    }

    /*
        |evlist| is the list of event regarding |this| canvas.
    */
    ape_pool_list_t *evlist = NULL;

    /*
        Loop through all new events
    */
    for (; ev != NULL; ev = ev->m_Next) {
        /* This event is happening in a zone inside |this| canvas  */
        if (ev->isInRect(actualRect)) {
            /*
                Increment depth (Nth canvas affected by this event)
            */
            ev->inc();

            if (!evlist) {
                evlist = ape_new_pool_list(0, 4);
            }

            /* 
               Dupplicate the event and set |this|
               as the handler of the new event 
            */
            InputEvent *dup = ev->dupWithHandler(this);

            ape_pool_push(evlist, dup);
        }
    }


    if (evlist) {
        /*
            m_NidiumContext->m_CanvasEventsCanvas is a set of all lists
        */
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
    int maxChildrenHeight = this->getPropHeight(),
        maxChildrenWidth  = this->getPropWidth();

    // double pzoom = this->zoom * azoom;
    double popacity = m_Opacity * layerContext.m_aOpacity;

    int tmpLeft;
    int tmpTop;

    if (m_CoordPosition == COORD_RELATIVE
        && (m_FlowMode & kFlowBreakAndInlinePreviousSibling)) {

        CanvasHandler *prev = getPrevInlineSibling();

        if (!prev) {
            p_Left = tmpLeft = m_Margin.left;
            p_Top = tmpTop = m_Margin.top;

        } else {
            int prevWidth = prev->m_Visibility == CANVAS_VISIBILITY_HIDDEN
                                ? 0
                                : prev->getPropWidth();

            p_Left = tmpLeft = (prev->p_Left + prevWidth + prev->m_Margin.right)
                               + m_Margin.left;
            p_Top = tmpTop = (prev->p_Top - prev->m_Margin.top) + m_Margin.top;

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
                         || (m_Parent->p_MaxWidth
                             && tmpLeft + this->getPropWidth()
                                    > m_Parent->p_MaxWidth))
                        && tmpLeft + this->getPropWidth() > m_Parent->getPropWidth())) {

                    sctx->m_MaxLineHeightPreviousLine = sctx->m_MaxLineHeight;
                    sctx->m_MaxLineHeight
                        = this->getPropHeight() + m_Margin.bottom + m_Margin.top;

                    tmpTop = p_Top = (prev->p_Top - prev->m_Margin.top)
                                     + sctx->m_MaxLineHeightPreviousLine
                                     + m_Margin.top;
                    tmpLeft = p_Left = m_Margin.left;
                }
            }
        }

        sctx->m_MaxLineHeight
            = nidium_max(this->getPropHeight() + m_Margin.bottom + m_Margin.top,
                         sctx->m_MaxLineHeight);

    } else {
        tmpLeft = this->getPropLeft();
        tmpTop  = this->getPropTop();
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
        p_Left.setAlternativeValue(cleft + tmpLeft);
        p_Top.setAlternativeValue(ctop + tmpTop);

        /*
            draw current context on top of the root layer
        */
        willDraw
            = (!layerContext.m_Clip || m_CoordPosition == COORD_ABSOLUTE
               || (layerContext.m_Clip->checkIntersect(
                      p_Left.getAlternativeValue() - p_Coating, p_Top.getAlternativeValue() - p_Coating,
                      p_Left.getAlternativeValue() + p_Coating + this->getPropWidth(),
                      p_Top.getAlternativeValue() + p_Coating + this->getPropHeight())));

        if (willDraw && !m_Loaded) {
            m_Loaded = true;
            this->checkLoaded();
        }

        if (draw && m_Context && willDraw) {

            ComposeContext compctx = {
                .handler  = this,
                .left     = p_Left.getAlternativeValue() - p_Coating,
                .top      = p_Top.getAlternativeValue() - p_Coating,
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
            layerContext.m_Clip->m_fLeft   = p_Left.getAlternativeValue();
            layerContext.m_Clip->m_fTop    = p_Top.getAlternativeValue();
            layerContext.m_Clip->m_fRight  = p_Width + p_Left.getAlternativeValue();
            layerContext.m_Clip->m_fBottom = p_Height + p_Top.getAlternativeValue();
            /*
                if clip is not null, reduce it to intersect the current rect.
                /!\ clip->intersect changes "clip"
            */
        } else if (!layerContext.m_Clip->intersect(
                       p_Left.getAlternativeValue(), p_Top.getAlternativeValue(), p_Width + p_Left.getAlternativeValue(), p_Height + p_Top.getAlternativeValue())
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
                   .m_pLeft = tmpLeft + layerContext.m_pLeft
                              + offsetLeft,
                   .m_pTop
                   = tmpTop + layerContext.m_pTop + offsetTop,
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
                    = cur->getPropTop() + (cur->m_Overflow ? cur->m_Content._height
                                                       : cur->getPropHeight());
                int actualChildrenWidthPlusLeft
                    = cur->getPropLeft() + (cur->m_Overflow ? cur->m_Content._width
                                                        : cur->getPropWidth());

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

        int newHeight = p_MaxHeight ? nidium_clamp(contentHeight, p_MinHeight,
                                                   p_MaxHeight)
                                    : nidium_max(contentHeight, p_MinHeight);

        if (p_Height != newHeight) {
            this->setHeight(newHeight, true);
        }
    }

    if (m_FluidWidth) {
        int contentWidth = this->getContentWidth(true);

        int newWidth = p_MaxWidth
                           ? nidium_clamp(contentWidth, p_MinWidth, p_MaxWidth)
                           : nidium_max(contentWidth, p_MinWidth);

        if (p_Width != newWidth) {
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
        p_Top.setAlternativeValue(this->getPropTop());
        p_Left.setAlternativeValue(this->getPropLeft());
        return;
    }

    if (m_CoordPosition == COORD_RELATIVE
        && (m_FlowMode & kFlowBreakAndInlinePreviousSibling)) {

        if (m_Parent == NULL) {
            p_Top.setAlternativeValue(0);
            p_Left.setAlternativeValue(0);
            return;
        }

        CanvasHandler *elem, *prev = NULL;

        m_Parent->computeAbsolutePosition();

        double offset_x = m_Parent->p_Left.getAlternativeValue() - m_Parent->m_Content.scrollLeft,
               offset_y = m_Parent->p_Top.getAlternativeValue() - m_Parent->m_Content.scrollTop;

        double maxLineHeightPreviousLine = 0, maxLineHeight = 0;

        for (elem = m_Parent->getFirstChild(); elem != NULL;
             elem = elem->m_Next) {

            if (!(elem->getFlowMode() & kFlowInlinePreviousSibling)) {
                continue;
            }

            if (prev) {
                int prevWidth = prev->m_Visibility == CANVAS_VISIBILITY_HIDDEN
                                    ? 0
                                    : prev->getPropWidth();

                elem->p_Left = (prev->p_Left + prevWidth + prev->m_Margin.right)
                               + elem->m_Margin.left;
                elem->p_Top
                    = (prev->p_Top - prev->m_Margin.top) + elem->m_Margin.top;

                if ((elem->m_FlowMode & kFlowBreakPreviousSibling)
                    || ((!m_Parent->isWidthFluid()
                         || (m_Parent->p_MaxWidth
                             && elem->p_Left + elem->getPropWidth()
                                    > m_Parent->p_MaxWidth))
                        && elem->p_Left + elem->getPropWidth()
                               > m_Parent->getPropWidth())) {

                    maxLineHeightPreviousLine = maxLineHeight;
                    maxLineHeight             = elem->getPropHeight() + elem->m_Margin.bottom
                                    + elem->m_Margin.top;

                    elem->p_Top = (prev->p_Top - prev->m_Margin.top)
                                  + maxLineHeightPreviousLine
                                  + elem->m_Margin.top;
                    elem->p_Left = elem->m_Margin.left;
                }
            } else {
                /* The first element is aligned to the parent's top-left */
                elem->p_Left = elem->m_Margin.left;
                elem->p_Top  = elem->m_Margin.top;
            }

            elem->p_Left.setAlternativeValue(elem->p_Left + offset_x);
            elem->p_Top.setAlternativeValue(elem->p_Top + offset_y);

            maxLineHeight = nidium_max(elem->getPropHeight() + elem->m_Margin.bottom
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

    p_Top.setAlternativeValue(ctop);
    p_Left.setAlternativeValue(cleft);
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
                this->getPropLeftAbsolute() + getPropWidth() <= cur->getPropLeftAbsolute()
                || this->getPropTopAbsolute() + getPropHeight() <= cur->getPropTopAbsolute()
                || this->getPropLeftAbsolute() >= cur->getPropLeftAbsolute() + cur->getPropWidth()
                || this->getPropTopAbsolute() >= cur->getPropTopAbsolute() + cur->getPropHeight());
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

            Rect rect = { cur->getPropLeftAbsolute(), cur->getPropTopAbsolute(),
                          cur->getPropTopAbsolute() + cur->getPropHeight(),
                          cur->getPropLeftAbsolute() + cur->getPropWidth() };

            Rect prect = m_Parent->getViewport();

            rect.intersect(prect.m_fLeft, prect.m_fTop, prect.m_fRight,
                           prect.m_fBottom);

            return rect;
        }
    }
    if (!cur) cur = this;

    return { cur->getPropLeftAbsolute(), cur->getPropTopAbsolute(),
             cur->getPropTopAbsolute() + cur->getPropHeight(),
             cur->getPropLeftAbsolute() + cur->getPropWidth() };
}

Rect CanvasHandler::getVisibleRect()
{
    Rect vp = this->getViewport();
    this->computeAbsolutePosition();

    return {
        .m_fLeft
        = nidium_min(nidium_max(this->getPropLeftAbsolute(), vp.m_fLeft), vp.m_fRight),
        .m_fTop
        = nidium_min(nidium_max(this->getPropTopAbsolute(), vp.m_fTop), vp.m_fBottom),
        .m_fBottom = nidium_min(this->getPropTopAbsolute() + getPropHeight(), vp.m_fBottom),
        .m_fRight  = nidium_min(this->getPropLeftAbsolute() + getPropWidth(), vp.m_fRight)
    };
}

void CanvasHandler::computeContentSize(int *cWidth, int *cHeight, bool inner)
{
    CanvasHandler *cur;
    m_Content.width  = inner ? 0 : this->getPropWidth();
    m_Content.height = inner ? 0 : this->getPropHeight();

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

            if (retWidth + cur->getPropLeft() > m_Content.width) {
                m_Content.width = retWidth + cur->getPropLeft();
            }
            if (retHeight + cur->getPropTop() > m_Content.height) {
                m_Content.height = retHeight + cur->getPropTop();
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
    m_Context->translate(p_Coating, p_Coating);
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

bool CanvasHandler::containsPoint(double x, double y)
{
    return (x >= getPropLeftAbsolute() && x <= getPropLeftAbsolute() + p_Width
            && y >= getPropTopAbsolute() && y <= getPropTopAbsolute() + p_Height);
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
        this->deviceSetSize(p_Width, p_Height);
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
    arg[5].set(ev->m_x - p_Left.getAlternativeValue()); // layerX
    arg[6].set(ev->m_y - p_Top.getAlternativeValue());  // layerY
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
    arg[5].set(ev->m_x - p_Left.getAlternativeValue()); // layerX
    arg[6].set(ev->m_y - p_Top.getAlternativeValue());  // layerY
    arg[7].set(drop);

    this->fireEvent<CanvasHandler>(CanvasHandler::MOUSE_EVENT, arg);
}

void CanvasHandler::onMouseEvent(InputEvent *ev)
{
    CanvasHandler *underneath = this;
    if (CanvasHandler *tmp = ev->getUnderneathCanvas()) {
        underneath = tmp;
    }

    switch (ev->getType()) {
        case InputEvent::kMouseClick_Type:
            if (ev->m_data[0] == 1) // left click
                m_NidiumContext->setCurrentClickedHandler(this);
            break;
        case InputEvent::kMouseClickRelease_Type:
            if (ev->m_data[0] == 1) {
                CanvasHandler *drag;
                if ((drag = m_NidiumContext->getCurrentClickedHandler())
                    && (drag->m_Flags & kDrag_Flag)) {

                    CanvasHandler *target = (drag == this) ? underneath : this;

                    drag->onDrag(ev, target, true);
                    target->onDrop(ev, drag);

                    drag->m_Flags &= ~kDrag_Flag;
                }
                m_NidiumContext->setCurrentClickedHandler(NULL);
            }
            break;
        case InputEvent::kMouseMove_Type: {
            CanvasHandler *drag;
            if ((drag = m_NidiumContext->getCurrentClickedHandler())) {

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

/*
    Called by Context whenever there are pending events on this canvas
    Currently only handle mouse events.
*/
bool CanvasHandler::_handleEvent(InputEvent *ev)
{
    for (CanvasHandler *handler = this; handler != NULL;
         handler = handler->getParent()) {

        Args arg;

        arg[0].set(ev->getType());
        arg[1].set(ev->m_x);
        arg[2].set(ev->m_y);
        arg[3].set(ev->m_data[0]);     // xrel
        arg[4].set(ev->m_data[1]);     // yrel
        arg[5].set(ev->m_x - p_Left.getAlternativeValue()); // layerX
        arg[6].set(ev->m_y - p_Top.getAlternativeValue());  // layerY
        arg[7].set(this);              // target

        /* fireEvent returns false if a stopPropagation is detected */
        if (!handler->fireEvent<CanvasHandler>(CanvasHandler::MOUSE_EVENT,
                                               arg)) {
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
        // ndm_logf(NDM_LOG_WARN, "CanvasHandler", "A canvas got orphaned (%p)", cur);
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
