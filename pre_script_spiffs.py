#!/bin/python
Import("env")

def before_uploadfs(source, target, env):
	env.Execute("gulp buildfs")

env.AddPreAction("buildfs", before_uploadfs)
env.AddPreAction("uploadfs", before_uploadfs)