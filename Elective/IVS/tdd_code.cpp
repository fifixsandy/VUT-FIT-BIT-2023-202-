//======= Copyright (c) 2024, FIT VUT Brno, All rights reserved. ============//
//
// Purpose:     Test Driven Development - graph
//
// $NoKeywords: $ivs_project_1 $tdd_code.cpp
// $Author:     FILIP NOVAK <xnovakf00@stud.fit.vutbr.cz>
// $Date:       $2024-03-04
//============================================================================//
/**
 * @file tdd_code.cpp
 * @author Martin Dočekal
 * @author Karel Ondřej
 *
 * @brief Implementace metod tridy reprezentujici graf.
 */

#include "tdd_code.h"


Graph::Graph(){}

Graph::~Graph(){
    clear();
}

std::vector<Node*> Graph::nodes() {
    return graphNodes;
}

std::vector<Edge> Graph::edges() const{
    return graphEdges;
}

Node* Graph::addNode(size_t nodeId) {
    for(size_t nodeElement = 0; nodeElement < nodeCount(); nodeElement++) // cycle through the graph until last node
    {
        if(graphNodes[nodeElement]->id == nodeId) // if there is alredy node with that id, it will return nullptr
        {
            return nullptr;
        }
    }
    Node* addedNode = new Node; // creates new node
    if(addedNode != nullptr)
    {
        addedNode->id = nodeId; // adds id to the new node
        addedNode->color = 0; // sets color to default 0
        graphNodes.push_back(addedNode); // adds the new node into graph;
    }


    return addedNode; // returns the pointer to the new node
}

bool Graph::addEdge(const Edge& edge){
    if(edge.a == edge.b) //checks smycky
    {
        return false;
    }

    if(containsEdge(edge)) // checks if this edge already exists
    {
        return false;
    }

    Node *aNode = getNode(edge.a); // sets aNode to point to same plase as a node with id edge.a
    Node *bNode = getNode(edge.b); // sets bNode to point to same plase as a node with id edge.b

    if(aNode == nullptr) // if there is no such node, it will create one
    {
        aNode = addNode(edge.a);
    }
    if(bNode == nullptr)
    {
        bNode = addNode(edge.b);
    }

    aNode->neighbourNodes.push_back(bNode); // adds neighbour to nodes 
    bNode->neighbourNodes.push_back(aNode);
        
    Edge addedEdge = Edge(edge.a, edge.b); // creates new Edge and adds it into vector
    graphEdges.push_back(addedEdge);
    return true;
}

void Graph::addMultipleEdges(const std::vector<Edge>& edges) {
    for(size_t edgeElement = 0; edgeElement < edges.size(); edgeElement++)
    {
        addEdge(edges[edgeElement]);
    }
}

Node* Graph::getNode(size_t nodeId){
    for(size_t nodeElement = 0; nodeElement < nodeCount(); nodeElement++)
    {
        if(graphNodes[nodeElement]->id == nodeId)
        {
            return graphNodes[nodeElement];
        }
    }
    return nullptr;
}

bool Graph::containsEdge(const Edge& edge) const{
    for(size_t edgeElement = 0; edgeElement < edgeCount(); edgeElement++)
    {
        if(graphEdges[edgeElement] == edge)
        {
            return true;
        }
    }
    return false;
}

void Graph::removeNode(size_t nodeId){
    int nodeToRemove = -1;
    for(size_t nodeElement = 0; nodeElement < nodeCount(); nodeElement++)
    {
        if(graphNodes[nodeElement]->id == nodeId)
        {
            nodeToRemove = nodeElement;
        }
    }
    
    if(nodeToRemove == -1)
    {
        throw std::out_of_range("Nonexistent node with this ID");
        return;
    }


    size_t numOfEdges = edgeCount();
    size_t edgeElement = 0;

    while(edgeElement < numOfEdges)
    {
        if(graphEdges[edgeElement].a == nodeId || graphEdges[edgeElement].b == nodeId)
        {
            removeEdge(graphEdges[edgeElement]);
            edgeElement = 0;
            numOfEdges--;
        }
        else
        {
            edgeElement++;
        }

    }
    delete graphNodes[nodeToRemove];
    graphNodes.erase(graphNodes.begin() + nodeToRemove);
}


