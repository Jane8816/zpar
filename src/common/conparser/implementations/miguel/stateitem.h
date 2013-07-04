// Copyright (C) University of Oxford 2010
/*===============================================================
 *
 * stanford dependencies in ZPar  
 * Authors: Miguel Ballesteros and Yue Zhang
 * UPF-Spain and SUTD-Singapore
 *
 *==============================================================*/

#ifndef _COMMON_CON_PARSER_STATEITEM
#define _COMMON_CON_PARSER_STATEITEM

#include "action.h"
#include "linkclass.h"


/*===============================================================
 *
 * CStateNode - tree nodes 
 *
 *==============================================================*/

class CStateNode {
public:
   enum NODE_TYPE {LEAF=0, SINGLE_CHILD, HEAD_LEFT, HEAD_RIGHT};
public:
   int id;
   NODE_TYPE type;
   bool temp;
   // fields for constituents
   CConstituent constituent;
   const CStateNode* left_child;
   const CStateNode* right_child;
   // fields for tokens and constituents
   int lexical_head;
   int lexical_start;
   int lexical_end; 
 
   CLink* cLinkCollapsed; //miguel
   CLink* cLink; //miguel
   
   //const CHeadFinder* headFinder;

public:
   inline bool head_left() const { return type==HEAD_LEFT; }
   inline bool single_child() const { return type==SINGLE_CHILD; }
   inline bool is_constituent() const { return type!=LEAF; }

public:
   CStateNode(const int &id, const NODE_TYPE &type, const bool &temp, const unsigned long &constituent, CStateNode *left_child, CStateNode *right_child, const int &lexical_head, const int &lexical_start, const int &lexical_end) : id(id), type(type), temp(temp), constituent(constituent), left_child(left_child), right_child(right_child), lexical_head(lexical_head), lexical_start(lexical_start), lexical_end(lexical_end), cLinkCollapsed(0), cLink(0) {}
   CStateNode() : id(-1), type(), temp(0), constituent(), left_child(0), right_child(0), lexical_head(0), lexical_start(0), lexical_end(0), cLinkCollapsed(0), cLink(0) {}
   virtual ~CStateNode() {
	   CLink* temp=cLink;
	   while (temp!=0) {
		   CLink* next=temp->next;
		   delete temp;
		   temp=next;
	   }
   }
public:

   bool valid() const { return id!=-1; }
   void clear() { 
      this->id = -1;
      this->type = static_cast<NODE_TYPE>(0); 
      this->temp = 0; 
      this->constituent.clear(); 
      this->left_child = 0; 
      this->right_child = 0; 
      this->lexical_head = 0; 
      this->lexical_start = 0; 
      this->lexical_end = 0; 

      //this->cLink = 0; //miguel
      this->cLink = 0; //miguel
      //this->cLinkCollapsed =0; //miguel
      this->cLinkCollapsed = 0; //miguel
   }
   void set(const int &id, const NODE_TYPE &type, const bool &temp, const unsigned long &constituent, const CStateNode *left_child, const CStateNode *right_child, const int &lexical_head, const int &lexical_start, const int &lexical_end) { 
      this->id = id;
      this->type = type; 
      this->temp = temp; 
      this->constituent = constituent; 
      this->left_child = left_child; 
      this->right_child = right_child; 
      this->lexical_head = lexical_head; 
      this->lexical_start = lexical_start; 
      this->lexical_end = lexical_end;
      
      this->cLink=0; //Miguel
      
   }//{}

   bool operator == (const CStateNode &nd) const {
      return id == nd.id &&
             type == nd.type && 
             temp == nd.temp && 
             constituent == nd.constituent && 
             left_child == nd.left_child && 
             right_child == nd.right_child &&
             lexical_head == nd.lexical_head;
             lexical_start == nd.lexical_start;
             lexical_end == nd.lexical_end;
             
             //the stanford links are missing. Let's see whether they are necessary or not. (Miguel)
   }
   void operator = (const CStateNode &nd) {
      id = nd.id;
      type = nd.type;
      temp = nd.temp;
      constituent.copy(nd.constituent);
      left_child = nd.left_child;
      right_child = nd.right_child;
      lexical_head = nd.lexical_head;
      lexical_start = nd.lexical_start;
      lexical_end = nd.lexical_end;
      
      //the stanford links are missing. Let's see whether they are necessary or not. (Miguel)
   }
public:
   void toCCFGTreeNode(CCFGTreeNode &node) const {
//      node.parent = parent->id;
      node.is_constituent = is_constituent();
      node.temp = temp;
#ifdef NO_TEMP_CONSTITUENT
      ASSERT(!node.temp, "Internal error: this version does not temporary constituents but state items have them.")
      node.constituent = constituent.code();
#else
      node.constituent = constituent.extractConstituentCode();
#endif
      node.single_child = single_child();
      node.head_left = head_left();
      node.left_child = left_child ? left_child->id : -1;
      node.right_child = right_child ? right_child->id : -1;
      node.token = lexical_head;
   }
     
};



/*===============================================================
 *
 * CStateItem - the search state item, representing a partial
 *              candidate with shift reduce. 
 *
 *==============================================================*/

class CContext;

class CStateItem {
public:
	
   const std::vector<CTaggedWord<CTag, TAG_SEPARATOR> > *words; //Miguel
	
   SCORE_TYPE score;
   CStateNode node; //head of the stack (top)
   const CStateItem *statePtr; //points to the state item before the action is applied, (chain of actions)
   const CStateItem *stackPtr; //tail of the stack
   int current_word;
   CAction action;
#ifdef TRAIN_LOSS
   CStack<CLabeledBracket> gold_lb;
   unsigned correct_lb;
   unsigned plost_lb;
   unsigned rlost_lb;
   bool bTrain;
#endif
#ifdef SCALE
   unsigned size;
#endif
   
public:
#ifdef TRAIN_LOSS
#define LOSS_CON , correct_lb(0), plost_lb(0), rlost_lb(0), bTrain(false)
#else
#define LOSS_CON 
#endif
#ifdef SCALE
#define SCALE_CON , size(0)
#else
#define SCALE_CON 
#endif
   CStateItem() : current_word(0), score(0), action(), stackPtr(0), statePtr(0), node() LOSS_CON SCALE_CON {}
   virtual ~CStateItem() {}
public:
   void clear() {
      statePtr = 0;
      stackPtr = 0;
      current_word = 0;
      node.clear();
      score = 0;
      action.clear();
#ifdef TRAIN_LOSS
      gold_lb.clear();
      correct_lb=0;
      plost_lb=0;
      rlost_lb=0;
      bTrain = false;
#endif
#ifdef SCALE
      size = 0;
#endif
   }
   bool empty() const {
      if (current_word==0) {
         assert(stackPtr == 0 && statePtr == 0 && score==0);
         return true;
      }
      return false;
   }
   unsigned stacksize() const {
      unsigned retval = 0;
      const CStateItem *current = this;
      while (current) {
         if (current->node.valid()) ++retval;// no node -> start/fini
         current = current->stackPtr;
      }
      return retval;
   }
   unsigned unaryreduces() const {
      unsigned retval = 0;
      const CStateItem *current = this;
      while (current) {
         if (current->action.type() == CActionType::REDUCE_UNARY)
            ++retval;
         else
            return retval;
         current = current->statePtr;
      }
      return retval;
   }
   int newNodeIndex() const { return node.id+1; }

public:
   bool operator < (const CStateItem &st) const { return score < st.score; }
   bool operator > (const CStateItem &st) const { return score > st.score; }
   bool operator == (const CStateItem &st) const {
      THROW("State items are not comparable with each other");
   }
   bool operator != (const CStateItem &st) const { return !( (*this) == st ); }

protected:
   // now this actions MUST BE called by Move
   void shift(CStateItem *retval, const unsigned long &constituent = CConstituent::NONE) const {
      //TRACE("shift");
      assert(!IsTerminated());
      retval->node.set(node.id+1, CStateNode::LEAF, false, constituent, 0, 0, current_word, current_word, current_word);
      retval->current_word = current_word+1;
      retval->stackPtr = this; ///  
#ifdef TRAIN_LOSS
      retval->bTrain = this->bTrain;
      computeShiftLB(&(retval->gold_lb), retval->correct_lb, retval->plost_lb, retval->rlost_lb);
#endif
      assert(!retval->IsTerminated());
   }
   void reduce(CStateItem *retval, const unsigned long &constituent, const bool &single_child, const bool &head_left, const bool &temporary) const {
      //TRACE("reduce");
      assert(!IsTerminated());
      const static CStateNode *l, *r;
      assert(stackPtr!=0);
      retval->current_word = current_word; //modified by Miguel and Yue 3 jul 12.01 pm
      if (single_child) {
         assert(head_left == false);
         assert(temporary == false);
         l = &node;
         retval->node.set(node.id+1, CStateNode::SINGLE_CHILD, false, constituent, l, 0, l->lexical_head, l->lexical_start, l->lexical_end);
         retval->stackPtr = stackPtr;
#ifdef TRAIN_LOSS
         retval->bTrain = this->bTrain;
         computeReduceUnaryLB(&(retval->gold_lb), retval->correct_lb, retval->plost_lb, retval->rlost_lb, constituent);
#endif
      }
      else {

         static unsigned long fullconst; 
         assert(stacksize()>=2);
         r = &node;
         l = &(stackPtr->node);
#ifdef NO_TEMP_CONSTITUENT
         fullconst = constituent;
#else
         fullconst = CConstituent::encodeTmp(constituent, temporary);
#endif
         retval->node.set(node.id+1, (head_left?CStateNode::HEAD_LEFT:CStateNode::HEAD_RIGHT), temporary, fullconst, l, r, (head_left?l->lexical_head:r->lexical_head), l->lexical_start, r->lexical_end);
        

         retval->stackPtr = stackPtr->stackPtr;
#ifdef TRAIN_LOSS
         retval->bTrain = this->bTrain;
         computeReduceBinaryLB(&(retval->gold_lb), retval->correct_lb, retval->plost_lb, retval->rlost_lb, fullconst);
#endif
	//SOMEWHERE in this else block we MAKE THE stanford LINKS (Miguel)
	 //By using: R and L. Which are the right child and left child. 
	 //R and L are state nodes. So, the idea is to make links between them. And store them in CLink* stanfordLinks; //miguel
	 // Take into account that we process it in Inorder (or left order)
	
         //retval->node.generateStanford(); //here we call the method that generates the stanford dependencies which is in CStateNode
         
        	 //retval->generateStanford(head_left); //collapsed and then uncollapsed
         	retval->generateStanfordLinks(); //collapsed and then uncollapsed
        	 
      }

      assert(!IsTerminated());
   }

 



