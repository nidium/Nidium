/*
   Copyright 2017 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include "IOSUIInterface.h"
#include "System.h"
#include "Frontend/Context.h"

#include <SDL_config.h>
#include <ape_netlib.h>
#include <SDL.h>
#include <SDL_syswm.h>

#include "Graphics/GLHeader.h"
#include "Binding/JSWindow.h"

#import <objc/runtime.h>

using Nidium::Binding::JSWindow;
using Nidium::Frontend::InputEvent;

static const char *SDLViewAssociatedObject = "_IOSUIInterface";

typedef void (*OriginalPressesType)(id self, SEL selector, NSSet<UIPress *> *presses, UIPressesEvent *event);
static OriginalPressesType originalPressesBegan;

@interface NSPointer : NSObject
{
@public
    void *m_Ptr;
}

- (id) initWithPtr:(void *)ptr;
@end

@implementation NSPointer
- (id) initWithPtr:(void *)ptr
{
    self = [super init];
    if (!self) return nil;

    self->m_Ptr = ptr;

    return self;
}
@end

@interface NidiumEventInterceptor: UIView
    - (void)pressesBeganOverride:(NSSet<UIPress *> *)presses withEvent:(UIPressesEvent *)event;
@end

@implementation NidiumEventInterceptor
- (SDL_Scancode)scancodeFromPressType:(UIPressType)presstype
{
    switch (presstype) {
        case UIPressTypeUpArrow:
            return SDL_SCANCODE_UP;
        case UIPressTypeDownArrow:
            return SDL_SCANCODE_DOWN;
        case UIPressTypeLeftArrow:
            return SDL_SCANCODE_LEFT;
        case UIPressTypeRightArrow:
            return SDL_SCANCODE_RIGHT;
        case UIPressTypeSelect:
            /* HIG says: "primary button behavior" */
            return SDL_SCANCODE_SELECT;
        case UIPressTypeMenu:
            /* HIG says: "returns to previous screen" */
            return SDL_SCANCODE_MENU;
        case UIPressTypePlayPause:
            /* HIG says: "secondary button behavior" */
            return SDL_SCANCODE_PAUSE;
        default:
            return SDL_SCANCODE_UNKNOWN;
    }
}

- (void)pressesBeganOverride:(NSSet<UIPress *> *)presses withEvent:(UIPressesEvent *)event
{
    NSPointer *idthis = objc_getAssociatedObject(self, SDLViewAssociatedObject);
    Nidium::Interface::IOSUIInterface *ui =
        (Nidium::Interface::IOSUIInterface *)idthis->m_Ptr;

    JSWindow *window = nullptr;
    NSMutableSet *newPresses = nil;
    if (ui->m_NidiumCtx
            && (window = JSWindow::GetObject(ui->m_NidiumCtx->getNJS()))) {
        for (UIPress *press in presses) {
            SDL_Scancode scancode = [self scancodeFromPressType:press.type];
            if (scancode == SDL_SCANCODE_MENU) {
                if (!window->onMediaKey(InputEvent::kMediaKey_menu, false /* down */)) {
                    // Stop propagation call detected, don't send the event to SDL
                    if (newPresses == nil) {
                        newPresses = [[NSMutableSet alloc] initWithSet:presses];
                    }
                    [newPresses removeObject:press];
                }
            }
        }
    }

    originalPressesBegan(self,
                         @selector(pressesBegan:withEvent:),
                        newPresses != nil ? newPresses : presses,
                        event);
}

@end

