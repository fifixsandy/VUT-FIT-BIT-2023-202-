//======= Copyright (c) 2024, FIT VUT Brno, All rights reserved. ============//
//
// Purpose:     White Box - test suite
//
// $NoKeywords: $ivs_project_1 $white_box_tests.cpp
// $Author:     FILIP NOVAK <xnovakf00@stud.fit.vutbr.cz>
// $Date:       $2024-03-04
//============================================================================//
/**
 * @file white_box_tests.cpp
 * @author FILIP NOVAK
 * 
 * @brief Implementace testu hasovaci tabulky.
 */

#include <vector>

#include "gtest/gtest.h"

#include "white_box_code.h"

//============================================================================//
// ** ZDE DOPLNTE TESTY **
//
// Zde doplnte testy hasovaci tabulky, testujte nasledujici:
// 1. Verejne rozhrani hasovaci tabulky
//     - Vsechny funkce z white_box_code.h
//     - Chovani techto metod testuje pro prazdnou i neprazdnou tabulku.
// 2. Chovani tabulky v hranicnich pripadech
//     - Otestujte chovani pri kolizich ruznych klicu se stejnym hashem 
//     - Otestujte chovani pri kolizich hashu namapovane na stejne misto v 
//       indexu
//============================================================================//
using namespace ::testing;

class EmptyMap : public Test
{
    protected:
        hash_map_t* map;
    virtual void SetUp()
    {
        map = hash_map_ctor();
    }
    virtual void TearDown()
    {
        hash_map_dtor(map);
    }
};

class NonEmptyMap : public Test
{
    protected:
        hash_map_t* map;
    virtual void SetUp()
    {
        map = hash_map_ctor();
        hash_map_put(map, "Potatoes", 1);
        hash_map_put(map, "boilem", 2);
        hash_map_put(map, "mashem", 3);
    }
    virtual void TearDown()
    {
        hash_map_dtor(map);
    }
};

class Collisions : public Test
{
    protected:
        hash_map_t* map;
    virtual void SetUp()
    {
        map = hash_map_ctor();
        hash_map_put(map, "filip", 1);
        hash_map_put(map, "sofia", 2);
    }
    virtual void TearDown()
    {
        hash_map_dtor(map);
    }
};


/*  TESTS ON NONEMPTY MAP */

TEST_F(EmptyMap, clear)
{
    hash_map_clear(map);
    EXPECT_TRUE(map->first == NULL);
    EXPECT_TRUE(map->last == NULL);
    for (size_t i = 0; i < map->allocated; ++i)
    {
        EXPECT_TRUE(map->index[i] == NULL);
    }
    EXPECT_EQ(0, map->used);
}


TEST_F(EmptyMap, reserve)
{
    auto a = hash_map_reserve(map,8);
    EXPECT_EQ(OK, a);
    EXPECT_EQ(map->allocated, 8);
    
    auto b = hash_map_reserve(map,-1);
    EXPECT_EQ(MEMORY_ERROR, b);
    EXPECT_EQ(map->allocated, 8);

    auto c = hash_map_reserve(map,0);
    EXPECT_EQ(OK, c);
    EXPECT_EQ(map->allocated, 0);

    auto d = hash_map_reserve(map, 16);
    EXPECT_EQ(OK, d);
    EXPECT_EQ(map->allocated, 16);
    
}

TEST_F(EmptyMap, map_size)
{
    auto size = hash_map_size(map);
    EXPECT_EQ(0, size);
}

TEST_F(EmptyMap, map_capacity)
{
    auto capacity1 = hash_map_capacity(map);
    EXPECT_EQ(8, capacity1);

    hash_map_reserve(map, 16);
    auto capacity2 = hash_map_capacity(map);
    EXPECT_EQ(16, capacity2);
}

TEST_F(EmptyMap, contains)
{
    auto contains1 = hash_map_contains(map, "never");
    EXPECT_FALSE(contains1);
    auto contains2 = hash_map_contains(map, "gonna");
    EXPECT_FALSE(contains2);
    auto contains3 = hash_map_contains(map, "gi*ve");
    EXPECT_FALSE(contains3);
    auto contains4 = hash_map_contains(map, "yo_u");
    EXPECT_FALSE(contains4);
    auto contains5 = hash_map_contains(map, "up1");
    EXPECT_FALSE(contains5);
}

