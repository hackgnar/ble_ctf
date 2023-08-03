/*
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

/****************************************************************************
*
* This file is for a gatt server CTF (capture the flag). 
*
****************************************************************************/


 #include "freertos/FreeRTOS.h"
 #include "freertos/task.h"
 #include "freertos/event_groups.h"
 #include "esp_system.h"
 #include "esp_log.h"
 #include "nvs_flash.h"
 #include "esp_bt.h"
 #include "driver/gpio.h"

#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_bt_main.h"
#include "gatts_table_creat_demo.h"
#include "esp_gatt_common_api.h"

#define GATTS_TABLE_TAG "ESP_GATTS_DEMO"

#define PROFILE_NUM                 1
#define PROFILE_APP_IDX             0
#define ESP_APP_ID                  0x55
#define SAMPLE_DEVICE_NAME          "2b00042f7481c7b056c4b410d28f33cf"
#define SVC_INST_ID                 0

#define GATTS_DEMO_CHAR_VAL_LEN_MAX 100
#define PREPARE_BUF_MAX_SIZE        1024
#define CHAR_DECLARATION_SIZE       (sizeof(uint8_t))

#define ADV_CONFIG_FLAG             (1 << 0)
#define SCAN_RSP_CONFIG_FLAG        (1 << 1)


static uint8_t adv_config_done       = 0;

uint16_t blectf_handle_table[HRS_IDX_NB];

typedef struct {
    uint8_t                 *prepare_buf;
    int                     prepare_len;
} prepare_type_env_t;

static prepare_type_env_t prepare_write_env;

#define CONFIG_SET_RAW_ADV_DATA
#ifdef CONFIG_SET_RAW_ADV_DATA
static uint8_t raw_adv_data[] = {
        /* flags */
        0x02, 0x01, 0x06,
        /* tx power*/
        0x02, 0x0a, 0xeb,
        /* service uuid */
        0x03, 0x03, 0xFF, 0x00,
        /* device name (first number is the length) */
        0x07, 0x09, 'B', 'L', 'E', 'C', 'T', 'F'

};
static uint8_t raw_scan_rsp_data[] = {
        /* flags */
        0x02, 0x01, 0x06,
        /* tx power */
        0x02, 0x0a, 0xeb,
        /* service uuid */
        0x03, 0x03, 0xFF,0x00
};

#else
static uint8_t service_uuid[16] = {
    /* LSB <--------------------------------------------------------------------------------> MSB */
    //first uuid, 16bit, [12],[13] is the value
    0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00,
};

/* The length of adv data must be less than 31 bytes */
static esp_ble_adv_data_t adv_data = {
    .set_scan_rsp        = false,
    .include_name        = true,
    .include_txpower     = true,
    .min_interval        = 0x20,
    .max_interval        = 0x40,
    .appearance          = 0x00,
    .manufacturer_len    = 0,    //TEST_MANUFACTURER_DATA_LEN,
    .p_manufacturer_data = NULL, //test_manufacturer,
    .service_data_len    = 0,
    .p_service_data      = NULL,
    .service_uuid_len    = sizeof(service_uuid),
    .p_service_uuid      = service_uuid,
    .flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
};

// scan response data
static esp_ble_adv_data_t scan_rsp_data = {
    .set_scan_rsp        = true,
    .include_name        = true,
    .include_txpower     = true,
    .min_interval        = 0x20,
    .max_interval        = 0x40,
    .appearance          = 0x00,
    .manufacturer_len    = 0, //TEST_MANUFACTURER_DATA_LEN,
    .p_manufacturer_data = NULL, //&test_manufacturer[0],
    .service_data_len    = 0,
    .p_service_data      = NULL,
    .service_uuid_len    = 16,
    .p_service_uuid      = service_uuid,
    .flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
};
#endif /* CONFIG_SET_RAW_ADV_DATA */

static esp_ble_adv_params_t adv_params = {
    .adv_int_min         = 0x20,
    .adv_int_max         = 0x40,
    .adv_type            = ADV_TYPE_IND,
    .own_addr_type       = BLE_ADDR_TYPE_PUBLIC,
    .channel_map         = ADV_CHNL_ALL,
    .adv_filter_policy   = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
};

struct gatts_profile_inst {
    esp_gatts_cb_t gatts_cb;
    uint16_t gatts_if;
    uint16_t app_id;
    uint16_t conn_id;
    uint16_t service_handle;
    esp_gatt_srvc_id_t service_id;
    uint16_t char_handle;
    esp_bt_uuid_t char_uuid;
    esp_gatt_perm_t perm;
    esp_gatt_char_prop_t property;
    uint16_t descr_handle;
    esp_bt_uuid_t descr_uuid;
};

static void gatts_profile_event_handler(esp_gatts_cb_event_t event,
					esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);

/* One gatt-based profile one app_id and one gatts_if, this array will store the gatts_if returned by ESP_GATTS_REG_EVT */
static struct gatts_profile_inst blectf_profile_tab[PROFILE_NUM] = {
    [PROFILE_APP_IDX] = {
        .gatts_cb = gatts_profile_event_handler,
        .gatts_if = ESP_GATT_IF_NONE,       /* Not get the gatt_if, so initial is ESP_GATT_IF_NONE */
    },
};

/* Service */
static const uint16_t GATTS_SERVICE_UUID_TEST                   = 0x00FF;
static const uint16_t GATTS_CHAR_UUID_SCORE                     = 0xFF01;
static const uint16_t GATTS_CHAR_UUID_FLAG                      = 0xFF02;
static const uint16_t GATTS_CHAR_UUID_FLAG_SIMPLE_READ          = 0xFF03;
static const uint16_t GATTS_CHAR_UUID_FLAG_MD5                  = 0xFF04;
static const uint16_t GATTS_CHAR_UUID_FLAG_WRITE_ANYTHING       = 0xFF05;
static const uint16_t GATTS_CHAR_UUID_FLAG_WRITE_ASCII          = 0xFF06;
static const uint16_t GATTS_CHAR_UUID_FLAG_WRITE_HEX            = 0xFF07;
static const uint16_t GATTS_CHAR_UUID_FLAG_SIMPLE_WRITE2_READ   = 0xFF08;
static const uint16_t GATTS_CHAR_UUID_FLAG_SIMPLE_WRITE2        = 0xFF09;
static const uint16_t GATTS_CHAR_UUID_FLAG_BRUTE_WRITE          = 0xFF0a;
static const uint16_t GATTS_CHAR_UUID_FLAG_READ_ALOT            = 0xFF0b;
static const uint16_t GATTS_CHAR_UUID_FLAG_NOTIFICATION         = 0xFF0c;
static const uint16_t GATTS_CHAR_UUID_FLAG_INDICATE_READ        = 0xFF0d;
static const uint16_t GATTS_CHAR_UUID_FLAG_INDICATE             = 0xFF0e;
static const uint16_t GATTS_CHAR_UUID_FLAG_NOTIFICATION_MULTI   = 0xFF0f;
static const uint16_t GATTS_CHAR_UUID_FLAG_INDICATE_MULTI_READ  = 0xFF10;
static const uint16_t GATTS_CHAR_UUID_FLAG_INDICATE_MULTI       = 0xFF11;
static const uint16_t GATTS_CHAR_UUID_FLAG_MAC                  = 0xFF12;
static const uint16_t GATTS_CHAR_UUID_FLAG_MTU                  = 0xFF13;
static const uint16_t GATTS_CHAR_UUID_FLAG_WRITE_RESPONSE       = 0xFF14;
static const uint16_t GATTS_CHAR_UUID_FLAG_HIDDEN_NOTIFY        = 0xFF15;
static const uint16_t GATTS_CHAR_UUID_FLAG_CRAZY                = 0xFF16;
static const uint16_t GATTS_CHAR_UUID_FLAG_TWITTER              = 0xFF17;