namespace Nidium {
namespace Interface {

extern UIInterface *__NidiumUI;

IOSUIInterface::IOSUIInterface()
    : UIInterface(), m_Console(NULL)
{
#ifdef NDM_TARGET_TVOS
    /*
        By default SDL prevents default behavior for remote controller buttons.
        Change that, so we can have the default behavior for the menu button.
    */
    SDL_SetHint(SDL_HINT_APPLE_TV_CONTROLLER_UI_EVENTS, "1");
#else
    SDL_SetHint(SDL_HINT_ORIENTATIONS, "LandscapeLeft LandscapeRight Portrait PortraitUpsideDown");
#endif
}

void IOSUIInterface::setGLContextAttribute()
{
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    // Finally, if your application completely redraws the screen each frame,
    // you may find significant performance improvement by setting the attribute SDL_GL_RETAINED_BACKING to 0.
    SDL_GL_SetAttribute(SDL_GL_RETAINED_BACKING, 0);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
}

int IOSUIInterface::toLogicalSize(int size)
{
    System *sys = static_cast<System *>(SystemInterface::GetInstance());
    double pr = sys->backingStorePixelRatio();
    return ceil(size/pr);
}

bool IOSUIInterface::createWindow(int width, int height)
{
    /*
        iOS/tvOS has a fixed window size, thus we ignore the size from the NML.
        Also, SDL doesn't need the real values of the view, just the aspect ratio,
        the real size is retrived in the onWindowCreated() callback.
     */
#ifdef NDM_TARGET_TVOS
    width = 1920;
    height = 1080;
#else
    width = 320;
    height = 480;
#endif

    return UIInterface::createWindow(width, height);
}

void IOSUIInterface::handleEvent(const SDL_Event *ev)
{
    fprintf(stderr, "SDL got event %d\n", ev->type);
    switch (ev->type) {
        /*
            When the device is rotated, notify nidium of the screen resize
        */
        case SDL_WINDOWEVENT:
            if (ev->window.event == SDL_WINDOWEVENT_RESIZED) {
                this->getNidiumContext()->setWindowSize(ev->window.data1, ev->window.data2);
            }
            break;
#ifdef NDM_TARGET_TVOS
        /*
            Forward apple TV remote controller click (SDL_SCANCODE_SELECT) as mouse click
        */
        case SDL_KEYUP:
        case SDL_KEYDOWN: {
            JSWindow *window = NULL;
            if (!this->isContextReady()
                    || !(window = JSWindow::GetObject(m_NidiumCtx->getNJS()))) {
                break;
            }
            switch(ev->key.keysym.scancode) {
                case SDL_SCANCODE_SELECT:
                    window->mouseClick(ev->motion.x, ev->motion.y,
                                       ev->type == SDL_KEYDOWN ? true : false, 1 /* left click */,
                                       ev->button.clicks);
                    return;
                case SDL_SCANCODE_PAUSE:
                    window->onMediaKey(InputEvent::kMediaKey_pause, ev->type == SDL_KEYDOWN ? true : false);
                    return;
                case SDL_SCANCODE_MENU:
                    window->onMediaKey(InputEvent::kMediaKey_menu, ev->type == SDL_KEYDOWN ? true : false);
                    return;
                default:
                    break;
            }
            break;
        }
        /*
            SDL fakes apple TV remote controller touch event as
            mouse button down/up. Ignore them.
        */
        case SDL_MOUSEBUTTONUP:
        case SDL_MOUSEBUTTONDOWN:
            return;
#endif
    }

    UIInterface::handleEvent(ev);
}

void IOSUIInterface::quitApplication()
{
    exit(1);
}

void IOSUIInterface::hitRefresh()
{
    this->restartApplication();
}

void IOSUIInterface::bindFramebuffer()
{
    UIInterface::bindFramebuffer();
    glBindRenderbuffer(GL_RENDERBUFFER, m_FBO);
}

void IOSUIInterface::onWindowCreated()
{
    /*
        iOS/tvOS doesnt use window-system framebuffer.
        SDL already generate a fbo and renderbuffer for us (which is not 0)
    */

    SDL_SysWMinfo info;
    SDL_VERSION(&info.version);

    SDL_GetWindowWMInfo(m_Win, &info);

    m_FBO = info.info.uikit.framebuffer;
    m_Console = new DummyConsole(this);

    /* If the program implements NidiumWindow */
    if (NSClassFromString(@"NidiumWindow")) {
        m_NidiumWindow = [[NSClassFromString(@"NidiumWindow") alloc] initWithWindow:info.info.uikit.window];
    }
    
    /*
        When creating the SDL window, we don't know the size yet, so we query & set the
        window size once it's created and before nidium set it's internal buffer size
    */
    int w, h;
    SDL_GetWindowSize(m_Win, &w, &h);
    this->setWindowSize(w, h);
    
#ifdef NDM_TARGET_TVOS
    this->patchSDLEvents(info.info.uikit.window.rootViewController.view);
#endif
}

void IOSUIInterface::bridge(const char *data)
{
    if (m_NidiumWindow == nullptr) {
        fprintf(stderr, "Bridge data received but discarded : %s\n", data);
        return;
    }
    NSString *dataString = [NSString stringWithUTF8String:data];
    [m_NidiumWindow performSelector:@selector(bridge:) withObject:dataString];

}

void IOSUIInterface::openFileDialog(const char *files[],
                                    void (*cb)(void *nof,
                                    const char *lst[],
                                    uint32_t len),
                                    void *arg,
                                    int flags)
{
    return;
}

void IOSUIInterface::runLoop()
{
    APE_timer_create(m_Gnet, 1, UIInterface::HandleEvents,
                     static_cast<void *>(this));
    APE_loop_run(m_Gnet);
}

/*
    Override SDL UIView pressesBegan method so we can intercept
    the "menu" key in a sync way and allow the event to be
    prevented by the JS.
*/
void IOSUIInterface::patchSDLEvents(UIView *view)
{
    Class SDL_uiview = object_getClass((id)view);

   Method newPressesBeganMethod =
       class_getInstanceMethod([NidiumEventInterceptor class], @selector(pressesBeganOverride:withEvent:));
   Method oldPressesBeganMethod =
       class_getInstanceMethod(SDL_uiview, @selector(pressesBegan:withEvent:));

    originalPressesBegan = (OriginalPressesType)method_getImplementation(oldPressesBeganMethod);

    method_exchangeImplementations(oldPressesBeganMethod, newPressesBeganMethod);

    NSPointer *idthis = [[NSPointer alloc] initWithPtr:this];
    objc_setAssociatedObject(view, SDLViewAssociatedObject, idthis, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
}
} // namespace Interface
} // namespace Nidium
