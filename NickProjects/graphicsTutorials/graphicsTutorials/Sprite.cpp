#include "Sprite.h"
#include "Vertex.h"

#include <cstddef>

Sprite::Sprite() {
    _vboID = 0;
}
Sprite::~Sprite() {
    if (_vboID != 0) {
        glDeleteBuffers(1, &_vboID);
    }
}

void Sprite::init(float x, float y, float width, float height) {
    _x = x;
    _y = y;
    _width = width;
    _height = height;

    if (_vboID == 0) {
        glGenBuffers(1, &_vboID);
    }

    Vertex vertexData[6];

    //First Triangle
    vertexData[0].position.x = x + width;
    vertexData[0].position.y = y + width;

    vertexData[1].position.x = x;
    vertexData[1].position.y = y + height;

    vertexData[2].position.x = x;
    vertexData[2].position.y = y;

    //Second Triangle
    vertexData[3].position.x = x;
    vertexData[3].position.y = y;

    vertexData[4].position.x = x + width;
    vertexData[4].position.y = y;

    vertexData[5].position.x = x + width;
    vertexData[5].position.y = y + height;

    for (int i = 0; i < 6; i++) {
        vertexData[i].color.r = 255;
        vertexData[i].color.g = 0;
        vertexData[i].color.b = 255;
        vertexData[i].color.a = 255;
    }

    vertexData[1].color.r = 0;
    vertexData[1].color.g = 0;
    vertexData[1].color.b = 255;
    vertexData[1].color.a = 255;

    vertexData[4].color.r = 0;
    vertexData[4].color.g = 255;
    vertexData[4].color.b = 0;
    vertexData[4].color.a = 255;

    glBindBuffer(GL_ARRAY_BUFFER, _vboID);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), vertexData, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

}

void Sprite::draw() {

    //bind the buffer object
    glBindBuffer(GL_ARRAY_BUFFER, _vboID);

    //Tell openGl that we want to use the first attribute array.
    //We only need one array right now since we are only using this position.
    glEnableVertexAttribArray(0);

    //This is the position attribute pointer
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    //This is the color attribute pointer
    glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (void*)offsetof(Vertex, color));


    //Draw the 6 vertices to the screen
    glDrawArrays(GL_TRIANGLES, 0, 6);

    //Disable the vertex attrib array. This is not optional.
    glDisableVertexAttribArray(0);

    //Unbind the VBO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}