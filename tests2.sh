#!/bin/sh

THRESHOLD=34
THRESHOLDJUMP=1
THRESHOLDMAX=200
EXTEND=false
CONFIDENCETHRESHOLD=0.05
BLEACHINGSTART=0
BLEACHINGMAX=11
BLEACHINGJUMP=1
NUMBITSADDRSSTART=2
NUMBITSADDRSJUMP=1
NUMBITSADDRSMAX=31

MAXRIGHTS=0
MAXRIGHTSBLEACHING=0
MAXRIGHTSTHRESHOLD=0
MAXRIGHTSNUMBITSADDRS=0

make

NUMBITSADDRS=$NUMBITSADDRSSTART

while [ $NUMBITSADDRS -lt $NUMBITSADDRSMAX ]
do
    BLEACHING=$BLEACHINGSTART
    while [ $BLEACHING -lt $BLEACHINGMAX ]
    do
        ./bin/mnistWisard.out 0 $NUMBITSADDRS $BLEACHING $CONFIDENCETHRESHOLD 0 0 $THRESHOLD > ./results/$NUMBITSADDRS-$BLEACHING-$THRESHOLD-sequencial.txt
        ./bin/mnistWisard.out 1 $NUMBITSADDRS $BLEACHING $CONFIDENCETHRESHOLD 0 2 $THRESHOLD > ./results/$NUMBITSADDRS-$BLEACHING-$THRESHOLD-threads2.txt
        ./bin/mnistWisard.out 1 $NUMBITSADDRS $BLEACHING $CONFIDENCETHRESHOLD 0 4 $THRESHOLD > ./results/$NUMBITSADDRS-$BLEACHING-$THRESHOLD-threads4.txt
        echo "Done ./results/$NUMBITSADDRS-$BLEACHING-$THRESHOLD"
        line1=$(head -n 1 ./results/$NUMBITSADDRS-$BLEACHING-$THRESHOLD-sequencial.txt)
        line2=$(head -n 1 ./results/$NUMBITSADDRS-$BLEACHING-$THRESHOLD-threads2.txt)
        line3=$(head -n 1 ./results/$NUMBITSADDRS-$BLEACHING-$THRESHOLD-threads4.txt)
        media=$((($line1+$line2+$line3)/3))
        if [ $media -gt $MAXRIGHTS ]
        then
            MAXRIGHTS=$media
            MAXRIGHTSBLEACHING=$BLEACHING
            MAXRIGHTSNUMBITSADDRS=$NUMBITSADDRS
        fi
        BLEACHING=`expr $BLEACHING + $BLEACHINGJUMP`
    done
    echo "Max rights: $MAXRIGHTS - Bleaching: $MAXRIGHTSBLEACHING - Num bits addrs: $MAXRIGHTSNUMBITSADDRS"
    NUMBITSADDRS=`expr $NUMBITSADDRS + $NUMBITSADDRSJUMP`
done
echo "Max rights: $MAXRIGHTS - Bleaching: $MAXRIGHTSBLEACHING - Num bits addrs: $MAXRIGHTSNUMBITSADDRS"
