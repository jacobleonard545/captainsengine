#ifndef AST_H
#define AST_H

#include <string>
#include <vector>
#include <system/engine.h>

class ASTNode{
public:
  enum Type{
    UNDEFINED,
    IDENTIFIER,
    INTEGER,
    REALNUM,
    FUNCCALL,
    EVENT,
    CONDITION,
    VARIABLE,
    RELATIONAL,
    LEVELDEF,
    ROWDEF,
    TIMERFUNC,
    ARITHMETIC,
    UNARY_ARITH,
    CONCATENATION,
  };
  ASTNode(Type t) : mType(t) {}
  virtual ~ASTNode(){}
  Type getType() {return mType;}
protected:
  Type mType;
};

class NodeList{
public:
  NodeList() : mIdx(0){}
  virtual ~NodeList(){
    for (unsigned i = 0; i < mNodes.size(); ++i){
      delete mNodes[i];
    }
  }
  void addNode(ASTNode* node){
    if (node != NULL)
      mNodes.push_back(node);
  }
  void addNodes(NodeList* list){
    for (size_t i = 0; i < list->mNodes.size(); ++i){
      mNodes.push_back(list->mNodes[i]);
    }
    list->mNodes.clear();
  }
  void reset(bool start) {
    if (start)
      mIdx = 0;
    else
      mIdx = (unsigned)mNodes.size();
  }
  bool hasNext() {return mIdx < mNodes.size();}
  bool hasPrev() {return mIdx > 0;}
  ASTNode* next() {
    if(hasNext())
      return mNodes[mIdx++];
    return NULL;
  }
  ASTNode* prev(){
    if (hasPrev())
      return mNodes[--mIdx];
    return NULL;
  };
  unsigned size(){
    return (unsigned)mNodes.size();
  }
  ASTNode* first(){
    if (mNodes.size() == 0)
      return NULL;
    return mNodes[0];
  }
  bool removeNotOfType(ASTNode::Type type){
    bool changed = false;
    for (std::vector<ASTNode*>::iterator iter = mNodes.begin(); iter != mNodes.end(); ){
      if ((*iter)->getType() != type){
        delete *iter;
        iter = mNodes.erase(iter);
        changed = true;
      }
      else
        ++iter;
    }
    return changed;
  }
protected:
  std::vector<ASTNode*> mNodes;
  unsigned mIdx;
};

class StmtNode : public ASTNode{
public:
  StmtNode(Type t) : ASTNode(t){}
  virtual ~StmtNode(){}
};

class IdentNode : public ASTNode{
public:
  IdentNode(const std::string& value) : ASTNode(IDENTIFIER) {append(value.c_str());}
  virtual ~IdentNode(){}
  void append(const char* str);
  std::string& value() {return mValue;}
  char lastChar() {if (mValue.empty()) return '\0'; return mValue[mValue.size()-1];}
protected:
  std::string mValue;
};

class IntNode : public ASTNode{
public:
  IntNode(int value) : ASTNode(INTEGER) {
    mValue = value;
  }
  virtual ~IntNode(){}
  int& value() {return mValue;}
protected:
  int mValue;
};

class RealNode : public ASTNode{
public:
  RealNode(float value) : ASTNode(REALNUM) {
    mValue = value;
  }
  virtual ~RealNode(){}
  float& value() {return mValue;}
protected:
  float mValue;
};

class FuncCallNode : public StmtNode{
public:
  FuncCallNode(std::string func, NodeList* args) : StmtNode(FUNCCALL) {
    mFunc = func;
    toLower(mFunc);
    mArgs = args;
  }
  ~FuncCallNode(){
    delete mArgs;
  }
  std::string getName() {return mFunc;}
  NodeList* getArguments() {return mArgs;}
protected:
  std::string mFunc;
  NodeList* mArgs;
};

class EventNode : public StmtNode{
public:
  EventNode(const std::string& event, NodeList* eventblock) : StmtNode(EVENT){
    mEvents.push_back(event);
    mBlock = eventblock;
  }
  ~EventNode(){
    delete mBlock;
  }
  void addEvent(const std::string& event){
    mEvents.push_back(event);
  }
  void setEventBlock(NodeList* block) { mBlock = block; }
  std::string getEvent(unsigned idx) {return mEvents[idx];}
  unsigned getNumEvents() { return (unsigned)mEvents.size(); }
  NodeList* getBlock() {return mBlock;}
protected:
  std::vector<std::string> mEvents;
  NodeList* mBlock;
};

class CondNode : public StmtNode{
public:
  CondNode(bool negation) : StmtNode(CONDITION), mNegation(negation), mArgs(NULL), mIfBlock(NULL){
  }
  ~CondNode(){
    delete mArgs;
    delete mIfBlock;
  }
  void setCondition(const std::string& condition) {mCondition = condition;}
  void setCondArgs(NodeList* args) {mArgs = args;}
  void setIfBlock(NodeList* ifblock) {mIfBlock = ifblock;}
  std::string getCondFuncName() {return "if_"+mCondition;}
  NodeList* getArguments() {return mArgs;}
  NodeList* getIfBlock() {return mIfBlock;}
  bool isNegated() {return mNegation;}
protected:
  bool mNegation;
  std::string mCondition;
  NodeList* mArgs;
  NodeList* mIfBlock;
};

