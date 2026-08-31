#ifndef __APP_H
#define __APP_H

#define ADC_BUFFER_SIZE        512
#define ADC_BUFFER_SIZE_BYTES  (ADC_BUFFER_SIZE * 2)
#define UINT12_MAX             4095
#define LED_UNIT               ((ADC_BUFFER_SIZE) / 8)

void app_main(void);

#endif
