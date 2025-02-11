/*
 * Binární vyhledávací strom — rekurzivní varianta
 *
 * S využitím datových typů ze souboru btree.h a připravených koster funkcí
 * implementujte binární vyhledávací strom pomocí rekurze.
 * 
 * Funkcie doplnil: xnovakf00
 * Datum poslednej upravy: 30/10/2024
 */

#include "../btree.h"
#include <stdio.h>
#include <stdlib.h>

/*
 * Inicializace stromu.
 *
 * Uživatel musí zajistit, že inicializace se nebude opakovaně volat nad
 * inicializovaným stromem. V opačném případě může dojít k úniku paměti (memory
 * leak). Protože neinicializovaný ukazatel má nedefinovanou hodnotu, není
 * možné toto detekovat ve funkci.
 */
void bst_init(bst_node_t **tree)
{
  *tree = NULL;
}

/*
 * Vyhledání uzlu v stromu.
 *
 * V případě úspěchu vrátí funkce hodnotu true a do proměnné value zapíše
 * ukazatel na obsah daného uzlu. V opačném případě funkce vrátí hodnotu false a proměnná
 * value zůstává nezměněná.
 *
 * Funkci implementujte rekurzivně bez použité vlastních pomocných funkcí.
 */
bool bst_search(bst_node_t *tree, char key, bst_node_content_t **value)
{
  if(tree == NULL){return false;} // empty (sub)tree, element not found

  if(tree->key == key) // element found
  {
    *value = &tree->content;
    return true;
  }
  else if(tree->key > key) // going to left subtree
  {
    return(bst_search(tree->left, key, value));
  }
  else // going to right subtree
  {
    return(bst_search(tree->right, key, value));
  }
}

/*
 * Vložení uzlu do stromu.
 *
 * Pokud uzel se zadaným klíče už ve stromu existuje, nahraďte jeho hodnotu.
 * Jinak vložte nový listový uzel.
 *
 * Výsledný strom musí splňovat podmínku vyhledávacího stromu — levý podstrom
 * uzlu obsahuje jenom menší klíče, pravý větší.
 *
 * Funkci implementujte rekurzivně bez použití vlastních pomocných funkcí.
 */
void bst_insert(bst_node_t **tree, char key, bst_node_content_t value)
{
  bst_node_t *nodePtr = *tree;
  if(nodePtr == NULL) // not found, creating new node
  {
    bst_node_t *newNode = (bst_node_t*)malloc(sizeof(bst_node_t));
    if(newNode == NULL){return;}
    newNode->content    = value;
    newNode->key        = key;
    newNode->left       = NULL; // it will be a leaf node
    newNode->right      = NULL; //
    *tree               = newNode;
    return; 
  }

  if(nodePtr->key == key) // already exists, update content
  {
    free(nodePtr->content.value); // free the previous content
    nodePtr->content = value;
    return;
  }
  else if(nodePtr->key > key) // going left
  {
    bst_insert(&(nodePtr->left), key, value);
  }
  else // going right
  {
    bst_insert(&(nodePtr->right), key, value);
  }
}

/*
 * Pomocná funkce která nahradí uzel nejpravějším potomkem.
 *
 * Klíč a hodnota uzlu target budou nahrazeny klíčem a hodnotou nejpravějšího
 * uzlu podstromu tree. Nejpravější potomek bude odstraněný. Funkce korektně
 * uvolní všechny alokované zdroje odstraněného uzlu.
 *
 * Funkce předpokládá, že hodnota tree není NULL.
 *
 * Tato pomocná funkce bude využitá při implementaci funkce bst_delete.
 *
 * Funkci implementujte rekurzivně bez použití vlastních pomocných funkcí.
 */
void bst_replace_by_rightmost(bst_node_t *target, bst_node_t **tree)
{
  bst_node_t *nodePtr = *tree;
  
  if(nodePtr->right == NULL) // we found the rightmost node
  {
    free(target->content.value);
    target->content = nodePtr->content;
    target->key     = nodePtr->key;
    *tree           = nodePtr->left; // the rightmost is to be deleted and the new rightmost will be left of the previous 
                                     // (or replaced by NULL if there was no left child)
    free(nodePtr);
  }
  else // not yet the rightmost, go further right
  {
    bst_replace_by_rightmost(target, &nodePtr->right); 
  }
}

