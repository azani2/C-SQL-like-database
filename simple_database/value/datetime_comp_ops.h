#pragma once
#ifndef DATETIME_COMP_OPS_HEADER
#define DATETIME_COMP_OPS_HEADER
#include "datetime.h"

bool operator<(const datetime& left, const datetime& right);
bool operator>(const datetime& left, const datetime& right);

#endif 
