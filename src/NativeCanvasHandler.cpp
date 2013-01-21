#include "NativeCanvasHandler.h"
#include "NativeSkia.h"
#include "NativeCanvas2DContext.h"
#include <stdio.h>

#include <jsapi.h>

NativeCanvasHandler::NativeCanvasHandler(int width, int height) :
    context(NULL), left(0.0), top(0.0), a_left(0), a_top(0), opacity(1.0),
    parent(NULL), children(NULL), next(NULL),
    prev(NULL), last(NULL), coordPosition(COORD_RELATIVE),
    visibility(CANVAS_VISIBILITY_VISIBLE)
{
    this->width = width;
    this->height = height;
}

void NativeCanvasHandler::setPosition(double left, double top)
{
    this->left = left;
    this->top = top;
}

void NativeCanvasHandler::setPositioning(NativeCanvasHandler::COORD_POSITION mode)
{
    coordPosition = mode;
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

}

void NativeCanvasHandler::removeFromParent()
{   
    if (!parent) {
        return;
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

    parent = NULL;
    next = NULL;
    prev = NULL;
}

/*
    TODO: clipping/overflow
*/
void NativeCanvasHandler::layerize(NativeCanvasHandler *layer,
    double pleft, double ptop)
{
    NativeCanvasHandler *cur;

    if (visibility == CANVAS_VISIBILITY_HIDDEN || opacity == 0.0) {
        return;
    }

    /*
        Fill the root layer with white
        This is the base surface on top of the window frame buffer
    */
    if (layer == NULL) {
        layer = this;
        context->clear(0xFFFFFFFF);
    } else {
        double cleft = 0.0, ctop = 0.0;

        if (coordPosition == COORD_RELATIVE) {
            cleft = pleft;
            ctop = ptop;
        }

        /*
            Set the absolute position
        */
        this->a_left = cleft + this->left;
        this->a_top = ctop + this->top;

        /*
            draw current context on top of the root layer
        */      
        layer->context->composeWith(context,
            this->a_left, 
            this->a_top);
    }

    for (cur = children; cur != NULL; cur = cur->next) {
        cur->layerize(layer,
            this->left + pleft,
            this->top + ptop);
    }

}

/* Compute whether or the canvas is going to be drawn */
bool NativeCanvasHandler::isDisplayed()
{
    if (visibility == CANVAS_VISIBILITY_HIDDEN) {
        return false;
    }

    return (parent ? parent->isDisplayed() : true);
}

bool NativeCanvasHandler::isHidden()
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

NativeCanvasHandler::~NativeCanvasHandler()
{
    NativeCanvasHandler *cur;

    removeFromParent();

    /* all children got orphaned :( */
    for (cur = children; cur != NULL; cur = cur->next) {
        printf("Warning: a canvas got orphaned (%p)\n", cur);
        cur->removeFromParent();
    }
    if (context && context->jsobj && context->jscx) {
        JS_RemoveObjectRoot(context->jscx, &context->jsobj);
    }

    /* Don't delete context, otherwise
       context->jsobj's private would be undefined
    */
}
