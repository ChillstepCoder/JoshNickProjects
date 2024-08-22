#include "FloatList.h"
#include <iostream>
#include <string>
#include <assert.h>


FloatList::FloatList() { //Constructor
	_head = nullptr; 
}
FloatList::~FloatList() { //Destructor
	Node* nextNode = _head;
	Node* newNode = nullptr;
	while (nextNode != nullptr) {
		newNode = nextNode;
		nextNode = newNode->next;
		delete newNode;
	}
}


void FloatList::pushBack(float val) {

	Node* nextNode = new Node(val);

	if (_head == nullptr) { //Sets first value in list if it is empty.
		_head = nextNode;
		return;
	}

	Node* anotherNode = _head;

	while (anotherNode->next != nullptr) { //Moves to the end of the list.
		anotherNode = anotherNode->next;
	}
	anotherNode->next = nextNode; //Sets the end of the list to the new value.
}

float FloatList::popBack() {
	Node* removeNode = _head;
	Node* previousNode = nullptr;
	float value = 0.0f;
	int length = getSize();
	int index = length - 1;

	assert(!isEmpty());  //Checks if the list is empty
		

	if (removeNode->next == nullptr) { //Used when there is only 1 node in the list.
		value = removeNode->data;
		delete removeNode;
		_head = nullptr;
		return value;
	}

	while (removeNode->next != nullptr) { //Finds the end of the chain of nodes
		previousNode = removeNode;
		removeNode = removeNode->next;
	}
	if (removeNode != nullptr) { //Removes the last node in the chain.
		value = removeNode->data;
		previousNode->next = nullptr;
	}
	delete removeNode;
	return value;

}


int FloatList::getSize() {

	int length = 0;
	Node* anotherNode = _head;

	if (_head == nullptr) { //If list is empty, return.
		return 0;
	}

	while (anotherNode != nullptr) //Goes to end of list and counts the amount of nodes.
	{
		anotherNode = anotherNode->next;
		length++;
	}


	return length;
}

void FloatList::printList() {
	Node* anotherNode = _head;

	if (isEmpty()) { // Checks if list is empty
		return;
	}

	while (anotherNode != nullptr) { //Goes through list printing each node.
		std::cout << anotherNode->data;
		if (anotherNode->next != nullptr) { //Checks if you are not at the end of the list to print commas.
			std::cout << ", ";
		}
		anotherNode = anotherNode->next;
	}
	std::cout << std::endl;
}

float FloatList::removeElement(int index) {
	Node* removeNode = _head;
	Node* previousNode = nullptr;

	float value = 0.0f;
	int length = getSize();

	assert(!isEmpty());  //Checks if the list is empty

	if (index >= length || index < 0) { //Checks if index is within range.
		std::cout << "Out of range!!!\n The range was " << (length - 1) << ".\n";
		return 0.0f;
	}
	for (int i = 0; i < index; i++) { //Moves both pointers through the list until they reach the index.
		previousNode = removeNode;
		removeNode = removeNode->next;
	}

	value = removeNode->data;

	if (previousNode != nullptr) { //Points previousnode to the node after the one that gets deleted.
		previousNode->next = removeNode->next;
	}
	
	delete removeNode; //Deletes the node.

	return value;
}


void FloatList::insertElement(int index, float val) {
	Node* newNode = _head;
	Node* previousNode = nullptr;
	Node* insertedNode = new Node(val);

	int length = getSize();

	if (isEmpty()) { //Checks if the list is empty
		return ;
	}
	if (index >= length + 1|| index < 0) { //Checks if index is within range.
		std::cout << "Out of range!!!\nThe range was " << (length) << ".\n";
		return;
	}
	for (int i = 0; i < index; i++) { //Moves both pointers through the list until they reach the index.
		previousNode = newNode;
		newNode = newNode->next;
	}
	if (previousNode != nullptr) //Inserts the node in the chain
	{
		previousNode->next = insertedNode;
		insertedNode->next = newNode;
	}
}

void FloatList::clear() { //Clears the entire list.
	while (isEmpty() == false) {
		popBack();
	}
}
bool FloatList::isEmpty() {

	if (_head == nullptr) { //Checks if the list is empty
		std::cout << "The list is empty.\n";
		return true;

	}
	return false;
}
int FloatList::find(float val) {
	Node* checkNode = _head;

	int index = 0;

	if (isEmpty()) {
		return -1; //Checks if the list is empty
	}
	
	while (checkNode != nullptr) { //Finds in the chain of nodes where the value is.
		if (checkNode->data == val) {
			std::cout << "The value " << val << " is at " << index << ".\n";
			return index;
		}
		checkNode = checkNode->next;
		index++;
	}
	std::cout << "The value " << val << " has not been found in the list.\n";
	return -1;
}

Node::Node() //Node constructor
{
	data = 0;
	next = nullptr;
}
Node::Node(float val) //Node constructor
{
	data = val;
	next = nullptr;
}