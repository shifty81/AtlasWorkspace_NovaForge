#include <catch2/catch_test_macros.hpp>
#include "NF/GraphVM/GraphVM.h"

TEST_CASE("GraphVM initializes", "[GraphVM]") {
    NF::GraphVM vm;
    vm.init();
    REQUIRE_FALSE(vm.halted());
}

TEST_CASE("GraphVM loads and runs empty program", "[GraphVM]") {
    NF::GraphVM vm;
    vm.init();

    std::vector<NF::GraphInstruction> program;
    vm.loadProgram(program);

    vm.step();
    REQUIRE(vm.halted()); // empty program halts immediately
}

TEST_CASE("GraphVM steps through program", "[GraphVM]") {
    NF::GraphVM vm;
    vm.init();

    std::vector<NF::GraphInstruction> program = {
        {NF::GraphOpCode::Nop, 0, 0, 0},
        {NF::GraphOpCode::Nop, 0, 0, 0},
        {NF::GraphOpCode::Nop, 0, 0, 0}
    };
    vm.loadProgram(program);

    vm.step(); // Nop #1
    REQUIRE_FALSE(vm.halted());
    vm.step(); // Nop #2
    REQUIRE_FALSE(vm.halted());
    vm.step(); // Nop #3 — now at end
    REQUIRE_FALSE(vm.halted());
    vm.step(); // past end — halts
    REQUIRE(vm.halted());
}

TEST_CASE("GraphType enum covers 14 types", "[GraphVM]") {
    // Verify all 14 graph types are defined
    REQUIRE(static_cast<uint8_t>(NF::GraphType::World) == 0);
    REQUIRE(static_cast<uint8_t>(NF::GraphType::Asset) == 13);
}
