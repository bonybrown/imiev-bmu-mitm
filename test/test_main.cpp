/**
 * @file test_main.cpp
 * @brief Main entry point for CppUTest unit tests
 */

#include "CppUTest/CommandLineTestRunner.h"

int main(int ac, char **av)
{
    return CommandLineTestRunner::RunAllTests(ac, av);
}
