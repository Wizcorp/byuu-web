#include <emulator/emulator.hpp>
#include "sm83.hpp"

namespace higan {

#include "registers.cpp"
#include "memory.cpp"
#include "algorithms.cpp"
#include "instruction.cpp"
#include "instructions.cpp"
#include "serialization.cpp"

#if !defined(NO_EVENTINSTRUCTION_NOTIFY)
#include "disassembler.cpp"
#endif

auto SM83::power() -> void {
  r = {};
}

}
