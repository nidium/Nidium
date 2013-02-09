class NativeJS;

class NativeUIInterface
{
    public:
        enum CURSOR_TYPE {
            ARROW,
            BEAM,
            CROSS,
            CLOSEDHAND,
            OPENHAND,
            POINTING,
            RESIZELEFT,
            RESIZERIGHT,
            RESIZELEFTRIGHT,
            RESIZEUP,
            RESIZEDOWN,
            RESIZEUPDOWN,
            NOCHANGE
        } currentCursor;
        NativeJS *NJS;
        struct SDL_Window *win;
        struct _ape_global *gnet;
        NativeUIInterface();
        void createWindow();
        void setWindowTitle(const char *);
        void setCursor(CURSOR_TYPE);
        void runLoop();
};
