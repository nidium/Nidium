/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/

#include "Binding/JSAudioNode.h"

#include "Binding/JSEvents.h"
#include "Binding/JSVideo.h"

using namespace Nidium::AV;

namespace Nidium {
namespace Binding {

#define JS_PROPAGATE_ERROR(cx, ...)       \
    JS_ReportError(cx, __VA_ARGS__);      \
    if (!JS_ReportPendingException(cx)) { \
        JS_ClearPendingException(cx);     \
    }

#define GET_NODE(type, var)                                  \
    NIDIUM_JS_PROLOGUE_CLASS(JSAudioNode, &AudioNode_class); \
    var = static_cast<type *>(CppObj->m_Node);



typedef enum {
    NODE_CUSTOM_PROCESS_CALLBACK,
    NODE_CUSTOM_INIT_CALLBACK,
    NODE_CUSTOM_SETTER_CALLBACK,
    NODE_CUSTOM_SEEK_CALLBACK
} CustomNodeCallbacks;


/// {{{ Preamble
static JSClass AudioNodeEvent_class = { "AudioNodeEvent",
                                        JSCLASS_HAS_PRIVATE,
                                        JS_PropertyStub,
                                        JS_DeletePropertyStub,
                                        JS_PropertyStub,
                                        JS_StrictPropertyStub,
                                        JS_EnumerateStub,
                                        JS_ResolveStub,
                                        JS_ConvertStub,
                                        nullptr,
                                        nullptr,
                                        nullptr,
                                        nullptr,
                                        nullptr,
                                        JSCLASS_NO_INTERNAL_MEMBERS };

template <>
JSClass *JSExposer<JSAudioNode>::jsclass = &AudioNode_class;

static JSClass AudioNode_threaded_class = { "AudioNodeThreaded", JSCLASS_HAS_PRIVATE,
                                            JS_PropertyStub,
                                            JS_DeletePropertyStub,
                                            JS_PropertyStub,
                                            JS_StrictPropertyStub,
                                            JS_EnumerateStub,
                                            JS_ResolveStub,
                                            JS_ConvertStub,
                                            nullptr,
                                            nullptr,
                                            nullptr,
                                            nullptr,
                                            nullptr,
                                            JSCLASS_NO_INTERNAL_MEMBERS };

static bool nidium_AudioNode_constructor(JSContext *cx, unsigned argc, JS::Value *vp);
// }}}


JS::Value consumeSourceMessage(JSContext *cx,
                                      const SharedMessages::Message &msg)
{
    JS::RootedValue ev(cx);

    if (msg.event() == CUSTOM_SOURCE_SEND) {
        nidium_thread_msg *ptr
            = static_cast<struct nidium_thread_msg *>(msg.dataPtr());
        JS::RootedValue inval(cx, JSVAL_NULL);
        if (!JS_ReadStructuredClone(cx, ptr->data, ptr->nbytes,
                                    JS_STRUCTURED_CLONE_VERSION, &inval,
                                    nullptr, NULL)) {
            JS_PROPAGATE_ERROR(
                cx, "Failed to transfer custom node message to audio thread");
            return JS::UndefinedHandleValue;
        }

        JS::RootedObject evObj(cx, JSEvents::CreateEventObject(cx));
        JSObjectBuilder evBuilder(cx, evObj);

        evBuilder.set("data", inval);

        ev = evBuilder.jsval();

        delete ptr;
    } else {
        AVSourceEvent *cmsg
            = static_cast<struct AVSourceEvent *>(msg.dataPtr());

        if (cmsg->m_Ev == SOURCE_EVENT_ERROR) {
            int errorCode        = cmsg->m_Args[0].toInt();
            const char *errorStr = AV::AVErrorsStr[errorCode];
            JS::RootedObject evObj(
                cx, JSEvents::CreateErrorEventObject(cx, errorCode, errorStr));
            ev = OBJECT_TO_JSVAL(evObj);
        } else if (cmsg->m_Ev == SOURCE_EVENT_BUFFERING) {
            JS::RootedObject evObj(cx, JSEvents::CreateEventObject(cx));
            JSObjectBuilder evBuilder(cx, evObj);

            evBuilder.set("filesize", cmsg->m_Args[0].toInt());
            evBuilder.set("startByte", cmsg->m_Args[1].toInt());
            evBuilder.set("bufferedBytes", cmsg->m_Args[2].toInt());

            ev = evBuilder.jsval();
        }

        delete cmsg;
    }

    JS::RootedValue rval(cx);
    rval.set(ev);
    return rval;
}

JSAudioNode::JSAudioNode(JS::HandleObject obj,
                JSContext *cx,
                Audio::Node type,
                int in,
                int out,
                JSAudioContext *audio)
        : JSExposer<JSAudioNode>(obj, cx), m_nJs(NULL), m_AudioContext(audio),
          m_Node(NULL), m_NodeType(type), m_NodeObj(nullptr),
          m_HashObj(nullptr), m_ArrayContent(NULL), m_IsDestructing(false)
    {
        m_JSObject = NULL;

        try {
            m_Node = audio->m_Audio->createNode(type, in, out);
        } catch (AudioNodeException *e) {
            throw;
        }

        if (type == Audio::CUSTOM || type == Audio::CUSTOM_SOURCE) {
            NIDIUM_PTHREAD_VAR_INIT(&m_ShutdownWait);
        }

        this->add();

        for (int i = 0; i < END_FN; i++) {
            m_TransferableFuncs[i] = NULL;
        }
    }

JSAudioNode::JSAudioNode(JS::HandleObject obj,
                JSContext *cx,
                Audio::Node type,
                AudioNode *node,
                JSAudioContext *audio)
        : JSExposer<JSAudioNode>(obj, cx), m_nJs(NULL), m_AudioContext(audio),
          m_Node(node), m_NodeType(type), m_NodeObj(nullptr),
          m_HashObj(nullptr), m_ArrayContent(NULL), m_IsDestructing(false)
    {
        this->add();

        for (int i = 0; i < END_FN; i++) {
            m_TransferableFuncs[i] = NULL;
        }
    }


// {{{ Implementation
void JSAudioNode::add()
{
    JSAudioContext::Nodes *nodes = new JSAudioContext::Nodes(this, NULL, m_AudioContext->m_Nodes);

    if (m_AudioContext->m_Nodes != NULL) {
        m_AudioContext->m_Nodes->prev = nodes;
    }

    m_AudioContext->m_Nodes = nodes;
}

void nidium_audio_node_custom_set_internal(JSContext *cx,
                                           JSAudioNode *node,
                                           JS::HandleObject obj,
                                           const char *name,
                                           JS::HandleValue val)
{
    JS_SetProperty(cx, obj, name, val);

    JSTransferableFunction *setterFn;
    setterFn = node->m_TransferableFuncs[JSAudioNode::SETTER_FN];
    if (setterFn) {
        JS::RootedObject global(cx, JS::CurrentGlobalOrNull(cx));
        JS::RootedString jstr(cx, JS_NewStringCopyZ(cx, name));
        JS::AutoValueArray<3> params(cx);
        params[0].setString(jstr);
        params[1].set(val);
        params[2].setObjectOrNull(global);

        JS::RootedValue rval(cx);
        setterFn->call(*node->m_NodeObj, params, &rval);
    }
}

void JSAudioNode::SetPropCallback(AudioNode *node, void *custom)
{
    JSContext *tcx;
    JSAudioNode::Message *msg;

    msg = static_cast<struct JSAudioNode::Message *>(custom);
    tcx = msg->jsNode->m_AudioContext->m_JsTcx;

    JSAutoRequest ar(tcx);
    JSAutoCompartment ac(tcx, msg->jsNode->m_AudioContext->m_JsGlobalObj);

    JS::RootedValue data(tcx);
    if (!JS_ReadStructuredClone(tcx, msg->clone.datap, msg->clone.nbytes,
                                JS_STRUCTURED_CLONE_VERSION, &data, nullptr,
                                NULL)) {
        JS_PROPAGATE_ERROR(tcx, "Failed to read structured clone");

        JS_free(msg->jsNode->getJSContext(), msg->name);
        JS_ClearStructuredClone(msg->clone.datap, msg->clone.nbytes, nullptr,
                                NULL);
        delete msg;

        return;
    }
    if (msg->name == NULL) {
        JS::RootedObject props(tcx);
        JS_ValueToObject(tcx, data, &props);
        JS::AutoIdArray ida(tcx, JS_Enumerate(tcx, props));
        for (size_t i = 0; i < ida.length(); i++) {
            JS::RootedId id(tcx, ida[i]);
            JSAutoByteString name(tcx, JSID_TO_STRING(id));
            JS::RootedValue val(tcx);
            if (!JS_GetPropertyById(tcx, props, id, &val)) {
                break;
            }
            nidium_audio_node_custom_set_internal(
                tcx, msg->jsNode, *msg->jsNode->m_HashObj, name.ptr(), val);
        }
    } else {
        nidium_audio_node_custom_set_internal(
            tcx, msg->jsNode, *msg->jsNode->m_HashObj, msg->name, data);
    }

    if (msg->name != NULL) {
        JS_free(msg->jsNode->getJSContext(), msg->name);
    }

    JS_ClearStructuredClone(msg->clone.datap, msg->clone.nbytes, nullptr, NULL);

    delete msg;
}

void JSAudioNode::CustomCallback(const struct NodeEvent *ev)
{
    JSAudioNode *thiz;
    JSContext *tcx;
    JSTransferableFunction *processFn;
    unsigned long size;
    int count;

    thiz = static_cast<JSAudioNode *>(ev->custom);

    if (!thiz->m_AudioContext->m_JsTcx || !thiz->m_Cx || !thiz->getJSObject()
        || !thiz->m_Node) {
        return;
    }

    tcx = thiz->m_AudioContext->m_JsTcx;

    JSAutoRequest ar(tcx);
    JSAutoCompartment ac(tcx, thiz->m_AudioContext->m_JsGlobalObj);

    processFn = thiz->m_TransferableFuncs[JSAudioNode::PROCESS_FN];

    if (!processFn) {
        return;
    }

    count = thiz->m_Node->m_InCount > thiz->m_Node->m_OutCount
                ? thiz->m_Node->m_InCount
                : thiz->m_Node->m_OutCount;
    size = thiz->m_Node->m_Audio->m_OutputParameters->m_BufferSize / 2;

    JS::RootedObject obj(tcx, JS_NewObject(tcx, &AudioNodeEvent_class,
                                           JS::NullPtr(), JS::NullPtr()));
    JS::RootedObject frames(tcx, JS_NewArrayObject(tcx, 0));
    for (int i = 0; i < count; i++) {
        uint8_t *data;

        // TODO : Avoid memcpy (custom allocator for AudioNode?)
        JS::RootedObject arrBuff(tcx, JS_NewArrayBuffer(tcx, size));
        data = JS_GetArrayBufferData(arrBuff);
        memcpy(data, ev->data[i], size);
        JS::RootedObject arr(tcx,
                             JS_NewFloat32ArrayWithBuffer(tcx, arrBuff, 0, -1));
        if (arr.get()) {
            JS_DefineElement(tcx, frames, i, OBJECT_TO_JSVAL(arr), nullptr,
                             nullptr, JSPROP_ENUMERATE | JSPROP_PERMANENT);
        } else {
            JS_ReportOutOfMemory(tcx);
            return;
        }
    }

    JS::RootedObject global(tcx, JS::CurrentGlobalOrNull(tcx));
    JS::RootedValue vFrames(tcx, OBJECT_TO_JSVAL(frames));
    JS::RootedValue vSize(tcx, DOUBLE_TO_JSVAL(ev->size));

    JS_DefineProperty(tcx, obj, "data", vFrames,
                      JSPROP_PERMANENT | JSPROP_ENUMERATE);
    JS_DefineProperty(tcx, obj, "size", vSize,
                      JSPROP_PERMANENT | JSPROP_ENUMERATE);

    JS::AutoValueArray<2> params(tcx);
    params[0].setObjectOrNull(obj);
    params[1].setObjectOrNull(global);

    JS::RootedValue rval(tcx);
    processFn->call(*thiz->m_NodeObj, params, &rval);

    for (int i = 0; i < count; i++) {
        JS::RootedValue val(tcx);
        JS_GetElement(tcx, frames, i, &val);
        if (!val.isObject()) {
            continue;
        }

        JS::RootedObject arr(tcx, val.toObjectOrNull());
        if (!arr.get() || !JS_IsFloat32Array(arr)
            || JS_GetTypedArrayLength(arr) != ev->size) {
            continue;
        }

        memcpy(ev->data[i], JS_GetFloat32ArrayData(arr), size);
    }
}

void JSAudioNode::onMessage(const SharedMessages::Message &msg)
{
    if (m_IsDestructing) return;

    const char *evName = JSAVEventRead(msg.event());
    if (!evName) {
        return;
    }

    JS::RootedValue ev(m_Cx, consumeSourceMessage(m_Cx, msg));
    if (!ev.isNull()) {
        //this->fireJSEvent(evName, &ev);
    }
}

void JSAudioNode::onEvent(const struct AVSourceEvent *cev)
{
    JSAudioNode *jnode = static_cast<JSAudioNode *>(cev->m_Custom);
    jnode->postMessage((void *)cev, cev->m_Ev);
}


void JSAudioNode::ShutdownCallback(AudioNode *nnode, void *custom)
{
    JSAudioNode *node = static_cast<JSAudioNode *>(custom);
    JSAutoRequest ar(node->m_AudioContext->m_JsTcx);

    delete node->m_NodeObj;
    delete node->m_HashObj;

    // JSTransferableFunction need to be destroyed on the JS thread
    // since they belong to it
    for (int i = 0; i < JSAudioNode::END_FN; i++) {
        delete node->m_TransferableFuncs[i];
        node->m_TransferableFuncs[i] = NULL;
    }

    NIDIUM_PTHREAD_SIGNAL(&node->m_ShutdownWait)
}

void JSAudioNode::DeleteTransferableFunc(AudioNode *node, void *custom)
{
    JSTransferableFunction *fun = static_cast<JSTransferableFunction *>(custom);
    delete fun;
}

void JSAudioNode::InitCustomObject(AudioNode *node, void *custom)
{
    JSAudioNode *jnode = static_cast<JSAudioNode *>(custom);
    JSContext *tcx     = jnode->m_AudioContext->m_JsTcx;

    if (jnode->m_HashObj || jnode->m_NodeObj) {
        return;
    }

    JSAutoRequest ar(tcx);
    JS::RootedObject global(tcx, jnode->m_AudioContext->m_JsGlobalObj);
    JSAutoCompartment ac(tcx, global);

    jnode->m_HashObj = new JS::PersistentRootedObject(
        tcx, JS_NewObject(tcx, nullptr, JS::NullPtr(), JS::NullPtr()));
    if (!jnode->m_HashObj) {
        JS_PROPAGATE_ERROR(tcx, "Failed to create hash object for custom node");
        return;
    }

    jnode->m_NodeObj = new JS::PersistentRootedObject(
        tcx, JS_NewObject(tcx, &AudioNode_threaded_class, JS::NullPtr(),
                          JS::NullPtr()));
    if (!jnode->m_NodeObj) {
        JS_PROPAGATE_ERROR(tcx, "Failed to create node object for custom node");
        return;
    }

    JS_DefineFunctions(tcx, *jnode->m_NodeObj, AudioNodeCustom_threaded_funcs);
    JS_SetPrivate(*jnode->m_NodeObj, static_cast<void *>(jnode));

    JSTransferableFunction *initFn
        = jnode->m_TransferableFuncs[JSAudioNode::INIT_FN];

    if (initFn) {
        JS::AutoValueArray<1> params(tcx);
        JS::RootedValue glVal(tcx, OBJECT_TO_JSVAL(global));
        params[0].set(glVal);

        JS::RootedValue rval(tcx);
        initFn->call(*jnode->m_NodeObj, params, &rval);

        jnode->m_TransferableFuncs[JSAudioNode::INIT_FN] = NULL;

        delete initFn;
    }
}

JSAudioNode::~JSAudioNode()
{
    JSAudioContext::Nodes *nodes = m_AudioContext->m_Nodes;
    JSAutoRequest ar(m_Cx);

    m_IsDestructing = true;

    // Block Audio threads execution.
    // While the node is destructed we don't want any thread
    // to call some method on a node that is being destroyed
    m_AudioContext->m_Audio->lockQueue();
    m_AudioContext->m_Audio->lockSources();

    // Wakeup audio thread. This will flush all pending messages.
    // That way, we are sure nothing will need to be processed
    // later for this node.
    m_AudioContext->m_Audio->wakeup();

    if (m_NodeType == Audio::SOURCE) {
        // Only source from Video has reserved slot
        JS::RootedValue source(m_Cx, JS_GetReservedSlot(m_JSObject, 0));
        JS::RootedObject obj(m_Cx, source.toObjectOrNull());
        if (obj.get()) {
            // If it exist, we must inform the video
            // that audio node no longer exist
            JSVideo *video = (JSVideo *)JS_GetPrivate(obj);
            if (video != NULL) {
                JS_SetReservedSlot(m_JSObject, 0, JSVAL_NULL);
                video->stopAudio();
            }
        }
    }

    // Remove JS node from nodes linked list
    while (nodes != NULL) {
        if (nodes->curr == this) {
            if (nodes->prev != NULL) {
                nodes->prev->next = nodes->next;
            } else {
                m_AudioContext->m_Nodes = nodes->next;
            }

            if (nodes->next != NULL) {
                nodes->next->prev = nodes->prev;
            }

            delete nodes;

            break;
        }
        nodes = nodes->next;
    }


    // Custom nodes and sources must release all JS object on the JS thread
    if (m_Node != NULL && m_AudioContext->m_JsTcx != NULL
        && (m_NodeType == Audio::CUSTOM
            || m_NodeType == Audio::CUSTOM_SOURCE)) {

        m_Node->callback(JSAudioNode::ShutdownCallback, this, true);

        NIDIUM_PTHREAD_WAIT(&m_ShutdownWait);
    }

    if (m_ArrayContent != NULL) {
        free(m_ArrayContent);
    }

    if (m_JSObject != NULL) {
        JS_SetPrivate(m_JSObject, nullptr);
        m_JSObject = nullptr;
    }

    delete m_Node;

    m_AudioContext->m_Audio->unlockQueue();
    m_AudioContext->m_Audio->unlockSources();
}

void nidium_audionode_set_internal(JSContext *cx,
                                          AudioNode *node,
                                          const char *prop,
                                          JS::HandleValue val)
{
    ArgType type;
    void *value;
    int intVal       = 0;
    double doubleVal = 0;
    unsigned long size;

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
        JS_ReportError(cx, "Unsuported value\n");
        return;
    }

