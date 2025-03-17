#ifndef CUTILS_H_
#define CUTILS_H_

struct message {
	int TXID = 0;
	int RXID = 0;
	std::string data = "";
};

enum RETVAL {
	OK = 0,
	TXFAIL = 1,
	MISC = 2,
	END = 3
};

//add message queue here

#endif
