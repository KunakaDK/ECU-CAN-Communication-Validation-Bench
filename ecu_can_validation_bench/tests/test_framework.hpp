#pragma once

#include <exception>
#include <functional>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace testfw {

struct TestCase {
    std::string name;
    std::function<void()> function;
};

inline std::vector<TestCase>& registry() {
    static std::vector<TestCase> cases;
    return cases;
}

class Registrar {
public:
    Registrar(std::string name, std::function<void()> function) {
        registry().push_back(TestCase{std::move(name), std::move(function)});
    }
};

inline void require(bool condition, const char* expression,
                    const char* file, int line) {
    if (!condition) {
        std::ostringstream message;
        message << file << ':' << line << ": requirement failed: " << expression;
        throw std::runtime_error(message.str());
    }
}

inline int run_all() {
    std::size_t passed = 0;
    std::size_t failed = 0;
    for (const auto& test : registry()) {
        try {
            test.function();
            ++passed;
            std::cout << "[PASS] " << test.name << '\n';
        } catch (const std::exception& error) {
            ++failed;
            std::cerr << "[FAIL] " << test.name << " — " << error.what() << '\n';
        } catch (...) {
            ++failed;
            std::cerr << "[FAIL] " << test.name << " — unknown exception\n";
        }
    }
    std::cout << "\nTest summary: " << passed << " passed, " << failed << " failed, "
              << registry().size() << " total\n";
    return failed == 0U ? 0 : 1;
}

} // namespace testfw

#define TEST_CASE(name) \
    static void name(); \
    static const testfw::Registrar name##_registrar{#name, name}; \
    static void name()

#define REQUIRE(expression) \
    testfw::require(static_cast<bool>(expression), #expression, __FILE__, __LINE__)
