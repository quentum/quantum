#!/usr/bin/env python

import sys
import os
import util
import argparse

SOURCE_ROOT = os.path.abspath(os.path.dirname(os.path.dirname(__file__)))
VENDOR_DIR = os.path.join(SOURCE_ROOT, 'vendor')

def main():
    args = parse_args()
    check_args(args)
    download(args)
    build_prerequisites(args)

def parse_args():
    parser = argparse.ArgumentParser(description='download dependencies')
    parser.add_argument('--group',                                          \
                        action = 'store',                                   \
                        choices = ['basic', 'node', 'all'],                 \
                        default = 'basic',                                  \
                        help = 'specify the group to download')
    parser.add_argument('--target-arch',                                    \
                        action = 'store',                                   \
                        choices = ['x86', 'x64'],                           \
                        default = util.default_arch(),                      \
                        help = 'specify the arch to build for')
    parser.add_argument('--component',                                      \
                        action = 'store',                                   \
                        choices = ['static_library', 'shared_library'],     \
                        default = 'shared_library',                         \
                        help = 'build component shared or static')
    parser.add_argument('--library',                                        \
                        action = 'store',                                   \
                        choices = ['static_library', 'shared_library'],     \
                        default = 'static_library',                         \
                        help = 'build library shared or static')
    return parser.parse_args()

def check_args(args):
    if sys.platform == 'darwin' and args.target_arch == 'x86':
        raise Exception('do not support x86 on mac')

def download(args):
    os.chdir(SOURCE_ROOT)
    if sys.platform != 'win32':
        update_clang()
    update_submodules(args)

def build_prerequisites(args):
    os.chdir(VENDOR_DIR)
    setup_windows_env()
    generate_gyp_config(args)
    generate_lastchange()
    configura_node(args)
    generate_ninja_files(args)

#***************** sub function called by download **************************#
def update_clang():
    util.execute_stdout([os.path.join('script', 'update-clang.sh')])

def update_submodules(args):
    modules = get_submodules(args)
    for module in modules:
        print 'update module ' + module
        util.execute_stdout(['git', 'submodule', 'sync', '--', module])
        util.execute_stdout(['git', 'submodule', 'update', '--init', '--recursive', '--', module])

def get_submodules(args):
    tools = [
        'vendor/build',
        'vendor/testing',
        'vendor/tools/gyp',
        'vendor/tools/clang',
        'vendor/tools/depot_tools'
    ]
    basic = [
        'vendor/base',
        'vendor/ipc',
        'vendor/crypto',
        'vendor/third_party/icu',
        'vendor/third_party/zlib',
        'vendor/third_party/yasm',
        'vendor/third_party/libxml',
        'vendor/third_party/modp_b64',
        'vendor/third_party/boringssl',
        'vendor/third_party/apple_apsl'
    ]
    node = [
        'vendor/node',
        'vendor/v8',
        'vendor/gin',
        'vendor/native-mate',
        'vendor/third_party/WebKit/Source/core/streams',
    ]
    modules = tools
    if args.group == 'basic':
        modules += basic
    if args.group == 'node':
        modules += basic
        modules += node
    if args.group == 'all':
        modules += basic
        modules += node
    print 'will update the follow modules:'
    for module in modules:
        print '\t' + module
    return modules

#**************** sub function called by build_prerequisites ****************#
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

def generate_gyp_config(args):
    variables_value = {
        'have_basic_modules%': 'false',
        'have_node_modules%': 'false',
    }
    if args.group == 'basic':
        variables_value['have_basic_modules%'] = 'true'
    if args.group == 'node':
        variables_value['have_basic_modules%'] = 'true'
        variables_value['have_node_modules%'] = 'true'
    if args.group == 'all':
        variables_value['have_basic_modules%'] = 'true'
        variables_value['have_node_modules%'] = 'true'
    variables = {'variables': variables_value}
    print variables
    util.write_dict_to(variables, 'modules.gypi')

def generate_lastchange():
    lastchange_file = os.path.join('build', 'util', 'LASTCHANGE')
    if os.path.isfile(lastchange_file):
        return

    lastchange_script = os.path.join('build', 'util', 'lastchange.py')
    util.execute_stdout(['python', lastchange_script, lastchange_file])

def configura_node(args):
    configure_file = os.path.join('node', 'configure')
    if not os.path.isfile(configure_file):
        return
    util.execute_stdout(['python', configure_file, '--enable-shared', '--dest-cpu', args.target_arch])

def generate_ninja_files(args):
    gyp_file = os.path.join('build', 'gyp_chromium')
    node_common_gypi = os.path.join('node', 'common.gypi')
    node_config_gypi = os.path.join('node', 'config.gypi')
    exec_args = ['python', gyp_file, '--depth=.', '--toplevel-dir=.', 'vendor.gyp']
    exec_args += ['-I', 'custom.gypi']
    if os.path.isfile(node_common_gypi) and args.group != 'basic':
        exec_args += ['-I', node_common_gypi]
    if os.path.isfile(node_config_gypi) and args.group != 'basic':
        exec_args += ['-I', node_config_gypi]
    exec_args += ['-D', 'component=' + args.component]
    exec_args += ['-D', 'library=' + args.library]
    util.execute_stdout(exec_args)

if __name__ == '__main__':
    sys.exit(main())
