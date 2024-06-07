//======= Copyright (c) 2024, FIT VUT Brno, All rights reserved. ============//
//
// Purpose:     Red-Black Tree - public interface tests
//
// $NoKeywords: $ivs_project_1 $black_box_tests.cpp
// $Author:     FILIP NOVAK <xnovakf00@stud.fit.vutbr.cz>
// $Date:       $2024-03-04
//============================================================================//
/**
 * @file black_box_tests.cpp
 * @author FILIP NOVAK
 * 
 * @brief Implementace testu binarniho stromu.
 */

#include <vector>

#include "gtest/gtest.h"

#include "red_black_tree.h"

#include "limits.h"

//============================================================================//
// ** ZDE DOPLNTE TESTY **
//
// Zde doplnte testy Red-Black Tree, testujte nasledujici:
// 1. Verejne rozhrani stromu
//    - InsertNode/DeleteNode a FindNode
//    - Chovani techto metod testuje pro prazdny i neprazdny strom.
// 2. Axiomy (tedy vzdy platne vlastnosti) Red-Black Tree:
//    - Vsechny listove uzly stromu jsou *VZDY* cerne.
//    - Kazdy cerveny uz el muze mit *POUZE* cerne potomky.
//    - Vsechny cesty od kazdeho listoveho uzlu ke koreni stromu obsahuji
//      *STEJNY* pocet cernych uzlu.
//============================================================================//
using namespace ::testing;


class NonEmptyTree : public Test{
    protected:
        BinaryTree tree;
    void SetUp() 
    {
        int keys[] = {1, 2, 3, 9, 40, 50, -10, -99, 990, 7985, 2, 98566};
        for (auto key : keys)
        {
            tree.InsertNode(key);
        }          
    }

};


class EmptyTree : public Test{
    protected:
        BinaryTree tree;
};

class TreeAxioms : public Test{
    protected:
        BinaryTree tree;
    void SetUp() 
    {
        int keys[] = {5, 9, 99, -154, -589864, 15, 47};
        for (auto key : keys)
        {
            tree.InsertNode(key);
        }          
    }

};


TEST_F(EmptyTree, InsertNode)
{
    auto pair = tree.InsertNode(5);
    ASSERT_TRUE(pair.first);
    ASSERT_EQ(pair.second->key, 5);

    auto pair2 = tree.InsertNode(INT_MAX);
    ASSERT_TRUE(pair2.first);
    ASSERT_EQ(pair2.second->key, INT_MAX);

    auto pair3 = tree.InsertNode(5);
    ASSERT_FALSE(pair3.first);
    ASSERT_EQ(pair3.second->key, 5);

    auto pair4 = tree.InsertNode(INT_MAX);
    ASSERT_FALSE(pair4.first);
    ASSERT_EQ(pair4.second->key, INT_MAX);

    auto pair5 = tree.InsertNode(-20);
    ASSERT_TRUE(pair5.first);
    ASSERT_EQ(pair5.second->key, -20);
    
    auto pair6 = tree.InsertNode(INT_MIN);
    ASSERT_TRUE(pair6.first);
    ASSERT_EQ(pair6.second->key, INT_MIN);

    auto pair7 = tree.InsertNode(-20);
    ASSERT_FALSE(pair7.first);
    ASSERT_EQ(pair7.second->key, -20);

    auto pair8 = tree.InsertNode(INT_MIN);
    ASSERT_FALSE(pair8.first);
    ASSERT_EQ(pair8.second->key, INT_MIN);
}

TEST_F(EmptyTree, FindNode)
{
    tree.InsertNode(5);
    tree.InsertNode(6);
    auto return1 = tree.FindNode(5);
    EXPECT_TRUE(return1 != NULL);
    EXPECT_EQ(return1->key, 5);

    auto return2 = tree.FindNode(7);
    EXPECT_TRUE(return2 == NULL);
}

TEST_F(EmptyTree, DeleteNode)
{
    auto return1 = tree.DeleteNode(5);
    EXPECT_FALSE(return1);

    tree.InsertNode(5);
    tree.InsertNode(6);
    auto return2 = tree.DeleteNode(5);
    EXPECT_TRUE(return2);
    EXPECT_TRUE(tree.FindNode(5) == NULL);
    EXPECT_TRUE(tree.FindNode(6) != NULL);
}


TEST_F(NonEmptyTree, InsertNode)
{
    auto pair = tree.InsertNode(5);
    ASSERT_TRUE(pair.first);
    ASSERT_EQ(pair.second->key, 5);

    auto pair2 = tree.InsertNode(INT_MAX);
    ASSERT_TRUE(pair2.first);
    ASSERT_EQ(pair2.second->key, INT_MAX);

    auto pair3 = tree.InsertNode(40);
    ASSERT_FALSE(pair3.first);
    ASSERT_EQ(pair3.second->key, 40);

    auto pair4 = tree.InsertNode(INT_MAX);
    ASSERT_FALSE(pair4.first);
    ASSERT_EQ(pair4.second->key, INT_MAX);

    auto pair5 = tree.InsertNode(-99);
    ASSERT_FALSE(pair5.first);
    ASSERT_EQ(pair5.second->key, -99);
    
}

TEST_F(NonEmptyTree, FindNode)
{
    auto return1 = tree.FindNode(990);
    EXPECT_TRUE(return1 != NULL);
    EXPECT_EQ(return1->key, 990);

    auto return2 = tree.FindNode(7);
    EXPECT_TRUE(return2 == NULL);
}

TEST_F(NonEmptyTree, DeleteNode)
{
    auto return1 = tree.DeleteNode(9563);
    EXPECT_FALSE(return1);

    auto return2 = tree.DeleteNode(-10);
    EXPECT_TRUE(return2);
    EXPECT_TRUE(tree.FindNode(-10) == NULL);
}


TEST_F(TreeAxioms, Axiom1)
{
    std::vector<Node_t *> leafNodes;
    tree.GetLeafNodes(leafNodes);

    for(auto leaf : leafNodes)
    {
        EXPECT_EQ(leaf->color, BLACK);
    }
}

TEST_F(TreeAxioms, Axiom2)
{
    std::vector<Node_t *> leafNodes;
    tree.GetNonLeafNodes(leafNodes);

    for(auto leaf : leafNodes)
    {
        if(leaf->color == RED)
        {
            EXPECT_EQ(leaf->pLeft->color, BLACK);
            EXPECT_EQ(leaf->pRight->color, BLACK);
        }
        
    }
}

TEST_F(TreeAxioms, Axiom3)
{
    std::vector<Node_t *> leafNodes;
    tree.GetLeafNodes(leafNodes);
    auto root = tree.GetRoot();
    int blackCount = -1;
    for(auto leaf : leafNodes)
    {
        int blackCountCurrent = 0;
        auto leafParent = leaf->pParent;
        while(leafParent != root)
        {
            if(leafParent->color == BLACK)
            {
                blackCountCurrent++;
            }
            leafParent = leafParent->pParent;
        }
        if(blackCount == -1)
        {
            blackCount = blackCountCurrent;
        }
        ASSERT_EQ(blackCount, blackCountCurrent);
    }
}
/*** Konec souboru black_box_tests.cpp ***/
