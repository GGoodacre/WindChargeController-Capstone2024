import re
import csv

output_file = open("out.csv", 'w')
log_file = open("this.log", 'r', encoding='latin-1')
Lines = log_file.readlines()


regex_line = re.compile("[:]\s([^\x1B]*)[\x1B]")
regex_main = re.compile("(?:CSV)")

output_file.flush()

for line in Lines:
    if re.search(regex_main, line):
        match = re.findall(regex_line, line)
        output_file.write(match[0] + "\n")

print("I Finished")
output_file.close()
log_file.close()