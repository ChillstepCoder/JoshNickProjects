#pragma once

struct Node
{
	float data;
	Node* next = nullptr;
	Node();
	Node(float data);
};

class FloatList
{
public:
	FloatList();
	~FloatList();

	void pushBack(float val);
	float popBack();
	int getSize();
	void printList();
	float removeElement(int index);
	void insertElement(int index, float val);
	void clear();
	bool isEmpty();
	int find(float val);

private:
	Node* _head;
};


