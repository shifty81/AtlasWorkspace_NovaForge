#pragma once
// NF::GraphVM — Deterministic bytecode VM, compiler, serialization
#include "NF/Core/Core.h"
#include <variant>

namespace NF {

using GraphNodeID = uint32_t;

enum class GraphOpCode : uint8_t {
    Nop,
    LoadConst,
    Add,
    Subtract,
    Multiply,
    Divide,
    Compare,
    Branch,
    Call,
    Return,
    Store,
    Load,
    Halt
};

struct GraphInstruction {
    GraphOpCode opcode = GraphOpCode::Nop;
    uint32_t operandA = 0;
    uint32_t operandB = 0;
    uint32_t operandC = 0;
};

using GraphValue = std::variant<int32_t, float, bool, std::string>;

class GraphVM {
public:
    void init() { NF_LOG_INFO("GraphVM", "Graph VM initialized"); }
    void shutdown() { NF_LOG_INFO("GraphVM", "Graph VM shutdown"); }

    void loadProgram(const std::vector<GraphInstruction>& program) {
        m_program = program;
        m_pc = 0;
        m_halted = false;
    }

    void step() {
        if (m_halted || m_pc >= m_program.size()) {
            m_halted = true;
            return;
        }
        // Execute instruction at m_pc
        auto& inst = m_program[m_pc];
        (void)inst;
        m_pc++;
    }

    [[nodiscard]] bool halted() const { return m_halted; }

private:
    std::vector<GraphInstruction> m_program;
    size_t m_pc = 0;
    bool m_halted = false;
};

// ── Graph types ──────────────────────────────────────────────────

enum class GraphType : uint8_t {
    World,
    Strategy,
    Conversation,
    Behavior,
    Animation,
    Character,
    Weapon,
    Tile,
    Sound,
    UI,
    UIScreen,
    GameFlow,
    Story,
    Asset
};

} // namespace NF
