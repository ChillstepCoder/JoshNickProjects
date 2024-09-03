#include <iostream>
#include "FloatList.h"

Node::Node() : data(0), next(nullptr) {}

Node::Node(float data) : data(data), next(nullptr) {}

FloatList::FloatList() : _head(nullptr) {}

FloatList::~FloatList() {
  Node* currentNode = _head;
  while (currentNode != nullptr) {
    Node* nextNode = currentNode->next;
    delete currentNode;
    currentNode = nextNode;
  }
}

void FloatList::pushBack(float val) {
  Node* nNode = new Node(val);

  if (_head == nullptr) {
    _head = nNode;
    return;
  }

  Node* curNode = _head;
  while (curNode->next != nullptr) {
    curNode = curNode->next;
  }

  curNode->next = nNode;
}

void FloatList::printList() {
  Node* curNode = _head;

  if (isEmpty()) {
    std::cout << "List Contains No Data." << std::endl;
    return;
  }

  while (curNode != nullptr) {
    std::cout << curNode->data;
      if (curNode->next != nullptr) {
        std::cout << " , ";
      }
    curNode = curNode->next;
  }
  std::cout << std::endl;
}

float FloatList::removeElement(int index) {
  Node* curNode = _head;
  Node* prevNode = nullptr;
  int length = getSize();
  float removedValue = 0.0f;

  if (isEmpty()) {
    std::cout << "List Contains No Data." << std::endl;
    return 0.0f;
  }

  if (index >= length || index < 0) {
    std::cout << "Index is out of range (" << (length-1) << ")" << std::endl;
    return 0.0f;
  }

  // when theres only 1 data entry
  if (curNode->next == nullptr) {
    removedValue = curNode->data;
    delete curNode;
    _head = nullptr;
    return removedValue;
  }

  // find the last node
  for (int i = 0; i < index; i++) {
    prevNode = curNode;
    curNode = curNode->next;
  }

  if (prevNode != nullptr) {
    removedValue = curNode->data;
    prevNode->next = curNode->next;
  }

  delete curNode;
  return removedValue;
}

void FloatList::insertElement(int index, float val) {
  Node* curNode = _head;
  Node* prevNode = nullptr;
  Node* insertedNode = new Node(val);

  if (isEmpty()) {
    std::cout << "List Contains No Data." << std::endl;
    return;
  }
     
  int length = getSize();

  if (index >= length + 1 || index < 0) {
    std::cout << "Index is out of range (" << (length) << ")" << std::endl;
    return;
  }

  // find node to insert
  for (int i = 0; i < index; i++) {
    prevNode = curNode;
    curNode = curNode->next;
  }

  if (prevNode != nullptr) {
    prevNode->next = insertedNode;
    insertedNode->next = curNode;
  }
}

float FloatList::popBack() {
  Node* curNode = _head;
  Node* prevNode = nullptr;
  int index = getSize() - 1;
  float removedValue = 0.0f;

  if (isEmpty()) {
    std::cout << "List Contains No Data." << std::endl;
    return 0.0f;
  }

  // when theres only 1 data entry
  if (curNode->next == nullptr) {
    removedValue = curNode->data;
    delete curNode;
    _head = nullptr;
    return removedValue;
  }

  // find the last node
  while (curNode->next != nullptr) {
    prevNode = curNode;
    curNode = curNode->next;
  }

  if (prevNode != nullptr) {
    removedValue = curNode->data;
    prevNode->next = nullptr;
  }

  delete curNode;

  return removedValue;
}


int FloatList::getSize() {
  Node* curNode = _head;
  int length = 0;

  //find list length
  while (curNode != nullptr) {
    curNode = curNode->next;
    length++;
  }

  return length;
}

void FloatList::clear() {
  while (!isEmpty()) {
    popBack();
  }
}

bool FloatList::isEmpty() {
  if (_head == nullptr) {
    return true;
  }
  return false;
}

int FloatList::find(float val) {
  Node* curNode = _head;
  int index = 0;

  if (_head == nullptr) {
    std::cout << "List Contains No Data." << std::endl;
    return 0;
  }
  //find list length
  while (curNode != nullptr) {
    if (curNode->data == val) {
      std::cout << "Value (" << val << ") found at index (" << index << ")" << std::endl;
      return index;
    }
    curNode = curNode->next;
    index++;
  }

  std::cout << "Value not found!" << std::endl;
  return -1;
}

void FloatList::ascOrder() {
  Node* curNode;
  Node* lastNode = nullptr;
  bool swap;

  if (isEmpty()) {
    std::cout << "List Contains No Data." << std::endl;
    return;
  }

  if (getSize() == 1) {
    std::cout << "Not Enough Data to Sort." << std::endl;
    return;
  }

  do {
    swap = false;
    curNode = _head;

    while (curNode->next != nullptr) {
      //check if needs swap
      if (curNode->data > curNode->next->data) {
        // Swap data between neighboring nodes
        float temp = curNode->data;
        curNode->data = curNode->next->data;
        curNode->next->data = temp;
        swap = true;
      }
      curNode = curNode->next;
    }
    lastNode = curNode;
  } while (swap); //repeat until no more swapping takes place.
}

void FloatList::desOrder() {
  Node* curNode;
  Node* lastNode = nullptr;
  bool swap;

  if (isEmpty()) {
    std::cout << "List Contains No Data." << std::endl;
    return;
  }

  if (getSize() == 1) {
    std::cout << "Not Enough Data to Sort." << std::endl;
    return;
  }

  do {
    swap = false;
    curNode = _head;

    while (curNode->next != nullptr) {
      //check if needs swap
      if (curNode->data < curNode->next->data) {
        // Swap data between neighboring nodes
        float temp = curNode->data;
        curNode->data = curNode->next->data;
        curNode->next->data = temp;
        swap = true;
      }
      curNode = curNode->next;
    }
    lastNode = curNode;
  } while (swap);
}