static const uint16_t primary_service_uuid         = ESP_GATT_UUID_PRI_SERVICE;
static const uint16_t character_declaration_uuid   = ESP_GATT_UUID_CHAR_DECLARE;
static const uint16_t character_client_config_uuid = ESP_GATT_UUID_CHAR_CLIENT_CONFIG;
static const uint8_t char_prop_read                =  ESP_GATT_CHAR_PROP_BIT_READ;
static const uint8_t char_prop_write               = ESP_GATT_CHAR_PROP_BIT_WRITE;
static const uint8_t char_prop_read_write_notify   = ESP_GATT_CHAR_PROP_BIT_WRITE | ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_NOTIFY;
static const uint8_t char_prop_read_write_indicate   = ESP_GATT_CHAR_PROP_BIT_WRITE |ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_INDICATE;
static const uint8_t char_prop_read_write   = ESP_GATT_CHAR_PROP_BIT_WRITE | ESP_GATT_CHAR_PROP_BIT_READ;
static const uint8_t char_prop_crazy   = ESP_GATT_CHAR_PROP_BIT_WRITE | ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_EXT_PROP | ESP_GATT_CHAR_PROP_BIT_BROADCAST |  ESP_GATT_CHAR_PROP_BIT_NOTIFY ;
//static const uint8_t heart_measurement_ccc[2]      = {0x00, 0x00};
//static const uint8_t char_value[4]                 = {0x11, 0x22, 0x33, 0x44};

// start ctf data vars
static char writeData[100];
static char flag_state[20] = {'F','F','F','F','F','F','F','F','F','F','F','F','F','F','F','F','F','F','F','F'};
static uint8_t score_read_value[11] = {'S', 'c', 'o', 'r', 'e', ':', ' ', '0','/','2','0'};
static const char write_any_flag[] = "Write anything here";
static const char write_ascii_flag[] = "Write the ascii value \"yo\" here";
static const char write_hex_flag[] = "Write the hex value 0x07 here";
static const char read_alot_value[] = "Read me 1000 times";
static const char read_mac_value[] = "Connect with BT MAC address 11:22:33:44:55:66";
static const char read_mtu_value[] = "Set your connection MTU to 444";
static const char notification_read_value[] = "Listen to me for a single notification";
static const char indicate_read_value[] = "Listen to handle 0x0044 for a single indication";
static const char notification_multi_read_value[] = "Listen to me for multi notifications";
static const char indicate_multi_read_value[] = "Listen to handle 0x004a for multi indications";
static const char brute_write_flag[] = "Brute force my value 00 to ff";
static const char hidden_notify_value[] = "No notifications here! really?";
static const char crazy_value[] = "So many properties!";
static const char twitter_value[] = "md5 of author's twitter handle";
static const char write_response_data[20] = "Write+resp 'hello'  ";
static const uint8_t read_write2_value[23] = {'W','r','i','t','e',' ','0','x','C','9',' ','t','o',' ','h','a','n','d','l','e',' ','5','8'};

//static const uint8_t brute_write_flag[33] = {'B','r','u','t','e',' ','f','o','r','c','e',' ','m','y',' ','v','a','l','u','e', ' ', '0','x','0','0',' ','t','o',' ','0','x','f','f'};
static const uint8_t flag_read_value[16] = {'W','r','i','t','e', ' ', 'F', 'l','a','g','s', ' ', 'H','e','r', 'e'};
int read_alot_counter = 0;
int read_counter = 0;
int score = 0;
static char string_score[10] = "0";
int BLINK_GPIO=2;
int indicate_handle_state = 0;
int send_response=0;
int check_send_response=0;

