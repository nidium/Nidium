#import "NativeCocoaUIInterface.h"
#import "NativeUIConsole.h"
#import <NativeJS.h>
#import <NativeContext.h>
#import <NativeSkia.h>
#import <NativeApp.h>
#import <NativeJSWindow.h>
#import <SDL.h>
#import <SDL_opengl.h>
#import <SDL_syswm.h>
#import <Cocoa/Cocoa.h>
#import <native_netlib.h>
#import <NativeMacros.h>
#import <NativeMessages.h>
#import <NativePath.h>

#import <NativeNML.h>
#import <sys/stat.h>
#import "NativeSystem.h"
#import "SDL_keycode_translate.h"

#import "NativeDragNSView.h"
#import <objc/runtime.h>

#define kNativeWidth 1024
#define kNativeHeight 768

#define kNativeTitleBarHeight 0

#define kNativeVSYNC 1


@interface NativeCocoaUIInterfaceWrapper: NSObject {
    NativeCocoaUIInterface *base;
}

- (NSMenu *) renderSystemTray;
- (void) menuClicked:(id)ev;
- (id) initWithUI:(NativeCocoaUIInterface *)ui;

@end

uint64_t ttfps = 0;

static __inline__ void ConvertNSRect(NSRect *r)
{
    r->origin.y = CGDisplayPixelsHigh(kCGDirectMainDisplay) - r->origin.y - r->size.height;
}

static NSWindow *NativeCocoaWindow(SDL_Window *win)
{
    SDL_SysWMinfo info;
    SDL_VERSION(&info.version);
    SDL_GetWindowWMInfo(win, &info);

    return (NSWindow *)info.info.cocoa.window;
}

void NativeCocoaUIInterface::quitApplication()
{
    [[NSApplication sharedApplication] terminate:nil];
}

/*
    TODO: useless indirection?
*/
static int NativeProcessUI(void *arg)
{
    return NativeUIInterface::HandleEvents((NativeUIInterface *)arg)
}

static int NativeProcessSystemLoop(void *arg)
{
    SDL_PumpEvents();
    NativeCocoaUIInterface *ui = (NativeCocoaUIInterface *)arg;

    if (ui->m_NativeCtx) {
        ui->makeMainGLCurrent();
    }
    return 4;
}

static void NativeDoneExtracting(void *closure, const char *fpath)
{
    NativeCocoaUIInterface *ui = (NativeCocoaUIInterface *)closure;
    if (chdir(fpath) != 0) {
        fprintf(stderr, "Cant enter cache directory (%d)\n", errno);
        return;
    }
    fprintf(stdout, "Changing directory to : %s\n", fpath);

    ui->m_Nml = new NativeNML(ui->m_Gnet);
    ui->m_Nml->loadFile("./index.nml", NativeCocoaUIInterface_onNMLLoaded, ui);
}

void NativeCocoaUIInterface::log(const char *buf)
{
    if (this->m_Console && !this->m_Console->m_IsHidden) {
        this->m_Console->log(buf);
    } else {
        fwrite(buf, sizeof(char), strlen(buf), stdout);
        fflush(stdout);
    }
}

void NativeCocoaUIInterface::logf(const char *format, ...)
{
    char *buff;
    int len;
    va_list val;

    va_start(val, format);
    len = vasprintf(&buff, format, val);
    va_end(val);

    this->log(buff);

    free(buff);
}

void NativeCocoaUIInterface::vlog(const char *format, va_list ap)
{
    char *buff;
    int len;

    len = vasprintf(&buff, format, ap);

    this->log(buff);

    free(buff);
}

void NativeCocoaUIInterface::logclear()
{
    if (this->m_Console && !this->m_Console->m_IsHidden) {
        this->m_Console->clear();
    }
}


