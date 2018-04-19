/*
 * =====================================================================================
 *
 *       Filename:  ble.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  04/19/2018 10:49:16
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */
#include <stdio.h>
#include <string.h>

#include "kv.h"
#include "ble.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_bt.h"

#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_bt_main.h"
#include "esp_gatt_common_api.h"

#define ESP_APP_ID                  0x55
#define SAMPLE_DEVICE_NAME          "🤖🍁"
#define SVC_INST_ID                 0
#define ADV_CONFIG_FLAG             (1 << 0)
#define SCAN_RSP_CONFIG_FLAG        (1 << 1)

esp_gatt_if_t g_gatts_if;
uint16_t conn_id;

static uint8_t service_uuid[16] = {
    /* LSB <--------------------------------------------------------------------------------> MSB */
    //first uuid, 16bit, [12],[13] is the value
    0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00,
};

static const uint16_t GATTS_SERVICE_UUID      = 0x00FF;

#define CHAR_DECLARATION_SIZE       (sizeof(uint8_t))

static const uint16_t primary_service_uuid         = ESP_GATT_UUID_PRI_SERVICE;
static const uint16_t character_declaration_uuid   = ESP_GATT_UUID_CHAR_DECLARE;
static const uint16_t character_client_config_uuid = ESP_GATT_UUID_CHAR_CLIENT_CONFIG;
static const uint8_t char_prop_read                =  ESP_GATT_CHAR_PROP_BIT_READ;
static const uint8_t char_prop_write               = ESP_GATT_CHAR_PROP_BIT_WRITE;
static const uint8_t char_prop_read_write_notify   = ESP_GATT_CHAR_PROP_BIT_WRITE | ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_NOTIFY;

static uint8_t adv_config_done       = 0;

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
    .service_uuid_len    = sizeof(service_uuid),
    .p_service_uuid      = service_uuid,
    .flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
};

static esp_ble_adv_params_t adv_params = {
    .adv_int_min         = 0x20,
    .adv_int_max         = 0x40,
    .adv_type            = ADV_TYPE_IND,
    .own_addr_type       = BLE_ADDR_TYPE_PUBLIC,
    .channel_map         = ADV_CHNL_ALL,
    .adv_filter_policy   = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
};

esp_gatts_attr_db_t *gatt_db;

const esp_gatts_attr_db_t *get_gatts_db() {
  if (gatt_db) return gatt_db;
	const char **keys = list_keys();
  int n_keys = get_n_keys();
  gatt_db = malloc((1 + n_keys * 3) * sizeof(esp_gatts_attr_db_t));

  gatt_db[0] = (esp_gatts_attr_db_t) {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&primary_service_uuid, ESP_GATT_PERM_READ,
    sizeof(uint16_t), sizeof(GATTS_SERVICE_UUID), (uint8_t *)&GATTS_SERVICE_UUID}};

  for (int i = 0; i < n_keys; ++i) {
    int16_t value_id = 0xff00 + (i & 0xff);
    char value[100] = {0};
    if (hasi(keys[i])) {
      snprintf(value, 99, "%d", geti(keys[i]));
    } else if (hasstr(keys[i])) {
      getstr(keys[i], value, 99);
    } else {
      printf("no value for %s (?)\n", keys[i]);
      continue;
    }
    printf("%s: %s\n", keys[i], value);
    gatt_db[(i * 3) + 1] = (esp_gatts_attr_db_t) {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ, CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_read_write_notify}},
    gatt_db[(i * 3) + 2] = (esp_gatts_attr_db_t) {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&value_id, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE, 100, strlen(value), (uint8_t *)value}};
    gatt_db[(i * 3) + 3] = (esp_gatts_attr_db_t) {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_client_config_uuid, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE, strlen(keys[i]), strlen(keys[i]), (uint8_t *)keys[i]}};
  }
  return gatt_db;
}

