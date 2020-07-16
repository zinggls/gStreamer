#pragma once

class IDataCallback {
public:
	virtual ~IDataCallback() {}
	virtual void OnData(PUCHAR buf, LONG len) = 0;
};