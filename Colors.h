#include <ostream>


namespace Color {
  enum Code {
      FG_RED      = 31,
      FG_GREEN    = 32,
      FG_BLUE     = 34,
      FG_DEFAULT  = 39,
      BG_RED      = 41,
      BG_GREEN    = 42,
      BG_BLUE     = 44,
      BG_DEFAULT  = 49
  };
  class Modifier {
      Code code;
  public:
      Modifier(Code pCode) : code(pCode) {}
      friend std::ostream&
      operator<<(std::ostream& os, const Modifier& mod) {
          return os << "\033[" << mod.code << "m";
      }
  };
}


Color::Modifier red(Color::FG_RED);
Color::Modifier def(Color::FG_DEFAULT);
Color::Modifier green(Color::FG_GREEN);
Color::Modifier blue(Color::FG_BLUE);


Color::Modifier bred(Color::BG_RED);
Color::Modifier bdef(Color::BG_DEFAULT);
Color::Modifier bgreen(Color::BG_GREEN);
Color::Modifier bblue(Color::BG_BLUE);
