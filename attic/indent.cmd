#
set -x

rm -fr mod
mkdir -p mod/imc

pushd src
find . \
  -type f \( -name '*.c' -o -name '*.h' \) \
  -exec indent -kr -bad -bap -sob -br -brf -ce -i2 -cli2 -nut -nhnl -l256 {} -o ../mod/{} ';'
popd

