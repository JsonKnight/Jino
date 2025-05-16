# frozen_string_literal: true

require 'fileutils'
require 'etc'

PROJECT_ROOT = ENV.fetch('PROJECT_ROOT', Dir.pwd)
PROJECT_NAME_STR = ENV.fetch('PROJECT_NAME', File.basename(PROJECT_ROOT))
PROJECT_NAME_SYM = PROJECT_NAME_STR.to_sym

APP_NAME = PROJECT_NAME_STR

BUILD_DIR = ENV.fetch('BUILD_DIR', File.join(PROJECT_ROOT, 'build'))
BUILD_TYPE = ENV.fetch('BUILD_TYPE', 'Release')
JOBS = ENV.fetch('JOBS', Etc.nprocessors.to_s)

PROJECT_BIN_DIR_PATH = ENV.fetch('PROJECT_BIN_DIR', File.join(PROJECT_ROOT, 'bin'))
SRC_DIR = File.join(PROJECT_ROOT, 'src')

EXECUTABLE = File.join(PROJECT_BIN_DIR_PATH, APP_NAME)

DEFAULT_INSTALL_PREFIX = '/usr/local'
BUILD_SYSTEM_CMD = 'make'

namespace PROJECT_NAME_SYM do
  desc "Configure the project using CMake (Build type: #{BUILD_TYPE})"
  task :configure do
    puts "Configuring project '#{APP_NAME}' in #{BUILD_DIR} for #{BUILD_TYPE} build..."
    FileUtils.mkdir_p(BUILD_DIR)
    cmake_args = ENV.fetch('CMAKE_ARGS', '')
    cmake_command = [
      'cmake',
      '-S', PROJECT_ROOT,
      '-B', BUILD_DIR,
      "-DCMAKE_BUILD_TYPE=#{BUILD_TYPE}",
      "-DCMAKE_RUNTIME_OUTPUT_DIRECTORY=#{PROJECT_BIN_DIR_PATH}"
    ]
    cmake_command << "-DCMAKE_INSTALL_PREFIX=#{ENV.fetch('CMAKE_INSTALL_PREFIX', DEFAULT_INSTALL_PREFIX)}"
    cmake_command << cmake_args unless cmake_args.empty?
    sh cmake_command.join(' ')
  end

  desc "Build #{APP_NAME} (using #{JOBS} parallel jobs)"
  task build: :configure do
    puts "Building #{APP_NAME} in #{BUILD_DIR} (Jobs: #{JOBS})..."
    sh "cmake --build #{BUILD_DIR} --parallel #{JOBS}"
    FileUtils.mkdir_p(PROJECT_BIN_DIR_PATH)
    puts "Build complete. Local executable expected at: #{EXECUTABLE}"
  end

  desc "Install #{APP_NAME} system-wide (requires sudo - WILL prompt for password)"
  task install: :build do
    install_prefix = ENV.fetch('CMAKE_INSTALL_PREFIX', DEFAULT_INSTALL_PREFIX)
    install_command = "sudo cmake --install #{BUILD_DIR} --prefix #{install_prefix}"
    puts "Running system-wide install of #{APP_NAME} to #{install_prefix}: #{install_command}"
    sh install_command
    puts "#{APP_NAME} system-wide installation complete."
    puts 'Consider updating caches (e.g., desktop database, icon cache) manually if needed.'
  end

  desc "Build and install #{APP_NAME} (Deploy)"
  task deploy: :install do
    puts "Deployment (build and install) complete for #{APP_NAME}."
  end

  desc "Uninstall #{APP_NAME} system-wide (requires sudo - WILL prompt for password)"
  task uninstall: :configure do
    uninstall_target_cmd = "#{BUILD_SYSTEM_CMD} uninstall"
    unless File.directory?(BUILD_DIR) && File.exist?(File.join(BUILD_DIR, 'Makefile'))
      puts "\nWARNING: Build directory '#{BUILD_DIR}' or its Makefile does not exist."
      puts "Uninstall requires a configured build directory. Run 'rake #{PROJECT_NAME_SYM}:configure' first."
      next
    end

    uninstall_command_full = "cd #{BUILD_DIR} && sudo #{uninstall_target_cmd}"
    puts "Running system-wide uninstall of #{APP_NAME}: #{uninstall_command_full}"
    sh uninstall_command_full
    puts "#{APP_NAME} system-wide uninstall complete."
    puts 'Consider updating caches manually if needed.'
  end

  desc "Run #{APP_NAME} from #{PROJECT_BIN_DIR_PATH}/"
  task run: :build do
    raise "Local executable #{EXECUTABLE} not found after build!" unless File.exist?(EXECUTABLE)

    puts "Running #{EXECUTABLE}..."
    sh EXECUTABLE
  end

  desc "Clean the build and #{PROJECT_BIN_DIR_PATH} directories"
  task :clean do
    puts "Cleaning build directory: #{BUILD_DIR}"
    FileUtils.rm_rf(BUILD_DIR) if File.directory?(BUILD_DIR)
    if File.exist?(EXECUTABLE) && !BUILD_DIR.include?(PROJECT_BIN_DIR_PATH)
      puts "Cleaning executable: #{EXECUTABLE}"
      FileUtils.rm_f(EXECUTABLE)
    end
    puts 'Clean finished.'
  end

  desc "Rebuild #{APP_NAME} (clean + build)"
  task rebuild: %i[clean build]

  desc 'Format C++/HPP source files using clang-format'
  task :format do
    puts "Formatting C++/HPP files in #{SRC_DIR}/ ..."
    cpp_files = FileList["#{SRC_DIR}/**/*.cpp", "#{SRC_DIR}/**/*.hpp",
                         "#{SRC_DIR}/**/*.cc", "#{SRC_DIR}/**/*.hh",
                         "#{SRC_DIR}/**/*.cxx", "#{SRC_DIR}/**/*.hxx"]
    if cpp_files.empty?
      puts "No C++/HPP files found in #{SRC_DIR}/ to format."
    else
      unless system('command -v clang-format > /dev/null')
        puts 'WARNING: clang-format command not found. Please install it to format code.'
        next
      end
      sh "clang-format -i #{cpp_files.join(' ')}"
      puts "Formatting complete for #{cpp_files.count} file(s)."
    end
  end
end

desc "Alias for #{PROJECT_NAME_SYM}:run"
task r: "#{PROJECT_NAME_SYM}:run"
desc "Alias for #{PROJECT_NAME_SYM}:build"
task b: "#{PROJECT_NAME_SYM}:build"
desc "Alias for #{PROJECT_NAME_SYM}:clean"
task c: "#{PROJECT_NAME_SYM}:clean"
desc "Alias for #{PROJECT_NAME_SYM}:rebuild"
task rb: "#{PROJECT_NAME_SYM}:rebuild"
desc "Alias for #{PROJECT_NAME_SYM}:install"
task i: "#{PROJECT_NAME_SYM}:install"
desc "Alias for #{PROJECT_NAME_SYM}:uninstall"
task u: "#{PROJECT_NAME_SYM}:uninstall"
desc "Alias for #{PROJECT_NAME_SYM}:configure"
task cfg: "#{PROJECT_NAME_SYM}:configure"
desc "Alias for #{PROJECT_NAME_SYM}:deploy"
task d: "#{PROJECT_NAME_SYM}:deploy"
desc "Alias for #{PROJECT_NAME_SYM}:format"
task f: "#{PROJECT_NAME_SYM}:format"

task default: "#{PROJECT_NAME_SYM}:run"
