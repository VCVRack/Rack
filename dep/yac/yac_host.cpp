/// ----
/// ---- file   : yac_host.cpp
/// ----
/// ---- author : (c) 2003 - 2018 by bsp
/// ----
/// ---- date   : 21-Nov-2003 / 27-Jun-2003 / 13-Feb-2004 / 17-Feb-2004 / 19-Feb-2004 / 26-Mar-2004
/// ----          19-Feb-2005 / 24-Feb-2005 / 12-Mar-2005 / 19-Mar-2005 / 07-May-2005 / 28-Aug-2005
/// ----          05-Sep-2005 / 28-Nov-2005 / 03-Feb-2006 / 05-Feb-2006 / 07-Feb-2006 / 24-Apr-2006
/// ----          13-May-2006 / 01-Feb-2008 / 18-Feb-2008 / 24-Feb-2008 / 22-Mar-2008 / 14-Sep-2008
/// ----          08-Jan-2009 / 05-Mar-2009 / 01-Apr-2009 / 04-Apr-2009 / 18-Apr-2009 / 04-May-2009
/// ----          01-Jun-2009 / 11-Jul-2010 / 21-Apr-2011 / 21-Dec-2012 / 13-Aug-2013 / 05-Feb-2014
/// ----          26-Jun-2018
/// ----
/// ---- info   : YAC - Yet Another Component object model. originally for the "tks" project
/// ----
/// ---- license: MIT. See LICENSE.txt.
/// ---- 
/// ---- 

#ifndef __YAC_HOST_CPP__
#define __YAC_HOST_CPP__


//
// Define Dyac_object_counter_inc/dec macros before including this file to override
// the default implementation
//
#ifndef Dyac_object_counter_inc

#ifdef YAC_OBJECT_COUNTER
#define Dyac_object_counter_inc YAC_Object::object_counter++
#define Dyac_object_counter_dec YAC_Object::object_counter--
#else
#define Dyac_object_counter_inc (void)0
#define Dyac_object_counter_dec (void)0
#endif // YAC_OBJECT_COUNTER

#endif // Dyac_object_counter_inc


#ifdef YAC_TRACK_CHARALLOC
sSI yac_string_total_char_size = 0;
sSI yac_string_peak_char_size = 0;
sU8 *yac_string_alloc_chars(size_t _n) {
   yac_string_total_char_size += _n;
   if(yac_string_total_char_size > yac_string_peak_char_size)
   {
      yac_string_peak_char_size = yac_string_total_char_size;
   }
   sSI *adr = (sSI*)::malloc(_n + sizeof(sSI));
   *adr = _n;
   return (sU8*) (adr + 1);
}
void yac_string_free_chars(sU8 *_chars) {
   sSI *adr = ((sSI*)_chars) - 1;
   yac_string_total_char_size -= *adr;
   ::free(adr);
}
#endif // YAC_TRACK_CHARALLOC



#ifndef YAC_CUST_VALUE
     YAC_Value::YAC_Value                                    (void)                                             {type=0;}
     YAC_Value::~YAC_Value                                   ()                                                 { }
void YAC_Value::initVoid                                     (void)                                             {deleteme=0;type=0;value.any=0;}
void YAC_Value::initInt                                      (sSI _i)                                           {deleteme=0;type=1;value.int_val=_i;}
void YAC_Value::initFloat                                    (sF32 _f)                                          {deleteme=0;type=2;value.float_val=_f;}
void YAC_Value::initString                                   (YAC_String *_s, sBool _del)                       {deleteme=_del;type=4;value.string_val=_s;}
void YAC_Value::initNewString                                (YAC_String *s)                                    {unsetFast(); type=4; deleteme=1; value.string_val=YAC_New_String(); value.string_val->yacCopy(s); }
void YAC_Value::initObject                                   (YAC_Object *_o, sBool _del)                       {deleteme=_del;type=3;value.object_val=_o;}
void YAC_Value::initNull                                     (void)                                             {unsetFast(); type=YAC_TYPE_OBJECT; value.any = 0; deleteme = 0; }
void YAC_Value::safeInitInt                                  (sSI _i)                                           {unsetFast(); initInt(_i); }
void YAC_Value::safeInitFloat                                (sF32 _f)                                          {unsetFast(); initFloat(_f); }
void YAC_Value::safeInitString                               (YAC_String *_s, sBool _del)                       {unsetFast(); initString(_s, _del); }
void YAC_Value::safeInitObject                               (YAC_Object *_o, sBool _new)                       {unsetFast(); initObject(_o, _new); }
void YAC_Value::initEmptyString                              (void)                                             {deleteme=1; type=YAC_TYPE_STRING; value.string_val=(YAC_String*)YAC_NEW_CORE_POOLED(YAC_CLID_STRING);}
void YAC_Value::typecast                                     (sUI _type)                                        {
   sSI         i_t; 
   sF32        f_t; 
   switch(_type) 
   { 
   default: 
      unset(); 
      break; 
   case 1: 
      switch(type) 
      {  
      default:
      case 0: 
         initInt(0); 
         break; 
      case 1: 
         break; 
      case 2: 
         initInt((sSI)value.float_val);
         break; 
      case 3: 
         //i_t=(value.object_val!=0); 
         //safeInitInt(i_t);  
         if(value.object_val) 
         { 
            value.object_val->yacScanI(&i_t); 
            safeInitInt(i_t); 
         } 
         else 
            initInt(0);
         break; 
      case 4: 
         if(value.string_val)  
         { 
            ////if(value.string_val->checkFloatConversion()) 
            { 
               value.string_val->yacScanF32(&f_t); 
               safeInitInt((sSI)f_t); 
            } 
            ////else 
            //// {
            ////   value.string_val->yacScanI(&i_t);  
            ////   safeInitInt(i_t); 
            ///} 
         }
         else  
         { 
            safeInitInt(0); 
         }
         break; 
      } 
      break; 
      case 2: 
         switch(type) 
         {  
         default: 
         case 0: 
            initFloat(0.0f); 
            break;
         case 1: 
            initFloat((sF32)value.int_val);
            break; 
         case 2: 
            break; 
         case 3:  
         case 4: 
            // ---- convert object to float value 
            if(value.object_val) 
               value.object_val->yacScanF32(&f_t); 
            else 
               f_t=0.0f; 
            safeInitFloat(f_t); 
            break; 
         } 
         break; 
         case 3:  
            switch(type) 
            { 
            default: 
            case YAC_TYPE_VOID: 
               // ---- typecast void to "null" Object 
               initObject(NULL, 0); 
               break; 
            case YAC_TYPE_INT: 
               // ---- typecast int to Integer Object 
               { 
                  YAC_Integer *io = (YAC_Integer*) YAC_NEW_CORE_POOLED(YAC_CLID_INTEGER);
                  io->value=value.int_val; 
                  initObject(io, 1); 
               } 
               break; 
            case YAC_TYPE_FLOAT: 
               // ---- typecast float to Float Object 
               { 
                  YAC_Float *fo = (YAC_Float*) YAC_NEW_CORE_POOLED(YAC_CLID_FLOAT);
                  fo->value=value.float_val; 
                  initObject(fo, 1); 
               } 
               break; 
            case YAC_TYPE_OBJECT: 
            case YAC_TYPE_STRING: 
               break; 
            } 
            break;
            case 4: 
               { 
                  //YAC_String *s; 
                  switch(type) 
                  {  
                  default:
                  case YAC_TYPE_VOID: 
                     initEmptyString(); 
                     break; 
                  case YAC_TYPE_INT: 
                     i_t = value.int_val; 
                     value.string_val = (YAC_String*) YAC_NEW_CORE_POOLED(YAC_CLID_STRING16);
                     value.string_val->printf("%i", i_t); 
                     deleteme=1; 
                     type=YAC_TYPE_STRING; 
                     break; 
                  case YAC_TYPE_FLOAT: 
                     f_t = value.float_val; 
                     value.string_val = (YAC_String*) YAC_NEW_CORE_POOLED(YAC_CLID_STRING32);
                     value.string_val->printf("%g", f_t); 
                     deleteme=1; 
                     type=YAC_TYPE_STRING; 
                     break; 
                  case YAC_TYPE_OBJECT:  
                     if(YAC_VALID(value.object_val))
                     { 
                        if(value.object_val->class_ID!=YAC_CLID_STRING) // string fix? 
                        { 
                           YAC_String *s = (YAC_String*) YAC_NEW_CORE_POOLED(YAC_CLID_STRING);
                           value.object_val->yacToString(s); // convert Object to String 
                           safeInitString(s, 1); 
                        } 
                     } 
                     else 
                     { 
                        // ---- convert null object to empty String "" 
                        initString((YAC_String*) YAC_NEW_CORE_POOLED(YAC_CLID_STRING), 1); 
                     } 
                     break;
                  case YAC_TYPE_STRING: 
                     break; 
                  } 
               } 
               break; 
   }
} 

void YAC_Value::unsetFast                                    (void)                                             {if(type>=YAC_TYPE_OBJECT&&deleteme&&value.any)yac_host->yacDelete(value.object_val);} 
void YAC_Value::unset                                        (void)                                             {if(type>=YAC_TYPE_OBJECT&&deleteme&&value.any)yac_host->yacDelete(value.object_val);value.object_val=0;type=0;deleteme=0;} 
void YAC_Value::operator =                                   (YAC_Value *_v)                                    {deleteme=_v->deleteme;_v->deleteme=0;type=_v->type;value=_v->value;};
void YAC_Value::copyRef                                      (const YAC_Value *_o)                         {type=_o->type;class_type=_o->class_type;value.any=_o->value.any;deleteme=0;}
#endif

#ifndef YAC_CUST_EVENT
                 YAC_Event::YAC_Event                 (void)                       {class_ID=YAC_CLID_EVENT;}
                 YAC_Event::~YAC_Event                ()                           { }
#endif // YAC_CUST_EVENT

#ifndef YAC_CUST_LISTNODE
YAC_ListNode::YAC_ListNode                              (void)                                             {class_ID=YAC_CLID_LISTNODE;prev=next=0;}
YAC_ListNode::~YAC_ListNode                             ()                                                 {if(next){yac_host->yacDelete(next);next=0;}unset();}
#endif

#ifndef YAC_CUST_TREENODE
YAC_TreeNode::YAC_TreeNode                              (void)                                             {class_ID=YAC_CLID_TREENODE;left=0;right=0;deleteme=0;value.int_val=0;}
YAC_TreeNode::~YAC_TreeNode                             ()                                                 {if(left){yac_host->yacDelete(left);left=0;}if(right){yac_host->yacDelete(right); right=0;}unset();}
#endif


   // ---- 
   // ---- 
   // ---- LEVEL (1<<0) interface 
   // ----                    ( C++ reflectance support )
   // ---- 
   // ---- 
#ifdef YAC_OBJECT_COUNTER
sUI              YAC_Object::object_counter                  =0;
#endif //YAC_OBJECT_COUNTER
                 YAC_Object::YAC_Object                      (void)                                  {
                    Dyac_object_counter_inc;

#ifdef YAC_OBJECT_TAGS
                    validation_tag = YAC_VALID_TAG;
#endif

#ifdef YAC_OBJECT_POOL
                    pool_handle.pool_id = 0;
#endif // YAC_OBJECT_POOL
                    
                    // Note: pool_handle initialization is left out by intention! (it has already been set *before* this constructor is called!)
                 }
                 YAC_Object::~YAC_Object                     ()                                      {
                    Dyac_object_counter_dec;
#ifdef YAC_OBJECT_TAGS
                    validation_tag = YAC_INVALID_TAG;
#endif

#ifdef YAC_OBJECT_POOL
                    pool_handle.pool_id = 0;
#endif // YAC_OBJECT_POOL
                 }
sUI              YAC_VCALL YAC_Object::yacQueryInterfaces              (void)                                  {return YAC_INTERFACE_ALL;}
#ifndef YAC_CUST_OBJECT
const sChar*     YAC_VCALL YAC_Object::yacClassName                    (void)                                  {return "null";}
YAC_Object*      YAC_VCALL YAC_Object::yacNewObject                    (void)                                  {return 0;}
sUI              YAC_VCALL YAC_Object::yacMemberGetNum                 (void)                                  {return 0;}
const char     **YAC_VCALL YAC_Object::yacMemberGetNames               (void)                                  {return 0;}
const sUI       *YAC_VCALL YAC_Object::yacMemberGetTypes               (void)                                  {return 0;}
const char     **YAC_VCALL YAC_Object::yacMemberGetObjectTypes         (void)                                  {return 0;}
const sU8      **YAC_VCALL YAC_Object::yacMemberGetOffsets             (void)                                  {return (const sU8**)0;}
sUI              YAC_VCALL YAC_Object::yacMethodGetNum                 (void)                                  {return 0;}
const char     **YAC_VCALL YAC_Object::yacMethodGetNames               (void)                                  {return 0;}
const sUI       *YAC_VCALL YAC_Object::yacMethodGetNumParameters       (void)                                  {return 0;}
const sUI      **YAC_VCALL YAC_Object::yacMethodGetParameterTypes      (void)                                  {return 0;}
const char    ***YAC_VCALL YAC_Object::yacMethodGetParameterObjectTypes(void)                                  {return 0;}
const sUI       *YAC_VCALL YAC_Object::yacMethodGetReturnTypes         (void)                                  {return 0;}
const char     **YAC_VCALL YAC_Object::yacMethodGetReturnObjectTypes   (void)                                  {return 0;}
const void     **YAC_VCALL YAC_Object::yacMethodGetAdr                 (void)                                  {return 0;}
sUI              YAC_VCALL YAC_Object::yacConstantGetNum               (void)                                  {return 0;}
const char     **YAC_VCALL YAC_Object::yacConstantGetNames             (void)                                  {return 0;}
const sUI       *YAC_VCALL YAC_Object::yacConstantGetTypes             (void)                                  {return 0;}
yacmemptr        YAC_VCALL YAC_Object::yacConstantGetValues            (void)                                  {yacmemptr r; r.any=0; return r;}
#endif // YAC_CUST_OBJECT
void             YAC_VCALL YAC_Object::yacFinalizeObject               (YAC_ContextHandle)                     { }
void             YAC_VCALL YAC_Object::yacGetConstantStringList        (YAC_String *)                          { }
sBool            YAC_VCALL YAC_Object::yacIsComposite                  (void)                                  {return 0;}
sUI              YAC_VCALL YAC_Object::yacPoolGetSize                  (void)                                  {return 0;}
void             YAC_VCALL YAC_Object::yacPoolInit                     (YAC_Object *)                          { }
sUI              YAC_VCALL YAC_Object::yacPoolGetPriority              (void)                                  {return 0;}
void             YAC_VCALL YAC_Object::vtable_entry_0_25_reserved      (void)                                  { }
void             YAC_VCALL YAC_Object::vtable_entry_0_26_reserved      (void)                                  { }
void             YAC_VCALL YAC_Object::vtable_entry_0_27_reserved      (void)                                  { }
void             YAC_VCALL YAC_Object::vtable_entry_0_28_reserved      (void)                                  { }
void             YAC_VCALL YAC_Object::vtable_entry_0_29_reserved      (void)                                  { }
void             YAC_VCALL YAC_Object::vtable_entry_0_30_reserved      (void)                                  { }
void             YAC_VCALL YAC_Object::vtable_entry_0_31_reserved      (void)                                  { }


   // ---- 
   // ---- 
   // ---- LEVEL (1<<1) interface 
   // ----                    ( operator support )
   // ----                        - also see YAC_OP_xxx constants
   // ---- 
   // ---- 
