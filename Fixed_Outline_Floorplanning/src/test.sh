#!/bin/bash

#for sitenu in $(seq 1 350)
#do
	#str_sitenu="$str_sitenu$sitenu"
  time ./hw3 ../testcase/n100.hardblocks ../testcase/n100.nets ../testcase/n100.pl ./n100_1.out 0.1 >> ./n100_1.rpt
  time ./hw3 ../testcase/n100.hardblocks ../testcase/n100.nets ../testcase/n100.pl ./n100_15.out 0.15 >> ./n100_15.rpt
  time ./hw3 ../testcase/n200.hardblocks ../testcase/n200.nets ../testcase/n200.pl ./n200_1.out 0.1 >> ./n200_1.rpt
  time ./hw3 ../testcase/n200.hardblocks ../testcase/n200.nets ../testcase/n200.pl ./n200_15.out 0.15 >> ./n200_15.rpt
  time ./hw3 ../testcase/n300.hardblocks ../testcase/n300.nets ../testcase/n300.pl ./n300_1.out 0.1 >> ./n300_1.rpt
  time ./hw3 ../testcase/n300.hardblocks ../testcase/n300.nets ../testcase/n300.pl ./n300_15.out 0.15 >> ./n300_15.rpt
#done
