#! /usr/bin/python3

import os
import subprocess

extensions = [".cpp", ".cxx", "cc", ".h", ".hpp"]

##########################################################################
def getQualifiedFileNames(extensions):
  codefiles = []
  for root, dirs, files in os.walk(os.getcwd()):
    for fname in files:
      qname = os.path.join(root, fname)
      ext = os.path.splitext(qname)[1]
      if ext in extensions:
        codefiles.append(qname)

  return codefiles
##########################################################################

def main():
  codefiles = getQualifiedFileNames(extensions)
  #print(codefiles)

  for qname in codefiles:
    lint_command = ["clang-format", "--style=Google", "-i", qname]
    subprocess.call(lint_command)


if __name__ == "__main__":
  main()

