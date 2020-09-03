#!/bin/bash


python ../../../../scripts/Python/tsvtool.py --output Potsdammindir.tsv extract_columns SimQuality_TF02/results/loads_minutely.tsv "0|1|2|3|4|12|20|28|5|13|21|29|6|14|22|30|7|15|23|31|8|16|24|32|9|17|25|33|10|18|26|34|11|19|27|35" "Time [min]|Sun azimuth angle [Deg]|Sun elevation angle [Deg]|DIR_-_0|DIR_N_90|DIR_N_60|DIR_N_45|DIR_N_30|DIR_N-E_90|DIR_N-E_60|DIR_N-E_45|DIR_N-E_30|DIR_E_90|DIR_E_60|DIR_E_45|DIR_E_30|DIR_S-E_90|DIR_S-E_60|DIR_S-E_45|DIR_S-E_30|DIR_S_90|DIR_S_60|DIR_S_45|DIR_S_30|DIR_S-W_90|DIR_S-W_60|DIR_S-W_45|DIR_S-W_30|DIR_W_90|DIR_W_60|DIR_W_45|DIR_W_30|DIR_N-W_90|DIR_N-W_60|DIR_N-W_45|DIR_N-W_30"

python ../../../../scripts/Python/tsvtool.py --output Potsdamhourlydir.tsv extract_columns SimQuality_TF02/results/loads_hourly.tsv "0|1|2|3|4|12|20|28|5|13|21|29|6|14|22|30|7|15|23|31|8|16|24|32|9|17|25|33|10|18|26|34|11|19|27|35" "Time [min]|Sun azimuth angle [Deg]|Sun elevation angle [Deg]|DIR_-_0|DIR_N_90|DIR_N_60|DIR_N_45|DIR_N_30|DIR_N-E_90|DIR_N-E_60|DIR_N-E_45|DIR_N-E_30|DIR_E_90|DIR_E_60|DIR_E_45|DIR_E_30|DIR_S-E_90|DIR_S-E_60|DIR_S-E_45|DIR_S-E_30|DIR_S_90|DIR_S_60|DIR_S_45|DIR_S_30|DIR_S-W_90|DIR_S-W_60|DIR_S-W_45|DIR_S-W_30|DIR_W_90|DIR_W_60|DIR_W_45|DIR_W_30|DIR_N-W_90|DIR_N-W_60|DIR_N-W_45|DIR_N-W_30"

python ../../../../scripts/Python/tsvtool.py --output Potsdammeandir.tsv extract_columns SimQuality_TF02/results/load_integrals.tsv "0|1|2|3|4|12|20|28|5|13|21|29|6|14|22|30|7|15|23|31|8|16|24|32|9|17|25|33|10|18|26|34|11|19|27|35" "Time [min]|Sun azimuth angle [Deg]|Sun elevation angle [Deg]|DIR_-_0|DIR_N_90|DIR_N_60|DIR_N_45|DIR_N_30|DIR_N-E_90|DIR_N-E_60|DIR_N-E_45|DIR_N-E_30|DIR_E_90|DIR_E_60|DIR_E_45|DIR_E_30|DIR_S-E_90|DIR_S-E_60|DIR_S-E_45|DIR_S-E_30|DIR_S_90|DIR_S_60|DIR_S_45|DIR_S_30|DIR_S-W_90|DIR_S-W_60|DIR_S-W_45|DIR_S-W_30|DIR_W_90|DIR_W_60|DIR_W_45|DIR_W_30|DIR_N-W_90|DIR_N-W_60|DIR_N-W_45|DIR_N-W_30"

