#pragma once

struct Node {
public:
  float data;
  Node* next;
  Node();
  Node(float data);
};

class FloatList
{
  
public:
  FloatList();
  ~FloatList();
  void pushBack(float val);
  void printList();
  float removeElement(int index);
  void insertElement(int index, float val);
  float popBack();
  int getSize();
  void clear();
  bool isEmpty();
  int find(float val);
  void ascOrder();
  void desOrder();

private:
  Node* _head;
};

