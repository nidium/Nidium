#include "NativeCanvasHandler.h"
#include "NativeSkia.h"
#include "NativeCanvas2DContext.h"
#include <stdio.h>

#include <jsapi.h>

NativeCanvasHandler::NativeCanvasHandler(int width, int height) :
    context(NULL), parent(NULL), children(NULL), next(NULL),
    prev(NULL), last(NULL), left(0.0), top(0.0), coordPosition(COORD_RELATIVE)
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
		case NativeCanvasHandler::POSITION_FRONT:
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
		case NativeCanvasHandler::POSITION_BACK:
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

	if (parent->last == this->last) {
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

    /*
		Fill the root layer with white
		This is the base surface on top of the window frame buffer
    */
    if (layer == NULL) {
        layer = this;
        context->clear(0xFFFFFFFF);
    } else {
        double cleft = 0.0, ctop = 0.0;

        if (coordPosition == NativeCanvasHandler::COORD_RELATIVE) {
            cleft = pleft;
            ctop = ptop;
        }

        /*
			draw current context on top of the root layer
        */
        layer->context->composeWith(context,
        	cleft + this->left, 
        	ctop + this->top);
    }

    for (cur = children; cur != NULL; cur = cur->next) {
        cur->layerize(layer,
        	this->left + pleft,
        	this->top + ptop);
    }

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
	if (context) {
		JS_RemoveObjectRoot(context->jscx, &context->jsobj);
	}

    /* Don't delete context, otherwise
       context->jsobj's private would be undefined
    */
}
