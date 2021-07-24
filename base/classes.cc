#ifndef CLASSES
#define CLASSES

#include "classes.h"

#include <chrono>

//////////////////////// GLOBAL VARIABLES ////////////////////////


bool ivec3_comparator::operator() (const ivec3& lhs, const ivec3& rhs) const
{
    return lhs.x < rhs.x || lhs.x == rhs.x && (lhs.y < rhs.y || lhs.y == rhs.y && lhs.z < rhs.z);
}

bool vec3_comparator::operator() (const vec3& lhs, const vec3& rhs) const
{
    return lhs.x < rhs.x || lhs.x == rhs.x && (lhs.y < rhs.y || lhs.y == rhs.y && lhs.z < rhs.z);
}

size_t ivec3_hash::operator() (const ivec3& pos) const {
	size_t sum = 221523657787344691L*pos.x + 175311628825151297L*pos.y + 755312049640874567L*pos.z;
	 // 13680553*pos.x + 47563643*pos.y + 84148333*pos.z;
	sum = (sum ^ (sum >> 26)) * 746525087535803393;
  return sum ^ (sum >> 32);
  // std::hash<int> ihash;
	// return ihash(pos.x) ^ ihash(pos.y) ^ ihash(pos.z);
}

size_t ivec4_hash::operator() (const ivec4& pos) const {
  std::hash<int> ihash;
	return ihash(pos.x) ^ ihash(pos.y) ^ ihash(pos.z) ^ ihash(pos.w);
}

ostream& operator<<(ostream& out, ivec3 pos) {
	out << pos.x << ',' << pos.y << ',' << pos.z;
	return out;
}

ostream& operator<<(ostream& out, vec3 pos) {
	out << pos.x << ',' << pos.y << ',' << pos.z;
	return out;
}

ostream& operator<<(ostream& out, quat rot) {
	out << rot.w << ',' << rot.x << ',' << rot.y << ',' << rot.z;
	return out;
}

double getTime() {
  static std::chrono::time_point<std::chrono::steady_clock> startTime = std::chrono::steady_clock::now();
  return std::chrono::duration<double>(std::chrono::steady_clock::now() - startTime).count();
}

#endif