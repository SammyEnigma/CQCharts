#include <CTclParse.h>
#include <CStrParse.h>
#include <CStrUtil.h>
#include <CFile.h>
#include <cmath>
#include <cassert>

class CTclStrParse : public CStrParse {
 public:
  CTclStrParse(CTclParse *tcl, const std::string &filename);
 ~CTclStrParse();

  bool eof() const override;

 private:
  bool fillBuffer();

 private:
  CTclParse* tcl_  { nullptr };
  CFile*     file_ { nullptr };
};

//------

CTclParse::
CTclParse()
{
}

CTclParse::
~CTclParse()
{
}

bool
CTclParse::
isCompleteLine(const std::string &line)
{
  startStringParse(line);

  bool rc = isCompleteLine1('\0');

  endParse();

  return rc;
}

bool
CTclParse::
isCompleteLine1(char endChar)
{
  while (! parse_->eof()) {
    if      (parse_->isChar('[')) {
      parse_->skipChar();

      if (! isCompleteLine1(']'))
        return false;

      if (! parse_->isChar(']'))
        return false;

      parse_->skipChar();
    }
    else if (parse_->isChar('{')) {
      parse_->skipChar();

      if (! isCompleteLine1('}'))
        return false;

      if (! parse_->isChar('}'))
        return false;

      parse_->skipChar();
    }
    else if (parse_->isChar(endChar)) {
      return true;
    }
    else if (parse_->isChar('\"')) {
      parse_->skipChar();

      if (! isCompleteLine1('\"'))
        return false;

      if (! parse_->isChar('\"'))
        return false;

      parse_->skipChar();
    }
    else if (parse_->isChar('\'')) {
      parse_->skipChar();

      if (! isCompleteLine1('\''))
        return false;

      if (! parse_->isChar('\''))
        return false;

      parse_->skipChar();
    }
    else if (parse_->isChar('\\')) {
      parse_->skipChar();
      parse_->skipChar();
    }
    else
      parse_->skipChar();
  }

  return true;
}

bool
CTclParse::
parseFile(const std::string &filename, Tokens &tokens)
{
  if (! CFile::isRegular(filename)) {
    std::cerr << "Invalid file " << filename << std::endl;
    return false;
  }

  startFileParse(filename);

  bool rc = true;

  while (! parse_->eof()) {
    Tokens tokens1;

    if (! readArgList(tokens1)) {
      rc = false;
      break;
    }

    for (auto &token1 : tokens1)
      tokens.push_back(token1);
  }

  endParse();

  return rc;
}

bool
CTclParse::
parseLine(const std::string &str, Tokens &tokens)
{
  return parseString(str, tokens);
}

bool
CTclParse::
parseString(const std::string &str, Tokens &tokens)
{
  startStringParse(str);

  bool rc = true;

  while (! parse_->eof()) {
    Tokens tokens1;

    if (! readArgList(tokens1)) {
      rc = false;
      break;
    }

    for (auto &token1 : tokens1)
      tokens.push_back(token1);
  }

  endParse();

  return rc;
}

