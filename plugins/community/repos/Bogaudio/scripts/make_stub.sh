#!/bin/bash

MODULE=$1
HP=$2
CLASS=$3

if [ "x$MODULE" == "x" ] || [ "x$HP" == "x" ]
then
  echo "Usage: $0 <module name> <width in HP> [param class]" 1>&2
  exit 1
fi
if [ "x$CLASS" == "x" ]
then
  CLASS=Knob26
fi

WIDGETS="./scripts/svg_widgets.rb"
HPP_OUT="./src/$MODULE.hpp"
CPP_OUT="./src/$MODULE.cpp"

($WIDGETS ./res-src/$MODULE-src.svg --stub-hpp --module=$MODULE --plugin=Bogaudio --manufacturer=Bogaudio --hp=$HP --param-class=$CLASS | ruby -e 's = STDIN.read; s.sub!(/\/\* For.*?\*\/\s*/m, ""); puts s' > $HPP_OUT) || exit 1
echo "Wrote: $HPP_OUT"

($WIDGETS ./res-src/$MODULE-src.svg --stub-cpp --module=$MODULE --plugin=Bogaudio --manufacturer=Bogaudio --hp=$HP --param-class=$CLASS > $CPP_OUT) || exit 1
echo "Wrote: $CPP_OUT"

PLUGIN="./src/bogaudio.cpp"
if [ -e $PLUGIN ]
then
  if [ "x$(grep NEW_INCLUDES_HERE $PLUGIN)" != "x" ]
  then
    INCLUDE="#include \"$MODULE.hpp\""
    echo Patching $PLUGIN with: $INCLUDE
    ruby -e "s = File.read('$PLUGIN'); s.sub!(/\/\/NEW_INCLUDES_HERE/, %Q{$INCLUDE\n//NEW_INCLUDES_HERE}); File.write('$PLUGIN', s)"
  fi

  if [ "x$(grep NEW_MODELS_HERE $PLUGIN)" != "x" ]
  then
    MODEL="p->addModel(model$MODULE);"
    echo Patching $PLUGIN with: $MODEL
    ruby -e "s = File.read('$PLUGIN'); s.sub!(/\/\/NEW_MODELS_HERE/, %Q{$MODEL\n\t//NEW_MODELS_HERE}); File.write('$PLUGIN', s)"
  fi
fi
