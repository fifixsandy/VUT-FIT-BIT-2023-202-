/*
 * Použití binárních vyhledávacích stromů.
 *
 * S využitím Vámi implementovaného binárního vyhledávacího stromu (soubory ../iter/btree.c a ../rec/btree.c)
 * implementujte triviální funkci letter_count. Všimněte si, že výstupní strom může být značně degradovaný 
 * (až na úroveň lineárního seznamu). Jako typ hodnoty v uzlu stromu využijte 'INTEGER'.
 * 
 * Funkcie doplnil: xnovakf00
 * Datum poslednej upravy: 30/10/2024
 */

#include "../btree.h"
#include <stdio.h>
#include <stdlib.h>


/**
 * Vypočítání frekvence výskytů znaků ve vstupním řetězci.
 * 
 * Funkce inicilializuje strom a následně zjistí počet výskytů znaků a-z (case insensitive), znaku 
 * mezery ' ', a ostatních znaků (ve stromu reprezentováno znakem podtržítka '_'). Výstup je v 
 * uložen ve stromu.
 * 
 * Například pro vstupní řetězec: "abBccc_ 123 *" bude strom po běhu funkce obsahovat:
 * 
 * key | value
 * 'a'     1
 * 'b'     2
 * 'c'     3
 * ' '     2
 * '_'     5
 * 
 * Pro implementaci si můžete v tomto souboru nadefinovat vlastní pomocné funkce.
*/
void letter_count(bst_node_t **tree, char *input) {
    bst_init(tree);
    if(input == NULL){return;}
    int counter = 0; // counter of read characters (index in input string)
    char currentChar = input[counter]; // character to process (starting at first character)
    bst_node_content_t *content = NULL;
    bool found = false;
    while(currentChar != '\0')
    {
        if(currentChar >= 'A' && currentChar <= 'Z'){currentChar = currentChar + ('a' - 'A');} // transform to lowercase
        if((currentChar < 'a' || currentChar > 'z') && currentChar != ' '){currentChar = '_';} // transform every non-letter and non-space to '_'   
        found = bst_search(*tree, currentChar, &content);
        if(found)
        {
            (*(int *)(content->value))++; // increase the value of counter of character
        }
        else // create new node for new character
        {
            bst_node_content_t newContent;
            newContent.value = malloc(sizeof(int)); 
            if(newContent.value == NULL){return;}
            newContent.type  = INTEGER;
            *((int*)(newContent.value)) = 1;
            bst_insert(tree, currentChar, newContent);
        }
        counter++; // advance in the input string
        currentChar = input[counter];
    }
}