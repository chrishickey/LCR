#!/bin/bash
IFS=$'\n'

for i in `ls $1/results-*`;
do
    name=$(echo $i | sed 's/.*\///' | sed 's/\.csv//g' | sed 's:results-::g' );
    j=`cat $i | head -n 9 | tr '\ ' '&' |  egrep -o '[0-9_]*\.[0-9_]{0,2}' | tr '_' ','`;
    #echo "\\textbf{${name}}"" & "`echo $j | awk '{ print $1,$3,$5,$7,$9 }' | sed 's: : \& :g'`" "\\\\
done;

echo "---"

for i in `ls $1/results-*`;
do
    name=$(echo $i | sed 's/.*\///' | sed 's/\.csv//g' | sed 's:results-::g' );
    j=`cat $i | head -n 9 | tr '\ ' '&' |  egrep -o '[0-9_]*\.[0-9_]{0,2}' | tr '_' ','`;
    #echo "\\textbf{${name}}"" & "`echo $j | awk '{ print $2,$4,$6,$8,$10 }' | sed 's: : \& :g'`" "\\\\
done;

echo "---"

for i in `ls $1/results-*`;
do
    name=$(echo $i | sed 's/.*\///' | sed 's/\.csv//g' | sed 's:results-::g' );
    j=`cat $i | egrep '^mean-true'  | tr ',' ' ' | awk '{ print $2*1000, $3*1000, $4*1000 }'`
    k=`cat $i | egrep '^mean-false'  | tr ',' ' ' | awk '{ print $2*1000, $3*1000, $4*1000 }'`
    #echo "\\textbf{${name}}"" & "`echo $j $k | egrep -o '(\ |[0-9]\.[0-9]{0,3})' | sed 's:\ : \& :g' `" "\\\\
done;

echo "---"

ARRAY=('ff' "ER" 'pl' '^V')
declare -a F;
declare -a AVG;
AVG=(0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0)

TMP=(`ls $1/output-* | egrep -v 'D(3|4|5|10)'`)

for i in ${TMP[@]}; do
    j=`echo $i | sed 's:output:results:g' | sed 's:txt:csv:g'`
    n=`cat $i | egrep -o '\|V\|=[0-9]*' | head -n 1 | egrep -o '[0-9]*'`
    echo $n" "$j
done | sort -n -k 1 | awk '{ print $2 }' > 'output.txt'

F=( $(cat output.txt) )

echo "---"

for k in ${ARRAY[@]}; do
    for i in ${F[@]}
    do
        if ! [[ -e $i ]];
        then
            continue;
        fi
        name=$(echo $i | sed 's/.*\///' | sed 's/\.csv//g' | sed 's:results-::g' );
        K=`echo $name | egrep $k`;
        #echo ${k}" "${K}" "${name}" "${i}
        if ! [[ -z "${K}" ]];
        then
            J=`cat $i | egrep '^mean-true'  | tr ',' ' ' | awk '{ print $2*1000, $3*1000, $4*1000 }'`
            K=`cat $i | egrep '^mean-false'  | tr ',' ' ' | awk '{ print $2*1000, $3*1000, $4*1000 }'`
            #echo "\\textbf{${name}}"" & "`echo $J $K | egrep -o '(\ |[0-9]\.[0-9]{0,3})' | sed 's:\ : \& :g' `" "\\\\
        fi;
    done;
    echo "\\midrule"
done;

for i in ${F[@]}
do
    if ! [[ -e $i ]];
    then
        continue;
    fi
    name=$(echo $i | sed 's/.*\///' | sed 's/\.csv//g' | sed 's:results-::g' );
    if ! [[ -z `echo $name | egrep -v '(ff|ER|pl|^V[0-9]*)'` ]];
    then
        J=`cat $i | egrep '^mean-true'  | tr ',' ' ' | awk '{ print $2*1000, $3*1000, $4*1000 }'`
        K=`cat $i | egrep '^mean-false'  | tr ',' ' ' | awk '{ print $2*1000, $3*1000, $4*1000 }'`
        #echo "\\textbf{${name}}"" & "`echo $J $K | egrep -o '(\ |[0-9]\.[0-9]{0,3})' | sed 's:\ : \& :g' `" "\\\\
    fi;
