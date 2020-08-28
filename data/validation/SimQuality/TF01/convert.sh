#!/bin/bash

python ../../../../scripts/Python/tsvtool.py --output Potsdam.tsv extract_columns SimQuality_TF01Var01/results/loads.tsv "0|1|2"  "Minute of Year [min]|Potsdam.Altitude [Deg]|Potsdam.Azimuth [Deg]"
python ../../../../scripts/Python/tsvtool.py --output Shanghai.tsv extract_columns SimQuality_TF01Var02/results/loads.tsv "1|2"  "Shanghai.Altitude [Deg]|Shanghai.Azimuth [Deg]"
python ../../../../scripts/Python/tsvtool.py --output Kaxgar.tsv extract_columns  SimQuality_TF01Var03/results/loads.tsv "1|2"  "Kaxgar.Altitude [Deg]|Kaxgar.Azimuth [Deg]"
python ../../../../scripts/Python/tsvtool.py --output Singapur.tsv extract_columns  SimQuality_TF01Var04/results/loads.tsv "1|2"  "Singapur.Altitude [Deg]|Singapur.Azimuth [Deg]"
python ../../../../scripts/Python/tsvtool.py --output Lima.tsv extract_columns  SimQuality_TF01Var05/results/loads.tsv "1|2"  "Lima.Altitude [Deg]|Lima.Azimuth [Deg]"
python ../../../../scripts/Python/tsvtool.py --output Denver.tsv extract_columns  SimQuality_TF01Var06/results/loads.tsv "1|2"  "Denver.Altitude [Deg]| Denver.Azimuth [Deg]"
python ../../../../scripts/Python/tsvtool.py --output Barrow.tsv extract_columns  SimQuality_TF01Var07/results/loads.tsv "1|2"  "Barrow.Altitude [Deg]|Barrow.Azimuth [Deg]"
python ../../../../scripts/Python/tsvtool.py --output Melbourne.tsv extract_columns  SimQuality_TF01Var08/results/loads.tsv "1|2"  "Melbourne.Altitude [Deg]|Melbourne.Azimuth [Deg]"

# insert columns in Result temp file
 
python ../../../../scripts/Python/tsvtool.py --output Result.tsv insert_columns Potsdam.tsv Potsdam.tsv 0
python ../../../../scripts/Python/tsvtool.py --output Result.tsv insert_columns Potsdam.tsv Shanghai.tsv 4
python ../../../../scripts/Python/tsvtool.py --output Result.tsv insert_columns Result.tsv Kaxgar.tsv 7
python ../../../../scripts/Python/tsvtool.py --output Result.tsv insert_columns Result.tsv Singapur.tsv 10 
python ../../../../scripts/Python/tsvtool.py --output Result.tsv insert_columns Result.tsv Lima.tsv 13
python ../../../../scripts/Python/tsvtool.py --output Result.tsv insert_columns Result.tsv Denver.tsv 16
python ../../../../scripts/Python/tsvtool.py --output Result.tsv insert_columns Result.tsv Barrow.tsv 19
python ../../../../scripts/Python/tsvtool.py --output Result.tsv insert_columns Result.tsv Melbourne.tsv 22

# delete temp files
rm Potsdam.tsv Shanghai.tsv Kaxgar.tsv Singapur.tsv Lima.tsv Denver.tsv Barrow.tsv Melbourne.tsv

# output dayfiles

python ../../../../scripts/Python/tsvtool.py --output results/NANDRAD2_Mar5.tsv extract_rows Result.tsv 0-1440
python ../../../../scripts/Python/tsvtool.py --output results/NANDRAD2_Jul27.tsv extract_rows Result.tsv 1441-2881
python ../../../../scripts/Python/tsvtool.py --output results/NANDRAD2_Sep22.tsv extract_rows Result.tsv 2882-4322
python ../../../../scripts/Python/tsvtool.py --output results/NANDRAD2_Oct24.tsv extract_rows Result.tsv 4323-5763
python ../../../../scripts/Python/tsvtool.py --output results/NANDRAD2_Dec17.tsv extract_rows Result.tsv 5764-7204

rm Result.tsv