TEST_F(EmptyMap, map_put)
{
    auto returnValue = hash_map_put(map, "Never", 5);
    EXPECT_EQ(OK, returnValue);
    EXPECT_EQ(map->first->value, 5);
    EXPECT_EQ(strcmp("Never", map->first->key),0);
    EXPECT_EQ(map->last->value, 5);
    EXPECT_EQ(strcmp("Never", map->last->key),0);
}

TEST_F(EmptyMap, map_get)
{
    int value = 0;
    auto get1 = hash_map_get(map, "never", &value);
    EXPECT_EQ(KEY_ERROR ,get1);
    EXPECT_EQ(0, value);
    auto get2 = hash_map_get(map, "gonna", &value);
    EXPECT_EQ(KEY_ERROR ,get2);
    EXPECT_EQ(0, value);
    auto get3 = hash_map_get(map, "give", &value);
    EXPECT_EQ(KEY_ERROR ,get3);
    EXPECT_EQ(0, value);
}

TEST_F(EmptyMap, hash_map_pop)
{
    int value = 0;
    auto pop1 = hash_map_pop(map, "never", &value);
    EXPECT_EQ(KEY_ERROR ,pop1);
    EXPECT_EQ(0, value);
    EXPECT_EQ(8, map->allocated);
    EXPECT_EQ(0,map->used);
    auto pop2 = hash_map_pop(map, "gonna", &value);
    EXPECT_EQ(KEY_ERROR ,pop2);
    EXPECT_EQ(0, value);
    EXPECT_EQ(8, map->allocated);
    EXPECT_EQ(0,map->used);
    auto pop3 = hash_map_pop(map, "give", &value);
    EXPECT_EQ(KEY_ERROR ,pop3);
    EXPECT_EQ(0, value);
    EXPECT_EQ(8, map->allocated);
    EXPECT_EQ(0,map->used);
}

TEST_F(EmptyMap, hash_map_remove)
{
    auto remove1 = hash_map_remove(map, "never");
    EXPECT_EQ(KEY_ERROR, remove1);
}

/*  TESTS ON NONEMPTY MAP */

TEST_F(NonEmptyMap, contains)
{
    auto contains1 = hash_map_contains(map, "Potatoes");
    EXPECT_TRUE(contains1);
    auto contains2 = hash_map_contains(map, "boilem");
    EXPECT_TRUE(contains2);
    auto contains3 = hash_map_contains(map, "mashem");
    EXPECT_TRUE(contains3);
    auto contains4 = hash_map_contains(map, "stickem");
    EXPECT_FALSE(contains4);
}

TEST_F(NonEmptyMap, contains_after_realloc)
{
    hash_map_reserve(map, 16);
    auto contains1 = hash_map_contains(map, "Potatoes");
    EXPECT_TRUE(contains1);
    auto contains2 = hash_map_contains(map, "boilem");
    EXPECT_TRUE(contains2);
    auto contains3 = hash_map_contains(map, "mashem");
    EXPECT_TRUE(contains3);
    auto contains4 = hash_map_contains(map, "stickem");
    EXPECT_FALSE(contains4);
}


TEST_F(NonEmptyMap, put_one_different)
{
    auto return1 = hash_map_put(map, "stickem", 8);
    ASSERT_EQ(OK, return1);
    auto contains = hash_map_contains(map, "stickem");
    EXPECT_TRUE(contains);
    EXPECT_EQ(4, map->used);
}

TEST_F(NonEmptyMap, put_existing)
{
    auto return1 = hash_map_put(map, "Potatoes", 8);
    ASSERT_EQ(KEY_ALREADY_EXISTS, return1);
    auto contains = hash_map_contains(map, "Potatoes");
    EXPECT_TRUE(contains);
    int dst;
    auto get = hash_map_get(map, "Potatoes", &dst);
    EXPECT_EQ(OK, get);
    EXPECT_EQ(8, dst);
    EXPECT_EQ(3, map->used);
}

