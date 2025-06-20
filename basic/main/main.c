/* The example of ESP-IDF
 *
 * This sample code is in the public domain.
 */

#include <stdio.h>
#include <inttypes.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "nRF905.h"

#define RXADDR 0xE7E7E7E7 // Address of this device
#define TXADDR 0xE7E7E7E7 // Address of device to send to

#define PAYLOAD_SIZE NRF905_MAX_PAYLOAD // 32

#if CONFIG_SENDER
void tx_task(void *pvParameters)
{
	ESP_LOGI(pcTaskGetName(NULL), "Start");

	// Initialize PHY
	nRF905_begin();
	nRF905_setChannel(CONFIG_RF69_CHANNEL);
	nRF905_printConfig();

	uint8_t buffer[PAYLOAD_SIZE];

	while(1) {
		memset(buffer, 0, PAYLOAD_SIZE);
		TickType_t nowTick = xTaskGetTickCount();
		sprintf((char *)buffer, "Hello World %"PRIu32, nowTick);

		// Show data
		//ESP_LOG_BUFFER_HEXDUMP(pcTaskGetName(NULL), buffer, PAYLOAD_SIZE, ESP_LOG_INFO);
		ESP_LOGI(pcTaskGetName(NULL),"Sending data: [%s]", buffer);
		
		// Write data
		nRF905_write(TXADDR, buffer, sizeof(buffer));

		// Send the data (send fails if other transmissions are going on, keep trying until success) and enter RX mode on completion
		while(nRF905_TX(NRF905_NEXTMODE_RX, true) == false);
		vTaskDelay(100);
	}

	// never reach here
	vTaskDelete( NULL );
}
#endif // CONFIG_SENDER


#if CONFIG_RECEIVER
void rx_task(void *pvParameters)
{
	ESP_LOGI(pcTaskGetName(NULL), "Start");

	// Initialize PHY
	nRF905_begin();
	nRF905_setChannel(CONFIG_RF69_CHANNEL);
	nRF905_printConfig();

	// Set address of this device
	nRF905_setListenAddress(RXADDR);

	// Put into receive mode
	nRF905_RX();

	uint8_t buffer[PAYLOAD_SIZE];

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
			//ESP_LOG_BUFFER_HEXDUMP(pcTaskGetName(NULL), buffer, PAYLOAD_SIZE, ESP_LOG_INFO);
			ESP_LOGI(pcTaskGetName(NULL), "%s", buffer);
		}
		vTaskDelay(1); // Avoid Watchdog asserts
	}

	// never reach here
	vTaskDelete( NULL );
}
#endif // CONFIG_RECEIVER

void app_main()
{
#if CONFIG_SENDER
	xTaskCreate(&tx_task, "TX", 1024*3, NULL, 5, NULL);
#endif
#if CONFIG_RECEIVER
	xTaskCreate(&rx_task, "RX", 1024*3, NULL, 5, NULL);
#endif
}

