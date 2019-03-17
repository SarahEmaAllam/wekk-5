#include <stdio.h>
#include <stdlib.h>
#include "scanner.h"
#include "treeForm.h"

FormTree copyTree(FormTree tree) {
  // copy subtrees, and add them to new tree
  return tree == NULL ? NULL : newFormTreeNode(
    tree->tt,
    tree->t,
    copyTree(tree->left),
    copyTree(tree->right)
  );
}

FormTree childToParent( FormTree t, char c){
  // If c=='l', return left child, free the parent and the right child
  if (c=='l'){  
    FormTree t1=t->left;
    freeTree(t->right);
    free(t);
    return t1;
  } // else, return the right child, free the parent and the left child
  FormTree t1=t->right;
  freeTree(t->left);
  free(t);
  return t1;
}

FormTree recSimplify ( FormTree t ) {
	if(t==NULL) {
    return t;
  }
  t->left = recSimplify(t->left);
  t->right = recSimplify(t->right);
  // Negation
  if(t->t.symbol=='~' && t->left->t.symbol == 'T') { 
    t->left->t.symbol='F';
    return t=childToParent(t,'l');
	}
	if(t->t.symbol=='~' && t->left->t.symbol == 'F'){
		t->left->t.symbol='T';
    return t=childToParent(t,'l');
	}
  if(t->t.symbol=='~' && t->left->t.symbol == '~') {
    /* Call childToParent twice so the first negation changes into the 
      formula*/
    t->left=childToParent(t->left,'l');
    return t=childToParent(t,'l');
  }
  // Disjunction
  if(t->t.symbol=='|' && t->left->tt == Symbol) { 
    if (t->left->t.symbol == 'T'){
      return t=childToParent(t,'l');
    }
    else if (t->left->t.symbol == 'F') {
      return t=childToParent(t,'r');
    }
  }
  if(t->t.symbol=='|' && t->right->tt == Symbol) {
    if (t->right->t.symbol == 'T'){
      return t=childToParent(t,'r');
    }
    else if (t->right->t.symbol == 'F') {
      return t=childToParent(t,'l');
    }
  }
  // Conjuction
  if(t->t.symbol=='&' && t->left->tt == Symbol) { 
    if (t->left->t.symbol == 'F'){
      return t=childToParent(t,'l');
    }
    else if (t->left->t.symbol == 'T') {
      return t=childToParent(t,'r');
    }
  }
  if(t->t.symbol=='&' && t->right->tt == Symbol) {
    if (t->right->t.symbol == 'F'){
      return t=childToParent(t,'r');
    }
    else if (t->right->t.symbol == 'T') {
      return t=childToParent(t,'l');
    }
  }
  // Implication
  if(t->t.symbol=='>' && t->left->tt == Symbol) { 
    if (t->left->t.symbol == 'F'){
      t->left->t.symbol='T';
      return t=childToParent(t,'l');
    }
    else if (t->left->t.symbol == 'T') {
      return t=childToParent(t,'r');
    }
  }
  if(t->t.symbol=='>' && t->right->tt == Symbol) {
    if (t->right->t.symbol == 'T'){
      return t=childToParent(t,'r');
    }
    else if (t->right->t.symbol == 'F') {
      t->t.symbol='~';
      freeTree(t->right);
      t->right=NULL;
      /* a negation is added, so call recSimplify again to possibly 
       simplify it further*/
      return recSimplify(t);
    }
  }
  // Biconditional
  if(t->t.symbol=='<' && t->left->tt == Symbol) { 
    if (t->left->t.symbol == 'T'){
      return t=childToParent(t,'r');
    }
    else if (t->left->t.symbol == 'F') {
      t->t.symbol='~';
      free(t->left);
      t->left=t->right;
      t->right=NULL;
      return recSimplify(t);
    }
  }
  if(t->t.symbol=='<' && t->right->tt == Symbol) {
    if (t->right->t.symbol == 'T'){
      return t=childToParent(t,'l');
    }
    else if (t->right->t.symbol == 'F') {
      t->t.symbol='~';
      freeTree(t->right);
      t->right=NULL;
      return recSimplify(t);
    }
  }
  return t;
}  

void simplify( FormTree *t){
  *t=recSimplify(*t);
  return;
}

FormTree disToCon( FormTree t){
  t->t.symbol='~';
  /* Form new left conjuctional subtree with the children of t, and make 
    that the left child of t */
  t->left=newFormTreeNode(Symbol,t->t,t->left,t->right);
  t->left->t.symbol='&';
  t->right=NULL;
  // Add two negations to the conjuctional subtree
  t->left->left=newFormTreeNode(Symbol,t->t,t->left->left,NULL);
  t->left->right=newFormTreeNode(Symbol,t->t,t->left->right,NULL);
  return t;
}

FormTree impToDis( FormTree t){
  t->t.symbol='|';
  t->left=newFormTreeNode(Symbol,t->t,t->left,NULL);
  t->left->t.symbol='~';
  return t;
}

FormTree biToCon( FormTree t){
  t->t.symbol='|';
  t->left=newFormTreeNode(Symbol,t->t,t->left,t->right);
  t->left->t.symbol='&';
  // Copy the left child of t and make it the right child of t
  FormTree copy=copyTree(t->left);
  // Add two negations to the copied tree
  copy->left=newFormTreeNode(Symbol,t->t,copy->left,NULL);
  copy->left->t.symbol='~';
  copy->right=newFormTreeNode(Symbol,copy->left->t,copy->right,NULL);
  t->right=copy;
  return t;
}

FormTree recTranslate( FormTree t){
  if(t==NULL) {
    return t;
  }
  t->left = recTranslate(t->left);
  t->right = recTranslate(t->right);
  if (t->t.symbol=='|'){
    return t=disToCon(t);
  }
  if (t->t.symbol=='>'){
    t=impToDis(t);
    return t=disToCon(t);
  }
  if (t->t.symbol=='<'){
    t=biToCon(t);
    return t=disToCon(t);
  }
  return t;
}

void translate( FormTree *t){
  *t=recTranslate(*t);
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
    // If no '>' is accepted after '-' , return 0
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
      printf("complexity: %d\n", max);
      printf("simplified: ");
      simplify(&t);
      printTree(t,0,&max);
      printf("\n");
      printf("translated: ");
      translate(&t);
      printTree(t,0,&max);
      printf("\n");
      printf("simplified: ");
      simplify(&t);
      printTree(t,0,&max);
      printf("\n");
      freeTree(t);
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
