/*****************************************************************************
*
* ALPS Project Applications
*
* Copyright (C) 2026 ALPS Collaboration
*
* ALPS Project: https://alps.comp-phys.org/
* SPDX-License-Identifier: MIT
*
*****************************************************************************/

#include "maxent_parameters.hpp"

#include <algorithm>
#include <cctype>
#include <cerrno>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <limits>
#include <sstream>

namespace maxent {

namespace {

std::string strip(const std::string& s) {
  std::string::size_type b = 0, e = s.size();
  while (b < e && std::isspace(static_cast<unsigned char>(s[b]))) ++b;
  while (e > b && std::isspace(static_cast<unsigned char>(s[e - 1]))) --e;
  return s.substr(b, e - b);
}

// One logical line, following iniparser_line() in ALPSCore's bundled
// iniparser. Returns false for a syntax error.
bool parse_ini_line(const std::string& input, std::string& section, std::map<std::string, std::string>& map) {
  const std::string line = strip(input);
  if (line.empty() || line[0] == '#' || line[0] == ';')
    return true;
  if (line[0] == '[' && line.back() == ']') {
    section = strip(line.substr(1, line.find(']') - 1));
    return true;
  }
  const std::string::size_type eq = line.find('=');
  if (eq == std::string::npos || eq == 0)
    return false;
  const std::string key = strip(line.substr(0, eq));
  std::string rest = strip(line.substr(eq + 1));
  std::string value;
  if (!rest.empty() && (rest[0] == '"' || rest[0] == '\'')
      && rest.find(rest[0], 1) != std::string::npos && rest.find(rest[0], 1) > 1) {
    // quoted value: kept verbatim, anything after the closing quote is ignored
    value = rest.substr(1, rest.find(rest[0], 1) - 1);
  } else {
    // unquoted value: ends at the first comment character
    value = strip(rest.substr(0, rest.find_first_of(";#")));
    if (value == "\"\"" || value == "''")
      value.clear();
  }
  std::string full_key = section + "." + key;
  if (!full_key.empty() && full_key[0] == '.')
    full_key.erase(0, 1);
  map[full_key] = value;
  return true;
}

} // namespace

void parse_ini(std::istream& in, const std::string& source, std::map<std::string, std::string>& map) {
  std::string section, line, logical;
  int lineno = 0;
  while (std::getline(in, line)) {
    ++lineno;
    if (!line.empty() && line.back() == '\r')
      line.pop_back();
    // a trailing backslash continues the value on the next line
    std::string stripped = strip(line);
    if (!stripped.empty() && stripped.back() == '\\') {
      stripped.pop_back();
      logical += stripped;
      continue;
    }
    logical += line;
    if (!parse_ini_line(logical, section, map))
      throw std::runtime_error("syntax error in parameter file " + source + ", line "
                               + std::to_string(lineno) + ": " + logical);
    logical.clear();
  }
  if (!logical.empty() && !parse_ini_line(logical, section, map))
    throw std::runtime_error("syntax error at the end of parameter file " + source);
}

std::string remove_extensions(const std::string& filename) {
  std::string::size_type base = filename.rfind('/');
  base = (base == std::string::npos) ? 0 : base + 1;
  if (base == filename.size())
    return filename;
  if (filename.compare(base, std::string::npos, ".") == 0 || filename.compare(base, std::string::npos, "..") == 0)
    return filename;
  if (filename.compare(base, 3, "...") == 0)
    return filename.substr(0, base + 2);
  if (filename.compare(base, 2, "..") == 0)
    return filename.substr(0, base + 1);
  return filename.substr(0, filename.find('.', base));
}

// ---------------------------------------------------------------------------

namespace detail {

bool parse_bool(const std::string& raw, bool& out) {
  std::string s;
  for (char c : raw) s += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
  if (s == "true" || s == "on" || s == "yes" || s == "1") { out = true; return true; }
  if (s == "false" || s == "off" || s == "no" || s == "0") { out = false; return true; }
  return false;
}

bool parse_signed(const std::string& raw, std::int64_t& out) {
  if (raw.empty() || std::isspace(static_cast<unsigned char>(raw[0])))
    return false;
  errno = 0;
  char* end = nullptr;
  const long long v = std::strtoll(raw.c_str(), &end, 10);
  if (errno != 0 || *end != '\0')
    return false;
  out = v;
  return true;
}

bool parse_unsigned(const std::string& raw, std::uint64_t& out) {
  if (raw.empty() || raw[0] == '-' || std::isspace(static_cast<unsigned char>(raw[0])))
    return false;
  errno = 0;
  char* end = nullptr;
  const unsigned long long v = std::strtoull(raw.c_str(), &end, 10);
  if (errno != 0 || *end != '\0')
    return false;
  out = v;
  return true;
}

bool parse_double(const std::string& raw, double& out) {
  if (raw.empty() || std::isspace(static_cast<unsigned char>(raw[0])))
    return false;
  std::istringstream in(raw);
  in.imbue(std::locale::classic());
  double v;
  if (!(in >> v))
    return false;
  if (in.peek() != std::char_traits<char>::eof())
    return false;
  out = v;
  return true;
}

} // namespace detail

// ---------------------------------------------------------------------------

void param_value::mismatch(const char* wanted) const {
  throw std::invalid_argument("parameter '" + name_ + "' cannot be converted to " + wanted);
}

std::string param_value::to_string() const {
  std::ostringstream out;
  out << std::boolalpha << std::setprecision(std::numeric_limits<double>::max_digits10);
  std::visit([&out](const auto& v) {
    if constexpr (!std::is_same<std::decay_t<decltype(v)>, std::monostate>::value)
      out << v;
  }, value_);
  return out.str();
}

// ---------------------------------------------------------------------------

params::params(int argc, const char* const* argv) {
  if (argc > 0)
    argv0_ = argv[0];
  std::ostringstream options;
  bool files_only = false;
  for (int i = 1; i < argc; ++i) {
    const std::string arg = argv[i];
    if (files_only) {
      ini_files_.push_back(arg);
      continue;
    }
    std::string::size_type key_begin = 0;
    const std::string::size_type key_end = arg.find('=');
    if (arg.compare(0, 2, "--") == 0) {
      if (arg.size() == 2) {
        files_only = true;
        continue;
      }
      key_begin = 2;
    } else if (arg.compare(0, 1, "-") == 0) {
      key_begin = 1;
    }
    if (key_begin == 0 && key_end == std::string::npos) {
      ini_files_.push_back(arg);
      continue;
    }
    options << arg.substr(key_begin) << (key_end == std::string::npos ? "=true" : "") << "\n";
  }
  for (const std::string& f : ini_files_)
    read_ini_file(f);
  std::istringstream cmdline(options.str());
  parse_ini(cmdline, "command line", raw_);
  if (!defined("help"))
    define("help", "Print help message");
}

params::params(const std::string& inifile) {
  ini_files_.push_back(inifile);
  read_ini_file(inifile);
}

void params::read_ini_file(const std::string& fname) {
  std::ifstream in(fname.c_str());
  if (!in)
    throw std::runtime_error("Cannot read INI file " + fname);
  parse_ini(in, fname, raw_);
}

void params::supply(const std::string& name, const std::string& raw) {
  raw_[name] = raw;
}

param_value& params::operator[](const std::string& name) {
  auto it = values_.find(name);
  if (it == values_.end())
    it = values_.emplace(name, param_value(name)).first;
  return it->second;
}

const param_value& params::operator[](const std::string& name) const {
  auto it = values_.find(name);
  if (it == values_.end())
    throw std::runtime_error("parameter '" + name + "' is not defined");
  return it->second;
}

bool params::exists(const std::string& name) const {
  auto it = values_.find(name);
  return it != values_.end() && !it->second.empty();
}

params& params::description(const std::string& message) {
  help_header_ = message;
  if (!defined("help"))
    define("help", "Print help message");
  return *this;
}

bool params::help_requested() const {
  return exists("help") && (*this)["help"].convertible<bool>() && (*this)["help"].as<bool>();
}

bool params::help_requested(std::ostream& out) const {
  if (!help_requested())
    return false;
  print_help(out);
  return true;
}

std::ostream& params::print_help(std::ostream& out) const {
  out << help_header_ << "\nAvailable options:\n";
  std::vector<std::pair<std::string, std::string>> rows(descr_.size());
  std::string::size_type width = 0;
  for (const auto& d : descr_) {
    const std::string name_and_type = d.first + " (" + d.second.type + "):";
    width = std::max(width, name_and_type.size());
    std::string text = d.second.descr;
    if (exists(d.first) && d.first != "help")
      text += " (default value: " + (*this)[d.first].to_string() + ")";
    rows[d.second.number] = std::make_pair(name_and_type, text);
  }
  const std::ios::fmtflags flags = out.flags();
  for (const auto& row : rows)
    out << std::left << std::setw(static_cast<int>(width + 4)) << row.first << row.second << "\n";
  out.flags(flags);
  return out;
}

std::string params::origin_name() const {
  if (!ini_files_.empty())
    return ini_files_.front();
  std::string::size_type slash = argv0_.rfind('/');
  return slash == std::string::npos ? argv0_ : argv0_.substr(slash + 1);
}

} // namespace maxent
