#pragma once
#ifndef DATE_HEADER
#define DATE_HEADER

struct date {
	int year;
	unsigned int month;
	unsigned int day;
};

bool operator==(const date&, const date&);
#endif
