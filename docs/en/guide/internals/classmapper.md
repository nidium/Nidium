---
description: Binding between javascript en nidium
---

## Basic concept

`ClassMapper<T>` is a class template built in nidium that helps creating mapping between C++ classes and exposed Javascript classes.  

It provides a way to map constructor, methods, getter and setter in a 1:1 manner between the two world.  

### Example

```cpp
class MyClass : public ClassMapper<MyClass>
{
public:
    static void RegisterObject(JSContext *cx);

    static MyClass *Constructor(JSContext *cx, JS::CallArgs &args,
        JS::HandleObject obj);

    static JSFunctionSpec *ListMethods();
    static JSPropertySpec *ListProperties();

    virtual ~MyClass() {}
    MyClass() {}
protected:
    /*
        Declare a JS method
    */
    NIDIUM_DECL_JSCALL(myMethod);

    /*
        Declare a JS property
    */
    NIDIUM_DECL_JSGETTER(myProperty);
};



void MyClass::RegisterObject(JSContext *cx)
{
    /**
        Expose |MyClass| to the global scope
        Prototype is available through "MyClass.prototype"
    */
    MyClass::ExposeClass<1>(cx, "MyClass");
//                       ^-------,
//                               Minimum number of arguments
//                               for the Constructor
}

/**
    Called during a JS "var obj = new MyClass(param) call"
    Returning nullptr trigger a JS exception
    
    @param args  A list of arguments provided to the contructor
    @param obj   The JSObject corresponding to the instance
*/
MyClass *MyClass::Constructor(JSContext *cx, JS::CallArgs &args,
    JS::HandleObject obj)
{
    /**
        arguments are available in args() like a regular method call.
    */
    return new MyClass();
}

/**
    Called during a JS "obj.myMethod(param)" call
    Returning nullptr trigger a JS exception

    @param args  A list of arguments provided to method call
                 Return value is set via args.rval().setXXX()
*/
bool MyClass::JS_myMethod(JSContext *cx, JS::CallArgs &args)
{

    /*
        Convert the first argument as a string
    */
    JS::RootedString myparam(cx, JS::ToString(cx, args[0]));
    JSAutoByteString cmyparam;

    cmyparam.encodeUtf8(cx, myparam);

    printf("First argument is %s\n", cmyparam.ptr());

    /*
        Set the JavaScript value returned to "myMethod"
    */
    args.rval().setBoolean(true);

    /*
        No Exception
    */
    return true;
}

/**
    Called when accessing "obj.myProperty".

    @param vp  The value of "myProperty" to be returned.
               Set using vp.setXXX()
*/
bool MyClass::JSGetter_myProperty(JSContext *cx, JS::MutableHandleValue vp)
{
    vp.setInt32(42);

    return true;
}

/**
    Optional.
    List all the methods available on MyClass prototype

    CLASSMAPPER_FN arguments :
        1. Must be the name of the current C++ class
        2. The name of the method exposed to the JS
        3. Minimum number of arguments
*/
JSFunctionSpec *MyClass::ListMethods()
{
    static JSFunctionSpec funcs[] = {
        CLASSMAPPER_FN(MyClass, myMethod, 1),
        JS_FS_END
    };

    return funcs;
}

/**
    Optional.
    List all the properties available on MyClass prototype
*/
JSPropertySpec *MyClass::ListProperties()
{
    static JSPropertySpec props[] = {
        CLASSMAPPER_PROP_G(MyClass, myProperty),
        JS_PS_END
    };

    return props;
}
```

## GC Rooting and object lifetime

ClassMapper provides two different ways to handle the lifetime of instantiated objects.  

The default behavior is to let the JS garbage collector handles the destruction of the object. That is, when the object become unreachable from the JS code, the JS engine will finalize the underlying `JSObject` and ClassMapper will `delete` the C++ instance.  

### Example of GC controlled lifetime

