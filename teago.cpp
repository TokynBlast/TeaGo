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

#define uint8 uint8_t
#define uint64 uint64_t

#define MAX_VARS 36
#define JMP_INT_SIZE uint64

namespace fs = std::filesystem;

using Var = std::variant<
    std::pair<std::string, std::string>,
    std::pair<uint8, uint8>
>;

std::array<Var, MAX_VARS> variables;

uint8 num_vars = 0;
std::array<bool, MAX_VARS> vars;
uint64 pos = 0;
std::string str_pender;
std::string name;

std::string code;

inline std::string rand_chooser(std::vector<std::string> list) {
  std::random_device dev;
  std::mt19937 randomness_generator(dev());
  std::uniform_int_distribution<std::size_t> index_distribution(0, list.size() - 1);
  auto chosen = index_distribution(randomness_generator);
  return list[chosen];
}

std::variant<bool, std::pair<uint8, std::string>, std::pair<uint8, uint8>> getVar(const std::string& name) {
  for (int i = 0; i < MAX_VARS; i++)
  {
    if (!vars[i])
      continue;

    // string variable
    if (std::holds_alternative<std::array<std::string, 2>>(variables[i]))
    {
      auto &arr = std::get<std::array<std::string, 2>>(variables[i]);
      if (arr[0] == name)
        return std::make_pair(static_cast<uint8>(i), arr[1]); // return the index and string value
    }

    // integer variable
    if (std::holds_alternative<integers>(variables[i]))
    {
      auto &iv = std::get<integers>(variables[i]);
      if (iv.value == name)
        return std::make_pair(static_cast<uint8>(i), iv.number); // return the index and integer value
    }
  }

  return false;
}

inline uint8 findFreeSlot() {
  for (uint8 i = 0; i < MAX_VARS; i++) {
    if (!vars[i]) return i;
  }
  return MAX_VARS + 1;
}

inline bool nextIs(const std::string& kw) {
  if(code.compare(pos, kw.size(), kw) == 0) {
    pos += kw.size();
    return true;
  }
  return false;
}

inline void skip() {
  while (pos < code.size() && (code[pos] == ' ' || code[pos] == '\n')) {
    pos++;
  }
}

inline std::variant<bool, uint8> toInt(const std::string& str) {
  try {
    size_t idx = 0;
    int num = std::stoi(str, &idx);
    // Check if entire string was consumed
    if (idx == str.length()) {
      return static_cast<uint8>(num);
    }
  } catch (...) {
    // Not a valid integer
  }
  return false;
}

int RunCode() {
  while (pos < code.size())
  {
    skip();
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
      std::variant<bool, std::pair<uint8, std::string>, std::pair<uint8, uint8>> spot = getVar(name);
      int slot = -1;

      if (!std::holds_alternative<bool>(spot))
      {
        // Variable exists - get its slot for reassignment (type-unsafe!)
        if (std::holds_alternative<std::pair<uint8, std::string>>(spot))
        {
          slot = std::get<std::pair<uint8, std::string>>(spot).first;
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
        num_vars++;
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
        variables[slot] = std::array<std::string, 2>{name, str_pender};
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
        uint8 number = static_cast<uint8>(std::stoi(str_pender));
        vars[slot] = true;
        variables[slot] = integers{name, number};
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
      std::variant<bool, std::pair<uint8, std::string>, std::pair<uint8, uint8>> spot = getVar(name);
      if (std::holds_alternative<std::pair<uint8, std::string>>(spot))
      {
        vars[std::get<std::pair<uint8, std::string>>(spot).first] = false;
      }
      else if (std::holds_alternative<std::pair<uint8, uint8>>(spot))
      {
        vars[std::get<std::pair<uint8, uint8>>(spot).first] = false;
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
        std::variant<bool, std::pair<uint8, std::string>, std::pair<uint8, uint8>> varPrint = getVar(name);
        if (!std::holds_alternative<bool>(varPrint))
        {
          if (std::holds_alternative<std::pair<uint8, std::string>>(varPrint))
          {
            std::cout << std::get<std::pair<uint8, std::string>>(varPrint).second;
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
      JMP_INT_SIZE jmpCount = name.empty() ? 1 : static_cast<JMP_INT_SIZE>(std::stoi(name));
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
          if (pos >= jmpEnd) break;

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

            std::variant<bool, std::pair<uint8, std::string>, std::pair<uint8, uint8>> spot = getVar(name);
            int slot = -1;

            if (!std::holds_alternative<bool>(spot))
            {
              if (std::holds_alternative<std::pair<uint8, std::string>>(spot))
              {
                slot = std::get<std::pair<uint8, std::string>>(spot).first;
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
              num_vars++;
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
              variables[slot] = std::array<std::string, 2>{name, str_pender};
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
              uint8 number = static_cast<uint8>(std::stoi(str_pender));
              vars[slot] = true;
              variables[slot] = integers{name, number};
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
              std::variant<bool, std::pair<uint8, std::string>, std::pair<uint8, uint8>> varPrint = getVar(name);
              if (!std::holds_alternative<bool>(varPrint))
              {
                if (std::holds_alternative<std::pair<uint8, std::string>>(varPrint))
                {
                  std::cout << std::get<std::pair<uint8, std::string>>(varPrint).second;
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
            break;  // Unknown command or end of loop
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

int main(int argc, char* argv[]) {
  if (argc < 2) {
    std::cerr << "No classroom provided (no file)";
    return 1;
  }

  std::ifstream f(argv[1]);
  if (!f.is_open()) {
    std::cerr << "Could not open door to classroom (file)";
    return 1;
  }

  code.assign(
    (std::istreambuf_iterator<char>(f)),
    std::istreambuf_iterator<char>()
  );

  // Set all variables as open :)
  vars.fill(false);
  RunCode();
}


