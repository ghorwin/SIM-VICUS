#!/bin/bash

# Isotrop

python ../../../../scripts/Python/tsvtool.py --output ./results/NANDRAD2_Isotrop_minutely.tsv extract_columns ./SimQuality_TF02/results/loads_minutely.tsv "0|1|2|3|4|12|5|13|6|14|7|15|8|16|9|17|10|18|11|19|20|21|29|22|30|23|31|24|32|25|33|26|34|27|35|28|36" "Zeit [min]|Sun azimuth angle [Deg]|Sun elevation angle [Deg]|DIR_-_0|DIR_N_90|DIR_N_30|DIR_N-E_90|DIR_N-E_30|DIR_E_90|DIR_E_30|DIR_S-E_90|DIR_S-E_30|DIR_S_90|DIR_S_30|DIR_S-W_90|DIR_S-W_30|DIR_W_90|DIR_W_30|DIR_N-W_90|DIR_N-W_30|DIFF_-_0|DIFF_N_90|DIFF_N_30|DIFF_N-E_90|DIFF_N-E_30|DIFF_E_90|DIFF_E_30|DIFF_S-E_90|DIFF_S-E_30|DIFF_S_90|DIFF_S_30|DIFF_S-W_90|DIFF_S-W_30|DIFF_W_90|DIFF_W_30|DIFF_N-W_90|DIFF_N-W_30"


python ../../../../scripts/Python/tsvtool.py --output ./results/NANDRAD2_Isotrop_hourly.tsv extract_columns ./SimQuality_TF02/results/load_integrals.tsv "1|2|3|4|12|5|13|6|14|7|15|8|16|9|17|10|18|11|19|20|21|29|22|30|23|31|24|32|25|33|26|34|27|35|28|36" "Sun azimuth angle [Deg]|Sun elevation angle [Deg]|DIR_-_0|DIR_N_90|DIR_N_30|DIR_N-E_90|DIR_N-E_30|DIR_E_90|DIR_E_30|DIR_S-E_90|DIR_S-E_30|DIR_S_90|DIR_S_30|DIR_S-W_90|DIR_S-W_30|DIR_W_90|DIR_W_30|DIR_N-W_90|DIR_N-W_30|DIFF_-_0|DIFF_N_90|DIFF_N_30|DIFF_N-E_90|DIFF_N-E_30|DIFF_E_90|DIFF_E_30|DIFF_S-E_90|DIFF_S-E_30|DIFF_S_90|DIFF_S_30|DIFF_S-W_90|DIFF_S-W_30|DIFF_W_90|DIFF_W_30|DIFF_N-W_90|DIFF_N-W_30"

# finally insert time [h] column into result file 'NANDRAD2_Isotrop_hourly.tsv'
python ../../../../scripts/Python/tsvtool.py --output ./results/NANDRAD2_Isotrop_hourly.tsv insert_columns ./results/NANDRAD2_Isotrop_hourly.tsv Hourly_TimeColumn.tsv 0

# With Perez
python ../../../../scripts/Python/tsvtool.py --output ./results/NANDRAD2_Perez_minutely.tsv extract_columns ./SimQuality_TF02_Perez/results/loads_minutely.tsv "0|1|2|3|4|12|5|13|6|14|7|15|8|16|9|17|10|18|11|19|20|21|29|22|30|23|31|24|32|25|33|26|34|27|35|28|36" "Zeit [min]|Sun azimuth angle [Deg]|Sun elevation angle [Deg]|DIR_-_0|DIR_N_90|DIR_N_30|DIR_N-E_90|DIR_N-E_30|DIR_E_90|DIR_E_30|DIR_S-E_90|DIR_S-E_30|DIR_S_90|DIR_S_30|DIR_S-W_90|DIR_S-W_30|DIR_W_90|DIR_W_30|DIR_N-W_90|DIR_N-W_30|DIFF_-_0|DIFF_N_90|DIFF_N_30|DIFF_N-E_90|DIFF_N-E_30|DIFF_E_90|DIFF_E_30|DIFF_S-E_90|DIFF_S-E_30|DIFF_S_90|DIFF_S_30|DIFF_S-W_90|DIFF_S-W_30|DIFF_W_90|DIFF_W_30|DIFF_N-W_90|DIFF_N-W_30"


python ../../../../scripts/Python/tsvtool.py --output ./results/NANDRAD2_Perez_hourly.tsv extract_columns ./SimQuality_TF02_Perez/results/load_integrals.tsv "1|2|3|4|12|5|13|6|14|7|15|8|16|9|17|10|18|11|19|20|21|29|22|30|23|31|24|32|25|33|26|34|27|35|28|36" "Sun azimuth angle [Deg]|Sun elevation angle [Deg]|DIR_-_0|DIR_N_90|DIR_N_30|DIR_N-E_90|DIR_N-E_30|DIR_E_90|DIR_E_30|DIR_S-E_90|DIR_S-E_30|DIR_S_90|DIR_S_30|DIR_S-W_90|DIR_S-W_30|DIR_W_90|DIR_W_30|DIR_N-W_90|DIR_N-W_30|DIFF_-_0|DIFF_N_90|DIFF_N_30|DIFF_N-E_90|DIFF_N-E_30|DIFF_E_90|DIFF_E_30|DIFF_S-E_90|DIFF_S-E_30|DIFF_S_90|DIFF_S_30|DIFF_S-W_90|DIFF_S-W_30|DIFF_W_90|DIFF_W_30|DIFF_N-W_90|DIFF_N-W_30"

# finally insert time [h] column into result file 'NANDRAD2_Perez_hourly.tsv'
python ../../../../scripts/Python/tsvtool.py --output ./results/NANDRAD2_Perez_hourly.tsv insert_columns ./results/NANDRAD2_Perez_hourly.tsv Hourly_TimeColumn.tsv 0
