#include <stdio.h>
#include <stdlib.h>

#include "scanner.h"
#include "treeForm.h"


typedef struct FormTreeNode *FormTree;




FormTree recSimplify ( FormTree t ) {
	if( t->left!=NULL) {
		t->left = recSimplify(t->left);
	}
	if(t->right!=NULL) {
		t->right = recSimplify(t->right);
	}
	
	if(t->left == NULL && t->right == NULL) return t;
	else if(t->t.symbol=='~' && t->left->t.symbol == 'T') {
		t->t.symbol='F';
		free(t->left);
		return t;
	}
	else if(t->t.symbol=='~' && t->left->t.symbol == 'F'){
		t->t.symbol='T';
		free(t->left);
		return t;
	}
	else if(t->t.symbol=='~' && t->left->tt == Identifier) return t; 
	else if(t->t.symbol=='~' && t->left->t.symbol == '~' && t->left->left->tt == Identifier) return t->left->left;
}

void simplify( FormTree *t){
  *t=recSimplify(*t);
  return;
}
	


int acceptCharacterEx(List *lp, char c) {
  if (*lp != NULL && (*lp)->tt == Symbol && ((*lp)->t).symbol == c ) {
    *lp = (*lp)->next;
    return 1;
  }
  return 0;
}

int treeDisjunction(List *lp, FormTree *t) {
  if ( !treeFormula(lp,t) ) {
    return 0;
  }
  while ( acceptCharacterEx(lp,'|') ) {
    FormTree tL = *t;
    FormTree tR = NULL;
    if ( !treeFormula(lp,&tR) ) {
      freeTree(tR);
      return 0;
    }
    Token tok;
    tok.symbol = '|';
    *t = newFormTreeNode(Symbol, tok, tL, tR);
  } 
  return 1;
}

int treeImplication(List *lp, FormTree *t) {
  if ( !treeDisjunction(lp,t) ) {
    return 0;
  }
  if (acceptCharacterEx(lp,'-')) {
    if (!acceptCharacterEx(lp,'>')){
      return 0;
    }
    else {
      FormTree tL = *t;
      FormTree tR = NULL;
      if ( !treeDisjunction(lp,&tR) ) {
        freeTree(tR);
        return 0;
      }
      Token tok;
      tok.symbol = '>';
      *t = newFormTreeNode(Symbol, tok, tL, tR);
    }
  } 
  return 1;
}

int treeBiconditional(List *lp, FormTree *t) {
  if ( !treeImplication(lp,t) ) {
    return 0;
  }
  if (acceptCharacterEx(lp,'<')) {
    if (!acceptCharacterEx(lp,'-')){
      return 0;
    }
    if (!acceptCharacterEx(lp,'>')){
        return 0;
      }
    else {
      FormTree tL = *t;
      FormTree tR = NULL;
      if ( !treeImplication(lp,&tR) ) {
        freeTree(tR);
        return 0;
      }
      Token tok;
      tok.symbol = '<';
      *t = newFormTreeNode(Symbol, tok, tL, tR);
    }
  } 
  return 1;
}

int main(int argc, char *argv[]) {
  char *ar;
  List tl, tl1;
  tl1 = NULL;
  printf("give a formula: ");
  ar = readInput();
  while (ar[0] != '!') {
    tl = tokenList(ar);
    printList(tl);
    tl1 = tl;
    FormTree t = NULL;
    if ( treeBiconditional(&tl1,&t) && tl1 == NULL ) {
      printf("with parentheses: ");
      int max=0;
      printTree(t,0,&max);
      
      printf("\n");
      freeTree(t);
      printf("complexity: %d\n", max);
    } else {
      printf("this is not a formula\n");
      if (t != NULL) {
        freeTree(t);
      }
    }
    free(ar);
    freeTokenList(tl);
    printf("\ngive a formula: ");
    ar = readInput();
  }
  free(ar);
  printf("good bye\n");
  return 0;
}
