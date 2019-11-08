.PHONY: build_parser build_vm clean

output_folder = build
parser_output = $(output_folder)/parser
vm_output = $(output_folder)/vm

parser_source_files = src/parser.cpp
vm_source_files = src/vm.cpp


build_parser:
	mkdir -p $(output_folder)
	clang++ -std=c++17 -DDEBUG $(parser_source_files) -o $(parser_output) -Weverything -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-padded -Wno-exit-time-destructors -Wno-global-constructors -Wno-newline-eof

build_vm:
	mkdir -p $(output_folder)
	clang++ -std=c++17 -DDEBUG $(vm_source_files) -o $(vm_output) -Weverything -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-padded -Wno-exit-time-destructors -Wno-global-constructors -Wno-newline-eof

clean:
	rm $(output)