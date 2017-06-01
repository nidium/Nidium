---
description: JavaScript rooting API - nidium
---

Rooting remains the hardest part to handle when using JSAPI though the rules are pretty simple.

There are two main rooting things :

* Values created on the **Stack** (short lived on the C++ scope)
* Values living on the **Heap** (long lived on the C++ scope)

## Stack
This is the simplest form of rooting yet it's the most verbose.
There are 4 kinds of "type" of GC things :

### JS::Value / JSObject (and so on)

Those are unboxed values and not rooted. They are usually returned by a function call (e.g. JS_NewObject).

### JS::Rooted<T>

Those are used in order to root a value or object returned by a JSAPI call (or any call). JSAPI always returns unboxed values (non rooted). This is needed if you need to call JSAPI with a value or object as argument.

```cpp
JS::RootedObject obj(cx, JS_NewObject(cx, nullptr, JS::NullPtr(), JS::NullPtr()));
```

### JS::Handle<T>

Those are boxed version of JS::Value, JSObject, etc... They must be used as the input of a function argument. They ensure that you're not calling a function with a non rooted thing at compile time. Whenever you create a method that takes a JS::Value/JSObject (or any GC thing) as argument input. **Use this**. A JS::Rooted<t> automatically convert to this type when needed.

```cpp
void foobar(JS::HandleObject obj)
{
  // you can use obj without rooting it. (You can call into JSAPI directly with it)
}

JS::RootedObject obj(cx, JS_NewObject(cx, nullptr, JS::NullPtr(), JS::NullPtr()));
foobar(obj);
```

### JS::MutableHandle<T>

Used as output parameter of a function call. This is the mutable version of ```JS::Handle<T>```.

```cpp
bool foobar(JS::HandleObject obj, JS::MutableHandleValue out)
{
  out.setInt32(42);
  return true;
}

JS::RootedObject obj(cx, JS_NewObject(cx, nullptr, JS::NullPtr(), JS::NullPtr()));
JS::RootedValue out(cx);
foobar(obj, &out);
```

*TODO: AutoRooter explanation (for Array)*

Rules :

- Never create a method or function that take an ***unboxed*** value/object as argument.
- Always return an ***unboxed*** value/object from a function/method.

## Heap

This is the most difficult one. How to root a long lived JS thing.  
It often happen that we need to keep a reference to a JS thing (be it JSObject or JS::Value) that is invisible to the JS user land.

Example :

```javascript
function run() {
    var socket = new Socket("0.0.0.0", 8000).listen();
    socket.onaccept = function(client) {
        client.write("Hello");
    }
}

run();
```

Here, after the end of the ```run()``` function, the Javascript thing that ```socket``` became unreachable and tell the GC to recollect its memory. It has no way to know by itself that the ```onaccept``` callback can be called later by a persistant state the engine is not aware of (in that case, a listening socket).

We thus need to tell the engine that ```socket``` is still reachable and alive in our C++ code, and we have multiple way to achieve this fate.

Since you're not allowed to store an unboxed value/object on the heap, there are two way to keep a reference to  JS::Value/JSObject/JSWhatever on the Heap :

### JS::Heap<T>

This are boxed value of GC things. But they are ***not rooted***! It's exactly like storing a bare unboxed value/object (though in the correct way).

```cpp
class foobar
{
public:
    JS::Heap<JSObject *> m_MyObject;
}

JS::RootedObject obj(cx, JS_NewObject(cx, nullptr, JS::NullPtr(), JS::NullPtr()));

foobar *foo = new foobar();
foo->m_MyObject = obj;

// We now need to somehow root foo.m_MyObject
```

### JS::PersistentRooted<T>

This are boxed value of GC things but automatically rooted and unrooted upon deletion (be it through ```delete``` or whatever).

```cpp
class foobar
{
public:
    foobar(JSContext *cx) : m_MyObject(cx) {};
    JS::PersistentRooted<JSObject *> m_MyObject;
}

JS::RootedObject obj(cx, JS_NewObject(cx, nullptr, JS::NullPtr(), JS::NullPtr()));

foobar *foo = new foobar(cx); // We need a cx in order to initialize the JS::PersistentRooted
foo->m_MyObject = obj;

// m_MyObject is rooted and will live as long as ```foo``` lives.

[...]
delete foo; /* m_MyObject will be deleted along with our object, and m_MyObject will thus be unrooted */
```
The guideline from Mozilla, is to use this only when no other choice are possible. Also, Nidium often use the model where we delete the C++ object when the JSObject become unreachable. We would have a circular dependency issue in that case.

