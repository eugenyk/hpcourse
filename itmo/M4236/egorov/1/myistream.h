
#pragma once

class myistream {
public:
	virtual bool operator >> (int& buffer) = 0;
};