class VariableNode : public ASTNode{
public:
  VariableNode(const std::string& name) : ASTNode(VARIABLE), mName(name), mChildVar(NULL){}
  virtual ~VariableNode(){delete mChildVar;}
  std::string& name() {return mName;}
  void setChild(VariableNode* child) {mChildVar = child;}
  VariableNode* getChild() {return mChildVar;}
protected:
  std::string mName;
  VariableNode* mChildVar;
};

class RelationalNode : public ASTNode{
public:
  enum Type{
    REL_EQUAL,
    REL_LESS,
    REL_GREATER,
    REL_PLUS,
    REL_MINUS,
    REL_TIMES,
    REL_DIVIDE,
  };
  RelationalNode() : ASTNode(RELATIONAL), mType(REL_EQUAL), mChild(NULL), mNegated(false){}
  virtual ~RelationalNode(){
    delete mChild;
  }
  Type& type() {return mType;}
  ASTNode*& child() {return mChild;}
  bool& negated() {return mNegated;}
protected:
  Type mType;
  ASTNode* mChild;
  bool mNegated;
};

class LevelNode : public StmtNode{
public:
  LevelNode(const int level, NodeList* levelblock) : StmtNode(LEVELDEF){
    mLevel = level;
    mBlock = levelblock;
  }
  ~LevelNode(){
    delete mBlock;
  }
  int getLevel() {return mLevel;}
  NodeList* getBlock() {return mBlock;}
protected:
  int mLevel;
  NodeList* mBlock;
};

class RowNode : public StmtNode{
public:
  RowNode(const int row, const std::string& text, bool visible, NodeList* rowblock) : StmtNode(ROWDEF){
    mRow = row;
    mText = text;
    mVisible = visible;
    mBlock = rowblock;
  }
  ~RowNode(){
    delete mBlock;
  }
  int getRow() {return mRow;}
  std::string getText() {return mText;}
  bool isVisible() {return mVisible;}
  NodeList* getBlock() {return mBlock;}
protected:
  int mRow;
  std::string mText;
  bool mVisible;
  NodeList* mBlock;
};

class TimerNode : public StmtNode{
public:
  TimerNode(ASTNode* time) : StmtNode(TIMERFUNC), mExec(NULL) {
    mTimeStmt = new NodeList(); mTimeStmt->addNode(time);
  }
  ~TimerNode(){
    delete mExec;
    delete mTimeStmt;
  }
  void setCommands(NodeList* exec) {mExec = exec;}
  NodeList* getCommands() {return mExec;}
  NodeList* getTime() {return mTimeStmt;}
protected:
  NodeList* mTimeStmt;
  NodeList* mExec;
};

class ArithmeticNode : public ASTNode{
public:
  enum Type{
    AR_PLUS,
    AR_MINUS,
    AR_TIMES,
    AR_DIV,
    AR_IDIV,
  };
  ArithmeticNode() : ASTNode(ARITHMETIC), mType(AR_PLUS), mLeft(NULL), mRight(NULL) {}
  //ArithmeticNode(ASTNode* left, Type t, ASTNode* right) : ASTNode(ARITHMETIC), mLeft(left), mType(t), mRight(right) {}
  virtual ~ArithmeticNode(){
    delete mLeft;
    delete mRight;
  }
  Type& type() {return mType;}
  ASTNode*& left() {return mLeft;}
  ASTNode*& right() {return mRight;}
protected:
  Type mType;
  ASTNode* mLeft;
  ASTNode* mRight;
};

class UnaryArithNode : public ASTNode{
public:
  enum Type{
    UA_DEC_SHIFT,
    UA_I2R,
  };
  UnaryArithNode() : ASTNode(UNARY_ARITH), mType(UA_DEC_SHIFT), mNode(NULL) {}
  virtual ~UnaryArithNode(){
    delete mNode;
  }
  Type& type() {return mType;}
  ASTNode*& node() {return mNode;}
protected:
  Type mType;
  ASTNode* mNode;
};

class ConcatenationNode : public ASTNode{
public:
  ConcatenationNode() : ASTNode(CONCATENATION), mLeft(NULL), mRight(NULL) {}
  //ArithmeticNode(ASTNode* left, Type t, ASTNode* right) : ASTNode(ARITHMETIC), mLeft(left), mType(t), mRight(right) {}
  virtual ~ConcatenationNode(){
    delete mLeft;
    delete mRight;
  }
  ASTNode*& left() {return mLeft;}
  ASTNode*& right() {return mRight;}
protected:
  ASTNode* mLeft;
  ASTNode* mRight;
};

#endif
