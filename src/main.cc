#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <vector>

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

#include "config.h"

namespace boostpo = boost::program_options;
namespace boostfs = boost::filesystem;

using std::string;

bool file_exists(const string &filename) {
  std::ifstream file(filename);
  return file.good();
}

int main(int argc, char *argv[]) {
  boostpo::options_description desc("Allowed Options");
  desc.add_options()("help", "give help message")(
      "config", boostpo::value<string>(), "set custom config file location")(
      "snippet,s", boostpo::value<string>(), "copy snippet to your location")(
      "group,g", boostpo::value<string>(), "copy group to your location");

  boostpo::variables_map v_map;

  boostpo::store(boostpo::parse_command_line(argc, argv, desc), v_map);
  boostpo::notify(v_map);

  if (v_map.count("help")) {
    std::cout << desc << '\n';
    return 0;
  }

  string filename;

  if (v_map.count("config")) {
    filename = v_map["config"].as<string>();
  } else {
    string config = std::getenv("XDG_CONFIG_HOME");

    if (config.empty()) {
      config = string(std::getenv("HOME")) + string("/.config");
    }

    filename = string(config) + string("/snippet/config.toml");
  }

  if (!file_exists(filename)) {
    std::cerr << "Couldn't find file " << filename << '\n';
    return 1;
  }

  Config conf(filename);

  const auto table = conf.table();

  if (v_map.count("snippet")) {
    const string snippet_name = v_map["snippet"].as<string>();

    const string snippet = table["snippets"][snippet_name].value_or("");

    if (snippet.empty()) {
      std::cerr << "snippet '" << snippet_name << "' not found in " << filename
                << '\n';

      return 1;
    }

    if (file_exists(snippet)) {
      const string filename = string(std::filesystem::path(snippet).filename());

      boostfs::copy_file(
          snippet, string(std::filesystem::current_path()) + '/' + filename,
          boostfs::copy_option::none);

      std::cout << "Moving " << filename << '\n';

    } else {
      std::cerr << "'" << snippet << "' not found, skipping.\n";
    }
  }

  if (v_map.count("group")) {
    const string group_name = v_map["group"].as<string>();

    const auto group = table["snippets"]["groups"][group_name];

    if (!group.is_array()) {
      std::cerr << "snippet group '" << group_name
                << "' not found or not an array in " << filename << '\n';

      return 1;
    }
    const auto snippets = group.as_array();

    for (auto &&snippet_path : *snippets) {
      string snippet = snippet_path.value_or("");

      if (file_exists(snippet)) {
        const string filename =
            string(std::filesystem::path(snippet).filename());

        boostfs::copy_file(
            snippet, string(std::filesystem::current_path()) + '/' + filename,
            boostfs::copy_option::none);

        std::cout << "Moving " << filename << '\n';
      } else {
        std::cerr << "'" << snippet << "' not found, skipping.\n";
      }
    }
  }
}
