#pragma once

class IXfer {
public:
	virtual int open()=0;
	virtual int process()=0;
	virtual void close()=0;
};