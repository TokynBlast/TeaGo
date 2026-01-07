#include <filesystem>
#include <string>
#include <vector>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <variant>
#include <array>
#include <algorithm>
#include <random>
#include <cstring>

// These are the only two we use.
#define uint8 uint8_t   // numbers
#define uint64 uint64_t // position and jump num size

#define MAX_VARS 36
#define JMP_INT_SIZE uint64

namespace fs = std::filesystem;

// Wrapper for char[255] to use in variant/pair
struct str {
  char data[255];

  str() { data[0] = '\0'; }

  str(const str& s) {
    std::strcpy(data, s.data);
  }

  str(const std::string& s) {
    size_t len = s.length();
    if (len > 254) len = 254;
    std::memcpy(data, s.c_str(), len);
    data[len] = '\0';
  }

  str(const char* s) {
    size_t len = std::strlen(s);
    if (len > 254) len = 254;
    std::memcpy(data, s, len);
    data[len] = '\0';
  }

  const char* c_str() const { return data; }
  size_t length() const { return std::strlen(data); }

  void clear() { data[0] = '\0'; }
  bool empty() const { return data[0] == '\0'; }

  str& operator+=(char c) {
    size_t len = length();
    if (len < 254) {  // Leave room for null terminator
      data[len] = c;
      data[len + 1] = '\0';
    }
    return *this;
  }

  bool operator==(const std::string& s) const { return std::strcmp(data, s.c_str()) == 0; }
  bool operator==(const str& other) const { return std::strcmp(data, other.data) == 0; }
};

// Allow outputting str to std::ostream
inline std::ostream& operator<<(std::ostream& os, const str& s) {
  return os << s.data;
}

// The possible types in the language
using Var = std::variant<
    std::pair<str, str>,
    std::pair<str, uint8>>;

// The variables :D
std::array<Var, MAX_VARS> variables;

// Which ones are available :)
std::array<bool, MAX_VARS> vars;

// pointer to current posistion :3
uint64 pos = 0;

// Used as trackers for certain things
// Should use local instead of global
str str_pender;
str name;

// file contents
std::string code;

// For errors :)
inline std::string rand_chooser(std::vector<std::string> list)
{
  std::random_device dev;
  std::mt19937 randomness_generator(dev());
  std::uniform_int_distribution<std::size_t> index_distribution(0, list.size() - 1);
  auto chosen = index_distribution(randomness_generator);
  return list[chosen];
}

// Get the contents of a variable, and it's location
std::variant<bool, std::pair<uint8, str>, std::pair<uint8, uint8>>
getVar(const str &name)
{
  for (int i = 0; i < MAX_VARS; i++)
  {
    if (!vars[i])
      continue;

    // string variable
    if (std::holds_alternative<std::pair<str, str>>(variables[i]))
    {
      auto &arr = std::get<std::pair<str, str>>(variables[i]);
      if (arr.first == name)
        return std::make_pair(static_cast<uint8>(i), arr.second); // return the index and string value
    }

    // integer variable
    if (std::holds_alternative<std::pair<str, uint8>>(variables[i]))
    {
      auto &iv = std::get<std::pair<str, uint8>>(variables[i]);
      if (iv.first == name)
        return std::make_pair(static_cast<uint8>(i), iv.second); // return the index and integer value
    }
  }
  return false;
}

// Finds the lowest open slot in the array of variables
inline uint8 findFreeSlot()
{
  for (uint8 i = 0; i < MAX_VARS; i++)
  {
    if (!vars[i])
      return i;
  }
  return MAX_VARS + 1;
}

inline bool nextIs(const std::string &kw)
{
  if (code.compare(pos, kw.size(), kw) == 0)
  {
    pos += kw.size();
    return true;
  }
  return false;
}

// Skip WS
inline void skip()
{
  while (pos < code.size() && (code[pos] == ' ' || code[pos] == '\n'))
  {
    pos++;
  }
}

// Turn a string into an int
inline std::variant<bool, uint8> toInt(const str &stri)
{
  try
  {
    size_t idx = 0;
    int num = std::stoi(stri.c_str(), &idx);
    // Check if entire string was consumed
    if (idx == stri.length())
    {
      return static_cast<uint8>(num);
    }
  }
  catch (...)
  {
    // Not a valid integer
  }
  return false;
}

