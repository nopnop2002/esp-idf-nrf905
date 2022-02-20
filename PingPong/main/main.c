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

#define TIMEOUT 500 // 500ms ping timeout

#define PAYLOAD_SIZE	NRF905_MAX_PAYLOAD

#if CONFIG_CLIENT
void client_task(void *pvParameters)
{
	ESP_LOGI(pcTaskGetName(NULL), "Start");

	// Initialize PHY
	nRF905_begin();

	uint8_t buffer[PAYLOAD_SIZE];
	uint8_t replyBuffer[PAYLOAD_SIZE];
	uint8_t counter = 0;
	uint32_t sent = 0;
	uint32_t replies = 0;
	uint32_t timeouts = 0;
	uint32_t invalids = 0;
	uint32_t badData = 0;

	while(1) {
		memset(buffer, counter, PAYLOAD_SIZE);
		counter++;

		// Show data
		ESP_LOG_BUFFER_HEXDUMP(pcTaskGetTaskName(0), buffer, PAYLOAD_SIZE, ESP_LOG_INFO);
		
		// Write reply data and destination address to radio IC
		nRF905_write(TXADDR, buffer, sizeof(buffer));

		// Send the data (send fails if other transmissions are going on, keep trying until success) and enter RX mode on completion
		while(nRF905_TX(NRF905_NEXTMODE_RX, true) == false);
		sent++;

		ESP_LOGI(pcTaskGetTaskName(0), "Waiting for reply...");

		// Wait for reply with timeout
		uint32_t sendStartTime = millis();
		uint8_t packetStatus;
		while(1)
		{
			packetStatus = nRF905_poll();
			ESP_LOGD(pcTaskGetTaskName(0), "packetStatus=%d", packetStatus);

			if(packetStatus == NRF905_RX_COMPLETE) break;
			if(packetStatus == NRF905_RX_INVALID) break;
			if(millis() - sendStartTime > TIMEOUT) break;
			vTaskDelay(1);
		}

		if(packetStatus == NRF905_NONE)
		{
			ESP_LOGW(pcTaskGetTaskName(0), "Ping timed out!");
			timeouts++;
		} else if (packetStatus == NRF905_RX_INVALID) {
			ESP_LOGW(pcTaskGetTaskName(0), "Invalid packet!");
			invalids++;
			//nRF905_RX();
		} else if (packetStatus == NRF905_RX_COMPLETE) {
			replies++;
			ESP_LOGI(pcTaskGetTaskName(0), "Got packet!");
			// Read payload
			nRF905_read(replyBuffer, sizeof(replyBuffer));
			// Validate data
			for(uint8_t i=0;i<PAYLOAD_SIZE;i++)
			{
				if(replyBuffer[i] != counter)
				{
					badData++;
					ESP_LOGW(pcTaskGetTaskName(0), "Bad data!");
					break;
				}
			}
			// Show received data
			ESP_LOG_BUFFER_HEXDUMP(pcTaskGetTaskName(0), replyBuffer, PAYLOAD_SIZE, ESP_LOG_INFO);
		}

		ESP_LOGI(pcTaskGetTaskName(0), "Send:%d Replies:%d Timeouts:%d Invalid:%d Bad:%d", sent, replies, timeouts, invalids, badData);
		vTaskDelay(100);
	} // end while
}
#endif // CONFIG_CLIENT


#if CONFIG_SERVER
void server_task(void *pvParameters)
{
	ESP_LOGI(pcTaskGetName(NULL), "Start");

	// Initialize PHY
	nRF905_begin();

	// Set address of this device
	nRF905_setListenAddress(RXADDR);

	// Put into receive mode
	nRF905_RX();

	uint32_t pings = 0;
	uint32_t invalids = 0;
	uint32_t badData = 0;
	uint8_t buffer[PAYLOAD_SIZE];
	uint8_t replyBuffer[PAYLOAD_SIZE];

	while(1) {
		uint8_t packetStatus = nRF905_poll();
		if (packetStatus == NRF905_RX_INVALID) {
			ESP_LOGW(pcTaskGetTaskName(0), "Invalid packet!");
			invalids++;
			nRF905_RX();
		} else if (packetStatus == NRF905_RX_COMPLETE) {
			ESP_LOGI(pcTaskGetTaskName(0), "Got packet!");
			pings++;
			// Read payload
			nRF905_read(buffer, sizeof(buffer));
			// Show received data
			ESP_LOG_BUFFER_HEXDUMP(pcTaskGetTaskName(0), buffer, PAYLOAD_SIZE, ESP_LOG_INFO);

			memcpy(replyBuffer, buffer, PAYLOAD_SIZE);
			// Validate data and modify
			// Each byte ofthe payload should be the same value, increment this value and send back to the client
			bool dataIsBad = false;
			uint8_t value = replyBuffer[0];
			for(uint8_t i=0;i<PAYLOAD_SIZE;i++)
			{
				if(replyBuffer[i] == value)
					replyBuffer[i]++;
				else
				{
					badData++;
					dataIsBad = true;
					break;
				}
			}
			if(dataIsBad)
				ESP_LOGI(pcTaskGetTaskName(0),"Received data was bad!");

			// Write reply data and destination address to radio
			nRF905_write(TXADDR, replyBuffer, sizeof(replyBuffer));

			// Send the reply data, once the transmission has completed go into receive mode
			while(!nRF905_TX(NRF905_NEXTMODE_RX, true));

			ESP_LOGI(pcTaskGetTaskName(0),"Pings:%d Invalid:%d Bad:%d", pings, invalids, badData);
		}
		vTaskDelay(10);
	}
}
#endif // CONFIG_SERVER

void app_main()
{
#if CONFIG_CLIENT
	xTaskCreate(&client_task, "client", 1024*4, NULL, 5, NULL);
#endif
#if CONFIG_SERVER
	xTaskCreate(&server_task, "server", 1024*4, NULL, 5, NULL);
#endif
}

