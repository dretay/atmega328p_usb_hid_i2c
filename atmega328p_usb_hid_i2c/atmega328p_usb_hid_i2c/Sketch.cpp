#include <Arduino.h>
#include <cQueue.h>
#include <hidboot.h>
#include <usbhub.h>
#include <SPI.h>
#include <Wire.h>
#include <pb.h>
#include <pb_common.h>
#include <pb_encode.h>
#include "hid.pb.h"

void requestEvent();
void(* resetFunc) (void) = 0;


class MouseRptParser : public MouseReportParser
{
	protected:
	void OnMouseMove  (MOUSEINFO *mi);
	void OnLeftButtonUp (MOUSEINFO *mi);
	void OnLeftButtonDown (MOUSEINFO *mi);
	void OnRightButtonUp  (MOUSEINFO *mi);
	void OnRightButtonDown  (MOUSEINFO *mi);
	void OnMiddleButtonUp (MOUSEINFO *mi);
	void OnMiddleButtonDown (MOUSEINFO *mi);
};
Queue_t    q;
#define  IMPLEMENTATION  FIFO
#define BUFFER_SIZE 32
typedef struct strRec {
	uint8_t buffer[BUFFER_SIZE];
} Rec;

Rec rec;
bool status;
MouseUpdate mouse_update;
pb_ostream_t stream;
USB     Usb;
USBHub     Hub(&Usb);
HIDBoot<USB_HID_PROTOCOL_MOUSE>    HidMouse(&Usb);
MouseRptParser                               Prs;

static void push_mouse_update(MouseUpdate *mouse_update){
	stream = pb_ostream_from_buffer(rec.buffer, sizeof(rec.buffer));
	status = pb_encode_delimited(&stream, MouseUpdate_fields, mouse_update);

	if(status == true){
		status = q_push(&q, &rec);
		if(status == false){
			Serial.println("Circular Buffer Overflow!");
			delay(100);
			resetFunc();
		}
	}
	else{
		Serial.println("PB Encoding Error!");
		Serial.println(PB_GET_ERROR(&stream));
	}
}

void MouseRptParser::OnMouseMove(MOUSEINFO *mi)
{
	mouse_update = MouseUpdate_init_zero;
	mouse_update.type = MouseUpdate_Type_XY;
	mouse_update.x = mi->dX;
	mouse_update.y = mi->dY;
	push_mouse_update(&mouse_update);
};
void MouseRptParser::OnLeftButtonUp  (MOUSEINFO *mi)
{
	mouse_update = MouseUpdate_init_zero;
	mouse_update.type = MouseUpdate_Type_LEFT;
	push_mouse_update(&mouse_update);
};
void MouseRptParser::OnLeftButtonDown (MOUSEINFO *mi)
{
	mouse_update = MouseUpdate_init_zero;
	mouse_update.type = MouseUpdate_Type_LEFT;
	mouse_update.x = 1;
	push_mouse_update(&mouse_update);
};
void MouseRptParser::OnRightButtonUp  (MOUSEINFO *mi)
{
	mouse_update = MouseUpdate_init_zero;
	mouse_update.type = MouseUpdate_Type_RIGHT;
	push_mouse_update(&mouse_update);
};
void MouseRptParser::OnRightButtonDown  (MOUSEINFO *mi)
{
	mouse_update = MouseUpdate_init_zero;
	mouse_update.type = MouseUpdate_Type_RIGHT;
	mouse_update.x = 1;
	push_mouse_update(&mouse_update);
};
void MouseRptParser::OnMiddleButtonUp (MOUSEINFO *mi)
{
	mouse_update = MouseUpdate_init_zero;
	mouse_update.type = MouseUpdate_Type_MIDDLE;
	push_mouse_update(&mouse_update);
};
void MouseRptParser::OnMiddleButtonDown (MOUSEINFO *mi)
{
	mouse_update = MouseUpdate_init_zero;
	mouse_update.type = MouseUpdate_Type_MIDDLE;
	mouse_update.x = 1;
	push_mouse_update(&mouse_update);
};



void setup()
{
	Serial.begin( 115200 );
	#if !defined(__MIPSEL__)
	while (!Serial); // Wait for serial port to connect - used on Leonardo, Teensy and other boards with built-in USB CDC serial connection
	#endif
	Serial.println("Booting...");
	pinMode(2, OUTPUT);	
	digitalWrite(2, LOW);
	delay(1);
	digitalWrite(2, HIGH);

	if (Usb.Init() == -1){
		Serial.println("OSC did not start.");
	}
	else{
		Serial.println("OSC started successfully");
	}
	

	delay( 200 );

	HidMouse.SetReportParser(0, &Prs);
	Wire.begin(0x08);
	Wire.onRequest(requestEvent);

	q_init(&q, sizeof(Rec), 10, IMPLEMENTATION, false);
	pinMode(8, OUTPUT);
	digitalWrite(8, LOW);
	
	Serial.println("Finished booting!");
	
}

void loop()
{
	Usb.Task();
	if(q_isEmpty(&q) == false){
		digitalWrite(8, HIGH);
	}
	else{
		digitalWrite(8, LOW);
	}
}
void requestEvent() {
	Rec rec;
	if(q_isEmpty(&q) == false){
		q_pop(&q, &rec);
		Wire.write(rec.buffer, BUFFER_SIZE);
	}
	else{
		Serial.println("Request on non-empty queue!");
	}
}


