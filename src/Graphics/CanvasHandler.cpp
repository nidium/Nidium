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
    : m_Context(NULL), m_JsCx(nctx->getNJS()->getJSContext()),
      m_Overflow(true), m_Parent(NULL), m_Children(NULL), m_Next(NULL),
      m_Prev(NULL), m_Last(NULL), m_Flags(0), m_nChildren(0),
      m_CoordPosition(COORD_DEFAULT), m_Visibility(CANVAS_VISIBILITY_VISIBLE),
      m_Zoom(1.0), m_ScaleX(1.0), m_ScaleY(1.0),
      m_AllowNegativeScroll(false), m_NidiumContext(nctx), m_Pending(0),
      m_Loaded(!lazyLoad), m_Cursor(UIInterface::ARROW)
{
    m_Identifier.idx = ++nctx->m_CanvasCreatedIdx;
    m_NidiumContext->m_CanvasListIdx.insert({m_Identifier.idx, this});
    m_Identifier.str = nullptr;

    p_Width     = nidium_max(width, -1);
    p_Height    = nidium_max(height, -1);

    m_FluidHeight = false;
    m_FluidWidth  = false;

    m_YogaRef = YGNodeNewWithConfig(nctx->m_YogaConfig);

    YGNodeSetContext(m_YogaRef, this);

    YGNodeStyleSetPositionType(m_YogaRef, YGPositionTypeRelative);
    YGNodeStyleSetPosition(m_YogaRef, YGEdgeLeft, p_Left);
    YGNodeStyleSetPosition(m_YogaRef, YGEdgeTop, p_Top);


    if (p_Width >= 0) {
        YGNodeStyleSetWidth(m_YogaRef, p_Width);
    }

    if (p_Height >= 0) {
        YGNodeStyleSetHeight(m_YogaRef, p_Height);
    }

    YGNodeStyleSetMinWidth(m_YogaRef, p_MinWidth);
    YGNodeStyleSetMinHeight(m_YogaRef, p_MinHeight);

    memset(&m_Margin, 0, sizeof(m_Margin));
    memset(&m_Padding, 0, sizeof(m_Padding));
    memset(&m_MousePosition, 0, sizeof(m_MousePosition));

    m_MousePosition.consumed = true;
    //p_Flex = true;

    m_Content.width  = p_Width;
    m_Content.height = p_Height;

    m_Content._width  = p_Width;
    m_Content._height = p_Height;

    m_Content.scrollLeft = 0;
    m_Content.scrollTop  = 0;

    p_Flex = true;
}

void CanvasHandler::computeLayoutPositions()
{
    YGNodeCalculateLayout(m_YogaRef, p_Width, p_Height, YGDirectionLTR);
}

void CanvasHandler::setPositioning(CanvasHandler::COORD_POSITION mode)
{
    m_CoordPosition = mode;
    switch (mode) {
        case CanvasHandler::COORD_DEFAULT:
        case CanvasHandler::COORD_FIXED:
            /* Relative in Yoga means that it interacts with the flexbox */
            YGNodeStyleSetPositionType(m_YogaRef, YGPositionTypeRelative);
            break;
        case CanvasHandler::COORD_RELATIVE:
        case CanvasHandler::COORD_ABSOLUTE:
            YGNodeStyleSetPositionType(m_YogaRef, YGPositionTypeAbsolute);
            break;
    }

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

    YGNodeStyleSetMinWidth(m_YogaRef, p_MinWidth);

    if (p_Width < p_MinWidth) {
        this->setWidth(p_MinWidth);
    }
}

void CanvasHandler::setPropMinHeight(int height)
{
    if (height < 1) height = 1;

    p_MinHeight = p_MaxHeight ? nidium_min(height, p_MaxHeight) : height;

    YGNodeStyleSetMinHeight(m_YogaRef, p_MinHeight);

    if (p_Height < p_MinHeight) {
        this->setHeight(p_MinHeight);
    }
}

void CanvasHandler::setPropMaxWidth(int width)
{
    if (width < 1) width = 1;

    p_MaxWidth = nidium_max(p_MinWidth, width);

    YGNodeStyleSetMaxWidth(m_YogaRef, p_MaxWidth);

    if (p_Width > p_MaxWidth) {
        this->setWidth(p_MaxWidth);
    }
}