```javascript
function foobar()
{
    var x = new MyClass("hello");

    // After foobar() has returned, |x| becomes unreachable
    // and is marked for deletion by the garbage collector.
    // The C++ destructor ~MyClass() is then automatically called.
}

foobar();
```

However it's also possible for a ClassMapper instance to takeover the ownership of the object and controls its lifetime.  
Let's say you have some internal pending operation on your C++ object and you don't want the GC to delete the instance for you.  

The only thing you need to do is to call `root()` on your C++ instance.  

### Example of C++ controlled lifetime

```cpp
MyClass *MyClass::Constructor(JSContext *cx, JS::CallArgs &args,
    JS::HandleObject obj)
{
    MyClass *myclass = new MyClass();
    myclass->root(); // Tells the JS engine to keep a
                     // reference to the underlying JSObject.
    return myclass();
}
```

```javascript
function foobar()
{
    var x = new MyClass("hello");

    // After foobar() has returned, |x| becomes unreachable to the JavaScript code.
    // However, |x| was internally maked as
    // reachable by calling |myclass->root()| meanings 
    // it's still reachable by C++
}

foobar();
```

It's now up to the C++ to call either `unroot()` on the instance or directly `delete` the object.

## Non constructible Class

It's possible that a JS class is not constructible by the end-user but only by the C++.  

For instance, imagine a `Socket` class (which is constructible), where an instance of `SocketClientConnection` is passed as a callback argument when a connection is established.  
In that case, the user doesn't have to create the instance manually.

```javascript
var socket = new Socket(8080).listen();
socket.onaccept = function(client) {
    // client is an instance of SocketClientConnection
}
```

It's however possible for the user to change the prototype of `SocketClientConnection`, so we need to expose this Class to the JavaScript but disallow its constructor.


```javascript
// Error Uncaught TypeError: Illegal constructor(…)
var test = new SocketClientConnection();
```

```javascript
/** Extending the prototype through
    the exposed Class is possible.
*/

SocketClientConnection.prototype.foo = 42;

var socket = new Socket(8080).listen();
socket.onaccept = function(client) {
    console.log(client.foo); // 42
}
```

In order to create a non constructible Class, the only thing you need is to omit the `Constructor` method in your C++ class (`static T *Constructor()`).

You can then create an instance from the C++ using `ClassMapper<T>::CreateObject()`.

```cpp
MyClass *myclass = new MyClass();
JS::RootedObject jinstance(m_Cx,
    MyClass::CreateObject(m_Cx, myclass));
```

## Relationship between `JSObject` and C++ instance

Because `ClassMapper<T>` maps a JS Class to a C++ Class, there is a close relationship between a `JSObject` and its C++ mapped object.

You can easily retrieve a `JSObject` from a `ClassMapper<T>` instance and Vice versa.

### Get a C++ object from a JSObject

`ClassMapper<T>::GetInstance(JS::HandleObject)` returns the mapped C++ object if existing. Otherwise returns `nullptr`

```cpp
MyClass *myclass = MyClass::GetInstance(a_js_object);
if (myclass != nullptr) {

}
```

### Get a JSObject from a C++ object

```cpp
JS::RootedObject my_js_object(cx, myclass->getJSObject());
```

## Custom JSClass flags and properties

A default `JSClass *` is created per ClassMapper<T> variant (but has the same address for different instance of the same variant).  

It's possible to pass over custom flags (`.flags` field of `struct JSClass`) via `ClassMapper<T>::ExposeClass` third argument.

```cpp
void MyClass::RegisterObject(JSContext *cx)
{
    MyClass::ExposeClass<1>(cx, "MyClass", JSCLASS_HAS_RESERVED_SLOTS(2));
}
```

Please note that it's not possible to use `JSCLASS_HAS_PRIVATE` because it's internally used and reserved by ClassMapper.

### Replacing the default JSClass entirely.

It's possible to replace the default JSClass by setting the static method `static JSClass *GetJSClass();` on your Class.

