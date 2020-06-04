struct Board;

struct Chip {
  Chip(Board& board);
  auto tick(uint clocks = 1) -> void;

  Board& board;
};
