#ifndef binding_jssystem_h__
#define binding_jssystem_h__

#include <Binding/JSExposer.h>

using Nidium::Binding::JSExposer;

namespace Nidium {
namespace Server {

class JSSystem : public JSExposer<JSSystem>
{
  public:
    static void RegisterObject(JSContext *cx);
};

} // namespace Server
} // namespce Nidium

#endif