   void terminate(CStateItem *retval) const {
      //TRACE("terminate");
//      assert(IsComplete());
      assert(!IsTerminated());
      retval->node = this->node;
      retval->stackPtr=this->stackPtr;
      retval->current_word = current_word;
      // compute loss
#ifdef TRAIN_LOSS
      retval->bTrain = this->bTrain;
      computeTerminateLoss(&(retval->gold_lb), retval->correct_lb, retval->plost_lb, retval->rlost_lb);
#endif
      assert(retval->IsTerminated());
   }
   void noact(CStateItem *retval) const {
      //TRACE("noact");
      assert(IsTerminated());
      retval->node = this->node;
      retval->stackPtr=this->stackPtr;
      retval->current_word = current_word;
      // compute loss
#ifdef TRAIN_LOSS
      retval->bTrain = this->bTrain;
      computeIdleLoss(&(retval->gold_lb), retval->correct_lb, retval->plost_lb, retval->rlost_lb);
#endif
   }

protected:

#ifdef TRAIN_LOSS
   void computeShiftLB( CStack<CLabeledBracket> *gold, unsigned &correct, unsigned &plost, unsigned &rlost) const {
      // compute shift
      if (gold) gold->clear();
      correct = correct_lb;
      plost = plost_lb;
      rlost = rlost_lb;
      if (!bTrain) return;
      static CStack< CLabeledBracket >::const_iterator it;
      it = gold_lb.begin();
      while ( it != gold_lb.end() ) {
         if ( node.valid() && (*it).end == node.lexical_end ) {
            ++ (rlost);
         }
         else {
            if (gold) gold->push( *it );
         }
         ++it;
      } // while
   }
   
   void computeReduceUnaryLB(CStack<CLabeledBracket> *gold, unsigned &correct, unsigned &plost, unsigned &rlost, const unsigned long &constituent) const {
      static CStack< CLabeledBracket >::const_iterator it;
      static bool bCorrect;
      if (gold) gold->clear();
      plost = plost_lb;
      rlost = rlost_lb;
      correct = correct_lb;
      if (!bTrain) return;
      // loop
      it = gold_lb.begin();
      bCorrect = false;
      while ( it != gold_lb.end() ) {
         if ( (*it).begin == node.lexical_start &&
              (*it).end == node.lexical_end &&
              (*it).constituent == constituent ) {
            bCorrect = true;
            ++correct;
         }
         else {
            if (gold) gold->push(*it);
         }
         ++it;
      } //while
      if (!bCorrect)
         ++(plost);
   }

   void computeReduceBinaryLB(CStack<CLabeledBracket> *gold, unsigned &correct, unsigned &plost, unsigned &rlost, const unsigned long &constituent) const {
      const static CStateNode *l, *r;
      static CStack< CLabeledBracket >::const_iterator it;
      static bool bCorrect;
      if (gold) gold->clear();
      correct = correct_lb;
      plost = plost_lb;
      rlost = rlost_lb;
      if (!bTrain) return;
      r = &node;
      l = &(stackPtr->node);
      // loop
      bCorrect = false;
      it=gold_lb.begin();
      while ( it != gold_lb.end() ) {
         if ( (*it).begin == l->lexical_start &&
              (*it).end == r->lexical_end &&
              (*it).constituent == constituent ) {
            bCorrect=true;
            ++correct;
         }
         else if ( (*it).begin == r->lexical_start ) {
            ++(rlost);
         }
         else {
            if (gold) gold->push(*it);
         }
         ++it;
      } // while
      if (!bCorrect)
         ++(plost);
   }
   void computeTerminateLoss(CStack<CLabeledBracket> *gold, unsigned &correct, unsigned &plost, unsigned &rlost) const {
      if (gold) gold->clear();
      correct = correct_lb;
      plost = plost_lb;
      rlost = rlost_lb;
      if (!bTrain) return;
      static CStack< CLabeledBracket >::const_iterator it;
      it = gold_lb.begin();
      while ( it != gold_lb.end() ) {
         assert( (*it).begin == node.lexical_start && (*it).end == node.lexical_end);
         ++(rlost);
         ++it;
      }//while
   }
   void computeIdleLoss(CStack<CLabeledBracket> *gold, unsigned &correct, unsigned &plost, unsigned &rlost) const {
      if (gold) gold->clear();
      correct = correct_lb;
      plost = plost_lb;
      rlost = rlost_lb;
      if (!bTrain) return;
   }
#endif

public:

   void NextMove(const CCFGTree &snt, CAction &retval) const {
      int s = node.id;
      const CCFGTreeNode &nd = snt.nodes[s];
      const CCFGTreeNode &hd = snt.nodes[snt.parent(s)];
      assert(!hd.constituent.empty()); // so that reduce and reduce_root are not same
      bool single_child;
      bool head_left;
      bool temporary;
      // stack top single child ? reduce unary
      if (hd.single_child) {
         single_child = true;
         head_left = false; assert(hd.head_left==false);
         temporary = false; assert(hd.temp==false);
      }
      else {
         // stack top left child ? shift
         if (s == hd.left_child) {
            retval.encodeShift(snt.nodes[newNodeIndex()].constituent.code()); return;
         }
         // stack top right child ? reduce bin
         assert(s==hd.right_child);
         single_child = false;
         head_left = hd.head_left;
         temporary = hd.temp;
#ifdef NO_TEMP_CONSTITUENT
         ASSERT(!temporary, "This version does not accept temporary constituents, but the training data give them.");
#endif
      }
      retval.encodeReduce(hd.constituent.code(), single_child, head_left, temporary);
   }

   void StandardMove(const CCFGTree &tr, CAction &retval) const {
      if (IsTerminated()) {
         retval.encodeIdle();
         return;
      }
      // stack empty?shift
      if (stacksize()==0) {
         retval.encodeShift(tr.nodes[newNodeIndex()].constituent.code());
         return;
      }
      if (tr.parent(node.id) == -1) {
         assert(IsComplete(tr.words.size()));
         retval.encodeReduceRoot();
         return;
      }
      NextMove(tr, retval);
   }

