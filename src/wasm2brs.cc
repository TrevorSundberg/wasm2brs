/*
  Copyright 2020 Trevor Sundberg
  This file has been modified to output BrightScript
  All modifications are licenced under LICENSE.md
*/
/*
 * Copyright 2017 WebAssembly Community Group participants
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <cassert>
#include <cinttypes>
#include <cstdio>
#include <cstdlib>

#include "src/apply-names.h"
#include "src/wast-parser.h"
#include "src/binary-reader.h"
#include "src/binary-reader-ir.h"
#include "src/error-formatter.h"
#include "src/feature.h"
#include "src/generate-names.h"
#include "src/ir.h"
#include "src/option-parser.h"
#include "src/stream.h"
#include "src/validator.h"
#include "src/wast-lexer.h"

#include "brs-writer.h"

using namespace wabt;

static int s_verbose;
static std::string s_infile;
static std::string s_prefix;
static Features s_features;
static WriteCOptions s_write_c_options;
static bool s_read_debug_names = true;
static std::unique_ptr<FileStream> s_log_stream;

static const char s_description[] =
R"(  Read a file in the WebAssembly binary format, and convert it to
  a C source file and header.

examples:
  # parse binary file test.wasm and write test.c and test.h
  $ wasm2c test.wasm -o test.c

  # parse test.wasm, write test.c and test.h, but ignore the debug names, if any
  $ wasm2c test.wasm --no-debug-names -o test.c
)";

static void ParseOptions(int argc, char** argv) {
  OptionParser parser("wasm2c", s_description);

  parser.AddOption('v', "verbose", "Use multiple times for more info", []() {
    s_verbose++;
    s_log_stream = FileStream::CreateStdout();
  });
  parser.AddOption(
      'o', "output", "FILENAME",
      "Output file for the generated C source file, by default use stdout",
      [](const char* argument) {
        s_write_c_options.out_filename = argument;
        ConvertBackslashToSlash(&s_write_c_options.out_filename);
      });
  s_features.AddOptions(&parser);
  parser.AddOption("no-debug-names", "Ignore debug names in the binary file",
                   []() { s_read_debug_names = false; });
  parser.AddArgument("filename", OptionParser::ArgumentCount::One,
                     [](const char* argument) {
                       s_infile = argument;
                       ConvertBackslashToSlash(&s_infile);
                     });
  parser.AddOption('n', "name-prefix", "NAMEPREFIX", "The prefix to use on all function and variable names",
                     [](const char* argument) {
                       s_write_c_options.name_prefix = argument;
                     });
  parser.Parse(argc, argv);

  // TODO(binji): currently wasm2c doesn't support any non-default feature
  // flags.
  bool any_non_default_feature = false;
#define WABT_FEATURE(variable, flag, default_, help) \
  any_non_default_feature |= (s_features.variable##_enabled() != default_);
#include "src/feature.def"
#undef WABT_FEATURE

  if (any_non_default_feature) {
    fprintf(stderr, "wasm2c currently support only default feature flags.\n");
    exit(1);
  }
}

int ProgramMain(int argc, char** argv) {
  Result result;

  InitStdio();
  ParseOptions(argc, argv);

  std::vector<uint8_t> file_data;
  result = ReadFile(s_infile.c_str(), &file_data);
  if (Succeeded(result)) {
    Errors errors;
    std::unique_ptr<wabt::Module> module;
    Location::Type location_type = Location::Type::Text;
    const char first = file_data.front();
    if (first == '(' || first == ';' || first == ' ' || first == '\n') {
      std::unique_ptr<WastLexer> lexer = WastLexer::CreateBufferLexer(
          s_infile, file_data.data(), file_data.size());
      WastParseOptions options(s_features);
      options.debug_parsing = s_read_debug_names;
      result = ParseWatModule(lexer.get(), &module, &errors, &options);
    } else {
      location_type = Location::Type::Binary;
      module = std::make_unique<wabt::Module>();
      const bool kStopOnFirstError = true;
      const bool kFailOnCustomSectionError = true;
      ReadBinaryOptions options(s_features, s_log_stream.get(),
                                s_read_debug_names, kStopOnFirstError,
                                kFailOnCustomSectionError);
      result = ReadBinaryIr(s_infile.c_str(), file_data.data(), file_data.size(),
                            options, &errors, module.get());
    }

    if (Succeeded(result)) {
      if (Succeeded(result)) {
        ValidateOptions options(s_features);
        result = ValidateModule(module.get(), &errors, options);
        result |= GenerateNames(module.get());
      }

      if (Succeeded(result)) {
        /* TODO(binji): This shouldn't fail; if a name can't be applied
         * (because the index is invalid, say) it should just be skipped. */
        Result dummy_result = ApplyNames(module.get());
        WABT_USE(dummy_result);
      }

      if (Succeeded(result)) {
        if (s_write_c_options.name_prefix.empty()) {
          if (module->name.empty()) {
            s_write_c_options.name_prefix = "w2b";
          } else {
            s_write_c_options.name_prefix = module->name;
          }
        }
        result = WriteBrs(module.get(), s_write_c_options);
      }
    }
    FormatErrorsToFile(errors, location_type);
  }
  return result != Result::Ok;
}

int main(int argc, char** argv) {
  WABT_TRY
  return ProgramMain(argc, argv);
  WABT_CATCH_BAD_ALLOC_AND_EXIT
}

