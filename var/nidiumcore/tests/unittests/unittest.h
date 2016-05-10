/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef __TESTS_UNITTEST_H_
#define __TESTS_UNITTEST_H_

#include <gtest/gtest.h>

extern int ape_running;
extern unsigned long _ape_seed;

#define NIDIUMJS_FIXTURE(name) \
class  name: public ::testing::Test {\
protected:\
    ape_global *ape;\
    ::Nidium::Binding::Nidiumcore *njs;\
    name(){\
        ape = APE_init();\
        njs = new ::Nidium::Binding::Nidiumcore(ape);\
    };\
    ~name() {\
        delete njs;\
        APE_destroy(ape);\
    };\
};

#endif