   void Move(CStateItem *retval, const CAction &action) const {
      retval->action = action; // this makes it necessary for the actions to 
      retval->statePtr = this; // be called by Move
      
      retval->words = this->words; // Miguel
      
      if (action.isIdle()) {
         noact(retval);
#ifdef SCALE
         retval->size = this->size;
#endif
      }
      else {
         if (action.isShift())
            { shift(retval, action.getConstituent()); }
         else if (action.isReduceRoot())
            { terminate(retval); }
         else
            { reduce(retval, action.getConstituent(), action.singleChild(), action.headLeft(), action.isTemporary()); }
#ifdef SCALE
         retval->size = this->size + 1;
#endif
      }
   }
   
   bool IsComplete(const int &nWords) const {
#ifdef FRAGMENTED_TREE
      return current_word == nWords; // allow multiple-rt.
#else
      return current_word == nWords && stacksize() == 1;
#endif
   }

   bool IsTerminated() const {
      return action.type() == CActionType::POP_ROOT or action.type() == CActionType::IDLE; 
   }

   bool IsIdle() const {
      return action.type() == CActionType::IDLE; 
   }

   void GenerateTree(const CTwoStringVector &tagged, CSentenceParsed &out) const {
      // parsing done?
      assert(IsTerminated());
//      assert(tagged.size()==sent->size());
      out.clear();
#ifdef FRAGMENTED_TREE
      if (stacksize()>1) {
         static const CStateItem *item;
         item = statePtr;
         assert(item==stackPtr);
         static CStateItem *tmp;
         tmp = new CStateItem[stacksize()];
         static CStateItem *current;
         current = tmp;
         static CAction action;
         action.encodeReduce(CConstituent::NONE, false, false, false);
         while (item->stacksize()>1) {
            // form NONE nodes
            item->Move(current, action); 
            item = current;
            ++ current;
         }
         action.encodeReduceRoot();
         item->Move(current, action);
         item = current;
         item->GenerateTree(tagged, out);
         delete [] tmp;
         return;
      }
#else
      if (stacksize()>1) { WARNING("Parser failed.");return; }
#endif
      // generate nodes for out
      static int i,j;
      // first words
      for (i=0; i<tagged.size(); ++i) 
         out.newWord(tagged[i].first, tagged[i].second);
      // second constituents
      static const CStateNode* nodes[MAX_SENTENCE_SIZE*(2+UNARY_MOVES)+2];
      static int count;
      count = 0;
      const static CStateItem *current;
      current = this;
      while (current) {
         if (!current->IsTerminated() && current->node.valid()) {
            nodes[count] = &current->node;
            ++count; 
         }
         current = current->statePtr;
      }

      for (i=count-1; i>=0; --i) {
         j = out.newNode();
         // copy node
         assert(j==nodes[i]->id);
         nodes[i]->toCCFGTreeNode(out.nodes[j]);
         //iterate through nodes[i] Clink
        //printing the stanford dependencies
         const CLink* temp=nodes[i]->cLink;
         while(temp!=0) {
        	 //std::cout<<"i="<<i<<": "<<temp->label<<"("<<tagged.at(nodes[i]->lexical_head).first<<", "<<tagged.at(temp->dependent).first<<")\n"; //miguel
        	 std::cout<<"i="<<i<<": "<<temp->label<<"("<<tagged.at(temp->head).first<<", "<<tagged.at(temp->dependent).first<<")\n"; //miguel
        	 temp=temp->next;
         }
      }
      out.root = nodes[0]->id;
   }


   //===============================================================================

   void trace(const CTwoStringVector *s=0) const {
      static const CStateItem* states[MAX_SENTENCE_SIZE*(2+UNARY_MOVES)+2];
      static int count;
      const static CStateItem *current;
      count = 0;
      current = this;
      while (current->statePtr) {
         states[count] = current;
         ++count ; //updating
         current = current->statePtr;
      }
      #ifdef SCALE
      TRACE("State item score == " << score);
      TRACE("State item size == " << size);
      #endif
#ifdef TRAIN_LOSS
      TRACE("cor = " << correct_lb << ", plo = " << plost_lb << ", rlo = " << rlost_lb << ", Loss = " << FLoss());
#endif
      --count;
      while (count>=0) {
         if (s) {
            TRACE(states[count]->action.str()<<" ["<<(states[count]->stacksize()>0?s->at(states[count]->node.lexical_head).first:"")<<"]"); 
         }
         else {
            TRACE(states[count]->action.str());
         }
         --count;
      }
      TRACE("");
   }
   
   /*void generateStanford(bool head_left) {
   	   
   	   //TODO MIGUEL
   	   //this method should include the stanford dependencies from the left child and the right child
   	   
   	   //1st, create grammaticalstructure.
   	   //2nd. use semantic head finder.
   	   //3rd. add transformations.
   	   //if a head is not included in the subtree just forget about it... so far, so good. Speak with yue about this.
   	   generateStanfordLinks();
	   
   	   /*if (head_left){ 
   		   //the head is the left, therefore the right_child is the dependent.
   		   //find dependency label for this thing. We already have the arcs.
   		   updateStanfordLink(node.right_child->lexical_head);
   	   }
   	   else { 
   		   //the head is the right, therefore the left_child is the dependent.
   		   //find dependency label for this thing. We already have the arcs.
   		   updateStanfordLink(node.left_child->lexical_head);
   	   }*/
   	   
   	   
      //}
   
   //Miguel
   //this method generates the stanford links that are available for the current node.
   void generateStanfordLinks() {
	   
	   
	   //nsubj
	   //S < (NP=target $+ NP|ADJP) > VP
	   buildNsubj1(&this->node);
	   //SQ|PRN < (NP=target !< EX $++ VP)
	   buildNsubj2(&this->node);
	   //"S < ((NP|WHNP=target !< EX !<# (/^NN/ < (" + timeWordRegex + "))) $++ VP)"
	   //if (buildNsubj3(this->node))  return;
	   //"S < ( NP=target <# (/^NN/ < " + timeWordRegex + ") !$++ NP $++VP)",
	   //if (buildNsubj4(this->node))  return;
	   //if (buildNsubj5(this->node))  return;
	   //"SQ < ((NP=target !< EX) $- /^(?:VB|AUX)/ !$++ VP)",
	   //if (buildNsubj6(this->node))  return;
	   //"SQ < ((NP=target !< EX) $- (RB $- /^(?:VB|AUX)/) ![$++ VP])",
	   //if (buildNsubj7(this->node))  return;
	   //"SBARQ < WHNP=target < (SQ < (VP ![$-- NP]))",
	   buildNsubj8(&this->node);
	   //"SBARQ < (SQ=target < /^(?:VB|AUX)/ !< VP)",
	   //if (buildNsubj9(this->node))  return;
	   //"SINV < (NP|WHNP=target [ $- VP|VBZ|VBD|VBP|VB|MD|AUX | $- (@RB|ADVP $- VP|VBZ|VBD|VBP|VB|MD|AUX) | !$- __ !$ @NP] )",
	   //if (buildNsubj10(this->node))  return;
	   //"SBAR < WHNP=target [ < (S < (VP !$-- NP) !< SBAR) | < (VP !$-- NP) !< S ]"
	   buildNsubj11(&this->node);
	   //"SBAR !< WHNP < (S !< (NP $++ VP)) > (VP > (S $- WHNP=target))",
	   buildNsubj12(&this->node);
	   //"SQ < ((NP < EX) $++ NP=target)",
	   buildNsubj13(&this->node);
	   //"S < (NP < EX) <+(VP) (VP < NP=target)"
	   //if (buildNsubj14(this->node))  return;
	   
	   //Conj
	   //if (buildConj1(&this->node))  return;
	   //if (buildConj2(this->node))  return;
	   //if (buildConj3(this->node))  return;
	   //if (buildConj4(this->node))  return;
	   //if (buildConj5(this->node))  return;
	   
	   //Copula
	   //if (buildCopula1(&this->node))  return;
	   //if (buildCopula2(this->node))  return;
	   //if (buildCopula3(this->node))  return;
	   //if (buildCopula4(this->node))  return;
	   //if (buildCopula5(this->node))  return;
	   
	   //...
	   
   }
   
   
   
