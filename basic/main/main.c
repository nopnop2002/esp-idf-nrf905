/* The example of ESP-IDF
 *
 * This sample code is in the public domain.
 */

#include "string.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "nRF905.h"

#define RXADDR 0xE7E7E7E7 // Address of this device
#define TXADDR 0xE7E7E7E7 // Address of device to send to

#define PAYLOAD_SIZE	NRF905_MAX_PAYLOAD

#if CONFIG_SENDER
void tx_task(void *pvParameters)
{
	ESP_LOGI(pcTaskGetName(NULL), "Start");

	// Initialize PHY
	nRF905_begin();
	nRF905_printConfig();

	uint8_t buffer[PAYLOAD_SIZE];
	uint8_t counter = 0;

	while(1) {
		memset(buffer, counter, PAYLOAD_SIZE);
		counter++;

		// Show data
		ESP_LOG_BUFFER_HEXDUMP(pcTaskGetName(0), buffer, PAYLOAD_SIZE, ESP_LOG_INFO);
		
		// Write reply data and destination address to radio IC
		nRF905_write(TXADDR, buffer, sizeof(buffer));

		// Send the data (send fails if other transmissions are going on, keep trying until success) and enter RX mode on completion
		while(nRF905_TX(NRF905_NEXTMODE_RX, true) == false);
		vTaskDelay(100);
	}
}
#endif // CONFIG_SENDER


#if CONFIG_RECEIVER
void rx_task(void *pvParameters)
{
	ESP_LOGI(pcTaskGetName(NULL), "Start");

	// Initialize PHY
	nRF905_begin();
	nRF905_printConfig();

	// Set address of this device
	nRF905_setListenAddress(RXADDR);

	// Put into receive mode
	nRF905_RX();

	uint8_t buffer[PAYLOAD_SIZE];

	while(1) {
		uint8_t packetStatus = nRF905_poll();
		if (packetStatus == NRF905_RX_INVALID) {
			ESP_LOGW(pcTaskGetName(0), "Invalid packet!");
			nRF905_RX();
		} else if (packetStatus == NRF905_ADDR_MATCH) {
			ESP_LOGI(pcTaskGetName(0), "Address match!");
		} else if (packetStatus == NRF905_RX_COMPLETE) {
			ESP_LOGI(pcTaskGetName(0), "Got packet!");
			// Read payload
			nRF905_read(buffer, sizeof(buffer));
			// Show received data
			ESP_LOG_BUFFER_HEXDUMP(pcTaskGetName(0), buffer, PAYLOAD_SIZE, ESP_LOG_INFO);
		}
		vTaskDelay(10);
	}
}
#endif // CONFIG_RECEIVER

void app_main()
{
#if CONFIG_SENDER
	xTaskCreate(&tx_task, "sender", 1024*3, NULL, 5, NULL);
#endif
#if CONFIG_RECEIVER
	xTaskCreate(&rx_task, "receiver", 1024*3, NULL, 5, NULL);
#endif
}