void             YAC_VCALL Object__operator                           (void *_o, yacmemptr _args,YAC_Value*_v){((YAC_Object*)_o)->yacOperator(_args.si[0], _args.io[1], _v);}
sBool            YAC_VCALL YAC_Object::yacCopy                        (YAC_Object *)                          {return 0;}
sBool            YAC_VCALL YAC_Object::yacEquals                      (YAC_Object *_o)                        {return ((void*)this)==((void*)_o);} 
void             YAC_VCALL YAC_Object::yacOperator                    (sSI _i, YAC_Object *_o, YAC_Value *_r) { 
	switch(_i)                                                                                       { 
	case YAC_OP_SHL: if(YAC_IS_STREAM(_o)){yacDeserialize(_o,1/**include type information**/);}       break;
	case YAC_OP_CLT: _r->initInt(0);                                                                  break;
	case YAC_OP_CLE: _r->initInt(0);                                                                  break;
   case YAC_OP_CEQ: 
      if(_o)
         if(class_ID==YAC_CLID_OBJECT/*null*/)
            _r->initInt(_o->class_ID==YAC_CLID_OBJECT);
         else 
            _r->initInt(((void*)this)==((void*)_o));
      else 
         _r->initInt(class_ID==YAC_CLID_OBJECT/*null*/); 
      break;
   case YAC_OP_CNE: 
      //_r->initInt(((sUI)this)!=((sUI)_o));                                             break;
      if(_o)
         if(class_ID==YAC_CLID_OBJECT/*null*/)
            _r->initInt(_o->class_ID!=YAC_CLID_OBJECT);
         else 
            _r->initInt(((void*)this)!=((void*)_o));
      else 
         _r->initInt(class_ID!=YAC_CLID_OBJECT/*null*/); 
      break;
	case YAC_OP_CGE: _r->initInt(0);                                                                  break;
	case YAC_OP_CGT: _r->initInt(0);                                                                  break;
	                                                                                                 }
                                                                                                    } 
void             YAC_VCALL YAC_Object::yacOperatorInit                (void *_context, YAC_Object *_robj)     {YAC_Value r;yacOperator(YAC_OP_INIT,_robj,&r);r.unsetFast();} 
void             YAC_VCALL YAC_Object::yacOperatorAssign              (YAC_Object *_robj)                     {YAC_Value r;yacOperator(YAC_OP_ASSIGN,_robj,&r);r.unsetFast();}
void             YAC_VCALL YAC_Object::yacOperatorAdd                 (YAC_Object *_robj)                     {YAC_Value ret;yacOperator(YAC_OP_ADD,_robj,&ret);ret.unsetFast();}
void             YAC_VCALL YAC_Object::yacOperatorSub                 (YAC_Object *_robj)                     {YAC_Value ret;yacOperator(YAC_OP_SUB,_robj,&ret);ret.unsetFast();}
void             YAC_VCALL YAC_Object::yacOperatorMul                 (YAC_Object *_robj)                     {YAC_Value ret;yacOperator(YAC_OP_MUL,_robj,&ret);ret.unsetFast();}
void             YAC_VCALL YAC_Object::yacOperatorDiv                 (YAC_Object *_robj)                     {YAC_Value ret;yacOperator(YAC_OP_DIV,_robj,&ret);ret.unsetFast();}
void             YAC_VCALL YAC_Object::yacOperatorClamp               (YAC_Object *, YAC_Object *)            { }
void             YAC_VCALL YAC_Object::yacOperatorWrap                (YAC_Object *, YAC_Object *)            { }
sBool            YAC_VCALL YAC_Object::yacScanI                       (sSI *_ip)                              {*_ip=0;   return 0;}
sBool            YAC_VCALL YAC_Object::yacScanF32                     (sF32 *_fp)                             {*_fp=0.0f;return 0;}
sBool            YAC_VCALL YAC_Object::yacScanF64                     (sF64 *_dp)                             {*_dp=0.0; return 0;}
sBool            YAC_VCALL YAC_Object::yacToString                    (YAC_String *) const                    {return 0;}
sBool            YAC_VCALL YAC_Object::yacScanI64                     (sS64 *)                                {return 0;}
void             YAC_VCALL YAC_Object::yacOperatorI                   (sSI _cmd,sSI _val,YAC_Value *)         {if(_cmd==YAC_OP_ASSIGN)yacValueOfI(_val);}
void             YAC_VCALL YAC_Object::yacOperatorF32                 (sSI _cmd,sF32 _val,YAC_Value *_r)      {if(_cmd==YAC_OP_ASSIGN)yacValueOfF32(_val);else yacOperatorI(_cmd, (sSI)_val, _r);}
void             YAC_VCALL YAC_Object::yacOperatorF64                 (sSI _cmd,sF64 _val,YAC_Value *_r)      {if(_cmd==YAC_OP_ASSIGN)yacValueOfF64(_val);else yacOperatorF32(_cmd, (sF32)_val, _r);}
void             YAC_VCALL YAC_Object::yacValueOfI                    (sSI)                                   {}
void             YAC_VCALL YAC_Object::yacValueOfF32                  (sF32 _val)                             {yacValueOfI((sSI)_val);}
void             YAC_VCALL YAC_Object::yacValueOfF64                  (sF64 _val)                             {yacValueOfF32((sF32)_val);}
sUI              YAC_VCALL YAC_Object::yacOperatorPriority            (void)                                  {return 0;}
void             YAC_VCALL YAC_Object::yacValueOfI64                  (sS64 _val)                             {yacValueOfI((sSI)_val);}
sSI              YAC_VCALL YAC_Object::yacTensorRank                  (void)                                  {return YAC_TENSOR_RANK_NONE;}
sBool            YAC_VCALL YAC_Object::yacToParsableString            (YAC_String *s) const                   {return yacToString(s);}
void             YAC_VCALL YAC_Object::yacOperatorI64                 (sSI _cmd,sS64 _val,YAC_Value *_r)      {if(_cmd==YAC_OP_ASSIGN)yacValueOfI64(_val);else yacOperatorI(_cmd, (sSI)_val, _r);}
void             YAC_VCALL YAC_Object::vtable_entry_1_28_reserved     (void)                                  { }
void             YAC_VCALL YAC_Object::vtable_entry_1_29_reserved     (void)                                  { }
void             YAC_VCALL YAC_Object::vtable_entry_1_30_reserved     (void)                                  { }
void             YAC_VCALL YAC_Object::vtable_entry_1_31_reserved     (void)                                  { }

   // ---- 
   // ---- 
   // ---- LEVEL (1<<2) interface 
   // ----                    ( stream support )
   // ---- 
   // ---- 
   // ---- 
sBool            YAC_VCALL YAC_Object::yacIsStream                    (void)                                  {return 0;}
void             YAC_VCALL YAC_Object::yacStreamClose                 (void)                                  { }
sBool            YAC_VCALL YAC_Object::yacStreamOpenLocal             (sChar *, sSI)                          {return 0;}
sBool            YAC_VCALL YAC_Object::yacStreamOpenLogic             (sChar *)                               {return 0;}
sUI              YAC_VCALL YAC_Object::yacStreamGetByteOrder          (void)                                  {return 0;}
void             YAC_VCALL YAC_Object::yacStreamSetByteOrder          (sUI)                                   { }        
sBool            YAC_VCALL YAC_Object::yacStreamEOF                   (void)                                  {return 1;}
void             YAC_VCALL YAC_Object::yacStreamSeek                  (sSI, sUI)                              { }        
sUI              YAC_VCALL YAC_Object::yacStreamGetOffset             (void)                                  {return 0;}
void             YAC_VCALL YAC_Object::yacStreamSetOffset             (sUI)                                   { }        
sUI              YAC_VCALL YAC_Object::yacStreamGetSize               (void)                                  {return 0;}
sSI              YAC_VCALL YAC_Object::yacStreamRead                  (sU8 *, sUI)                            {return 0;}
sU8              YAC_VCALL YAC_Object::yacStreamReadI8                (void)                                  {return 0;}
sU16             YAC_VCALL YAC_Object::yacStreamReadI16               (void)                                  {return 0;}
sU32             YAC_VCALL YAC_Object::yacStreamReadI32               (void)                                  {return 0;}
sF32             YAC_VCALL YAC_Object::yacStreamReadF32               (void)                                  {return 0;}
void             YAC_VCALL YAC_Object::yacStreamReadObject            (YAC_Object *)                          { }        
sSI              YAC_VCALL YAC_Object::yacStreamReadString            (YAC_String *, sUI)                     {return 0;}
sSI              YAC_VCALL YAC_Object::yacStreamReadBuffer            (YAC_Buffer *, sUI, sUI, sBool)         {return 0;}
sSI              YAC_VCALL YAC_Object::yacStreamReadLine              (YAC_String *, sUI)                     {return 0;}
sSI              YAC_VCALL YAC_Object::yacStreamWrite                 (sU8 *, sUI)                            {return 0;}
void             YAC_VCALL YAC_Object::yacStreamWriteI8               (sU8)                                   { }        
void             YAC_VCALL YAC_Object::yacStreamWriteI16              (sU16)                                  { }        
void             YAC_VCALL YAC_Object::yacStreamWriteI32              (sS32)                                  { }        
void             YAC_VCALL YAC_Object::yacStreamWriteF32              (sF32)                                  { }        
void             YAC_VCALL YAC_Object::yacStreamWriteObject           (YAC_Object*)                           { }        
sSI              YAC_VCALL YAC_Object::yacStreamWriteString           (YAC_String *, sUI, sUI)                {return 0;}
sSI              YAC_VCALL YAC_Object::yacStreamWriteBuffer           (YAC_Buffer *, sUI, sUI)                {return 0;}
sSI              YAC_VCALL YAC_Object::yacStreamGetErrorCode          (void)                                  {return 0xCCCCCCCC;}
void             YAC_VCALL YAC_Object::yacStreamGetErrorStringByCode  (sSI, YAC_Value *)                      { }
sF64             YAC_VCALL YAC_Object::yacStreamReadF64               (void)                                  {return 0;}
void             YAC_VCALL YAC_Object::yacStreamWriteF64              (sF64)                                  { }        
sU64             YAC_VCALL YAC_Object::yacStreamReadI64               (void)                                  {return 0;}
void             YAC_VCALL YAC_Object::yacStreamWriteI64              (sS64)                                  { }        
void             YAC_VCALL YAC_Object::vtable_entry_2_35_reserved     (void)                                  { }
void             YAC_VCALL YAC_Object::vtable_entry_2_36_reserved     (void)                                  { }
void             YAC_VCALL YAC_Object::vtable_entry_2_37_reserved     (void)                                  { }
void             YAC_VCALL YAC_Object::vtable_entry_2_38_reserved     (void)                                  { }
void             YAC_VCALL YAC_Object::vtable_entry_2_39_reserved     (void)                                  { }
void             YAC_VCALL YAC_Object::vtable_entry_2_40_reserved     (void)                                  { }
void             YAC_VCALL YAC_Object::vtable_entry_2_41_reserved     (void)                                  { }
void             YAC_VCALL YAC_Object::vtable_entry_2_42_reserved     (void)                                  { }
void             YAC_VCALL YAC_Object::vtable_entry_2_43_reserved     (void)                                  { }
void             YAC_VCALL YAC_Object::vtable_entry_2_44_reserved     (void)                                  { }
void             YAC_VCALL YAC_Object::vtable_entry_2_45_reserved     (void)                                  { }
void             YAC_VCALL YAC_Object::vtable_entry_2_46_reserved     (void)                                  { }
void             YAC_VCALL YAC_Object::vtable_entry_2_47_reserved     (void)                                  { }

   // ---- 
   // ---- 
   // ---- LEVEL (1<<3) interface 
   // ----                    ( serialization support )
   // ---- 
   // ---- 
   // ---- 
