#include "scrollmouserptparser.h"

ScrollMouseReportParser::ScrollMouseReportParser(ScrollMouseEvents *evt) :
mouseEvents(evt),
oldHat(0xDE),
oldButtons(0) {
	for (uint8_t i = 0; i < RPT_GEMEPAD_LEN; i++)
	oldPad[i] = 0xD;
}

void ScrollMouseReportParser::Parse(USBHID *hid, bool is_rpt_id, uint8_t len, uint8_t *buf) {
	bool match = true;

#ifdef DEBUG_USB_HOST
	for(uint8_t i = 0; i < len; i++) {
        PrintHex<uint8_t > (buf[i], 0x80);
        USBTRACE1(" ", 0x80);
    }
    if(len)
        USBTRACE1("\r\n", 0x80);
#endif

	// Checking if there are changes in report since the method was last called
	for (uint8_t i = 0; i < RPT_GEMEPAD_LEN; i++)
	if (buf[i] != oldPad[i]) {
		match = false;
		break;
	}

	// Calling Game Pad event handler
	if (!match && mouseEvents) {
		mouseEvents->OnGamePadChanged((const ScrollMouseEventData*)buf);

		for (uint8_t i = 0; i < RPT_GEMEPAD_LEN; i++) oldPad[i] = buf[i];
	}

	uint8_t hat = (buf[5] & 0xF);

	// Calling Hat Switch event handler
	if (hat != oldHat && mouseEvents) {
		mouseEvents->OnHatSwitch(hat);
		oldHat = hat;
	}

	uint16_t buttons = (0x0000 | buf[6]);
	buttons <<= 4;
	buttons |= (buf[5] >> 4);
	uint16_t changes = (buttons ^ oldButtons);

	// Calling Button Event Handler for every button changed
	if (changes) {
		for (uint8_t i = 0; i < 0x0C; i++) {
			uint16_t mask = (0x0001 << i);

			if (((mask & changes) > 0) && mouseEvents) {
				if ((buttons & mask) > 0)
				mouseEvents->OnButtonDn(i + 1);
				else
				mouseEvents->OnButtonUp(i + 1);
			}
		}
		oldButtons = buttons;
	}
}

void ScrollMouseEvents::OnGamePadChanged(const ScrollMouseEventData *evt) {
	//Serial.print("X1: ");
	//PrintHex<uint8_t > (evt->X, 0x80);
	//Serial.print("\tY1: ");
	//PrintHex<uint8_t > (evt->Y, 0x80);
	//Serial.print("\tX2: ");
	//PrintHex<uint8_t > (evt->Z1, 0x80);
	//Serial.print("\tY2: ");
	//PrintHex<uint8_t > (evt->Z2, 0x80);
	//Serial.print("\tRz: ");
	//PrintHex<uint8_t > (evt->Rz, 0x80);
	//Serial.println("");
}

void ScrollMouseEvents::OnHatSwitch(uint8_t hat) {
	//Serial.print("Hat Switch: ");
	//PrintHex<uint8_t > (hat, 0x80);
	//Serial.println("");
}

void ScrollMouseEvents::OnButtonUp(uint8_t but_id) {
	//Serial.print("Up: ");
	//Serial.println(but_id, DEC);
}

void ScrollMouseEvents::OnButtonDn(uint8_t but_id) {
	//Serial.print("Dn: ");
	//Serial.println(but_id, DEC);
}