bool
CTclParse::
readArgList(Tokens &tokens)
{
  while (! parse_->eof()) {
    while (parse_->isChar(' ') || parse_->isChar('\t'))
      parse_->skipChar();

    if (parse_->eof())
      return true;

    if      (parse_->isChar(';') || parse_->isChar('\n')) {
      parse_->skipChar();

      return true;
    }
    else if (parse_->isChar('[')) {
      Tokens tokens1;

      int pos = parse_->getPos();

      if (! readExecString(tokens1))
        return false;

      auto str = parse_->getBefore(pos);

      auto *token = new CTclToken(CTclToken::Type::SUB_COMMAND, str, pos);

      if (tokens1.size() > 0)
        tokens1[0]->setType(CTclToken::Type::COMMAND);

      for (const auto &token1 : tokens1)
        token->addToken(token1);

      tokens.push_back(token);
    }
    else if (parse_->isChar(']')) {
      return true;
    }
    else if (parse_->isChar('{')) {
      std::string str;

      int pos = parse_->getPos();

      Tokens tokens1;

      if (! readLiteralString(str, tokens1))
        return false;

      str = parse_->getBefore(pos);

      auto *token = new CTclToken(CTclToken::Type::LITERAL_STRING, str, pos);

      for (const auto &token1 : tokens1)
        token->addToken(token1);

      tokens.push_back(token);
    }
    else if (parse_->isChar('\"')) {
      std::string str;

      int pos = parse_->getPos();

      Tokens tokens1;

      if (! readDoubleQuotedString(str, tokens1))
        return false;

      str = parse_->getBefore(pos);

      auto *token = new CTclToken(CTclToken::Type::QUOTED_STRING, str, pos);

      for (const auto &token1 : tokens1)
        token->addToken(token1);

      tokens.push_back(token);
    }
    else if (parse_->isChar('\'')) {
      std::string str;

      int pos = parse_->getPos();

      if (! readSingleQuotedString(str))
        return false;

      tokens.push_back(new CTclToken(CTclToken::Type::QUOTED_STRING, str, pos));
    }
    else if (parse_->isChar('$')) {
      parse_->skipChar();

      std::string varName;
      bool        is_array;

      int pos = parse_->getPos();

      Tokens tokens1;

      if (! readVariableName(varName, is_array, tokens1))
        return false;

      auto str = parse_->getBefore(pos);

      auto *token = new CTclToken(CTclToken::Type::VARIABLE, str, pos);

      for (const auto &token1 : tokens1)
        token->addToken(token1);

      tokens.push_back(token);

      if (! parse_->isSpace()) {
        int pos1 = parse_->getPos();

        std::string str2;

        Tokens tokens1;

        if (! readWord(str2, ';', tokens1))
          return false;

        str2 = parse_->getBefore(pos);

        auto *token = new CTclToken(CTclToken::Type::STRING, str2, pos);

        for (const auto &token1 : tokens1)
          token->addToken(token1);

        tokens.push_back(token);
      }
    }
    else {
      std::string str;

      int pos = parse_->getPos();

      Tokens tokens1;

      if (! readWord(str, ';', tokens1))
        return false;

      str = parse_->getBefore(pos);

      auto *token = new CTclToken(CTclToken::Type::STRING, str, pos);

      if (tokens.empty())
        token->setType(CTclToken::Type::COMMAND);

      for (const auto &token1 : tokens1)
        token->addToken(token1);

      tokens.push_back(token);
    }
  }

  return true;
}

bool
CTclParse::
readExecString(Tokens &tokens)
{
  assert(parse_->isChar('['));

  parse_->skipChar();

  parse_->skipSpace();

  while (! parse_->isChar(']')) {
    if      (parse_->isChar('{')) {
      std::string str1;

      int pos = parse_->getPos();

      Tokens tokens1;

      if (! readLiteralString(str1, tokens1))
        return false;

      str1 = parse_->getBefore(pos);

      auto *token = new CTclToken(CTclToken::Type::LITERAL_STRING, str1, pos);

      for (const auto &token1 : tokens1)
        token->addToken(token1);

      tokens.push_back(token);
    }
    else if (parse_->isChar('\"')) {
      std::string str1;

      int pos = parse_->getPos();

      Tokens tokens1;

      if (! readDoubleQuotedString(str1, tokens1))
        return false;

      str1 = parse_->getBefore(pos);

      auto *token = new CTclToken(CTclToken::Type::QUOTED_STRING, str1, pos);

      for (const auto &token1 : tokens1)
        token->addToken(token1);

      tokens.push_back(token);
    }
    else if (parse_->isChar('\'')) {
      std::string str1;

      int pos = parse_->getPos();

      if (! readSingleQuotedString(str1))
        return false;

      tokens.push_back(new CTclToken(CTclToken::Type::QUOTED_STRING, str1, pos));
    }
    else {
      std::string str;

      int pos = parse_->getPos();

      Tokens tokens1;

      if (! readWord(str, ']', tokens1))
        return false;

      str = parse_->getBefore(pos);

      auto *token = new CTclToken(CTclToken::Type::STRING, str, pos);

      if (tokens.empty())
        token->setType(CTclToken::Type::COMMAND);

      for (const auto &token1 : tokens1)
        token->addToken(token1);

      tokens.push_back(token);
    }

    parse_->skipSpace();
  }

  parse_->skipChar();

  return true;
}

bool
CTclParse::
readLiteralString(std::string &str, Tokens &tokens)
{
  assert(parse_->isChar('{'));

  SetSeparator sepSep(this, '\n');

  parse_->skipChar();

  while (! parse_->eof() && ! parse_->isChar('}')) {
    if (parse_->isChar('{')) {
      std::string str1;

      Tokens tokens1;

      if (! readLiteralString(str1, tokens1))
        return false;

      str += "{" + str1 + "}";
    }
    else {
      char c;

      if (! parse_->readChar(&c)) {
        std::cerr << "Unterminated string" << std::endl;
        return false;
      }

      str += c;
    }
  }

  if (! parse_->isChar('}')) {
    std::cerr << "Unterminated string" << std::endl;
    return false;
  }

  parse_->skipChar();

  return true;
}