void             YAC_VCALL YAC_Object::yacSerializeClassName          (YAC_Object *_ofs)                      {YAC_String cln;cln.visit((sChar*)yacClassName());cln.yacSerialize(_ofs, 0);}
void             YAC_VCALL YAC_Object::yacSerialize                   (YAC_Object *, sUI)                     { }
sUI              YAC_VCALL YAC_Object::yacDeserialize                 (YAC_Object *, sUI)                     {return 0;}
void             YAC_VCALL YAC_Object::vtable_entry_3_3_reserved      (void)                                  { }
void             YAC_VCALL YAC_Object::vtable_entry_3_4_reserved      (void)                                  { }
void             YAC_VCALL YAC_Object::vtable_entry_3_5_reserved      (void)                                  { }
void             YAC_VCALL YAC_Object::vtable_entry_3_6_reserved      (void)                                  { }
void             YAC_VCALL YAC_Object::vtable_entry_3_7_reserved      (void)                                  { }
void             YAC_VCALL YAC_Object::vtable_entry_3_8_reserved      (void)                                  { }
void             YAC_VCALL YAC_Object::vtable_entry_3_9_reserved      (void)                                  { }
void             YAC_VCALL YAC_Object::vtable_entry_3_10_reserved     (void)                                  { }
void             YAC_VCALL YAC_Object::vtable_entry_3_11_reserved     (void)                                  { }
void             YAC_VCALL YAC_Object::vtable_entry_3_12_reserved     (void)                                  { }
void             YAC_VCALL YAC_Object::vtable_entry_3_13_reserved     (void)                                  { }
void             YAC_VCALL YAC_Object::vtable_entry_3_14_reserved     (void)                                  { }
void             YAC_VCALL YAC_Object::vtable_entry_3_15_reserved     (void)                                  { }

   // ---- 
   // ---- 
   // ---- LEVEL (1<<4) interface 
   // ----                    ( iterator support )
   // ---- 
   // ---- 
   // ---- 

sBool            YAC_VCALL YAC_Object::yacIteratorInit                (YAC_Iterator *) const                  {return 0;} 
void             YAC_VCALL YAC_Object::vtable_entry_4_1_reserved      (void)                                  { }
void             YAC_VCALL YAC_Object::vtable_entry_4_2_reserved      (void)                                  { }
void             YAC_VCALL YAC_Object::vtable_entry_4_3_reserved      (void)                                  { }
void             YAC_VCALL YAC_Object::vtable_entry_4_4_reserved      (void)                                  { }
void             YAC_VCALL YAC_Object::vtable_entry_4_5_reserved      (void)                                  { }
void             YAC_VCALL YAC_Object::vtable_entry_4_6_reserved      (void)                                  { }
void             YAC_VCALL YAC_Object::vtable_entry_4_7_reserved      (void)                                  { }
void             YAC_VCALL YAC_Object::vtable_entry_4_8_reserved      (void)                                  { }
void             YAC_VCALL YAC_Object::vtable_entry_4_9_reserved      (void)                                  { }
void             YAC_VCALL YAC_Object::vtable_entry_4_10_reserved     (void)                                  { }
void             YAC_VCALL YAC_Object::vtable_entry_4_11_reserved     (void)                                  { }
void             YAC_VCALL YAC_Object::vtable_entry_4_12_reserved     (void)                                  { }
void             YAC_VCALL YAC_Object::vtable_entry_4_13_reserved     (void)                                  { }
void             YAC_VCALL YAC_Object::vtable_entry_4_14_reserved     (void)                                  { }
void             YAC_VCALL YAC_Object::vtable_entry_4_15_reserved     (void)                                  { }

   // ---- 
   // ---- 
   // ---- LEVEL (1<<5) interface 
   // ----                    ( array and hashtable support )
   // ---- 
   // ---- 
   // ---- 
YAC_Object*      YAC_VCALL YAC_Object::yacArrayNew                    (void)                                  {return 0;} 
sUI              YAC_VCALL YAC_Object::yacArrayAlloc                  (sUI, sUI, sUI, sUI)                    {return 0;}
sBool            YAC_VCALL YAC_Object::yacArrayRealloc                (sUI,sUI,sUI,sUI)                       {return 0;}
sUI              YAC_VCALL YAC_Object::yacArrayGetNumElements         (void)                                  {return 0;}
sUI              YAC_VCALL YAC_Object::yacArrayGetMaxElements         (void)                                  {return 0;}
void             YAC_VCALL YAC_Object::yacArrayCopySize               (YAC_Object *)                          { }
void             YAC_VCALL YAC_Object::yacArraySet                    (void *_context, sUI, YAC_Value *)      { }
void             YAC_VCALL YAC_Object::yacArrayGet                    (void *_context, sUI, YAC_Value *)      { }
sUI              YAC_VCALL YAC_Object::yacArrayGetWidth               (void)                                  {return 0;} 
sUI              YAC_VCALL YAC_Object::yacArrayGetHeight              (void)                                  {return 0;}
sUI              YAC_VCALL YAC_Object::yacArrayGetElementType         (void)                                  {return 0;}
sUI              YAC_VCALL YAC_Object::yacArrayGetElementByteSize     (void)                                  {return 0;}
sUI              YAC_VCALL YAC_Object::yacArrayGetStride              (void)                                  {return 0;} 
void *           YAC_VCALL YAC_Object::yacArrayGetPointer             (void)                                  {return 0;} 
void             YAC_VCALL YAC_Object::yacArraySetWidth               (sUI)                                   { }
void             YAC_VCALL YAC_Object::yacArraySetTemplate            (YAC_Object *)                          { }
void             YAC_VCALL YAC_Object::yacArrayGetDeref               (void *_context, sUI _i, YAC_Value *_r) {yacArrayGet(_context, _i, _r);}
void             YAC_VCALL YAC_Object::vtable_entry_5_17_reserved     (void)                                  { }
void             YAC_VCALL YAC_Object::vtable_entry_5_18_reserved     (void)                                  { }
void             YAC_VCALL YAC_Object::vtable_entry_5_19_reserved     (void)                                  { }
void             YAC_VCALL YAC_Object::vtable_entry_5_20_reserved     (void)                                  { }
void             YAC_VCALL YAC_Object::vtable_entry_5_21_reserved     (void)                                  { }
void             YAC_VCALL YAC_Object::vtable_entry_5_22_reserved     (void)                                  { }
void             YAC_VCALL YAC_Object::vtable_entry_5_23_reserved     (void)                                  { }
void             YAC_VCALL YAC_Object::vtable_entry_5_24_reserved     (void)                                  { }
void             YAC_VCALL YAC_Object::vtable_entry_5_25_reserved     (void)                                  { }
void             YAC_VCALL YAC_Object::vtable_entry_5_26_reserved     (void)                                  { }
void             YAC_VCALL YAC_Object::vtable_entry_5_27_reserved     (void)                                  { }
void             YAC_VCALL YAC_Object::vtable_entry_5_28_reserved     (void)                                  { }
void             YAC_VCALL YAC_Object::vtable_entry_5_29_reserved     (void)                                  { }
void             YAC_VCALL YAC_Object::vtable_entry_5_30_reserved     (void)                                  { }
void             YAC_VCALL YAC_Object::vtable_entry_5_31_reserved     (void)                                  { }

void             YAC_VCALL YAC_Object::yacHashSet                     (void *_context, YAC_String *_s, YAC_Value *_v)         { 
   sSI i=0;
   if(YAC_Is_String(_s))
   {
      YAC_String *s=(YAC_String*)_s;
      s->yacScanI(&i);
   }
   yacArraySet(_context, (sUI)i, _v);
} 
void             YAC_VCALL YAC_Object::yacHashGet                     (void *_context, YAC_String*_s, YAC_Value *_v)          { 
   sSI i=0;
   if(YAC_Is_String(_s))
   {
      YAC_String *s=(YAC_String*)_s;
      s->yacScanI(&i);
   }
   yacArrayGet(_context, (sUI)i, _v);
}
void             YAC_VCALL YAC_Object::yacHashGetDeref                (void *_context, YAC_String*_s, YAC_Value *_v)          {yacHashGet(NULL, _s, _v);} // xxx TKS_MT: should use *real* thread context (exceptions!)
void             YAC_VCALL YAC_Object::vtable_entry_5_35_reserved     (void)                                  { }
void             YAC_VCALL YAC_Object::vtable_entry_5_36_reserved     (void)                                  { }
void             YAC_VCALL YAC_Object::vtable_entry_5_37_reserved     (void)                                  { }
void             YAC_VCALL YAC_Object::vtable_entry_5_38_reserved     (void)                                  { }
void             YAC_VCALL YAC_Object::vtable_entry_5_39_reserved     (void)                                  { }

   // ---- 
   // ---- 
   // ---- LEVEL (1<<6) interface 
   // ----                    ( metaclass support )
   // ---- 
   // ---- 
   // ---- 
sChar *          YAC_VCALL YAC_Object::yacMetaClassName                     (void)                            {return 0;} 
sUI              YAC_VCALL YAC_Object::yacMetaClassMemberGetNum             (void)                            {return 0;}
sUI              YAC_VCALL YAC_Object::yacMetaClassMemberGetAccessKeyByIndex(sUI)                             {return 0;}
sUI              YAC_VCALL YAC_Object::yacMetaClassMemberGetAccessKeyByName (const sChar *)                   {return 0;}
sUI              YAC_VCALL YAC_Object::yacMetaClassMemberGetType            (sUI)                             {return 0;}
sChar *          YAC_VCALL YAC_Object::yacMetaClassMemberGetName            (sUI)                             {return 0;}
void             YAC_VCALL YAC_Object::yacMetaClassMemberSet                (sUI, YAC_Value *)                { }
void             YAC_VCALL YAC_Object::yacMetaClassMemberGet                (sUI, YAC_Value *)                { }
sSI              YAC_VCALL YAC_Object::yacMetaClassInstanceOf               (YAC_Object *)                    {return 0;}
void             YAC_VCALL YAC_Object::vtable_entry_6_10_reserved           (void)                            { }
void             YAC_VCALL YAC_Object::vtable_entry_6_11_reserved           (void)                            { }
void             YAC_VCALL YAC_Object::vtable_entry_6_12_reserved           (void)                            { }
void             YAC_VCALL YAC_Object::vtable_entry_6_13_reserved           (void)                            { }
void             YAC_VCALL YAC_Object::vtable_entry_6_14_reserved           (void)                            { }
void             YAC_VCALL YAC_Object::vtable_entry_6_15_reserved           (void)                            { }

   // ---- 
   // ---- 
   // ---- LEVEL (1<<7) interface 
   // ----                    ( signal/callback support )
   // ---- 
void             YAC_VCALL YAC_Object::yacRegisterSignal              (sUI, YAC_FunctionHandle)               { }
void             YAC_VCALL YAC_Object::yacGetSignalStringList         (YAC_String *)                          { }


   // ---- 
   // ---- 
   // ---- non-virtual helper methods
   // ---- 
   // ---- 
   // ---- 
#ifdef YAC_OBJECT_POOL
YAC_Object *     YAC_Object::yacNew                         (YAC_ContextHandle _context)            {YAC_Object*t=yacNewObject();if(t){t->pool_handle.pool_id = 0; t->yacOperatorInit(_context, this);} return t;}
#else
YAC_Object *     YAC_Object::yacNew                         (YAC_ContextHandle _context)            {YAC_Object*t=yacNewObject();if(t){t->yacOperatorInit(_context, this);} return t;}
#endif // YAC_OBJECT_POOL
#ifdef YAC_OBJECT_POOL
YAC_Object *     YAC_Object::yacNewPooled                   (YAC_ContextHandle _context, sUI _poolHint) {YAC_Object*t=yac_host->yacNewPooledByID(class_ID, _poolHint); if(t) { t->yacOperatorInit(_context, this); return t; } else { return yacNew(_context); } }
#else
YAC_Object *     YAC_Object::yacNewPooled                   (YAC_ContextHandle _context, sUI) { return yacNew(_context); }
#endif // YAC_OBJECT_POOL
// sBool            YAC_Object::yacCanDeserializeClass         (YAC_Object *_s)                        {YAC_String s;_s->yacStreamReadString(&s, 64);if( (yacClassName()&&s.compare((sChar*)yacClassName())) || (yacMetaClassName()&&s.compare((sChar*)yacMetaClassName())) ) {return 1;}else{_s->yacStreamSeek(-((sSI)s.length), YAC_CUR);/*expect ASCIIZ*/return 0;}}
sBool            YAC_Object::yacCanDeserializeClass         (YAC_Object *_s)                        {
   YAC_String *s = (YAC_String*) YAC_NEW_CORE_POOLED(YAC_CLID_STRING);
   _s->yacStreamReadString(s, 64);// calls host-specific string methods so we need a "real" String object !
   if( (yacClassName()&&s->compare((sChar*)yacClassName())) || (yacMetaClassName()&&s->compare((sChar*)yacMetaClassName())) ) 
   {
      YAC_DELETE(s);
      return 1;
   }
   else
   {
      _s->yacStreamSeek(-((sSI)s->length), YAC_CUR);/*expect ASCIIZ*/
      YAC_DELETE(s);
      return 0;
   }
}
sBool            YAC_Object::yacInstanceOf                  (YAC_Object *_o)                        {if(_o)return(yac_host->cpp_typecast_map[class_ID][_o->class_ID]);else return 0;}

