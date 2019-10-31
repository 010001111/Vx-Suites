#include "rtltypes.h"

size_t i64tostr_a(signed long long x, char *s)
{
	signed long long	t=x;
	size_t		i, r=1, sign;

	if (x < 0) {
		sign = 1;
		while (t <= -10) {
			t /= 10;
			r++;
		}
	}
	else {
		sign = 0;
		while (t >= 10) {
			t /= 10;
			r++;
		}
	}

	if (s == 0)
		return r + sign;

	if (sign) {
		*s = '-';
		s++;
	}

	for (i = r; i != 0; i--) {
		s[i - 1] = (char)byteabs(x % 10) + '0';
		x /= 10;
	}

	s[r] = (char)0;
	return r + sign;
}

size_t i64tostr_w(signed long long x, wchar_t *s)
{
	signed long long	t=x;
	size_t		i, r=1, sign;

	if (x < 0) {
		sign = 1;
		while (t <= -10) {
			t /= 10;
			r++;
		}
	} else {
		sign = 0;
		while (t >= 10) {
			t /= 10;
			r++;
		}
	}

	if (s == 0)
		return r+sign;
	
	if (sign) {
		*s = '-';
		s++;
	}

	for (i = r; i != 0; i--) {
		s[i-1] = (wchar_t)byteabs(x % 10) + L'0';
		x /= 10;
	}

	s[r] = (wchar_t)0;
	return r+sign;
}
