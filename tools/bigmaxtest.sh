MODEL=0 # we start with the geometric tests
DATE=`date +%Y-%m-%d`
REPEAT=2048 # let us be generous
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

for i in $(seq 4 20);
do
      SIZE=$((2**i))
      for MODEL in $(seq 0 2)
      do
            outputfile=bigmax_"$DATE"_"$MODEL"_"$SIZE"_"$REPEAT".txt
            echo "Producing $outputfile ..."
            set -x #echo on
            $DIR/maxprobedistribution.py "$MODEL" "$SIZE" "$REPEAT" > $outputfile
            set +x #echo off
            sleep 1
            echo "Done"
      done
done
