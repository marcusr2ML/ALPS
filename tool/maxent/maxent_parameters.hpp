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

// Parameter store for Maxent inside ALPS.
//
// Maxent was written against ALPSCore's alps::params. ALPS has an unrelated
// class of the same name (alps/ngs/params.hpp), so Maxent carries its own,
// reproducing the ALPSCore semantics it relies on:
//
//  * parameter files in ALPSCore's INI dialect: KEY = value, '#' and ';'
//    comments (also trailing ones), quoted values, [section] prefixes,
//    backslash line continuation;
//  * command-line overrides --KEY=value (a bare --KEY means KEY=true),
//    applied after all parameter files;
//  * define<T>(name, [default,] description): typed parsing of supplied
//    values, defaults, and required parameters;
//  * exists / defined / defaulted / supplied, --help with descriptions;
//  * values convert like ALPSCore's: integral <-> integral when the value
//    fits, integral -> floating point, never floating point -> integral,
//    bool and string only to themselves.
//
// See ALPSCore params/src/params.cpp and params_impl.hpp for the reference.

#pragma once

#include <cstdint>
#include <limits>
#include <map>
#include <ostream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <variant>
#include <vector>

namespace maxent {

class param_value {
public:
  typedef std::variant<std::monostate, bool, std::int64_t, std::uint64_t, double, std::string> storage_type;

  param_value() {}
  explicit param_value(const std::string& name) : name_(name) {}

  bool empty() const { return std::holds_alternative<std::monostate>(value_); }
  void clear() { value_ = std::monostate(); }
  const std::string& name() const { return name_; }

  /// true if the stored value converts to T under the rules above
  template <typename T> bool convertible() const;

  template <typename T> T as() const;

  template <typename T, typename = std::enable_if_t<!std::is_same<T, char>::value>>
  operator T() const { return as<T>(); }

  template <typename T>
  param_value& operator=(const T& v) { set(v); return *this; }
  param_value& operator=(const char* v) { value_ = std::string(v); return *this; }

  /// the value as it would appear in a parameter file (for --help)
  std::string to_string() const;

  friend bool operator==(const param_value& lhs, const std::string& rhs) { return lhs.as<std::string>() == rhs; }
  friend bool operator==(const param_value& lhs, const char* rhs) { return lhs.as<std::string>() == rhs; }
  friend bool operator==(const param_value& lhs, bool rhs) { return lhs.as<bool>() == rhs; }
  friend bool operator!=(const param_value& lhs, const std::string& rhs) { return !(lhs == rhs); }
  friend bool operator!=(const param_value& lhs, const char* rhs) { return !(lhs == rhs); }
  friend bool operator!=(const param_value& lhs, bool rhs) { return !(lhs == rhs); }

private:
  template <typename T> void set(const T& v) {
    if constexpr (std::is_same<T, bool>::value) value_ = v;
    else if constexpr (std::is_integral<T>::value && std::is_signed<T>::value) value_ = static_cast<std::int64_t>(v);
    else if constexpr (std::is_integral<T>::value) value_ = static_cast<std::uint64_t>(v);
    else if constexpr (std::is_floating_point<T>::value) value_ = static_cast<double>(v);
    else value_ = std::string(v);
  }

  [[noreturn]] void mismatch(const char* wanted) const;

  std::string name_;
  storage_type value_;
};

class params {
public:
  params() {}
  /// ALPSCore command line: parameter files and --KEY=value overrides
  params(int argc, const char* const* argv);
  /// a single parameter file
  explicit params(const std::string& inifile);

  /// supply a raw value, as if it came from a parameter file; define<T>() parses it
  void supply(const std::string& name, const std::string& raw);

  param_value& operator[](const std::string& name);
  const param_value& operator[](const std::string& name) const;

  bool exists(const std::string& name) const;
  bool supplied(const std::string& name) const { return raw_.count(name) != 0; }
  bool defaulted(const std::string& name) const { return exists(name) && !supplied(name); }
  bool defined(const std::string& name) const { return descr_.count(name) != 0 || exists(name); }

  template <typename T> params& define(const std::string& name, const std::string& descr);
  template <typename T> params& define(const std::string& name, const T& defval, const std::string& descr);
  /// a flag: boolean option with default false
  params& define(const std::string& name, const std::string& descr) { return define<bool>(name, false, descr); }

  params& description(const std::string& message);
  bool help_requested() const;
  bool help_requested(std::ostream& out) const;
  std::ostream& print_help(std::ostream& out) const;

  /// problems found while defining parameters (missing or unparsable values)
  const std::vector<std::string>& errors() const { return errors_; }

  /// first parameter file, or the program name when there is none
  std::string origin_name() const;

private:
  struct definition {
    std::string type, descr;
    int number;
  };

  template <typename T> bool define_(const std::string& name, const std::string& descr);
  template <typename T> static bool parse(const std::string& raw, T& out);
  template <typename T> static const char* type_name();
  void read_ini_file(const std::string& fname);