#ifdef YAC_OBJECT_YAC
// ---- auto-generated by gen_yac_object_c_calls.tks (Fri, 21/Dec/2012 23:12:48)
void YAC_Object::_yacClassName(YAC_Value *_r) {
    yac_object_yacClassName(this, _r);
}
void YAC_Object::_yacNewObject(YAC_Value *_r) {
    yac_object_yacNewObject(this, _r);
}
sSI YAC_Object::_yacMemberGetNum(void) {
    return yac_object_yacMemberGetNum(this);
}
void YAC_Object::_yacMemberGetNames(YAC_Value *_r) {
    yac_object_yacMemberGetNames(this, _r);
}
void YAC_Object::_yacMemberGetTypes(YAC_Value *_r) {
    yac_object_yacMemberGetTypes(this, _r);
}
void YAC_Object::_yacMemberGetObjectTypes(YAC_Value *_r) {
    yac_object_yacMemberGetObjectTypes(this, _r);
}
void YAC_Object::_yacMemberGetOffsets(YAC_Value *_r) {
    yac_object_yacMemberGetOffsets(this, _r);
}
sSI YAC_Object::_yacMethodGetNum(void) {
    return yac_object_yacMethodGetNum(this);
}
void YAC_Object::_yacMethodGetNames(YAC_Value *_r) {
    yac_object_yacMethodGetNames(this, _r);
}
void YAC_Object::_yacMethodGetNumParameters(YAC_Value *_r) {
    yac_object_yacMethodGetNumParameters(this, _r);
}
void YAC_Object::_yacMethodGetParameterTypes(YAC_Value *_r) {
    yac_object_yacMethodGetParameterTypes(this, _r);
}
void YAC_Object::_yacMethodGetParameterObjectTypes(YAC_Value *_r) {
    yac_object_yacMethodGetParameterObjectTypes(this, _r);
}
void YAC_Object::_yacMethodGetReturnTypes(YAC_Value *_r) {
    yac_object_yacMethodGetReturnTypes(this, _r);
}
void YAC_Object::_yacMethodGetReturnObjectTypes(YAC_Value *_r) {
    yac_object_yacMethodGetReturnObjectTypes(this, _r);
}
void YAC_Object::_yacMethodGetAdr(YAC_Value *_r) {
    yac_object_yacMethodGetAdr(this, _r);
}
sSI YAC_Object::_yacConstantGetNum(void) {
    return yac_object_yacConstantGetNum(this);
}
void YAC_Object::_yacConstantGetNames(YAC_Value *_r) {
    yac_object_yacConstantGetNames(this, _r);
}
void YAC_Object::_yacConstantGetTypes(YAC_Value *_r) {
    yac_object_yacConstantGetTypes(this, _r);
}
void YAC_Object::_yacConstantGetValues(YAC_Value *_r) {
    yac_object_yacConstantGetValues(this, _r);
}
sSI YAC_Object::_yacCopy(YAC_Object *_os) {
    return yac_object_yacCopy(this, _os);
}
sSI YAC_Object::_yacEquals(YAC_Object *_ro) {
    return yac_object_yacEquals(this, _ro);
}
void YAC_Object::_yacOperator(sSI _cmd, YAC_Object *_ro, YAC_Value *_r) {
    yac_object_yacOperator(this, _cmd, _ro, _r);
}
void YAC_Object::_yacOperatorInit(YAC_Object *_ro) {
    yac_object_yacOperatorInit(this, _ro);
}
void YAC_Object::_yacOperatorAssign(YAC_Object *_ro) {
    yac_object_yacOperatorAssign(this, _ro);
}
void YAC_Object::_yacOperatorAdd(YAC_Object *_ro) {
    yac_object_yacOperatorAdd(this, _ro);
}
void YAC_Object::_yacOperatorSub(YAC_Object *_ro) {
    yac_object_yacOperatorSub(this, _ro);
}
void YAC_Object::_yacOperatorMul(YAC_Object *_ro) {
    yac_object_yacOperatorMul(this, _ro);
}
void YAC_Object::_yacOperatorDiv(YAC_Object *_ro) {
    yac_object_yacOperatorDiv(this, _ro);
}
void YAC_Object::_yacOperatorClamp(YAC_Object *_min, YAC_Object *_max) {
    yac_object_yacOperatorClamp(this, _min, _max);
}
void YAC_Object::_yacOperatorWrap(YAC_Object *_min, YAC_Object *_max) {
    yac_object_yacOperatorWrap(this, _min, _max);
}
sSI YAC_Object::_yacScanI32(YAC_Object *_vo) {
    return yac_object_yacScanI32(this, _vo);
}
sSI YAC_Object::_yacScanI64(YAC_Object *_vo) {
    return yac_object_yacScanI64(this, _vo);
}
sSI YAC_Object::_yacScanF32(YAC_Object *_vo) {
    return yac_object_yacScanF32(this, _vo);
}
sSI YAC_Object::_yacScanF64(YAC_Object *_vo) {
    return yac_object_yacScanF64(this, _vo);
}
sSI YAC_Object::_yacToString(YAC_Object *_s) const {
    return yac_object_yacToString(this, _s);
}
void YAC_Object::_yacOperatorI(sSI _cmd, sSI _i, YAC_Value *_r) {
    yac_object_yacOperatorI(this, _cmd, _i, _r);
}
void YAC_Object::_yacOperatorI64(sSI _cmd, YAC_Object *_no, YAC_Value *_r) {
    yac_object_yacOperatorI64(this, _cmd, _no, _r);
}
void YAC_Object::_yacOperatorF32(sSI _cmd, sF32 _f32, YAC_Value *_r) {
    yac_object_yacOperatorF32(this, _cmd, _f32, _r);
}
void YAC_Object::_yacOperatorF64(sSI _cmd, YAC_Object *_no, YAC_Value *_r) {
    yac_object_yacOperatorF64(this, _cmd, _no, _r);
}
void YAC_Object::_yacValueOfI(sSI _i) {
    yac_object_yacValueOfI(this, _i);
}
void YAC_Object::_yacValueOfF32(sF32 _f32) {
    yac_object_yacValueOfF32(this, _f32);
}
void YAC_Object::_yacValueOfF64(YAC_Object *_no) {
    yac_object_yacValueOfF64(this, _no);
}
void YAC_Object::_yacValueOfI64(YAC_Object *_no) {
    yac_object_yacValueOfI64(this, _no);
}
sSI YAC_Object::_yacToParsableString(YAC_Object *_s) const {
    return yac_object_yacToParsableString(this, _s);
}
sSI YAC_Object::_yacIsStream(void) {
    return yac_object_yacIsStream(this);
}
void YAC_Object::_yacStreamClose(void) {
    yac_object_yacStreamClose(this);
}
sSI YAC_Object::_yacStreamOpenLocal(YAC_Object *_name, sSI _access) {
    return yac_object_yacStreamOpenLocal(this, _name, _access);
}
sSI YAC_Object::_yacStreamOpenLogic(YAC_Object *_name) {
    return yac_object_yacStreamOpenLogic(this, _name);
}
sSI YAC_Object::_yacStreamGetByteOrder(void) {
    return yac_object_yacStreamGetByteOrder(this);
}
void YAC_Object::_yacStreamSetByteOrder(sSI _order) {
    yac_object_yacStreamSetByteOrder(this, _order);
}
sSI YAC_Object::_yacStreamEOF(void) {
    return yac_object_yacStreamEOF(this);
}
void YAC_Object::_yacStreamSeek(sSI _off, sSI _mode) {
    yac_object_yacStreamSeek(this, _off, _mode);
}
sSI YAC_Object::_yacStreamGetOffset(void) {
    return yac_object_yacStreamGetOffset(this);
}
void YAC_Object::_yacStreamSetOffset(sSI _off) {
    yac_object_yacStreamSetOffset(this, _off);
}
sSI YAC_Object::_yacStreamGetSize(void) {
    return yac_object_yacStreamGetSize(this);
}
sSI YAC_Object::_yacStreamRead(YAC_Object *_ret, sSI _num) {
    return yac_object_yacStreamRead(this, _ret, _num);
}
sSI YAC_Object::_yacStreamReadI8(void) {
    return yac_object_yacStreamReadI8(this);
}
sSI YAC_Object::_yacStreamReadI16(void) {
    return yac_object_yacStreamReadI16(this);
}
sSI YAC_Object::_yacStreamReadI32(void) {
    return yac_object_yacStreamReadI32(this);
}
void YAC_Object::_yacStreamReadI64(YAC_Value *_r) {
    yac_object_yacStreamReadI64(this, _r);
}
sF32 YAC_Object::_yacStreamReadF32(void) {
    return yac_object_yacStreamReadF32(this);
}
void YAC_Object::_yacStreamReadF64(YAC_Value *_r) {
    yac_object_yacStreamReadF64(this, _r);
}
void YAC_Object::_yacStreamReadObject(YAC_Object *_p) {
    yac_object_yacStreamReadObject(this, _p);
}
sSI YAC_Object::_yacStreamReadString(YAC_Object *_s, sSI _maxlen) {
    return yac_object_yacStreamReadString(this, _s, _maxlen);
}
sSI YAC_Object::_yacStreamReadBuffer(YAC_Object *_buffer, sSI _off, sSI _num, sSI _resize) {
    return yac_object_yacStreamReadBuffer(this, _buffer, _off, _num, _resize);
}
sSI YAC_Object::_yacStreamReadLine(YAC_Object *_s, sSI _maxlen) {
    return yac_object_yacStreamReadLine(this, _s, _maxlen);
}
sSI YAC_Object::_yacStreamWrite(YAC_Object *_in, sSI _num) {
    return yac_object_yacStreamWrite(this, _in, _num);
}
void YAC_Object::_yacStreamWriteI8(sSI _i) {
    yac_object_yacStreamWriteI8(this, _i);
}
void YAC_Object::_yacStreamWriteI16(sSI _i) {
    yac_object_yacStreamWriteI16(this, _i);
}
void YAC_Object::_yacStreamWriteI32(sSI _i) {
    yac_object_yacStreamWriteI32(this, _i);
}
void YAC_Object::_yacStreamWriteI64(YAC_Object *_no) {
    yac_object_yacStreamWriteI64(this, _no);
}
void YAC_Object::_yacStreamWriteF32(sF32 _f) {
    yac_object_yacStreamWriteF32(this, _f);
}
void YAC_Object::_yacStreamWriteF64(YAC_Object *_no) {
    yac_object_yacStreamWriteF64(this, _no);
}
void YAC_Object::_yacStreamWriteObject(YAC_Object *_p) {
    yac_object_yacStreamWriteObject(this, _p);
}
sSI YAC_Object::_yacStreamWriteString(YAC_Object *_s, sSI _off, sSI _num) {
    return yac_object_yacStreamWriteString(this, _s, _off, _num);
}
sSI YAC_Object::_yacStreamWriteBuffer(YAC_Object *_b, sSI _off, sSI _num) {
    return yac_object_yacStreamWriteBuffer(this, _b, _off, _num);
}
sSI YAC_Object::_yacStreamGetErrorCode(void) {
    return yac_object_yacStreamGetErrorCode(this);
}
void YAC_Object::_yacStreamGetErrorStringByCode(sSI _code, YAC_Value *_r) {
    yac_object_yacStreamGetErrorStringByCode(this, _code, _r);
}
void YAC_Object::_yacSerializeClassName(YAC_Object *_ofs) {
    yac_object_yacSerializeClassName(this, _ofs);
}
void YAC_Object::_yacSerialize(YAC_Object *_ofs, sSI _usetypeinfo) {
    yac_object_yacSerialize(this, _ofs, _usetypeinfo);
}
sSI YAC_Object::_yacDeserialize(YAC_Object *_ifs, sSI _usetypeinfo) {
    return yac_object_yacDeserialize(this, _ifs, _usetypeinfo);
}
void YAC_Object::_yacArrayNew(YAC_Value *_r) {
    yac_object_yacArrayNew(this, _r);
}
sSI YAC_Object::_yacArrayAlloc(sSI _sx, sSI _sy, sSI _type, sSI _ebytesize) {
    return yac_object_yacArrayAlloc(this, _sx, _sy, _type, _ebytesize);
}
sSI YAC_Object::_yacArrayRealloc(sSI _sx, sSI _sy, sSI _type, sSI _ebytesize) {
    return yac_object_yacArrayRealloc(this, _sx, _sy, _type, _ebytesize);
}
sSI YAC_Object::_yacArrayGetNumElements(void) {
    return yac_object_yacArrayGetNumElements(this);
}
sSI YAC_Object::_yacArrayGetMaxElements(void) {
    return yac_object_yacArrayGetMaxElements(this);
}
void YAC_Object::_yacArrayCopySize(YAC_Object *_p) {
    yac_object_yacArrayCopySize(this, _p);
}
void YAC_Object::_yacArraySet(sSI _index, YAC_Object *_value) {
    yac_object_yacArraySet(this, _index, _value);
}
void YAC_Object::_yacArrayGet(sSI _index, YAC_Value *_r) {
    yac_object_yacArrayGet(this, _index, _r);
}
sSI YAC_Object::_yacArrayGetWidth(void) {
    return yac_object_yacArrayGetWidth(this);
}
sSI YAC_Object::_yacArrayGetHeight(void) {
    return yac_object_yacArrayGetHeight(this);
}
sSI YAC_Object::_yacArrayGetElementType(void) {
    return yac_object_yacArrayGetElementType(this);
}
sSI YAC_Object::_yacArrayGetElementByteSize(void) {
    return yac_object_yacArrayGetElementByteSize(this);
}
sSI YAC_Object::_yacArrayGetStride(void) {
    return yac_object_yacArrayGetStride(this);
}
sSI YAC_Object::_yacArrayGetPointer(void) {
    return yac_object_yacArrayGetPointer(this);
}
void YAC_Object::_yacArraySetWidth(sSI _width) {
    yac_object_yacArraySetWidth(this, _width);
}
void YAC_Object::_yacArraySetTemplate(YAC_Object *_template) {
    yac_object_yacArraySetTemplate(this, _template);
}
void YAC_Object::_yacArrayGetDeref(sSI _index, YAC_Value *_r) {
    yac_object_yacArrayGetDeref(this, _index, _r);
}
void YAC_Object::_yacHashSet(YAC_Object *_key, YAC_Object *_value) {
    yac_object_yacHashSet(this, _key, _value);
}
void YAC_Object::_yacHashGet(YAC_Object *_key, YAC_Value *_r) {
    yac_object_yacHashGet(this, _key, _r);
}
void YAC_Object::_yacHashGetDeref(YAC_Object *_key, YAC_Value *_r) {
    yac_object_yacHashGetDeref(this, _key, _r);
}
void YAC_Object::_yacGetSignalStringList(YAC_Object *_s) {
    yac_object_yacGetSignalStringList(this, _s);
}
void YAC_Object::_yacMetaClassName(YAC_Value *_r) {
    yac_object_yacMetaClassName(this, _r);
}
sSI YAC_Object::_yacMetaClassMemberGetNum(void) {
    return yac_object_yacMetaClassMemberGetNum(this);
}
sSI YAC_Object::_yacMetaClassMemberGetAccessKeyByIndex(sSI _index) {
    return yac_object_yacMetaClassMemberGetAccessKeyByIndex(this, _index);
}
sSI YAC_Object::_yacMetaClassMemberGetAccessKeyByName(YAC_Object *_s) {
    return yac_object_yacMetaClassMemberGetAccessKeyByName(this, _s);
}
sSI YAC_Object::_yacMetaClassMemberGetType(sSI _ak) {
    return yac_object_yacMetaClassMemberGetType(this, _ak);
}
void YAC_Object::_yacMetaClassMemberGetName(sSI _ak, YAC_Value *_r) {
    yac_object_yacMetaClassMemberGetName(this, _ak, _r);
}
void YAC_Object::_yacMetaClassMemberSet(sSI _ak, YAC_Object *_value) {
    yac_object_yacMetaClassMemberSet(this, _ak, _value);
}
void YAC_Object::_yacMetaClassMemberGet(sSI _ak, YAC_Value *_r) {
    yac_object_yacMetaClassMemberGet(this, _ak, _r);
}
sSI YAC_Object::_yacMetaClassInstanceOf(YAC_Object *_o) {
    return yac_object_yacMetaClassInstanceOf(this, _o);
}
void YAC_Object::_yacNew(YAC_Value *_r) {
    yac_object_yacNew(this, _r);
}
sSI YAC_Object::_yacCanDeserializeClass(YAC_Object *_ifs) {
    return yac_object_yacCanDeserializeClass(this, _ifs);
}
sSI YAC_Object::_yacInstanceOf(YAC_Object *_o) {
    return yac_object_yacInstanceOf(this, _o);
}
#endif