   /**
    * Miguel
    */
   /*void updateStanfordLink(const int dependent) {
	   //cl=new CDependencyLabel(0);
	   	   
	   //this->lexical_start (word).
	   //this->lexical_end (word)
	   	   
	   //start matching patterns. We know the dependent, and we can traverse it.
	   	   
	   CDependencyLabel* label;
	   if (true) {
		   //Nsubject
		   //S < (NP=target $+ NP|ADJP) > VP
		   //WE ALREADY HAVE THE DEPENDENT... so ??? what about pattern matching in which the target is the same one ...???
		   //words->at(node.lexical_head).word; this is the word
		   //words->at(node.lexical_head).tag; this is the pos
		   
		   label=new CDependencyLabel(STANFORD_DEP_NSUBJ);
	   }
	   //pattern matching
	   
	   CLink* newNode=new CLink(*label, dependent, 0);
	   	 	  
	   newNode->next=this->node.cLink;
	   node.cLink=newNode; //the new node (with the arc and label is added to the list)
	 
   }*/
   
   
   //===============================================================================
   /**
    * Miguel  (checked)
    * 
    * A < B (A inmediately dominates B) A: input, B: output
    */
   const CStateNode* inmediatelyDominates(const CStateNode* node, CConstituent constituent) {
	   if (node->type==CStateNode::LEAF) {
		   return 0;
	   }
	   else if(node->type==CStateNode::SINGLE_CHILD) {
		   if (node->left_child->constituent==constituent) {
			   return node->left_child;
		   }
		   else {
			   return 0;
		   }
	   }
	   else if (node->type==CStateNode::HEAD_LEFT) {
		   if (node->right_child->constituent==constituent) {
			   return node->right_child;
		   }
		   else if (node->left_child->temp) {
			   return inmediatelyDominates(node->left_child,constituent); //we have to do this because we have binarize structures. VP* for instance.
		   }
	   }
	   else {//if (node->type==CStateNode::HEAD_RIGHT) {
		   	assert(node->type=CStateNode::HEAD_RIGHT);
	   		if (node->left_child->constituent==constituent) {
 			   return node->left_child;
  		   }
  		   else if (node->right_child->temp) {
 			   return inmediatelyDominates(node->right_child,constituent); //we have to do this because we have binarize structures. VP* for instance.
 		   }
	   		   
  	   }
   }
   
   
   	 /**
       * Miguel  (checked)
       * 
       * A <# B.
       *  B (output) is the inmediate head of phrase A(input)
       */
      const CStateNode* inmediateLexHeadOf(const CStateNode* node, CConstituent constituent) {
    	  const CStateNode* b=inmediatelyDominates(node, constituent);
    	  if (b!=0) {
    		  if (b->lexical_head==node->lexical_head) { 
    			  return b;
    		  }
    	  }
    	  return 0;
      }
   
   /**
     * Miguel (seems fine)
     * 
     * A << B 
     * A (input) dominates B (output)
     */
   const CStateNode* dominates(const CStateNode* node, CConstituent constituent) {
   	   if (node->type==CStateNode::LEAF) { //it can't have children!
   		   return 0;
   	   }
   	   else if(node->type==CStateNode::SINGLE_CHILD) {
   		   if (node->left_child->constituent==constituent) {
   			   return node->left_child;
   		   }
   		   else {
   			   const CStateNode* aux=dominates(node->left_child,constituent);
   			   if (aux!=0) return aux;
   		   }
   	   }
   	   else 
   		{
   		   if (node->right_child->constituent==constituent) {
   		   return node->right_child;
   		   }
   		   else if (node->left_child->constituent==constituent) {
   			   return node->left_child;
   		   }
   		   else {
   			   const CStateNode* aux=dominates(node->left_child,constituent);
   			   if (aux!=0) return aux;
   			   aux=dominates(node->right_child,constituent);
   			   if (aux!=0) return aux;			   
   		   }
   		}
   	   return 0;
   	}
   
   /*
    * Miguel 
    * A $+ B
    * A (input) is the inmediate left sister of B (output)
    * 
    * 
    */
   const CStateNode* inmediateLeftSister(const CStateNode* parent, const CStateNode* inmLeftSister, CConstituent constituent) {
	   		if(parent->type==CStateNode::SINGLE_CHILD) {
	       		return 0;
	       	}
	   		else if (parent->right_child->constituent==constituent) {
	   			return parent->right_child;
	   		}
	   		else if (parent->right_child->temp) {
	   			const CStateNode* aux=parent->right_child;
	   			//we should follow the left childs
	   			//-----------------------------------
	   			//           Y
	   			//     LS             X*
	   			//               X         Z
	   			//-------------------------------
	   			//We want to find X
	   			while(aux->temp) {
	   				aux=aux->left_child;	
	   			}
	   			if (aux!=0 && aux->constituent==constituent) {
	   				return aux;
	   			}
	   		}
	   		else return 0;
   }
   
   /*
    * Miguel 
    * A $++ B
    * 
    * A (input) is a left sister of B (output)
    */
    const CStateNode* leftSister(const CStateNode* parent, const CStateNode* leftSister, CConstituent constituent) {
    		if(parent->type==CStateNode::SINGLE_CHILD) {
    			return 0;
    		}
    		else if (parent->right_child->constituent==constituent) {
    			   			return parent->right_child;
    		}
    		else return dominates(parent->right_child,constituent);
      }
    
    	/*
        * Miguel 
        * A $- B
        * A (input) is the inmediate right sister of B (output)
        * 
        * 
        */
       const CStateNode* inmediateRightSister(const CStateNode* parent, const CStateNode* inmRightSister, CConstituent constituent) {
    	   	if(parent->type==CStateNode::SINGLE_CHILD) {
    	   			return 0;
    	   	   	}
    	   		else if (parent->left_child->constituent==constituent) {
    	   	   		return parent->left_child;
    	   	   	}
    	   	   	else if (parent->left_child->temp) {
    	   	   		const CStateNode* aux=parent->left_child;
    	   	   		while(aux!=0 && aux->temp) {
    	   	   			aux=aux->right_child;	
    	   	   		}
    	   	   		if (aux!=0 && aux->constituent==constituent) {
    	   	   			return aux;
    	   	   		}
    	   	   	}
    	   	   	else return 0;
       }
       
       /*
        * Miguel 
        * A $-- B
        * This one must be correct.
        * A (input) is a right sister of B (output)
        */
        const CStateNode* rightSister(const CStateNode* parent, const CStateNode* rightSister, CConstituent constituent) {
        		if(parent->type==CStateNode::SINGLE_CHILD) {
        			return 0;
        		}
        		else if (parent->left_child->constituent==constituent) {
        			   			return parent->left_child;
        		}
        		else return dominates(parent->left_child,constituent);
          }
   
    //===============================================================================
    
   /*
    * Miguel: RULES FOR NSUBJ. There are 13
    * 
    * //S < (NP=target $+ NP|ADJP) > VP
    */
   bool buildNsubj1(const CStateNode* node) {
	   if (node->constituent==PENN_CON_VP) {
		   const CStateNode* s=inmediatelyDominates(node, PENN_CON_S);
		   if (s!=0) {
			   const CStateNode* np=inmediatelyDominates(s, PENN_CON_NP);
			   if (np!=0) {
				   const CStateNode* npadjp = inmediateLeftSister(s, np, PENN_CON_NP); //np is the inmediateleftsister of another np ?
				   if (npadjp==0) npadjp=inmediateLeftSister(s, np, PENN_CON_ADJP);  //np is the inmediateleftsister of a adjp ?
				   if (npadjp!=0) {
					   CDependencyLabel* label=new CDependencyLabel(STANFORD_DEP_NSUBJ);
					   buildStanfordLink(label, np->lexical_head, s->lexical_head);
					   //std::cout<<"nSubj1"<<" (head: "<<s->lexical_head<<")"<<"(dependent"<<np->lexical_head<<")\n"; //miguel
					   return true;
				   }
			   }
		   }
	   }
	   return false;
   }
   
   
   //SQ|PRN < (NP=target !< EX $++ VP)
   
   //I'm quite sure about this one. ASK YUE, though
   
   bool buildNsubj2(const CStateNode* node) {
      	   if ((node->constituent==PENN_CON_SQ)||(node->constituent==PENN_CON_PRN)) {
      		   const CStateNode* np=inmediatelyDominates(node, PENN_CON_NP);
      		   if (np!=0) {
      			   const CStateNode* ex=inmediatelyDominates(np, PENN_CON_EX); //is NP the inmediate child of SQ? is X, EX????????????
      			   if (ex==0) { //negation of <
      				   const CStateNode* vp = leftSister(node, np, PENN_CON_VP); //is the sister of the first np, that's for sure
      				   if (vp!=0) {
      					   CDependencyLabel* label=new CDependencyLabel(STANFORD_DEP_NSUBJ);
      					   buildStanfordLink(label, np->lexical_head, node->lexical_head);
      					   //std::cout<<"nSubj2"<<" (head: "<<node->lexical_head<<")"<<"(dependent"<<np->lexical_head<<")\n"; //miguel
      					   return true;
      				   }
      			   } 
      		   }
      	   }
      	   return false;
         }
   
   
   	   //"SBARQ < WHNP=target < (SQ < (VP ![$-- NP]))",
   	   // YUE: is this one the same as this one? Why did they put the brackets in this case. Any idea?
   	   //"SBARQ < WHNP=target < (SQ < (VP !$-- NP))", I did it, presuming that it is like this.
  
