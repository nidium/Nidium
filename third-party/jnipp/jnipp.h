//
// jnipp
// Copyright 2015 Moritz Moeller
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#ifndef _JNIPP_H_INCLUDED
#define _JNIPP_H_INCLUDED

#include <jni.h>
#include "Macros.h"

#include <string>
#include <stdexcept>
// #include <initializer_list>

#include <assert.h>

#if defined(JNIPP_THREAD_LOCAL_PTHREAD)
    #include <pthread.h>
#endif

#if defined(JNIPP_USE_BOOST)
    #include <boost/type_traits.hpp>
    #define JNIPP_ENABLE_IF_C boost::enable_if_c
    #define JNIPP_IS_BASE_OF boost::is_base_of
#elif defined(JNIPP_USE_TYPE_TRAITS)
    #include <type_traits>
    #define JNIPP_ENABLE_IF_C std::enable_if
    #define JNIPP_IS_BASE_OF std::is_base_of
#endif

#if !defined(JNIPP_THREAD_LOCAL)
    #if defined(__GNUC__)
        #define JNIPP_THREAD_LOCAL __thread
    #elif defined(_MSC_VER)
        #define JNIPP_THREAD_LOCAL __declspec(thread)
    #else
        #define JNIPP_THREAD_LOCAL thread_local
    #endif
#endif

#if !defined(JNIPP_ASSERT)
    #if defined(__EXCEPTIONS) && false
        #define JNIPP_ASSERT(condition, message) if (!(condition)) throw std::logic_error(message "(" #condition ")")
    #elif defined(ANDROID)
        #include <android/log.h>
        #define JNIPP_ASSERT(condition, message) if (!(condition)) __android_log_print(ANDROID_LOG_VERBOSE, "LOG", message);
    #else
        #define JNIPP_ASSERT(condition, message) assert(condition && message)
    #endif
#endif

namespace jnipp {

#if defined(ANDROID)
#include <android/log.h>
#define JNIPP_LOG(...) __android_log_print(ANDROID_LOG_VERBOSE, "LOG", __VA_ARGS__);
// #define JNIPP_RLOG(...) JNIPP_LOG(__VA_ARGS__)
#endif

#ifndef JNIPP_RLOG
#define JNIPP_RLOG(...)
#endif

////////////////////////////////////////////////////////////////////////////////

#define JNIPP_M_FOR_ALL_TYPES \
    M(jboolean, Boolean) \
    M(jchar, Char) \
    M(jbyte, Byte) \
    M(jshort, Short) \
    M(jint, Int) \
    M(jlong, Long) \
    M(jfloat, Float) \
    M(jdouble, Double)

template <typename T> class LocalRef;
template <typename T> class GlobalRef;
template <typename T> class WeakRef;
template <typename T, typename A1=void, typename A2=void> class Ref;

template <typename R, typename... A> class Method;


class String;
class Class;
class Object;
template <typename T> class Array;

////////////////////////////////////////////////////////////////////////////////

/**
 * provide JNIEnv* pointer. at your entry point you need to put a local Env::Scope scope(your_jni_env);
 */
class Env
{
protected:
#if defined(JNIPP_THREAD_LOCAL_PTHREAD)
    static pthread_key_t _curkey() {
        static pthread_key_t key;
        static bool inited = false;
        if (!inited) {
            inited = true;
            pthread_key_create(&key, NULL);
        }
        return key;
    }
    static JNIEnv* _getcur() {
        return (JNIEnv*)pthread_getspecific(_curkey());
    }
    static void _setcur(JNIEnv* val) {
        pthread_setspecific(_curkey(), (void*)val);
    }
#else
    static JNIEnv** _cur() {
        static JNIPP_THREAD_LOCAL JNIEnv* value = nullptr;
        return &value;
    }
    static JNIEnv* _getcur() {
        return *(_cur());
    }
    static void _setcur(JNIEnv* val) {
        *(_cur()) = val;
    }
#endif
public:
    class Scope {
        private:
            JNIEnv* _prev;
        public:
        Scope(JNIEnv* env) {
            _prev = _getcur();
            _setcur(env);
        }
        Scope(JavaVM* vm) {
            JNIEnv* env = nullptr;
            // @TODO: there are different definitions here :/
            // jint res = vm->AttachCurrentThread((void**)&env, NULL);
            jint res = reinterpret_cast<jint(*)(JavaVM*, void**, void*)>(vm->functions->AttachCurrentThread)(vm, (void**)&env, NULL);
            assert(res == JNI_OK);
            _prev = _getcur();
            _setcur(env);
        }
        ~Scope() {
            _setcur(_prev);
        }
    };
public:
    static JNIEnv* peek() {
        return _getcur();
    }
    static JNIEnv* get() {
        JNIEnv* res = peek();
        JNIPP_ASSERT(res, "EnvScope: no environment set");
        return res;
    }
    static void pushLocalFrame(jint capacity=16) {
        JNIPP_RLOG("Env::pushLocalFrame");
        jint res = get()->PushLocalFrame(capacity);
        assert(res == 0);
    }
    static void popLocalFrame() {
        JNIPP_RLOG("Env::popLocalFrame");
        get()->PopLocalFrame(nullptr);
    }
    static jobject popLocalFrame(jobject result) {
        JNIPP_RLOG("Env::popLocalFrame");
        return get()->PopLocalFrame(result);
    }
    static void ensureLocalCapacity(jint capacity) {
        jint res = get()->EnsureLocalCapacity(capacity);
        assert(res == 0);
    }
    static void throwException(Ref<Object> exception);
    static void throwException(Ref<Class> cls, Ref<String> message);
    static void throwException(const char* cls, const char* message);
    static bool hasException() {
        return get()->ExceptionCheck();
    }
    static LocalRef<Object> getException();
    static JavaVM* getVM() {
        JavaVM* result = nullptr;
        jint res = get()->GetJavaVM(&result);
        assert(res == 0);
        return result;
    }
};

////////////////////////////////////////////////////////////////////////////////

class LocalFrame
{
private:
    bool done = false;
public:
    LocalFrame(jint capacity=16) {
        Env::pushLocalFrame(capacity);
    }
    ~LocalFrame() {
        if (!done) {
            Env::popLocalFrame();
            done = true;
        }
    }
    jobject escape(jobject result) {
        assert(!done);
        done = true;
        return Env::popLocalFrame(result);
    }
};

////////////////////////////////////////////////////////////////////////////////

/**
 * monitor / lock object
 * can be passed/returned by rvalue
 * automatically unlocks on destruction
 */
class Monitor {
protected:
    jobject _object;
public:
    Monitor(jobject object) : _object(object) {
        Env::get()->MonitorEnter(_object);
    }
    Monitor(const Monitor& other) = delete;
    Monitor(Monitor&& other) : _object(other._object) {
        other._object = nullptr;
    }
    ~Monitor() {
        if (_object) {
            Env::get()->MonitorExit(_object);
            _object = nullptr;
        }
    }
};

////////////////////////////////////////////////////////////////////////////////

/**
 * base object class implementation
 */
class Object {
protected:
    jobject _value;
    JNIEnv* env() const {
        return Env::get();
    }
    void __clear() { // @TODO: see if we can get rid of this.
        _value = nullptr;
    }
    template<typename A> friend class LocalRef;
    template<typename A> friend class GlobalRef;
    template<typename A> friend class WeakRef;
    template<typename A, typename A1, typename A2> friend class Ref;
public:
    Object(jobject value) : _value(value) {
    }
    operator jobject() const {
        return _value;
    }

