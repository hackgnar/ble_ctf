#!/usr/bin/python
import csv
import fileinput
import shutil
import os
import sys

flag_file_data = {}

def generate_devicename(proj_dir):
    main_dir = os.path.join(proj_dir, "main")
    sig1 = "FLAG_XX"
    sig2 = "'F','L','A','G','_','X','X'"
    for i in range(len(flag_file_data)):
        if i == 0:
            continue
        flag_name = flag_file_data["flag_%s" % (str(i))]["gatt_name"]
        dst = os.path.join(main_dir, flag_name)
        dst_c = dst + ".c"
        code_gen1 = "FLAG_%d" % (i)
        code_gen2 = "'F','L','A','G','_','0','%d'" % (i)

        f = open(dst_c,'r')
        filedata = f.read()
        f.close()
        newdata = filedata.replace(sig1,code_gen1)
        newdata = newdata.replace(sig2,code_gen2)
        f = open(dst_c,'w')
        f.write(newdata)
        f.close()
 

#todo: codegen flag values
def generate_flag_values(proj_dir):
    main_dir = os.path.join(proj_dir, "main")
    for k, v in flag_file_data.iteritems():
        sig = "//CODEGEN_FLAG_VALUES"
        template = """
strcpy(flag_%s_value, "%s");"""
        code_gen = sig
        code_gen += template % (v["gatt_name"], v["flag_value"])
        
        dst = os.path.join(main_dir, v["gatt_name"])
        dst_c = dst + ".c"

        f = open(dst_c,'r')
        filedata = f.read()
        f.close()
        newdata = filedata.replace(sig,code_gen)
        f = open(dst_c,'w')
        f.write(newdata)
        f.close()

def generate_app_main(proj_dir, dashboard):
    main_dir = os.path.join(proj_dir, "main", dashboard)
    dst_c = main_dir + '.c'
    sig = "//CODEGEN_APP_MAIN"
    code_gen_header ="""
void app_main()
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK( err );
    nvs_handle my_handle;
    err = nvs_open("storage", NVS_READWRITE, &my_handle);
    int32_t current_flag = 0; // value will default to 0, if not set yet in NVS
    if (err != ESP_OK) {
        ESP_LOGI(GATTS_TABLE_TAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
    } else {
        err = nvs_get_i32(my_handle, "current_flag", &current_flag);
        switch (err) {
            case ESP_OK:
                ESP_LOGI(GATTS_TABLE_TAG, "Retrieved current flag value");
                break;
            case ESP_ERR_NVS_NOT_FOUND:
                ESP_LOGI(GATTS_TABLE_TAG, "The value is not initialized yet!");
                break;
            default :
                ESP_LOGI(GATTS_TABLE_TAG, "Error (%s) reading!", esp_err_to_name(err));
        }
    }
    nvs_close(my_handle);
"""
    template = """
if (current_flag == %d){
    %s_main();
    }
"""
    code_gen = ""
    for i in range(len(flag_file_data)):
        code_gen += template % (i, flag_file_data["flag_%d" % i]["gatt_name"])
        i += 1
    code_gen_header = sig + code_gen_header + code_gen + '}'
    f = open(dst_c,'r')
    filedata = f.read()
    f.close()
    newdata = filedata.replace(sig,code_gen_header)
    f = open(dst_c,'w')
    f.write(newdata)
    f.close()
    

def generate_main_gatt_method_names(proj_dir):
    main_dir = os.path.join(proj_dir, "main")
    sig = "void app_main()"
    for k, v in flag_file_data.iteritems():
        dst = os.path.join(main_dir, v["gatt_name"])
        dst_c = dst + ".c"
        dst_h = dst + ".h"
        code_gen = "void %s_main()" % (v["gatt_name"])

        f = open(dst_c,'r')
        filedata = f.read()
        f.close()
        newdata = filedata.replace(sig,code_gen)
        f = open(dst_c,'w')
        f.write(newdata)
        f.close()
        f = open(dst_h,'r')
        filedata = f.read()
        f.close()
        newdata = filedata.replace(sig,code_gen)
        f = open(dst_h,'w')
        f.write(newdata)
        f.close()
 
