#include <iostream>
#include "FloatList.h"

int main()
{
  FloatList list;
  list.pushBack(1.2);
  list.pushBack(2.9);
  list.pushBack(5.1);
  list.pushBack(23.634);
  list.pushBack(3.14159);
  list.printList();

  list.removeElement(3);
  list.printList();

  list.find(5.1);
  list.popBack();
  list.printList();

  list.clear();
  list.pushBack(69);
  list.pushBack(28);
  list.pushBack(1);
  list.pushBack(20.9);
  list.pushBack(5);
  list.printList();

  list.insertElement(3, 10.0);
  list.printList();

  list.ascOrder();
  list.printList();

  list.desOrder();
  list.printList();

}