void Graph::removeEdge(const Edge& edge){
    auto edgeToRemove = std::find(graphEdges.begin(), graphEdges.end(), edge); // returns position of wanted edge
    if(edgeToRemove == graphEdges.end()) // if edge is not there, it throws exception
    {
        throw std::out_of_range("Edge does not exist");
        return;
    }

    Node *aNode = getNode(edge.a); // pointer to node that matches id 
    size_t aNumNeigh = aNode->neighbourNodes.size(); // number of neighbours
    size_t aNeighDelPosition = -1; // position of neighbour to delete
    Node *bNode = getNode(edge.b);
    size_t bNumNeigh = bNode->neighbourNodes.size();
    size_t bNeighDelPosition = -1;

    for(size_t neighbourElement = 0; neighbourElement < aNumNeigh; neighbourElement++) // goes through all neighbours
    {
        if(aNode->neighbourNodes[neighbourElement]->id == edge.b) // if neighbour id matches the second end of edge
        {
            aNeighDelPosition = neighbourElement; // sets position
        }
    }
    
    for(size_t neighbourElement = 0; neighbourElement < bNumNeigh; neighbourElement++)
    {
        if(bNode->neighbourNodes[neighbourElement]->id == edge.a)
        {
            bNeighDelPosition = neighbourElement;
        }
    }

    aNode->neighbourNodes.erase(aNode->neighbourNodes.begin() + aNeighDelPosition); // deletes the neighbour
    bNode->neighbourNodes.erase(bNode->neighbourNodes.begin() + bNeighDelPosition);

    graphEdges.erase(edgeToRemove); // erases the edge from graphEdges
}


size_t Graph::nodeCount() const{
    return graphNodes.size();
}

size_t Graph::edgeCount() const{
    return graphEdges.size();
}

size_t Graph::nodeDegree(size_t nodeId) const{
    for(size_t nodeElement = 0; nodeElement < nodeCount(); nodeElement++)
    {
        if(graphNodes[nodeElement]->id == nodeId)
        {
            return graphNodes[nodeElement]->neighbourNodes.size();
        }
    }
    throw std::out_of_range("Node with this id does not exist");
}

size_t Graph::graphDegree() const{
    size_t maxDegree = 0;
    for(size_t nodeElement = 0; nodeElement < nodeCount(); nodeElement++)
    {
        size_t currentId = graphNodes[nodeElement]->id;
        if(nodeDegree(currentId) > maxDegree)
        {
            maxDegree = nodeDegree(currentId);
        }
    }
    return maxDegree;
}

void Graph::coloring(){
    for(size_t nodeElement = 0; nodeElement < nodeCount(); nodeElement++)
    {
        Node *currentNode = graphNodes[nodeElement];
        std::vector<size_t> possibleColors;
        for(size_t color = 1; color <= graphDegree() + 1; color++)
        {
            possibleColors.push_back(color);
        }

        for(size_t neighbourElement = 0; neighbourElement < currentNode->neighbourNodes.size(); neighbourElement++)
        {
            possibleColors.erase(std::remove(possibleColors.begin(), possibleColors.end(), currentNode->neighbourNodes[neighbourElement]->color), possibleColors.end());
        }
        currentNode->color = possibleColors[0];
    }
}

void Graph::clear() {
    for(size_t node = 0; node < nodeCount(); node++)
    {
        delete graphNodes[node];
        graphNodes[node] = nullptr;
    }
    graphEdges.clear();
    graphNodes.clear();
}

/*** Konec souboru tdd_code.cpp ***/
