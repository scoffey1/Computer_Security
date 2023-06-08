import os
import re

# define the rules
num_docx_renames = 3
num_modifications = 3
num_self_deletes = 1

# define the path to the log file
log_path = 'log.txt'

# define the path to the ScapeGoat folder
scapegoat_path = 'ScapeGoat/'

# count the number of renamed docx files in the ScapeGoat folder
docx_renames_count = len([f for f in os.listdir(scapegoat_path) if f.endswith('.docx.')])

# count the number of modified files in the ScapeGoat folder
modifications_count = 0
for root, dirs, files in os.walk(scapegoat_path):
    for file in files:
        file_path = os.path.join(root, file)
        if os.path.getmtime(file_path) > os.path.getctime(file_path):
            modifications_count += 1

# count the number of self-deletes in the log file
self_deletes_count = 0
with open(log_path, 'r') as f:
    log_contents = f.read()
    self_deletes_count = len(re.findall(r'(?<=file created:\s).*?(?=,\sfile deleted)', log_contents))

# check if the activities are malicious
if docx_renames_count > num_docx_renames and modifications_count > num_modifications and self_deletes_count >= num_self_deletes:
    print('malware detected - HEUR:Trojan- Ransom.DocxEncrypt.Generic')
else:
    print('no malware detected')
