#!/usr/bin/env python

import json
import os
from string import Template
import subprocess
import sys

scriptDirectory = os.path.dirname(os.path.abspath(__file__))
packageJsonFile = os.path.join(scriptDirectory, 'package.json')

if len(sys.argv) != 2:
    print "Usage: build.py [inputFile] > path/to/outputFile"
    sys.exit(1)

inputFile = sys.argv[1]
version = 'N/A'
commit = subprocess.check_output(["git", "describe", "--always"]).strip().decode()
dirty = 'false'

with open(packageJsonFile) as packageFile:
    data = json.loads(packageFile.read())
    version = data['version']

process = subprocess.Popen(['git', 'diff-index', '--quiet', 'HEAD'])
isDirty = process.returncode

if isDirty != 0:
    dirty = 'true'

with open(inputFile, 'r') as file:
    templateString = file.read()
    template = Template(templateString)
    print template.substitute(version=version, commit=commit, dirty=dirty)