#ifdef YAC_OBJECT_YAC
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ 
//~                                                                ~ 
//~ YAC "C" interface for YAC_Object baseclass                     ~ 
//~                                                                ~ 
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ 
void YAC_CALL yac_object_yacClassName(YAC_Object *_o, YAC_Value *_r) {
    YAC_String *s=(YAC_String*)yac_host->yacNewByID(YAC_CLID_STRING);
    s->visit((sChar*)_o->yacClassName());
    YAC_RETS(s, 1);
}

void YAC_CALL yac_object_yacNewObject(YAC_Object *_o, YAC_Value *_r) {
    YAC_Object *o=_o->yacNewObject();
    YAC_RETO(o, (o!=0));
}

// ---- m e m b e r s

sSI YAC_CALL yac_object_yacMemberGetNum(YAC_Object *_o) {
    return (sSI)_o->yacMemberGetNum();
}

void YAC_CALL yac_object_yacMemberGetNames(YAC_Object *_o, YAC_Value *_r) {
    YAC_StringArray *a=(YAC_StringArray*)yac_host->yacNewByID(YAC_CLID_STRINGARRAY);
    if(a)
    {
        sSI num=(sSI)_o->yacMemberGetNum();
        if(num&&a->yacArrayAlloc(num, 0,0,0))
        {
            a->num_elements=a->max_elements;
            const char **memberNames=_o->yacMemberGetNames();
            for(sSI i=0; i<num; i++)
            {
                a->elements[i].visit((sChar*)memberNames[i]);
            }
        }
    }
    YAC_RETO(a, (a!=0));
}

void YAC_CALL yac_object_yacMemberGetTypes(YAC_Object *_o, YAC_Value *_r) {
    YAC_IntArray *a=(YAC_IntArray*)yac_host->yacNewByID(YAC_CLID_INTARRAY);
    if(a)
    {
        sSI num=(int)_o->yacMemberGetNum();
        if(num&&a->yacArrayAlloc(num, 0,0,0))
        {
            a->num_elements=a->max_elements;
            const sUI *memberTypes=_o->yacMemberGetTypes();
            for(sSI i=0; i<num; i++)
            {
                a->elements[i]=(sSI)memberTypes[i];
            }
        }
    }
    YAC_RETO(a, (a!=0));
}

void YAC_CALL yac_object_yacMemberGetObjectTypes(YAC_Object *_o, YAC_Value *_r) {
    YAC_StringArray *a=(YAC_StringArray*)yac_host->yacNewByID(YAC_CLID_STRINGARRAY);
    if(a)
    {
        sSI num=(int)_o->yacMemberGetNum();
        if(num&&a->yacArrayAlloc(num, 0,0,0))
        {
            a->num_elements=a->max_elements;
            const char **objectTypes=_o->yacMemberGetObjectTypes();
            for(sSI i=0; i<num; i++)
            {
                a->elements[i].visit((sChar*)objectTypes[i]);
            }
        }
    }
    YAC_RETO(a, (a!=0));
}

void YAC_CALL yac_object_yacMemberGetOffsets(YAC_Object *_o, YAC_Value *_r) {
    YAC_IntArray *a=(YAC_IntArray*)yac_host->yacNewByID(YAC_CLID_INTARRAY);
    if(a)
    {
        sSI num=(int)_o->yacMemberGetNum();
        if(num&&a->yacArrayAlloc(num, 0,0,0))
        {
            a->num_elements=a->max_elements;
            const sU8**memberOffsets = _o->yacMemberGetOffsets();
            for(sSI i=0; i<num; i++)
            {
#ifdef YAC_64
               a->elements[i]=(sSI) ( (*(sU64*)&memberOffsets[i]) & 0xFFFFFFF );
#else
               a->elements[i]=(sSI) ( (*(sU32*)&memberOffsets[i]) & 0xFFFFFFF );
#endif // YAC_64
            }
        }
    }
    YAC_RETO(a, (a!=0));
}

// ---- m e t h o d s

sSI YAC_CALL yac_object_yacMethodGetNum(YAC_Object *_o) {
    return (sSI)_o->yacMethodGetNum();
}

void YAC_CALL yac_object_yacMethodGetNames(YAC_Object *_o, YAC_Value *_r) {
    YAC_StringArray *a=(YAC_StringArray*)yac_host->yacNewByID(YAC_CLID_STRINGARRAY);
    if(a)
    {
        sSI num=(int)_o->yacMethodGetNum();
        if(num&&a->yacArrayAlloc(num, 0,0,0))
        {
            a->num_elements=a->max_elements;
            const char **methodNames=_o->yacMethodGetNames();
            for(sSI i=0; i<num; i++)
            {
                a->elements[i].visit((sChar*)methodNames[i]);
            }
        }
    }
    YAC_RETO(a, (a!=0));
}

void YAC_CALL yac_object_yacMethodGetNumParameters(YAC_Object *_o, YAC_Value *_r) {
    YAC_IntArray *a=(YAC_IntArray*)yac_host->yacNewByID(YAC_CLID_INTARRAY);
    if(a)
    {
        sSI num=(int)_o->yacMethodGetNum();
        if(num&&a->yacArrayAlloc(num, 0,0,0))
        {
            a->num_elements=a->max_elements;
            const sUI *numParameters=_o->yacMethodGetNumParameters();
            for(sSI i=0; i<num; i++)
            {
                a->elements[i]=(sSI)numParameters[i];
            }
        }
    }
    YAC_RETO(a, (a!=0));
}

void YAC_CALL yac_object_yacMethodGetParameterTypes(YAC_Object *_o, YAC_Value *_r) {
    YAC_ValueArray *a=(YAC_ValueArray*)yac_host->yacNewByID(YAC_CLID_VALUEARRAY);
    if(a)
    {
        sSI num=(int)_o->yacMethodGetNum();
        if(num&&a->yacArrayAlloc(num, 0,0,0))
        {
             a->num_elements=a->max_elements;
             const sUI  *numParameters =_o->yacMethodGetNumParameters();
             const sUI **allParameterTypes=_o->yacMethodGetParameterTypes();
             for(sSI i=0; i<num; i++)
             {
                 YAC_IntArray *ia=(YAC_IntArray*)yac_host->yacNewByID(YAC_CLID_INTARRAY);
                 if(ia)
                 {
                     a->elements[i].initObject(ia, 1);
                     sSI inum=(sSI)numParameters[i];
                     if(inum&&ia->yacArrayAlloc(inum, 0,0,0))
                     {
                        ia->num_elements=ia->max_elements;
                        const sUI *parameterTypes=allParameterTypes[i];
                        for(sSI j=0; j<inum; j++)
                        {
                           ia->elements[j] =(sSI)parameterTypes[j];
                        }
                     }
                 }
                 else
                 {
                     yac_host->printf("[---] yac_object_yacMethodGetParameterTypes: cannot allocate inner array %i/%i.\n", i, num);
                     yac_host->yacDelete(a);
                     YAC_RETO(0, 0);
                     return;
                 }
             }
        }
    }
    YAC_RETO(a, (a!=0));
}

void YAC_CALL yac_object_yacMethodGetParameterObjectTypes(YAC_Object *_o, YAC_Value *_r) {
    YAC_ValueArray *a=(YAC_ValueArray*)yac_host->yacNewByID(YAC_CLID_VALUEARRAY);
    if(a)
    {
        sSI num=(int)_o->yacMethodGetNum();
        if(num&&a->yacArrayAlloc(num, 0,0,0))
        {
            a->num_elements=a->max_elements;
            const sUI    *numParameters =_o->yacMethodGetNumParameters();
            const char ***allParameterObjectTypes=_o->yacMethodGetParameterObjectTypes();
            for(sSI i=0; i<num; i++)
            {
                YAC_StringArray *ia=(YAC_StringArray*)yac_host->yacNewByID(YAC_CLID_STRINGARRAY);
                if(ia)
                {
                    a->elements[i].initObject(ia, 1);
                    sSI inum=(sSI)numParameters[i];
                    if(inum&&ia->yacArrayAlloc(inum, 0,0,0))
                    {
                        ia->num_elements=ia->max_elements;
                        const char **parameterObjectTypes=allParameterObjectTypes[i];
                        for(sSI j=0; j<inum; j++)
			  {
//               yac_host->printf("ia->elements[%i]=%p (%s) *s=%c\n", j, parameterObjectTypes[j],
//                                (char *)parameterObjectTypes[j],
//                                parameterObjectTypes[j]?parameterObjectTypes[j][0]:'?');
              ia->elements[j].visit((sChar*)parameterObjectTypes[j]);
			  } 
                    }
                }
                else
                {
                    yac_host->printf("[---] yac_object_yacMethodGetParameterObjectTypes: cannot allocate inner array %i/%i.\n", i, num);
                    yac_host->yacDelete(a);
                    YAC_RETO(0, 0);
                    return;
                }
            }
        }
    }
    YAC_RETO(a, (a!=0));
}

void YAC_CALL yac_object_yacMethodGetReturnTypes(YAC_Object *_o, YAC_Value *_r) {
    YAC_IntArray *a=(YAC_IntArray*)yac_host->yacNewByID(YAC_CLID_INTARRAY);
    if(a)
    {
        sSI num=(int)_o->yacMethodGetNum();
        if(num&&a->yacArrayAlloc(num, 0,0,0))
        {
            a->num_elements=a->max_elements;
            const sUI *returnTypes=_o->yacMethodGetReturnTypes();
            for(sSI i=0; i<num; i++)
                a->elements[i]=(sSI)returnTypes[i];
        }
    }
    YAC_RETO(a, (a!=0));
}

void YAC_CALL yac_object_yacMethodGetReturnObjectTypes(YAC_Object *_o, YAC_Value *_r) {
    YAC_StringArray *a=(YAC_StringArray*)yac_host->yacNewByID(YAC_CLID_STRINGARRAY);
    if(a)
    {
        sSI num=(int)_o->yacMethodGetNum();
        if(num&&a->yacArrayAlloc(num, 0,0,0))
        {
            a->num_elements=a->max_elements;
            const char **returnObjectTypes=_o->yacMethodGetReturnObjectTypes();
            for(sSI i=0; i<num; i++)
                a->elements[i].visit((sChar*)returnObjectTypes[i]);
        }
    }
    YAC_RETO(a, (a!=0));
}

void YAC_CALL yac_object_yacMethodGetAdr(YAC_Object *_o, YAC_Value *_r) {
    YAC_IntArray *a=(YAC_IntArray*)yac_host->yacNewByID(YAC_CLID_INTARRAY);
    if(a)
    {
        sSI num=(int)_o->yacMethodGetNum();
        if(num&&a->yacArrayAlloc(num, 0,0,0))
        {
            a->num_elements=a->max_elements;
            const void **methodAdr=_o->yacMethodGetAdr();
            for(sSI i=0; i<num; i++)
                a->elements[i]=*(sSI*)&methodAdr[i];
        }
    }
    YAC_RETO(a, (a!=0));
}

// ---- c o n s t a n t s

sSI YAC_CALL yac_object_yacConstantGetNum(YAC_Object *_o) {
    return (sSI)_o->yacConstantGetNum();
}

