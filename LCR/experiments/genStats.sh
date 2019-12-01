datasets=`find experiments/graphs/ | egrep '\.edge$' | sed 's:\.edge::g'`;
for ds in $datasets;
do
    if [ ! -f "${ds}.csv" ];
    then
        echo "build/default/genStats ${ds}"
        build/default/genStats $ds
    fi
done;
