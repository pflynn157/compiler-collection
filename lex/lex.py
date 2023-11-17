#!/usr/bin/python3
import sys
import os
from shutil import copyfile

sys.path.append(os.getcwd())

import config
keywords = config.keywords
symbols = dict()

for item in config.symbols:
	name = item[0]
	value = item[1]
	key = value[0]		# This is the first character of the symbol sequence
	if key in symbols:
		symbols[key].append((value, name))
	else:
		symbols[key] = [(value, name)]

base_path = sys.argv[1]

#copyfile(base_path + "/lex.cpp", "./lex.cpp")
#copyfile(base_path + "/lex.hpp", "./lex.hpp")

##
## Generate the header file
##
writer = open("./lex.hpp", "w")

with open(base_path + "/lex.hpp", "r") as reader:
    for line in reader:
        ln = line.strip()
        
        if ln == "///LEX_KEYWORDS":
            for keyword in keywords:
                writer.write("\t" + keyword[0] + ",\n")
        
        elif ln == "///LEX_SYMBOLS":
            for sym in config.symbols:
                writer.write("\t" + sym[0] + ",\n")
        
        else:
            writer.write(line)

writer.close()

##
## Generate the source file
##
writer = open("./lex.cpp", "w")

with open(base_path + "/lex.cpp", "r") as reader:
    for line in reader:
        ln = line.strip()
        
        # Keyword if-else block
        if ln == "///LEX_KEYWORD_CHECK":
            first = True
            for keyword in keywords:
                writer.write("\t\t\t")
                if first:
                    writer.write("if ")
                    first = False
                else:
                    writer.write("else if ")
                writer.write("(buffer == \"" + keyword[1] + "\") t = " + keyword[0] + ";\n")
        
        # Keyword debug section
        elif ln == "///LEX_KEYWORD_DEBUG":
            for keyword in keywords:
                writer.write("\t\tcase " + keyword[0] + ": std::cout << \"" + keyword[1] + "\" << std::endl; break;\n")
        
        # Symbol debug section
        elif ln == "///LEX_SYMBOL_DEBUG":
            for sym in config.symbols:
                writer.write("\t\tcase " + sym[0] + ": std::cout << \"" + sym[1] + "\" << std::endl; break;\n")
        
        # Symbol checking
        elif ln == "///LEX_SYMBOL_CHECK":
            for value, name in symbols.items():
                writer.write("\t\tcase \'" + value[0] + "\': return true;\n")
        
        # Return the proper symbol
        elif ln == "///LEX_SYMBOL_RETURN":
            for value, name_list in symbols.items():
                if len(name_list) == 1 and len(name_list[0][0]) == 1:
                    writer.write("\t\tcase \'" + value + "\': return " + name_list[0][1] + ";\n")
                else:
                    writer.write("\t\tcase \'" + value + "\': {\n")
                    writer.write("\t\t\tchar c2 = reader.get();\n")

                    found_first = False
                    default_name = None

                    for item in name_list:
                        if len(item[0]) == 1:
                            default_name = item[1]
                            continue

                        symbol = item[0]
                        name = item[1]
                        
                        if found_first:
                            writer.write("\t\t\t} else ")
                        found_first = True
                        writer.write("\t\t\tif (c2 == \'" + symbol[1] + "\') {\n")
                        writer.write("\t\t\t\treturn " + name + ";\n")
                        
                    # Final else statement
                    writer.write("\t\t\t} else {\n")
                    writer.write("\t\t\t\treader.unget();\n")
                    if default_name != None:
                        writer.write("\t\t\t\treturn " + default_name + ";\n")
                    writer.write("\t\t\t}\n")

                    # Closing brace of break statement
                    writer.write("\t\t} break;\n")
                
        else:
            writer.write(line)

writer.close()

