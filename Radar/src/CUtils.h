#ifndef UTILS_H
#define UTILS_H

enum RETVAL {
	OK = 0,
	IO = 1,
	MEM = 2,
	MISC = 3,
	END_ENUM = 4
};

struct Vec3 {
	int x = 0;
	int y = 0;
	int z = 0;
}

struct Vec3f {
	float x = 0.0;
	float y = 0.0;
	float z = 0.0;
}

#endif