python ../../../../scripts/Python/tsvtool.py --output Potsdammindif.tsv extract_columns SimQuality_TF02/results/loads_minutely.tsv "0|1|2|3|37|45|53|61|38|46|54|62|39|47|55|63|40|48|56|64|41|49|57|65|42|50|58|66|43|51|59|67|44|52|60|68" "Time [min]|Sun azimuth angle [Deg]|Sun elevation angle [Deg]|DIF_-_0|DIF_N_90|DIF_N_60|DIF_N_45|DIF_N_30|DIF_N-E_90|DIF_N-E_60|DIF_N-E_45| DIF_N-E_30|DIF_E_90|DIF_E_60|DIF_E_45|DIF_E_30|DIF_S-E_90|DIF_S-E_60|DIF_S-E_45|DIF_S-E_30|DIF_S_90|DIF_S_60|DIF_S_45|DIF_S_30|DIF_S-W_90|DIF_S-W_60|DIF_S-W_45|DIF_S-W_30|DIF_W_90|DIF_W_60|DIF_W_45|DIF_W_30|DIF_N-W_90|DIF_N-W_60|DIF_N-W_45|DIF_N-W_30"

python ../../../../scripts/Python/tsvtool.py --output Potsdamhourlydif.tsv extract_columns SimQuality_TF02/results/loads_hourly.tsv "0|1|2|3|37|45|53|61|38|46|54|62|39|47|55|63|40|48|56|64|41|49|57|65|42|50|58|66|43|51|59|67|44|52|60|68" "Time [min]|Sun azimuth angle [Deg]|Sun elevation angle [Deg]|DIF_-_0|DIF_N_90|DIF_N_60|DIF_N_45|DIF_N_30|DIF_N-E_90|DIF_N-E_60|DIF_N-E_45| DIF_N-E_30|DIF_E_90|DIF_E_60|DIF_E_45|DIF_E_30|DIF_S-E_90|DIF_S-E_60|DIF_S-E_45|DIF_S-E_30|DIF_S_90|DIF_S_60|DIF_S_45|DIF_S_30|DIF_S-W_90|DIF_S-W_60|DIF_S-W_45|DIF_S-W_30|DIF_W_90|DIF_W_60|DIF_W_45|DIF_W_30|DIF_N-W_90|DIF_N-W_60|DIF_N-W_45|DIF_N-W_30"

python ../../../../scripts/Python/tsvtool.py --output PotsdamMeanDif.tsv extract_columns SimQuality_TF02/results/load_integrals.tsv "0|1|2|3|37|45|53|61|38|46|54|62|39|47|55|63|40|48|56|64|41|49|57|65|42|50|58|66|43|51|59|67|44|52|60|68" "Time [min]|Sun azimuth angle [Deg]|Sun elevation angle [Deg]|DIF_-_0|DIF_N_90|DIF_N_60|DIF_N_45|DIF_N_30|DIF_N-E_90|DIF_N-E_60|DIF_N-E_45| DIF_N-E_30|DIF_E_90|DIF_E_60|DIF_E_45|DIF_E_30|DIF_S-E_90|DIF_S-E_60|DIF_S-E_45|DIF_S-E_30|DIF_S_90|DIF_S_60|DIF_S_45|DIF_S_30|DIF_S-W_90|DIF_S-W_60|DIF_S-W_45|DIF_S-W_30|DIF_W_90|DIF_W_60|DIF_W_45|DIF_W_30|DIF_N-W_90|DIF_N-W_60|DIF_N-W_45|DIF_N-W_30"


# dir
python ../../../../scripts/Python/tsvtool.py --output results/Mar5_min_dir.tsv extract_rows Potsdammindir.tsv 0-1440
python ../../../../scripts/Python/tsvtool.py --output results/Jul27_min_dir.tsv extract_rows Potsdammindir.tsv 1441-2881
python ../../../../scripts/Python/tsvtool.py --output results/Sep22_min_dir.tsv extract_rows Potsdammindir.tsv 2882-4322
python ../../../../scripts/Python/tsvtool.py --output results/Oct24_min_dir.tsv extract_rows Potsdammindir.tsv 4323-5763
python ../../../../scripts/Python/tsvtool.py --output results/Dec17_min_dir.tsv extract_rows Potsdammindir.tsv 5764-7204

