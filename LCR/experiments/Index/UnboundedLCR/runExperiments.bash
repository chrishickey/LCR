#!/bin/bash
#datasets=`find ./experiments/graphs/ | egrep '\.edge$' | sed 's:\.edge::g' | egrep -i '(V5kD(2|3|4|5)L8exp)'`;
datasets=`find ./experiments/graphs/ | egrep '\.edge$' | sed 's:\.edge::g' | egrep 'L8exp' | egrep 'V5kD'`;

#datasets=`find ./experiments/graphs/ | egrep '\.edge$' | sed 's:\.edge::g' | egrep -i '(notre|bio)'`;


for ds in $datasets;
do
#    if [[ "`cat $ds".edge" | wc -l`" -le $2 &&  "`cat $ds".edge" | wc -l`" -gt $3 ]]; then
        dsp=`echo $ds | sed 's/.*\///'`;
        echo "build/default/runExperiment "$ds".edge 3 "$1"results-"$dsp".csv > "$1"output-"$dsp".txt";
        build/default/runExperiment $ds".edge" 3 $1"results-"$dsp".csv" > $1"output-"$dsp".txt";
#    fi
done;
