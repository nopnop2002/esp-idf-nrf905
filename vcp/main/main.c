/* The example of nRF905
 *
 * This sample code is in the public domain.
 */

#include <stdio.h>
#include <inttypes.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/message_buffer.h"
#include "esp_log.h"

#include "nRF905.h"

static const char *TAG = "MAIN";

MessageBufferHandle_t xMessageBufferTx;
MessageBufferHandle_t xMessageBufferRx;

// The total number of bytes (not single messages) the message buffer will be able to hold at any one time.
size_t xBufferSizeBytes = 1024;
// The size, in bytes, required to hold each item in the message,
size_t xItemSize = NRF905_MAX_PAYLOAD;

#define RXADDR 0xE7E7E7E7 // Address of this device
#define TXADDR 0xE7E7E7E7 // Address of device to send to

#if CONFIG_SENDER
void tx_task(void *pvParameters)
{
	ESP_LOGI(pcTaskGetName(NULL), "Start");

	uint8_t buffer[xItemSize];

	while(1) {
		memset(buffer, 0x00, xItemSize);
		size_t received = xMessageBufferReceive(xMessageBufferRx, buffer, sizeof(buffer), portMAX_DELAY);
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

			int rxLen = strlen((char *)buffer);
			ESP_LOGI(pcTaskGetName(NULL), "rxLen=%d", rxLen);
			size_t spacesAvailable = xMessageBufferSpacesAvailable( xMessageBufferTx );
			ESP_LOGI(pcTaskGetName(NULL), "spacesAvailable=%d", spacesAvailable);
			size_t sended = xMessageBufferSend(xMessageBufferTx, buffer, rxLen, 100);
			if (sended != rxLen) {
				ESP_LOGE(pcTaskGetName(NULL), "xMessageBufferSend fail rxLen=%d sended=%d", rxLen, sended);
				break;
			}
		}
		vTaskDelay(1); // Avoid Watchdog asserts
	} // end while

	vTaskDelete(NULL);
}
#endif // CONFIG_RECEIVER

void cdc_acm_vcp_task(void *pvParameters);

void app_main()
{
	// Create MessageBuffer
	xMessageBufferTx = xMessageBufferCreate(xBufferSizeBytes);
	configASSERT( xMessageBufferTx );
	xMessageBufferRx = xMessageBufferCreate(xBufferSizeBytes);
	configASSERT( xMessageBufferRx );

	// Initialize nRF905
	nRF905_begin();
	nRF905_setChannel(CONFIG_RF69_CHANNEL);
	nRF905_printConfig();

#if CONFIG_SENDER
	xTaskCreate(&tx_task, "TX", 1024*3, NULL, 5, NULL);
#endif
#if CONFIG_RECEIVER
	xTaskCreate(&rx_task, "RX", 1024*3, NULL, 5, NULL);
#endif
    // Start CDC_ACM_VCP
    xTaskCreate(&cdc_acm_vcp_task, "CDC_ACM_VCP", 1024*4, NULL, 5, NULL);
}