void CanvasHandler::setPropMaxHeight(int height)
{
    if (height < 1) height = 1;

    p_MaxHeight = nidium_max(p_MinHeight, height);

    YGNodeStyleSetMaxHeight(m_YogaRef, p_MaxHeight);

    if (p_Height > p_MaxHeight) {
        this->setHeight(p_MaxHeight);
    }
}

bool CanvasHandler::setWidth(int width, bool force)
{
    width = p_MaxWidth ? nidium_clamp(width, p_MinWidth, p_MaxWidth)
                       : nidium_max(width, p_MinWidth);

    if (p_Width == width) {
        return true;
    }

    p_Width = width;

    this->setPendingFlags(kPendingResizeWidth);

    YGNodeStyleSetWidth(m_YogaRef, width >= 0 ? width : YGUndefined);

    return true;
}

bool CanvasHandler::setHeight(int height, bool force)
{

    height = p_MaxHeight ? nidium_clamp(height, p_MinHeight, p_MaxHeight)
                         : nidium_max(height, p_MinHeight);

    if (p_Height == height) {
        return true;
    }
    p_Height = height;

    this->setPendingFlags(kPendingResizeHeight);

    YGNodeStyleSetHeight(m_YogaRef, height >= 0 ? height : YGUndefined);

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


    YGNodeStyleSetWidth(m_YogaRef, width >= 0 ? width : YGUndefined);
    YGNodeStyleSetHeight(m_YogaRef, height >= 0 ? height : YGUndefined);

    this->setPendingFlags(kPendingResizeWidth | kPendingResizeHeight);
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

    YGNodeInsertChild(m_YogaRef, insert->m_YogaRef, m_nChildren);

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

    YGNodeInsertChild(m_YogaRef, insert->m_YogaRef, m_nChildren);

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

    YGNodeRemoveChild(m_Parent->m_YogaRef, m_YogaRef);

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
    std::vector<InputEvent> *eventList = m_NidiumContext->getInputHandler()->getEvents();

    if (eventList->size() == 0) {
        return;
    }

    Rect actualRect;
    actualRect.m_fLeft   = p_Left.getAlternativeValue() - p_Coating;
    actualRect.m_fTop    = p_Top.getAlternativeValue() - p_Coating;
    actualRect.m_fRight  = p_Width.getAlternativeValue() + p_Left.getAlternativeValue();
    actualRect.m_fBottom = p_Height.getAlternativeValue() + p_Top.getAlternativeValue();

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
    for (auto &ev : *eventList) {
        /* This event is happening in a zone inside |this| canvas  */
        if (ev.isInRect(actualRect)) {
            /*
                Increment depth (Nth canvas affected by this event)
            */
            ev.inc();

            if (!evlist) {
                evlist = ape_new_pool_list(0, 4);
            }

            /* 
               Dupplicate the event and set |this|
               as the handler of the new event 
            */
            InputEvent *dup = ev.dupWithHandler(this);

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

    if (m_Visibility == CANVAS_VISIBILITY_HIDDEN || p_Opacity == 0.0) {
        return;
    }

    // double pzoom = this->zoom * azoom;
    double popacity = p_Opacity * layerContext.m_aOpacity;

    int tmpLeft;
    int tmpTop;

    if (m_Parent && m_Parent->p_Flex) {
#if 0
        printf("===== YOGA =====\n");
        YGNodePrint(m_YogaRef, YGPrintOptionsLayout);
        printf("\n");
#endif
        int nwidth, nheight;

        /* Read the values from Yoga */
        getDimensions(&nwidth, &nheight, &tmpLeft, &tmpTop);

        /*
            Check if we need to resize the element.
            p_Width|Height alternative values hold the last computed Yoga value.

            This will trigger an onResize event on the element
        */
        if (nwidth != p_Width.getAlternativeValue()
            || nheight != p_Height.getAlternativeValue()) {

            p_Width.setAlternativeValue(nwidth);
            p_Height.setAlternativeValue(nheight);

            deviceSetSize(nwidth, nheight);
            
        }
    } else {
        tmpLeft = this->getPropLeft();
        tmpTop  = this->getPropTop();
    }

    int maxChildrenHeight = p_Width.getAlternativeValue(),
        maxChildrenWidth  = p_Height.getAlternativeValue();

    /*
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
        willDraw = (!layerContext.m_Clip || m_CoordPosition == COORD_ABSOLUTE
                    || (layerContext.m_Clip->checkIntersect(
                      p_Left.getAlternativeValue() - p_Coating, p_Top.getAlternativeValue() - p_Coating,
                      p_Left.getAlternativeValue() + p_Coating + YGNodeLayoutGetWidth(m_YogaRef),
                      p_Top.getAlternativeValue() + p_Coating + YGNodeLayoutGetHeight(m_YogaRef))));

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
            layerContext.m_Clip->m_fRight  = getComputedWidth() + p_Left.getAlternativeValue();
            layerContext.m_Clip->m_fBottom = getComputedHeight() + p_Top.getAlternativeValue();
            /*
                if clip is not null, reduce it to intersect the current rect.
                /!\ clip->intersect changes "clip"
            */

        } else if (!layerContext.m_Clip->intersect(
                       p_Left.getAlternativeValue(),
                       p_Top.getAlternativeValue(),
                       p_Width.getAlternativeValue() + p_Left.getAlternativeValue(),
                       p_Height.getAlternativeValue() + p_Top.getAlternativeValue())
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
        for (cur = m_Children; cur != NULL; cur = cur->m_Next) {
            int offsetLeft = 0, offsetTop = 0;
            if (cur->m_CoordPosition != COORD_FIXED) {
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
                   .m_Clip       = layerContext.m_Clip};

            cur->layerize(ctx, compList, draw);

            /*
                Incrementaly check the bottom/right most children
                in order to compute the contentHeight/Width
            */
            if (cur->m_CoordPosition == COORD_RELATIVE
                && cur->m_Visibility == CANVAS_VISIBILITY_VISIBLE) {

                int actualChildrenHeightPlusTop
                    = cur->getComputedTop() + (cur->m_Overflow ? cur->m_Content._height
                                                       : cur->getComputedWidth());
                int actualChildrenWidthPlusLeft
                    = cur->getComputedLeft() + (cur->m_Overflow ? cur->m_Content._width
                                                        : cur->getComputedHeight());

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

void CanvasHandler::scroll(int x, int y)
{
    if (m_ScrollableY) {
        int pos = m_Content.scrollTop - (y - m_CurrentScroll.y);
        int max = this->getContentHeight(true) - this->getComputedHeight();

        pos = nidium_clamp(pos, 0, max);

        this->setScrollTop(pos);
    }

    if (m_ScrollableX) {
        int pos = m_Content.scrollLeft - (x - m_CurrentScroll.x);
        int max = this->getContentWidth(true) - this->getComputedWidth();

        pos = nidium_clamp(pos, 0, max);

        this->setScrollLeft(pos);
    }

    m_CurrentScroll.x = x;
    m_CurrentScroll.y = y;
}

void CanvasHandler::onScroll(int x, int y,
                             int velocityX, int velocityY,
                             InputEvent::ScrollState state)
{
    InputHandler *inputHandler = m_NidiumContext->getInputHandler();

    switch (state) {
        case InputEvent::kScrollState_start:
            m_CurrentScroll.active = true;
            m_CurrentScroll.x      = x;
            m_CurrentScroll.y      = y;

            inputHandler->setCurrentScrollHandler(this);
            break;
        case InputEvent::kScrollState_move:
            if (!m_CurrentScroll.active) {
                // Scroll has been enabled while user was already scrolling
                // Fake a scroll start event
                this->onScroll(x, y,
                               velocityX, velocityY,
                               InputEvent::kScrollState_start);
            }
            this->scroll(x, y);
            break;
        case InputEvent::kScrollState_end:
            m_CurrentScroll.active = false;
            break;
    }
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

void CanvasHandler::setPropOpacity(double val)
{
    val = nidium_min(1, nidium_max(0, val));

    p_Opacity = val;
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
        this->deviceSetSize(nidium_max(p_Width, 1), nidium_max(1, p_Height));
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

void CanvasHandler::checkDrop(InputEvent *ev,
                              Graphics::CanvasHandler *drag)
{
    if (!drag || !(drag->m_Flags & kDrag_Flag)) return;

    CanvasHandler *underneath = this;
    if (CanvasHandler *tmp = ev->getUnderneathCanvas()) {
        underneath = tmp;
    }

    CanvasHandler *target = (drag == this) ? underneath : this;

    drag->onDrag(ev, target, true);
    target->onDrop(ev, drag);

    drag->m_Flags &= ~kDrag_Flag;
}

void CanvasHandler::checkDrag(InputEvent *ev,
                              Graphics::CanvasHandler *drag)
{
    if (!drag) return;

    CanvasHandler *underneath = this;
    if (CanvasHandler *tmp = ev->getUnderneathCanvas()) {
        underneath = tmp;
    }

    drag->onDrag(ev, (this == drag) ? underneath : this);
}

void CanvasHandler::onInputEvent(InputEvent *ev)
{
    InputHandler *inputHandler = m_NidiumContext->getInputHandler();

    switch (ev->getType()) {
        case InputEvent::kScroll_type: {
            int consumed = ev->m_Origin->m_data[3];
            Graphics::CanvasHandler *scrollHandler
                = inputHandler->getCurrentScrollHandler();
            InputEvent::ScrollState state
                = static_cast<InputEvent::ScrollState>(ev->m_data[2]);

            if (consumed || !scrollHandler) break;

            ev->m_Origin->m_data[3] = 1;

            scrollHandler->onScroll(ev->m_x, ev->m_y,
                                    ev->m_data[0], ev->m_data[1],
                                    state);

            if (state == InputEvent::kScrollState_end) {
                inputHandler->setCurrentScrollHandler(nullptr);
            }
        } break;
        case InputEvent::kTouchStart_Type:
            inputHandler->setCurrentTouchedHandler(ev->getTouch()->getIdentifier(), this);
            break;
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

            this->checkDrag(ev, inputHandler->getCurrentTouchHandler(ev->getTouch()->getIdentifier()));
            break;
        case InputEvent::kTouchEnd_Type: {
            unsigned int id = ev->getTouch()->getIdentifier();

            this->checkDrop(ev, inputHandler->getCurrentTouchHandler(id));
            inputHandler->setCurrentTouchedHandler(id, nullptr);
        } break;

        case InputEvent::kMouseClick_Type:
            if (ev->m_data[0] == 1) { // left click
                inputHandler->setCurrentClickedHandler(this);
            }
            break;
        case InputEvent::kMouseClickRelease_Type:
            if (ev->m_data[0] == 1) {
                this->checkDrop(ev, inputHandler->getCurrentClickedHandler());
                inputHandler->setCurrentClickedHandler(nullptr);
            }
            break;
        case InputEvent::kMouseMove_Type:
            this->checkDrag(ev, inputHandler->getCurrentClickedHandler());
            break;
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
    Currently only handle mouse, touch & scroll events.
*/
bool CanvasHandler::_handleEvent(InputEvent *ev)
{
    Events canvasEvent = MOUSE_EVENT;
    bool scrollConsumed = false;

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
                arg[5].set(ev->m_x - p_Left.getAlternativeValue()); // layerX
                arg[6].set(ev->m_y - p_Top.getAlternativeValue());  // layerY
                arg[7].set(this);              // target
                break;
            case InputEvent::kScroll_type: {
                if (!handler->isScrollable() || ev->m_Origin->m_data[3]) {
                    continue;
                }

                canvasEvent    = SCROLL_EVENT;
                scrollConsumed = true;

                InputEvent::ScrollState state
                    = static_cast<InputEvent::ScrollState>(ev->m_data[2]);

                arg[0].set(ev->getType());
                arg[1].set(ev->m_x);
                arg[2].set(ev->m_y);
                arg[3].set(ev->m_data[0]);     // velocityY
                arg[4].set(ev->m_data[1]);     // velocityY
                arg[5].set(ev->m_data[2]);     // state

                /*
                    Set a flag on the event so onInputEvent()
                    can know if the event has been consumed.
                */
                ev->m_Origin->m_data[3] = 1;

                handler->onScroll(ev->m_x, ev->m_y,
                                  ev->m_data[0], ev->m_data[1],
                                  state);
            } break;
            case InputEvent::kTouchStart_Type:
            case InputEvent::kTouchEnd_Type:
            case InputEvent::kTouchMove_Type: {
                /*
                    If the handler isn't one of the handlers that
                    received the touchstart event ignore it.
                 */
                if (ev->getType() != InputEvent::kTouchStart_Type &&
                        !ev->getTouch()->hasOrigin(handler)) {
                    continue;
                }

                canvasEvent = TOUCH_EVENT;

                this->onTouch(ev, arg, handler);
            } break;
        }

        /* fireEvent returns false if a stopPropagation is detected */
        if (!handler->fireEventSync<CanvasHandler>(canvasEvent, arg)) {
            break;
        }

        /* Scroll events does not bubble */
        if (scrollConsumed) {
            break;
        }
    }

    this->onInputEvent(ev);

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

    YGNodeFree(m_YogaRef);
}

} // namespace Graphics
} // namespace Nidium