// Main code loop
int RunCode()
{
  while (pos < code.size())
  {
    skip();
    // Makes variables
    if (nextIs("teach"))
    {
      name.clear();
      str_pender.clear();
      skip();
      while (pos < code.size() && code[pos] != ' ' && code[pos] != '\n')
      {
        name += code[pos];
        pos++;
      }

      // Check if variable already exists (for reassignment)
      std::variant<bool, std::pair<uint8, str>, std::pair<uint8, uint8>> spot = getVar(name);
      int slot = -1;

      if (!std::holds_alternative<bool>(spot))
      {
        // Variable exists - get its slot for reassignment (type-unsafe!)
        if (std::holds_alternative<std::pair<uint8, str>>(spot))
        {
          slot = std::get<std::pair<uint8, str>>(spot).first;
        }
        else if (std::holds_alternative<std::pair<uint8, uint8>>(spot))
        {
          slot = std::get<std::pair<uint8, uint8>>(spot).first;
        }
      }
      else
      {
        // Variable doesn't exist - find a new slot
        slot = findFreeSlot();
        if (slot == MAX_VARS + 1)
        {
          std::cerr << rand_chooser(std::vector<std::string>{"GO TO SRO NOW! TOO MANY VARIABLES!", "That's too many variables. Too bad this ain't 1976...", "Out of my classroom. Now."});
          break;
        }
      }

      skip();
      if (nextIs("\""))
      {
        while (pos < code.size() && code[pos] != '\"')
        {
          str_pender += code[pos];
          pos++;
        }
        vars[slot] = true;
        variables[slot] = std::make_pair(name, str_pender);
        pos++;
      }
      else
      {
        while (pos < code.size() && std::isdigit(code[pos]))
        {
          str_pender += code[pos];
          pos++;
        }
        if (str_pender.empty())
        {
          str_pender = "0";
        }
        uint8 number = static_cast<uint8>(std::stoi(str_pender.c_str()));
        vars[slot] = true;
        variables[slot] = std::make_pair(name, number);
      }
      continue;
    }
    else if (nextIs("sro"))
    {
      name.clear();
      skip();
      while (pos < code.size() && code[pos] != ' ' && code[pos] != '\n')
      {
        name += code[pos];
        pos++;
      }
      std::variant<bool, std::pair<uint8, str>, std::pair<uint8, uint8>> spot = getVar(name);
      if (std::holds_alternative<std::pair<uint8, str>>(spot))
      {
        uint8 idx = std::get<std::pair<uint8, str>>(spot).first;
        vars[idx] = false;
        // Clear the name so it can't be accessed
        std::get<std::pair<str, str>>(variables[idx]).first = str("");
      }
      else if (std::holds_alternative<std::pair<uint8, uint8>>(spot))
      {
        uint8 idx = std::get<std::pair<uint8, uint8>>(spot).first;
        vars[idx] = false;
        // Clear the name so it can't be accessed
        std::get<std::pair<str, uint8>>(variables[idx]).first = str("");
      }
      continue;
    }
    else if (nextIs("write"))
    {
      skip();
      name.clear();
      if (nextIs("\""))
      {
        while (pos < code.size() && code[pos] != '\"')
        {
          if (code[pos] == '\\' && pos + 1 < code.size())
          {
            pos++;
            if (code[pos] == '\"')
            {
              name += '\"';
            }
            else if (code[pos] == 'n')
            {
              name += '\n';
            }
            else
            {
              name += code[pos];
            }
          }
          else
          {
            name += code[pos];
          }
          pos++;
        }
        std::cout << name;
        pos++;
      }
      else
      {
        while (pos < code.size() && code[pos] != ' ' && code[pos] != '\n')
        {
          name += code[pos];
          pos++;
        }
        std::variant<bool, std::pair<uint8, str>, std::pair<uint8, uint8>> varPrint = getVar(name);
        if (!std::holds_alternative<bool>(varPrint))
        {
          if (std::holds_alternative<std::pair<uint8, str>>(varPrint))
          {
            std::cout << std::get<std::pair<uint8, str>>(varPrint).second.data;
          }
          else if (std::holds_alternative<std::pair<uint8, uint8>>(varPrint))
          {
            std::cout << static_cast<int>(std::get<std::pair<uint8, uint8>>(varPrint).second);
          }
        }
        else
        {
          std::cerr << rand_chooser(std::vector<std::string>{"The apple doesn't fall far from the tree...", "Why did you do that??"});
          break;
        }
      }
      continue;
    }
    else if (nextIs("do"))
    {
      skip();
      name.clear();
      while (pos < code.size() && std::isdigit(code[pos]))
      {
        name += code[pos];
        pos++;
      }
      JMP_INT_SIZE jmpCount = name.empty() ? 1 : static_cast<JMP_INT_SIZE>(std::stoi(name.c_str()));
      skip();
      JMP_INT_SIZE jmpStart = pos;

      // Find matching "stop"
      JMP_INT_SIZE jmpEnd = pos;
      while (jmpEnd < code.size() && code.compare(jmpEnd, 4, "stop") != 0)
      {
        jmpEnd++;
      }
      if (jmpEnd >= code.size())
      {
        std::cerr << rand_chooser(std::vector<std::string>{"Really? Couldn't end the the do? ;-;\n", "This isn't a Mountain Dew commercial, you don't have to do the dew dude.\n"});
        break;
      }

      // Execute loop jmpCount times
      for (JMP_INT_SIZE i = 0; i < jmpCount; i++)
      {
        pos = jmpStart;

        // Process commands until we hit "stop"
        while (pos < jmpEnd)
        {
          skip();
          if (pos >= jmpEnd)
            break;

          // Process the main commands inline
          if (nextIs("teach"))
          {
            name.clear();
            str_pender.clear();
            skip();
            while (pos < code.size() && code[pos] != ' ' && code[pos] != '\n')
            {
              name += code[pos];
              pos++;
            }

            std::variant<bool, std::pair<uint8, str>, std::pair<uint8, uint8>> spot = getVar(name);
            int slot = -1;

            if (!std::holds_alternative<bool>(spot))
            {
              if (std::holds_alternative<std::pair<uint8, str>>(spot))
              {
                slot = std::get<std::pair<uint8, str>>(spot).first;
              }
              else if (std::holds_alternative<std::pair<uint8, uint8>>(spot))
              {
                slot = std::get<std::pair<uint8, uint8>>(spot).first;
              }
            }
            else
            {
              slot = findFreeSlot();
              if (slot == MAX_VARS + 1)
              {
                std::cerr << rand_chooser(std::vector<std::string>{"GO TO SRO NOW! TOO MANY VARIABLES!", "That's too many variables. Too bad this ain't 1976...", "Out of my classroom. Now."});
                return 1;
              }
            }

            skip();
            if (nextIs("\""))
            {
              str_pender.clear();
              while (pos < code.size() && code[pos] != '\"')
              {
                str_pender += code[pos];
                pos++;
              }
              vars[slot] = true;
              variables[slot] = std::make_pair(name, str_pender);
              pos++;
            }
            else
            {
              str_pender.clear();
              while (pos < code.size() && std::isdigit(code[pos]))
              {
                str_pender += code[pos];
                pos++;
              }
              if (str_pender.empty())
              {
                str_pender = "0";
              }
              uint8 number = static_cast<uint8>(std::stoi(str_pender.c_str()));
              vars[slot] = true;
              variables[slot] = std::make_pair(name, number);
            }
          }
          else if (nextIs("write"))
          {
            skip();
            name.clear();
            if (nextIs("\""))
            {
              while (pos < code.size() && code[pos] != '\"')
              {
                if (code[pos] == '\\' && pos + 1 < code.size())
                {
                  pos++;
                  if (code[pos] == '\"')
                  {
                    name += '\"';
                  }
                  else if (code[pos] == 'n')
                  {
                    name += '\n';
                  }
                  else
                  {
                    name += code[pos];
                  }
                }
                else
                {
                  name += code[pos];
                }
                pos++;
              }
              std::cout << name;
              pos++;
            }
            else
            {
              while (pos < code.size() && code[pos] != ' ' && code[pos] != '\n')
              {
                name += code[pos];
                pos++;
              }
              std::variant<bool, std::pair<uint8, str>, std::pair<uint8, uint8>> varPrint = getVar(name);
              if (!std::holds_alternative<bool>(varPrint))
              {
                if (std::holds_alternative<std::pair<uint8, str>>(varPrint))
                {
                  std::cout << std::get<std::pair<uint8, str>>(varPrint).second.data;
                }
                else if (std::holds_alternative<std::pair<uint8, uint8>>(varPrint))
                {
                  std::cout << static_cast<int>(std::get<std::pair<uint8, uint8>>(varPrint).second);
                }
              }
              else
              {
                std::cerr << rand_chooser(std::vector<std::string>{"The apple doesn't fall far from the tree...", "Why did you do that??"});
                return 1;
              }
            }
          }
          else
          {
            break; // Unknown command or end of loop
          }
        }
      }

      // Skip past "stop"
      pos = jmpEnd + 4;
      continue;
    }
    else if (nextIs("end"))
    {
      break;
    }
    else
    {
      std::cerr << "Unknown teacher lingo...";
      std::exit(1);
      break;
    }
  }
  return 0;
}

int main(int argc, char *argv[])
{
  if (argc < 2)
  {
    std::cerr << "No classroom provided (no file)";
    return 1;
  }

  std::ifstream f(argv[1]);
  if (!f.is_open())
  {
    std::cerr << "Could not open door to classroom (file)";
    return 1;
  }

  code.assign(
      (std::istreambuf_iterator<char>(f)),
      std::istreambuf_iterator<char>());

  // Set all variables as open :)
  vars.fill(false);
  RunCode();
}
