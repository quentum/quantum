#!/usr/bin/env python

import sys
import os
from util import execute_stdout

SOURCE_ROOT = os.path.abspath(os.path.dirname(os.path.dirname(__file__)))
VENDOR_DIR = os.path.join(os.path.dirname(SOURCE_ROOT), 'vendor')

def main():
    os.chdir(SOURCE_ROOT)
    if sys.platform != 'win32':
        update_clang()
    update_submodules()

def update_clang():
    execute_stdout([os.path.join(SOURCE_ROOT, 'script', 'update-clang.sh')])

def update_submodules():
    execute_stdout(['git', 'submodule', 'sync'])
    execute_stdout(['git', 'submodule', 'update', '--init', '--recursive'])

if __name__ == '__main__':
    sys.exit(main())