TEST_F(NonEmptyMap, put_existing_after_realloc)
{
    // set capacity to 6
    auto return1 = hash_map_reserve(map, 6);
    ASSERT_EQ(OK, return1);
    ASSERT_EQ(hash_map_capacity(map), 6);

    // used before this put is 3, which is less than 2/3 of 6 -> should not reallocate
    auto return2 = hash_map_put(map, "Alpha", 4);
    ASSERT_EQ(OK, return2);
    auto contains = hash_map_contains(map, "Alpha");
    ASSERT_TRUE(contains);
    ASSERT_EQ(hash_map_capacity(map), 6);

    // used before this put is 4, which is exactly 2/3 of 6 -> should reallocate
    auto return3 = hash_map_put(map, "Alpha", 9);
    ASSERT_EQ(hash_map_capacity(map), 12);
    // as we added an item with key "Alpha" before, it should return KEY_ALREADY_EXISTS
    EXPECT_EQ(KEY_ALREADY_EXISTS, return3);
    
    int value;
    EXPECT_EQ(hash_map_get(map,"Alpha",&value), OK);
    EXPECT_EQ(9, value);
    
}

TEST_F(NonEmptyMap, put_reallocation_under_8)
{
    /* 
        space for 8
        used: 3
        2/3 of 8 = 5.333 = threshold according to specification
    */

    hash_map_put(map, "stickem", 8);
    auto contains1 = hash_map_contains(map, "stickem");
    ASSERT_TRUE(contains1);
    hash_map_put(map, "in a stew", 9);
    auto contains2 = hash_map_contains(map, "in a stew");
    ASSERT_TRUE(contains2);
    /* 
        added 2
        should not reallocate as 5<5.333
    */

    EXPECT_EQ(5, map->used);
    EXPECT_EQ(8, map->allocated);
}

TEST_F(NonEmptyMap, put_reallocation_over_8)
{
    /* 
        space for 8
        used: 3
        2/3 of 8 = 5.333 = threshold according to specification
    */

    hash_map_put(map, "stickem", 8);
    auto contains1 = hash_map_contains(map, "stickem");
    ASSERT_TRUE(contains1);

    hash_map_put(map, "in a stew", 9);
    auto contains2 = hash_map_contains(map, "in a stew");
    ASSERT_TRUE(contains2);

    hash_map_put(map, "precious", 20);
    auto contains3 = hash_map_contains(map, "precious");
    ASSERT_TRUE(contains3);
    /* 
        added 3
        should reallocate as 6>5.333
    */
    ASSERT_EQ(6,map->used);
    EXPECT_EQ(16, map->allocated);
}

TEST_F(NonEmptyMap, put_reallocation_under_16)
{
    ASSERT_EQ(hash_map_reserve(map, 16), OK);
    ASSERT_EQ(map->allocated, 16);
    // reserved 16 slots
    std::vector<const char *> keys {"one","two","three","four","five","six","seven"};
    std::vector<int> values {99,98,-97,96,-95,94,93};
    int count = 0;
    for(auto key : keys)
    {
        ASSERT_EQ(hash_map_put(map, key, values[count]),OK);
        count++;
    }
    /*
        added 8
    */
    ASSERT_EQ(map->used, 10);
    ASSERT_EQ(hash_map_put(map, "eight", 92),OK);
    /*
        2/3 of 16 is 10.666
        used before putting "eight" is 10, which is < than 10.666
        should not reallocate     
    */
   ASSERT_EQ(map->used, 11);
   ASSERT_EQ(map->allocated, 16);
}

TEST_F(NonEmptyMap, clear)
{    
    hash_map_clear(map);
    EXPECT_TRUE(map->first == NULL);
    EXPECT_TRUE(map->last == NULL);
    for (size_t i = 0; i < map->allocated; ++i)
    {
        EXPECT_TRUE(map->index[i] == NULL);
    }
    EXPECT_EQ(0, map->used);
}

TEST_F(NonEmptyMap, reserve)
{
    auto return1 = hash_map_reserve(map, 8);
    ASSERT_EQ(OK, return1);
    ASSERT_EQ(8, map->allocated);

    auto return2 = hash_map_reserve(map, 16);
    ASSERT_EQ(OK, return2);
    ASSERT_EQ(16, map->allocated);

    auto return3 = hash_map_reserve(map, 2);
    ASSERT_EQ(VALUE_ERROR, return3);
    ASSERT_EQ(16, map->allocated);

    auto return4 = hash_map_reserve(map, -1);
    ASSERT_EQ(MEMORY_ERROR, return4);
    ASSERT_EQ(16, map->allocated);
}

