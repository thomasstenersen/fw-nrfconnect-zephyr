/*
 * Copyright (c) 2016 ARM Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <entropy.h>
#include <stdio.h>
#include <kernel.h>
#include <irq.h>
#include <nrf52.h>

static void my_isr(void *param)
{
	printf("Inside interrupt\n");

	struct device *dev = param;

	for (size_t j = 0; j < 5; j++) {
#define BUFFER_LENGTH 10
		u8_t buffer[BUFFER_LENGTH] = {0};
		int r;

		/* The BUFFER_LENGTH-1 is used so the driver will not
		 * write the last byte of the buffer. If that last
		 * byte is not 0 on return it means the driver wrote
		 * outside the passed buffer, and that should never
		 * happen.
		 */
		r = entropy_get_entropy_isr(dev, buffer, BUFFER_LENGTH-1, ENTROPY_BUSYWAIT);
//		if (!r) {
//			printf("entropy_get_entropy_isr failed: %d\n", r);
//			break;
//		}

		if (buffer[BUFFER_LENGTH-1] != 0) {
			printf("entropy_get_entropy_isr buffer overflow\n");
		}

		for (int i = 0; i < BUFFER_LENGTH-1; i++) {
			printf("  0x%02x", buffer[i]);
		}

		printf("\n");

//		k_sleep(1000);
	}
}

void main(void)
{
	struct device *dev;

	printf("Entropy Example! %s\n", CONFIG_ARCH);

	dev = device_get_binding(CONFIG_ENTROPY_NAME);
	if (!dev) {
		printf("error: no entropy device\n");
		return;
	}

	irq_connect_dynamic(QDEC_IRQn, 4 /*CONFIG_ENTROPY_NRF5_PRI*/, my_isr, dev, 0);
	irq_enable(QDEC_IRQn);


	printf("entropy device is %p, name is %s\n",
	       dev, dev->config->name);

	while(1)  {
		k_sleep(1000);
		NVIC_SetPendingIRQ(QDEC_IRQn);
	}

	while (1) {
#define BUFFER_LENGTH 10
		u8_t buffer[BUFFER_LENGTH] = {0};
		int r;

		/* The BUFFER_LENGTH-1 is used so the driver will not
		 * write the last byte of the buffer. If that last
		 * byte is not 0 on return it means the driver wrote
		 * outside the passed buffer, and that should never
		 * happen.
		 */
		r = entropy_get_entropy(dev, buffer, BUFFER_LENGTH-1);
		if (r) {
			printf("entropy_get_entropy failed: %d\n", r);
			break;
		}

		if (buffer[BUFFER_LENGTH-1] != 0) {
			printf("entropy_get_entropy buffer overflow\n");
		}

		for (int i = 0; i < BUFFER_LENGTH-1; i++) {
			printf("  0x%02x", buffer[i]);
		}

		printf("\n");

		k_sleep(1000);

//		NVIC_SetPendingIRQ(QDEC_IRQn);
	}
}