python ../../../../scripts/Python/tsvtool.py --output results/Mar5_hourly_dir.tsv extract_rows Potsdamhourlydir.tsv 1512-1535
python ../../../../scripts/Python/tsvtool.py --output results/Jul27_hourly_dir.tsv extract_rows Potsdamhourlydir.tsv 4968-4991
python ../../../../scripts/Python/tsvtool.py --output results/Sep22_hourly_dir.tsv extract_rows Potsdamhourlydir.tsv 6336-6359
python ../../../../scripts/Python/tsvtool.py --output results/Oct24_hourly_dir.tsv extract_rows Potsdamhourlydir.tsv 7104-7127
python ../../../../scripts/Python/tsvtool.py --output results/Dec17_hourly_dir.tsv extract_rows Potsdamhourlydir.tsv 8400-8423

python ../../../../scripts/Python/tsvtool.py --output results/Mar5_int_dir.tsv extract_rows Potsdammeandir.tsv 1512-1535
python ../../../../scripts/Python/tsvtool.py --output results/Jul27_int_dir.tsv extract_rows Potsdammeandir.tsv 4968-4991
python ../../../../scripts/Python/tsvtool.py --output results/Sep22_int_dir.tsv extract_rows Potsdammeandir.tsv 6336-6359
python ../../../../scripts/Python/tsvtool.py --output results/Oct24_int_dir.tsv extract_rows Potsdammeandir.tsv 7104-7127
python ../../../../scripts/Python/tsvtool.py --output results/Dec17_int_dir.tsv extract_rows Potsdammeandir.tsv 8400-8423


# dif
python ../../../../scripts/Python/tsvtool.py --output results/Mar5_min_dif.tsv extract_rows Potsdammindif.tsv 0-1440
python ../../../../scripts/Python/tsvtool.py --output results/Jul27_min_dif.tsv extract_rows Potsdammindif.tsv 1441-2881
python ../../../../scripts/Python/tsvtool.py --output results/Sep22_min_dif.tsv extract_rows Potsdammindif.tsv 2882-4322
python ../../../../scripts/Python/tsvtool.py --output results/Oct24_min_dif.tsv extract_rows Potsdammindif.tsv 4323-5763
python ../../../../scripts/Python/tsvtool.py --output results/Dec17_min_dif.tsv extract_rows Potsdammindif.tsv 5764-7204

python ../../../../scripts/Python/tsvtool.py --output results/Mar5_hourly_dif.tsv extract_rows Potsdamhourlydif.tsv 1512-1535
python ../../../../scripts/Python/tsvtool.py --output results/Jul27_hourly_dif.tsv extract_rows Potsdamhourlydif.tsv 4968-4991
python ../../../../scripts/Python/tsvtool.py --output results/Sep22_hourly_dif.tsv extract_rows Potsdamhourlydif.tsv 6336-6359
python ../../../../scripts/Python/tsvtool.py --output results/Oct24_hourly_dif.tsv extract_rows Potsdamhourlydif.tsv 7104-7127
python ../../../../scripts/Python/tsvtool.py --output results/Dec17_hourly_dif.tsv extract_rows Potsdamhourlydif.tsv 8400-8423

python ../../../../scripts/Python/tsvtool.py --output results/Mar5_int_dif.tsv extract_rows PotsdamMeanDif.tsv  1512-1535
python ../../../../scripts/Python/tsvtool.py --output results/Jul27_int_dif.tsv extract_rows PotsdamMeanDif.tsv  4968-4991
python ../../../../scripts/Python/tsvtool.py --output results/Sep22_int_dif.tsv extract_rows PotsdamMeanDif.tsv  6336-6359
python ../../../../scripts/Python/tsvtool.py --output results/Oct24_int_dif.tsv extract_rows PotsdamMeanDif.tsv  7104-7127
python ../../../../scripts/Python/tsvtool.py --output results/Dec17_int_dif.tsv extract_rows PotsdamMeanDif.tsv  8400-8423

# remove temporaries
rm Potsdamhourlydif.tsv Potsdammindif.tsv PotsdamMeanDif.tsv Potsdamhourlydir.tsv Potsdammindir.tsv Potsdammeandir.tsv