def copy_gatt_server_files(proj_dir):
    gatt_server_dir = os.path.join(proj_dir, "gatt_servers")
    main_dir = os.path.join(proj_dir, "main")
    for k, v in flag_file_data.iteritems():
        src = os.path.join(gatt_server_dir, v["gatt_name"], "main", v["gatt_name"])
        dst = os.path.join(main_dir, v["gatt_name"])
        shutil.copyfile(src + ".c", dst + ".c")
        shutil.copyfile(src + ".h", dst + ".h")
    #copy over common files
    src = os.path.join(gatt_server_dir, "common", "gatt_server_common")
    dst = os.path.join(main_dir, "gatt_server_common")
    shutil.copyfile(src + ".c", dst + ".c")
    shutil.copyfile(src + ".h", dst + ".h")
    #copy over componet file
    src = os.path.join(gatt_server_dir, "common", "component.mk")
    dst = os.path.join(main_dir, "component.mk")
    shutil.copyfile(src, dst)

def import_flag_file_data(filename):
    i = 0
    with open(filename) as csvfile:
        reader = csv.DictReader(csvfile, skipinitialspace=True)
        for row in reader:
            flag = "flag_" + str(i)
            flag_file_data[flag] = row
            i+=1

def generate_flag_status_msgs(filename):
    sig = "//CODEGEN_FLAG_STATUS"
    template = """
if (strcmp(flag_name, "flag_%i") == 0){
    if (current_flag == 1){
        strcpy(flag_%i_value, "Flag %i: Complete  ");
    }
    else {
        strcpy(flag_%i_value, "Flag %i: Incomplete");
    }
    esp_ble_gatts_set_attr_value(blectf_handle_table[FLAG_SCOREBOARD_IDX_CHAR_READ_FLAG_%i]+1, sizeof(flag_%i_value)-1, (uint8_t *)flag_%i_value);
}
"""
    code_gen = sig
    for i in range(len(flag_file_data)):
        code_gen += template % (i, i, i, i, i, i, i, i)
    f = open(filename,'r')
    filedata = f.read()
    f.close()
    newdata = filedata.replace(sig,code_gen)
    f = open(filename,'w')
    f.write(newdata)
    f.close()


def generate_header_flag_idx(filename):
    sig = "//CODEGEN_HEADER_FLAG_IDX"
    template = """
FLAG_SCOREBOARD_IDX_CHAR_READ_FLAG_%i,
FLAG_SCOREBOARD_IDX_CHAR_VAL_READ_FLAG_%i,"""
    code_gen = sig
    for i in range(len(flag_file_data)):
        code_gen += template % (i, i)
    f = open(filename,'r')
    filedata = f.read()
    f.close()
    newdata = filedata.replace(sig,code_gen)
    f = open(filename,'w')
    f.write(newdata)
    f.close()

def generate_includes(filename):
    sig = "//CODEGEN_INCLUDES"
    template = """
#include "%s.h" """
    code_gen = sig
    f = open(filename,'r')
    filedata = f.read()
    f.close()
    for i in range(len(flag_file_data)):
        flag_header = flag_file_data["flag_%s" % (str(i))]["gatt_name"]
        if flag_header not in filedata:
            code_gen += template % (flag_header)

    newdata = filedata.replace(sig,code_gen)

    f = open(filename,'w')
    f.write(newdata)
    f.close()

def generate_handle_locations(filename):
    sig = "//CODEGEN_HANDLE_LOCATIONS"
    template = """
static const uint16_t GATTS_CHAR_UUID_READ_FLAG_%s = 0xFF%s;"""
    location = 6
    code_gen = sig
    for i in range(len(flag_file_data)):
        hex_location = hex(location+i)[2:].zfill(2)
        code_gen += template % (str(i), str(hex_location).upper())
    
    f = open(filename,'r')
    filedata = f.read()
    f.close()
    newdata = filedata.replace(sig,code_gen)
    f = open(filename,'w')
    f.write(newdata)
    f.close()