    LocalRef<String> toString() const;
    LocalRef<Class> getClass() const;
    jboolean isInstanceOf(Ref<Class> cls) const;
    Monitor lock() const;

    static Ref<Class> clazz();
};

/**
 * a java string
*/
class String : public Object
{
public:
    using Object::Object;
    typedef std::basic_string<jchar> _jstring;

    static LocalRef<String> create(const char* value);
    static LocalRef<String> create(const std::string value);
    static LocalRef<String> create(const jchar* value);
    static LocalRef<String> create(const jchar* value, size_t length);
    static LocalRef<String> create(const _jstring value);

    operator jstring() const {
        return (jstring)(jobject)*this;
    }

    jsize length() const {
        return env()->GetStringLength((jstring)*this);
    }
    char *str() const {
        if (Env::get()->IsSameObject((jobject)*this, NULL)) {
            // LOG("WARNING: String::std_str() is null!");
            return strdup("");
        }
        const char* data = env()->GetStringUTFChars((jstring)*this, nullptr);
        char *res = strdup(data);
        env()->ReleaseStringUTFChars((jstring)*this, data);
        return res;
    }

    std::string std_str() const {
        std::string res(str());
        return res;
    }

    _jstring std_jstr() const {
        const jchar* data = env()->GetStringChars((jstring)*this, nullptr);
        _jstring res = data;
        env()->ReleaseStringChars((jstring)*this, data);
        return res;
    }

    operator const std::string() const {
        return std_str();
    }

    operator const _jstring() const {
        return std_jstr();
    }

    bool operator == (const char* str) const {
        return std_str() == str;
    }
    bool operator != (const char* str) const {
        return std_str() != str;
    }
    bool operator == (const jchar* str) const {
        return std_jstr() == str;
    }
    bool operator != (const jchar* str) const {
        return std_jstr() != str;
    }

    static Ref<Class> clazz();
};

/**
 * a java class
*/
class Class : public Object
{
public:
    using Object::Object;

    static LocalRef<Class> forName(const char* name);

    operator jclass() const {
        return (jclass)(jobject)*this;
    }

    LocalRef<String> getName() const;
    jboolean isAssignableFrom(Ref<Class> other) const;
    LocalRef<Class> getSuperclass() const;

    static Ref<Class> clazz();
};

/**
 * generic object array
*/
template <typename T>
class Array : public Object
{
public:
    using Object::Object;

    operator jarray() const {
        return (jarray)(jobject)*this;
    }
    operator jobjectArray() const {
        return (jobjectArray)(jobject)*this;
    }

    jsize length() const {
        return env()->GetArrayLength((jarray)*this);
    }

    LocalRef<T> get(jsize index) const {
        return LocalRef<T>::use( env()->GetObjectArrayElement((jobjectArray)*this, index) );
    }

    void set(jsize index, Ref<T> value) {
        env()->SetObjectArrayElement((jobjectArray)*this, index, value);
    }

    const LocalRef<T> operator[](jsize index) const {
        return get(index);
    }

    static LocalRef<Array<T>> create(jsize length) {
        jclass elementClass = (jclass)T::clazz();
        return LocalRef<Array<T>>::use( Env::get()->NewObjectArray(length, elementClass, nullptr) );
    }

    // static LocalRef<Array<T>> create(std::initializer_list<T> list) {
    //     LocalRef<Array<T>> res = create(list.length());
    //     return res;
    // }

