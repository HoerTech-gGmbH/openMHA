#!/bin/bash

for srate in 44100 48000 ; do
    jack_control dps rate $srate >> report_delay.txt    
    for fragsize in 32 48 64 ; do
	jack_control dps period $fragsize >> report_delay.txt
	vDelay=""
	for (( n = 1; n <= 20; n++)) ; do
	    jack_control start > /dev/null
	    out=$(script -q -c 'timeout 2 jack_delay -O system:playback_1 -I system:capture_5' /dev/null | tail -n 1)
	    sleep 1
	    jack_control stop > /dev/null
	    vDelay=$vDelay" "$(awk '{print $3}' <(echo "$out"))
	done

    echo "$vDelay" >> report_delay.txt
    awk '{s+=$1}END{print "Average delay from " NR-1 " measurements:",s/(NR-1) "\n"}' RS=" " <(echo "$vDelay") >> report_delay.txt
    done
done