   	   bool buildNsubj8(const CStateNode* node) {
   		   if (node->constituent==PENN_CON_SBARQ) {
   			const CStateNode* whnp_target=inmediatelyDominates(node, PENN_CON_WHNP);
   			if (whnp_target!=0) {
   				const CStateNode* sq=inmediatelyDominates(node, PENN_CON_SQ);
   				if (sq!=0) {
   					const CStateNode* vp=inmediatelyDominates(sq, PENN_CON_VP);
   					if (vp!=0) {
   						const CStateNode* np=rightSister(sq, vp, PENN_CON_NP);
   						if (np==0) {//! negation
   							CDependencyLabel* label=new CDependencyLabel(STANFORD_DEP_NSUBJ);
   							buildStanfordLink(label, whnp_target->lexical_head, node->lexical_head);
   							//std::cout<<"nSubj8"<<" (head: "<<node->lexical_head<<")"<<"(dependent"<<whnp_target->lexical_head<<")\n"; //miguel
   							return true;
   						}
   					}
   				}
   			}
       }
   	   }
      
   	   //"SBAR < WHNP=target [ < (S < (VP !$-- NP) !< SBAR) | < (VP !$-- NP) !< S ]"
   	   //Let's try to put it into words
       //Relations can be grouped using brackets [ and ]. So the expression, NP [< NN | < NNS] & > S means that
       // (1) NP dominates either an NN or an NNS
       // (2) S dominates NP
   
       //So in our case of study.
       // 1. SBAR < WHNP=target
       // 2.1 SBAR < ((S < VP !$-- NP) !< SBAR)  OR
       // 2.2 SBAR < (VP !$-- NP) & !< S
      
   	   bool buildNsubj11(const CStateNode* node) {
   		   if (node->constituent==PENN_CON_SBAR) {
   			   const CStateNode* whnp_target=inmediatelyDominates(node, PENN_CON_WHNP);
   			   if (whnp_target!=0) {
   				   bool first=false; //2.1 SBAR < ((S < VP !$-- NP) !< SBAR) 
   				   const CStateNode* s=inmediatelyDominates(node, PENN_CON_S);
   				   if (s!=0) {
   					   const CStateNode* vp=inmediatelyDominates(s, PENN_CON_VP);
   					   if (vp!=0) {
   						   const CStateNode* np=rightSister(node, s, PENN_CON_NP); // (S < VP !$-- NP) S<VP &!$-- NP (so S is not the right sister of NP)
   						   if (np==0) { //negation of !$--
   							   const CStateNode* sbar=inmediatelyDominates(s, PENN_CON_SBAR);
   							   if (sbar==0){
   								   first=true;
   							   }
   						   }
   						   
   					   }
   				   }
   				   
   				   bool second=false; //2.2 SBAR < (VP !$-- NP) & !< S
   				   const CStateNode* s2=inmediatelyDominates(node, PENN_CON_S);
   				   if (s2==0) { //!< negation
   					   const CStateNode* vp=inmediatelyDominates(node, PENN_CON_VP);
   					   if (vp!=0) {
   						   const CStateNode* np=rightSister(node, vp, PENN_CON_NP);
   						   if (np==0) {
   							   second=true;
   						   }
   					   }
   				   }
   				   
   				   
   				   if (first || second) {
   					   CDependencyLabel* label=new CDependencyLabel(STANFORD_DEP_NSUBJ);
   					   buildStanfordLink(label, whnp_target->lexical_head, node->lexical_head); //dependent, head
   					   std::cout<<"nSubj8"<<" (head: "<<node->lexical_head<<")"<<"(dependent"<<whnp_target->lexical_head<<")\n"; //miguel
   					   return true;
   				   }
   			   }
   		   }
   	   }
   
   	
   
   		//"SBAR !< WHNP < (S !< (NP $++ VP)) > (VP > (S $- WHNP=target))",
   		//Simplified version
   		// SBAR !< WHNP & < (S !< (NP $++ VP)) & > (VP > (S $- WHNP=target))
        //which means that SBAR is not the inmediate head of WHNP but it is the inmediate head of (S !< (NP $++ VP)), and it also means that 
        //SBAR is the inmediate child of the following structure (VP > (S $- WHNP=target)) which is headed by S.
        bool buildNsubj12(const CStateNode* node) {
        	
        	//little trick in this one. I have to start with the inmediate head of the rightmost S (in the rule)... but I don't care about its constituent
        	//S must have a head, because otherwise it cannot be the inmediate right sister of WHNP
        	 const CStateNode* s=inmediatelyDominates(node, PENN_CON_S);
        	   if (s->constituent==PENN_CON_S) {
        		   const CStateNode* whnp_targ=inmediateRightSister(node, s, PENN_CON_WHNP); 
        		   if (whnp_targ!=0) {
        			   const CStateNode* vp2=inmediatelyDominates(s, PENN_CON_VP);
        			   if (vp2!=0) {
        				   const CStateNode* sbar_head=inmediatelyDominates(s, PENN_CON_VP);
        				   if (sbar_head!=0) {
        					   const CStateNode* whnp=inmediatelyDominates(sbar_head, PENN_CON_WHNP);
        					   if (whnp==0) {
        						   const CStateNode* s1=inmediatelyDominates(sbar_head, PENN_CON_S);
        						   if (s1!=0) {
        							   bool success=true;
        							   const CStateNode* np=inmediatelyDominates(s1, PENN_CON_NP);
        							   if (np!=0) {
        								   const CStateNode* vp=leftSister(s1, np, PENN_CON_VP);
        								   if (vp!=0){
        									   success=false;
        								   }
        							   }
        							   if (success) {
        								   CDependencyLabel* label=new CDependencyLabel(STANFORD_DEP_NSUBJ);
        								   buildStanfordLink(label, whnp_targ->lexical_head, sbar_head->lexical_head); //dependent, head
        								   //std::cout<<"nSubj12"<<" (head: "<<sbar_head->lexical_head<<")"<<"(dependent"<<whnp_targ->lexical_head<<")\n"; //miguel
        								   return true;
        							   }
        						   }
        					   }
        				   }
        			   }
        		   }
        		   
        	   	}
        	    return false;
         }
   
   
        //"SQ < ((NP < EX) $++ NP=target)",
      //Doubts: not sure whether the order is fine or not. See comments below.
      
      bool buildNsubj13(const CStateNode* node) {
      	   if (node->constituent==PENN_CON_SQ) {
      		   const CStateNode* np=inmediatelyDominates(node, PENN_CON_NP); //is NP the inmediate child of SQ?
      		   if (np!=0) {
      			   const CStateNode* ex = inmediatelyDominates(np, PENN_CON_EX); //is the child of the first np? IS IT EX???????
      			   if (ex!=0) {
      				   const CStateNode* np2=leftSister(node,np,PENN_CON_NP); //is the sister of np or the sister of ex. I understand that given the parenthesis it is like this,
      				   if (np2!=0) {
      					   CDependencyLabel* label=new CDependencyLabel(STANFORD_DEP_NSUBJ);
      					   buildStanfordLink(label, np2->lexical_head, node->lexical_head);
      					   //std::cout<<"nSubj13"<<" (head: "<<node->lexical_head<<")"<<"(dependent"<<np2->lexical_head<<")\n"; //miguel
      					   return true;
      				   }
      			   }
      		   } 
      	   	}
      	    return false;
       }
      
     //===============================================================================
      
      //aux rules: there are 4
      
      //"VP < VP < /^(?:TO|MD|VB.*|AUXG?|POS)$/=target",
      //"SQ|SINV < (/^(?:VB|MD|AUX)/=target $++ /^(?:VP|ADJP)/)",
      //"CONJP < TO=target < VB", // (CONJP not to mention)
                // add handling of tricky VP fronting cases...
      //"SINV < (VP=target < (/^(?:VB|AUX|POS)/ < " + beAuxiliaryRegex + ") $-- (VP < VBG))",
      
      
      //"CONJP < TO=target < VB"
      
      //YUE: neither VB nor TO were in the penn.h. I added them. We are in 31 if I am not mistaken
      
