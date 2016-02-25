#!/usr/bin/env python

import os
import sys
import json
import subprocess

def execute_stdout(argv, env=os.environ):
    print ' '.join(argv)
    try:
        subprocess.check_call(argv, env=env)
    except subprocess.CalledProcessError as e:
        print e.output
        raise e

def default_arch():
    arch = 'x86'
    if sys.platform == 'darwin':
        arch = 'x64'
    return arch

def write_dict_to(obj, filename):
    with open(filename, 'w') as file_handle:
        json.dump(obj, file_handle, indent = 4)