done;

echo "---"

for k in ${ARRAY[@]}; do
    for i in ${F[@]}
    do
        if ! [[ -e $i ]];
        then
            continue;
        fi
        name=$(echo $i | sed 's/.*\///' | sed 's/\.csv//g' | sed 's:results-::g' );
        if ! [[ -z `echo $name | egrep $k` ]];
        then
            j=`cat $i | head -n 9 | tr '\ ' '&' |  egrep -o '[0-9_]*\.[0-9_]{0,2}' | tr '_' ','`;
            #echo "\\textbf{${name}}"" & "`echo $j | awk '{ print $1,$3,$5,$7,$9 }' | sed 's: : \& :g'`" "\\\\
        fi;
    done;
    echo "\\midrule"
done;

echo "---"

for k in ${ARRAY[@]}; do
    for i in ${F[@]}
    do
        if ! [[ -e $i ]];
        then
            continue;
        fi
        name=$(echo $i | sed 's/.*\///' | sed 's/\.csv//g' | sed 's:results-::g' );
        if ! [[ -z `echo $name | egrep $k` ]];
        then
            j=`cat $i | head -n 9 | tr '\ ' '&' |  egrep -o '[0-9_]*\.[0-9_]{0,2}' | tr '_' ','`;
            #echo "\\textbf{${name}}"" & "`echo $j | awk '{ print $2,$4,$6,$8,$10 }' | sed 's: : \& :g'`" "\\\\
        fi;
    done;
    echo "\\midrule"
done;

echo "---"

for k in ${ARRAY[@]}; do
    for i in ${F[@]}
    do
        if ! [[ -e $i ]];
        then
            continue;
        fi
        name=$(echo $i | sed 's/.*\///' | sed 's/\.csv//g' | sed 's:results-::g' );
        if ! [[ -z `echo $name | egrep $k` ]];
        then
            j=`cat $i | tail -n 1 | tr '\ ' '&' |  egrep -o '[0-9_]*\.[0-9_]{0,2}' | tr '_' ','`;
            #echo "\\textbf{${name}}"" & "`echo $j | awk 'BEGIN { i=1; j=6; } { print $i,$(i+1),$(i+j),$(i+j+1),$(i+2*j),$(i+2*j+1),$(i+3*j),$(i+3*j+1); }' | sed 's: : \& :g'`" "\\\\
            #echo "\\textbf{${name}}"" & "`echo $j | awk 'BEGIN { i=37; j=6; } { print $i,$(i+1),$(i+2),$(i+3),$(i+4),$(i+5) }' | sed 's: : \& :g'`" "\\\\
        fi;
    done;
    echo "\\midrule"
done;


for i in ${F[@]}
do
    if ! [[ -e $i ]];
    then
        continue;
    fi
    name=$(echo $i | sed 's/.*\///' | sed 's/\.csv//g' | sed 's:results-::g' );
    if ! [[ -z `echo $name | egrep -v '(ff|ER|pl|^V[0-9]*)'` ]];
    then
        j=`cat $i | tail -n 1 | tr '\ ' '&' |  egrep -o '[0-9_]*\.[0-9_]{0,2}' | tr '_' ','`;
        #echo "\\textbf{${name}}"" & "`echo $j | awk 'BEGIN { i=3; j=6; } { print $i,$(i+1),$(i+j),$(i+j+1),$(i+2*j),$(i+2*j+1),$(i+3*j),$(i+3*j+1); }' | sed 's: : \& :g'`" "\\\\
        #echo "\\textbf{${name}}"" & "`echo $j | awk 'BEGIN { i=37; j=6; } { print $i,$(i+1),$(i+2),$(i+3),$(i+4),$(i+5) }' | sed 's: : \& :g'`" "\\\\
    fi;