static void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param) {
  switch (event) {
    case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
      adv_config_done &= (~ADV_CONFIG_FLAG);
      if (adv_config_done == 0) {
        esp_ble_gap_start_advertising(&adv_params);
      }
      break;
    case ESP_GAP_BLE_SCAN_RSP_DATA_SET_COMPLETE_EVT:
      adv_config_done &= (~SCAN_RSP_CONFIG_FLAG);
      if (adv_config_done == 0) {
        esp_ble_gap_start_advertising(&adv_params);
      }
      break;
    case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
      if (param->adv_start_cmpl.status != ESP_BT_STATUS_SUCCESS) {
        printf("advertising start failed\n");
      } else {
        printf("advertising start successfully\n");
      }
      break;
    case ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT:
      if (param->adv_stop_cmpl.status != ESP_BT_STATUS_SUCCESS) {
        printf("Advertising stop failed\n");
      } else {
        printf("Stop adv successfully\n");
      }
      break;
    case ESP_GAP_BLE_UPDATE_CONN_PARAMS_EVT:
      printf("update connetion params status = %d, min_int = %d, max_int = %d,conn_int = %d,latency = %d, timeout = %d\n",
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

static void gatts_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param) {
  switch (event) {
    case ESP_GATTS_REG_EVT: {
      g_gatts_if = gatts_if;
      esp_err_t set_dev_name_ret = esp_ble_gap_set_device_name(SAMPLE_DEVICE_NAME);
      if (set_dev_name_ret) {
          printf("set device name failed, error code = %x\n", set_dev_name_ret);
      }
      //config adv data
      esp_err_t ret = esp_ble_gap_config_adv_data(&adv_data);
      if (ret) {
          printf("config adv data failed, error code = %x\n", ret);
      }
      adv_config_done |= ADV_CONFIG_FLAG;
      //config scan response data
      ret = esp_ble_gap_config_adv_data(&scan_rsp_data);
      if (ret) {
          printf("config scan response data failed, error code = %x\n", ret);
      }
      int n_keys = get_n_keys();
      adv_config_done |= SCAN_RSP_CONFIG_FLAG;
      esp_err_t create_attr_ret = esp_ble_gatts_create_attr_tab(get_gatts_db(), gatts_if, (1 + n_keys * 3), SVC_INST_ID);
      if (create_attr_ret) {
          printf("create attr table failed, error code = %x\n", create_attr_ret);
      }
  }
      break;
    case ESP_GATTS_READ_EVT:
        printf("ESP_GATTS_READ_EVT\n");
        break;
    case ESP_GATTS_WRITE_EVT:
        printf("ESP_GATTS_WRITE_EVT\n");
        break;
    case ESP_GATTS_EXEC_WRITE_EVT:
        printf("ESP_GATTS_EXEC_WRITE_EVT\n");
        break;
    case ESP_GATTS_MTU_EVT:
        printf("ESP_GATTS_MTU_EVT, MTU %d\n", param->mtu.mtu);
        break;
    case ESP_GATTS_CONF_EVT:
        printf("ESP_GATTS_CONF_EVT, status = %d\n", param->conf.status);
        break;
    case ESP_GATTS_START_EVT:
        printf("SERVICE_START_EVT, status %d, service_handle %d\n", param->start.status, param->start.service_handle);
        break;
    case ESP_GATTS_CONNECT_EVT:
        printf("ESP_GATTS_CONNECT_EVT, conn_id = %d\n", param->connect.conn_id);
        esp_ble_conn_update_params_t conn_params = {0};
        memcpy(conn_params.bda, param->connect.remote_bda, sizeof(esp_bd_addr_t));
        /* For the IOS system, please reference the apple official documents about the ble connection parameters restrictions. */
        conn_params.latency = 0;
        conn_params.max_int = 0x20;    // max_int = 0x20*1.25ms = 40ms
        conn_params.min_int = 0x10;    // min_int = 0x10*1.25ms = 20ms
        conn_params.timeout = 400;    // timeout = 400*10ms = 4000ms
        //start sent the update connection parameters to the peer device.
        conn_id = param->connect.conn_id;
        esp_ble_gap_update_conn_params(&conn_params);
        break;
    case ESP_GATTS_DISCONNECT_EVT:
        printf("ESP_GATTS_DISCONNECT_EVT, reason = %d\n", param->disconnect.reason);
        esp_ble_gap_start_advertising(&adv_params);
        break;
    case ESP_GATTS_CREAT_ATTR_TAB_EVT:{
        printf("ESP_GATTS_CREAT_ATTR_TAB_EVT\n");
        break;
    }
    case ESP_GATTS_STOP_EVT:
        printf("ESP_GATTS_STOP_EVT\n");
        break;
    case ESP_GATTS_OPEN_EVT:
        printf("ESP_GATTS_OPEN_EVT\n");
        break;
    case ESP_GATTS_CANCEL_OPEN_EVT:
        printf("ESP_GATTS_CANCEL_OPEN_EVT\n");
        break;
    case ESP_GATTS_CLOSE_EVT:
        printf("ESP_GATTS_CLOSE_EVT\n");
        break;
    case ESP_GATTS_LISTEN_EVT:
        printf("ESP_GATTS_LISTEN_EVT\n");
        break;
    case ESP_GATTS_CONGEST_EVT:
        printf("ESP_GATTS_CONGEST_EVT\n");
        break;
    case ESP_GATTS_UNREG_EVT:
        printf("ESP_GATTS_UNREG_EVT\n");
        break;
    case ESP_GATTS_DELETE_EVT:
        printf("ESP_GATTS_DELETE_EVT\n");
        break;
    default:
        break;
  }
}

void init_ble() {
  esp_err_t ret;
  ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));

  esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
  ret = esp_bt_controller_init(&bt_cfg);
  if (ret) {
    printf("%s enable controller failed\n", __func__);
    return;
  }

  ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
  if (ret) {
    printf("%s enable controller failed\n", __func__);
    return;
  }

  ret = esp_bluedroid_init();
  if (ret) {
    printf("%s init bluetooth failed\n", __func__);
    return;
  }

  ret = esp_bluedroid_enable();
  if (ret) {
    printf("%s enable bluetooth failed\n", __func__);
    return;
  }

  ret = esp_ble_gatts_register_callback(gatts_event_handler);
  if (ret) {
    printf("gatts register error, error code = %x\n", ret);
    return;
  }

  ret = esp_ble_gap_register_callback(gap_event_handler);
  if (ret) {
    printf("gap register error, error code = %x\n", ret);
    return;
  }

  ret = esp_ble_gatts_app_register(ESP_APP_ID);
  if (ret) {
    printf("gatts app register error, error code = %x\n", ret);
    return;
  }

  esp_err_t local_mtu_ret = esp_ble_gatt_set_local_mtu(500);
  if (local_mtu_ret) {
    printf("set local  MTU failed, error code = %x\n", local_mtu_ret);
  }
}