bool
CTclParse::
readDoubleQuotedString(std::string &str, Tokens &tokens)
{
  assert(parse_->isChar('\"'));

  parse_->skipChar();

  while (! parse_->isChar('\"')) {
    if      (parse_->isChar('[')) {
      Tokens tokens1;

      int pos = parse_->getPos();

      if (! readExecString(tokens1))
        return false;

      auto str = parse_->getBefore(pos);

      auto *token = new CTclToken(CTclToken::Type::SUB_COMMAND, str, pos);

      if (tokens1.size() > 0)
        tokens1[0]->setType(CTclToken::Type::COMMAND);

      for (const auto &token1 : tokens1)
        token->addToken(token1);

      tokens.push_back(token);
    }
    else if (parse_->isChar('$')) {
      parse_->skipChar();

      std::string varName;
      bool        is_array;

      int pos = parse_->getPos();

      Tokens tokens1;

      if (! readVariableName(varName, is_array, tokens1))
        return false;

      auto str = parse_->getBefore(pos);

      auto *token = new CTclToken(CTclToken::Type::VARIABLE, str, pos);

      for (const auto &token1 : tokens1)
        token->addToken(token1);

      tokens.push_back(token);
    }
    else if (parse_->isChar('\\')) {
      parse_->skipChar();

      char c;

      if (! parse_->readChar(&c)) {
        std::cerr << "Invalid char after \\" << std::endl;
        return false;
      }

      switch (c) {
        case 'a': str += '\a'; break;
        case 'b': str += '\b'; break;
        case 'f': str += '\f'; break;
        case 'n': str += '\n'; break;
        case 'r': str += '\r'; break;
        case 't': str += '\t'; break;
        case 'v': str += '\v'; break;
        default : str += c   ; break;

        // octal
        case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': {
          char value = c - '0';

          int num = 1;

          while (! parse_->eof()) {
            if (! parse_->readChar(&c)) {
              std::cerr << "Invalid octal" << std::endl;
              return false;
            }

            if (! CStrUtil::isodigit(c)) {
              parse_->unreadChar();
              break;
            }

            value = (value << 3) | (c - '0');

            ++num;

            if (num == 3) break;
          }

          str += value;

          break;
        }

        // hex
        case 'x': {
          char value = 0;

          while (! parse_->eof()) {
            if (! parse_->readChar(&c)) {
              std::cerr << "Invalid hex" << std::endl;
              return false;
            }

            if (! isxdigit(c)) {
              parse_->unreadChar();
              break;
            }

            char value1 = 0;

            if (isdigit(c))
              value1 = c - '0';
            else if (islower(c))
              value1 = c - 'a';
            else
              value1 = c - 'A';

            value = (value << 4) | value1;
          }

          str += value;

          break;
        }
      }
    }
    else {
      char c;

      if (! parse_->readChar(&c)) {
        std::cerr << "Invalid char" << std::endl;
        return false;
      }

      str += c;
    }
  }

  parse_->skipChar();

  return true;
}

bool
CTclParse::
readSingleQuotedString(std::string &str)
{
  assert(parse_->isChar('\''));

  parse_->skipChar();

  while (! parse_->isChar('\'')) {
    char c;

    if (! parse_->readChar(&c)) {
      std::cerr << "Unterminated string" << std::endl;
      return false;
    }

    str += c;
  }

  parse_->skipChar();

  return true;
}

bool
CTclParse::
readVariableName(std::string &varName, bool &is_array, Tokens &tokens)
{
  // ${name} - name can have any characters
  if (parse_->isChar('{')) {
    Tokens tokens1;

    if (! readLiteralString(varName, tokens1))
      return false;
  }
  // $name - sequence of one or more characters that are a letter, digit,
  // underscore, or namespace separators (two or more colons)
  else {
    while (! parse_->eof()) {
      char c;

      if (! parse_->readChar(&c)) {
        std::cerr << "Missing variable char" << std::endl;
        return false;
      }

      if (! isalnum(c) && c != '_' && c != ':') {
        parse_->unreadChar();
        break;
      }

      varName += c;
    }
  }

  is_array = false;

  // $name(index) - name must contain only letters, digits, underscores, and namespace
  // separators, and may be an empty string. Command substitutions, variable substitutions,
  // and backslash substitutions are performed on the characters of index.
  if (parse_->isChar('(')) {
    parse_->skipChar();

    std::string str2;

    int depth = 1;

    while (! parse_->eof()) {
      char c;

      if (! parse_->readChar(&c)) {
        std::cerr << "Missing variable char" << std::endl;
        return false;
      }

      if      (c == '(')
        ++depth;
      else if (c == ')') {
        --depth;

        if (depth == 0) break;
      }

      str2 += c;
    }

    if (depth != 0) {
      std::cerr << "Invalid () nesting" << std::endl;
      return false;
    }

    is_array = true;
  }

  return true;
}

