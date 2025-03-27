#ifndef UTILS_H
#define UTILS_H

enum RETVAL {
	OK = 0, //success
	IO = 1, //file error
	MEM = 2, //memory error
	MISC = 3, //other errors
	END_ENUM = 4
};

struct Vec3 {
	int x = 0;
	int y = 0;
	int z = 0;
};

struct Vec3f {
	float x = 0.0;
	float y = 0.0;
	float z = 0.0;
};

#endif
