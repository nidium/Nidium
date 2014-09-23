#ifndef nativejstracer_h__
#define nativejstracer_h__
#include "NativeHash.h"
#include "NativeUtils.h"
#include <jsapi.h>
#include <jsfriendapi.h>
#include <jsdbgapi.h>
#include <string>

#define NATIVE_MAX_PROFILER 1024

class NativeProfileEntry;
class NativeProfileChildEntry;

class NativeProfiler
{
    public:
        static NativeProfiler *getInstance(JSContext *cx);
        void start(const char *name);
        void stop();
        JSObject *getJSObject();
        JSObject *toJSObject();
        bool toCacheGrind(const char *dest);
    private:
        NativeProfiler(JSContext *cx) 
            : m_Cx(cx), m_Running(false), 
              m_MainEntry(NULL), m_LastEntryEnter(NULL), m_LastEntryExit(NULL)
        {
            m_Entries.setAutoDelete(true);
        }

        static NativeProfiler *m_Instance;

        static void *trace(JSContext *cx, JSAbstractFramePtr frame, bool isConstructing,
                      JSBool before, JSBool *ok, void *closure);
        NativeProfileEntry *add(const char *script, const char *fun, int line, NativeProfileEntry *parent, unsigned parentLine);

        JSContext *m_Cx;
        bool m_Running;

        NativeProfileEntry *m_MainEntry;
        NativeHash<NativeProfileEntry *> m_Entries;

        NativeProfileEntry *m_LastEntryEnter;
        NativeProfileEntry *m_LastEntryExit;
};

class NativeProfileChildEntry
{
    public:
        NativeProfileChildEntry(NativeProfileEntry *entry, unsigned line)
            : m_Entry(entry), m_TotalTime(0), m_TotalTSC(0), m_TotalCall(0), m_Line(line)
        {
        }

        void update(unsigned time, unsigned tsc)
        {
            m_TotalTime += time;
            m_TotalTSC += tsc;
            m_TotalCall++;
        }

        NativeProfileEntry *getEntry()
        {
           return m_Entry; 
        }

        unsigned getTotalTime() {
            return m_TotalTime;
        }

        unsigned getTotalTSC() {
            return m_TotalTSC;
        }

        unsigned getTotalCall() {
            return m_TotalCall;
        }

        unsigned getCallLine() {
            return m_Line;
        }

        JSObject *toJSObject(JSContext *cx);
        std::string toCacheGrind();

    private:
        NativeProfileEntry *m_Entry;

        unsigned m_TotalTime;
        unsigned m_TotalTSC;
        unsigned m_TotalCall;
        unsigned m_Line;
};

class NativeProfileEntry
{
    public:
        NativeProfileEntry(const char *script, const char *fun, unsigned line, char *signature,
            NativeProfileEntry *parent)
            : m_Script(strdup(script)), m_Fun(strdup(fun)), m_Line(line), m_Signature(strdup(signature)),
              m_MeanTime(0), m_MaxTime(0), m_MinTime(0), m_TotalCall(0), m_TotalTime(0), m_TotalTSC(0),
              m_CurrentChild(NULL), m_Parent(parent)
        {
            m_Childs.setAutoDelete(true);
        }

        ~NativeProfileEntry()
        {
            free(m_Script);
            free(m_Fun);
            free(m_Signature);
        }

        void enter();
        void exit();

        NativeProfileChildEntry *addChild(NativeProfileEntry *entry, unsigned line);

        JSObject *toJSObject(JSContext *cx);
        std::string toCacheGrind();

        static char *generateSignature(const char *script, const char *fun, int line);

        unsigned getTotalTime() {
            return m_TotalTime;
        }

        unsigned getTotalTSC() {
            return m_TotalTSC;
        }

        unsigned getTotalCall() {
            return m_TotalCall;
        }

        unsigned getLine() {
            return m_Line;
        }

        const char *getScript() {
            return m_Script;
        }
    
        const char *getFunction() {
            return m_Fun;
        }

        const char *getSignature() 
        {
            return m_Signature;
        }

        NativeProfileEntry *getParent() 
        {
            return m_Parent;
        }
    private:
        char *m_Script;
        char *m_Fun;
        unsigned m_Line;
        char *m_Signature;

        uint64_t m_StartTime;
        uint64_t m_StartTSC;

        unsigned m_MeanTime;
        unsigned m_MaxTime;
        unsigned m_MinTime;
        unsigned m_TotalCall;
        unsigned m_TotalTime;
        uint64_t m_TotalTSC;

        NativeProfileEntry *m_Parent;
        NativeProfileChildEntry *m_CurrentChild;
        NativeHash<NativeProfileChildEntry *> m_Childs;
};

#endif
