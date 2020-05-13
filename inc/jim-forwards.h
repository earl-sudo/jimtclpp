// We include this more than once on purpose.  Once in the "Jim" namespace and 
// once in the global namespace.

/* -----------------------------------------------------------------------------
 * Forwards
 * ---------------------------------------------------------------------------*/
struct Jim_Stack;
struct Jim_HashEntry;
struct Jim_HashTableType;
struct Jim_HashTable;
struct Jim_HashTableIterator;
struct Jim_Var;
struct Jim_Obj;
struct Jim_Cmd;
struct Jim_Reference;
struct Jim_ObjType;
struct Jim_Interp;
struct Jim_CallFrame;
struct Jim_PrngState;
struct Jim_ExprOperator;
struct jim_subcmd_type;
struct Jim_ListIter;
// Private to jim.cpp
struct ParseTokenList;
struct ParseToken;
struct JimExprNode;
struct ScriptToken;
struct ScriptObj;
struct JimParseMissing;
struct JimParserCtx;
struct lsort_info;
struct AssocDataValue;
struct ExprTree;
struct ExprBuilder;
struct ScanFmtPartDescr;
struct ScanFmtStringObj;
// Private elsewhere
struct regexp;

typedef char*                   charArray;
typedef unsigned long long      unsigned_long_long;
typedef long long               long_long;
typedef unsigned short          unsigned_short;
typedef unsigned long           unsigned_long;
typedef unsigned char           unsigned_char;
typedef const unsigned char     const_unsigned_char;
typedef const unsigned long     const_unsigned_long;
typedef unsigned int            unsigned_int;
typedef unsigned                unsigned_t;
typedef uint64_t                unsigned_jim_wide;
typedef int                     Retval;
typedef Jim_HashEntry*          Jim_HashEntryArray;
typedef Jim_HashEntry*          Jim_HashEntryPtr;
typedef void*                   VoidPtrArray;
typedef Jim_Obj*                Jim_ObjArray;
typedef const char*             constCharArray;
typedef Jim_Obj* const*         Jim_ObjConstArray;
typedef Jim_Stack*              Jim_StackPtr;
typedef Jim_HashTable*          Jim_HashTablePtr;
typedef Jim_Interp*             Jim_InterpPtr;
// Private to jim.cpp
typedef ScriptToken*            ScriptTokenPtr;
typedef ParseTokenList*         ParseTokenListPtr;
typedef ParseToken*             ParseTokenPtr;
typedef JimExprNode*            JimExprNodePtr;
typedef Jim_ExprOperator*       Jim_ExprOperatorPtr;
typedef Jim_ExprOperator*       const_Jim_ExprOperatorPtr;
typedef ExprTree*               ExprTreePtr;
typedef ExprBuilder*            ExprBuilderPtr;
typedef ScanFmtPartDescr*       ScanFmtPartDescrPtr;
typedef ScanFmtStringObj*       ScanFmtStringObjPtr;
typedef Jim_ListIter*           Jim_ListIterPtr;
typedef Jim_Obj*                Jim_ObjPtr;
typedef Jim_CallFrame*          Jim_CallFramePtr;
typedef Jim_Cmd*                Jim_CmdPtr;
typedef Jim_Reference*          Jim_ReferencePtr;
