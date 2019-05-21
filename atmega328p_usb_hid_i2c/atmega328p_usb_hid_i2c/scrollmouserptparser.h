#if !defined(__HIDJOYSTICKRPTPARSER_H__)
#define __HIDJOYSTICKRPTPARSER_H__

#include <usbhid.h>

struct ScrollMouseEventData {
	uint8_t B, X, Y, Z;
};

class ScrollMouseEvents {
	public:
		virtual void OnGamePadChanged(const ScrollMouseEventData *evt);
		virtual void OnHatSwitch(uint8_t hat);
		virtual void OnButtonUp(uint8_t but_id);
		virtual void OnButtonDn(uint8_t but_id);
};

#define RPT_GEMEPAD_LEN		5

class ScrollMouseReportParser : public HIDReportParser {
	ScrollMouseEvents *mouseEvents;

	uint8_t oldPad[RPT_GEMEPAD_LEN];
	uint8_t oldHat;
	uint16_t oldButtons;

	public:
		ScrollMouseReportParser(ScrollMouseEvents *evt);

		virtual void Parse(USBHID *hid, bool is_rpt_id, uint8_t len, uint8_t *buf);
};

#endif // __HIDJOYSTICKRPTPARSER_H__