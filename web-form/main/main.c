/* The example of nRF905
 *
 * This sample code is in the public domain.
 */

#include <stdio.h>
#include <inttypes.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/message_buffer.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "mdns.h"
#include "cJSON.h"

#include "nRF905.h"
#include "websocket_server.h"

/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group;

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1

static const char *TAG = "MAIN";

static int s_retry_num = 0;

MessageBufferHandle_t xMessageBufferTrans;
MessageBufferHandle_t xMessageBufferRecv;

// The total number of bytes (not single messages) the message buffer will be able to hold at any one time.
size_t xBufferSizeBytes = 1024;
// The size, in bytes, required to hold each item in the message,
size_t xItemSize = NRF905_MAX_PAYLOAD;

#define RXADDR 0xE7E7E7E7 // Address of this device
#define TXADDR 0xE7E7E7E7 // Address of device to send to

static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
	if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
		esp_wifi_connect();
	} else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
		if (s_retry_num < CONFIG_ESP_MAXIMUM_RETRY) {
			esp_wifi_connect();
			s_retry_num++;
			ESP_LOGI(TAG, "retry to connect to the AP");
		} else {
			xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
		}
		ESP_LOGI(TAG,"connect to the AP fail");
	} else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
		ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
		ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
		s_retry_num = 0;
		xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
	}
}

esp_err_t wifi_init_sta(void)
{
	s_wifi_event_group = xEventGroupCreate();

	ESP_ERROR_CHECK(esp_netif_init());
	ESP_ERROR_CHECK(esp_event_loop_create_default());
	esp_netif_create_default_wifi_sta();

	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&cfg));

	esp_event_handler_instance_t instance_any_id;
	esp_event_handler_instance_t instance_got_ip;
	ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
		ESP_EVENT_ANY_ID,
		&event_handler,
		NULL,
		&instance_any_id));
	ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
		IP_EVENT_STA_GOT_IP,
		&event_handler,
		NULL,
		&instance_got_ip));

	wifi_config_t wifi_config = {
		.sta = {
			.ssid = CONFIG_ESP_WIFI_SSID,
			.password = CONFIG_ESP_WIFI_PASSWORD,
			/* Setting a password implies station will connect to all security modes including WEP/WPA.
			 * However these modes are deprecated and not advisable to be used. Incase your Access point
			 * doesn't support WPA2, these mode can be enabled by commenting below line */
			.threshold.authmode = WIFI_AUTH_WPA2_PSK,

			.pmf_cfg = {
				.capable = true,
				.required = false
			},
		},
	};
	ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));
	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
	ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
	ESP_ERROR_CHECK(esp_wifi_start());

	/* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
	 * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
	esp_err_t ret_value = ESP_OK;
	EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
		WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
		pdFALSE,
		pdFALSE,
		portMAX_DELAY);

	/* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
	 * happened. */
	if (bits & WIFI_CONNECTED_BIT) {
		ESP_LOGI(TAG, "connected to ap SSID:%s password:%s", CONFIG_ESP_WIFI_SSID, CONFIG_ESP_WIFI_PASSWORD);
	} else if (bits & WIFI_FAIL_BIT) {
		ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s", CONFIG_ESP_WIFI_SSID, CONFIG_ESP_WIFI_PASSWORD);
		ret_value = ESP_FAIL;
	} else {
		ESP_LOGE(TAG, "UNEXPECTED EVENT");
		ret_value = ESP_FAIL;
	}

	/* The event will not be processed after unregister */
	ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, instance_got_ip));
	ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, instance_any_id));
	vEventGroupDelete(s_wifi_event_group);
	return ret_value;
}

void initialize_mdns(void)
{
	//initialize mDNS
	ESP_ERROR_CHECK( mdns_init() );
	//set mDNS hostname (required if you want to advertise services)
	ESP_ERROR_CHECK( mdns_hostname_set(CONFIG_MDNS_HOSTNAME) );
	ESP_LOGI(TAG, "mdns hostname set to: [%s]", CONFIG_MDNS_HOSTNAME);

#if 0
	//set default mDNS instance name
	ESP_ERROR_CHECK( mdns_instance_name_set("ESP32 with mDNS") );
#endif
}

#if CONFIG_SENDER
void tx_task(void *pvParameters)
{
	ESP_LOGI(pcTaskGetName(NULL), "Start");

	uint8_t buffer[xItemSize];

	while(1) {
		memset(buffer, 0x00, xItemSize);
		size_t received = xMessageBufferReceive(xMessageBufferRecv, buffer, sizeof(buffer), portMAX_DELAY);
		ESP_LOGI(pcTaskGetName(NULL), "xMessageBufferReceive received=%d", received);

		// Show data
		ESP_LOG_BUFFER_HEXDUMP(pcTaskGetName(NULL), buffer, sizeof(buffer), ESP_LOG_INFO);
		ESP_LOGI(pcTaskGetName(NULL),"Sending data: [%s]", buffer);
		
		// Write data
		nRF905_write(TXADDR, buffer, sizeof(buffer));

		// Send the data (send fails if other transmissions are going on, keep trying until success) and enter RX mode on completion
		while(nRF905_TX(NRF905_NEXTMODE_RX, true) == false);
	} // end while

	// never reach here
	vTaskDelete(NULL);
}
#endif // CONFIG_SENDER

