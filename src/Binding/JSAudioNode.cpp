/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include "Binding/JSAudioNode.h"

#include "Binding/JSEvents.h"
#include "Binding/JSVideo.h"
#include "Binding/JSUtils.h"

using namespace Nidium::AV;

namespace Nidium {
namespace Binding {

#define JS_PROPAGATE_ERROR(cx, ...)       \
    JS_ReportErrorUTF8(cx, __VA_ARGS__);  \
    if (!JS_ReportPendingException(cx)) { \
        JS_ClearPendingException(cx);     \
    }

// {{{ Utilities
JS::Value setArgsToObject(JSContext *cx, JS::CallArgs &args)
{
    if (args.length() == 1) {
        // Object with key/value pairs to set
        if (args[0].isObject()) {
            return args[0];
        } else {
            JS_ReportErrorUTF8(cx, "Invalid argument");
            return JS::NullValue();
        }
    } else {
        // Key & value as argument
        if (!args[0].isString()) {
            JS_ReportErrorUTF8(cx, "First argument must be a string");
            return JS::NullValue();
        }

        JSObjectBuilder o(cx);
        JSAutoByteString name(cx, args[0].toString());

        o.set(name.ptr(), args[1]);

        return o.jsval();
    }
}
// }}}

// {{{ JSAudioNode
void JSAudioNode::releaseNode()
{
    JSAudioContext *jaudio = JSAudioContext::GetContext();
    AV::Audio *audio       = jaudio->m_Audio;

    m_IsReleased = true;

    // Block Audio threads execution.
    // While the node is destructed we don't want any thread
    // to call some method on a node that is being destroyed
    audio->lockQueue();
    audio->lockSources();

    // Wakeup audio thread. This will flush all pending messages.
    // That way, we are sure nothing will need to be processed
    // later for this node.
    audio->wakeup();

    // Delete (invalidate) all JSAudioNodeLink
    for (auto const& link: m_Links) {
        delete link;
    }

    // Remove JS node from nodes linked list
    jaudio->removeNode(this);

    delete m_Node;

    m_Node = nullptr;

    audio->unlockQueue();
    audio->unlockSources();
}

bool JSAudioNode::JS__set(JSContext *cx, JS::CallArgs &args)
{
    JS::RootedValue val(cx, setArgsToObject(cx, args));
    if (!val.isObject()) {
        return false;
    }

    ArgType type;
    void *value;
    int intVal       = 0;
    double doubleVal = 0;
    unsigned long size;

    JS::RootedObject props(cx, val.toObjectOrNull());

    JS::Rooted<JS::IdVector> ida(cx, JS::IdVector(cx));

    JS_Enumerate(cx, props, &ida);

    for (size_t i = 0; i < ida.length(); i++) {
        // Retrieve the current key & value
        JS::RootedId id(cx, ida[i]);
        JSAutoByteString key(cx, JSID_TO_STRING(id));
        JS::RootedValue val(cx);
        if (!JS_GetPropertyById(cx, props, id, &val)) {
            break;
        }

        // Convert the value to the appropriate native type
        if (val.isInt32()) {
            type   = AV::INT;
            size   = sizeof(int);
            intVal = (int)val.toInt32();
            value = &intVal;
        } else if (val.isDouble()) {
            type      = AV::DOUBLE;
            size      = sizeof(double);
            doubleVal = val.toNumber();
            value     = &doubleVal;
        } else {
            JS_ReportErrorUTF8(cx, "Unsuported value for property %s\n", key.ptr());
            return false;
        }

        // And set the value on the node
        if (!m_Node->set(key.ptr(), type, value, size)) {
            JS_ReportErrorUTF8(cx, "Unknown argument name %s\n", key.ptr());
            return false;
        }
    }

    return true;
}
// }}}

// {{{ JSAudioNodeSource
JSPropertySpec *JSAudioNodeSource::ListProperties()
{
    static JSPropertySpec props[]
        = { CLASSMAPPER_PROP_GS(JSAudioNodeSource, position),
            CLASSMAPPER_PROP_G(JSAudioNodeSource, duration),
            CLASSMAPPER_PROP_G(JSAudioNodeSource, metadata),
            CLASSMAPPER_PROP_G(JSAudioNodeSource, bitrate), JS_PS_END };

    return props;
}

JSFunctionSpec *JSAudioNodeSource::ListMethods()
{
    static JSFunctionSpec funcs[]
        = { CLASSMAPPER_FN(JSAudioNodeSource, open, 1),
            CLASSMAPPER_FN(JSAudioNodeSource, play, 0),
            CLASSMAPPER_FN(JSAudioNodeSource, pause, 0),
            CLASSMAPPER_FN(JSAudioNodeSource, stop, 0),
            CLASSMAPPER_FN(JSAudioNodeSource, close, 0),
            JS_FS_END };

    return funcs;
}
// }}}

// {{{ JSAudioNodeCustomBase
JSAudioNodeCustomBase::JSAudioNodeCustomBase()
{
    NIDIUM_PTHREAD_VAR_INIT(&m_ShutdownWait);
    JSAudioContext *audioContext = JSAudioContext::GetContext();

    for (int i = 0; i < END_FN; i++) {
        m_TransferableFuncs[i]
            = new JSTransferableFunction(audioContext->m_JsTcx,
                                         audioContext->m_JsGlobalObj);
        m_TransferableFuncs[i]->setPrivate(this);
    }
}

void JSAudioNodeCustomBase::onMessage(const Core::SharedMessages::Message &msg)
{
    if (this->isReleased()) return;

    if (msg.event() == JSAudioNodeThreaded::THREADED_MESSAGE) {
        JSTransferable *t = static_cast<JSTransferable *>(msg.dataPtr());
        JSContext *cx = t->getJSContext();
        JS::RootedValue val(cx, t->get());
        JS::RootedValue ev(cx);

        JS::RootedObject evObj(cx, JSEvents::CreateEventObject(cx));
        JSObjectBuilder evBuilder(cx, evObj);

        evBuilder.set("data", val);

        ev = evBuilder.jsval();

        this->fireJSEvent("message", &ev);

        delete t;
    }
}

bool JSAudioNodeCustomBase::JS__set(JSContext *cx, JS::CallArgs &args)
{
    JS::RootedValue val(cx, setArgsToObject(cx, args));
    if (!val.isObject()) {
        return false;
    }

    JSTransferable *transferable
        = new JSTransferable(cx, this->getAudioContext()->m_JsTcx,
                             this->getAudioContext()->m_JsGlobalObj, val);

    transferable->setPrivate(this);

    m_Node->callback(JSAudioNodeCustomBase::OnSet, transferable);

    return true;
}

bool JSAudioNodeCustomBase::JS__assignProcessor(JSContext *cx,
                                                JS::CallArgs &args)
{
    JSTransferableFunction *fun
        = this->getFunction(JSAudioNodeCustomBase::PROCESS_FN);

    if (!fun->set(cx, args[0])) {
        JS_ReportErrorUTF8(cx, "First argument must be a function or null");
        return false;
    }

    AudioNodeCustom *node = this->getNode<AudioNodeCustom>();

    if (args[0].isNull()) {
        node->setProcessor(nullptr, nullptr);
    } else {
        node->setProcessor(JSAudioNodeCustomBase::Processor,
                           static_cast<void *>(this));
    }

    return true;
}

bool JSAudioNodeCustomBase::JS__assignInit(JSContext *cx, JS::CallArgs &args)
{
    JSTransferableFunction *fun
        = this->getFunction(JSAudioNodeCustomBase::INIT_FN);

    if (!fun->set(cx, args[0])) {
        JS_ReportErrorUTF8(cx, "First argument must be a function or null");
        return false;
    }

    m_Node->callback(JSAudioNodeCustomBase::OnAssignInit, this);

    return true;
}

bool JSAudioNodeCustomBase::JS__assignSetter(JSContext *cx, JS::CallArgs &args)
{
    JSTransferableFunction *fun
        = this->getFunction(JSAudioNodeCustomBase::SETTER_FN);

    if (!fun->set(cx, args[0])) {
        JS_ReportErrorUTF8(cx, "First argument must be a function or null");
        return false;
    }

    return true;
}

void JSAudioNodeCustomBase::Processor(const struct NodeEvent *ev)
{
    JS_AUDIO_THREAD_PROLOGUE()

    JSAudioNodeCustomBase *jsNode
        = static_cast<JSAudioNodeCustomBase *>(ev->custom);
    JSTransferableFunction *fn
        = jsNode->getFunction(JSAudioNodeCustomBase::PROCESS_FN);

    jsNode->initThreadedNode();

    // XXX : We should re-use the same JSObject each time
    JSAudioNodeBuffers *buffers
        = new JSAudioNodeBuffers(audioContext, ev->data, jsNode,
                                 audioContext->m_Audio->m_OutputParameters);

    JS::RootedObject obj(cx, buffers->getJSObject());
    JS::RootedObject threadedNodeObj(cx, jsNode->m_ThreadedNode->getJSObject());
    JS::RootedValue rval(cx);
    JS::AutoValueArray<2> params(cx);
    params[0].setObjectOrNull(obj);
    params[1].setObjectOrNull(audioContext->m_JsGlobalObj);

    fn->call(threadedNodeObj, params, &rval);

    for (unsigned int i = 0; i < buffers->getCount(); i++) {
        JS::RootedValue val(cx);
        float *data = buffers->getBuffer(i);

        if (!data) {
            continue;
        }

        memcpy(ev->data[i], data, buffers->getSize());
    }
}

void JSAudioNodeCustomBase::OnAssignInit(AudioNode *n, void *custom)
{
    JS_AUDIO_THREAD_PROLOGUE()

    JSAudioNodeCustomBase *node
        = reinterpret_cast<JSAudioNodeCustomBase *>(custom);

    node->initThreadedNode();
}

void JSAudioNodeCustomBase::OnSet(AudioNode *n, void *custom)
{
    JS_AUDIO_THREAD_PROLOGUE()

    JSTransferable *t           = reinterpret_cast<JSTransferable *>(custom);
    JSAudioNodeCustomBase *node = t->getPrivate<JSAudioNodeCustomBase>();

    node->initThreadedNode();

    JS::RootedValue val(cx, t->get());
    node->m_ThreadedNode->set(val);

    delete t;
}

void JSAudioNodeCustomBase::releaseFunction(TransferableFunction funID)
{
    delete m_TransferableFuncs[funID];
    m_TransferableFuncs[funID] = nullptr;
}

void JSAudioNodeCustomBase::ShutdownCallback(AudioNode *nnode, void *custom)
{
    JS_AUDIO_THREAD_PROLOGUE()

    JSAudioNodeCustomBase *node = static_cast<JSAudioNodeCustomBase *>(custom);

    delete node->m_ThreadedNode;

    for (int i = 0; i < END_FN; i++) {
        delete node->m_TransferableFuncs[i];
        node->m_TransferableFuncs[i] = nullptr;
    }

    NIDIUM_PTHREAD_SIGNAL(&node->m_ShutdownWait)
}

void JSAudioNodeCustomBase::initThreadedNode()
{
    if (m_ThreadedNode) {
        return;
    }

    m_ThreadedNode = new JSAudioNodeThreaded(this);
}


JSObject *JSAudioNodeCustomBase::getThreadedObject()
{
    return m_ThreadedNode->getJSObject();
}
// }}}

// {{{ JSAudioNodeBuffers
JSAudioNodeBuffers::JSAudioNodeBuffers(JSAudioContext *audioCtx,
                                       float **framesData,
                                       JSAudioNodeCustomBase *jsNode,
                                       AudioParameters *params)
{
    JSContext *cx   = audioCtx->m_JsTcx;
    AudioNode *node = jsNode->getNode();
    JS::RootedObject obj(cx, JSAudioNodeBuffers::CreateObject(cx, this));

    // Maximum number of buffers (max channels)
    m_Count = node->m_InCount > node->m_OutCount ? node->m_InCount
                                                 : node->m_OutCount;

    // Size of one buffer in bytes
    m_Size = params->m_BufferSize;

    JS::RootedObject frames(cx, JS_NewArrayObject(cx, 0));

    m_DataArray = frames;

    for (unsigned int i = 0; i < m_Count; i++) {
        // TODO : Avoid memcpy (custom allocator for AudioNode?)
        JS::RootedObject arrBuff(cx,
            JSUtils::NewArrayBufferWithCopiedContents(cx, m_Size, framesData[i]));

        JS::RootedObject arr(cx,
                             JS_NewFloat32ArrayWithBuffer(cx, arrBuff, 0, -1));

        JS_DefineElement(cx, frames, i, arr, JSPROP_ENUMERATE | JSPROP_PERMANENT);
    }

    JS::RootedObject global(cx, JS::CurrentGlobalOrNull(cx));
    JS::RootedValue vFrames(cx, JS::ObjectValue(*frames));
    JS::RootedValue vSize(cx, JS::DoubleValue(params->m_FramesPerBuffer));

    JS_DefineProperty(cx, obj, "data", vFrames,
                      JSPROP_PERMANENT | JSPROP_ENUMERATE);
    JS_DefineProperty(cx, obj, "size", vSize,
                      JSPROP_PERMANENT | JSPROP_ENUMERATE);
}

/*
    Returned value could be modified by the GC.
    Make sure we're not calling any GC'able JSAPI function before using the
    returned value
*/
float *JSAudioNodeBuffers::getBuffer(unsigned int idx)
{
    JSContext *cx = this->getJSContext();
    JS::RootedObject obj(cx, m_DataArray);
    JS::RootedValue val(cx);

    JS_GetElement(cx, obj, idx, &val);
    if (!val.isObject()) {
        return nullptr;
    }

    JS::RootedObject arr(cx, val.toObjectOrNull());
    if (!arr.get() || !JS_IsFloat32Array(arr)) {
        return nullptr;
    }

    bool shared;
    JS::AutoCheckCannotGC nogc;

    return JS_GetFloat32ArrayData(arr, &shared, nogc);
}
// }}}

// {{{ JSAudioNodeCustomSource
JSPropertySpec *JSAudioNodeCustomSource::ListProperties()
{
    static JSPropertySpec props[]
        = { CLASSMAPPER_PROP_GS(JSAudioNodeCustomSource, position), JS_PS_END };

    return props;
}
JSFunctionSpec *JSAudioNodeCustomSource::ListMethods()
{
    static JSFunctionSpec funcs[]
        = { CLASSMAPPER_FN(JSAudioNodeCustomSource, play, 0),
            CLASSMAPPER_FN(JSAudioNodeCustomSource, pause, 0),
            CLASSMAPPER_FN(JSAudioNodeCustomSource, stop, 0),

            CLASSMAPPER_FN(JSAudioNodeCustomSource, set, 1),
            CLASSMAPPER_FN(JSAudioNodeCustomSource, assignProcessor, 1),
            CLASSMAPPER_FN(JSAudioNodeCustomSource, assignInit, 1),
            CLASSMAPPER_FN(JSAudioNodeCustomSource, assignSetter, 1),
            CLASSMAPPER_FN(JSAudioNodeCustomSource, assignSeek, 1),
            JS_FS_END };

    return funcs;
}

void JSAudioNodeCustomSource::onMessage(
    const Core::SharedMessages::Message &msg)
{
    if (msg.event() == JSAudioNodeThreaded::THREADED_MESSAGE) {
        JSAudioNodeCustomBase::onMessage(msg);
    } else {
        JSAVSourceEventInterface::onMessage(msg);
    }
}

bool JSAudioNodeCustomSource::JSGetter_position(JSContext *cx,
                                                JS::MutableHandleValue vp)
{
    vp.setDouble(0);
    return true;
}

bool JSAudioNodeCustomSource::JSSetter_position(JSContext *cx,
                                                JS::MutableHandleValue vp)
{
    if (vp.isNumber()) {
        this->getNode<AudioSourceCustom>()->seek(vp.toNumber());
    }

    return true;
}

bool JSAudioNodeCustomSource::JS_play(JSContext *cx, JS::CallArgs &args)
{
    this->getNode<AudioSourceCustom>()->play();
    return true;
}

bool JSAudioNodeCustomSource::JS_pause(JSContext *cx, JS::CallArgs &args)
{
    this->getNode<AudioSourceCustom>()->pause();
    return true;
}

bool JSAudioNodeCustomSource::JS_stop(JSContext *cx, JS::CallArgs &args)
{
    this->getNode<AudioSourceCustom>()->stop();
    return true;
}

bool JSAudioNodeCustomSource::JS_assignSeek(JSContext *cx, JS::CallArgs &args)
{
    JSTransferableFunction *fun
        = this->getFunction(JSAudioNodeCustomBase::SEEK_FN);

    if (!fun->set(cx, args[0])) {
        JS_ReportErrorUTF8(cx, "First argument must be a function or null");
        return false;
    }

    AudioSourceCustom *node = this->getNode<AudioSourceCustom>();

    if (args[0].isNull()) {
        node->setSeek(nullptr, nullptr);
    } else {
        node->setSeek(JSAudioNodeCustomSource::OnSeek,
                      static_cast<void *>(this));
    }

    return true;
}

void JSAudioNodeCustomSource::OnSeek(AudioSourceCustom *node,
                                     double seekTime,
                                     void *custom)
{
    JS_AUDIO_THREAD_PROLOGUE()

    JSAudioNodeCustomSource *jnode
        = static_cast<JSAudioNodeCustomSource *>(custom);

    jnode->initThreadedNode();

    JSTransferableFunction *t
        = jnode->getFunction(JSAudioNodeCustomBase::SEEK_FN);

    JS::RootedObject obj(cx, jnode->getThreadedObject());
    JS::RootedValue rval(cx);

    JS::AutoValueArray<2> params(cx);
    params[0].setDouble(seekTime);
    params[1].setObjectOrNull(global);

    t->call(obj, params, &rval);
}
// }}}

// {{{ JSAudioNodeSource
JSFunctionSpec *JSAudioNodeCustom::ListMethods()
{
    static JSFunctionSpec funcs[]
        = { CLASSMAPPER_FN(JSAudioNodeCustom, set, 1),
            CLASSMAPPER_FN(JSAudioNodeCustom, assignProcessor, 1),
            CLASSMAPPER_FN(JSAudioNodeCustom, assignInit, 1),
            CLASSMAPPER_FN(JSAudioNodeCustom, assignSetter, 1), JS_FS_END };

    return funcs;
}
// }}}

// {{{ JSAudioNodeThreaded
JSFunctionSpec *JSAudioNodeThreaded::ListMethods()
{
    static JSFunctionSpec funcs[]
        = { CLASSMAPPER_FN(JSAudioNodeThreaded, set, 1),
            CLASSMAPPER_FN(JSAudioNodeThreaded, get, 0),
            CLASSMAPPER_FN(JSAudioNodeThreaded, send, 1), JS_FS_END };

    return funcs;
}

JSAudioNodeThreaded::JSAudioNodeThreaded(JSAudioNodeCustomBase *node)
    : m_Node(node)
{
    JS_AUDIO_THREAD_PROLOGUE()

    m_Cx = cx;

    JSAudioNodeThreaded::CreateObject(m_Cx, this);

    JS::RootedObject hashObj(m_Cx, JS_NewPlainObject(m_Cx));

    m_HashObj = hashObj;

    // Root |m_HashObj| on this object
    JS_SetReservedSlot(this->getJSObject(), 0, JS::ObjectValue(*hashObj));
    // Root this object on the global object of the context
    JS_SetReservedSlot(global, 0, JS::ObjectValue(*this->getJSObject()));

    JSTransferableFunction *initFn
        = m_Node->getFunction(JSAudioNodeCustomBase::INIT_FN);

    if (initFn && initFn->isSet()) {
        JS::AutoValueArray<1> params(m_Cx);
        JS::RootedObject thisObj(m_Cx, this->getJSObject());

        params[0].setObjectOrNull(global);

        JS::RootedValue rval(m_Cx);

        initFn->call(thisObj, params, &rval);

        m_Node->releaseFunction(JSAudioNodeCustomBase::INIT_FN);
    }
}

void JSAudioNodeThreaded::set(JS::HandleValue val)
{
    if (!val.isObject()) return;

    JS::RootedObject global(m_Cx, JSAudioContext::GetContext()->m_JsGlobalObj);
    JS::RootedObject thisObj(m_Cx, this->getJSObject());
    JS::RootedObject props(m_Cx, val.toObjectOrNull());

    JS::Rooted<JS::IdVector> ida(m_Cx, JS::IdVector(m_Cx));

    JS_Enumerate(m_Cx, props, &ida);

    for (size_t i = 0; i < ida.length(); i++) {
        // Retrieve the current key & value
        JS::RootedId id(m_Cx, ida[i]);
        JSAutoByteString key(m_Cx, JSID_TO_STRING(id));
        JS::RootedValue val(m_Cx);
        if (!JS_GetPropertyById(m_Cx, props, id, &val)) {
            break;
        }

        // Set the property
        JS::RootedObject hashObj(m_Cx, m_HashObj);
        JS_SetProperty(m_Cx, hashObj, key.ptr(), val);

        // Call the setter callback
        JSTransferableFunction *fun
            = m_Node->getFunction(JSAudioNodeCustomBase::SETTER_FN);
        if (fun->isSet()) {
            JS::RootedValue rval(m_Cx);
            JS::RootedString jskey(m_Cx, JS_NewStringCopyZ(m_Cx, key.ptr()));
            JS::AutoValueArray<3> params(m_Cx);

            params[0].setString(jskey);
            params[1].set(val);
            params[2].setObjectOrNull(global);

            fun->call(thisObj, params, &rval);
        }
    }
}

bool JSAudioNodeThreaded::JS_get(JSContext *cx, JS::CallArgs &args)
{
    if (!args[0].isString()) {
        JS_ReportErrorUTF8(cx, "First argument must be a string");
        return false;
    }

    JS::RootedObject obj(cx, m_HashObj.get());
    JSAutoByteString str(cx, args[0].toString());
    JS::RootedValue val(cx);

    JS_GetProperty(cx, obj, str.ptr(), &val);

    args.rval().set(val);

    return true;
}

bool JSAudioNodeThreaded::JS_set(JSContext *cx, JS::CallArgs &args)
{
    JS::RootedValue val(cx, setArgsToObject(cx, args));
    if (!val.isObject()) {
        return false;
    }

    this->set(val);

    return true;
}

bool JSAudioNodeThreaded::JS_send(JSContext *cx, JS::CallArgs &args)
{
    JSAudioContext *audioContext = JSAudioContext::GetContext();

    JSTransferable *t
        = new JSTransferable(cx, audioContext->getJSContext(),
                             audioContext->m_JsGlobalObj, args[0]);

    m_Node->postMessage(t, JSAudioNodeThreaded::THREADED_MESSAGE);

    return true;
}
// }}}

} // namespace Binding
} // namespace Nidium
