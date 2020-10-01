struct Namco163 : Board {
  Namco163(Markup::Node& document) : Board(document), n163(*this, document) {
  }

  auto main() -> void {
    n163.main();
  }

  auto readPRG(uint addr) -> uint8 {
    return n163.readPRG(addr);
  }

  auto writePRG(uint addr, uint8 data) -> void {
    n163.writePRG(addr, data);
  }

  auto readCHR(uint addr) -> uint8 {
    return n163.readCHR(addr);
  }

  auto writeCHR(uint addr, uint8 data) -> void {
    n163.writeCHR(addr, data);
  }

  auto serialize(serializer& s) -> void {
    Board::serialize(s);
    n163.serialize(s);
  }

  auto power(bool reset) -> void {
    n163.power(reset);
  }

  N163 n163;
};