/* Full Database Description - Used to add attributes into the database */
static const esp_gatts_attr_db_t gatt_db[HRS_IDX_NB] =
{
    // Service Declaration
    [IDX_SVC]        =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&primary_service_uuid, ESP_GATT_PERM_READ,
      sizeof(uint16_t), sizeof(GATTS_SERVICE_UUID_TEST), (uint8_t *)&GATTS_SERVICE_UUID_TEST}},

    /* SCORE Characteristic Declaration */
    [IDX_CHAR_SCORE]      =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ,
      CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_read}},

    /* SCORE Characteristic Value */
    [IDX_CHAR_VAL_SCORE]  =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&GATTS_CHAR_UUID_SCORE, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
      GATTS_DEMO_CHAR_VAL_LEN_MAX, sizeof(score_read_value), (uint8_t *)score_read_value}},
    
    /* FLAG Characteristic Declaration */
    [IDX_CHAR_FLAG]      =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
      CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_read_write}},

    /* FLAG Characteristic Value */
    [IDX_CHAR_VAL_FLAG]  =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&GATTS_CHAR_UUID_FLAG, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
      GATTS_DEMO_CHAR_VAL_LEN_MAX, sizeof(flag_read_value), (uint8_t *)flag_read_value}},

    /* FLAG MD5 Characteristic Declaration */
    [IDX_CHAR_FLAG_SIMPLE_READ]      =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ,
      CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_read}},

    /* Characteristic Value */
    [IDX_CHAR_VAL_FLAG_SIMPLE_READ]  =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&GATTS_CHAR_UUID_FLAG_SIMPLE_READ, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
      GATTS_DEMO_CHAR_VAL_LEN_MAX, sizeof("d205303e099ceff44835")-1, (uint8_t *)"d205303e099ceff44835"}},

    /* FLAG MD5 Characteristic Declaration */
    [IDX_CHAR_FLAG_MD5]      =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ,
      CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_read}},

    /* Characteristic Value */
    [IDX_CHAR_VAL_FLAG_MD5]  =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&GATTS_CHAR_UUID_FLAG_MD5, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
      GATTS_DEMO_CHAR_VAL_LEN_MAX, sizeof("MD5 of Device Name")-1, (uint8_t *)"MD5 of Device Name"}},

    /* FLAG WRITE ANYTHING Characteristic Declaration */
    [IDX_CHAR_FLAG_WRITE_ANYTHING]      =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ|ESP_GATT_PERM_WRITE,
      CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_read_write}},

    /* Characteristic Value */
    [IDX_CHAR_VAL_FLAG_WRITE_ANYTHING]  =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&GATTS_CHAR_UUID_FLAG_WRITE_ANYTHING, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
      GATTS_DEMO_CHAR_VAL_LEN_MAX, sizeof(write_any_flag)-1, (uint8_t *)write_any_flag}},

    /* FLAG WRITE ASCII Characteristic Declaration */
    [IDX_CHAR_FLAG_WRITE_ASCII]      =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ|ESP_GATT_PERM_WRITE,
      CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_read_write}},

    /* Characteristic Value */
    [IDX_CHAR_VAL_FLAG_WRITE_ASCII]  =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&GATTS_CHAR_UUID_FLAG_WRITE_ASCII, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
      GATTS_DEMO_CHAR_VAL_LEN_MAX, sizeof(write_ascii_flag)-1, (uint8_t *)write_ascii_flag}},

    /* FLAG simple write Characteristic Declaration */
    [IDX_CHAR_FLAG_WRITE_HEX]      =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ|ESP_GATT_PERM_WRITE,
      CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_read_write}},

    /* Characteristic Value */
    [IDX_CHAR_VAL_FLAG_WRITE_HEX]  =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&GATTS_CHAR_UUID_FLAG_WRITE_HEX, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
      GATTS_DEMO_CHAR_VAL_LEN_MAX, sizeof(write_hex_flag)-1, (uint8_t *)write_hex_flag}},

    /* FLAG brute write Characteristic Declaration */
    [IDX_CHAR_FLAG_BRUTE_WRITE]      =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ|ESP_GATT_PERM_WRITE,
      CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_read_write}},

    /* Characteristic Value */
    [IDX_CHAR_VAL_FLAG_BRUTE_WRITE]  =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&GATTS_CHAR_UUID_FLAG_BRUTE_WRITE, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
      GATTS_DEMO_CHAR_VAL_LEN_MAX, sizeof(brute_write_flag)-1, (uint8_t *)brute_write_flag}},

    /* FLAG read write Characteristic Declaration */
    [IDX_CHAR_FLAG_SIMPLE_WRITE2_READ]      =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ,
      CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_read}},

    /* Characteristic Value */
    [IDX_CHAR_VAL_FLAG_SIMPLE_WRITE2_READ]  =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&GATTS_CHAR_UUID_FLAG_SIMPLE_WRITE2_READ, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
      GATTS_DEMO_CHAR_VAL_LEN_MAX, sizeof(read_write2_value), (uint8_t *)read_write2_value}},

    /* FLAG read write Characteristic Declaration */
    [IDX_CHAR_FLAG_SIMPLE_WRITE2]      =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ,
      CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_write}},

    /* Characteristic Value */
    [IDX_CHAR_VAL_FLAG_SIMPLE_WRITE2]  =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&GATTS_CHAR_UUID_FLAG_SIMPLE_WRITE2, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
      GATTS_DEMO_CHAR_VAL_LEN_MAX, sizeof(read_write2_value), (uint8_t *)read_write2_value}},

    /* FLAG read alot Characteristic Declaration */
    [IDX_CHAR_FLAG_READ_ALOT]      =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ,
      CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_read}},

    /* Characteristic Value */
    [IDX_CHAR_VAL_FLAG_READ_ALOT]  =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&GATTS_CHAR_UUID_FLAG_READ_ALOT, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
      GATTS_DEMO_CHAR_VAL_LEN_MAX, sizeof(read_alot_value)-1, (uint8_t *)read_alot_value}},

    /* Notification flag Characteristic Declaration */
    [IDX_CHAR_FLAG_NOTIFICATION]     =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ,
      CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_read_write_notify}},

    /* Characteristic Value */
    [IDX_CHAR_VAL_FLAG_NOTIFICATION] =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&GATTS_CHAR_UUID_FLAG_NOTIFICATION, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
      GATTS_DEMO_CHAR_VAL_LEN_MAX, sizeof(notification_read_value)-1, (uint8_t *)notification_read_value}},

    /* Client Characteristic Configuration Descriptor */
    [IDX_CHAR_CFG_FLAG_NOTIFICATION]  =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_client_config_uuid, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
      sizeof(uint16_t), sizeof(notification_read_value)-1, (uint8_t *)notification_read_value}},

    /* FLAG indicate read Characteristic Declaration */
    [IDX_CHAR_FLAG_INDICATE_READ]      =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ,
      CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_read}},

    /* Characteristic Value */
    [IDX_CHAR_VAL_FLAG_INDICATE_READ]  =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&GATTS_CHAR_UUID_FLAG_INDICATE_READ, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
      GATTS_DEMO_CHAR_VAL_LEN_MAX, sizeof(indicate_read_value)-1, (uint8_t *)indicate_read_value}},

    
    /* indicate flag Characteristic Declaration */
    [IDX_CHAR_FLAG_INDICATE]     =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ,
      CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_read_write_indicate}},

    /* Characteristic Value */
    [IDX_CHAR_VAL_FLAG_INDICATE] =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&GATTS_CHAR_UUID_FLAG_INDICATE, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
      GATTS_DEMO_CHAR_VAL_LEN_MAX, sizeof(indicate_read_value), (uint8_t *)indicate_read_value}},

    /* Client Characteristic Configuration Descriptor */
    [IDX_CHAR_CFG_FLAG_INDICATE]  =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_client_config_uuid, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
      sizeof(uint16_t), sizeof(indicate_read_value)-1, (uint8_t *)indicate_read_value}},

    /* Notification flag Characteristic Declaration */
    [IDX_CHAR_FLAG_NOTIFICATION_MULTI]     =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ,
      CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_read_write_notify}},

    /* Characteristic Value */
    [IDX_CHAR_VAL_FLAG_NOTIFICATION_MULTI] =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&GATTS_CHAR_UUID_FLAG_NOTIFICATION_MULTI, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
      GATTS_DEMO_CHAR_VAL_LEN_MAX, sizeof(notification_multi_read_value)-1, (uint8_t *)notification_multi_read_value}},

    /* Client Characteristic Configuration Descriptor */
    [IDX_CHAR_CFG_FLAG_NOTIFICATION_MULTI]  =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_client_config_uuid, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
      sizeof(uint16_t), sizeof(notification_multi_read_value)-1, (uint8_t *)notification_multi_read_value}},

    /* FLAG indicate read Characteristic Declaration */
    [IDX_CHAR_FLAG_INDICATE_MULTI_READ]      =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ,
      CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_read}},

    /* Characteristic Value */
    [IDX_CHAR_VAL_FLAG_INDICATE_MULTI_READ]  =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&GATTS_CHAR_UUID_FLAG_INDICATE_MULTI_READ, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
      GATTS_DEMO_CHAR_VAL_LEN_MAX, sizeof(indicate_multi_read_value)-1, (uint8_t *)indicate_multi_read_value}},
    
    /* indicate flag Characteristic Declaration */
    [IDX_CHAR_FLAG_INDICATE_MULTI]     =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ,
      CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_read_write_indicate}},

    /* Characteristic Value */
    [IDX_CHAR_VAL_FLAG_INDICATE_MULTI] =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&GATTS_CHAR_UUID_FLAG_INDICATE_MULTI, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
      GATTS_DEMO_CHAR_VAL_LEN_MAX, sizeof(indicate_multi_read_value), (uint8_t *)indicate_multi_read_value}},

    /* Client Characteristic Configuration Descriptor */
    [IDX_CHAR_CFG_FLAG_INDICATE_MULTI]  =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_client_config_uuid, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
      sizeof(uint16_t), sizeof(indicate_multi_read_value)-1, (uint8_t *)indicate_multi_read_value}},

    /* FLAG MAC Characteristic Declaration */
    [IDX_CHAR_FLAG_MAC]      =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ,
      CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_read}},

    /* Characteristic Value */
    [IDX_CHAR_VAL_FLAG_MAC]  =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&GATTS_CHAR_UUID_FLAG_MAC, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
      GATTS_DEMO_CHAR_VAL_LEN_MAX, sizeof(read_mac_value)-1, (uint8_t *)read_mac_value}},

    /* FLAG MTU Characteristic Declaration */
    [IDX_CHAR_FLAG_MTU]      =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ,
      CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_read}},

    /* Characteristic Value */
    [IDX_CHAR_VAL_FLAG_MTU]  =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&GATTS_CHAR_UUID_FLAG_MTU, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
      GATTS_DEMO_CHAR_VAL_LEN_MAX, sizeof(read_mtu_value)-1, (uint8_t *)read_mtu_value}},

    /* FLAG write response Characteristic Declaration */
    [IDX_CHAR_FLAG_WRITE_RESPONSE]      =
    {{ESP_GATT_RSP_BY_APP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ|ESP_GATT_PERM_WRITE,
    //{{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ|ESP_GATT_PERM_WRITE,
      CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_read_write}},

    /* Characteristic Value */
    [IDX_CHAR_VAL_FLAG_WRITE_RESPONSE]  =
    {{ESP_GATT_RSP_BY_APP}, {ESP_UUID_LEN_16, (uint8_t *)&GATTS_CHAR_UUID_FLAG_WRITE_RESPONSE, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
    //{{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&GATTS_CHAR_UUID_FLAG_WRITE_RESPONSE, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
      GATTS_DEMO_CHAR_VAL_LEN_MAX, sizeof(write_response_data)-1, (uint8_t *)write_response_data}},

    /* FLAG hidden notify Characteristic Declaration */
    [IDX_CHAR_FLAG_HIDDEN_NOTIFY]      =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ|ESP_GATT_PERM_WRITE,
      CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_read_write}},

    /* Characteristic Value */
    [IDX_CHAR_VAL_FLAG_HIDDEN_NOTIFY]  =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&GATTS_CHAR_UUID_FLAG_HIDDEN_NOTIFY, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
      GATTS_DEMO_CHAR_VAL_LEN_MAX, sizeof(hidden_notify_value)-1, (uint8_t *)hidden_notify_value}},

    /* FLAG crazy Characteristic Declaration */
    [IDX_CHAR_FLAG_CRAZY]      =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ|ESP_GATT_PERM_WRITE,
      CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_crazy}},

    /* Characteristic Value */
    [IDX_CHAR_VAL_FLAG_CRAZY]  =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&GATTS_CHAR_UUID_FLAG_CRAZY, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
      GATTS_DEMO_CHAR_VAL_LEN_MAX, sizeof(crazy_value)-1, (uint8_t *)crazy_value}},

    /* FLAG twitter Characteristic Declaration */
    [IDX_CHAR_FLAG_TWITTER]      =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ,
      CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_read}},

    /* FLAG twitter Characteristic Value */
    [IDX_CHAR_VAL_FLAG_TWITTER]  =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&GATTS_CHAR_UUID_FLAG_TWITTER, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
      GATTS_DEMO_CHAR_VAL_LEN_MAX, sizeof(twitter_value)-1, (uint8_t *)twitter_value}},

};

