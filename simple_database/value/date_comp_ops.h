#pragma once
#ifndef DATE_COMP_OPS_HEADER
#define DATE_COMP_OPS_HEADER
#include "date.h"

bool operator<(const date& left, const date& right);
bool operator>(const date& left, const date& right);

#endif 
