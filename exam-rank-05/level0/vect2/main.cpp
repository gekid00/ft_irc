#include "vect2.hpp"

int main(void)
{
	vect2		v1;
	vect2		v2(1, 2);
	const vect2	v3(v2);

	std::cout << "v1 = " << v1 << std::endl;          // {0, 0}
	std::cout << "v2 = " << v2 << std::endl;          // {1, 2}
	std::cout << "v3 = " << v3 << std::endl;          // {1, 2}

	std::cout << "++v2 = " << ++v2 << std::endl;      // {2, 3}
	std::cout << "v2++ = " << v2++ << std::endl;      // {2, 3}  (post: retourne avant)
	std::cout << "v2   = " << v2   << std::endl;      // {3, 4}

	std::cout << "--v2 = " << --v2 << std::endl;      // {2, 3}
	std::cout << "v2-- = " << v2-- << std::endl;      // {2, 3}
	std::cout << "v2   = " << v2   << std::endl;      // {1, 2}

	std::cout << "v2 += v3 → " << (v2 += v3) << std::endl;  // {2, 4}
	std::cout << "v2 -= v3 → " << (v2 -= v3) << std::endl;  // {1, 2}
	std::cout << "v2 *= 3  → " << (v2 *= 3)  << std::endl;  // {3, 6}

	vect2 v4(2, 2);
	std::cout << "v4 * 2   = " << (v4 * 2)   << std::endl;  // {4, 4}
	std::cout << "3 * v4   = " << (3 * v4)   << std::endl;  // {6, 6}
	std::cout << "-v4      = " << (-v4)       << std::endl;  // {-2, -2}

	std::cout << "v4[0]    = " << v4[0]       << std::endl;  // 2
	std::cout << "v4[1]    = " << v4[1]       << std::endl;  // 2
	std::cout << "v3[0]    = " << v3[0]       << std::endl;  // 1  (const)

	vect2 v5(2, 2);
	std::cout << "(v4 == v5) = " << (v4 == v5) << std::endl; // 1
	std::cout << "(v4 != v2) = " << (v4 != v2) << std::endl; // 1

	// test chaînage
	vect2 a(1, 1), b(2, 2), c(3, 3);
	std::cout << "a+b+c = " << (a + b + c) << std::endl;     // {6, 6}
	// v4 = {2,2} ici, vect2(2,2)*2 = {4,4} → pas égaux
	std::cout << "vect2(2,2)*2 == v4 : " << (vect2(2,2)*2 == v4) << std::endl; // 0
	// pour tester == true :
	std::cout << "vect2(2,2)*2 == vect2(4,4) : " << (vect2(2,2)*2 == vect2(4,4)) << std::endl; // 1

	return (0);
}