### Use of JS_SetReservedSlot

*TODO*

### Use a custom tracer

This is the most interesting way to root things along with JS::Heap.
Nidium already provides a convenient way to root a JSObject : ```NativeJS::rootObjectUntilShutdown(JSObject *obj)```.

This will keep the object alive until you either call ```NativeJS::unrootObject(JSObject *obj)```, or until Nidium is closed or refreshed.

It internally uses a custom tracer using ```JS_AddExtraGCRootsTracer()``` and a APE hashtable. That is, the address of the given JSObject is used as an Hashtable key. Every time the GC runs, it calls the callback given to ```JS_AddExtraGCRootsTracer``` where Nidium iterate through the Hashtable ans says *this should not be free'd* using ```JS_CallObjectTracer()``` on the stored address (given by ```NativeJS::rootObjectUntilShutdown```).
In that case, you can also instruct JSAPI to call a custom function given by the JSClass of that JSObject and trace dependents JS::Value or JSObject of that JSObject.

This is very useful when you have an internal object graph. Nidium has an example of this with ```NativeCanvasHandler```. The main canvas (the root canvas named document) is rooted using ```NativeJS::rootObjectUntilShutdown()```. The user can create new canvas and add them to the tree (```Canvas.add(child)```). A canvas can have multiple children, which in turn can have multiple children and so on.  
A user can move a canvas from one parent to another, remove it, etc... It would be cumbersome to manage the rooting/unrooting of each canvas and their subtree (e.g. you want to unroot each sub canvas of the hierarchy whenever a parent got detached from its own parent).

```
                +---------------+
                |               |
                |  Root canvas  +----> NativeJS::rootObjectUntilShutdown(rootCanvas);
                |               |
                ++-------------++
                 |             |
                 |             |
                 |             |
   +-------------v-+      +----v----------+
   |               |      |               |
   |    Child A    |      |    Child B    |
   |               |      |               |
   +------+--------+      +----------+----+
          |                          |
          |                          |
+---------v-----+                +---v-----------+
|               |                |               |
|    Child C    |                |    Child D    |
|               |                |               |
+---------------+                +---------------+

```

Every children (A, B, C, D) are reachable through the root canvas.  
Whenever the GC kicks in, it calls our custom tracer : [link](https://github.com/nidium/NativeJSCore/blob/de69f3dbb31b55f722cc5c0e040aed907145b2a5/NativeJS.cpp#L462-L483)

The consequence to this, is that JSAPI will not collect this object and call a callback we've defined in the JSClass definition : [link](https://github.com/nidium/NativeStudio/blob/c4078b59de26bc317b5b2251512f696eb199037e/src/NativeJSCanvas.cpp#L110)
where we in turns iterate over the children of this canvas, and tell the GC to not collect them, etc...
[link](https://github.com/nidium/NativeStudio/blob/b4f00377ae56d5a4b5b820a3a75cf651a07ef61a/src/NativeJSCanvas.cpp#L1580-L1593)

So what happen if we break a link in the object graph?

```
                +---------------+
                |               |
                |  Root canvas  +----> NativeJS::rootObjectUntilShutdown(rootCanvas);
                |               |
                ++--------------+
                 |
                 |
                 |                  +--------------->No parent (orphaned)
   +-------------v-+                |
   |               |           +----+----------+
   |    Child A    |           |               |
   |               |           |    Child B    |
   +------+--------+           |               |
          |                    +----------+----+
          |                               |
+---------v-----+                         |
|               |                     +---v-----------+
|    Child C    |                     |               |
|               |                     |    Child D    |
+---------------+                     |               |
                                      +---------------+
```

```Child B``` has no parent (because of a call to ```childb.removeFromParent()``` for instance).
It's no longer reachable through the Root Canvas, and will never enter our iteration. The GC will then claim the memory of ```Child B``` and then automatically all of its subtree (in that case ```Child D```).