      bool buildConj3(const CStateNode* node) {
    	  if (node->constituent==PENN_CON_CONJP) {
    		  const CStateNode* to=inmediatelyDominates(node, PENN_CON_TO); 
    		  if (to!=0) {
    			  const CStateNode* vb=inmediatelyDominates(node, PENN_CON_VB);
    			  if (vb!=0){
    				  CDependencyLabel* label=new CDependencyLabel(STANFORD_DEP_AUX);
    				  buildStanfordLink(label, to->lexical_head, node->lexical_head);
    				  //std::cout<<"nSubj13"<<" (head: "<<node->lexical_head<<")"<<"(dependent"<<np2->lexical_head<<")\n"; //miguel
    				  return true;
    			  }
    		  }
    	  }
    	  return false;
      }
            
      
   
    //===============================================================================
      
      //conj rules. There are 
      //(stf comment) this is more ugly, but the first 3 patterns are now duplicated and for clausal things, that daughter to the left of the CC/CONJP can't be a PP or RB or ADVP either
      //(stf comment) non-parenthetical or comma in suitable phrase with conjunction to left
      //"VP|S|SBAR|SBARQ|SINV|SQ < (CC|CONJP $-- !/^(?:``|-LRB-|PRN|PP|ADVP|RB)/ $+ !/^(?:PRN|``|''|-[LR]RB-|,|:|\\.)$/=target)",
      // non-parenthetical or comma in suitable phrase with conj then adverb to left
      //"VP|S|SBAR|SBARQ|SINV|SQ < (CC|CONJP $-- !/^(?:``|-LRB-|PRN|PP|ADVP|RB)/ $+ (ADVP $+ !/^(?:PRN|``|''|-[LR]RB-|,|:|\\.)$/=target))",
      // content phrase to the right of a comma or a parenthetical
      // The test at the end is to make sure that a conjunction or
      // comma etc actually show up between the target of the conj
      // dependency and the head of the phrase.  Otherwise, a
      // different relationship is probably more appropriate.
      //"VP|S|SBAR|SBARQ|SINV|SQ=root < (CC|CONJP $-- !/^(?:``|-LRB-|PRN|PP|ADVP|RB)/) < (/^(?:PRN|``|''|-[LR]RB-|,|:|\\.)$/ $+ (/^S|SINV$|^(?:A|N|V|PP|PRP|J|W|R)/=target $-- (/^CC|CONJP|:|,$/ $-- (__ ># =root))) )",
      // non-parenthetical or comma in suitable phrase with conjunction to left
      //"/^(?:ADJP|JJP|PP|QP|(?:WH)?NP(?:-TMP|-ADV)?|ADVP|UCP(?:-TMP|-ADV)?|NX|NML)$/ < (CC|CONJP $-- !/^(?:``|-LRB-|PRN)$/ $+ !/^(?:PRN|``|''|-[LR]RB-|,|:|\\.)$/=target)",
      // non-parenthetical or comma in suitable phrase with conj then adverb to left
      //"/^(?:ADJP|PP|(?:WH)?NP(?:-TMP|-ADV)?|ADVP|UCP(?:-TMP|-ADV)?|NX|NML)$/ < (CC|CONJP $-- !/^(?:``|-LRB-|PRN)$/ $+ (ADVP $+ !/^(?:PRN|``|''|-[LR]RB-|,|:|\\.)$/=target))",
      // content phrase to the right of a comma or a parenthetical
      //"/^(?:ADJP|PP|(?:WH)?NP(?:-TMP|-ADV)?|ADVP|UCP(?:-TMP|-ADV)?|NX|NML)$/ < (CC|CONJP $-- !/^(?:``|-LRB-|PRN)$/) < (/^(?:PRN|``|''|-[LR]RB-|,|:|\\.)$/ $+ /^S|SINV$|^(?:A|N|V|PP|PRP|J|W|R)/=target)",
                // content phrase to the left of a comma for at least NX
      //"NX|NML < (CC|CONJP $- __) < (/^,$/ $- /^(?:A|N|V|PP|PRP|J|W|R|S)/=target)",
                // to take the conjunct in a preconjunct structure "either X or Y"
      //"/^(?:VP|S|SBAR|SBARQ|SINV|ADJP|PP|QP|(?:WH)?NP(?:-TMP|-ADV)?|ADVP|UCP(?:-TMP|-ADV)?|NX|NML)$/ < (CC $++ (CC|CONJP $+ !/^(?:PRN|``|''|-[LR]RB-|,|:|\\.)$/=target))",
      
      
      
    //===============================================================================
      
      //cc coordination. THere is one rule.
      
      //"__ [ < (CC=target !< /^(?i:either|neither|both)$/ ) | < (CONJP=target !< (RB < /^(?i:not)$/ $+ (RB|JJ < /^(?i:only|just|merely)$/))) ]"
      
     //===============================================================================
      
      //punct punctuation. There are 2 rules.
      
      //"__ < /^(?:\\.|:|,|''|``|\\*|-LRB-|-RRB-|HYPH)$/=target",
      //"__ < (NFP=target !< /^(?:[<>]?[:;=8][\\-o\\*']?(?:-RRB-|-LRB-|[DPdpO\\/\\\\\\:}{@\\|\\[\\]])|(?:-RRB-|-LRB-|[DPdpO\\/\\\\\\:}{@\\|\\[\\]])[\\-o\\*']?[:;=8][<>]?)$/"
      //              + "!< /^(?:-LRB-)?[\\-\\^x=~<>'][_.]?[\\-\\^x=~<>'](?:-RRB-)?$/)"
      
      
      //===============================================================================
      
      //nsubjpass. THere is one rule.
      
      //"S|SQ < (WHNP|NP=target !< EX) < (VP < (/^(?:VB|AUX)/ < " + passiveAuxWordRegex + ")  < (VP < VBN|VBD))",
      //===============================================================================
      
      //csubj. Clausal subject. there is one rule.
      
      //"S < (SBAR|S=target !$+ /^,$/ $++ (VP !$-- NP))"
      
      //===============================================================================
      //csubjpass. There are 2 rules.
      
      //"S < (SBAR|S=target !$+ /^,$/ $++ (VP < (VP < VBN|VBD) < (/^(?:VB|AUXG?)/ < " + passiveAuxWordRegex + ") !$-- NP))",
      //"S < (SBAR|S=target !$+ /^,$/ $++ (VP <+(VP) (VP < VBN|VBD > (VP < (/^(?:VB|AUX)/ < " + passiveAuxWordRegex + "))) !$-- NP))"
      
      //===============================================================================
      //dobj, direct object. There are 8 rules.
      
      //"VP < (NP $+ (NP|WHNP=target !< (/^NN/ < " + timeWordLotRegex + "))) !<(/^(?:VB|AUX)/ < " + copularWordRegex + ")",  // this time one also included "lot"
      // match "give it next week"
      //"VP < (NP < (NP $+ (/^(NP|WHNP)$/=target !< (/^NN/ < " + timeWordLotRegex + "))))!< (/^(?:VB|AUX)/ < " + copularWordRegex + ")",  // this time one also included "lot"
      // basic direct object cases
      //"VP !<(/^(?:VB|AUX)/ < " + copularWordRegex + ") < (NP|WHNP=target [ [ !<# (/^NN/ < " + timeWordRegex + ") !$+ NP ] | $+ NP-TMP | $+ (NP <# (/^NN/ < " + timeWordRegex + ")) ] )",
      // cdm Jul 2010: No longer require WHNP as first child of SBARQ below: often not because of adverbials, quotes, etc., and removing restriction does no harm
      // this next pattern used to assume no empty NPs. Corrected.  If you adjust this pattern, also adjust the corresponding one for attr!
      //"SBARQ < (WHNP=target !< WRB !<# (/^NN/ < " + timeWordRegex + ")) <+(SQ|SINV|S|VP) (VP !< NP|TO !< (S < (VP < TO)) !< (/^(?:VB|AUX)/ < " + copularWordRegex + " $++ (VP < VBN|VBD)) !<- PRT !<- (PP <: IN) $-- (NP !< /^-NONE-$/))",
      // matches direct object in relative clauses "I saw the book that you bought"
      //"SBAR < (WHNP=target !< WRB) < (S < NP < (VP !< SBAR !<+(VP) (PP <- IN) !< (S < (VP < TO))))",
      // matches direct object in relative clauses "I saw the book that you said you bought"
      //"SBAR !< WHNP|WHADVP < (S < (@NP $++ (VP !$++ NP))) > (VP > (S < NP $- WHNP=target))",
      // matches direct object for long dependencies in relative clause without explicit relative pronouns
      //"SBAR !< (WHPP|WHNP|WHADVP) < (S < (@NP $+ (VP !< (/^(?:VB|AUX)/ < " + copularWordRegex + " !$+ VP)  !<+(VP) (/^(?:VB|AUX)/ < " + copularWordRegex + " $+ (VP < VBN|VBD)) !<+(VP) NP !< SBAR !<+(VP) (PP <- IN)))) !$-- CC $-- NP > NP=target",
      //"SBAR !< (WHPP|WHNP|WHADVP) < (S < (@NP $+ (ADVP $+ (VP !< (/^(?:VB|AUX)/ < " + copularWordRegex + " !$+ VP) !<+(VP) (/^(?:VB|AUX)/ < " + copularWordRegex + " $+ (VP < VBN|VBD)) !<+(VP) NP !< SBAR !<+(VP) (PP <- IN))))) !$-- CC $-- NP > NP=target"      
      //===============================================================================
      //iobj, indirect object. There are 2 rules.
      
