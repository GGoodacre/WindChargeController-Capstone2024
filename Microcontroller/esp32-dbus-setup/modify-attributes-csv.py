# modify-attributes-csv.py
# note: too lazy to implement file path finding, just drop this into the same directory and execute it

# library for handling csv file i/o
import csv

def main():
    # temp storage for information read from the current attributes.csv file
    read_lines = []
    
    # opening current attributes.csv file for reading and storing
    with open('/opt/victronenergy/dbus-modbustcp/attributes.csv', mode = 'r') as current_csv_file: 
        current_csv_reader = csv.reader(current_csv_file) 
        for line in current_csv_reader: 
            read_lines.append(line)
    current_csv_file.close()

    # writing a backup of the current attributes.csv file in case reverting is desired
    with open('/opt/victronenergy/dbus-modbustcp/attributes_backup.csv', mode = 'w', newline = '') as backup_csv_file:
        backup_csv_writer = csv.writer(backup_csv_file)
        for line in read_lines:
            backup_csv_writer.writerow(line)
    backup_csv_file.close()

    # writing over the current attributes.csv file to make all modbus tcp registers writable
    with open('/opt/victronenergy/dbus-modbustcp/attributes.csv', mode = 'w', newline = '') as next_csv_file:
        next_csv_writer = csv.writer(next_csv_file)
        for line in read_lines:
            line[len(line)-1] = 'W'
            next_csv_writer.writerow(line)
    backup_csv_file.close()

if __name__ == "__main__":
    main()