void YAC_CALL yac_object_yacConstantGetNames(YAC_Object *_o, YAC_Value *_r) {
    YAC_StringArray *a=(YAC_StringArray*)yac_host->yacNewByID(YAC_CLID_STRINGARRAY);
    if(a)
    {
        sSI num=(sSI)_o->yacConstantGetNum();
        if(num&&a->yacArrayAlloc(num, 0,0,0))
        {
            a->num_elements=a->max_elements;
            const char **constantNames=_o->yacConstantGetNames();
            for(sSI i=0; i<num; i++)
                a->elements[i].visit((sChar*)constantNames[i]);
        }
    }
    YAC_RETO(a, (a!=0));
}

void YAC_CALL yac_object_yacConstantGetTypes(YAC_Object *_o, YAC_Value *_r) {
    YAC_IntArray *a=(YAC_IntArray*)yac_host->yacNewByID(YAC_CLID_INTARRAY);
    if(a)
    {
        sSI num=(int)_o->yacConstantGetNum();
        if(num&&a->yacArrayAlloc(num, 0,0,0))
        {
            a->num_elements=a->max_elements;
            const sUI *constantTypes=_o->yacConstantGetTypes();
            for(sSI i=0; i<num; i++)
                a->elements[i]=(sSI)constantTypes[i];
        }
    }
    YAC_RETO(a, (a!=0));
}

void YAC_CALL yac_object_yacConstantGetValues(YAC_Object *_o, YAC_Value *_r) {
    YAC_ValueArray *a=(YAC_ValueArray*)yac_host->yacNewByID(YAC_CLID_VALUEARRAY);
    if(a)
    {
        sSI num=(int)_o->yacConstantGetNum();
        if(num&&a->yacArrayAlloc(num, 0,0,0))
        {
            a->num_elements=a->max_elements;
            const sUI *constantTypes=_o->yacConstantGetTypes();
            yacmemptr constantValues=_o->yacConstantGetValues();
            for(sSI i=0; i<num; i++)
            {
                switch(constantTypes[i])
                {
                case YAC_TYPE_INT:
                    a->elements[i].initInt(constantValues.si[i]);
                    break;
                case YAC_TYPE_FLOAT:
                    a->elements[i].initFloat(constantValues.f4[i]); // xxx 32bit float assumed
                    break;
                case YAC_TYPE_OBJECT:
                default:
                    a->elements[i].initObject(0, 0);
                    yac_host->printf("[---] yac_object_yacConstantGetValues(): constant %i has YAC_TYPE_OBJECT(%i) (not supported).\n", i, constantTypes[i]);
                    break;
                }
            }
        }
    }
    YAC_RETO(a, (a!=0));
}

// ---- o p e r a t o r s

sSI YAC_CALL yac_object_yacCopy(YAC_Object *_o, YAC_Object *_os) {
    return _o->yacCopy(_os);
}

sSI YAC_CALL yac_object_yacEquals(YAC_Object *_o, YAC_Object *_ro) {
    return _o->yacEquals(_ro);
}

void YAC_CALL yac_object_yacOperator(YAC_Object *_o, sSI _cmd, YAC_Object *_ro, YAC_Value *_r) {
    _o->yacOperator(_cmd, _ro, _r);
}

void YAC_CALL yac_object_yacOperatorInit(YAC_Object *_o, YAC_Object *_ro) {
   _o->yacOperatorInit(NULL/*_context*/, _ro);
}

void YAC_CALL yac_object_yacOperatorAssign(YAC_Object *_o, YAC_Object *_ro) {
    _o->yacOperatorAssign(_ro);
}

void YAC_CALL yac_object_yacOperatorAdd(YAC_Object *_o, YAC_Object *_ro) {
    _o->yacOperatorAdd(_ro);
}

void YAC_CALL yac_object_yacOperatorSub(YAC_Object *_o, YAC_Object *_ro) {
    _o->yacOperatorSub(_ro);
}

void YAC_CALL yac_object_yacOperatorMul(YAC_Object *_o, YAC_Object *_ro) {
    _o->yacOperatorMul(_ro);
}

void YAC_CALL yac_object_yacOperatorDiv(YAC_Object *_o, YAC_Object *_ro) {
    _o->yacOperatorDiv(_ro);
}

void YAC_CALL yac_object_yacOperatorClamp(YAC_Object *_o, YAC_Object *_min, YAC_Object *_max) {
    _o->yacOperatorClamp(_min, _max);
}

void YAC_CALL yac_object_yacOperatorWrap(YAC_Object *_o, YAC_Object *_min, YAC_Object *_max) {
    _o->yacOperatorWrap(_min, _max);
}

sSI YAC_CALL yac_object_yacScanI32(YAC_Object *_o, YAC_Object *_vo) {
    if(YAC_BCHK(_vo, YAC_CLID_INTEGER)) // xxx should be YAC_CLID_LONG
    {
        YAC_Integer *vo=(YAC_Integer*)_vo;
        return _o->yacScanI(&vo->value);
    }
    return 0;
}

sSI YAC_CALL yac_object_yacScanI64(YAC_Object *_o, YAC_Object *_vo) {
   if(YAC_VALID(_vo))
   {
      sS64 i64;
      if(_o->yacScanI64(&i64))
      {
         _vo->yacValueOfI64(i64);
      }
   }
    return 0;
}

sSI YAC_CALL yac_object_yacScanF32(YAC_Object *_o, YAC_Object *_vo) {
    if(YAC_BCHK(_vo, YAC_CLID_FLOAT))
    {
        YAC_Float *vo=(YAC_Float*)_vo;
        return _o->yacScanF32(&vo->value);
    }
    return 0;
}

sSI YAC_CALL yac_object_yacScanF64(YAC_Object *_o, YAC_Object *_vo) {
   if(YAC_VALID(_vo))
   {
      sF64 f64;
      if(_o->yacScanF64(&f64))
      {
         _vo->yacValueOfF64(f64);
      }
   }
   return 0;
}

sSI YAC_CALL yac_object_yacToString(const YAC_Object *_o, YAC_Object *_s) {
    if(YAC_BCHK(_s, YAC_CLID_STRING))
    {
        YAC_String *s=(YAC_String*)_s;
        return _o->yacToString(s);
    }
    return 0;
}

void YAC_CALL yac_object_yacOperatorI(YAC_Object *_o, sSI _cmd, sSI _i, YAC_Value *_r) {
    _o->yacOperatorI(_cmd, _i, _r);
}

void YAC_CALL yac_object_yacOperatorI64(YAC_Object *_o, sSI _cmd, YAC_Object *_no, YAC_Value *_r) {
   if(YAC_VALID(_no))
   {
      sS64 i64;
      if(_no->yacScanI64(&i64))
      {
         _o->yacOperatorI64(_cmd, i64, _r);
      }
   }
}

void YAC_CALL yac_object_yacOperatorF32(YAC_Object *_o, sSI _cmd, sF32 _f32, YAC_Value *_r) {
    _o->yacOperatorF32(_cmd, _f32, _r);
}

void YAC_CALL yac_object_yacOperatorF64(YAC_Object *_o, sSI _cmd, YAC_Object *_no, YAC_Value *_r) {
   if(YAC_VALID(_no))
   {
      sF64 f64;
      if(_no->yacScanF64(&f64))
      {
         _o->yacOperatorF64(_cmd, f64, _r);
      }
   }
}

void YAC_CALL yac_object_yacValueOfI(YAC_Object *_o, sSI _i) {
    _o->yacValueOfI(_i);
}

void YAC_CALL yac_object_yacValueOfI64(YAC_Object *_o, YAC_Object *_no) {
   if(YAC_VALID(_no))
   {
      sS64 i64;
      if(_no->yacScanI64(&i64))
      {
         _o->yacValueOfI64(i64);
      }
   }
}

void YAC_CALL yac_object_yacValueOfF32(YAC_Object *_o, sF32 _f32) {
    _o->yacValueOfF32(_f32);
}

void YAC_CALL yac_object_yacValueOfF64(YAC_Object *_o, YAC_Object *_no) {
   if(YAC_VALID(_no))
   {
      sF64 f64;
      if(_no->yacScanF64(&f64))
      {
         _o->yacValueOfF64(f64);
      }
   }
}

sSI YAC_CALL yac_object_yacToParsableString(const YAC_Object *_o, YAC_Object *_s) {
    if(YAC_BCHK(_s, YAC_CLID_STRING))
    {
        YAC_String *s=(YAC_String*)_s;
        return _o->yacToParsableString(s);
    }
    return 0;
}

// ---- s t r e a m s

sSI YAC_CALL yac_object_yacIsStream(YAC_Object *_o) {
    return _o->yacIsStream();
}

void YAC_CALL yac_object_yacStreamClose(YAC_Object *_o) {
    _o->yacStreamClose();
}

sSI YAC_CALL yac_object_yacStreamOpenLocal(YAC_Object *_o, YAC_Object *_name, sSI _access) {
    if(YAC_BCHK(_name, YAC_CLID_STRING))
    {
        return _o->yacStreamOpenLocal((sChar*)((YAC_String*)_name)->chars, _access);
    }
    return 0;
}

sSI YAC_CALL yac_object_yacStreamOpenLogic(YAC_Object *_o, YAC_Object *_name) {
    if(YAC_BCHK(_name, YAC_CLID_STRING))
    {
        return _o->yacStreamOpenLogic((sChar*)((YAC_String*)_name)->chars);
    }
    return 0;

}

sSI YAC_CALL yac_object_yacStreamGetByteOrder(YAC_Object *_o) {
    return _o->yacStreamGetByteOrder();
}

void YAC_CALL yac_object_yacStreamSetByteOrder(YAC_Object *_o, sSI _order) {
    _o->yacStreamSetByteOrder((sUI)_order);
}

sSI YAC_CALL yac_object_yacStreamEOF(YAC_Object *_o) {
    return _o->yacStreamEOF();
}

void YAC_CALL yac_object_yacStreamSeek(YAC_Object *_o, sSI _off, sSI _mode) {
    _o->yacStreamSeek(_off, (sUI)_mode);
}

sSI YAC_CALL yac_object_yacStreamGetOffset(YAC_Object *_o) {
    return _o->yacStreamGetOffset();
}

void YAC_CALL yac_object_yacStreamSetOffset(YAC_Object *_o, sSI _off) {
    _o->yacStreamSetOffset((sUI)_off);
}

sSI YAC_CALL yac_object_yacStreamGetSize(YAC_Object *_o) {
    return _o->yacStreamGetSize();
}

sSI YAC_CALL yac_object_yacStreamRead(YAC_Object *_o, YAC_Object *_ret, sSI _num) {
    if(YAC_BCHK(_ret, YAC_CLID_BUFFER))
    {
        YAC_Buffer *ret=(YAC_Buffer*)_ret;
        if(ret->yacArrayAlloc(_num, 0,0,0))
        {
            sUI num=(sUI)_num;
	    sUI i=0;
            for(; (!_o->yacStreamEOF())&&(i<num); i++)
                ret->yacStreamWriteI8(_o->yacStreamReadI8());
            return i;
        }
    }
    return 0;
}

sSI YAC_CALL yac_object_yacStreamReadI8(YAC_Object *_o) {
    return (sSI)_o->yacStreamReadI8();
}

sSI YAC_CALL yac_object_yacStreamReadI16(YAC_Object *_o) {
    return (sSI)_o->yacStreamReadI16();
}

sSI YAC_CALL yac_object_yacStreamReadI32(YAC_Object *_o) {
    return (sSI)_o->yacStreamReadI32();
}

void YAC_CALL yac_object_yacStreamReadI64(YAC_Object *_o, YAC_Value *_r) {
   YAC_Long *l = YAC_New_Long();
   l->value = _o->yacStreamReadI64();
   _r->initObject(l, 1);
}

sF32 YAC_CALL yac_object_yacStreamReadF32(YAC_Object *_o) {
    return (sF32)_o->yacStreamReadF32();
}

void YAC_CALL yac_object_yacStreamReadF64(YAC_Object *_o, YAC_Value *_r) {
   YAC_Double *d = YAC_New_Double();
   d->value = _o->yacStreamReadF64();
   _r->initObject(d, 1);
}

void YAC_CALL yac_object_yacStreamReadObject(YAC_Object *_o, YAC_Object *_p) {
    _o->yacStreamReadObject(_p);
}

sSI YAC_CALL yac_object_yacStreamReadString(YAC_Object *_o, YAC_Object *_s, sSI _maxlen) {
    if(YAC_BCHK(_s, YAC_CLID_STRING))
    {
        return _o->yacStreamReadString((YAC_String*)_s, (sUI)_maxlen);
    }
    return 0;
}

sSI YAC_CALL yac_object_yacStreamReadBuffer(YAC_Object *_o, YAC_Object *_buffer, sSI _off, sSI _num, sSI _resize) {
    if(YAC_BCHK(_buffer, YAC_CLID_BUFFER))
    {
        return _o->yacStreamReadBuffer((YAC_Buffer*)_buffer, (sUI)_off, (sUI)_num, _resize);
    }
    return 0;
}

sSI YAC_CALL yac_object_yacStreamReadLine(YAC_Object *_o, YAC_Object *_s, sSI _maxlen) {
    if(YAC_BCHK(_s, YAC_CLID_STRING))
    {
        return _o->yacStreamReadLine((YAC_String*)_s, (sUI)_maxlen);
    }
    return 0;
}

sSI YAC_CALL yac_object_yacStreamWrite(YAC_Object *_o, YAC_Object *_in, sSI _num) {
    if(YAC_BCHK(_in, YAC_CLID_BUFFER))
    {
        YAC_Buffer *in=(YAC_Buffer*)_in;
        sUI num=(sUI)_num;
        if(in->yacStreamGetSize()<num)
            num=in->yacStreamGetSize();
	in->yacStreamSetOffset(0);
	sUI i=0;
        for(; (!_o->yacStreamGetErrorCode())&&(i<num); i++)
            _o->yacStreamWriteI8( in->yacStreamReadI8() );
        return i;
    }
    return 0;
}

