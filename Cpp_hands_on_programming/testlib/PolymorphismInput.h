#ifndef TEST_HELPER
#define TEST_HELPER

#include <gtest/gtest.h>
#include <iostream>
#include "lib/abstract/Ipc_method.h"


class  PolymorphismInput : public::testing::TestWithParam<std::pair<const std::string, std::shared_ptr<Ipc_method>>> {
protected:
    static const off_t bigSize = 1000*1000*1000;
    static const off_t smallSize = 10000;
    static void SetUpTestSuite() {
        generateFile("test2", smallSize);
        generateFile("bigfile", bigSize);
    }

    static void generateFile(std::string name, off_t size) {
        std::ofstream out(name);
        out.seekp(size - 3);
        out << "end";
        out.close();
    }
    
    static void TearDownTestSuite() {
        unlink("test2");
        unlink("bigfile");
    }
};

#endif