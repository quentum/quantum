#!/usr/bin/env python

import sys
import os
import argparse
import util
import ctypes

SOURCE_ROOT = os.path.abspath(os.path.dirname(os.path.dirname(__file__)))
VENDOR_DIR = os.path.join(SOURCE_ROOT, 'vendor')

def main():
    args = parse_args()
    check_args(args)

    os.chdir(VENDOR_DIR)

    setup_windows_env()
    generate_lastchange()
    configura_node(args)
    generate_ninja_files(args)
    build_vendor(args)

def parse_args():
    parser = argparse.ArgumentParser(description='build vendor')
    parser.add_argument('--target-arch',            \
                        action = 'store',           \
                        choices = ['x86', 'x64'],   \
                        default = 'x86',            \
                        help = 'specify the arch to build for')
    parser.add_argument('--configuration',          \
                        action = 'store',           \
                        nargs = '+',                \
                        default = ['Debug'],        \
                        help = 'build with Release or Debug configuration')
    parser.add_argument('--component',                                          \
                        action = 'store',                                       \
                        choices = ['static_library', 'shared_library'],         \
                        default = 'shared_library',                             \
                        help = 'build component shared or static')
    parser.add_argument('--library',                                            \
                        action = 'store',                                       \
                        choices = ['static_library', 'shared_library'],         \
                        default = 'static_library',                             \
                        help = 'build library shared or static')

    return parser.parse_args()

def check_args(args):
    if sys.platform == 'darwin' and args.target_arch == 'x86':
        raise Exception('do not support x86 on mac')

    configuration_len = len(args.configuration)
    if configuration_len > 2:
        raise Exception('configuration count must be 1 or 2')

    valid_configuration = ['Debug', 'Release']
    for configuration in args.configuration:
        if not configuration in valid_configuration:
            raise Exception('unknown configuration ' + configuration)

def setup_windows_env():
    if sys.platform != 'win32':
        return
    os.environ['DEPOT_TOOLS_WIN_TOOLCHAIN'] = '0'
    winsdk_dir = os.environ['WindowsSdkDir']
    if not winsdk_dir or winsdk_dir.isspace():
        raise Exception('windows sdk not installed')

    setenv_cmd_path = os.path.join(winsdk_dir, 'bin')
    setenv_cmd_path += '/SetEnv.Cmd'
    setenv_cmd_path = os.path.normpath(setenv_cmd_path)
    if not os.path.isfile(setenv_cmd_path):
        is_admin = ctypes.windll.shell32.IsUserAnAdmin()
        if not is_admin:
            raise Exception('need admin privilege on Windows')
        open(setenv_cmd_path, 'a').close()

def get_ninja():
    ninja = os.path.join('tools', 'depot_tools', 'ninja')
    if sys.platform == 'win32':
        ninja += '.exe'
    return ninja

def generate_lastchange():
    lastchange_file = os.path.join('build', 'util', 'LASTCHANGE')
    if os.path.isfile(lastchange_file):
        return

    lastchange_script = os.path.join('build', 'util', 'lastchange.py')
    util.execute_stdout(['python', lastchange_script, lastchange_file])

def configura_node(args):
    configure_file = os.path.join('node', 'configure')
    util.execute_stdout(['python', configure_file, '--enable-shared', '--dest-cpu', args.target_arch])

def generate_ninja_files(args):
    gyp_file = os.path.join('build', 'gyp_chromium')
    node_common_gypi = os.path.join('node', 'common.gypi')
    node_config_gypi = os.path.join('node', 'config.gypi')
    node_custom_gypi = os.path.join('node', 'custom.gypi')
    exec_args = ['python', gyp_file, '--depth=.', '--toplevel-dir=.', 'vendor.gyp']
    exec_args += ['-I', node_common_gypi]
    exec_args += ['-I', node_config_gypi]
    exec_args += ['-I', node_custom_gypi]
    exec_args += ['-D', 'component=' + args.component]
    exec_args += ['-D', 'library=' + args.library]
    util.execute_stdout(exec_args)

def build_vendor(args):
    ninja = get_ninja()
    for config in args.configuration:
        build_dir = os.path.join('out', config)
        if args.target_arch == 'x64' and sys.platform == 'win32':
            build_dir += '_x64'
        util.execute_stdout([ninja, '-C', build_dir, 'vendor'])

if __name__ == '__main__':
    sys.exit(main())
