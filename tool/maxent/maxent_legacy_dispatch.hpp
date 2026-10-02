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

// Hands HDF5 input files to the legacy maxent.
//
// The legacy ALPS maxent read its parameters from an HDF5 input file
// (`maxent input.h5`); this maxent reads text parameter files. For one
// release the legacy code is installed as maxent_legacy, and maxent runs it,
// with the same arguments, whenever one of its arguments is an HDF5 file.

#pragma once

#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#ifndef _WIN32
#include <unistd.h>
#endif

namespace maxent {

/// true if path is an HDF5 file. The signature sits at offset 0, or at
/// 512, 1024, 2048, ... when the file has a user block.
inline bool is_hdf5_file(const std::string& path) {
  static const char signature[8] = {'\x89', 'H', 'D', 'F', '\r', '\n', '\x1a', '\n'};
  std::ifstream in(path, std::ios::binary);
  if (!in)
    return false;
  for (std::streamoff offset = 0; offset <= (std::streamoff(1) << 30);
       offset = offset ? 2 * offset : 512) {
    char buffer[8];
    in.seekg(offset);
    if (!in.read(buffer, 8))
      return false;
    if (std::memcmp(buffer, signature, 8) == 0)
      return true;
  }
  return false;
}

/// If any argument is an HDF5 file, replace this process by maxent_legacy
/// from the same directory (or PATH) and never return; returns 1 if that
/// fails. Returns 0, doing nothing, for text parameter files.
inline int dispatch_legacy_input(int argc, const char** argv) {
  bool legacy = false;
  for (int i = 1; i < argc && !legacy; ++i)
    legacy = argv[i][0] != '-' && is_hdf5_file(argv[i]);
  if (!legacy)
    return 0;

  std::cerr << "maxent: HDF5 input file, running the legacy maxent (maxent_legacy).\n"
            << "        The legacy maxent is deprecated; see `maxent --help` for the\n"
            << "        text parameter files of the current maxent.\n";
#ifndef _WIN32
  std::vector<char*> args;
  for (int i = 0; i < argc; ++i)
    args.push_back(const_cast<char*>(argv[i]));
  args.push_back(nullptr);

  const std::string self = argv[0];
  const std::string::size_type slash = self.rfind('/');
  if (slash != std::string::npos) {
    const std::string sibling = self.substr(0, slash + 1) + "maxent_legacy";
    args[0] = const_cast<char*>(sibling.c_str());
    execv(sibling.c_str(), args.data());
  }
  args[0] = const_cast<char*>("maxent_legacy");
  execvp("maxent_legacy", args.data());
#endif
  std::cerr << "maxent: could not run maxent_legacy (it is only built when LAPACK is found).\n";
  return 1;
}

} // namespace maxent
