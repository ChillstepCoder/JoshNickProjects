#include "FloatList.h"
#include <iostream>


int main() {
	FloatList floatList;

	floatList.isEmpty();

	floatList.pushBack(1.2);
	floatList.pushBack(2.2);
	floatList.pushBack(8.3);
	floatList.pushBack(6.5);
	floatList.pushBack(21.4);

	floatList.printList();

	std::cout << floatList.getSize() << std::endl;

	floatList.removeElement(1);
	floatList.printList();


	floatList.popBack();
	floatList.printList();

	floatList.clear();

	floatList.pushBack(1.5);
	floatList.pushBack(2.9);
	floatList.pushBack(7.56);
	floatList.pushBack(3.4);
	floatList.printList();

	floatList.find(2.9);

	floatList.insertElement(4, 32.1);
	floatList.printList();

}