      //"VP < (NP=target !< /\\$/ !<# (/^NN/ < " + timeWordRegex + ") $+ (NP !<# (/^NN/ < " + timeWordRegex + ")))",
      // this next one was meant to fix common mistakes of our parser, but is perhaps too dangerous to keep
      //"VP < (NP=target < (NP !< /\\$/ $++ (NP !< (/^NN/ < " + timeWordLotRegex + ")) !$ CC|CONJP !$ /^,$/ !$++ /^:$/))",
      
      //===============================================================================
      //pobj, prepositional object. There are 8 rules.
      
      //"/^(?:PP(?:-TMP)?|(?:WH)?(?:PP|ADVP))$/ < (IN|VBG|TO|FW|RB|RBR $++ (/^(?:WH)?(?:NP|ADJP)(?:-TMP|-ADV)?$/=target !$- @NP))",
      // We allow ADVP with NP objects for cases like (ADVP earlier this year)
      //"/^PP(?:-TMP)?$/ < (/^(?:IN|VBG|TO)$/ $+ (ADVP=target [ < (RB < /^(?i:here|there)$/) | < (ADVP < /^NP(?:-TMP)?$/) ] ))",
      // second disjunct is weird ADVP, only matches 1 tree in 2-21
      // to deal with preposition stranding in questions (e.g., "Which city do you live in?") -- the preposition is sometimes treated as a particle by the parser (works well but doesn't preserve the tree structure!)
      //"PRT >- (VP !< (S < (VP < TO)) >+(SQ|SINV|S|VP) (SBARQ <, (WHNP=target !< WRB)) $-- (NP !< /^-NONE-$/))",
      //"(PP <: IN|TO) >- (VP !< (S < (VP < TO)) >+(SQ|SINV|S|VP) (SBARQ <, (WHNP=target !< WRB)) $-- (NP !< /^-NONE-$/))",
      //"(PP <: IN|TO) $- (NP $-- (VBZ|VBD) !$++ VP) >+(SQ) (SBARQ <, (WHNP=target !< WRB)) $-- (NP !< /^-NONE-$/)",
      // to deal with preposition stranding in relative clause (works well but doesn't preserve the tree structure!)
      //"(PP <- IN|TO) >+(@VP|S|SINV|SBAR) (SBAR !< (WHPP|WHNP) < (S < (NP $+ (VP !<(/^(?:VB|AUX)/ < " + copularWordRegex + " !$+ VP) !<+(VP) NP !< SBAR ))) $-- NP > NP=target)",
      //"XS|ADVP < (IN < /^(?i:at)$/) < JJS|DT=target", // at least, at most, at best, at worst, at all
      //"PP < (CC < less) < NP"
      
      //===============================================================================
      //pcomp, prepositional complement. There are 3 rules.
      
      //"@PP|WHPP < (IN|VBG|VBN|TO $+ @SBAR|S|PP|ADVP=target)", // no intervening NP; VBN is for "compared with"
      //"@PP|WHPP < (RB $+ @SBAR|S=target)", // RB is for weird tagging like "after/RB adjusting for inflation"
      //"@PP|WHPP !< IN|TO < (SBAR=target <, (IN $+ S))",
      
      
      //===============================================================================
      //attr attributive. There are 4 rules.
      
      //"VP < NP=target <(/^(?:VB|AUX)/ < " + copularWordRegex + ") !$ (NP < EX)",
      // "What is that?"
      //"SBARQ < (WHNP|WHADJP=target $+ (SQ < (/^(?:VB|AUX)/ < " + copularWordRegex + " !$++ VP) !< (VP <- (PP <:IN)) !<- (PP <: IN)))",
      //"SBARQ <, (WHNP|WHADJP=target !< WRB) <+(SQ|SINV|S|VP) (VP !< (S < (VP < TO)) < (/^(?:VB|AUX)/ < " + copularWordRegex + " $++ (VP < VBN|VBD)) !<- PRT !<- (PP <: IN) $-- (NP !< /^-NONE-$/))",
      //"Is he the man?"
      //"SQ <, (/^(?:VB|AUX)/ < " + copularWordRegex + ") < (NP=target $-- (NP !< EX))"
      
      //===============================================================================
      //ccomp clausal complement
      
      //"VP < (S=target < (VP !<, TO|VBG|VBN) !$-- NP)",
      //"VP < (SBAR=target < (S <+(S) VP) <, (IN|DT < /^(?i:that|whether)$/))",
      //"VP < (SBAR=target < (SBAR < (S <+(S) VP) <, (IN|DT < /^(?i:that|whether)$/)) < CC|CONJP)",
      //"VP < (SBAR=target < (S < VP) !$-- NP !<, (IN|WHADVP))",
      //"VP < (SBAR=target < (S < VP) !$-- NP <, (WHADVP < (WRB < /^(?i:how)$/)))",
      //"VP < (/^VB/ < " + haveRegex + ") < (S=target < @NP < VP)",
      //// to find "...", he said or "...?" he asked.
      //"S|SINV < (S|SBARQ=target $+ /^(,|\\.|'')$/ !$- /^(?:CC|:)$/ !< (VP < TO|VBG|VBN))",
      //"ADVP < (SBAR=target < (IN < /^(?i:as|that)/) < (S < (VP !< TO)))", // ADVP is things like "As long as they spend ..."
      //"ADJP < (SBAR=target !< (IN < as) < S)", // ADJP is things like "sure (that) he'll lose" or for/to ones or object of comparison with than "than we were led to expect"; Leave aside as in "as clever as we thought.
      // That ... he know
      //"S <, (SBAR=target <, (IN < /^(?i:that|whether)$/) !$+ VP)",
      // JJ catches a couple of funny NPs with heads like "enough"
      // Note that we eliminate SBAR which also match an infmod pattern
      //"@NP < JJ|NN|NNS < (SBAR=target [ !<(S < (VP < TO )) | !$-- NP|NN|NNP|NNS ] )"
                
      //===============================================================================
      //xcomp xclausal complement
      
      //"VP < (S=target !$- (NN < order) < (VP < TO))",    // used to have !> (VP < (VB|AUX < be))
      //"ADJP < (S=target <, (VP <, TO))",
      //"VP < (S=target !$- (NN < order) < (NP $+ NP|ADJP))",
      // to find "help sustain ...
      //"VP < (/^(?:VB|AUX)/ $+ (VP=target < VB < NP))",
      //"VP < (SBAR=target < (S !$- (NN < order) < (VP < TO))) !> (VP < (VB|AUX < be)) ",
      //"VP < (S=target !$- (NN < order) <: NP) > VP",
      // stop eating
      // note that we eliminate parentheticals and clauses that could match a partmod
      //"(VP < (S=target < (VP < VBG ) !< NP !$- (/^,$/ [$- @NP  |$- (@PP $-- @NP ) |$- (@ADVP $-- @NP)]) !$-- /^:$/))",
      
      //===============================================================================
      //rel, relative
      
      //"SBAR <, WHNP|WHPP|WHADJP=target > /^NP/ [ !<, /^WHNP/ | < (S < (VP $-- (/^NP/ !< /^-NONE-$/)))]"
      
      //===============================================================================
      
      //expl, expletitive
      
      //"S|SQ|SINV < (NP=target <+(NP) EX)"      
      
      //===============================================================================
      
      //acomp adjectival complement
      
      //"VP [ < (ADJP=target !$-- NP) |  < (/^VB/ $+ (@S=target < (@ADJP < /^JJ/ ! $-- @NP|S))) ]",
      
      //===============================================================================
      
      //advcl, adverbial clause modifier
      
