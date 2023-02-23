GCC = g++
OUT = match_table

all: match_table.cpp
	python3 get_keys.py > keyinfo.txt
	$(GCC) -o $(OUT) match_table.cpp && ./match_table > table_info.txt
	