void NativeCocoaUIInterface::stopApplication()
{
    [this->m_DragNSView setResponder:nil];
    this->disableSysTray();
    m_SystemMenu.deleteItems();

    if (this->m_Nml) {
        delete this->m_Nml;
        this->m_Nml = NULL;
    }
    if (this->m_NativeCtx) {
        delete this->m_NativeCtx;
        this->m_NativeCtx = NULL;
        NativeMessages::destroyReader();
    }

    glClearColor(1, 1, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    /* Also clear the front buffer */
    SDL_GL_SwapWindow(this->m_Win);
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

bool NativeCocoaUIInterface::runJSWithoutNML(const char *path, int width, int height)
{
    NativeMessages::initReader(m_Gnet);
    if (path != this->m_FilePath) {
        if (this->m_FilePath) {
            free(this->m_FilePath);
        }
        this->m_FilePath = strdup(path);
    }
    if (strlen(path) < 5) {
        return false;
    }

    if (!this->createWindow(
        width, height+kNativeTitleBarHeight)) {
        return false;
    }

    this->setWindowTitle("Nidium");

    NativeJS::initNet(m_Gnet);

    NativePath jspath(path);

    NativePath::cd(jspath.dir());
    NativePath::chroot(jspath.dir());

    m_NativeCtx->getNJS()->LoadScript(path);

    return true;
}

NativeCocoaUIInterface::NativeCocoaUIInterface() :
    NativeUIInterface(), m_StatusItem(NULL), m_DragNSView(nil)
{
    m_Wrapper = [[NativeCocoaUIInterfaceWrapper alloc] initWithUI:this];
}

NativeUICocoaConsole *NativeCocoaUIInterface::getConsole(bool create, bool *created) {
    if (created) *created = false;
    if (this->m_Console == NULL && create) {
        this->m_Console = new NativeUICocoaConsole;
        if (created) *created = true;
    }
    return this->m_Console;
}

void NativeCocoaUIInterface::onWindowCreated()
{
    NSWindow *window = NativeCocoaWindow(m_Win);

    m_DragNSView = [[NativeDragNSView alloc] initWithFrame:NSMakeRect(0, 0, width, height)];
    [[window contentView] addSubview:this->m_DragNSView];
    [this->m_DragNSView setResponder:NativeJSwindow::getNativeClass(m_NativeCtx->getNJS())];

    this->patchSDLView([window contentView]);

    [window setCollectionBehavior:
             NSWindowCollectionBehaviorFullScreenPrimary];

    [window setFrameAutosaveName:@"nativeMainWindow"];
    if (kNativeTitleBarHeight != 0) {
        [window setStyleMask:NSTexturedBackgroundWindowMask|NSTitledWindowMask];
    }

#if NIDIUM_ENABLE_HIDPI
    NSView *openglview = [window contentView];
    [openglview setWantsBestResolutionOpenGLSurface:YES];
#endif

}

void NativeCocoaUIInterface::setTitleBarRGBAColor(uint8_t r, uint8_t g,
    uint8_t b, uint8_t a)
{
    NSWindow *window = NativeCocoaWindow(m_Win);
    NSUInteger mask = [window styleMask];

    fprintf(stdout, "setting titlebar color\n");

    if ((mask & NSTexturedBackgroundWindowMask) == 0) {
        [window setStyleMask:mask|NSTexturedBackgroundWindowMask];
        [window setMovableByWindowBackground:NO];
        [window setOpaque:NO];
    }

    [window setBackgroundColor:[NSColor
        colorWithSRGBRed:((double)r)/255.
                   green:((double)g)/255.
                    blue:((double)b)/255.
                   alpha:((double)a)/255]];
}

void NativeCocoaUIInterface::initControls()
{
    NSWindow *window = NativeCocoaWindow(m_Win);
    NSButton *close = [window standardWindowButton:NSWindowCloseButton];
    NSButton *min = [window standardWindowButton:NSWindowMiniaturizeButton];
    NSButton *max = [window standardWindowButton:NSWindowZoomButton];

    memset(&this->controls, 0, sizeof(CGRect));

    if (close) {
        this->controls.closeFrame = close.frame;
    }
    if (min) {
        this->controls.minFrame = min.frame;
    }
    if (max) {
        this->controls.zoomFrame = max.frame;
    }
}

void NativeCocoaUIInterface::setWindowControlsOffset(double x, double y)
{
    NSWindow *window = NativeCocoaWindow(m_Win);
    NSButton *close = [window standardWindowButton:NSWindowCloseButton];
    NSButton *min = [window standardWindowButton:NSWindowMiniaturizeButton];
    NSButton *max = [window standardWindowButton:NSWindowZoomButton];

    if (close) {
        [close setFrame:CGRectMake(
            this->controls.closeFrame.origin.x+x,
            this->controls.closeFrame.origin.y-y,
            this->controls.closeFrame.size.width,
            this->controls.closeFrame.size.height)];
    }
    if (min) {
        [min setFrame:CGRectMake(
            this->controls.minFrame.origin.x+x,
            this->controls.minFrame.origin.y-y,
            this->controls.minFrame.size.width,
            this->controls.minFrame.size.height)];
    }
    if (max) {
        [max setFrame:CGRectMake(
            this->controls.zoomFrame.origin.x+x,
            this->controls.zoomFrame.origin.y-y,
            this->controls.zoomFrame.size.width,
            this->controls.zoomFrame.size.height)];
    }
}

void NativeCocoaUIInterface::openFileDialog(const char *files[],
    void (*cb)(void *nof, const char *lst[], uint32_t len), void *arg, int flags)
{
    NSOpenPanel* openDlg = [NSOpenPanel openPanel];

    if (files != NULL) {
        NSMutableArray *fileTypesArray = [NSMutableArray arrayWithCapacity:0];

        for (int i = 0; files[i] != NULL; i++) {
            [fileTypesArray addObject:[NSString stringWithCString:files[i] encoding:NSASCIIStringEncoding]];
        }
        [openDlg setAllowedFileTypes:fileTypesArray];
    }

    [openDlg setCanChooseFiles:(flags & kOpenFile_CanChooseFile ? YES : NO)];
    [openDlg setCanChooseDirectories:(flags & kOpenFile_CanChooseDir ? YES : NO)];
    [openDlg setAllowsMultipleSelection:(flags & kOpenFile_AlloMultipleSelection ? YES : NO)];

    //[openDlg runModal];

#if 1
    // TODO: set a flag so that nidium can't be refreshed and unset after the block is called

    [openDlg beginSheetModalForWindow:NativeCocoaWindow(m_Win) completionHandler:^(NSInteger result) {
        if (result == NSFileHandlingPanelOKButton) {
            uint32_t len = [openDlg.URLs count];
            const char **lst = (const char **)malloc(sizeof(char **) * len);

            int i = 0;
            for (NSURL *url in openDlg.URLs) {
                lst[i] = [[url relativePath] UTF8String];
                i++;
            }

            cb(arg, lst, i);

            free(lst);
        } else {
            cb(arg, NULL, 0);
        }
    }];

    this->makeMainGLCurrent();
#endif
}

void NativeCocoaUIInterface::runLoop()
{
    APE_timer_create(m_Gnet, 1, NativeProcessUI, (void *)this);
    APE_timer_create(m_Gnet, 1, NativeProcessSystemLoop, (void *)this);

    APE_loop_run(m_Gnet);
}

void NativeCocoaUIInterface::setWindowFrame(int x, int y, int w, int h)
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    NSWindow *nswindow = NativeCocoaWindow(m_Win);

    this->m_Width = w;
    this->m_Height = h;
    CGRect oldframe = [nswindow frame];

    int screen_width, screen_height;
    if (x == NATIVE_WINDOWPOS_CENTER_MASK || y == NATIVE_WINDOWPOS_CENTER_MASK) {
        this->getScreenSize(&screen_width, &screen_height);

        if (x == NATIVE_WINDOWPOS_CENTER_MASK) {
            x = (screen_width - w) / 2;
        }
        if (y == NATIVE_WINDOWPOS_CENTER_MASK) {
            y = (screen_height - h) / 2;
        }
    }

    if (x == NATIVE_WINDOWPOS_UNDEFINED_MASK) {
        x = oldframe.origin.x;
    }
    if (y == NATIVE_WINDOWPOS_UNDEFINED_MASK) {
        y = oldframe.origin.y;
    }

    CGRect newframe = [nswindow frameRectForContentRect:CGRectMake(x, y, w, h)];
    [nswindow setFrame:newframe display:YES];

    [pool drain];
}

#if 0
bool NativeCocoaUIInterface::makeMainGLCurrent()
{
    [((NSOpenGLContext *)m_mainGLCtx) makeCurrentContext];

    return true;
}

SDL_GLContext NativeCocoaUIInterface::getCurrentGLContext()
{
    return (SDL_GLContext)[NSOpenGLContext currentContext];
}
#endif

#if 0
bool NativeCocoaUIInterface::makeGLCurrent(SDL_GLContext ctx)
{
    [((NSOpenGLContext *)ctx) makeCurrentContext];

    return true;
}
#endif

static const char *drawRect_Associated_obj = "_NativeUIInterface";

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
@interface NativeDrawRectResponder : NSView
    - (void) drawRect:(NSRect)dirtyRect;
@end

@implementation NativeDrawRectResponder
- (void) drawRect:(NSRect)dirtyRect
{
    NSPointer *idthis = objc_getAssociatedObject(self, drawRect_Associated_obj);
    NativeCocoaUIInterface *UI = (NativeCocoaUIInterface *)idthis->m_Ptr;
    NativeContext *ctx = UI->getNativeContext();

    if (ctx && ctx->isSizeDirty()) {
        [(NSOpenGLContext *)UI->getGLContext() update];
        ctx->sizeChanged(UI->getWidth(), UI->getHeight());
    }
}
@end

void NativeCocoaUIInterface::patchSDLView(NSView *sdlview)
{
    Class SDLView = NSClassFromString(@"SDLView");
    Method drawRectNewMeth = class_getInstanceMethod([NativeDrawRectResponder class], @selector(drawRect:));
    IMP drawRectNewImp = method_getImplementation(drawRectNewMeth);
    const char *types = method_getTypeEncoding(drawRectNewMeth);

    class_replaceMethod(SDLView, @selector(drawRect:), drawRectNewImp, types);

    NSPointer *idthis = [[NSPointer alloc] initWithPtr:this];

    objc_setAssociatedObject(sdlview, drawRect_Associated_obj, idthis, OBJC_ASSOCIATION_RETAIN_NONATOMIC);

    [idthis release];
}

void NativeCocoaUIInterface::enableSysTray(const void *imgData,
    size_t imageDataSize)
{
#if 0
    m_StatusItem = [[[NSStatusBar systemStatusBar] statusItemWithLength:NSVariableStatusItemLength] retain];


    NSImage *icon = [NSApp applicationIconImage];
    [icon setScalesWhenResized:YES];
    [icon setSize: NSMakeSize(20.0, 20.0)];

    m_StatusItem.image = icon;
    m_StatusItem.highlightMode = YES;

    NSMenu *stackMenu = [[NSMenu alloc] initWithTitle:@""];

    NSMenuItem *menuOpen =
        [[NSMenuItem alloc] initWithTitle:@"Open" action:nil keyEquivalent:@""];
    NSMenuItem *menuClose =
        [[NSMenuItem alloc] initWithTitle:@"Close" action:nil keyEquivalent:@""];

    [stackMenu addItem:menuOpen];
    [stackMenu addItem:menuClose];

    [menuOpen setEnabled:YES];
    [menuClose setEnabled:YES];

    [m_StatusItem setMenu:stackMenu];
    m_StatusItem.title = @"";
#endif

    this->renderSystemTray();
}

void NativeCocoaUIInterface::disableSysTray()
{
    if (!m_StatusItem) {
        return;
    }
    [[NSStatusBar systemStatusBar] removeStatusItem:m_StatusItem];
    [m_StatusItem release];

    m_StatusItem = NULL;
}

void NativeCocoaUIInterface::quit()
{
    this->stopApplication();
    SDL_Quit();
    [[NSApplication sharedApplication] terminate:nil];
}

void NativeCocoaUIInterface::hideWindow()
{
    if (!m_Hidden) {
        m_Hidden = true;
        SDL_HideWindow(m_Win);

        /* Hide the Application (Dock, etc...) */
        [NSApp setActivationPolicy: NSApplicationActivationPolicyAccessory];

        set_timer_to_low_resolution(&this->m_Gnet->timersng, 1);
    }
}

void NativeCocoaUIInterface::showWindow()
{
    if (m_Hidden) {
        m_Hidden = false;
        SDL_ShowWindow(m_Win);

        [NSApp setActivationPolicy: NSApplicationActivationPolicyRegular];

        set_timer_to_low_resolution(&this->m_Gnet->timersng, 0);
    }
}

void NativeCocoaUIInterface::renderSystemTray()
{
    NativeSystemMenuItem *item = m_SystemMenu.items();
    if (!item) {
        return;
    }

    if (!m_StatusItem) {
        m_StatusItem = [[[NSStatusBar systemStatusBar] statusItemWithLength:NSVariableStatusItemLength] retain];
        m_StatusItem.highlightMode = YES;

        size_t icon_len, icon_width, icon_height;
        const uint8_t *icon_custom = m_SystemMenu.getIcon(&icon_len,
                                        &icon_width, &icon_height);

        NSImage *icon;

        if (!icon_len || !icon_custom) {
            icon = [NSApp applicationIconImage];
            [icon setSize: NSMakeSize(20.0, 20.0)];
        } else {

            NSBitmapImageRep *rep = [[NSBitmapImageRep alloc] initWithBitmapDataPlanes:(unsigned char **)&icon_custom
                                                                            pixelsWide:icon_width
                                                                            pixelsHigh:icon_height
                                                                            bitsPerSample:8
                                                                            samplesPerPixel:4
                                                                            hasAlpha:YES
                                                                            isPlanar:NO
                                                                            colorSpaceName:NSDeviceRGBColorSpace
                                                                            bitmapFormat:NSAlphaNonpremultipliedBitmapFormat
                                                                            bytesPerRow:icon_width*4
                                                                            bitsPerPixel:32];

            icon = [[[NSImage alloc] initWithSize:NSMakeSize(32.f, 32.f)] autorelease];

            [icon addRepresentation:rep];
        }
        m_StatusItem.image = icon;
    }

    NSMenu *stackMenu = [m_Wrapper renderSystemTray];

    [m_StatusItem setMenu:stackMenu];
}

void NativeCocoaUIInterface::setSystemCursor(CURSOR_TYPE cursorvalue)
{
    switch(cursorvalue) {
        case NativeCocoaUIInterface::ARROW:
            [[NSCursor arrowCursor] set];
            break;
        case NativeCocoaUIInterface::BEAM:
            [[NSCursor IBeamCursor] set];
            break;
        case NativeCocoaUIInterface::CROSS:
            [[NSCursor crosshairCursor] set];
            break;
        case NativeCocoaUIInterface::POINTING:
            [[NSCursor pointingHandCursor] set];
            break;
        case NativeCocoaUIInterface::CLOSEDHAND:
            [[NSCursor closedHandCursor] set];
            break;
        case NativeCocoaUIInterface::RESIZELEFTRIGHT:
            [[NSCursor resizeLeftRightCursor] set];
            break;
        case NativeCocoaUIInterface::HIDDEN:
            SDL_ShowCursor(0);
            break;
        default:
            break;
    }
}

@implementation NativeCocoaUIInterfaceWrapper

- (id) initWithUI:(NativeCocoaUIInterface *)ui
{
    if (self = [super init]) {
        self->base = ui;
    }
    return self;
}

- (void) menuClicked:(id)sender
{
    NSString *identifier = [sender representedObject];

    NativeJSwindow *window = NativeJSwindow::getNativeClass(self->base->m_NativeCtx->getNJS());
    if (window) {
        window->systemMenuClicked([identifier cStringUsingEncoding:NSUTF8StringEncoding]);
    }
}

- (NSMenu *) renderSystemTray
{
    NativeCocoaUIInterface *ui = self->base;
    NativeSystemMenu &m_SystemMenu = ui->getSystemMenu();

    NativeSystemMenuItem *item = m_SystemMenu.items();
    if (!item) {
        return nil;
    }

    NSMenu *stackMenu = [[[NSMenu alloc] initWithTitle:@""] retain];

    while (item) {
        NSString *title = [NSString stringWithCString:item->title() encoding:NSUTF8StringEncoding];
        NSString *identifier = [NSString stringWithCString:item->id() encoding:NSUTF8StringEncoding];
        NSMenuItem *curMenu;
        if ([title isEqualToString:@"-"]) {
            curMenu = [NSMenuItem separatorItem];
        } else {
            curMenu =
                [[[NSMenuItem alloc] initWithTitle:title action:@selector(menuClicked:) keyEquivalent:@""] autorelease];
        }
        [stackMenu addItem:curMenu];
        [curMenu setEnabled:YES];

        item = item->m_Next;
        curMenu.target = self;

        [curMenu setRepresentedObject:identifier];
    }

    return stackMenu;
}

@end
