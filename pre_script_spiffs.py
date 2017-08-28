#!/bin/python
import os.path
from SCons.Script import DefaultEnvironment
env = DefaultEnvironment()

# def before_build_spiffs(source, target, env):
# 	print "\nbefore_build_spiffs\n"
# 	env.Execute("gulp buildfs")

if any(s in BUILD_TARGETS for s in ('buildfs', 'uploadfs')):
	if not(os.path.exists('./node_modules')):
		env.Execute("npm install")
	env.Execute("gulp buildfs")

# env.AddPreAction(".pioenvs/%s/spiffs.bin" % env['PIOENV'], before_build_spiffs)
