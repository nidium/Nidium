#ifndef binding_jsconsole_h__
#define binding_jsconsole_h__

#include <Binding/JSExposer.h>

using Nidium::Binding::NidiumJS;
using Nidium::Binding::JSExposer;

namespace Nidium {
namespace Server {

class JSconsole : public JSExposer<JSconsole>
{
  public:
    static void RegisterObject(JSContext *cx);
};

} // namespace Nidium
} // namespace Server

#endif

