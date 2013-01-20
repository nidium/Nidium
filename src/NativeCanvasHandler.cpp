#include "NativeCanvasHandler.h"
#include "NativeSkia.h"
#include "NativeCanvas2DContext.h"
#include <stdio.h>

#include <jsapi.h>

NativeCanvasHandler::NativeCanvasHandler(int width, int height) :
    context(NULL), jsobj(NULL), jscx(NULL), left(0.0), top(0.0), a_left(0), a_top(0),
    opacity(1.0), overflow(true),
    parent(NULL), children(NULL), next(NULL),
    prev(NULL), last(NULL), nchildren(0), coordPosition(COORD_RELATIVE),
    visibility(CANVAS_VISIBILITY_VISIBLE)
{
    this->width = width;
    this->height = height;

    memset(&this->padding, 0, sizeof(this->padding));
}

void NativeCanvasHandler::setPosition(double left, double top)
{
    this->left = left;
    this->top = top;
}

void NativeCanvasHandler::setPositioning(NativeCanvasHandler::COORD_POSITION mode)
{
    coordPosition = mode;
    this->computeAbsolutePosition();
}

void NativeCanvasHandler::setWidth(int width)
{
    this->width = width;

    if (context) {
        context->setSize(this->width + (this->padding.global * 2),
            this->height + (this->padding.global * 2));
    }
}

void NativeCanvasHandler::setHeight(int height)
{
    this->height = height;

    if (context) {
        context->setSize(this->width + (this->padding.global * 2),
            this->height + (this->padding.global * 2));
    }
}

void NativeCanvasHandler::setSize(int width, int height)
{
    this->width = width;
    this->height = height;

    if (context) {
        context->setSize(this->width + (this->padding.global * 2),
            this->height + (this->padding.global * 2));
    }
}

void NativeCanvasHandler::setPadding(int padding)
{
    if (!context) {
        return;
    }

    context->translate(-this->padding.global, -this->padding.global);

    this->padding.global = padding;
 
    context->setSize(this->width + (this->padding.global * 2),
        this->height + (this->padding.global * 2));

    context->translate(this->padding.global, this->padding.global);
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

/*
    TODO: clipping/overflow
*/
void NativeCanvasHandler::layerize(NativeCanvasHandler *layer,
    double pleft, double ptop, double aopacity, NativeRect *clip)
{
    NativeCanvasHandler *cur;
    NativeRect nclip;

    if (visibility == CANVAS_VISIBILITY_HIDDEN || opacity == 0.0) {
        return;
    }

    double popacity = opacity * aopacity;
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
            this->a_left - this->padding.global, 
            this->a_top - this->padding.global, popacity,
            (coordPosition == COORD_ABSOLUTE) ? NULL : clip);
    }

    if (!this->overflow) {

        if (clip == NULL) {
            clip = &nclip;
            clip->fLeft = this->a_left;
            clip->fTop = this->a_top;
            clip->fRight = this->width + this->a_left;
            clip->fBottom = this->height + this->a_top;
        } else if (!clip->intersect(this->a_left, this->a_top,
                    this->width + this->a_left, this->height + this->a_top)) {
            /* don't need to draw children (out of bounds) */
            return;
        }
    }
    NativeRect tmpClip;
    if (clip != NULL) {
        memcpy(&tmpClip, clip, sizeof(NativeRect));
    }
    for (cur = children; cur != NULL; cur = cur->next) {

        cur->layerize(layer,
            this->left + pleft,
            this->top + ptop, popacity, clip);
            if (clip != NULL) {
                memcpy(clip, &tmpClip, sizeof(NativeRect));
            }
    }

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
        this->a_top = this->top;
        this->a_left = this->left;
        return;
    }

    double ctop = top, cleft = left;
    NativeCanvasHandler *cparent = this->parent;

    while (cparent != NULL) {
        ctop += cparent->top;
        cleft += cparent->left;

        if (cparent->coordPosition != COORD_RELATIVE) {
            break;
        }

        cparent = cparent->parent;
    }

    this->a_top = ctop;
    this->a_left = cleft;

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

NativeCanvasHandler *NativeCanvasHandler::getParent()
{
    return this->parent;
}

void NativeCanvasHandler::getChildren(NativeCanvasHandler **out) const
{
    NativeCanvasHandler *cur;
    int i = 0;
    for (cur = children; cur != NULL; cur = cur->next) {
        out[i++] = cur;
    }
}

int32_t NativeCanvasHandler::countChildren() const
{
    return this->nchildren;
}

bool NativeCanvasHandler::containsPoint(double x, double y) const
{
    return (x >= a_left && x <= a_left+width &&
            y >= a_top && y <= a_top+height);
}

void NativeCanvasHandler::unrootHierarchy()
{
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
        //JS_RemoveObjectRoot(context->jscx, &context->jsobj);
    }
    if (jsobj) {
        //JS_RemoveObjectRoot(jscx, &jsobj);
    }

    /* Don't delete context, otherwise
       context->jsobj's private would be undefined
    */
}