static void set_score()
{
    //set scores
    score = 0;
    for (int i = 0 ; i < 20 ; ++i)
    {
        if (flag_state[i] == 'T'){
            score += 1;
        }
    }
    
    itoa(score, string_score, 10);
    for (int i = 0 ; i < strlen(string_score) ; ++i)
    {
        if (strlen(string_score) == 1){
            score_read_value[7] = ' ';}
        score_read_value[6+i] = string_score[i];
    }
    esp_ble_gatts_set_attr_value(blectf_handle_table[IDX_CHAR_SCORE]+1, sizeof score_read_value, score_read_value);
}

static void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
    switch (event) {
    #ifdef CONFIG_SET_RAW_ADV_DATA
        case ESP_GAP_BLE_ADV_DATA_RAW_SET_COMPLETE_EVT:
            adv_config_done &= (~ADV_CONFIG_FLAG);
            if (adv_config_done == 0){
                esp_ble_gap_start_advertising(&adv_params);
            }
            break;
        case ESP_GAP_BLE_SCAN_RSP_DATA_RAW_SET_COMPLETE_EVT:
            adv_config_done &= (~SCAN_RSP_CONFIG_FLAG);
            if (adv_config_done == 0){
                esp_ble_gap_start_advertising(&adv_params);
            }
            break;
    #else
        case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
            adv_config_done &= (~ADV_CONFIG_FLAG);
            if (adv_config_done == 0){
                esp_ble_gap_start_advertising(&adv_params);
            }
            break;
        case ESP_GAP_BLE_SCAN_RSP_DATA_SET_COMPLETE_EVT:
            adv_config_done &= (~SCAN_RSP_CONFIG_FLAG);
            if (adv_config_done == 0){
                esp_ble_gap_start_advertising(&adv_params);
            }
            break;
    #endif
        case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
            /* advertising start complete event to indicate advertising start successfully or failed */
            if (param->adv_start_cmpl.status != ESP_BT_STATUS_SUCCESS) {
                ESP_LOGE(GATTS_TABLE_TAG, "advertising start failed");
            }else{
                ESP_LOGI(GATTS_TABLE_TAG, "advertising start successfully");
            }
            break;
        case ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT:
            if (param->adv_stop_cmpl.status != ESP_BT_STATUS_SUCCESS) {
                ESP_LOGE(GATTS_TABLE_TAG, "Advertising stop failed");
            }
            else {
                ESP_LOGI(GATTS_TABLE_TAG, "Stop adv successfully\n");
            }
            break;
        case ESP_GAP_BLE_UPDATE_CONN_PARAMS_EVT:
            ESP_LOGI(GATTS_TABLE_TAG, "update connetion params status = %d, min_int = %d, max_int = %d,conn_int = %d,latency = %d, timeout = %d",
                  param->update_conn_params.status,
                  param->update_conn_params.min_int,
                  param->update_conn_params.max_int,
                  param->update_conn_params.conn_int,
                  param->update_conn_params.latency,
                  param->update_conn_params.timeout);
            break;
        default:
            break;
    }
}

