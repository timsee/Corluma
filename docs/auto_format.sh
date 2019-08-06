

FORMAT="clang-format-9 -i -style=file"

cd ../src

$FORMAT *.cpp *.h
$FORMAT colorpicker/*.cpp colorpicker/*.h
$FORMAT comm/*.cpp comm/*.h
$FORMAT comm/arducor/*.cpp comm/arducor/*.h
$FORMAT comm/hue/*.cpp comm/hue/*.h
$FORMAT comm/nanoleaf/*.cpp comm/nanoleaf/*.h
$FORMAT cor/*.cpp cor/*.h
$FORMAT cor/widgets/*.cpp cor/widgets/*.h cor/objects/*.h
$FORMAT discovery/*.cpp discovery/*.h

cd ../docs/
