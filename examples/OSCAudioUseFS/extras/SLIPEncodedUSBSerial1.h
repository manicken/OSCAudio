/*
Extends the Serial class to encode SLIP over serial
*/
#include "Arduino.h"

#ifndef SLIPEncodedUSBSerial1_h
#define SLIPEncodedUSBSerial1_h


#include <Stream.h>



#if (defined(TEENSYDUINO) && (defined(USB_SERIAL) || defined(USB_DUAL_SERIAL) || defined(USB_TRIPLE_SERIAL) || defined(USB_SERIAL_HID) || defined(USB_MIDI_SERIAL) || defined(USB_MIDI_AUDIO_DUAL_SERIAL) || defined(USB_MIDI4_SERIAL) || defined(USB_MIDI16_SERIAL) || defined(USB_MIDI_AUDIO_SERIAL) || defined(USB_MIDI16_AUDIO_SERIAL))) || (!defined(TEENSYDUINO) && defined(__AVR_ATmega32U4__)) || defined(__SAM3X8E__) || (defined(_USB) && defined(_USE_USB_FOR_SERIAL_))  || defined(_SAMD21_) || (defined(__PIC32MX__) || defined(__PIC32MZ__))


//import the serial USB object
#if defined(TEENSYDUINO) && defined (__arm__)
#if !defined(USB_HOST_TEENSY36_)
#include <usb_serial.h>
#endif

#endif



class SLIPEncodedUSBSerial1: public Stream{
	
private:
	enum erstate {CHAR, FIRSTEOT, SECONDEOT, SLIPESC } rstate;
//different type for each platform

#if  defined(CORE_TEENSY) 
#if defined(USB_HOST_TEENSY36)
    USBSerial1
#else
    usb_serial2_class
#endif
#endif
							* serial;
	
public:
	SLIPEncodedUSBSerial1(
//different constructor for each platform
#if  defined(CORE_TEENSY)
#if defined(USB_HOST_TEENSY36)
                         USBSerial1
#else
                         usb_serial2_class
#endif

#endif
						 &		);
	
	int available();
	int read();
    int readBytes( uint8_t *buffer, size_t size);

	int peek();
	void flush();
	
	//same as Serial.begin
	void begin(unsigned long);
    //SLIP specific method which begins a transmitted packet
	void beginPacket();
	//SLIP specific method which ends a transmittedpacket
	void endPacket();
	// SLIP specific method which indicates that an EOT was received 
	bool endofPacket();
	
	

	//overrides the Stream's write function to encode SLIP
	size_t write(uint8_t b);
    size_t write(const uint8_t *buffer, size_t size);
	//using Print::write;	


};
#endif



#endif
