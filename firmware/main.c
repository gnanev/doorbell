#include "common.h"
#include "433rc_receiver.h"

u32 data = 0;
		
int main(void)
{
    setup433Receiver();
	
	DDRC = 1;
	
    while (1) 
    {
		if (get433Data(&data)) {
			if (data == 32867592) {
				PORTC = 1;
				_delay_ms(200);
				PORTC = 0;
				_delay_ms(1000);
				PORTC = 1;
				_delay_ms(200);
				PORTC = 0;
			}
			
			data = 0;
			dump433Data();
		}
    }
}

