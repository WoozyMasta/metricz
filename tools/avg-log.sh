#!/usr/bin/env bash

grep "MetricZ stored in" "${1:?}" | \
  awk '
  {
    t=$NF; gsub("s","",t);
    ms=t*1000;
    sum+=ms;

    if(NR==1||ms<min)
      min=ms;
    if(ms>max)
      max=ms;

    count++
  }

  END {
    if (count>0)
      printf "Count:\t%d\nMin:\t%.3f ms\nMax:\t%.3f ms\nAvg:\t%.3f ms\n", count, min, max, sum/count
  }'
