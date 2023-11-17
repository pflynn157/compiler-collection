#!/usr/bin/python3
import sys
import os
from shutil import copyfile

sys.path.append(os.getcwd())

import config

base_path = sys.argv[1]

copyfile(base_path + "/lex.cpp", "./lex.cpp")
copyfile(base_path + "/lex.hpp", "./lex.hpp")