void example_prepare_write_event_env(esp_gatt_if_t gatts_if, prepare_type_env_t *prepare_write_env, esp_ble_gatts_cb_param_t *param)
{
    ESP_LOGI(GATTS_TABLE_TAG, "prepare write, handle = %d, value len = %d", param->write.handle, param->write.len);
    esp_gatt_status_t status = ESP_GATT_OK;
    if (prepare_write_env->prepare_buf == NULL) {
        prepare_write_env->prepare_buf = (uint8_t *)malloc(PREPARE_BUF_MAX_SIZE * sizeof(uint8_t));
        prepare_write_env->prepare_len = 0;
        if (prepare_write_env->prepare_buf == NULL) {
            ESP_LOGE(GATTS_TABLE_TAG, "%s, Gatt_server prep no mem", __func__);
            status = ESP_GATT_NO_RESOURCES;
        }
    } else {
        if(param->write.offset > PREPARE_BUF_MAX_SIZE) {
            status = ESP_GATT_INVALID_OFFSET;
        } else if ((param->write.offset + param->write.len) > PREPARE_BUF_MAX_SIZE) {
            status = ESP_GATT_INVALID_ATTR_LEN;
        }
    }
    /*send response when param->write.need_rsp is true */
    if (param->write.need_rsp){
        esp_gatt_rsp_t *gatt_rsp = (esp_gatt_rsp_t *)malloc(sizeof(esp_gatt_rsp_t));
        if (gatt_rsp != NULL){
            gatt_rsp->attr_value.len = param->write.len;
            gatt_rsp->attr_value.handle = param->write.handle;
            gatt_rsp->attr_value.offset = param->write.offset;
            gatt_rsp->attr_value.auth_req = ESP_GATT_AUTH_REQ_NONE;
            memcpy(gatt_rsp->attr_value.value, param->write.value, param->write.len);
            esp_err_t response_err = esp_ble_gatts_send_response(gatts_if, param->write.conn_id, param->write.trans_id, status, gatt_rsp);
            if (response_err != ESP_OK){
               ESP_LOGE(GATTS_TABLE_TAG, "Send response error");
            }
            free(gatt_rsp);
        }else{
            ESP_LOGE(GATTS_TABLE_TAG, "%s, malloc failed", __func__);
        }
    }
    if (status != ESP_GATT_OK){
        return;
    }
    memcpy(prepare_write_env->prepare_buf + param->write.offset,
           param->write.value,
           param->write.len);
    prepare_write_env->prepare_len += param->write.len;

}

void example_exec_write_event_env(prepare_type_env_t *prepare_write_env, esp_ble_gatts_cb_param_t *param){
    if (param->exec_write.exec_write_flag == ESP_GATT_PREP_WRITE_EXEC && prepare_write_env->prepare_buf){
        esp_log_buffer_hex(GATTS_TABLE_TAG, prepare_write_env->prepare_buf, prepare_write_env->prepare_len);
    }else{
        ESP_LOGI(GATTS_TABLE_TAG,"ESP_GATT_PREP_WRITE_CANCEL");
    }
    if (prepare_write_env->prepare_buf) {
        free(prepare_write_env->prepare_buf);
        prepare_write_env->prepare_buf = NULL;
    }
    prepare_write_env->prepare_len = 0;
}

