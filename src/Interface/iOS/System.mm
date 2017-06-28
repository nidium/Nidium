/*
   Copyright 2017 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include "System.h"
#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

namespace Nidium {
namespace Interface {

System::System()
{
    NSString *embedPath = [[NSBundle mainBundle] pathForResource:@"Embed" ofType:nil];
    NSURL *url = [NSURL fileURLWithPath:embedPath];
    const char *path = [url.absoluteString UTF8String];

    // XXX: Should I free something here ?

    m_EmbedPath = strdup(&path[7]);

    m_BackingStorePixelRatio = [[UIScreen mainScreen] scale];
}

} // namespace Interface
} // namespace Nidium
