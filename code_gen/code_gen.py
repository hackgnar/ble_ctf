#!/usr/bin/python
import csv
import fileinput
import shutil

flag_file_data = {}

#todo: generate gatt server load conditional
#todo: generate/change main methods in each gatt file and header
#todo: codegen flag values

def copy_gatt_server_files():
    pass

def import_flag_file_data(filename):
    i = 0
    with open(filename) as csvfile:
        reader = csv.DictReader(csvfile, skipinitialspace=True)
        for row in reader:
            flag = "flag_" + str(i)
            flag_file_data[flag] = row
            i+=1
    print("######### DEBUG #########")
    print(flag_file_data)
    print("#########################")

#//CODEGEN_HEADER_FLAG_IDX
def generate_header_flag_idx():
    template = """
FLAG_SCOREBOARD_IDX_CHAR_READ_FLAG_%i,
FLAG_SCOREBOARD_IDX_CHAR_VAL_READ_FLAG_%i,"""
    print("######### HEADER_FLAG_IDX #########")
    for i in range(len(flag_file_data)):
        print(template % (i, i))

#//CODEGEN_INCLUDES
def generate_includes():
    template = """
#include "%s.h"""
    print("######### INCLUDES #########")
    for i in range(len(flag_file_data)):
        flag_header = flag_file_data["flag_%s" % (str(i))]["gatt_name"]
        print(template % (flag_header))

#//CODEGEN_HANDLE_LOCATIONS
def generate_handle_locations():
    template = """
static const uint16_t GATTS_CHAR_UUID_READ_FLAG_%s = 0xFF%s;"""
    print("######### HANDLE LOCATIONS #########")
    location = 6
    for i in range(len(flag_file_data)):
        hex_location = hex(location+i)[2:].zfill(2)
        print(template % (str(i), str(hex_location).upper()))

#//CODEGEN_TOTAL_FLAGS
def generate_total_flags():
    template = """
int_total_flags = %d;
string_total_flags = "%d";"""
    print("######### TOTAL FLAGS #########")
    i = len(flag_file_data)
    print(template % (i, i))

#todo: generate flag string read values
#//CODEGEN_FLAG_READ_VALUES
"""
static const char flag_xx_value[] = "Flag xx: Complete|Incomplete";
"""
def generate_flag_read_values():
    template = """
static const char flag_%d_value[] = "Flag %d: Incomplete";"""
    print("######### FLAG READ VALUES #########")
    for i in range(len(flag_file_data)):
        print(template % (i, i))

#//CODEGEN_FLAG_DECLARATIONS
def generate_flag_declarations():
    template = """
[FLAG_SCOREBOARD_IDX_CHAR_READ_FLAG_%d]      =
{{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ,
  CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_read}},

[FLAG_SCOREBOARD_IDX_CHAR_VAL_READ_FLAG_%d]  =
{{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&GATTS_CHAR_UUID_READ_DOCS, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
  GATTS_DEMO_CHAR_VAL_LEN_MAX, sizeof(flag_%d_value)-1, (uint8_t *)flag_%d_value}},"""
    print("######### FLAG DECLARATIONS #########")
    for i in range(len(flag_file_data)):
        print(template % (i, i, i, i))

#//CODEGEN_FLAG_VALIDATE_CONDITIONAL
def generate_flag_validate_conditional():
    template = """
if (strcmp(writeData, "%s") == 0){
    err = nvs_set_i32(my_handle, "flag_%d", 1);
}"""
    print("######### FLAG VALIDATE CONDITIONAL #########")
    for i in range(len(flag_file_data)):
        flag_value = flag_file_data["flag_%s" % (str(i))]["flag_value"]
        print(template % (flag_value, i))

if __name__ == "__main__":
    # user args
    filename = "flag_config.csv"
    ble_ctf_dir = "/home/ripper/src/ble_ctf"

    # default args
    gatt_server_dir = "gatt_servers"
    dashboard_dir = "gatt_servers/dashboards"
    dashboard_default = "flag_scoreboard"


    import_flag_file_data(filename)
    generate_header_flag_idx()
    generate_includes()
    generate_handle_locations()
    generate_total_flags()
    generate_flag_read_values()
    generate_flag_declarations()
    generate_flag_validate_conditional()
