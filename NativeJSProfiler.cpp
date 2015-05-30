#include "NativeJSProfiler.h"
#include "NativeFile.h"
#include "NativeJS.h"
#include "NativeJSExposer.h"

#if defined(__MACH__) && defined(__APPLE__)
#include <mach/mach_time.h>
#endif
#include <sstream> 

/* {{{ Utilities */
/* XXX : Dono if Nidium should be C++11 (in that case we could use std::to_string) */
template <typename T>
std::string number2string(T Number)
{
    std::ostringstream ss;
    ss << Number;
    return ss.str();
}

/* XXX : Move to NativePath ? */
char *relativizePath(const char *root, char *file)
{
    int len = strlen(root);
    for (int i = 0; i < len; i++) {
        if (root[i] != file[i]) {
            return &file[i];
        }
    }
    return &file[len];
}

/* From https://code.google.com/p/gperftools/source/browse/src/base/cycleclock.h
 * XXX : Move to NativeUtils ?  */
inline uint64_t getTSC()
{
#if defined(__MACH__) && defined(__APPLE__)
    return mach_absolute_time();
#elif defined(__i386__)
    uint64_t ret;
    __asm__ volatile ("rdtsc" : "=A" (ret) );
    return ret;
#elif defined(__x86_64__) || defined(__amd64__)
    uint64_t low, high;
    __asm__ volatile ("rdtsc" : "=a" (low), "=d" (high));
    return (high << 32) | low;
#elif defined(__ia64__)
    uint64_t itc;
    asm("mov %0 = ar.itc" : "=r" (itc));
    return itc;
#elif defined(_MSC_VER)
    return __rdtsc();
#endif
}

// }}}

/* {{{ NativeProfile JS class definition */
static void native_profile_finalizer(JSFreeOp *fop, JSObject *obj);

static JSClass native_profile_class = {
    "NativeProfile", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_DeletePropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, native_profile_finalizer,
    nullptr, nullptr, nullptr, nullptr, JSCLASS_NO_INTERNAL_MEMBERS
};