void YAC_CALL yac_object_yacStreamWriteI8(YAC_Object *_o, sSI _i) {
    _o->yacStreamWriteI8((sS8)_i);
}

void YAC_CALL yac_object_yacStreamWriteI16(YAC_Object *_o, sSI _i) {
    _o->yacStreamWriteI16((sS16)_i);
}

void YAC_CALL yac_object_yacStreamWriteI32(YAC_Object *_o, sSI _i) {
    _o->yacStreamWriteI32((sS32)_i);
}

void YAC_CALL yac_object_yacStreamWriteI64(YAC_Object *_o, YAC_Object *_no) {
   if(YAC_VALID(_no))
   {
      sS64 i64;
      if(_no->yacScanI64(&i64))
      {
         _o->yacStreamWriteI64(i64);
      }
   }
}

void YAC_CALL yac_object_yacStreamWriteF32(YAC_Object *_o, sF32 _f) {
    _o->yacStreamWriteF32((sF32)_f);
}

void YAC_CALL yac_object_yacStreamWriteF64(YAC_Object *_o, YAC_Object *_no) {
   if(YAC_VALID(_no))
   {
      sF64 f64;
      if(_no->yacScanF64(&f64))
      {
         _o->yacStreamWriteF64(f64);
      }
   }
}

void YAC_CALL yac_object_yacStreamWriteObject(YAC_Object *_o, YAC_Object *_p) {
    _o->yacStreamWriteObject(_p);
}

sSI YAC_CALL yac_object_yacStreamWriteString(YAC_Object *_o, YAC_Object *_s, sSI _off, sSI _num) {
    if(YAC_BCHK(_s, YAC_CLID_STRING))
    {
        return _o->yacStreamWriteString((YAC_String*)_s, (sUI)_off, (sUI)_num);
    }
    return 0;
}

sSI YAC_CALL yac_object_yacStreamWriteBuffer(YAC_Object *_o, YAC_Object *_b, sSI _off, sSI _num) {
    if(YAC_BCHK(_b, YAC_CLID_BUFFER))
    {
        return _o->yacStreamWriteBuffer((YAC_Buffer *)_b, (sUI)_off, (sUI)_num);
    }
    return 0;
}

sSI YAC_CALL yac_object_yacStreamGetErrorCode(YAC_Object *_o) {
    return _o->yacStreamGetErrorCode();
}

void YAC_CALL yac_object_yacStreamGetErrorStringByCode(YAC_Object *_o, sSI _code, YAC_Value *_r) {
    _o->yacStreamGetErrorStringByCode(_code, _r);
}

// ---- s e r i a l i z a t i o n

void YAC_CALL yac_object_yacSerializeClassName(YAC_Object *_o, YAC_Object *_ofs) {
    _o->yacSerializeClassName(_ofs);
}

void YAC_CALL yac_object_yacSerialize(YAC_Object *_o, YAC_Object *_ofs, sSI _usetypeinfo) {
    _o->yacSerialize(_ofs, (sUI)_usetypeinfo);
}

sSI YAC_CALL yac_object_yacDeserialize(YAC_Object *_o, YAC_Object *_ifs, sSI _usetypeinfo) {
    return _o->yacDeserialize(_ifs, (sUI)_usetypeinfo);
}

// ---- i t e r a t o r s

// (( skipped ))

// ---- a r r a y s   /   h a s h t a b l e s

void YAC_CALL yac_object_yacArrayNew(YAC_Object *_o, YAC_Value *_r) {
    YAC_Object *n=_o->yacArrayNew();
    _r->initObject(n, (n!=0));
}

sSI YAC_CALL yac_object_yacArrayAlloc(YAC_Object *_o, sSI _sx, sSI _sy, sSI _type, sSI _ebytesize) {
    return (sSI)_o->yacArrayAlloc((sUI)_sx, (sUI)_sy, (sUI)_type, (sUI)_ebytesize);
}

sSI YAC_CALL yac_object_yacArrayRealloc(YAC_Object *_o, sSI _sx, sSI _sy, sSI _type, sSI _ebytesize) {
    return (sSI)_o->yacArrayRealloc((sUI)_sx, (sUI)_sy, (sUI)_type, (sUI)_ebytesize);
}

sSI YAC_CALL yac_object_yacArrayGetNumElements(YAC_Object *_o) {
    return (sSI)_o->yacArrayGetNumElements();
}

sSI YAC_CALL yac_object_yacArrayGetMaxElements(YAC_Object *_o) {
    return (sSI)_o->yacArrayGetMaxElements();
}

void YAC_CALL yac_object_yacArrayCopySize(YAC_Object *_o, YAC_Object *_p) {
    _o->yacArrayCopySize(_p);
}

void YAC_CALL yac_object_yacArraySet(YAC_Object *_o, sSI _index, YAC_Object *_value) {
    if(YAC_BCHK(_value, YAC_CLID_VALUE))
    {
       _o->yacArraySet(NULL, (sUI)_index, (YAC_ValueObject*)_value);  // xxx TKS_MT use real _context
    }
}

void YAC_CALL yac_object_yacArrayGet(YAC_Object *_o, sSI _index, YAC_Value *_r) {
    _o->yacArrayGet(NULL, (sUI)_index, _r);  // xxx TKS_MT use real _context
}

void YAC_CALL yac_object_yacArrayGetDeref(YAC_Object *_o, sSI _index, YAC_Value *_r) {
    _o->yacArrayGetDeref(NULL, (sUI)_index, _r);  // xxx TKS_MT use real _context
}

sSI YAC_CALL yac_object_yacArrayGetWidth(YAC_Object *_o) {
    return (sSI)_o->yacArrayGetWidth();
}

sSI YAC_CALL yac_object_yacArrayGetHeight(YAC_Object *_o) {
    return (sSI)_o->yacArrayGetHeight();
}

sSI YAC_CALL yac_object_yacArrayGetElementType(YAC_Object *_o) {
    return (sSI)_o->yacArrayGetElementType();
}

sSI YAC_CALL yac_object_yacArrayGetElementByteSize(YAC_Object *_o) {
    return (sSI)_o->yacArrayGetElementByteSize();

}

sSI YAC_CALL yac_object_yacArrayGetStride(YAC_Object *_o) {
    return (sSI)_o->yacArrayGetStride();
}

sSI YAC_CALL yac_object_yacArrayGetPointer(YAC_Object *_o) {
   /// xxx return Long object on 64bit
   //void *adr = _o->yacArrayGetPointer();
   //return *(sSI*)&adr;
   return 0; // !
}

void YAC_CALL yac_object_yacArraySetWidth(YAC_Object *_o, sSI _width) {
    _o->yacArraySetWidth((sUI)_width);
}

void YAC_CALL yac_object_yacArraySetTemplate(YAC_Object *_o, YAC_Object *_template) {
    _o->yacArraySetTemplate(_template);
}

void YAC_CALL yac_object_yacHashSet(YAC_Object *_o, YAC_Object *_key, YAC_Object *_value) {
    if(YAC_BCHK(_key, YAC_CLID_STRING))
    if(YAC_BCHK(_value, YAC_CLID_VALUE))
    {
       _o->yacHashSet(NULL, (YAC_String*)_key, (YAC_ValueObject*)_value); // xxx TKS_MT use real _context
    }
}

void YAC_CALL yac_object_yacHashGet(YAC_Object *_o, YAC_Object *_key, YAC_Value *_r) {
   if(YAC_BCHK(_key, YAC_CLID_STRING))
      _o->yacHashGet(NULL, (YAC_String*)_key, _r);  // xxx TKS_MT use real _context
}

void YAC_CALL yac_object_yacHashGetDeref(YAC_Object *_o, YAC_Object *_key, YAC_Value *_r) {
    if(YAC_BCHK(_key, YAC_CLID_STRING))
        _o->yacHashGetDeref(NULL, (YAC_String*)_key, _r);  // xxx TKS_MT use real _context
}

// ---- s i g n a l s

void YAC_CALL yac_object_yacGetSignalStringList(YAC_Object *_o, YAC_Object *_s) {
    if(YAC_BCHK(_s, YAC_CLID_STRING))
        _o->yacGetSignalStringList((YAC_String*)_s);
}


// ---- m e t a c l a s s e s

void YAC_CALL yac_object_yacMetaClassName(YAC_Object *_o, YAC_Value *_r) {
    YAC_String *s=YAC_New_String();
    if(s)
    {
        s->visit((char*)_o->yacMetaClassName());
    }
    YAC_RETS(s, (s!=0));
}

sSI YAC_CALL yac_object_yacMetaClassMemberGetNum(YAC_Object *_o) {
    return (sSI)_o->yacMetaClassMemberGetNum();
}

sSI YAC_CALL yac_object_yacMetaClassMemberGetAccessKeyByIndex(YAC_Object *_o, sSI _index) {
    return (sSI)_o->yacMetaClassMemberGetAccessKeyByIndex((sUI)_index);
}

sSI YAC_CALL yac_object_yacMetaClassMemberGetAccessKeyByName(YAC_Object *_o, YAC_Object *_s) {
    if(YAC_BCHK(_s, YAC_CLID_STRING))
        return (sSI)_o->yacMetaClassMemberGetAccessKeyByName((sChar*)((YAC_String*)_s)->chars);
    return -1;
}

sSI YAC_CALL yac_object_yacMetaClassMemberGetType(YAC_Object *_o, sSI _ak) {
    return (sSI)_o->yacMetaClassMemberGetType((sUI)_ak);
}

void YAC_CALL yac_object_yacMetaClassMemberGetName(YAC_Object *_o, sSI _ak, YAC_Value *_r) {
    YAC_String *s=YAC_New_String();
    if(s)
    {
        s->visit((char*)_o->yacMetaClassMemberGetName((sUI)_ak));
    }
    YAC_RETS(s, (s!=0));
}


void YAC_CALL yac_object_yacMetaClassMemberSet(YAC_Object *_o, sSI _ak, YAC_Object *_value) {
    if(YAC_BCHK(_value, YAC_CLID_VALUE))
    {
        _o->yacMetaClassMemberSet((sUI)_ak, (YAC_ValueObject*)_value);
    }
}

void YAC_CALL yac_object_yacMetaClassMemberGet(YAC_Object *_o, sSI _ak, YAC_Value *_r) {
    _o->yacMetaClassMemberGet((sUI)_ak, _r);
}

sSI YAC_CALL yac_object_yacMetaClassInstanceOf(YAC_Object *_this, YAC_Object *_o) {
    return _this->yacMetaClassInstanceOf(_o);
}

// ---- n o n - v i r t u a l
void YAC_CALL yac_object_yacNew(YAC_Object *_o, YAC_Value *_r) {
    YAC_Object *o=_o->yacNew(NULL/*_context*/);
    YAC_RETO(o, (o!=0));
}

sSI YAC_CALL yac_object_yacCanDeserializeClass(YAC_Object *_o, YAC_Object *_ifs) {
    return _o->yacCanDeserializeClass(_ifs);
}

sSI YAC_CALL yac_object_yacInstanceOf(YAC_Object *_this, YAC_Object *_o) {
  return _this->yacInstanceOf(_o);
}

#endif // YAC_OBJECT_YAC


#ifndef YAC_CUST_BUFFER
                 YAC_Buffer::YAC_Buffer                     (void)                                  { }
                 YAC_Buffer::~YAC_Buffer                    ()                                      { } // never called, see host impl. instead
void             YAC_Buffer::yacArraySet                    (void *_context, sUI, YAC_Value *)      { }
void             YAC_Buffer::yacArrayGet                    (void *_context, sUI, YAC_Value *_ret)  {_ret->initVoid();}
sUI              YAC_Buffer::yacArrayGetWidth               (void)                                  {return size;}
sUI              YAC_Buffer::yacArrayGetHeight              (void)                                  {return 1;}
sUI              YAC_Buffer::yacArrayGetElementType         (void)                                  {return 1;}
sUI              YAC_Buffer::yacArrayGetElementByteSize     (void)                                  {return sizeof(sU8);}
sUI              YAC_Buffer::yacArrayGetStride              (void)                                  {return 0;}
void *           YAC_Buffer::yacArrayGetPointer             (void)                                  {return (void*)buffer;}
#endif
                 YAC_Iterator::YAC_Iterator                 (void)                                  { }
                 YAC_Iterator::~YAC_Iterator                ()                                      { }
void             YAC_VCALL YAC_Iterator::getNext                      (YAC_Value*r)                           {r->initVoid();}
void             YAC_VCALL YAC_Iterator::begin                        (void)                                  { }
void             YAC_VCALL YAC_Iterator::end                          (void)                                  { }


	// ----
	// ----
	// ----
	// ---- LEVEL (1<<0) interface:
	// ----                       ( C/C++ reflection support )
	// ----
	// ----
                 YAC_Host::YAC_Host                         (void)                                  { }
                 YAC_Host::~YAC_Host                        ()                                      { }


#ifndef YAC_CUST_STRING
sUI              YAC_strlen                           (const char *_s)            {if(_s){sUI r=0;while(*_s++)r++;return r;}else return 0;}
                 YAC_String::YAC_String               ()                          {class_ID=YAC_CLID_STRING;buflen=0;bflags=0;length=0;chars=0;key=YAC_LOSTKEY;}
                 YAC_String::~YAC_String              ()                          {free();}