      //"VP < (@SBAR=target [ < (IN !< /^(?i:that|whether)$/) | <: (SINV <1 /^(?:VB|MD|AUX)/) | < (IN < that) < (RB|IN < so) ] )",
      //"S|SQ|SINV < (SBAR|SBAR-TMP=target <, (IN !< /^(?i:that|whether)$/ !$+ (NN < order)) !$-- /^(?!CC|CONJP|``|,|INTJ|PP(-.*)?).*$/ !$+ VP)",
      // to get "rather than"
      // "S|SQ|SINV < (SBAR|SBAR-TMP=target <2 (IN !< /^(?i:that|whether)$/ !$+ (NN < order)) !$-- /^(?!CC|CONJP|``|,|INTJ|PP(-.*)?$).*$/)",
      // "S|SQ|SINV < (SBAR|SBAR-TMP=target <, (IN !< /^(?i:that|whether)$/ !$+ (NN < order)) !$+ @VP $+ /^,$/ $++ @NP)", // this one might just be better, but at any rate license one with quotation marks or a conjunction beforehand
      // "SBARQ < (SBAR|SBAR-TMP|SBAR-ADV=target <, (IN !< /^(?i:that|whether)$/ !$+ (NN < order)) $+ /^,$/ $++ @SQ|S|SBARQ)", // the last part should probably only be @SQ, but this captures some strays at no cost
      // "VP < (SBAR|SBAR-TMP=target <, (WHADVP|WHNP < (WRB !< /^(?i:how)$/)) !< (S < (VP < TO)))", // added the (S < (VP <TO)) part so that "I tell them how to do so" doesn't get a wrong advcl
      // "S|SQ < (SBAR|SBAR-TMP=target <, (WHADVP|WHNP < (WRB !< /^(?i:how)$/)) !< (S < (VP < TO)))",
      
      //"@S < (@SBAR=target $++ @NP $++ @VP)",  // fronted adverbial clause
      //"@S < (@S=target < (VP < TO) $+ (/^,$/ $++ @NP))"
      
      
      
      
      //===============================================================================
      //"rcmod", "relative clause modifier",
      
      //"NP|WHNP|NML $++ (SBAR=target <+(SBAR) WHPP|WHNP) !$-- NP|WHNP|NML > @NP|WHNP",
      //"NP|WHNP|NML $++ (SBAR=target <: (S !<, (VP <, TO))) !$-- NP|WHNP|NLP > @NP|WHNP",
      // this next pattern is restricted to where and why because
      //"when" is usually incorrectly parsed: temporal clauses
      // are put inside the NP; 2nd is for case of relative
      // clauses with no relativizer (it doesn't distinguish
      // whether actually gapped).
      //"NP|NML $++ (SBAR=target < (WHADVP < (WRB </^(?i:where|why|when)/))) !$-- NP|NML > @NP",
      //"NP|WHNP|NML $++ RRC=target !$-- NP|WHNP|NML",
      //"@ADVP < (@ADVP < (RB < /where$/)) < @SBAR=target",
      
      //===============================================================================
      //mark marker.
      
      //"SBAR|SBAR-TMP < (IN|DT=target $++ S|FRAG)",
      //"SBAR < (IN|DT=target < that|whether) [ $-- /^(?:VB|AUX)/ | $- NP|NN|NNS | > ADJP|PP | > (@NP|UCP|SBAR < CC|CONJP $-- /^(?:VB|AUX)/) ]",
      
      //===============================================================================
      
      //amod, adjectival modifier
      
      // "/^(?:NP(?:-TMP|-ADV)?|NX|NML|NAC|WHNP)$/ < (ADJP|WHADJP|JJ|JJR|JJS|JJP|VBN|VBG|VBD|IN=target !< QP !$- CC)",
      // IN above is needed for "next" in "next week" etc., which is often tagged IN.
      //"ADJP !< CC|CONJP < (JJ|NNP $ JJ|NNP=target)",
      // Cover the case of "John, 34, works at Stanford" - similar to an expression for appos
      //"WHNP|WHNP-TMP|WHNP-ADV|NP|NP-TMP|NP-ADV < (NP=target <: CD $- /^,$/ $-- /^(?:WH)?NP/ !$ CC|CONJP)",
      
      
      
      //===============================================================================
      
      
      
   void buildStanfordLink(CDependencyLabel* label, int dependent, int head) {
	   if (head==dependent) return;
   	   CLink* newNode=new CLink(*label, dependent, head, 0);
   	   newNode->next=this->node.cLink;
   	   node.cLink=newNode; //the new node (with the arc and label is added to the list)
      }

   //===============================================================================
#ifdef TRAIN_LOSS   
public:
   SCORE_TYPE actionLoss(const CAction &action, unsigned &correct, unsigned &plost, unsigned &rlost) const {
      static unsigned long constituent;
      if (action.isShift()) {
         computeShiftLB(0, correct, plost, rlost);
      }
      else if (action.isReduceRoot()) {
         computeTerminateLoss(0, correct, plost, rlost);
      }
      else if (action.isIdle()) {
         computeIdleLoss(0, correct, plost, rlost);
      }
      else if (action.singleChild()) {
         computeReduceUnaryLB(0, correct, plost, rlost, action.getConstituent());
      }
      else {
#ifdef NO_TEMP_CONSTITUENT
         constituent = action.getConstituent();
#else
         constituent = CConstituent::encodeTmp(action.getConstituent(), action.isTemporary());
#endif
         computeReduceBinaryLB(0, correct, plost, rlost, constituent);
      }
   }
   SCORE_TYPE actionFLoss(const CAction &action) const {
      static unsigned correct, plost, rlost;
      actionLoss(action, correct, plost, rlost);
      return FLoss(correct, plost, rlost);
   }
   SCORE_TYPE FLoss(const unsigned &correct, const unsigned &plost, const unsigned &rlost) const {
      static SCORE_TYPE p, r, f;
      if (correct == 0) {
         if (plost == 0 && rlost == 0) {
            return static_cast<SCORE_TYPE>(0);
         }
         else {
            return static_cast<SCORE_TYPE>(1);
         }
      }
      p = static_cast<SCORE_TYPE>(correct) / (correct + plost);
      r = static_cast<SCORE_TYPE>(correct) / (correct + rlost);
      f = 2*static_cast<SCORE_TYPE>(p)*r / (p + r);
      return 1-f;
   }
   SCORE_TYPE FLoss() const {
      return FLoss(correct_lb, plost_lb, rlost_lb);
   }
   SCORE_TYPE HammingLoss() const {
      return plost_lb + rlost_lb;
   }
   SCORE_TYPE stepHammingLoss() const {
      if (statePtr == 0)
         return 0;
      return plost_lb-statePtr->plost_lb + rlost_lb-statePtr->rlost_lb;
   }
   SCORE_TYPE actionHammingLoss(const CAction &action) const {
      static unsigned correct, plost, rlost;
      actionLoss(action, correct, plost, rlost);
      return plost + rlost;
   }
   SCORE_TYPE actionStepHammingLoss(const CAction &action) const {
      static unsigned correct, plost, rlost;
      actionLoss(action, correct, plost, rlost);
      return plost-plost_lb + rlost-rlost_lb;
   }
#endif
   
   
   
   
   
   
};

/*===============================================================
 *
 * scored actions
 *
 *==============================================================*/

class CScoredStateAction {

public:
   CAction action;
   const CStateItem *item;
   SCORE_TYPE score;

public:
   CScoredStateAction() : item(0), action(), score(0) {}
   void load(const CAction &action, const CStateItem *item, const SCORE_TYPE &score) {
      SCORE_TYPE item_sc;
      this->action = action; 
      this->item = item;
      item_sc = item->score;
#ifdef SCALE
      item_sc *= item->size;
#endif
#ifdef TRAIN_LOSS
#define LOSS_ADD + item->actionStepHammingLoss(action)
//#define LOSS_ADD -std::sqrt(item->HammingLoss()) + std::sqrt(item->actionHammingLoss(action))
#else
#define LOSS_ADD 
#endif
      this->score = item_sc + score LOSS_ADD;
#ifdef SCALE
      this->score /= (item->size + 1);
#endif
   }
   

public:
   bool operator < (const CScoredStateAction &a1) const { return score < a1.score; }
   bool operator > (const CScoredStateAction &a1) const { return score > a1.score; }
   bool operator <= (const CScoredStateAction &a1) const { return score <= a1.score; }
   bool operator >= (const CScoredStateAction &a1) const { return score >= a1.score; }

};

#endif