static bool native_profile_tojs(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_profile_tocachegrind(JSContext *cx, unsigned argc, JS::Value *vp);

static JSFunctionSpec native_profile_funcs[] = {
    JS_FN("toJSObject", native_profile_tojs, 0, 0),
    JS_FN("dump", native_profile_tocachegrind, 1, 0),
    JS_FS_END
};
/* }}} */

/* {{{ NativeProfiler */
NativeProfiler *NativeProfiler::m_Instance = NULL;

NativeProfiler *NativeProfiler::getInstance(JSContext *cx) {
    /* TODO : Handle multiple cx ? */
    if (!NativeProfiler::m_Instance) {
        NativeProfiler::m_Instance = new NativeProfiler(cx);
    }

    return NativeProfiler::m_Instance;
}

void NativeProfiler::start(const char *name) 
{
    if (m_Running) return;
    m_Running = true;

    JS_SetCallHook(JS_GetRuntime(m_Cx), NativeProfiler::trace, this);
    JS_SetRuntimeDebugMode(JS_GetRuntime(m_Cx), true);

    m_MainEntry = this->add("Profiler", "start", 0, NULL, 0);
    m_MainEntry->enter();
    m_LastEntryEnter = m_MainEntry;
}

void NativeProfiler::stop() 
{
    m_MainEntry->exit();
    JS_SetCallHook(JS_GetRuntime(m_Cx), NULL, NULL);
    m_Running = false;
}

void *NativeProfiler::trace(JSContext *cx, JSAbstractFramePtr frame, bool isConstructing,
        bool before, bool *ok, void *closure) 
{
    
    NativeProfileEntry *entry;
    NativeProfiler *profiler = NativeProfiler::getInstance(cx);

    if (before) {
        char *funName;
        JS::RootedString jfunName(cx);
        char *scriptName;
        unsigned lineno;
        unsigned parentLineNo;

        JSScript *script = frame.script();
        JSAutoByteString name;

        if (script) {
            scriptName = relativizePath(NativeJS::getNativeClass(cx)->getPath(), (char *)JS_GetScriptFilename(cx, script));
            lineno = JS_GetScriptBaseLineNumber(cx, script);
        } else {
            /* Don't think we will get here since JS_SetCallHook no longer
             * works with native function */
            scriptName = (char *)"(native)";
            lineno = 0;
        }

        JSFunction *fun = frame.maybeFun();
        if (fun) {
            jfunName = JS_GetFunctionId(fun);
            if (jfunName) {
                name.initBytes(JS_EncodeString(cx, jfunName));
                funName = name.ptr();
            } else {
                int size = 8 + strlen(scriptName) + 8;
                funName = (char *)malloc(size);
                snprintf(funName, size, "%s:%s@%d", scriptName, "(inline)", lineno);
            }
        } else {
            funName = (char *)"N/A";
        }

        JS::RootedScript parentScript(cx);
        if (!JS_DescribeScriptedCaller(cx, parentScript.address(), &parentLineNo)) {
            parentLineNo = 0;
        }

        entry = profiler->add(scriptName, funName, lineno, profiler->m_LastEntryEnter, parentLineNo);
        entry->enter();

        if (jfunName == NULL) {
            free(funName);
        }

        profiler->m_LastEntryEnter = entry;

        return entry;
    } else {
        entry = static_cast<NativeProfileEntry *>(closure);
        entry->exit();

        profiler->m_LastEntryEnter = entry->getParent();
        profiler->m_LastEntryExit = entry;

        return NULL;
    }

    return entry;
}

JSObject *NativeProfiler::getJSObject() 
{
    JS::RootedObject obj(m_Cx, JS_NewObject(m_Cx, &native_profile_class, JS::NullPtr(), JS::NullPtr()));

    JS_DefineFunctions(m_Cx, obj, native_profile_funcs);
    JS_SetPrivate(obj, static_cast<void*>(this));

    return obj;
}

JSObject *NativeProfiler::toJSObject() 
{
    JS::RootedObject obj(m_Cx, JS_NewObject(m_Cx, NULL, JS::NullPtr(), JS::NullPtr()));

    for (NativeHash<NativeProfileEntry *>::iterator entry = m_Entries.begin(), end = m_Entries.end(); entry != end; ++entry) {
        JS::RootedValue val(m_Cx, OBJECT_TO_JSVAL(entry->toJSObject(m_Cx)));
        JS_SetProperty(m_Cx, obj, entry->getSignature(), val);
    }

    return obj;
}

bool NativeProfiler::toCacheGrind(const char *dest)
{
    int err;
    NativeFile *out = new NativeFile(dest);

    out->openSync("w+", &err);
    if (err != 0) {
        delete out;
        return false;
    }

    std::string ret = "";

    for (NativeHash<NativeProfileEntry *>::iterator entry = m_Entries.begin(), end = m_Entries.end(); entry != end; ++entry) {
        ret = entry->toCacheGrind() + "\n\n" + ret;
    }
    
    ret = "events: Cycles\n" + ret;

    out->writeSync((char *)ret.c_str(), ret.length(), &err);
    if (err) {
        out->closeSync();
        delete out;
        return false;
    }

    delete out;
    return true;
}
/* }}} */

/* {{{ NativeProfileEntry */
NativeProfileEntry *NativeProfiler::add(const char *script, const char *fun, int line, NativeProfileEntry *parent, unsigned parentLine)
{
    NativeProfileEntry *entry;
    char *signature = NativeProfileEntry::generateSignature(script, fun, line);

    entry = m_Entries.get(signature);
    if (!entry) {
        entry = new NativeProfileEntry(script, fun, line, signature, parent); 
        m_Entries.set(signature, entry);
    }

    if (parent != NULL) {
        parent->addChild(entry, parentLine);
    }

    free(signature);
    return entry;
}

void NativeProfileEntry::enter()
{
    m_StartTime = NativeUtils::getTick(false);
    m_StartTSC = getTSC();
    m_TotalCall++;
}

void NativeProfileEntry::exit()
{
    uint64_t time = NativeUtils::getTick(false) - m_StartTime;
    uint64_t tsc = getTSC() - m_StartTSC;
    m_TotalTime += time;
    m_TotalTSC += tsc;

    if (m_Parent && m_Parent->m_CurrentChild) {
        m_Parent->m_CurrentChild->update(time, tsc);
        m_Parent->m_CurrentChild = NULL;
    }
}

NativeProfileChildEntry *NativeProfileEntry::addChild(NativeProfileEntry *entry, unsigned line)
{
    // TODO : Handle recursive call
    NativeProfileChildEntry *child = m_Childs.get(entry->getSignature());
    if (!child) {
        child = new NativeProfileChildEntry(entry, line);
        m_Childs.set(entry->getSignature(), child);
    }

    m_CurrentChild = child;

    return child;
}

char *NativeProfileEntry::generateSignature(const char *script, const char *fun, int line)
{
    int size = strlen(script) + strlen(fun) + 8;
    char *signature = (char *)malloc(size * sizeof(char));
    snprintf(signature, size, "%s:%s@%d", script, fun, line);

    return signature;
}

JSObject *NativeProfileEntry::toJSObject(JSContext *cx)
{
    uint64_t childCostTSC = 0;
    uint64_t childCostTime = 0;
    JS::RootedObject obj(cx, JS_NewObject(cx, NULL, JS::NullPtr(), JS::NullPtr()));
    JS::RootedObject childsObj(cx, JS_NewObject(cx, NULL, JS::NullPtr(), JS::NullPtr()));

    JS::RootedValue childs(cx, OBJECT_TO_JSVAL(childsObj));
    JS_SetProperty(cx, obj, "childs", childs);
    JS_SetPrivate(obj, this);

    for (NativeHash<NativeProfileChildEntry *>::iterator child = m_Childs.begin(), end = m_Childs.end(); child != end; ++child) {
        
        JS::RootedValue val(cx, OBJECT_TO_JSVAL(child->toJSObject(cx)));
        JS_SetProperty(cx, childsObj, child->getEntry()->getSignature(), val);

        childCostTSC += child->getTotalTSC();
        childCostTime += child->getTotalTime();
    }

    JS::RootedValue script(cx, STRING_TO_JSVAL(JS_NewStringCopyZ(cx, m_Script)));
    JS::RootedValue fun(cx, STRING_TO_JSVAL(JS_NewStringCopyZ(cx, m_Fun)));
    JS::RootedValue line(cx, INT_TO_JSVAL(m_Line));

    JS::RootedValue total(cx, INT_TO_JSVAL(m_TotalCall));
    JS::RootedValue time(cx, DOUBLE_TO_JSVAL((double)(m_TotalTime - childCostTime)/1000000.));
#if DEBUG
    JS::RootedValue mean(cx, DOUBLE_TO_JSVAL((double)m_MeanTime/1000000.));
    JS::RootedValue max(cx, DOUBLE_TO_JSVAL((double)m_MaxTime/1000000.));
    JS::RootedValue min(cx, DOUBLE_TO_JSVAL((double)m_MinTime/1000000.));
#endif
    JS::RootedValue tsc(cx, INT_TO_JSVAL(m_TotalTSC - childCostTSC));

    JS_SetProperty(cx, obj, "script", script);
    JS_SetProperty(cx, obj, "function", fun);
    JS_SetProperty(cx, obj, "line", line);

    JS_SetProperty(cx, obj, "call", total);

    JS_SetProperty(cx, obj, "tsc", tsc);
    JS_SetProperty(cx, obj, "time", time);
    //JS_SetProperty(cx, obj, "mean", mean);
    //JS_SetProperty(cx, obj, "max", max);
    //JS_SetProperty(cx, obj, "min", min);

    return obj;
}

std::string NativeProfileEntry::toCacheGrind()
{
    std::string ret = "";
    uint64_t childCost = 0;

    for (NativeHash<NativeProfileChildEntry *>::iterator child = m_Childs.begin(), end = m_Childs.end(); child != end; ++child) {
        ret = child->toCacheGrind() + ret;
        childCost += child->getTotalTSC();
    }

    ret = "fl=" + std::string(m_Script) + "\n" +
          "fn=" + std::string(m_Fun) + "\n" +
          number2string<int>(m_Line) + " " + number2string<uint64_t>(m_TotalTSC - childCost) + "\n" +
          ret;

    return ret;
}

/* }}} */

/* {{{ NativeProfileChildEntry */
JSObject *NativeProfileChildEntry::toJSObject(JSContext *cx)
{
    JS::RootedObject obj(cx, JS_NewObject(cx, NULL, JS::NullPtr(), JS::NullPtr()));

    JS::RootedValue script(cx, STRING_TO_JSVAL(JS_NewStringCopyZ(cx, m_Entry->getScript())));
    JS::RootedValue fun(cx, STRING_TO_JSVAL(JS_NewStringCopyZ(cx, m_Entry->getFunction())));
    JS::RootedValue line(cx, INT_TO_JSVAL(m_Entry->getLine()));
    JS::RootedValue callLine(cx, INT_TO_JSVAL(m_Line));

    JS::RootedValue total(cx, INT_TO_JSVAL(m_TotalCall));
    JS::RootedValue time(cx, DOUBLE_TO_JSVAL((double)m_TotalTime/1000000.));
    //JS::RootedValue mean(DOUBLE_TO_JSVAL((double)m_MeanTime/1000000.));
    //JS::RootedValue max(DOUBLE_TO_JSVAL((double)m_MaxTime/1000000.));
    //JS::RootedValue min(DOUBLE_TO_JSVAL((double)m_MinTime/1000000.));
    JS::RootedValue tsc(cx, INT_TO_JSVAL(m_TotalTSC));

    JS_SetProperty(cx, obj, "script", script);
    JS_SetProperty(cx, obj, "function", fun);
    JS_SetProperty(cx, obj, "line", line);

    JS_SetProperty(cx, obj, "call", total);
    JS_SetProperty(cx, obj, "callLine", callLine);

    JS_SetProperty(cx, obj, "tsc", tsc);
    JS_SetProperty(cx, obj, "time", time);
    //JS_SetProperty(cx, obj, "mean", mean);
    //JS_SetProperty(cx, obj, "max", max);
    //JS_SetProperty(cx, obj, "min", min);

    return obj;
}

std::string NativeProfileChildEntry::toCacheGrind()
{
    std::string ret;

    ret = "cfl=" + std::string(m_Entry->getScript()) + "\n" +
          "cfn=" + std::string(m_Entry->getFunction()) + "\n" + 
          "calls=" + number2string<int>(m_TotalCall) + "\n" + 
          number2string<int>(m_Entry->getLine()) + " " + number2string<uint64_t>(m_TotalTSC) + "\n";

    return ret;
}
/* }}} */

/* {{{ NativeProfile JS function implementation */
static bool native_profile_tojs(JSContext *cx, unsigned argc, JS::Value *vp)
{
	JSNATIVE_PROLOGUE_CLASS(NativeProfiler, &native_profile_class);
    JS::RootedObject obj(cx, CppObj->toJSObject());

    args.rval().setObjectOrNull(obj);

    return true;
}

static bool native_profile_tocachegrind(JSContext *cx, unsigned argc, JS::Value *vp)
{
	JSNATIVE_PROLOGUE_CLASS(NativeProfiler, &native_profile_class);

    JS::RootedString tmp(cx);

    if (!JS_ConvertArguments(cx, args, "S", tmp)) {
        return false;
    }

    JSAutoByteString out(cx, tmp);

    if (!CppObj->toCacheGrind(out.ptr())) {
        JS_ReportError(cx, "Failed to export cachegrind profile");
        return false;
    }
    
    return true;
}

static void native_profile_finalizer(JSFreeOp *fop, JSObject *obj) {
    NativeProfiler *profiler = static_cast<NativeProfiler *>(JS_GetPrivate(obj));
    delete profiler;
}
/* }}} */
