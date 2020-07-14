#pragma once

#include "IXfer.h"

class CXferBulkOut : public IXfer {
public:
	CXferBulkOut();
	virtual ~CXferBulkOut();

	virtual int open();
	virtual int process();
	virtual void close();
};