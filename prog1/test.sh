#!/bin/sh
rm times.txt
make minicat

i=0
echo running tests...
while [ $i -le 12 ]; do
rm out1.txt
b=$((2 ** i))

echo run $i of size $b >> times.txt
/usr/bin/time ./minicat -b $b -o out1.txt text1.txt text2.txt 2>> times.txt
echo run $i of size $b completed
diff out1.txt out.txt
i=$((i+1))
done

echo runs completed