static void gatts_profile_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param)
{
    switch (event) {
        case ESP_GATTS_REG_EVT:{
            ESP_LOGI(GATTS_TABLE_TAG, "ESP_GATTS_REG_EVT");
            esp_err_t set_dev_name_ret = esp_ble_gap_set_device_name(SAMPLE_DEVICE_NAME);
            if (set_dev_name_ret){
                ESP_LOGE(GATTS_TABLE_TAG, "set device name failed, error code = %x", set_dev_name_ret);
            }
    #ifdef CONFIG_SET_RAW_ADV_DATA
            esp_err_t raw_adv_ret = esp_ble_gap_config_adv_data_raw(raw_adv_data, sizeof(raw_adv_data));
            if (raw_adv_ret){
                ESP_LOGE(GATTS_TABLE_TAG, "config raw adv data failed, error code = %x ", raw_adv_ret);
            }
            adv_config_done |= ADV_CONFIG_FLAG;
            esp_err_t raw_scan_ret = esp_ble_gap_config_scan_rsp_data_raw(raw_scan_rsp_data, sizeof(raw_scan_rsp_data));
            if (raw_scan_ret){
                ESP_LOGE(GATTS_TABLE_TAG, "config raw scan rsp data failed, error code = %x", raw_scan_ret);
            }
            adv_config_done |= SCAN_RSP_CONFIG_FLAG;
    #else
            //config adv data
            esp_err_t ret = esp_ble_gap_config_adv_data(&adv_data);
            if (ret){
                ESP_LOGE(GATTS_TABLE_TAG, "config adv data failed, error code = %x", ret);
            }
            adv_config_done |= ADV_CONFIG_FLAG;
            //config scan response data
            ret = esp_ble_gap_config_adv_data(&scan_rsp_data);
            if (ret){
                ESP_LOGE(GATTS_TABLE_TAG, "config scan response data failed, error code = %x", ret);
            }
            adv_config_done |= SCAN_RSP_CONFIG_FLAG;
    #endif
            esp_err_t create_attr_ret = esp_ble_gatts_create_attr_tab(gatt_db, gatts_if, HRS_IDX_NB, SVC_INST_ID);
            if (create_attr_ret){
                ESP_LOGE(GATTS_TABLE_TAG, "create attr table failed, error code = %x", create_attr_ret);
            }
        }
       	    break;
        case ESP_GATTS_READ_EVT:
            ESP_LOGI(GATTS_TABLE_TAG, "ESP_GATTS_READ_EVT");
            read_counter += 1;
            //set gpio
            esp_rom_gpio_pad_select_gpio(BLINK_GPIO);
            /* Set the GPIO as a push/pull output */
            gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
            gpio_set_level(BLINK_GPIO, 1);
            if (read_counter > 1000){
                esp_ble_gatts_set_attr_value(blectf_handle_table[IDX_CHAR_FLAG_READ_ALOT]+1, 20, (uint8_t *)"6ffcd214ffebdc0d069e");
            }
            if (param->read.handle == blectf_handle_table[IDX_CHAR_FLAG_WRITE_RESPONSE]+1){
                // add an ascii value write check to this one
                esp_gatt_rsp_t *rsp = (esp_gatt_rsp_t *)malloc(sizeof(esp_gatt_rsp_t));
                if (flag_state[16] == 'T' || flag_state[16] == 'H'){
                    char write_response_flag[] = "d41d8cd98f00b204e980";
                    rsp->attr_value.len = sizeof(write_response_flag);
                    rsp->attr_value.auth_req = ESP_GATT_AUTH_REQ_NONE;
                    memcpy(rsp->attr_value.value, (uint8_t *)write_response_flag, sizeof(write_response_flag));
                }else{
                    rsp->attr_value.len = sizeof(write_response_data);
                    rsp->attr_value.auth_req = ESP_GATT_AUTH_REQ_NONE;
                    memcpy(rsp->attr_value.value, (uint8_t *)write_response_data, sizeof(write_response_data));
                }

                esp_ble_gatts_send_response(gatts_if, param->read.conn_id, param->read.trans_id, ESP_GATT_OK, rsp);
            }

       	    break;
        case ESP_GATTS_WRITE_EVT:
            ESP_LOGI(GATTS_TABLE_TAG, "ESP_GATTS_WRITE_EVT");
            
            if (!param->write.is_prep){
                ESP_LOGI(GATTS_TABLE_TAG, "GATT_WRITE_EVT, handle = %d, value len = %d, value :", param->write.handle, param->write.len);
                esp_log_buffer_hex(GATTS_TABLE_TAG, param->write.value, param->write.len);
                
                // store write data for flag checking
                memset(writeData, 0, sizeof writeData);
                memcpy(writeData, param->write.value, 20); 

                // any write
                if (blectf_handle_table[IDX_CHAR_FLAG_WRITE_ANYTHING]+1 == param->write.handle)
                {
                    esp_ble_gatts_set_attr_value(blectf_handle_table[IDX_CHAR_FLAG_WRITE_ANYTHING]+1, 20, (uint8_t *)"3873c0270763568cf7aa");
                }

                // hex ascii
                if (blectf_handle_table[IDX_CHAR_FLAG_WRITE_ASCII]+1 == param->write.handle)
                {
                    if (strcmp(writeData,"yo") == 0){
                        esp_ble_gatts_set_attr_value(blectf_handle_table[IDX_CHAR_FLAG_WRITE_ASCII]+1, 20, (uint8_t *)"c55c6314b3db0a6128af");
                    }
                    else
                    {
                        esp_ble_gatts_set_attr_value(blectf_handle_table[IDX_CHAR_FLAG_WRITE_ASCII]+1, sizeof(write_hex_flag)-1, (uint8_t *)write_ascii_flag);
                    }
                }

                // hex write
                if (blectf_handle_table[IDX_CHAR_FLAG_WRITE_HEX]+1 == param->write.handle)
                {
                    uint16_t descr_value = param->write.value[1]<<8 |param->write.value[0];
                    if (descr_value == 0x0007){
                        esp_ble_gatts_set_attr_value(blectf_handle_table[IDX_CHAR_FLAG_WRITE_HEX]+1, 20, (uint8_t *)"1179080b29f8da16ad66");
                    }
                    else
                    {
                        esp_ble_gatts_set_attr_value(blectf_handle_table[IDX_CHAR_FLAG_WRITE_HEX]+1, sizeof(write_hex_flag)-1, (uint8_t *)write_hex_flag);
                    }
                }
                // brute write
                if (blectf_handle_table[IDX_CHAR_FLAG_BRUTE_WRITE]+1 == param->write.handle)
                {
                    uint16_t descr_value = param->write.value[1]<<8 |param->write.value[0];
                    if (descr_value == 0x00D1 || flag_state[8] == 'T' || flag_state[8] == 'H'){
                        esp_ble_gatts_set_attr_value(blectf_handle_table[IDX_CHAR_FLAG_BRUTE_WRITE]+1, 20, (uint8_t *)"933c1fcfa8ed52d2ec05");
                        if (flag_state[8] != 'T'){
                            flag_state[8] = 'H';
                        }
                    }
                    else
                    {
                        esp_ble_gatts_set_attr_value(blectf_handle_table[IDX_CHAR_FLAG_BRUTE_WRITE]+1, sizeof(brute_write_flag)-1, (uint8_t *)brute_write_flag);
                    }
                }
                // read write
                if (blectf_handle_table[IDX_CHAR_FLAG_SIMPLE_WRITE2]+1 == param->write.handle)
                {
                    uint16_t descr_value = param->write.value[1]<<8 |param->write.value[0];
                    if (descr_value == 0x00C9 || flag_state[7] == 'T' || flag_state[7] == 'H'){
                        esp_ble_gatts_set_attr_value(blectf_handle_table[IDX_CHAR_FLAG_SIMPLE_WRITE2_READ]+1, 20, (uint8_t *)"f8b136d937fad6a2be9f");
                        if (flag_state[7] != 'T'){
                            flag_state[7] = 'H';
                        }
                    }
                    else
                    {
                        esp_ble_gatts_set_attr_value(blectf_handle_table[IDX_CHAR_FLAG_SIMPLE_WRITE2_READ]+1, sizeof read_write2_value, read_write2_value);
                    }
                }
                // notify single response flag
                if (blectf_handle_table[IDX_CHAR_FLAG_NOTIFICATION]+1 == param->write.handle)
                {
                    char notify_data[20] = "5ec3772bcd00cf06d8eb";
                    esp_ble_gatts_set_attr_value(blectf_handle_table[IDX_CHAR_FLAG_NOTIFICATION]+1, sizeof(notification_read_value)-1, (uint8_t *)notification_read_value);
                    esp_ble_gatts_send_indicate(gatts_if, param->write.conn_id, blectf_handle_table[IDX_CHAR_VAL_FLAG_NOTIFICATION], sizeof(notify_data), (uint8_t *)notify_data, false);
                }

                // indicate single response flag flag
                if (blectf_handle_table[IDX_CHAR_FLAG_INDICATE]+1 == param->write.handle)
                {
                    char indicate_data[20] = "c7b86dd121848c77c113";
                    esp_ble_gatts_send_indicate(gatts_if, param->write.conn_id, blectf_handle_table[IDX_CHAR_VAL_FLAG_INDICATE], sizeof(indicate_data), (uint8_t *)indicate_data, true);
                }

                // notify multi response flag
                if (blectf_handle_table[IDX_CHAR_FLAG_NOTIFICATION_MULTI]+1 == param->write.handle)
                {
                    indicate_handle_state = blectf_handle_table[IDX_CHAR_FLAG_NOTIFICATION_MULTI]; 
                    char notify_data[20] = "U no want this msg";
                    esp_ble_gatts_set_attr_value(blectf_handle_table[IDX_CHAR_FLAG_NOTIFICATION_MULTI]+1, sizeof(notification_multi_read_value)-1, (uint8_t *)notification_multi_read_value);
                    esp_ble_gatts_send_indicate(gatts_if, param->write.conn_id, blectf_handle_table[IDX_CHAR_VAL_FLAG_NOTIFICATION_MULTI], sizeof(notify_data), (uint8_t *)notify_data, false);
                }

                // indicate multi response flag flag
                if (blectf_handle_table[IDX_CHAR_FLAG_INDICATE_MULTI]+1 == param->write.handle)
                {
                    indicate_handle_state = blectf_handle_table[IDX_CHAR_FLAG_INDICATE_MULTI]; 
                    char indicate_data[20] = "U no want this msg";
                    esp_ble_gatts_send_indicate(gatts_if, param->write.conn_id, blectf_handle_table[IDX_CHAR_VAL_FLAG_INDICATE_MULTI], sizeof(indicate_data), (uint8_t *)indicate_data, true);
                }
                // write response
                //"d41d8cd98f00b204e980"
                if (blectf_handle_table[IDX_CHAR_FLAG_WRITE_RESPONSE]+1 == param->write.handle)
                {
                    if (strcmp(writeData,"hello") == 0){
                        check_send_response=1;
                        // we dont have to do send_response here it will hit the catchall
                    }
                }
                // notify hidden notify flag
                if (blectf_handle_table[IDX_CHAR_FLAG_HIDDEN_NOTIFY]+1 == param->write.handle)
                {
                    indicate_handle_state = blectf_handle_table[IDX_CHAR_FLAG_HIDDEN_NOTIFY]; 
                    char notify_data[20] = "fc920c68b6006169477b";
                    esp_ble_gatts_set_attr_value(blectf_handle_table[IDX_CHAR_FLAG_HIDDEN_NOTIFY]+1, sizeof(hidden_notify_value)-1, (uint8_t *)hidden_notify_value);
                    esp_ble_gatts_send_indicate(gatts_if, param->write.conn_id, blectf_handle_table[IDX_CHAR_VAL_FLAG_HIDDEN_NOTIFY], sizeof(notify_data), (uint8_t *)notify_data, false);
                }

                // so many properties
                if (blectf_handle_table[IDX_CHAR_FLAG_CRAZY]+1 == param->write.handle)
                {
                    esp_ble_gatts_set_attr_value(blectf_handle_table[IDX_CHAR_FLAG_CRAZY]+1, 10, (uint8_t *)"fbb966958f");
                    char notify_data[10] = "07e4a0cc48";
                    esp_ble_gatts_send_indicate(gatts_if, param->write.conn_id, blectf_handle_table[IDX_CHAR_VAL_FLAG_CRAZY], sizeof(notify_data), (uint8_t *)notify_data, false);
                }

                //handle flags
                if (blectf_handle_table[IDX_CHAR_FLAG]+1 == param->write.handle)
                {
                    // make sure flag read value stays static
                    esp_ble_gatts_set_attr_value(blectf_handle_table[IDX_CHAR_FLAG]+1, sizeof flag_read_value, flag_read_value);

                    //TODO: break this out into a function
                    if (strcmp(writeData,"12345678901234567890") == 0){
                        //gimmi from the hints docs or this source
                        flag_state[0] = 'T';
                    }
                    if (strcmp(writeData,"2b00042f7481c7b056c4") == 0){
                        //attributes device name
                        flag_state[1] = 'T';
                    }
                    if (strcmp(writeData,"d205303e099ceff44835") == 0){
                        //simple read
                        flag_state[2] = 'T';
                    }
                    if (strcmp(writeData,"5cd56d74049ae40f442e") == 0){
                        //md5 of device name
                        flag_state[3] = 'T';
                    }
                    if (strcmp(writeData,"3873c0270763568cf7aa") == 0){
                        //write any
                        flag_state[4] = 'T';
                    }
                    if (strcmp(writeData,"c55c6314b3db0a6128af") == 0){
                        //write ascii
                        flag_state[5] = 'T';
                    }
                    if (strcmp(writeData,"1179080b29f8da16ad66") == 0){
                        //write hex
                        flag_state[6] = 'T';
                    }
                    if (strcmp(writeData,"f8b136d937fad6a2be9f") == 0){
                        //read & write
                        flag_state[7] = 'T';
                    }
                    if (strcmp(writeData,"933c1fcfa8ed52d2ec05") == 0){
                        //brute write
                        flag_state[8] = 'T';
                    }
                    if (strcmp(writeData,"6ffcd214ffebdc0d069e") == 0){
                        //brute write
                        flag_state[9] = 'T';
                    }
                    if (strcmp(writeData,"5ec3772bcd00cf06d8eb") == 0){
                        //notify
                        flag_state[10] = 'T';
                    }
                    if (strcmp(writeData,"c7b86dd121848c77c113") == 0){
                        //indicate
                        flag_state[11] = 'T';
                    }
                    if (strcmp(writeData,"c9457de5fd8cafe349fd") == 0){
                        //notify multi
                        flag_state[12] = 'T';
                    }
                    if (strcmp(writeData,"b6f3a47f207d38e16ffa") == 0){
                        //indicate multi
                        flag_state[13] = 'T';
                    }
                    if (strcmp(writeData,"aca16920583e42bdcf5f") == 0){
                        //mac
                        flag_state[14] = 'T';
                    }
                    if (strcmp(writeData,"b1e409e5a4eaf9fe5158") == 0){
                        //mtu
                        flag_state[15] = 'T';
                    }
                    if (strcmp(writeData,"d41d8cd98f00b204e980") == 0){
                        //write response
                        flag_state[16] = 'T';
                    }
                    if (strcmp(writeData,"fc920c68b6006169477b") == 0){
                        //hidden notify
                        flag_state[17] = 'T';
                    }
                    if (strcmp(writeData,"fbb966958f07e4a0cc48") == 0){
                        //hidden notify
                        flag_state[18] = 'T';
                    }
                    if (strcmp(writeData,"d953bfb9846acc2e15ee") == 0){
                        //final flag
                        flag_state[19] = 'T';
                    }

                    ESP_LOGI(GATTS_TABLE_TAG, "FLAG STATE = %s", flag_state);
                    set_score();
                }
                /* send response when param->write.need_rsp is true*/
                //if (param->write.need_rsp && send_response == 0){
                if (param->write.need_rsp){
                    ESP_LOGI(GATTS_TABLE_TAG, "CATCH ALL SEND RESPONSE TRIGGERED");
                    esp_ble_gatts_send_response(gatts_if, param->write.conn_id, param->write.trans_id, ESP_GATT_OK, NULL);
                }
            }
            else{
                /* handle prepare write */
                ESP_LOGI(GATTS_TABLE_TAG, "PREPARE WRITE TRIGGERED");
                example_prepare_write_event_env(gatts_if, &prepare_write_env, param);
            }
      	    break;
        case ESP_GATTS_EXEC_WRITE_EVT:
            ESP_LOGI(GATTS_TABLE_TAG, "ESP_GATTS_EXEC_WRITE_EVT");
            example_exec_write_event_env(&prepare_write_env, param);
            break;
        case ESP_GATTS_MTU_EVT:
            ESP_LOGI(GATTS_TABLE_TAG, "ESP_GATTS_MTU_EVT, MTU %d", param->mtu.mtu);
            if (param->mtu.mtu == 444) {
                esp_ble_gatts_set_attr_value(blectf_handle_table[IDX_CHAR_FLAG_MTU]+1, 20, (uint8_t *)"b1e409e5a4eaf9fe5158");
            }
            break;
        case ESP_GATTS_CONF_EVT:
            ESP_LOGI(GATTS_TABLE_TAG, "ESP_GATTS_CONF_EVT, status = %d", param->conf.status);
            // notify multi
            if (indicate_handle_state == blectf_handle_table[IDX_CHAR_FLAG_NOTIFICATION_MULTI]){
                char indicate_data[20] = "c9457de5fd8cafe349fd";
                // delay was added cause with none, this crashed the server
                vTaskDelay(100);
                esp_ble_gatts_send_indicate(gatts_if, param->write.conn_id, blectf_handle_table[IDX_CHAR_VAL_FLAG_NOTIFICATION_MULTI], sizeof(indicate_data), (uint8_t *)indicate_data, false);
            }
            // indicate multi
            if (indicate_handle_state == blectf_handle_table[IDX_CHAR_FLAG_INDICATE_MULTI]){
                char indicate_data[20] = "b6f3a47f207d38e16ffa";
                esp_ble_gatts_send_indicate(gatts_if, param->write.conn_id, blectf_handle_table[IDX_CHAR_VAL_FLAG_INDICATE_MULTI], sizeof(indicate_data), (uint8_t *)indicate_data, true);
            }
            break;
        case ESP_GATTS_START_EVT:
            ESP_LOGI(GATTS_TABLE_TAG, "SERVICE_START_EVT, status %d, service_handle %d", param->start.status, param->start.service_handle);
            break;
        case ESP_GATTS_CONNECT_EVT:
            ESP_LOGI(GATTS_TABLE_TAG, "ESP_GATTS_CONNECT_EVT, conn_id = %d", param->connect.conn_id);
            esp_log_buffer_hex(GATTS_TABLE_TAG, param->connect.remote_bda, 6);
            uint8_t match_mac[8] = {0x11,0x22,0x33,0x44,0x55,0x66};
            if (match_mac[0] == param->connect.remote_bda[0] &&
                match_mac[1] == param->connect.remote_bda[1] &&
                match_mac[2] == param->connect.remote_bda[2] &&
                match_mac[3] == param->connect.remote_bda[3] &&
                match_mac[4] == param->connect.remote_bda[4] &&
                match_mac[5] == param->connect.remote_bda[5]){
                ESP_LOGI(GATTS_TABLE_TAG, "THIS IS THE MAC YOU ARE LOOKING FOR");
                esp_ble_gatts_set_attr_value(blectf_handle_table[IDX_CHAR_FLAG_MAC]+1, 20, (uint8_t *)"aca16920583e42bdcf5f");
            }


            esp_ble_conn_update_params_t conn_params = {0};
            memcpy(conn_params.bda, param->connect.remote_bda, sizeof(esp_bd_addr_t));
            /* For the IOS system, please reference the apple official documents about the ble connection parameters restrictions. */
            conn_params.latency = 0;
            conn_params.max_int = 0x20;    // max_int = 0x20*1.25ms = 40ms
            conn_params.min_int = 0x10;    // min_int = 0x10*1.25ms = 20ms
            conn_params.timeout = 400;    // timeout = 400*10ms = 4000ms
            //start sent the update connection parameters to the peer device.
            esp_ble_gap_update_conn_params(&conn_params);
            break;
        case ESP_GATTS_DISCONNECT_EVT:
            ESP_LOGI(GATTS_TABLE_TAG, "ESP_GATTS_DISCONNECT_EVT, reason = %d", param->disconnect.reason);
            indicate_handle_state=0;
            esp_ble_gap_start_advertising(&adv_params);
            break;
        case ESP_GATTS_CREAT_ATTR_TAB_EVT:{
            if (param->add_attr_tab.status != ESP_GATT_OK){
                ESP_LOGE(GATTS_TABLE_TAG, "create attribute table failed, error code=0x%x", param->add_attr_tab.status);
            }
            else if (param->add_attr_tab.num_handle != HRS_IDX_NB){
                ESP_LOGE(GATTS_TABLE_TAG, "create attribute table abnormally, num_handle (%d) \
                        doesn't equal to HRS_IDX_NB(%d)", param->add_attr_tab.num_handle, HRS_IDX_NB);
            }
            else {
                ESP_LOGI(GATTS_TABLE_TAG, "create attribute table successfully, the number handle = %d\n",param->add_attr_tab.num_handle);
                memcpy(blectf_handle_table, param->add_attr_tab.handles, sizeof(blectf_handle_table));
                esp_ble_gatts_start_service(blectf_handle_table[IDX_SVC]);
            }
            break;
        }
        case ESP_GATTS_STOP_EVT:
        case ESP_GATTS_OPEN_EVT:
        case ESP_GATTS_CANCEL_OPEN_EVT:
        case ESP_GATTS_CLOSE_EVT:
            ESP_LOGI(GATTS_TABLE_TAG, "ESP_GATTS_CLOSE_EVT");
            break;
        case ESP_GATTS_LISTEN_EVT:
            ESP_LOGI(GATTS_TABLE_TAG, "ESP_GATTS_LISTEN_EVT");
            break;
        case ESP_GATTS_CONGEST_EVT:
            ESP_LOGI(GATTS_TABLE_TAG, "ESP_GATTS_CONGEST_EVT");
            break;
        case ESP_GATTS_UNREG_EVT:
            ESP_LOGI(GATTS_TABLE_TAG, "ESP_GATTS_UNREG_EVT");
            break;
        case ESP_GATTS_DELETE_EVT:
            ESP_LOGI(GATTS_TABLE_TAG, "ESP_GATTS_DELETE_EVT");
            break;
        case ESP_GATTS_RESPONSE_EVT:
            ESP_LOGI(GATTS_TABLE_TAG, "ESP_GATTS_RESPONSE_EVT");
            //TODO: change the following to set a read value instead of doing a notify
            if (check_send_response == 1){
                check_send_response = 0;
                if (flag_state[16] != 'T'){
                    flag_state[16] = 'H';
                }
            }
            
            break;
        default:
            break;
    }
}


