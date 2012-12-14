#include "NativeCanvasHandler.h"
#include "NativeSkia.h"
#include <stdio.h>

NativeCanvasHandler::NativeCanvasHandler() :
    parent(NULL), children(NULL), next(NULL),
    prev(NULL), last(NULL), left(0.0), top(0.0), coordPosition(COORD_RELATIVE)
{

}

NativeCanvasHandler::~NativeCanvasHandler()
{
	NativeSkia *cur;

	removeFromParent();

	/* all children got orphaned :( */
	for (cur = children; cur != NULL; cur = cur->handler.next) {
		printf("Warning: a canvas got orphaned (%p)\n", cur);
		cur->handler.removeFromParent();
	}
}

void NativeCanvasHandler::addChild(NativeSkia *insert,
	NativeCanvasHandler::Position position)
{
	/* Already belong to a parent? move it */
	insert->handler.removeFromParent();

	switch(position) {
		case NativeCanvasHandler::POSITION_FRONT:
			if (!children) {
				children = insert;
			}
			insert->handler.next = NULL;
			insert->handler.prev = last;

			if (last) {
				last->handler.next = insert;
			}

			last = insert;
			break;
		case NativeCanvasHandler::POSITION_BACK:
			if (!last) {
				last = insert;
			}
			if (children) {
				children->handler.prev = insert;
			}
			insert->handler.next = children;
			insert->handler.prev = NULL;
			children = insert;
			break;
	}

	insert->handler.parent = self;

}

void NativeCanvasHandler::removeFromParent()
{	
	if (!parent) {
		return;
	}

	if (parent->handler.children == this->self) {
		parent->handler.children = next;
	}

	if (parent->handler.last == this->last) {
		parent->handler.last = prev;
	}

	if (prev) {
		prev->handler.next = next;
	}
	if (next) {
		next->handler.prev = prev;
	}

	parent = NULL;
	next = NULL;
	prev = NULL;
}