TEST_F(NonEmptyMap, size)
{
    auto return1 = hash_map_size(map);
    ASSERT_EQ(3, return1);
}

TEST_F(NonEmptyMap, capacity)
{
    auto return1 = hash_map_capacity(map);
    ASSERT_EQ(8, return1);
}

TEST_F(NonEmptyMap, pop_first)
{
    int dst;
    auto return1 = hash_map_pop(map, "Potatoes", &dst);
    ASSERT_EQ(OK, return1);
    EXPECT_EQ(1, dst);
    ASSERT_FALSE(hash_map_contains(map, "Potatoes"));
    EXPECT_EQ(3, map->used); 
    EXPECT_EQ(8, map->allocated);
}

TEST_F(NonEmptyMap, pop_not_first)
{
    int dst;
    auto return1 = hash_map_pop(map, "boilem", &dst);
    ASSERT_EQ(OK, return1);
    EXPECT_EQ(2, dst);
    ASSERT_FALSE(hash_map_contains(map, "boilem"));
    EXPECT_EQ(3, map->used); 
    EXPECT_EQ(8, map->allocated);
}

TEST_F(NonEmptyMap, pop_last)
{
    int dst;
    auto return1 = hash_map_pop(map, "mashem", &dst);
    ASSERT_EQ(OK, return1);
    EXPECT_EQ(3, dst);
    ASSERT_FALSE(hash_map_contains(map, "mashem"));
    EXPECT_EQ(3, map->used); 
    EXPECT_EQ(8, map->allocated);
}

TEST_F(NonEmptyMap, pop_non_existing)
{
    int dst;
    auto return1 = hash_map_pop(map, "stick", &dst);
    ASSERT_EQ(KEY_ERROR, return1);
    EXPECT_EQ(3, map->used); 
    EXPECT_EQ(8, map->allocated);
}

TEST_F(NonEmptyMap, remove_not_first)
{
    auto return1 = hash_map_remove(map, "boilem");
    ASSERT_EQ(OK, return1);
    ASSERT_FALSE(hash_map_contains(map, "boilem"));
    EXPECT_EQ(3, map->used); 
    EXPECT_EQ(8, map->allocated);
}

TEST_F(NonEmptyMap, remove_last)
{
    auto return1 = hash_map_remove(map, "mashem");
    ASSERT_EQ(OK, return1);
    ASSERT_FALSE(hash_map_contains(map, "mashem"));
    EXPECT_EQ(3, map->used); 
    EXPECT_EQ(8, map->allocated);
}

TEST_F(NonEmptyMap, remove_non_existing)
{
    auto return1 = hash_map_remove(map, "stick");
    ASSERT_EQ(KEY_ERROR, return1);
    EXPECT_EQ(3, map->used); 
    EXPECT_EQ(8, map->allocated);
}
TEST_F(NonEmptyMap, remove_after_realloc)
{
    hash_map_reserve(map, 16);
    auto return1 = hash_map_remove(map, "boilem");
    ASSERT_EQ(OK, return1);
    ASSERT_FALSE(hash_map_contains(map, "boilem"));
    EXPECT_EQ(3, map->used); 
    EXPECT_EQ(8, map->allocated);
}


// COLLISION TESTING
TEST_F(Collisions, collision_put_get)
{
    auto return1 = hash_map_put(map, "pilif", 60);
    ASSERT_EQ(OK, return1);
    auto return2 = hash_map_put(map, "fliip", 70);
    ASSERT_EQ(OK, return2);

    ASSERT_TRUE(hash_map_contains(map,"filip"));
    ASSERT_TRUE(hash_map_contains(map,"pilif"));
    ASSERT_TRUE(hash_map_contains(map,"fliip"));
    int value;
    hash_map_get(map, "filip", &value);
    int value1;
    hash_map_get(map, "pilif", &value1);
    int value2;
    hash_map_get(map, "fliip", &value2);
    EXPECT_EQ(1, value);
    EXPECT_EQ(60, value1);
    EXPECT_EQ(70, value2);
}

/*** Konec souboru white_box_tests.cpp ***/