done;

A=0
B=4
for i in ${F[@]}
do
    if ! [[ -e $i ]];
    then
        continue;
    fi

    j=`cat $i | tail -n 1 | tr '\ ' '&' | sed 's:_::g'| egrep -o '[0-9]*\.[0-9]{0,2}'`;
    j1="`echo $j | awk 'BEGIN { method=$A; methods=$B; } { print $(1) }'`"
    j2="`echo $j | awk 'BEGIN { method=$A; methods=$B; } { print $(2) }'`"
    j3="`echo $j | awk 'BEGIN { method=$A; methods=$B; } { print $(7) }'`"
    j4="`echo $j | awk 'BEGIN { method=$A; methods=$B; } { print $(8) }'`"
    j5="`echo $j | awk 'BEGIN { method=$A; methods=$B; } {  print $(13) }'`"
    j6="`echo $j | awk 'BEGIN { method=$A; methods=$B; } {  print $(14) }'`"
    j7="`echo $j | awk 'BEGIN { method=$A; methods=$B; } {  print $(19) }'`"
    j8="`echo $j | awk 'BEGIN { method=$A; methods=$B; } {  print $(20) }'`"

    AVG[0]=`echo "scale=4;${AVG[0]}+$j1" | bc`
    AVG[1]=`echo "scale=4;${AVG[1]}+$j2" | bc`
    AVG[2]=`echo "scale=4;${AVG[2]}+$j3" | bc`
    AVG[3]=`echo "scale=4;${AVG[3]}+$j4" | bc`
    AVG[4]=`echo "scale=4;${AVG[4]}+$j5" | bc`
    AVG[5]=`echo "scale=4;${AVG[5]}+$j6" | bc`
    AVG[6]=`echo "scale=4;${AVG[6]}+$j7" | bc`
    AVG[7]=`echo "scale=4;${AVG[7]}+$j8" | bc`

    #echo $i" "$j1" "$j2" "$j3" "$j4" "$j5" "$j6" "$j7" "$j8
    #echo $j;
    #echo $j1" "$j2" "$j3" "$j4" "$j5" "$j6" "$j7" "$j8
done;

AVG[0]=`echo "scale=4;${AVG[0]}/${#F[@]}" | bc | egrep -o '[0-9]*\.[0-9]{0,2}'`
AVG[1]=`echo "scale=4;${AVG[1]}/${#F[@]}" | bc| egrep -o '[0-9]*\.[0-9]{0,2}'`
AVG[2]=`echo "scale=4;${AVG[2]}/${#F[@]}" | bc | egrep -o '[0-9]*\.[0-9]{0,2}'`
AVG[3]=`echo "scale=4;${AVG[3]}/${#F[@]}" | bc| egrep -o '[0-9]*\.[0-9]{0,2}'`
AVG[4]=`echo "scale=4;${AVG[4]}/${#F[@]}" | bc| egrep -o '[0-9]*\.[0-9]{0,2}'`
AVG[5]=`echo "scale=4;${AVG[5]}/${#F[@]}" | bc| egrep -o '[0-9]*\.[0-9]{0,2}'`
AVG[6]=`echo "scale=4;${AVG[6]}/${#F[@]}" | bc| egrep -o '[0-9]*\.[0-9]{0,2}'`
AVG[7]=`echo "scale=4;${AVG[7]}/${#F[@]}" | bc| egrep -o '[0-9]*\.[0-9]{0,2}'`

echo " & "${AVG[0]}" & "${AVG[1]}" & "${AVG[2]}" & "${AVG[3]}" & "${AVG[4]}" & "${AVG[5]}" & "${AVG[6]}" & "${AVG[7]}\\\\;