/*
 * Odstranění uzlu ze stromu.
 *
 * Pokud uzel se zadaným klíčem neexistuje, funkce nic nedělá.
 * Pokud má odstraněný uzel jeden podstrom, zdědí ho rodič odstraněného uzlu.
 * Pokud má odstraněný uzel oba podstromy, je nahrazený nejpravějším uzlem
 * levého podstromu. Nejpravější uzel nemusí být listem.
 *
 * Funkce korektně uvolní všechny alokované zdroje odstraněného uzlu.
 *
 * Funkci implementujte rekurzivně pomocí bst_replace_by_rightmost a bez
 * použití vlastních pomocných funkcí.
 */
void bst_delete(bst_node_t **tree, char key)
{
  if(tree == NULL){return;}
  bst_node_t *nodePtr = *tree;
  if(nodePtr == NULL){return;} // nothing to delete

  if(nodePtr->key == key) // found the node to delete
  {
    if(nodePtr->left == NULL && nodePtr->right == NULL) // no children
    {
      free(nodePtr->content.value);
      free(nodePtr);
      *tree = NULL;
      return;
    }
    else if(nodePtr->left != NULL && nodePtr->right != NULL) // both children present
    {
      bst_replace_by_rightmost(nodePtr, &nodePtr->left); // replace by rightmost of left subtree
      return;
    }
    else // only one child
    {
      if(nodePtr->left != NULL) // the left child is present
      {
        *tree = nodePtr->left; // replace the node to delete with left child in tree
        free(nodePtr->content.value);
        free(nodePtr); // free the one to delete
        return;
      }
      else // right child is present
      {
        *tree = nodePtr->right; // replace the node to delete with right child in tree
        free(nodePtr->content.value);
        free(nodePtr); // free the one to delete
        return;
      }
    }
  }
  else if(nodePtr->key > key) // go left
  {
    bst_delete(&nodePtr->left, key);
  }
  else // go right
  {
    bst_delete(&nodePtr->right, key);
  }
}

/*
 * Zrušení celého stromu.
 *
 * Po zrušení se celý strom bude nacházet ve stejném stavu jako po
 * inicializaci. Funkce korektně uvolní všechny alokované zdroje rušených
 * uzlů.
 *
 * Funkci implementujte rekurzivně bez použití vlastních pomocných funkcí.
 */
void bst_dispose(bst_node_t **tree)
{
  if(tree == NULL){return;} // void pointer as argument
  bst_node_t *nodePtr = *tree;
  if(nodePtr == NULL){return;} // nothing to delete

  bst_dispose(&nodePtr->left); // delete whole left subtree
  bst_dispose(&nodePtr->right); // delete whole right subtree
  free(nodePtr->content.value);
  free(nodePtr); // free the node
  *tree = NULL; // set the pointer of root to NULL
}

/*
 * Preorder průchod stromem.
 *
 * Pro aktuálně zpracovávaný uzel zavolejte funkci bst_add_node_to_items.
 *
 * Funkci implementujte rekurzivně bez použití vlastních pomocných funkcí.
 */
void bst_preorder(bst_node_t *tree, bst_items_t *items)
{
  if(tree != NULL)
  {
    bst_add_node_to_items(tree, items);
    bst_preorder(tree->left, items);
    bst_preorder(tree->right, items);
  }
}

/*
 * Inorder průchod stromem.
 *
 * Pro aktuálně zpracovávaný uzel zavolejte funkci bst_add_node_to_items.
 *
 * Funkci implementujte rekurzivně bez použití vlastních pomocných funkcí.
 */
void bst_inorder(bst_node_t *tree, bst_items_t *items)
{
    if(tree != NULL)
  {
    bst_inorder(tree->left, items);
    bst_add_node_to_items(tree, items);
    bst_inorder(tree->right, items);
  }
}

/*
 * Postorder průchod stromem.
 *
 * Pro aktuálně zpracovávaný uzel zavolejte funkci bst_add_node_to_items.
 *
 * Funkci implementujte rekurzivně bez použití vlastních pomocných funkcí.
 */
void bst_postorder(bst_node_t *tree, bst_items_t *items)
{
  if(tree != NULL)
  {
    bst_postorder(tree->left, items);
    bst_postorder(tree->right, items);
    bst_add_node_to_items(tree, items);
  }
}
