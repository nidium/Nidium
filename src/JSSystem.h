#ifndef serverjssystem_h__
#define serverjssystem_h__

#include <Binding/JSExposer.h>

namespace Nidium {
namespace Server {

class JSSystem : public Nidium::Binding::JSExposer<JSSystem>
{
  public:
    static void registerObject(JSContext *cx);
};

} // namespace Server
} // namespce Nidium

#endif

