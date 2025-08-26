this is the block copy command for staging that loads at bootup.  It requires the asembler as.  This can be found at http://john.ccac.rwth-aachen.de:8000/as/

  if you encounter errors compiling you may wish to consult this gist

  https://gist.github.com/rain1024/98dd5e2c6c8c28f9ea9d

under linux run:

run ./build.sh to build 

run ./build.sh install to copy the resulting .h file to the ardiuno root

run ./build.sh clean to clean the codespace to just the asembly and the build script.