    if (!node->set(prop, type, value, size)) {
        JS_ReportError(cx, "Unknown argument name %s\n", prop);
    }
}
bool nidium_audionode_set(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JSAudioNode *jnode;

    NIDIUM_JS_PROLOGUE_CLASS(JSAudioNode, &AudioNode_class);
    jnode = CppObj;

    AudioNode *node = jnode->m_Node;

    NIDIUM_JS_CHECK_ARGS("set", 1)

    if (args[0].isObject()) {
        JS::RootedObject props(cx, args[0].toObjectOrNull());

        if (!props) {
            JS_ReportError(cx, "Invalid argument");
            return false;
        }

        JS::AutoIdArray ida(cx, JS_Enumerate(cx, props));

        for (size_t i = 0; i < ida.length(); i++) {
            JS::RootedId id(cx, ida[i]);
            JSAutoByteString cname(cx, JSID_TO_STRING(id));
            JS::RootedValue val(cx);
            if (!JS_GetPropertyById(cx, props, id, &val)) {
                break;
            }
            nidium_audionode_set_internal(cx, node, cname.ptr(), val);
        }
    } else {
        NIDIUM_JS_CHECK_ARGS("set", 2)

        JS::RootedString name(cx);
        if (!JS_ConvertArguments(cx, args, "S", name.address())) {
            return false;
        }

        JSAutoByteString cname(cx, name);
        nidium_audionode_set_internal(cx, node, cname.ptr(), args[1]);
    }

    return true;
}

bool nidium_audionode_get(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JS_ReportError(cx, "Not implemented");
    return true;
}

bool nidium_audionode_custom_set(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JSAudioNode::Message *msg;
    AudioNodeCustom *node;
    JSAudioNode *jnode;

    NIDIUM_JS_PROLOGUE_CLASS(JSAudioNode, &AudioNode_class);
    jnode = CppObj;

    JS::RootedString name(cx);
    JS::RootedValue val(cx);

    if (argc == 1) {
        if (!args[0].isPrimitive()) {
            val = args[0];
        } else {
            JS_ReportError(cx, "Invalid argument");
            return false;
        }
    } else if (argc == 2) {
        if (!args[0].isString()) {
            JS_ReportError(cx, "First argument must be a string");
            return false;
        }

        if (!JS_ConvertArguments(cx, args, "S", name.address())) {
            return false;
        }

        val = args[1];
    } else {
        JS_ReportError(cx, "Invalid arguments");
        return false;
    }

    node = static_cast<AudioNodeCustom *>(jnode->m_Node);

    msg = new JSAudioNode::Message();

    if (name && JS_GetStringLength(name) > 0) {
        msg->name = JS_EncodeString(cx, name);
    }
    msg->jsNode = jnode;

    if (!JS_WriteStructuredClone(cx, val, &msg->clone.datap, &msg->clone.nbytes,
                                 nullptr, nullptr, JS::NullHandleValue)) {
        JS_ReportError(cx, "Failed to write structured clone");

        JS_free(cx, msg->name);
        delete msg;

        return false;
    }

    if (!jnode->m_NodeObj) {
        node->callback(JSAudioNode::InitCustomObject,
                       static_cast<void *>(jnode));
    }

    node->callback(JSAudioNode::SetPropCallback, msg, true);

    return true;
}

bool nidium_audionode_custom_assign(JSContext *cx,
                                           JSAudioNode *jnode,
                                           CustomNodeCallbacks callbackID,
                                           JS::HandleValue callback)
{
    JSTransferableFunction *fun;
    JSAudioNode::TransferableFunction funID;
    AudioNode *node = static_cast<AudioNode *>(jnode->m_Node);

    switch (callbackID) {
        case NODE_CUSTOM_PROCESS_CALLBACK:
            funID = JSAudioNode::PROCESS_FN;
            break;
        case NODE_CUSTOM_SETTER_CALLBACK:
            funID = JSAudioNode::SETTER_FN;
            break;
        case NODE_CUSTOM_INIT_CALLBACK:
            funID = JSAudioNode::INIT_FN;
            break;
        case NODE_CUSTOM_SEEK_CALLBACK:
            funID = JSAudioNode::SEEK_FN;
            break;
        default:
            return true;
            break;
    }

    if (jnode->m_TransferableFuncs[funID] != NULL) {
        node->callback(JSAudioNode::DeleteTransferableFunc,
                       static_cast<void *>(jnode->m_TransferableFuncs[funID]));
        jnode->m_TransferableFuncs[funID] = NULL;
    }

    if (!callback.isObject()
        || !JS_ObjectIsCallable(cx, &callback.toObject())) {
        return true;
    }

    fun = new JSTransferableFunction(jnode->m_AudioContext->m_JsTcx);

    if (!fun->prepare(cx, callback)) {
        JS_ReportError(cx, "Failed to read custom node callback function\n");
        delete fun;
        return false;
    }

    jnode->m_TransferableFuncs[funID] = fun;

    if (!jnode->m_NodeObj) {
        node->callback(JSAudioNode::InitCustomObject,
                       static_cast<void *>(jnode));
    }

    return true;
}

bool nidium_audionode_custom_assign_processor(JSContext *cx,
                                                     unsigned argc,
                                                     JS::Value *vp)
{
    NIDIUM_JS_PROLOGUE_CLASS(JSAudioNode, &AudioNode_class);
    NIDIUM_JS_CHECK_ARGS("assignProcessor", 1);

    AudioNodeCustom *node = static_cast<AudioNodeCustom *>(CppObj->m_Node);

    if (!nidium_audionode_custom_assign(
            cx, CppObj, NODE_CUSTOM_PROCESS_CALLBACK, args[0])) {
        return false;
    }

    node->setCallback(JSAudioNode::CustomCallback, static_cast<void *>(CppObj));

    return true;
}

bool nidium_audionode_custom_assign_init(JSContext *cx, unsigned argc, JS::Value *vp)
{
    NIDIUM_JS_PROLOGUE_CLASS(JSAudioNode, &AudioNode_class);
    NIDIUM_JS_CHECK_ARGS("assignInit", 1);

    return nidium_audionode_custom_assign(cx, CppObj, NODE_CUSTOM_INIT_CALLBACK,
                                          args[0]);
}

bool nidium_audionode_custom_assign_setter(JSContext *cx,
                                                  unsigned argc,
                                                  JS::Value *vp)
{
    NIDIUM_JS_PROLOGUE_CLASS(JSAudioNode, &AudioNode_class);
    NIDIUM_JS_CHECK_ARGS("assignSetter", 1);

    return nidium_audionode_custom_assign(cx, CppObj,
                                          NODE_CUSTOM_SETTER_CALLBACK, args[0]);
}

bool nidium_audionode_custom_assign_seek(JSContext *cx, unsigned argc, JS::Value *vp)
{
    NIDIUM_JS_PROLOGUE_CLASS(JSAudioNode, &AudioNode_class);
    NIDIUM_JS_CHECK_ARGS("assignSeek", 1);

    AudioSourceCustom *node = static_cast<AudioSourceCustom *>(CppObj->m_Node);

    if (!nidium_audionode_custom_assign(cx, CppObj, NODE_CUSTOM_SEEK_CALLBACK,
                                        args[0])) {
        return false;
    }

    node->setSeek(JSAudioNode::SeekCallback, CppObj);

    return true;
}

bool nidium_audionode_custom_source_position_setter(JSContext *cx,
                                                           unsigned argc,
                                                           JS::Value *vp)
{
    NIDIUM_JS_PROLOGUE_CLASS(JSAudioNode, &AudioNode_class);

    AudioSourceCustom *source
        = static_cast<AudioSourceCustom *>(CppObj->m_Node);

    if (args[0].isNumber()) {
        source->seek(args[0].toNumber());
    }

    return true;
}

bool nidium_audionode_custom_source_position_getter(JSContext *cx,
                                                           unsigned argc,
                                                           JS::Value *vp)
{
    NIDIUM_JS_PROLOGUE_CLASS(JSAudioNode, &AudioNode_class);

    args.rval().setDouble(0.0);
    return true;
}

#if 0
bool nidium_audionode_custom_get(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JSAudioNode *jsNode;

    NUI_LOG("hello get\n");
    NUI_LOG("get node\n");

    jsNode = GET_NODE(JS_THIS_OBJECT(cx, vp));

    NUI_LOG("convert\n");
    JS::RootedString name(cx);
    if (!JS_ConvertArguments(cx, m_Args, "S", name, address())) {
        return true;
    }

    NUI_LOG("str\n");
    JSAutoByteString str(cx, name);
    JS_SetRuntimeThread(JS_GetRuntime(cx));

    NUI_LOG("get\n");
    JS::RootedObject m_HashObj(cx, jnode->m_HashObj);
    JS::RootedValuel val(cx);
    JS_GetProperty(jsNode->m_AudioContext->m_JsTcx, m_HashObj, str.ptr(), val.address());

    NUI_LOG("return\n");
    JS_ClearRuntimeThread(JS_GetRuntime(cx));

    m_Args.rval().set(val);

    return true;
}
#endif

bool nidium_audionode_custom_threaded_set(JSContext *cx,
                                                 unsigned argc,
                                                 JS::Value *vp)
{
    JSAudioNode *jnode;
    JSTransferableFunction *fn;

    NIDIUM_JS_PROLOGUE_CLASS(JSAudioNode, &AudioNode_threaded_class);
    jnode = CppObj;

    if (argc != 2) {
        JS_ReportError(cx, "set() require two arguments\n");
        return false;
    }

    JS::RootedString name(cx);
    if (!JS_ConvertArguments(cx, args, "S", name.address())) {
        return false;
    }

    fn = jnode->m_TransferableFuncs[JSAudioNode::SETTER_FN];
    JS::RootedValue val(cx, args[1]);
    JSAutoByteString str(cx, name);
    JS_SetProperty(cx, *jnode->m_HashObj, str.ptr(), val);

    if (fn) {
        JS::AutoValueArray<3> params(cx);
        JS::RootedObject global(cx, JS::CurrentGlobalOrNull(cx));

        params[0].setString(name);
        params[1].set(val);
        params[2].setObjectOrNull(global);

        JS::RootedValue rval(cx);
        fn->call(*jnode->m_NodeObj, params, &rval);
    }

    return true;
}

bool nidium_audionode_custom_threaded_get(JSContext *cx,
                                                 unsigned argc,
                                                 JS::Value *vp)
{
    JSAudioNode *jnode;

    NIDIUM_JS_PROLOGUE_CLASS(JSAudioNode, &AudioNode_threaded_class);
    jnode = CppObj;

    if (!jnode->m_HashObj) {
        return false;
    }

    JS::RootedString name(cx);
    if (!JS_ConvertArguments(cx, args, "S", name.address())) {
        return false;
    }

    JSAutoByteString str(cx, name);
    JS::RootedValue val(cx);

    JS_GetProperty(jnode->m_AudioContext->m_JsTcx, *jnode->m_HashObj, str.ptr(), &val);

    args.rval().set(val);

    return true;
}

bool nidium_audionode_custom_threaded_send(JSContext *cx,
                                                  unsigned argc,
                                                  JS::Value *vp)
{
    uint64_t *datap;
    size_t nbytes;
    JSAudioNode *jnode;

    NIDIUM_JS_PROLOGUE_CLASS(JSAudioNode, &AudioNode_threaded_class);
    NIDIUM_JS_CHECK_ARGS("send", 1);

    jnode = CppObj;

    struct nidium_thread_msg *msg;
    if (!JS_WriteStructuredClone(cx, args[0], &datap, &nbytes, nullptr, nullptr,
                                 JS::NullHandleValue)) {
        JS_ReportError(cx, "Failed to write structured clone");
        return false;
    }

    msg = new struct nidium_thread_msg;

    msg->data   = datap;
    msg->nbytes = nbytes;
    msg->callee = jnode->getJSObject();

    jnode->postMessage(msg, CUSTOM_SOURCE_SEND);

    return true;
}

bool nidium_audionode_source_open(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JSAudioNode *jnode;

    NIDIUM_JS_PROLOGUE_CLASS(JSAudioNode, &AudioNode_class);
    jnode = CppObj;

    AudioSource *source = (AudioSource *)jnode->m_Node;

    JS::RootedValue src(cx, args[0]);

    int ret = -1;

    if (src.isString()) {
        JSAutoByteString csrc(cx, src.toString());
        ret = source->open(csrc.ptr());
    } else if (src.isObject()) {
        JS::RootedObject arrayBuff(cx, src.toObjectOrNull());
        if (!JS_IsArrayBufferObject(arrayBuff)) {
            JS_ReportError(cx, "Data is not an ArrayBuffer\n");
            return false;
        }
        int length            = JS_GetArrayBufferByteLength(arrayBuff);
        jnode->m_ArrayContent = JS_StealArrayBufferContents(cx, arrayBuff);
        ret                   = source->open(jnode->m_ArrayContent, length);
    } else {
        JS_ReportError(cx, "Invalid argument", ret);
        return false;
    }

    if (ret < 0) {
        JS_ReportError(cx, "Failed to open stream %d\n", ret);
        return false;
    }

    return true;
}
bool nidium_audionode_source_play(JSContext *cx, unsigned argc, JS::Value *vp)
{
    AudioSource *source;

    GET_NODE(AudioSource, source);

    source->play();

    // play() may call the JS "onready" callback in a synchronous way
    // thus, if an exception happen in the callback, we should return false
    return !JS_IsExceptionPending(cx);
}

bool nidium_audionode_source_pause(JSContext *cx, unsigned argc, JS::Value *vp)
{
    AudioSource *source;

    GET_NODE(AudioSource, source);

    source->pause();

    return !JS_IsExceptionPending(cx);
}

bool nidium_audionode_source_stop(JSContext *cx, unsigned argc, JS::Value *vp)
{
    AudioSource *source;

    GET_NODE(AudioSource, source);

    source->stop();

    return !JS_IsExceptionPending(cx);
}

bool nidium_audionode_source_close(JSContext *cx, unsigned argc, JS::Value *vp)
{
    AudioSource *source;

    GET_NODE(AudioSource, source);

    source->close();

    return true;
}

bool nidium_audionode_source_prop_getter(JSContext *cx, JS::HandleObject obj,
                                                uint8_t id,
                                                JS::MutableHandleValue vp)
{
    JSAudioNode *jnode = (JSAudioNode *)JS_GetPrivate(obj);

    CHECK_INVALID_CTX(jnode);

    AudioSource *source = static_cast<AudioSource *>(jnode->m_Node);

    return JSAVSource::PropGetter(source, cx, id, vp);
}

bool nidium_audionode_source_prop_setter(JSContext *cx, JS::HandleObject obj,
                                                uint8_t id,
                                                bool strict,
                                                JS::MutableHandleValue vp)
{
    JSAudioNode *jnode = (JSAudioNode *)JS_GetPrivate(obj);

    CHECK_INVALID_CTX(jnode);

    AudioSource *source = static_cast<AudioSource *>(jnode->m_Node);

    return JSAVSource::PropSetter(source, id, vp);
}

bool nidium_audionode_custom_source_play(JSContext *cx, unsigned argc, JS::Value *vp)
{
    AudioSourceCustom *source;

    GET_NODE(AudioSourceCustom, source);

    source->play();

    // play() may call the JS "onready" callback in a synchronous way
    // thus, if an exception happen in the callback, we should return false
    return !JS_IsExceptionPending(cx);
}

bool nidium_audionode_custom_source_pause(JSContext *cx, unsigned argc,
                                                 JS::Value *vp)
{
    AudioSourceCustom *source;

    GET_NODE(AudioSourceCustom, source);

    source->pause();

    return !JS_IsExceptionPending(cx);
}

bool nidium_audionode_custom_source_stop(JSContext *cx, unsigned argc, JS::Value *vp)
{
    AudioSourceCustom *source;

    GET_NODE(AudioSourceCustom, source);

    source->stop();

    return !JS_IsExceptionPending(cx);
}

void AudioNode_Finalize(JSFreeOp *fop, JSObject *obj)
{
    JSAudioNode *node = (JSAudioNode *)JS_GetPrivate(obj);
    if (node != NULL) {
        JS_SetPrivate(obj, nullptr);
        delete node;
    }
}

static bool
nidium_AudioNode_constructor(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JS_ReportError(cx, "Illegal constructor");
    return false;
}

bool JSAudioNode::PropSetter(JSAudioNode *jnode,
                             JSContext *cx,
                             uint8_t id,
                             JS::MutableHandleValue vp)
{
    AudioSourceCustom *source = static_cast<AudioSourceCustom *>(jnode->m_Node);

    switch (id) {
        case SOURCE_PROP_POSITION:
            if (vp.isNumber()) {
                source->seek(vp.toNumber());
            }
            break;
        default:
            break;
    }

    return true;
}

void JSAudioNode::SeekCallback(AudioSourceCustom *node,
                               double seekTime,
                               void *custom)
{
    JSAudioNode *jnode  = static_cast<JSAudioNode *>(custom);
    JSContext *threadCx = jnode->m_AudioContext->m_JsTcx;

    JSAutoRequest ar(threadCx);
    JSAutoCompartment ac(threadCx, jnode->m_AudioContext->m_JsGlobalObj);

    JSTransferableFunction *fn
        = jnode->m_TransferableFuncs[JSAudioNode::SEEK_FN];
    if (!fn) return;

    JS::AutoValueArray<2> params(jnode->m_AudioContext->m_JsTcx);
    JS::RootedObject global(jnode->m_AudioContext->m_JsTcx,
                            JS::CurrentGlobalOrNull(jnode->m_AudioContext->m_JsTcx));
    params[0].setDouble(seekTime);
    params[1].setObjectOrNull(global);

    JS::RootedValue rval(jnode->m_AudioContext->m_JsTcx);
    fn->call(*jnode->m_NodeObj, params, &rval);
}


// }}}

// {{{ Registration
void JSAudioNode::RegisterObject(JSContext *cx)
{
    JS::RootedObject global(cx, JS::CurrentGlobalOrNull(cx));
    JS::RootedObject obj(
        cx, JS_InitClass(cx, global, JS::NullPtr(), &AudioNode_class,
                         nidium_AudioNode_constructor, 0, AudioNode_props,
                         AudioNode_funcs, nullptr, nullptr));
}
// }}}



} // namespace Binding
} // namespace Nidium


