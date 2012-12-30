class NativeJS;

class NativeUIInterface
{
    public:
        NativeJS *NJS;
        struct SDL_Window *win;
        struct _ape_global *gnet;
        NativeUIInterface();
        void createWindow();
        void setWindowTitle(const char *);
        void runLoop();
};