    class Iterator
    {
    private:
        const Array<T>* _obj;
        size_t _idx;
    public:
        Iterator(const Array<T>* obj, size_t idx) : _obj(obj), _idx(idx) {
        }
        bool operator != (const Iterator& that) {
            return that._idx != _idx;
        }
        const Iterator& operator++() {
            _idx++;
            return *this;
        }
        LocalRef<T> operator* () const {
            return (*_obj)[_idx];
        }
    };
    Iterator begin() const {
        return Iterator(this, 0);
    }
    Iterator end() const {
        return Iterator(this, length());
    }
};

/**
 * typed array
*/
template <typename T>
struct _Array {
};

#define M(type,tag) \
template <> struct _Array<type> { \
    using arrayType = type ## Array; \
    static jarray create(jsize length) { return Env::get()->New ## tag ## Array(length); } \
    static void getRegion(arrayType array, jsize index, jsize length, type* buffer) { return Env::get()->Get ## tag ## ArrayRegion(array, index, length, buffer); } \
    static type* getElements(arrayType array, jboolean* isCopy) { return Env::get()->Get ## tag ## ArrayElements(array, isCopy); } \
    static void releaseElements(arrayType array, type* data, jint mode) { Env::get()->Release ## tag ## ArrayElements(array, data, mode); } \
};
JNIPP_M_FOR_ALL_TYPES
#undef M

template<typename T>
class _ElementArray : public Object {
private:
    mutable T* _data;
    mutable bool _dirty;
public:
    using arrayType = typename _Array<T>::arrayType;
    using Object::Object;

    _ElementArray(jobject value) : Object(value), _data(nullptr) {
    }

    _ElementArray(const _ElementArray& other) : Object(other), _data(nullptr) {
    }

    ~_ElementArray() {
        unlock();
    }

    operator jobject() const {
        unlock();
        return Object::operator jobject();
    }

    operator jarray() const {
        return (jarray)_value;
    }
    operator arrayType() const {
        return (arrayType)_value;
    }

    jsize length() const {
        return env()->GetArrayLength((jarray)*this);
    }

    static LocalRef<Array<T>> create(jsize length) {
        return LocalRef<Array<T>>::use( _Array<T>::create(length) );
    }

    void lock() const {
        if (!_data) {
            _data = _Array<T>::getElements((arrayType)*this, nullptr);
            //LOG("Array::lock this=%p _data=%p", this, _data);
            _dirty = false;
        }
    }
    void unlock() const {
        if (_data) {
            T* data = _data;
            _data = nullptr;
            //LOG("Array::unlock this=%p _data=%p _dirty=%d", this, data, _dirty);
            _Array<T>::releaseElements((arrayType)*this, data, _dirty ? 0 : JNI_ABORT);
        }
    }

    T get(jsize index) const {
        lock();
        return _data[index];
    }
    T& getRef(jsize index) {
        lock();
        _dirty = true;
        return _data[index];
    }
    void set(jsize index, T value) {
        lock();
        _dirty = true;
        _data[index] = value;
    }
    T operator[](jsize index) const {
        return get(index);
    }
    T& operator[](jsize index) {
        return getRef(index);
    }
    operator const T*() {
        lock();
        return _data;
    }
    operator T*() {
        lock();
        _dirty = true;
        return _data;
    }
};

#define M(type,tag) \
template<> class Array<type> : public _ElementArray<type> { \
public: \
    using _ElementArray::_ElementArray; \
};
JNIPP_M_FOR_ALL_TYPES
#undef M

////////////////////////////////////////////////////////////////////////////////

/**
 * simple ref base class.
 * does _not_ free the reference on destruction
 * use Ref<X> when passing objects as parameters
 * use LocalRef<X> for returning new objects
 */
template <typename T>
class RefBase {
protected:
    T _impl;
public:
    explicit RefBase(jobject value) : _impl(value) {
    }
    RefBase(const RefBase<T>& value) : _impl((jobject)value) {
    }
    // @TODO: how do we access value._impl ?
    template <typename S>
    RefBase(const RefBase<S>& value) : _impl( static_cast<S>(value.__impl()) ) {
    }
    //template <typename S>
    //Ref(const Ref<S>& value) : _impl((jobject)value) {
    //}
    const T& __impl() const {
        return _impl;
    }

    const T* operator->() const {
        return &_impl;
    }
    T* operator->() {
        return &_impl;
    }
    const T& operator*() const {
        return _impl;
    }
    T& operator*() {
        return _impl;
    }

    // disallow use of []
private:
    void operator[](int idx) const { // !!! use (*ref)[idx]
    }
public:

    template <typename S>
    bool operator == (const Ref<S>& other) const {
        return Env::get()->IsSameObject((jobject)*this, (jobject)*other);
    }
    template <typename S>
    bool operator != (const Ref<S>& other) const {
        return !Env::get()->IsSameObject((jobject)*this, (jobject)*other);
    }

    bool isInstanceOf(const Ref<Class>& cls) const;

    operator jobject() const {
        return (jobject)_impl;
    }
    operator bool() const {
        return (jobject)*this != nullptr && !Env::get()->IsSameObject((jobject)*this, NULL);
    }
};

/**
 * Ref specializations for easier access of Strings and Arrays
*/

template <typename T, typename A1, typename A2>
class Ref : public RefBase<T> {
public:
    using RefBase<T>::RefBase;

    operator jclass() const {
        return (jclass)(**this);
    }

};

/**
 * ref specialization for strings
*/
#if defined(JNIPP_ENABLE_IF_C)
template <typename T>
class Ref<T, typename JNIPP_ENABLE_IF_C<JNIPP_IS_BASE_OF<String, T>::value>::type> : public RefBase<T> {
protected:
    bool _free = false;
public:
    using RefBase<T>::RefBase;
    Ref(const char* value) : RefBase<T>( Env::get()->NewStringUTF(value) ), _free(true) {
        JNIPP_RLOG("Ref::Ref(const char*) this=%p jobject=%p", this, (jobject)*this);
    }
    ~Ref() {
        if (_free) {
            JNIPP_RLOG("Ref::~Ref() this=%p jobject=%p DeleteLocalRef", this, (jobject)*this);
            Env::get()->DeleteLocalRef( (jobject)*this );
        }
    }
    operator std::string() const {
        return std_str();
    }
    std::string std_str() const {
        return (*this)->std_str();
    }
    bool operator == (const char* str) const {
        return this->std_str() == str;
    }
    bool operator != (const char* str) const {
        return this->std_str() != str;
    }
};
#endif

/**
 * ref specialization for classes
*/
#if defined(JNIPP_ENABLE_IF_C)
template <typename T>
class Ref<T, void, typename JNIPP_ENABLE_IF_C<JNIPP_IS_BASE_OF<Class, T>::value>::type> : public RefBase<T> {
public:
    using RefBase<T>::RefBase;
    operator jclass() const {
        return (jclass)(**this);
    }
};
#endif

/**
 * ref specialization for object arrays
*/
template <typename T>
class Ref<Array<T>> : public RefBase<Array<T>> {
public:
    using RefBase<Array<T>>::RefBase;
    const LocalRef<T> operator[] (jsize index) const {
        return (*this)->get(index);
    }
    typename Array<T>::Iterator begin() const {
        return (*this)->begin();
    }
    typename Array<T>::Iterator end() const {
        return (*this)->end();
    }
};

/**
 * ref specialization for typed array
*/
#define M(type,tag) \
template <> \
class Ref<Array<type>> : public RefBase<Array<type>> { \
public: \
    using RefBase<Array<type>>::RefBase; \
    type operator[] (jsize index) const { \
        return (*this)->get(index); \
    } \
    type& operator[] (jsize index) { \
        return (*this)->getRef(index); \
    } \
};
JNIPP_M_FOR_ALL_TYPES
#undef M

/**
 * java local ref
 * calls DeleteLocalRef on destruction
*/
template <typename T>
class LocalRef : public Ref<T> {
protected:
    void __clear() {
        this->_impl.__clear();
    }
    template<typename S> friend class LocalRef;
    explicit LocalRef(jobject value) : Ref<T>(value) {
        if (Env::get()->ExceptionCheck()) {
            __clear();
            value = nullptr; // on android we sometimes get a stale reference on exceptions.
        }
        JNIPP_RLOG("LocalRef::LocalRef(jobject) this=%p jobject=%p (explicit)", this, (jobject)*this);
        if (value) {
            assert( Env::get()->GetObjectRefType(value) == JNILocalRefType );
        }
    }
public:
    template <typename S>
    LocalRef(LocalRef<S>&& value) : Ref<T>((jobject)value) {
        JNIPP_RLOG("LocalRef::LocalRef(LocalRef&&) this=%p value=<%p> jobject=%p (move)", this, &value, (jobject)*this);
        value.__clear();
        if (*this && !Env::get()->ExceptionCheck()) {
            assert( Env::get()->GetObjectRefType((jobject)*this) == JNILocalRefType );
        }
    }
    template <typename S>
    LocalRef(const Ref<S>& value) : Ref<T>(Env::get()->NewLocalRef((jobject)value)) {
        JNIPP_RLOG("LocalRef::LocalRef(Ref&) this=%p value=<%p> jobject=%p (copy)", this, &value, (jobject)*this);
    }
    LocalRef(const WeakRef<T>& value) : Ref<T>(Env::get()->NewLocalRef((jweak)value)) {
        JNIPP_RLOG("LocalRef::LocalRef(WeakRef&) this=%p value=<%p> jobject=%p", this, &value, (jobject)*this);
    }
    ~LocalRef() {
        if ((jobject)*this) {
            JNIPP_RLOG("LocalRef::~LocalRef() this=%p jobject=%p (DeleteLocalRef)", this, (jobject)*this);
            if (Env::peek()) {
                Env::get()->DeleteLocalRef((jobject)*this);
            }
            this->__clear();
        }
    }
    static LocalRef<T> create(jobject value) {
        return LocalRef(Env::get()->NewLocalRef(value));
    }
    static LocalRef<T> use(jobject value)
    {
        return LocalRef<T>(value);
    }
    jobject steal() {
        jobject val = (jobject)*this;
        __clear();
        return val;
    }
    static bool is(jobject value) {
        return Env::get()->GetObjectRefType(value) == JNILocalRefType;
    }
};

/**
 * java global ref
 * calls DeleteGlobalRef on destruction
 */
template <typename T>
class GlobalRef : public Ref<T> {
protected:
    void __clear() {
        this->_impl.__clear();
    }
    explicit GlobalRef(jobject value) : Ref<T>(value) {
        if (value) {
            assert( Env::get()->GetObjectRefType(value) == JNIGlobalRefType );
        }
        JNIPP_RLOG("GlobalRef::GlobalRef(jobject) this=%p jobject=%p (explicit)", this, (jobject)*this);
    }
public:
    GlobalRef() : Ref<T>((jobject)nullptr) {
        JNIPP_RLOG("GlobalRef::GlobalRef() this=%p (empty)", this);
    }
    template <typename S>
    GlobalRef(GlobalRef<S>&& value) : Ref<T>((jobject)value) {
        JNIPP_RLOG("GlobalRef::GlobalRef(GlobalRef&&) this=%p value=<%p> jobject=%p (move)", this, &value, (jobject)*this);
        value.__clear();
    }
    template <typename S>
    GlobalRef(const Ref<S>& value) : Ref<T>(Env::get()->NewGlobalRef((jobject)value)) {
        JNIPP_RLOG("GlobalRef::GlobalRef(Ref&) this=%p value=<%p> jobject=%p (copy)", this, &value, (jobject)*this);
    }
    GlobalRef(const WeakRef<T>& value) : Ref<T>(Env::get()->NewGlobalRef((jweak)value)) {
        JNIPP_RLOG("GlobalRef::GlobalRef(WeakRef&) this=%p value=<%p> jobject=%p", this, &value, (jobject)*this);
    }
    ~GlobalRef() {
        if ((jobject)*this) {
            JNIPP_RLOG("GlobalRef::~GlobalRef() this=%p jobject=%p (DeleteGlobalRef)", this, (jobject)*this);
            if (Env::peek()) Env::get()->DeleteGlobalRef((jobject)*this);
            this->__clear();
        }
    }
//    static GlobalRef<T> use(jobject value)
//    {
//        return GlobalRef<T>(value);
//    }
    template <typename S>
    void set(const Ref<S>& value) {
        if (*this) Env::get()->DeleteGlobalRef((jobject)*this);
        this->_impl = T(Env::get()->NewGlobalRef((jobject)value));
    }
    static bool is(jobject value) {
        return Env::get()->GetObjectRefType(value) == JNIGlobalRefType;
    }
    template <typename S>
    void operator= (const Ref<S>& value) {
        set(value);
    }
};

/**
 * java weak ref
 * operator bool will return false if the weak reference is gone
 * cannot be used directly - will be cast to LocalRef instead to make sure the reference is not GCed
 */
template <typename T>
class WeakRef {
protected:
    T _impl;
    void __clear() {
        this->_impl.__clear();
    }
    template<typename S> friend class WeakRef;
public:
    WeakRef() : _impl(nullptr) {
        JNIPP_RLOG("WeakRef::WeakRef() this=%p", this);
    }
    // explicit WeakRef(jobject value) : _impl(Env::get()->NewWeakGlobalRef(value)) {
    //     JNIPP_RLOG("WeakRef::WeakRef(jobject) this=%p jobject=%p", this, (jweak)*this);
    // }
    explicit WeakRef(jweak value) : _impl((jobject)value) {
        assert( Env::get()->GetObjectRefType(value) == JNIWeakGlobalRefType );
        JNIPP_RLOG("WeakRef::WeakRef(jweak) this=%p jobject=%p", this, (jweak)*this);
    }
    template <typename S>
    WeakRef(WeakRef<S>&& value) : _impl((jobject)(jweak)value) {
        JNIPP_RLOG("WeakRef::WeakRef(WeakRef&&) this=%p value=<%p> jobject=%p", this, &value, (jweak)*this);
        value.__clear();
    }
    template <typename S>
    WeakRef(const Ref<S>& value) : _impl(Env::get()->NewWeakGlobalRef((jobject)value)) {
        JNIPP_RLOG("WeakRef::WeakRef(Ref&) this=%p value=<%p> jobject=%p", this, &value, (jweak)*this);
    }
    ~WeakRef() {
        if ((jweak)*this) {
            JNIPP_RLOG("WeakRef::~WeakRef() this=%p jobject=%p", this, (jweak)*this);
            if (Env::peek()) Env::get()->DeleteWeakGlobalRef((jweak)*this);
            this->__clear();
        }
    }
    operator jweak() const {
        return (jweak)(jobject)_impl;
    }
    template <typename S>
    bool operator == (const Ref<S>& other) {
        return Env::get()->IsSameObject((jweak)*this, (jobject)*other);
    }
    template <typename S>
    bool operator != (const Ref<S>& other) {
        return !Env::get()->IsSameObject((jweak)*this, (jobject)*other);
    }
    template <typename S>
    bool operator == (const WeakRef<S>& other) {
        return Env::get()->IsSameObject((jweak)*this, (jweak)*other);
    }
    template <typename S>
    bool operator != (const WeakRef<S>& other) {
        return !Env::get()->IsSameObject((jweak)*this, (jweak)*other);
    }
    operator bool() const {
        return (jweak)*this != nullptr && !Env::get()->IsSameObject((jweak)*this, NULL);
    }
    template <typename S>
    void set(const Ref<S>& value) {
        if ((jweak)*this) Env::get()->DeleteWeakGlobalRef((jweak)*this);
        this->_impl = T(Env::get()->NewWeakGlobalRef((jobject)value));
    }
    static bool is(jobject value) {
        return Env::get()->GetObjectRefType(value) == JNIWeakGlobalRefType;
    }
};

////////////////////////////////////////////////////////////////////////////////

/**
 * templates for converting function arguments
*/

#define M(type,tag) \
inline type _ConvertArg(type value) { \
    return value; \
}
JNIPP_M_FOR_ALL_TYPES
#undef M

inline jstring _ConvertArg(jstring value) {
    return value;
}
inline jobject _ConvertArg(jobject value) {
    return value;
}
inline jclass _ConvertArg(jclass value) {
    return value;
}
template <typename S>
inline jobject _ConvertArg(const Ref<S>& value) {
    return (jobject)value;
}

template <typename T>
struct _AsRef {
    using R = Ref<T>;
};
#define M(type,tag) \
template <> struct _AsRef<type> { \
    using R = type; \
};
JNIPP_M_FOR_ALL_TYPES
#undef M

////////////////////////////////////////////////////////////////////////////////

class MethodBase {
protected:
    jclass _cls;
    const char* _clsName;
    const char* _name;
    const char* _signature;
    mutable jmethodID _methodID;
public:
    MethodBase(const char* clsName, const char* name, const char* signature) : _cls(nullptr), _clsName(clsName), _name(name), _signature(signature), _methodID(0) {
    }
    MethodBase(GlobalRef<Class>& cls, const char* name, const char* signature) : _cls((jclass)(jobject)cls), _clsName(nullptr), _name(name), _signature(signature), _methodID(0) {
    }
    jmethodID getMethodID() const {
        if (_methodID == nullptr) {
            JNIEnv* env = Env::get();
            jclass cls = _cls ? _cls : env->FindClass(_clsName);
            JNIPP_ASSERT(cls, "Method: clsName not found");
            jmethodID res = env->GetMethodID(cls, _name, _signature);
            JNIPP_ASSERT(res, "Method: method not found");
            _methodID = res;
        }
        return _methodID;
    }
    operator jmethodID() const {
        return getMethodID();
    }
};

template <typename R, typename... A>
class Method : public MethodBase
{
public:
    using MethodBase::MethodBase;
    LocalRef<R> call(jobject target, typename _AsRef<A>::R... args) const {
        return LocalRef<R>::use( Env::get()->CallObjectMethod(target, getMethodID(), _ConvertArg(args)...) );
    }
    LocalRef<R> operator()(jobject target, typename _AsRef<A>::R... args) const {
        return call(target, args...);
    }
};

template<typename... A>
class Method<void, A...> : public MethodBase
{
public:
    using MethodBase::MethodBase;
    void call(jobject target, typename _AsRef<A>::R... args) const {
        Env::get()->CallVoidMethod(target, getMethodID(), _ConvertArg(args)...);
    }
    void operator()(jobject target, typename _AsRef<A>::R... args) const {
        call(target, args...);
    }
};

#define M(type,tag) \
template<typename... A> \
class Method<type, A...> : public MethodBase \
{ \
public: \
    using MethodBase::MethodBase; \
    type call(jobject target, typename _AsRef<A>::R... args) const { \
        return Env::get()->Call ## tag ## Method(target, getMethodID(), _ConvertArg(args)...); \
    } \
    type operator()(jobject target, typename _AsRef<A>::R... args) const { \
        return call(target, args...); \
    } \
};
JNIPP_M_FOR_ALL_TYPES
#undef M

////////////////////////////////////////////////////////////////////////////////

class NonvirtualMethodBase {
protected:
    jclass _cls;
    const char* _clsName;
    const char* _name;
    const char* _signature;
    mutable jmethodID _methodID;
public:
    NonvirtualMethodBase(const char* clsName, const char* name, const char* signature) : _cls(nullptr), _clsName(clsName), _name(name), _signature(signature), _methodID(0) {
    }
    NonvirtualMethodBase(GlobalRef<Class>& cls, const char* name, const char* signature) : _cls((jclass)(jobject)cls), _clsName(nullptr), _name(name), _signature(signature), _methodID(0) {
    }
    jmethodID getMethodID() const {
        if (_methodID == nullptr) {
            JNIEnv* env = Env::get();
            jclass cls = _cls ? _cls : env->FindClass(_clsName);
            JNIPP_ASSERT(cls, "NonvirtualMethod: clsName not found");
            jmethodID res = env->GetMethodID(cls, _name, _signature);
            JNIPP_ASSERT(res, "NonvirtualMethod: method not found");
            _methodID = res;
        }
        return _methodID;
    }
    operator jmethodID() const {
        return getMethodID();
    }
};

template <typename R, typename... A>
class NonvirtualMethod : public NonvirtualMethodBase
{
public:
    using NonvirtualMethodBase::NonvirtualMethodBase;
    LocalRef<R> call(jobject target, typename _AsRef<A>::R... args) const {
        return LocalRef<R>::use( Env::get()->CallNonvirtualObjectMethod(target, getMethodID(), _ConvertArg(args)...) );
    }
    LocalRef<R> operator()(jobject target, typename _AsRef<A>::R... args) const {
        return call(target, args...);
    }
};

template<typename... A>
class NonvirtualMethod<void, A...> : public NonvirtualMethodBase
{
public:
    using NonvirtualMethodBase::NonvirtualMethodBase;
    void call(jobject target, typename _AsRef<A>::R... args) const {
        return Env::get()->CallNonvirtualVoidMethod(target, getMethodID(), _ConvertArg(args)...);
    }
    void operator()(jobject target, typename _AsRef<A>::R... args) const {
        call(target, args...);
    }
};

#define M(type,tag) \
template<typename... A> \
class NonvirtualMethod<type, A...> : public NonvirtualMethodBase \
{ \
public: \
    using NonvirtualMethodBase::NonvirtualMethodBase; \
    type call(jobject target, typename _AsRef<A>::R... args) const { \
        return Env::get()->CallNonvirtual ## tag ## Method(target, getMethodID(), _ConvertArg(args)...); \
    } \
    type operator()(jobject target, typename _AsRef<A>::R... args) const { \
        return call(target, args...); \
    } \
};
JNIPP_M_FOR_ALL_TYPES
#undef M

////////////////////////////////////////////////////////////////////////////////

class StaticMethodBase {
protected:
    jclass _cls;
    const char* _clsName;
    const char* _name;
    const char* _signature;
    mutable jmethodID _methodID;
public:
    StaticMethodBase(const char* clsName, const char* name, const char* signature) : _cls(nullptr), _clsName(clsName), _name(name), _signature(signature), _methodID(0) {
    }
    StaticMethodBase(GlobalRef<Class>& cls, const char* name, const char* signature) : _cls((jclass)(jobject)cls), _clsName(nullptr), _name(name), _signature(signature), _methodID(0) {
    }
    jmethodID getMethodID() const {
        if (_methodID == nullptr) {
            JNIEnv* env = Env::get();
            jclass cls = _cls ? _cls : getClass();
            jmethodID res = env->GetStaticMethodID(cls, _name, _signature);
            JNIPP_ASSERT(res, "StaticMethod: method not found");
            _methodID = res;
        }
        return _methodID;
    }
    jclass getClass() const {
        JNIEnv* env = Env::get();
        jclass cls = _cls ? _cls : env->FindClass(_clsName);
        JNIPP_ASSERT(cls, "StaticMethod: clsName not found");
        return cls;
    }
    operator jmethodID() const {
        return getMethodID();
    }
    operator jclass() const {
        return getClass();
    }
};

template <typename R, typename... A>
class StaticMethod : public StaticMethodBase
{
public:
    using StaticMethodBase::StaticMethodBase;
    LocalRef<R> call(typename _AsRef<A>::R... args) const {
        return LocalRef<R>::use( Env::get()->CallStaticObjectMethod(getClass(), getMethodID(), _ConvertArg(args)...) );
    }
    LocalRef<R> operator()(typename _AsRef<A>::R... args) const {
        return call(args...);
    }
};

template<typename... A>
class StaticMethod<void, A...> : public StaticMethodBase
{
public:
    using StaticMethodBase::StaticMethodBase;
    void call(typename _AsRef<A>::R... args) const {
        return Env::get()->CallStaticVoidMethod(getClass(), getMethodID(), _ConvertArg(args)...);
    }
    void operator()(typename _AsRef<A>::R... args) const {
        call(args...);
    }
};

#define M(type,tag) \
template<typename... A> \
class StaticMethod<type, A...> : public StaticMethodBase \
{ \
public: \
    using StaticMethodBase::StaticMethodBase; \
    type call(typename _AsRef<A>::R... args) const { \
        return Env::get()->CallStatic ## tag ## Method(getClass(), getMethodID(), _ConvertArg(args)...); \
    } \
    type operator()(typename _AsRef<A>::R... args) const { \
        return call(args...); \
    } \
};
JNIPP_M_FOR_ALL_TYPES
#undef M

////////////////////////////////////////////////////////////////////////////////


template <typename R, typename... A>
class Constructor {
protected:
    jclass _cls;
    const char* _clsName;
    const char* _signature;
    mutable jmethodID _methodID;
public:
    Constructor(const char* clsName, const char* signature) : _cls(nullptr), _clsName(clsName), _signature(signature), _methodID(0) {
    }
    Constructor(GlobalRef<Class>& cls, const char* signature) : _cls((jclass)(jobject)cls), _clsName(nullptr), _signature(signature), _methodID(0) {
    }
    jmethodID getMethodID() const {
        if (_methodID == nullptr) {
            JNIEnv* env = Env::get();
            jclass cls = getClass();
            jmethodID res = env->GetMethodID(cls, "<init>", _signature);
            JNIPP_ASSERT(res, "Constructor: method not found");
            _methodID = res;
        }
        return _methodID;
    }
    jclass getClass() const {
        JNIEnv* env = Env::get();
        jclass cls = _cls ? _cls : env->FindClass(_clsName);
        JNIPP_ASSERT(cls, "Constructor: clsName not found");
        return cls;
    }
    operator jmethodID() const {
        return getMethodID();
    }
    operator jclass() const {
        return getClass();
    }
    LocalRef<R> construct(typename _AsRef<A>::R... args) const {
        return LocalRef<R>::use( Env::get()->NewObject(getClass(), getMethodID(), _ConvertArg(args)...) );
    }
    LocalRef<R> operator()(typename _AsRef<A>::R... args) const {
        return construct(args...);
    }
};

////////////////////////////////////////////////////////////////////////////////

class FieldBase {
protected:
    jclass _cls;
    const char* _clsName;
    const char* _name;
    const char* _signature;
    mutable jfieldID _fieldID;
public:
    FieldBase(const char* clsName, const char* name, const char* signature) : _cls(nullptr), _clsName(clsName), _name(name), _signature(signature), _fieldID(0) {
    }
    FieldBase(GlobalRef<Class>& cls, const char* name, const char* signature) : _cls((jclass)(jobject)cls), _clsName(nullptr), _name(name), _signature(signature), _fieldID(0) {
    }
    jfieldID getFieldID() const {
        if (_fieldID == nullptr) {
            JNIEnv* env = Env::get();
            jclass cls = _cls ? _cls : env->FindClass(_clsName);
            JNIPP_ASSERT(cls, "Field: clsName not found");
            jfieldID res = env->GetFieldID(cls, _name, _signature);
            JNIPP_ASSERT(res, "Field: field not found");
            _fieldID = res;
        }
        return _fieldID;
    }
    operator jfieldID() const {
        return getFieldID();
    }
};

template <typename R>
class Field : public FieldBase
{
public:
    using FieldBase::FieldBase;
    LocalRef<R> get(jobject target) const {
        JNIPP_ASSERT(target, "Field::get: target is null");
        return LocalRef<R>::use( Env::get()->GetObjectField(target, getFieldID()) );
    }
    void set(jobject target, Ref<R> value) {
        JNIPP_ASSERT(target, "Field::set: target is null");
        Env::get()->SetObjectField(target, getFieldID(), value);
    }
};

#define M(type,tag) \
template <> \
class Field<type> : public FieldBase \
{ \
public: \
    using FieldBase::FieldBase; \
    type get(jobject target) const { \
        JNIPP_ASSERT(target, "Field.get: target is null"); \
        return Env::get()->Get ## tag ## Field(target, getFieldID()); \
    } \
    void set(jobject target, type value) { \
        JNIPP_ASSERT(target, "Field.set: target is null"); \
        Env::get()->Set ## tag ## Field(target, getFieldID(), value); \
    } \
};
JNIPP_M_FOR_ALL_TYPES
#undef M

////////////////////////////////////////////////////////////////////////////////

class BoundFieldBase {
protected:
    jclass _cls;
    const char* _clsName;
    const char* _name;
    const char* _signature;
    mutable jfieldID _fieldID;
    Object* _thiz;
public:
    BoundFieldBase() : _cls(nullptr), _clsName(nullptr), _name(nullptr), _signature(nullptr), _fieldID(0), _thiz(nullptr) {
        // @TODO should not be here.
    }
    BoundFieldBase(const char* clsName, const char* name, const char* signature, Object* thiz) : _cls(nullptr), _clsName(clsName), _name(name), _signature(signature), _fieldID(0), _thiz(thiz) {
    }
    BoundFieldBase(GlobalRef<Class>& cls, const char* name, const char* signature, Object* thiz) : _cls((jclass)(jobject)cls), _clsName(nullptr), _name(name), _signature(signature), _fieldID(0), _thiz(thiz) {
    }
    jfieldID getFieldID() const {
        if (_fieldID == nullptr) {
            JNIEnv* env = Env::get();
            jclass cls = _cls ? _cls : env->FindClass(_clsName);
            JNIPP_ASSERT(cls, "BoundField: clsName not found");
            jfieldID res = env->GetFieldID(cls, _name, _signature);
            JNIPP_ASSERT(res, "BoundField: field not found");
            _fieldID = res;
        }
        return _fieldID;
    }
    operator jfieldID() const {
        return getFieldID();
    }
};

template <typename R>
class BoundField : public BoundFieldBase
{
public:
    using BoundFieldBase::BoundFieldBase;
    LocalRef<R> get() const {
        JNIPP_ASSERT((jobject)*_thiz, "BoundField.get: this is null");
        return LocalRef<R>::use( Env::get()->GetObjectField(*_thiz, getFieldID()) );
    }
    operator LocalRef<R>() const {
        return get();
    }
    operator GlobalRef<R>() const {
        return get();
    }
    LocalRef<R> operator->() const {
        return get();
    }
    void set(Ref<R> value) {
        JNIPP_ASSERT((jobject)*_thiz, "BoundField.set: this is null");
        Env::get()->SetObjectField(*_thiz, getFieldID(), value);
    }
};

#define M(type,tag) \
template <> \
class BoundField<type> : public BoundFieldBase \
{ \
public: \
    using BoundFieldBase::BoundFieldBase; \
    type get() const { \
        JNIPP_ASSERT((jobject)*_thiz, "BoundField.get: this is null"); \
        return Env::get()->Get ## tag ## Field(*_thiz, getFieldID()); \
    } \
    operator type() const { \
        return get(); \
    } \
    void set(type value) { \
        JNIPP_ASSERT((jobject)*_thiz, "BoundField.set: this is null"); \
        Env::get()->Set ## tag ## Field(*_thiz, getFieldID(), value); \
    } \
    void operator=(type value) { \
        set(value); \
    } \
};
JNIPP_M_FOR_ALL_TYPES
#undef M

////////////////////////////////////////////////////////////////////////////////

class StaticFieldBase {
protected:
    mutable jclass _cls;
    const char* _clsName;
    const char* _name;
    const char* _signature;
    mutable jfieldID _fieldID;
public:
    StaticFieldBase(const char* clsName, const char* name, const char* signature) : _cls(nullptr), _clsName(clsName), _name(name), _signature(signature), _fieldID(0) {
    }
    StaticFieldBase(GlobalRef<Class>& cls, const char* name, const char* signature) : _cls((jclass)(jobject)cls), _clsName(nullptr), _name(name), _signature(signature), _fieldID(0) {
    }
    jfieldID getFieldID() const {
        if (_fieldID == nullptr) {
            JNIEnv* env = Env::get();
            if (_cls == nullptr && _clsName != nullptr) {
                _cls = (jclass)env->NewGlobalRef(env->FindClass(_clsName));
            }
            JNIPP_ASSERT(_cls, "StaticField: clsName not found");
            jfieldID res = env->GetStaticFieldID(_cls, _name, _signature);
            JNIPP_ASSERT(res, "StaticField: field not found");
            _fieldID = res;
        }
        return _fieldID;
    }
    jclass getClass() const {
        getFieldID();
        return _cls;
    }
    operator jfieldID() const {
        return getFieldID();
    }
    operator jclass() const {
        return getClass();
    }
};

template <typename R>
class StaticField : public StaticFieldBase
{
public:
    using StaticFieldBase::StaticFieldBase;
    LocalRef<R> get() const {
        return LocalRef<R>::use( Env::get()->GetStaticObjectField(getClass(), getFieldID()) );
    }
    operator LocalRef<R>() const {
        return get();
    }
    operator GlobalRef<R>() const {
        return get();
    }
    LocalRef<R> operator->() const {
        return get();
    }
    void set(Ref<R> value) {
        Env::get()->SetStaticObjectField(getClass(), getFieldID(), value);
    }
    void operator=(Ref<R> value) {
        set(value);
    }
};

#define M(type,tag) \
template <> \
class StaticField<type> : public StaticFieldBase \
{ \
public: \
    using StaticFieldBase::StaticFieldBase; \
    type get() const { \
        return Env::get()->GetStatic ## tag ## Field(getClass(), getFieldID()); \
    } \
    operator type() const { \
        return get(); \
    } \
    void set(type value) { \
        Env::get()->SetStatic ## tag ## Field(getClass(), getFieldID(), value); \
    } \
    void operator=(type value) { \
        set(value); \
    } \
};
JNIPP_M_FOR_ALL_TYPES
#undef M

////////////////////////////////////////////////////////////////////////////////

inline LocalRef<Object> Env::getException() {
    jobject e = get()->ExceptionOccurred();
    get()->ExceptionClear();
    return LocalRef<Object>::use(e);
}

inline void Env::throwException(Ref<Object> exception)
{
    get()->Throw((jthrowable)(jobject)*exception);
}
inline void Env::throwException(Ref<Class> cls, Ref<String> message) {
    GlobalRef<Class> gcls(cls);
    Constructor<Object,String> method(gcls, "(Ljava/lang/String;)V");
    throwException(method.construct(message));
}
inline void Env::throwException(const char* cls, const char* message) {
    throwException(Class::forName(cls), String::create(message));
}

////////////////////////////////////////////////////////////////////////////////

template <typename T>
bool RefBase<T>::isInstanceOf(const Ref<Class>& cls) const {
    return Env::get()->IsInstanceOf(*this, cls);
}

inline LocalRef<String> Object::toString() const {
    Method<String> method("java/lang/Object", "toString", "()Ljava/lang/String;");
    return method.call(*this);
}

inline LocalRef<Class> Object::getClass() const {
    return LocalRef<Class>::use( env()->GetObjectClass((jobject)*this) );
}

inline jboolean Object::isInstanceOf(Ref<Class> cls) const {
    return env()->IsInstanceOf((jobject)*this, (jclass)*cls);
}

inline Monitor Object::lock() const {
    return Monitor((jobject)*this);
}

inline Ref<Class> Object::clazz() {
    static GlobalRef<Class> cls;
    if (!cls) cls.set(Class::forName("java/lang/Object"));
    return cls;
}

inline LocalRef<String> String::create(const char* value) {
    return LocalRef<String>::use( Env::get()->NewStringUTF(value) );
}

inline LocalRef<String> String::create(const std::string value) {
    return LocalRef<String>::use( Env::get()->NewStringUTF(value.c_str()) );
}

inline LocalRef<String> String::create(const jchar* value) {
    String::_jstring tmp = value;
    return create(value, tmp.length());
}

inline LocalRef<String> String::create(const jchar* value, size_t length) {
    return LocalRef<String>::use( Env::get()->NewString(value, length) );
}

inline LocalRef<String> String::create(const String::_jstring value) {
    return create(value.c_str(), value.length());
}

inline Ref<Class> String::clazz() {
    static GlobalRef<Class> cls;
    if (!cls) cls.set(Class::forName("java/lang/String"));
    return cls;
}

inline LocalRef<Class> Class::forName(const char* name) {
    return LocalRef<Class>::use( (jobject)Env::get()->FindClass(name) );
}

inline LocalRef<String> Class::getName() const {
    static Method<String> method("java/lang/Class", "getName", "()Ljava/lang/String;");
    return method.call(*this);
}

inline jboolean Class::isAssignableFrom(Ref<Class> other) const {
    return env()->IsAssignableFrom(*other, *this);
}

inline LocalRef<Class> Class::getSuperclass() const {
    return LocalRef<Class>::use( env()->GetSuperclass((jclass)*this) );
}

inline Ref<Class> Class::clazz() {
    static GlobalRef<Class> cls;
    if (!cls) cls.set(Class::forName("java/lang/Class"));
    return cls;
}

////////////////////////////////////////////////////////////////////////////////

}

#endif


