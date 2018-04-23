/*
 * =====================================================================================
 *
 *       Filename:  kv_utils.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  04/19/2018 13:03:30
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */
#include <stdlib.h>

#include <stdio.h>
#include <string.h>
#include <esp_partition.h>

typedef struct
{
  uint8_t  ns;         // Namespace ID
  uint8_t  type;       // type of value
  uint8_t  span;       // Number of entries used for this item
  uint8_t  rvs;        // Reserved, should be 0xFF
  uint32_t CRC;        // CRC
  char     key[16];    // key in Ascii
  uint64_t data;       // data in entry 
} nvs_entry_t;

typedef struct                                      // For nvs entries
{                                                   // 1 page is 4096 bytes
  uint32_t  state;
  uint32_t  seqnr;
  
  uint32_t  unused[5];
  uint32_t  CRC;
  uint8_t   bitmap[32];
  nvs_entry_t entry[126];
} nvs_page_t;

#define PARTITION_NAME "nvs"
#define NAMESPACE_NAME "kv_store"

nvs_page_t buffer;
nvs_entry_t namespace_entry;
int n_keys;
char **keys;

typedef bool (*cb_entry)(nvs_entry_t entry);
nvs_entry_t iterate_entries(cb_entry cb) {
	esp_err_t                 result = ESP_OK;

	esp_partition_iterator_t  pi;                              // Iterator for find
	const esp_partition_t*    nvs;                             // Pointer to partition struct

	pi = esp_partition_find (ESP_PARTITION_TYPE_DATA,          // Get partition iterator for
			ESP_PARTITION_SUBTYPE_ANY,        // this partition
			PARTITION_NAME);
	if (pi) {
		nvs = esp_partition_get (pi);                          // Get partition struct
		esp_partition_iterator_release (pi);                   // Release the iterator
	} else {
		printf("Partition %s not found!\n", PARTITION_NAME);
		return (nvs_entry_t){0};
	}

	uint8_t                   pagenr = 0;                      // Page number in NVS
	uint32_t                  offset = 0;                      // Offset in nvs partition
	while (offset < nvs->size) {
		result = esp_partition_read(nvs, offset,                // Read 1 page in nvs partition
				&buffer,
				sizeof(nvs_page_t));
		if (result != ESP_OK) {
			printf("Error reading NVS!\n");
			return (nvs_entry_t){0};
		}
		uint8_t                   bm;                              // bitmap for an entry
		uint8_t i = 0;
		while (i < 126) {
			bm = (buffer.bitmap[i/4] >> ((i % 4) * 2)) & 0x03;  // Get bitmap for this entry
			if (bm == 2) {
				if (cb(buffer.entry[i]) == true) {
					return buffer.entry[i];
				}
				i += buffer.entry[i].span;                              // Next entry
			} else {
				i++;
			}
		}
		offset += sizeof(nvs_page_t);                              // Prepare to read next page in nvs
		pagenr++;
	}
	return (nvs_entry_t){0};
}

bool find_kv_entry(nvs_entry_t entry) {
	return (entry.ns == 0 && strcmp(NAMESPACE_NAME, entry.key) == 0);
}

bool update_n_keys(nvs_entry_t entry) {
	if (entry.ns == (namespace_entry.data & 0xFF)) {
		++n_keys;
	}
	return false;
}

bool update_keys(nvs_entry_t entry) {
	if (entry.ns != (namespace_entry.data & 0xFF)) {
    return false;
  }
  for (int i = 0; i < n_keys; ++i) {
    if (keys[i] == 0) {
      keys[i] = malloc(strlen(entry.key));
      strcpy(keys[i], entry.key);
      break;
    }
  }
  return false;
}

const char **list_keys() {
	if (keys) return (const char**)keys;
  n_keys = 0;
  namespace_entry = iterate_entries(find_kv_entry);
  iterate_entries(update_n_keys);

  keys = malloc(sizeof(char *) * (n_keys + 1));
  memset(keys, 0, sizeof(char *) * (n_keys + 1));

  iterate_entries(update_keys);
  return (const char **)keys;
}

int get_n_keys() {
  return n_keys;
}
