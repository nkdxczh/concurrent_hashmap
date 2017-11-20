for i in 4 16 64 128 256 500
do
    for j in {1..10}
    do
        ./obj32/test $i 100 20000 18 20
    done
done
