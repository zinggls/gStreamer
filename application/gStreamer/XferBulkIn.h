#pragma once

#include "IXfer.h"

class CXferBulkIn : public IXfer {
public:
	CXferBulkIn();
	virtual ~CXferBulkIn();

	virtual int open();
	virtual int process();
	virtual void close();
};