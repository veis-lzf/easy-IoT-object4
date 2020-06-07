#ifndef _DHT11_H
#define _DHT11_H

struct _dht11_data{
	uint8_t data[5];
	uint8_t valid;
};

void init_dht11(void);
void dht11_start_read(void);
int dht11_read_data(uint8_t buff[5]);

#endif
