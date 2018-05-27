/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef interface_sdlkeycodetranslate_h__
#define interface_sdlkeycodetranslate_h__

#include <stdint.h>

#define SDL_KEYCODE_TO_SCANCODE(X) (X & ~(1 << 30))

/*
    Store a "magic bit" that define the key location (e.g. left alt or right
   alt)
*/
#define L1(X) (X | (1 << 24))
#define L2(X) (X | (2 << 24))


namespace Nidium {
namespace Interface {

static const int32_t SDL_TO_DOM_KEYCODES[512] = {

    // Block Size is 8x4 == 32 keycodes

    // XXX: Currently broken-by-concept
    // index 58 is ; is 186
    // index 60 is = is 187
    // index 46 is / is 191




      0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,

      0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0, 188, 189,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,
      0, 112, 113, 114, 115, 116, 117, 118,

    119, 120, 121, 122, 123,   0,   0,   0,
     45,   0,   0, 188, 189, 190,  39,  37,
     40,  38,   0,   0,   0,   0,   0,  13,
      0,   0, 219, 220, 221,   0,   0, 192,

      0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,  46,   0,

      0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,

      0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,

      0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0, L1(17),

      L1(16), L1(18), L1(91), L2(17), L2(16), L2(18), L2(91), 0,
      0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,

      0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,

      0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,

      0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,

      0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,

      0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,

      0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,

      0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,

      0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0

};

#if 1
#define SDL_KEYCODE_TO_DOMCODE(X)                              \
    (SDL_TO_DOM_KEYCODES[SDL_KEYCODE_TO_SCANCODE(X) - 1]       \
         ? SDL_TO_DOM_KEYCODES[SDL_KEYCODE_TO_SCANCODE(X) - 1] \
         : SDL_KEYCODE_TO_SCANCODE(X))
#else
#define SDL_KEYCODE_TO_DOMCODE(X) (X)
#endif

/* Get the location bit value */
#define SDL_KEYCODE_GET_LOCATION(X) (X >> 24)

/* Strip the location bit */
#define SDL_KEYCODE_GET_CODE(X) (X & 0x00FFFFFF)

#undef L1
#undef L2

} // namespace Interface
} // namespace Nidium

#endif