bool
CTclParse::
readWord(std::string &str, char endChar, Tokens &tokens)
{
  assert(! parse_->isSpace());

  while (! parse_->eof()) {
    if (parse_->isSpace() || parse_->isChar(endChar))
      break;

    if      (parse_->isChar('[')) {
      Tokens tokens1;

      int pos = parse_->getPos();

      if (! readExecString(tokens1))
        return false;

      auto str = parse_->getBefore(pos);

      auto *token = new CTclToken(CTclToken::Type::SUB_COMMAND, str, pos);

      if (tokens1.size() > 0)
        tokens1[0]->setType(CTclToken::Type::COMMAND);

      for (const auto &token1 : tokens1)
        token->addToken(token1);

      tokens.push_back(token);
    }
    else if (parse_->isChar('$')) {
      parse_->skipChar();

      std::string varName;
      bool        is_array;

      int pos = parse_->getPos();

      Tokens tokens1;

      if (! readVariableName(varName, is_array, tokens1))
        return false;

      auto str = parse_->getBefore(pos);

      auto *token = new CTclToken(CTclToken::Type::VARIABLE, str, pos);

      for (const auto &token1 : tokens1)
        token->addToken(token1);

      tokens.push_back(token);
    }
    else if (parse_->isChar('\\')) {
      parse_->skipChar();

      char c;

      if (! parse_->readChar(&c)) {
        std::cerr << "Invalid char after \\" << std::endl;
        return false;
      }

      str += c;
    }
    else {
      char c;

      if (! parse_->readChar(&c)) {
        std::cerr << "Invalid char" << std::endl;
        return false;
      }

      str += c;
    }
  }

  return true;
}

void
CTclParse::
startFileParse(const std::string &fileName)
{
  parseStack_.push_back(parse_);

  parse_ = new CTclStrParse(this, fileName);
}

void
CTclParse::
startStringParse(const std::string &str)
{
  parseStack_.push_back(parse_);

  parse_ = new CStrParse(str);
}

void
CTclParse::
endParse()
{
  delete parse_;

  parse_ = parseStack_.back();

  parseStack_.pop_back();
}

CTclToken *
CTclParse::
getTokenForPos(const Tokens &tokens, int pos) const
{
  for (const auto &token : tokens) {
    if (token->hasPos(pos)) {
      auto *token1 = getTokenForPos(token->tokens(), pos);

      if (token1)
        return token1;

      return token;
    }
  }

  return nullptr;
}

bool
CTclParse::
needsBraces(const std::string &str)
{
  uint len = str.size();

  bool is_space = false;

  for (uint i = 0; i < len; ++i) {
    if (isspace(str[i])) {
      is_space = true;
      break;
    }
  }

  return (len == 0 || is_space);
}

//--------------

CTclStrParse::
CTclStrParse(CTclParse *tcl, const std::string &filename) :
 tcl_(tcl)
{
  file_ = new CFile(filename);
}

CTclStrParse::
~CTclStrParse()
{
  delete file_;
}

bool
CTclStrParse::
fillBuffer()
{
  std::string line;

  if (! file_->readLine(line))
    return false;

  uint len = line.size();

  while (len > 0 && line[len - 1] == '\\') {
    line = line.substr(0, len - 1);

    std::string line1;

    if (file_->readLine(line1)) {
      uint len1 = line1.size();

      uint i = 0;

      while (i < len1 && isspace(line1[i]))
        ++i;

      line += " " + line1.substr(0);
    }

    len = line.size();
  }

  if (getPos() > 0)
    addString(tcl_->getSeparator() + line);
  else
    addString(line);

  return true;
}

bool
CTclStrParse::
eof() const
{
  if (! CStrParse::eof())
    return false;

  CTclStrParse *th = const_cast<CTclStrParse *>(this);

  th->fillBuffer();

  if (! CStrParse::eof())
    return false;

  return file_->eof();
}
