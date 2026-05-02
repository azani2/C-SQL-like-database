#pragma once
#ifndef DATETIME_HEADER
#define DATETIME_HEADER
#include "date.h"

struct datetime {
	date dt;
	unsigned int hr;
	unsigned int min;
	unsigned int sec;
};

bool operator==(const datetime&, const datetime&);
#endif