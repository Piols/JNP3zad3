#include "clock.h"
#include <cstdlib>

clockTime nextTime(clockTime t) {
	if (t[3] != '9') {
		t[3] = char(int(t[3]) + 1);
		return t;
	}
	t[3] = '0';
	if (t[2] != '5') {
		t[2] = char(int(t[2]) + 1);
		return t;
	}
	t[2] = '0';
	if (t[0] == '2' && t[1] == '3') {
		t[0] = '0';
		t[1] = '0';
		return t;
	}
	if (t[1] != '9') {
		t[1] = char(int(t[1]) + 1);
		return t;
	}
	t[1] = '0';
	t[0] = char(int(t[0]) + 1);
	return t;
}

clockTime randTime() {
	clockTime t = "0000";
	for (int i = rand() % 1440; i > 0; i--) {
		t = nextTime(t);
	}
	return t;
}