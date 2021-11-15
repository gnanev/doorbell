#include "433rc_receiver.h"

#define INPUT_PORT	PINC
#define INPUT_PIN	2

#define LOW_LEVEL_THRESHOLD	70
#define MAX_BUFF_SIZE		25

#define LOGIC_0_MIN	16
#define LOGIC_0_MAX	25
#define LOGIC_1_MIN	51
#define LOGIC_1_MAX	62

//#define VALIDATING_MASK 0x01F00000

volatile BOOL blockReceive = FALSE;
volatile BOOL dataReceived = FALSE;
volatile u16 timeCount = 0;
volatile u08 prevState = 0;
volatile BOOL isReceiving = FALSE;
volatile u08 currentPulse = 0;
volatile u32 receivedData = 0;
volatile u32 issuedData = 0;

#ifdef RC_DIAGNOSTICS
volatile u08 buffPulses[MAX_BUFF_SIZE];
#endif

void resetDiagBuff();
void setDiagPin();
void clearDiagPin();

ISR(TIMER0_OVF_vect)
{
	register u08 state asm("r23");
	
	state = (INPUT_PORT & INPUT_PIN) >> (INPUT_PIN - 1);
	
	timeCount++;
	
	if (isReceiving) {
		// pin is held in low state for long enough after last bit is received ->
		// stop receiving and issue packet
		if ((state == 0) && (timeCount > LOW_LEVEL_THRESHOLD)) {
			isReceiving = FALSE;
			// must be exact count, otherwise is considered noise
			if (currentPulse == MAX_BUFF_SIZE) {
				// receiving can be blocked in order to process data
				if (!blockReceive)
				{
#ifdef VALIDATING_MASK
					if ((receivedData & VALIDATING_MASK) == VALIDATING_MASK) {
						// packet validated successfully
						issuedData = receivedData & ~VALIDATING_MASK;
						dataReceived = TRUE;
					}
#else
					issuedData = receivedData;
					dataReceived = TRUE;
#endif
				}
			}
			receivedData = 0;
			currentPulse = 0;
			prevState = state;
			timeCount = 0;
			clearDiagPin();
			return;
		}
		
		// edge found -> log pulse width
		if ((state == 0) && (prevState == 1)) {
			if (currentPulse == MAX_BUFF_SIZE) {
				// noise, drop packet
				isReceiving = FALSE;
				currentPulse = 0;
				receivedData = 0;
				clearDiagPin();
			}
			else {
#ifdef RC_DIAGNOSTICS
				buffPulses[currentPulse] = timeCount;
#endif				
				if ((timeCount > LOGIC_1_MIN) && (timeCount < LOGIC_1_MAX)) {
					receivedData = receivedData | ((u32)1 << (MAX_BUFF_SIZE - currentPulse - 1)); // little endian	
					currentPulse++;
				}
				else if ((timeCount > LOGIC_0_MIN) && (timeCount < LOGIC_0_MAX)) {
					currentPulse++;
				}
				else {
					// pulse does not fit into time templates
					// considered noise
					isReceiving = FALSE;
					currentPulse = 0;
					receivedData = 0;
					clearDiagPin();			
				}
			}
		}
	}
	
	if (state == prevState)
		return;

	// pin is held in low state for long enough -> start receiving
	if ((prevState == 0) && (timeCount > LOW_LEVEL_THRESHOLD)) {
		isReceiving = TRUE;
		setDiagPin();
	}

	prevState = state;
	timeCount = 0;
}


void resetDiagBuff()
{
#ifdef RC_DIAGNOSTICS
	for (u08 i=0; i<MAX_BUFF_SIZE; i++)
		buffPulses[i] = 0;
#endif
}

void setDiagPin()
{
#ifdef RC_DIAGNOSTICS
	PORTC = 1;
#endif 
}

void clearDiagPin()
{
#ifdef RC_DIAGNOSTICS
	PORTC = 0;
#endif
}

BOOL getDiagnosticsBuffer(u08* buff)
{
#ifdef RC_DIAGNOSTICS
	for (u08 i=0; i<MAX_BUFF_SIZE; i++)
		buff[i] = buffPulses[i];
	return TRUE;
#else
	return FALSE;
#endif
}

void setup433Receiver()
{
#ifdef RC_DIAGNOSTICS
	resetDiagBuff();
	DDRC = 1;
#endif

	TCCR0A = 0;
	TCCR0B = 0x01;
	TIMSK0 = 1;
		
	sei();
}

BOOL get433Data(u32* data)
{
	if (!dataReceived)
		return FALSE;
		
	blockReceive = TRUE;
	*data = issuedData;
	issuedData = 0;
	dataReceived = FALSE;
	blockReceive = FALSE;
	return TRUE;
}

void dump433Data()
{
	blockReceive = TRUE;
	issuedData = 0;
	receivedData = 0;
	currentPulse = 0;
	prevState = 0;
	timeCount = 0;
	clearDiagPin();
	blockReceive = FALSE;
}