void             YAC_String::free                     (void)                      {if(chars){if(bflags&DEL){Dyacfreechars(chars);}chars=0;length=0;buflen=0;bflags=0;key=YAC_LOSTKEY;}}
void             YAC_String::visit                    (const sChar*_cstring)      {YAC_String::free();chars=(sU8*)_cstring;key=YAC_LOSTKEY;length=YAC_strlen(_cstring)+1;buflen=length;}
sBool            YAC_String::compare                  (const sChar*e)             {yacmemptr f;f.u1=(sU8*)e;sBool ret=f.any!=0;if(ret&&*f.u1&&length){sU32 _key=1;do{_key++;}while(*++f.u1);ret=_key==length;if(ret){yacmemptr t;t.u1=chars;f.u1=(sU8*)e;sU32 i=0;sU32 l=(length>>RINT_SIZE_SHIFT);for(;i<l&&(*t.ui==*f.ui);t.ui++,f.ui++,i+=SIZEOF_RINT);if(i==length)return!f.u1[-1];ret=*t.u1==*f.u1;if(ret){while(*t.u1==*f.u1)if(!(*t.u1++ && *f.u1++))break;ret=(t.u1-length==chars);}}}return ret;}
sS32             YAC_String::lastIndexOf              (sChar _c, sUI _start)      {sSI li=-1;if(chars){sUI i=_start;for(;i<length;i++){if(chars[i]==((sChar)_c)){li=i;}}}return li;}
sS32             YAC_String::indexOf                  (sChar _c, sUI _start)      {if(chars){sUI i=_start;for(;i<length;i++){if(chars[i]==((sChar)_c))return i;}}return -1;}
#ifdef YAC_BIGSTRING
sUI              YAC_String::sum                      (void)                      {sU32 ret=0;sU8*c=chars;int i=1;if(NULL != c){for(;*c;c++,i+=128){ret=53*ret+i* *c;}} return ret;} //sU32 i=0;sU32 ret=0;sU8 cc;sU8 lc=0;for(;i<length;i++){cc=chars[i];ret+=((sU32)cc^lc)+(i<<2);lc=cc;}return ret;}
void             YAC_String::genKey                   (void)                      {key=sum();}
sUI              YAC_String::getKey                   (void)                      {if(key==YAC_LOSTKEY)genKey();return key;}
sBool            YAC_String::compare                  (YAC_String*s)              {if(s){if(s->length==length&&s->getKey()==getKey()&&s->chars&&chars){sU32 i=0;for(;i<length&&(s->chars[i]==chars[i]); i++);return i==length;}}return 0;}
void             YAC_String::fixLength                (void)                      {if(chars&&buflen) { length=0; sChar *s=(sChar*)chars; while(length<buflen&&*s++) length++; length++; /**count EOF**/ } else length=0; key=YAC_LOSTKEY;}
sBool            YAC_String::alloc                    (sU32 len)                  {if(length!=len){if(chars&&(bflags&DEL)){Dyacfreechars(chars);};buflen=len;length=len;if(len){chars=Dyacallocchars(length);if(chars){sU32 i;for(i=0; i<length; i++)chars[i]=0;}else{buflen=0;bflags=0;length=0;}}else	chars=0;}key=YAC_LOSTKEY;return (chars!=0)?1:0;}
sBool            YAC_String::copy                     (const sChar *e)            {if(e){sUI j=(sUI)YAC_strlen(e);sBool ret=YAC_String::realloc(j+1);if(ret){sU32 i=0;for(; i<j; i++) *(sChar *)&chars[i]=e[i];chars[i]=0;}return ret;}else return createEmpty();}
sBool            YAC_String::realloc                  (sU32 len)                  {if((!chars)||buflen<len){return YAC_String::alloc(len);}else{length=len;key=YAC_LOSTKEY;return length!=0;}}
sBool            YAC_String::createEmpty              (void)                      {sBool ret=YAC_String::alloc(1);if(ret)*chars=0;return ret;}
sBool            YAC_String::copy                     (YAC_String *_s)            {
   sBool ret;
   if( (ret=(_s!=0)) )
   {
      if(_s->length)
      {
         ret = YAC_String::realloc(_s->length);
         if(ret)
         {
            sU32 i;
            for(i=0; i<_s->length; i++)
            {
               chars[i]=_s->chars[i];
            }
            if(_s->bflags & YAC_String::QUOT)
            {
               bflags |= YAC_String::QUOT;
            }
         }
      }
      else 
      {
         ret=createEmpty();
      }
   }
   return ret;
}
sBool            YAC_String::append                   (YAC_String *o)             {if(o&&o->chars&&o->length){sUI k=o->length+length;if(buflen<k){sU8 *nc=Dyacallocchars(k);sBool ret=(nc!=0) ? 1 : 0;if(ret){sU32 i=0;if(chars){for(;i<length-1; i++)nc[i]=chars[i];if((bflags&DEL)){Dyacfreechars(chars);};length--;}for(k=0; i<(length/*-1*/+o->length); i++, k++)nc[i]=o->chars[k];chars=nc; length=length+o->length;key=YAC_LOSTKEY;}return ret;}else{sU8*d=chars+length-1;sU32 i=0;for(;i< o->length;i++)*d++=o->chars[i];length=k-1;if(length==(sU32)-1){length=length;}key=YAC_LOSTKEY;return 1;}}return 0;}
sBool            YAC_String::append                   (const char *_s)            {YAC_String s; s.visit(_s); return append(&s);}
sBool            YAC_String::substring                (YAC_String *s,sUI start, sUI len) {if(len){if(chars&&(start+len)<=length){sU8*a=chars+start;sU32 k=a[len-1]!=0?len+1:len;if(s->realloc(k)){sU32 i=0;for(;i<len;i++)s->chars[i]=*a++;if(k!=len)s->chars[i]=0;return 1;}}else{if(s){s->empty();}}}else{s->empty();}return 0;}
sBool            YAC_String::empty                    (void)                      {sBool ret=YAC_String::realloc(1);if(ret)*chars=0;return ret;}
void             YAC_String::printf                   (const char *_fmt, ...)     {va_list va;va_start(va,_fmt);
#ifdef YAC_VC
_vsnprintf((char*)chars, buflen, _fmt, va);
#else
vsnprintf((char*)chars, buflen, _fmt, va);
#endif
va_end(va);fixLength();}
#endif // YAC_BIGSTRING
#endif // YAC_CUST_STRING

#ifndef YAC_CUST_STRING
#endif
#ifdef YAC_PRINTF
void             YAC_Host::printf                     (const char *_fmt, ...)      {
   if(_fmt)
   {
#ifdef YAC_FORCE_NO_PRINTF_TLS
      static char buf[128*1024];
#else
      static YAC_TLS char buf[128*1024]; // TLS increases tks.exe file size by 128*1024 bytes :(
#endif // YAC_FORCE_NO_PRINTF_TLS
      va_list va;
      va_start(va,_fmt);
      ::vsprintf(buf,/*, sizeof(buf)/sizeof(char),*/_fmt,va);
      va_end(va);
      yacPrint(buf);
   }
}
#endif // YAC_PRINTF


// ----
// ----
// ---- Float relative (exponential) epsilon comparison helpers
// ---- contributed by Carsten Busse <carsten.busse@googlemail.com>
// ----
// ----
#ifdef YAC_EPSILONCOMPARE_REL
sSI yac_epsilon_flt_units = 10;
sS64 yac_epsilon_dbl_units = 100;

sF32 yac_epsilon_flt = YAC_FLT_EPSILON;
sF64 yac_epsilon_dbl = YAC_DBL_EPSILON;

#ifdef YAC_GCC
#include <math.h>
#define Dfabs(x) fabs(x)
#define Dfabsf(x) fabsf(x)
#else
#define Dfabs(x) ( (x<0.0)?(-x):(x) )
#define Dfabsf(x) ( (x<0.0f)?(-x):(x) )
#endif // YAC_GCC

sSI YAC_CALL yac_fltcmp_rel_fast(sF32 a, sF32 b) {
   yacmem _a, _b, _c;
   _a.f4 = a;
   _b.f4 = b;
   if( (a == 0.0f) || (b == 0.0f) ) 
   {
      _c.f4 = yac_epsilon_flt;
   }
   else 
   {
      _c.si = yac_epsilon_flt_units;
   }
   if(_a.si < 0)
   {
      _a.ui = 0x80000000 - _a.ui;
   }
   if(_b.si < 0) 
   {
      _b.ui = 0x80000000 - _b.ui;
   }
   if(abs(_a.si - _b.si) <= _c.si) 
   {
      return 0;
   }
   else if(a < b) 
   {
      return -1;
   }
   else 
   {
      return 1;
   }
}

sSI YAC_CALL yac_dblcmp_rel_fast(sF64 a, sF64 b) {
   yacmem64 _a, _b, _c;
   sS64 res;
   _a.f8 = a;
   _b.f8 = b;
   if( (a == 0.0) || (b == 0.0) )
   {
      _c.f8 = yac_epsilon_dbl;
   }
   else 
   {
      _c.si8 = yac_epsilon_dbl_units;
   }
   if(_a.si8 < 0)
   {
      _a.ui8 = 0x8000000000000000ull - _a.ui8;
   }
   if(_b.si8 < 0)
   {
      _b.ui8 = 0x8000000000000000ull - _b.ui8;
   }
   res = _a.si8 - _b.si8;
   if(res < 0) 
   {
      res = -res;
   }
   if(res <= _c.si8) 
   {
      return 0;
   }
   else if(a < b)
   {
      return -1;
   }
   else 
   {
      return 1;
   }
}

sSI YAC_CALL yac_fltcmp_rel(sF32 a, sF32 b, sF32 err) {
   //both 0
   if( (a == 0.0f) && (b == 0.0f))
   {
      return 0;
   }
   if(err <= 0.0f)
   {
      err = yac_epsilon_flt;
   }
   //special cases where one value is 0
   if(a == 0.0f) 
   {
      if(Dfabsf(b) <= err)
      {
         return 0;
      }
      else if(0.0f < b)
      {
         return -1;
      }
      else
      {
         return 1;
      }
   } 
   else if(b == 0.0f)
   {
      if(Dfabsf(a) <= err)
      {
         return 0;
      }
      else if(a < 0.0f)
      {
         return -1;
      }
      else 
      {
         return 1;
      }
   }
   //both values != 0
   sF32 amb = a - b;
   if(Dfabsf(amb) <= (Dfabsf(a)*err))
   {
      return 0;
   }
   else if(a < b)
   {
      return -1;
   }
   else 
   {
      return 1;
   }
}

sSI YAC_CALL yac_dblcmp_rel(sF64 a, sF64 b, sF64 err) {
   //both 0
   if( (a == 0.0) && (b == 0.0) )
   {
      return 0;
   }
   if(err <= 0.0)
   {
      err = yac_epsilon_dbl;
   }
   //special cases where one value is 0
   if(a == 0.0) 
   {
      if(Dfabs(b) <= err)
      {
         return 0;
      }
      else if(0.0 < b)
      {
         return -1;
      }
      else 
      {
         return 1;
      }
   } 
   else if(b == 0.0) 
   {
      if(Dfabs(a) <= err)
      {
         return 0;
      }
      else if(a < 0.0) 
      {
         return -1;
      }
      else 
      {
         return 1;
      }
   }
   //both values != 0
   sF64 amb = a - b;
   if(Dfabs(amb) <= (Dfabs(a) * err)) 
   {
      return 0;
   }
   else if(a < b)
   {
      return -1;
   }
   else 
   {
      return 1;
   }
}

#undef Dfabs
#undef Dfabsf

#endif // YAC_EPSILONCOMPARE_REL


// ----
// ----
// ---- PointerArray tool methods
// ----
// ----
#ifndef YAC_CUST_VALUE
sBool YAC_PointerArray::realloc(sUI _maxElements) {
   return (sBool) yacArrayRealloc(_maxElements, 0,0,0);
}

sBool YAC_PointerArray::add(YAC_Object *_o, sBool _bDelete) {
   if(num_elements == max_elements)
   {
      if(!YAC_PointerArray::realloc( (num_elements+10)+((sUI)(num_elements / 3)) ))
      {
         return 0;
      }
   }
   elements[num_elements++].initObject(_o, _bDelete);
   return 1;
}

void YAC_PointerArray::removeIndex(sUI _index) {
   if(_index < num_elements)
   {
      if(num_elements>1)
      {
         for(sUI i=((sUI)_index); i<(num_elements-1); i++) 
         {
            elements[i].unsetFast();
            elements[i] = &elements[i+1];
         }
         elements[num_elements-1].unset();
      }
      else
      {
         elements[0].unset();
      }
      num_elements--;
   }
}

sSI YAC_PointerArray::indexOfPointer(YAC_Object *_o, sUI _off) {
   sUI i = (sUI) _off;
   for(; i<num_elements; i++) 
   {
      if( ((void*)elements[i].value.object_val) == ((void*)_o) ) 
      {
         // Ok, found object address
         return (sSI) i;
      } 
   }
   // Failed, address not found
   return -1;
}

#endif // YAC_CUST_VALUE


#ifdef YAC_GLOBAL_NEWDELETE
#ifdef YAC_NO_EXPORTS
sSI yac_global_newdelete_counter = 0; // Only implemented on host side, plugins will call yacTrackNewDelete()
sSI yac_global_newdelete_numallocs = 0;
sSI yac_global_newdelete_numfrees = 0;
#endif // YAC_NO_EXPORTS
void* operator new(size_t size) {
#ifdef YAC_NO_EXPORTS
   yac_global_newdelete_counter += size;
   yac_global_newdelete_numallocs++;
#else
   yac_host->yacNewDeleteModifyCounter(size);
#endif // YAC_NO_EXPORTS
   sSI *p = (sSI*) malloc(size+sizeof(sSI));
   p[0] = size;
   return &p[1];
}

void operator delete(void *_p) {
   sSI *p = &((sSI*) _p)[-1];
#ifdef YAC_NO_EXPORTS
   yac_global_newdelete_counter -= *p;
   yac_global_newdelete_numfrees++;
#else
   yac_host->yacNewDeleteModifyCounter(-*p);
#endif // YAC_NO_EXPORTS
   free(p);
}
#endif // YAC_GLOBAL_NEWDELETE


#endif // __YAC_HOST_CPP__