  std::map<std::string, param_value> values_;
  std::map<std::string, std::string> raw_;
  std::map<std::string, definition> descr_;
  std::vector<std::string> errors_;
  std::vector<std::string> ini_files_;
  std::string argv0_;
  std::string help_header_;
};

/// ALPSCore alps::origin_name()
inline std::string origin_name(const params& p) { return p.origin_name(); }

/// ALPSCore alps::fs::remove_extensions(): strip everything from the first
/// dot of the file name on ("dir/a.b.c" -> "dir/a")
std::string remove_extensions(const std::string& filename);

/// parse KEY = value lines in ALPSCore's INI dialect into map (later keys win)
void parse_ini(std::istream& in, const std::string& source, std::map<std::string, std::string>& map);

// ---------------------------------------------------------------------------

template <typename T> bool param_value::convertible() const {
  if constexpr (std::is_same<T, bool>::value || std::is_same<T, std::string>::value) {
    return std::holds_alternative<T>(value_);
  } else if constexpr (std::is_integral<T>::value) {
    if (auto s = std::get_if<std::int64_t>(&value_)) {
      if constexpr (std::is_signed<T>::value)
        return *s >= std::numeric_limits<T>::min() && *s <= std::numeric_limits<T>::max();
      else
        return *s >= 0 && static_cast<std::uint64_t>(*s) <= std::numeric_limits<T>::max();
    }
    if (auto u = std::get_if<std::uint64_t>(&value_))
      return *u <= static_cast<std::uint64_t>(std::numeric_limits<T>::max());
    return false;
  } else if constexpr (std::is_floating_point<T>::value) {
    return std::holds_alternative<std::int64_t>(value_) || std::holds_alternative<std::uint64_t>(value_)
        || std::holds_alternative<double>(value_);
  } else {
    return false;
  }
}

template <typename T> T param_value::as() const {
  if (empty())
    throw std::runtime_error("parameter '" + name_ + "' has no value");
  if constexpr (std::is_same<T, bool>::value) {
    if (auto b = std::get_if<bool>(&value_)) return *b;
    mismatch("bool");
  } else if constexpr (std::is_same<T, std::string>::value) {
    if (auto s = std::get_if<std::string>(&value_)) return *s;
    mismatch("string");
  } else if constexpr (std::is_arithmetic<T>::value) {
    if (!convertible<T>()) mismatch(std::is_integral<T>::value ? "integer" : "floating point");
    if (auto s = std::get_if<std::int64_t>(&value_)) return static_cast<T>(*s);
    if (auto u = std::get_if<std::uint64_t>(&value_)) return static_cast<T>(*u);
    return static_cast<T>(std::get<double>(value_));
  } else {
    static_assert(std::is_arithmetic<T>::value, "unsupported parameter type");
  }
}

namespace detail {
bool parse_bool(const std::string& raw, bool& out);
bool parse_signed(const std::string& raw, std::int64_t& out);
bool parse_unsigned(const std::string& raw, std::uint64_t& out);
bool parse_double(const std::string& raw, double& out);
}

template <typename T> bool params::parse(const std::string& raw, T& out) {
  if constexpr (std::is_same<T, bool>::value) {
    return detail::parse_bool(raw, out);
  } else if constexpr (std::is_same<T, std::string>::value) {
    out = raw;
    return true;
  } else if constexpr (std::is_integral<T>::value && std::is_signed<T>::value) {
    std::int64_t v;
    if (!detail::parse_signed(raw, v) || v < std::numeric_limits<T>::min() || v > std::numeric_limits<T>::max())
      return false;
    out = static_cast<T>(v);
    return true;
  } else if constexpr (std::is_integral<T>::value) {
    std::uint64_t v;
    if (!detail::parse_unsigned(raw, v) || v > std::numeric_limits<T>::max())
      return false;
    out = static_cast<T>(v);
    return true;
  } else {
    double v;
    if (!detail::parse_double(raw, v))
      return false;
    out = static_cast<T>(v);
    return true;
  }
}

template <typename T> const char* params::type_name() {
  if constexpr (std::is_same<T, bool>::value) return "bool";
  else if constexpr (std::is_same<T, std::string>::value) return "std::string";
  else if constexpr (std::is_same<T, double>::value) return "double";
  else if constexpr (std::is_same<T, int>::value) return "int";
  else if constexpr (std::is_same<T, unsigned int>::value) return "unsigned int";
  else if constexpr (std::is_same<T, long>::value) return "long";
  else if constexpr (std::is_same<T, unsigned long>::value) return "unsigned long";
  else return "value";
}

template <typename T> bool params::define_(const std::string& name, const std::string& descr) {
  param_value& value = (*this)[name];
  if (!value.empty() && !value.convertible<T>())
    throw std::invalid_argument("parameter '" + name + "' already holds a value of a different type");

  auto def = descr_.find(name);
  if (def != descr_.end()) {
    if (def->second.type != type_name<T>())
      throw std::invalid_argument("parameter '" + name + "' already defined with a different type");
    def->second.descr = descr;
    return true;
  }
  descr_[name] = definition{type_name<T>(), descr, static_cast<int>(descr_.size())};

  auto raw = raw_.find(name);
  if (raw == raw_.end()) {
    if (!value.empty()) {
      value = value.as<T>(); // store with the defined type
      return true;
    }
    return false; // caller decides whether a default applies
  }
  T parsed;
  if (parse<T>(raw->second, parsed)) {
    value = parsed;
  } else {
    errors_.push_back("Cannot parse parameter '" + name + "' as the requested type");
    value.clear();
  }
  return true;
}

template <typename T> params& params::define(const std::string& name, const std::string& descr) {
  if (!define_<T>(name, descr))
    errors_.push_back("Required parameter '" + name + "' is missing");
  return *this;
}

template <typename T> params& params::define(const std::string& name, const T& defval, const std::string& descr) {
  if (!define_<T>(name, descr))
    (*this)[name] = defval;
  return *this;
}

} // namespace maxent