#if CONFIG_RECEIVER
void rx_task(void *pvParameters)
{
	ESP_LOGI(pcTaskGetName(NULL), "Start");

	// Set address of this device
	nRF905_setListenAddress(RXADDR);

	// Put into receive mode
	nRF905_RX();

	uint8_t buffer[xItemSize];

	while(1) {
		uint8_t packetStatus = nRF905_poll();
		if (packetStatus == NRF905_RX_INVALID) {
			ESP_LOGW(pcTaskGetName(NULL), "Invalid packet!");
			nRF905_RX();
		} else if (packetStatus == NRF905_ADDR_MATCH) {
			ESP_LOGI(pcTaskGetName(NULL), "Address match!");
		} else if (packetStatus == NRF905_RX_COMPLETE) {
			ESP_LOGI(pcTaskGetName(NULL), "Got packet!");
			// Read payload
			nRF905_read(buffer, sizeof(buffer));
			// Show received data
			ESP_LOG_BUFFER_HEXDUMP(pcTaskGetName(NULL), buffer, sizeof(buffer), ESP_LOG_INFO);
			ESP_LOGI(pcTaskGetName(NULL), "%s", buffer);
			int rxLen = strlen((char*)buffer);

			// WebSockets can only handle printable characters.
			// Therefore, determine whether the characters are printable.
			bool printable = true;
			for (int i=0;i<rxLen;i++) {
				int c = buffer[i];
				if (!isprint(c)) printable = false;
			}
			ESP_LOGI(pcTaskGetName(NULL), "printable=%d", printable);
			if (!printable) {
				ESP_LOGW(TAG, "Contains characters that cannot be printed");
				continue;
			}

			cJSON *root;
			root = cJSON_CreateObject();
			cJSON_AddStringToObject(root, "id", "recv-request");
			cJSON_AddStringToObject(root, "payload", (char*)buffer);
			//char *my_json_string = cJSON_Print(root);
			char *my_json_string = cJSON_PrintUnformatted(root);
			int my_json_length = strlen(my_json_string);
			ESP_LOGD(pcTaskGetName(NULL), "my_json_string=[%s]",my_json_string);
			cJSON_Delete(root);

			size_t spacesAvailable = xMessageBufferSpacesAvailable( xMessageBufferTrans );
			ESP_LOGI(pcTaskGetName(NULL), "spacesAvailable=%d", spacesAvailable);
			size_t sended = xMessageBufferSend(xMessageBufferTrans, my_json_string, my_json_length, 100);
			if (sended != my_json_length) {
				ESP_LOGE(pcTaskGetName(NULL), "xMessageBufferSend fail my_json_length=%d sended=%d", my_json_length, sended);
				break;
			}
			cJSON_free(my_json_string);
		}
		vTaskDelay(1); // Avoid Watchdog asserts
	} // end while

	vTaskDelete(NULL);
}

void task_dummy(void *pvParameters)
{
	ESP_LOGI(pcTaskGetName(NULL), "Start");
	uint8_t buf[xItemSize];
	while(1) {
		size_t received = xMessageBufferReceive(xMessageBufferRecv, buf, sizeof(buf), portMAX_DELAY);
		ESP_LOGI(pcTaskGetName(NULL), "xMessageBufferReceive received=[%.*s]", received, buf);
	} // end while
	vTaskDelete(NULL);
}
#endif // CONFIG_RECEIVER

void client_task(void* pvParameters);
void server_task(void* pvParameters);
	
void app_main()
{
	// Initialize NVS
	esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);

	// Initialize WiFi
	ESP_ERROR_CHECK(wifi_init_sta());

	// Create MessageBuffer
	xMessageBufferTrans = xMessageBufferCreate(xBufferSizeBytes);
	configASSERT( xMessageBufferTrans );
	xMessageBufferRecv = xMessageBufferCreate(xBufferSizeBytes);
	configASSERT( xMessageBufferRecv );

	// Initialize mDNS
	initialize_mdns();

	// Initialize nRF905
	nRF905_begin();
	nRF905_setChannel(CONFIG_RF69_CHANNEL);
	nRF905_printConfig();

	// Get the local IP address
	esp_netif_ip_info_t ip_info;
	ESP_ERROR_CHECK(esp_netif_get_ip_info(esp_netif_get_handle_from_ifkey("WIFI_STA_DEF"), &ip_info));
	char cparam0[64];
	sprintf(cparam0, IPSTR, IP2STR(&ip_info.ip));
	ESP_LOGI(TAG, "cparam0=[%s]", cparam0);

	// Start web socket server
	ws_server_start();

	// Start web server
	xTaskCreate(&server_task, "server_task", 1024*4, (void *)cparam0, 5, NULL);

	// Start web client
	xTaskCreate(&client_task, "client_task", 1024*4, NULL, 5, NULL);

#if CONFIG_SENDER
	xTaskCreate(&tx_task, "TX", 1024*3, NULL, 5, NULL);
#endif
#if CONFIG_RECEIVER
	xTaskCreate(&rx_task, "RX", 1024*3, NULL, 5, NULL);
	xTaskCreate(&task_dummy, "DUMMY", 1024*4, NULL, 5, NULL);
#endif

	while(1) {
		vTaskDelay(10);
	}
}

