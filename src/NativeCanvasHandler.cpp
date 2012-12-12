#include "NativeCanvasHandler.h"
#include "NativeSkia.h"
#include <stdio.h>

NativeCanvasHandler::NativeCanvasHandler() :
    parent(NULL), children(NULL), next(NULL), prev(NULL), left(0.0), top(0.0)
{

}

void NativeCanvasHandler::addChild(NativeSkia *insert)
{
	insert->handler.removeFromParent();
	
	if (children) {
		children->handler.prev = insert;
	}
	insert->handler.next = children;
	insert->handler.prev = NULL;
	insert->handler.parent = self;

	children = insert;
}

void NativeCanvasHandler::removeFromParent()
{	
	if (!parent) {
		return;
	}

	if (parent->handler.children == this->self) {
		parent->handler.children = next;
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
