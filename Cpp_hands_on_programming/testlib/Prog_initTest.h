#ifndef PROG_INIT_TEST
#define PROG_INIT_TEST

#include <gtest/gtest.h>
#include <iostream>
#include "lib/other/Prog_init.h"

class Prog_initTest : public::testing::Test {
protected:
    Prog_init init;
    
    static void SetUpTestSuite() {
        std::cout << "START PROG_INIT TESTS" << std::endl;
    }

    static void TearDownTestSuite() {
        std::cout << "END OF PROG_INIT TESTS" << std::endl;
    }
};

class Prog_initParamTest : public Prog_initTest, public ::testing::WithParamInterface<std::tuple<
std::pair<const std::string, Methods>,  
std::pair<const std::string, std::vector<std::string>>>>{};

#endif