static void gatts_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param)
{

    /* If event is register event, store the gatts_if for each profile */
    if (event == ESP_GATTS_REG_EVT) {
        if (param->reg.status == ESP_GATT_OK) {
            blectf_profile_tab[PROFILE_APP_IDX].gatts_if = gatts_if;
        } else {
            ESP_LOGE(GATTS_TABLE_TAG, "reg app failed, app_id %04x, status %d",
                    param->reg.app_id,
                    param->reg.status);
            return;
        }
    }
    do {
        int idx;
        for (idx = 0; idx < PROFILE_NUM; idx++) {
            /* ESP_GATT_IF_NONE, not specify a certain gatt_if, need to call every profile cb function */
            if (gatts_if == ESP_GATT_IF_NONE || gatts_if == blectf_profile_tab[idx].gatts_if) {
                if (blectf_profile_tab[idx].gatts_cb) {
                    blectf_profile_tab[idx].gatts_cb(event, gatts_if, param);
                }
            }
        }
    } while (0);
}

void app_main()
{
    esp_err_t ret;

    //uint8_t new_mac[8] = {0xDE,0xAD,0xBE,0xEF,0xBE,0xEF};
    //esp_base_mac_addr_set(new_mac);
    
    /* Initialize NVS. */
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK( ret );

    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ret = esp_bt_controller_init(&bt_cfg);
    if (ret) {
        ESP_LOGE(GATTS_TABLE_TAG, "%s enable controller failed", __func__);
        return;
    }

    ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
    if (ret) {
        ESP_LOGE(GATTS_TABLE_TAG, "%s enable controller failed", __func__);
        return;
    }

    ret = esp_bluedroid_init();
    if (ret) {
        ESP_LOGE(GATTS_TABLE_TAG, "%s init bluetooth failed", __func__);
        return;
    }

    ret = esp_bluedroid_enable();
    if (ret) {
        ESP_LOGE(GATTS_TABLE_TAG, "%s enable bluetooth failed", __func__);
        return;
    }

    ret = esp_ble_gatts_register_callback(gatts_event_handler);
    if (ret){
        ESP_LOGE(GATTS_TABLE_TAG, "gatts register error, error code = %x", ret);
        return;
    }

    ret = esp_ble_gap_register_callback(gap_event_handler);
    if (ret){
        ESP_LOGE(GATTS_TABLE_TAG, "gap register error, error code = %x", ret);
        return;
    }

    ret = esp_ble_gatts_app_register(ESP_APP_ID);
    if (ret){
        ESP_LOGE(GATTS_TABLE_TAG, "gatts app register error, error code = %x", ret);
        return;
    }

    esp_err_t local_mtu_ret = esp_ble_gatt_set_local_mtu(500);
    if (local_mtu_ret){
        ESP_LOGE(GATTS_TABLE_TAG, "set local  MTU failed, error code = %x", local_mtu_ret);
    }
}