def generate_total_flags(filename):
    sig = "//CODEGEN_TOTAL_FLAGS"
    template = """
int_total_flags = %d;
strcpy(string_total_flags, "%d");"""
    code_gen = sig
    i = len(flag_file_data)
    code_gen += template % (i, i)

    f = open(filename,'r')
    filedata = f.read()
    f.close()
    newdata = filedata.replace(sig,code_gen)
    f = open(filename,'w')
    f.write(newdata)
    f.close()

def generate_flag_read_values(filename):
    sig = "//CODEGEN_FLAG_READ_VALUES"
    template = """
static char flag_%d_value[] = "Flag %d: Incomplete";"""
    code_gen = sig
    for i in range(len(flag_file_data)):
        code_gen += template % (i, i)
    
    f = open(filename,'r')
    filedata = f.read()
    f.close()
    newdata = filedata.replace(sig,code_gen)
    f = open(filename,'w')
    f.write(newdata)
    f.close()

def generate_flag_declarations(filename):
    sig = "//CODEGEN_FLAG_DECLARATIONS"
    template = """
[FLAG_SCOREBOARD_IDX_CHAR_READ_FLAG_%d]      =
{{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ,
  CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_read}},

[FLAG_SCOREBOARD_IDX_CHAR_VAL_READ_FLAG_%d]  =
{{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&GATTS_CHAR_UUID_READ_DOCS, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
  GATTS_DEMO_CHAR_VAL_LEN_MAX, sizeof(flag_%d_value)-1, (uint8_t *)flag_%d_value}},"""
    code_gen = sig
    for i in range(len(flag_file_data)):
        code_gen += template % (i, i, i, i)

    f = open(filename,'r')
    filedata = f.read()
    f.close()
    newdata = filedata.replace(sig,code_gen)
    f = open(filename,'w')
    f.write(newdata)
    f.close()

def generate_flag_validate_conditional(filename):
    sig = "//CODEGEN_FLAG_VALIDATE_CONDITIONAL"
    template = """
if (strcmp(writeData, "%s") == 0){
    err = nvs_set_i32(my_handle, "flag_%d", 1);
    strcpy(flag_%d_value, "Flag %d: Complete  ");
    esp_ble_gatts_set_attr_value(blectf_handle_table[FLAG_SCOREBOARD_IDX_CHAR_READ_FLAG_%d]+1, sizeof(flag_%d_value)-1, (uint8_t *)flag_%d_value);
}"""
    code_gen = sig
    for i in range(len(flag_file_data)):
        flag_value = flag_file_data["flag_%s" % (str(i))]["flag_value"]
        code_gen += template % (flag_value, i,i,i,i,i,i)

    f = open(filename,'r')
    filedata = f.read()
    f.close()
    newdata = filedata.replace(sig,code_gen)
    f = open(filename,'w')
    f.write(newdata)
    f.close()

if __name__ == "__main__":
    # user args
    filename = os.path.join(sys.argv[1], "code_gen","flag_config.csv")
    ble_ctf_dir = sys.argv[1]
    dashboard = "flag_scoreboard" 

    # default args

    import_flag_file_data(filename)
    copy_gatt_server_files(ble_ctf_dir)

    dashboard_file = os.path.join(ble_ctf_dir, "main", flag_file_data["flag_0"]["gatt_name"]) 

    generate_header_flag_idx(dashboard_file + ".h")
    generate_includes(dashboard_file + ".c")
    generate_handle_locations(dashboard_file + ".c")
    generate_total_flags(dashboard_file + ".c")
    generate_flag_read_values(dashboard_file + ".c")
    generate_flag_declarations(dashboard_file + ".c")
    generate_flag_validate_conditional(dashboard_file + ".c")
    generate_flag_status_msgs(dashboard_file + ".c")
    generate_main_gatt_method_names(ble_ctf_dir)
    generate_app_main(ble_ctf_dir, dashboard)
    generate_flag_values(ble_ctf_dir)
    generate_devicename(ble_ctf_dir)
