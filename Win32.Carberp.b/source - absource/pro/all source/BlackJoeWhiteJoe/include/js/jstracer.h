/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 * vim: set ts=8 sw=4 et tw=99 ft=cpp:
 *
 * ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is Mozilla SpiderMonkey JavaScript 1.9 code, released
 * May 28, 2008.
 *
 * The Initial Developer of the Original Code is
 *   Brendan Eich <brendan@mozilla.org>
 *
 * Contributor(s):
 *   Andreas Gal <gal@mozilla.com>
 *   Mike Shaver <shaver@mozilla.org>
 *   David Anderson <danderson@mozilla.com>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either of the GNU General Public License Version 2 or later (the "GPL"),
 * or the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

#ifndef jstracer_h___
#define jstracer_h___

#ifdef JS_TRACER

#include "jscntxt.h"
#include "jsstddef.h"
#include "jstypes.h"
#include "jslock.h"
#include "jsnum.h"
#include "jsinterp.h"
#include "jsbuiltins.h"

#if defined(DEBUG) && !defined(JS_JIT_SPEW)
#define JS_JIT_SPEW
#endif

template <typename T>
class Queue : public avmplus::GCObject {
    T* _data;
    unsigned _len;
    unsigned _max;

    void ensure(unsigned size) {
        while (_max < size)
            _max <<= 1;
        _data = (T*)realloc(_data, _max * sizeof(T));
#if defined(DEBUG)
        memset(&_data[_len], 0xcd, _max - _len);
#endif
    }
public:
    Queue(unsigned max = 16) {
        this->_max = max;
        this->_len = 0;
        this->_data = (T*)malloc(max * sizeof(T));
    }

    ~Queue() {
        free(_data);
    }

    bool contains(T a) {
        for (unsigned n = 0; n < _len; ++n) {
            if (_data[n] == a)
                return true;
        }
        return false;
    }

    void add(T a) {
        ensure(_len + 1);
        JS_ASSERT(_len <= _max);
        _data[_len++] = a;
    }

    void add(T* chunk, unsigned size) {
        ensure(_len + size);
        JS_ASSERT(_len <= _max);
        memcpy(&_data[_len], chunk, size * sizeof(T));
        _len += size;
    }

    void addUnique(T a) {
        if (!contains(a))
            add(a);
    }

    void setLength(unsigned len) {
        ensure(len + 1);
        _len = len;
    }

    void clear() {
        _len = 0;
    }

    const T & get(unsigned i) const {
        return _data[i];
    }

    unsigned length() const {
        return _len;
    }

    T* data() const {
        return _data;
    }
};

/*
 * Tracker is used to keep track of values being manipulated by the interpreter
 * during trace recording.
 */
class Tracker {
    struct Page {
        struct Page*    next;
        jsuword         base;
        nanojit::LIns*  map[1];
    };
    struct Page* pagelist;

    jsuword         getPageBase(const void* v) const;
    struct Page*    findPage(const void* v) const;
    struct Page*    addPage(const void* v);
public:
    Tracker();
    ~Tracker();

    bool            has(const void* v) const;
    nanojit::LIns*  get(const void* v) const;
    void            set(const void* v, nanojit::LIns* ins);
    void            clear();
};

#ifdef JS_JIT_SPEW
extern bool js_verboseDebug;
#define debug_only_v(x) if (js_verboseDebug) { x; fflush(stdout); }
#else
#define debug_only_v(x)
#endif

/*
 * The oracle keeps track of hit counts for program counter locations, as
 * well as slots that should not be demoted to int because we know them to
 * overflow or they result in type-unstable traces. We are using simple
 * hash tables.  Collisions lead to loss of optimization (demotable slots
 * are not demoted, etc.) but have no correctness implications.
 */
#define ORACLE_SIZE 4096

class Oracle {
    avmplus::BitSet _stackDontDemote;
    avmplus::BitSet _globalDontDemote;
public:
    Oracle();

    JS_REQUIRES_STACK void markGlobalSlotUndemotable(JSContext* cx, unsigned slot);
    JS_REQUIRES_STACK bool isGlobalSlotUndemotable(JSContext* cx, unsigned slot) const;
    JS_REQUIRES_STACK void markStackSlotUndemotable(JSContext* cx, unsigned slot);
    JS_REQUIRES_STACK bool isStackSlotUndemotable(JSContext* cx, unsigned slot) const;
    void clearDemotability();
    void clear() {
        clearDemotability();
    }
};

typedef Queue<uint16> SlotList;

class TypeMap : public Queue<uint8> {
public:
    JS_REQUIRES_STACK void captureTypes(JSContext* cx, SlotList& slots, unsigned callDepth);
    JS_REQUIRES_STACK void captureMissingGlobalTypes(JSContext* cx,
                                                     SlotList& slots,
                                                     unsigned stackSlots);
    bool matches(TypeMap& other) const;
};

enum ExitType {
    /*
     * An exit at a possible branch-point in the trace at which to attach a
     * future secondary trace. Therefore the recorder must generate different
     * code to handle the other outcome of the branch condition from the
     * primary trace's outcome.
     */
    BRANCH_EXIT,

    /*
     * Exit at a tableswitch via a numbered case.
     */
    CASE_EXIT,

    /*
     * Exit at a tableswitch via the default case.
     */
    DEFAULT_EXIT,

    LOOP_EXIT,
    NESTED_EXIT,

    /*
     * An exit from a trace because a condition relied upon at recording time
     * no longer holds, where the alternate path of execution is so rare or
     * difficult to address in native code that it is not traced at all, e.g.
     * negative array index accesses, which differ from positive indexes in
     * that they require a string-based property lookup rather than a simple
     * memory access.
     */
    MISMATCH_EXIT,

    /*
     * A specialization of MISMATCH_EXIT to handle allocation failures.
     */
    OOM_EXIT,
    OVERFLOW_EXIT,
    UNSTABLE_LOOP_EXIT,
    TIMEOUT_EXIT,
    DEEP_BAIL_EXIT,
    STATUS_EXIT
};

struct VMSideExit : public nanojit::SideExit
{
    JSObject* block;
    jsbytecode* pc;
    jsbytecode* imacpc;
    intptr_t sp_adj;
    intptr_t rp_adj;
    int32_t calldepth;
    uint32 numGlobalSlots;
    uint32 numStackSlots;
    uint32 numStackSlotsBelowCurrentFrame;
    ExitType exitType;

    /*
     * Ordinarily 0.  If a slow native function is atop the stack, the 1 bit is
     * set if constructing and the other bits are a pointer to the funobj.
     */
    uintptr_t nativeCalleeWord;

    JSObject * nativeCallee() {
        return (JSObject *) (nativeCalleeWord & ~1);
    }

    bool constructing() {
        return bool(nativeCalleeWord & 1);
    }

    void setNativeCallee(JSObject *callee, bool constructing) {
        nativeCalleeWord = uintptr_t(callee) | (constructing ? 1 : 0);
    }
};

static inline uint8* getStackTypeMap(nanojit::SideExit* exit)
{
    return (uint8*)(((VMSideExit*)exit) + 1);
}

static inline uint8* getGlobalTypeMap(nanojit::SideExit* exit)
{
    return getStackTypeMap(exit) + ((VMSideExit*)exit)->numStackSlots;
}

static inline uint8* getFullTypeMap(nanojit::SideExit* exit)
{
    return getStackTypeMap(exit);
}

struct FrameInfo {
    JSObject*       callee;     // callee function object
    JSObject*       block;      // caller block chain head
    jsbytecode*     pc;         // caller fp->regs->pc
    jsbytecode*     imacpc;     // caller fp->imacpc
    uint16          spdist;     // distance from fp->slots to fp->regs->sp at JSOP_CALL

    /*
     * Bit  15 (0x8000) is a flag that is set if constructing (called through new).
     * Bits 0-14 are the actual argument count. This may be less than fun->nargs.
     */
    uint16          argc;

    /*
     * Stack pointer adjustment needed for navigation of native stack in
     * js_GetUpvarOnTrace. spoffset is the number of slots in the native
     * stack frame for the caller *before* the slots covered by spdist.
     * This may be negative if the caller is the top level script.
     * The key fact is that if we let 'cpos' be the start of the caller's
     * native stack frame, then (cpos + spoffset) points to the first 
     * non-argument slot in the callee's native stack frame.
     */
    int32          spoffset;

    // Safer accessors for argc.
    enum { CONSTRUCTING_MASK = 0x8000 };
    void   set_argc(uint16 argc, bool constructing) {
        this->argc = argc | (constructing ? CONSTRUCTING_MASK : 0);
    }
    uint16 get_argc() const { return argc & ~CONSTRUCTING_MASK; }
    bool   is_constructing() const { return (argc & CONSTRUCTING_MASK) != 0; }

    // The typemap just before the callee is called.
    uint8* get_typemap() { return (uint8*) (this+1); }
};

struct UnstableExit
{
    nanojit::Fragment* fragment;
    VMSideExit* exit;
    UnstableExit* next;
};

class TreeInfo MMGC_SUBCLASS_DECL {
public:
    nanojit::Fragment* const      fragment;
    JSScript*               script;
    unsigned                maxNativeStackSlots;
    ptrdiff_t               nativeStackBase;
    unsigned                maxCallDepth;
    TypeMap                 typeMap;
    unsigned                nStackTypes;
    SlotList*               globalSlots;
    /* Dependent trees must be trashed if this tree dies, and updated on missing global types */
    Queue<nanojit::Fragment*> dependentTrees;
    /* Linked trees must be updated on missing global types, but are not dependent */
    Queue<nanojit::Fragment*> linkedTrees;
    unsigned                branchCount;
    Queue<VMSideExit*>      sideExits;
    UnstableExit*           unstableExits;
#ifdef DEBUG
    const char*             treeFileName;
    uintN                   treeLineNumber;
    uintN                   treePCOffset;
#endif

    TreeInfo(nanojit::Fragment* _fragment,
             SlotList* _globalSlots)
      : fragment(_fragment),
        script(NULL),
        maxNativeStackSlots(0),
        nativeStackBase(0),
        maxCallDepth(0),
        nStackTypes(0),
        globalSlots(_globalSlots),
        branchCount(0),
        unstableExits(NULL)
            {}
    ~TreeInfo();

    inline unsigned nGlobalTypes() {
        return typeMap.length() - nStackTypes;
    }
    inline uint8* globalTypeMap() {
        return typeMap.data() + nStackTypes;
    }
    inline uint8* stackTypeMap() {
        return typeMap.data();
    }
};

#if defined(JS_JIT_SPEW) && (defined(NANOJIT_IA32) || (defined(NANOJIT_AMD64) && defined(__GNUC__)))
# define EXECUTE_TREE_TIMER
#endif

typedef enum JSBuiltinStatus {
    JSBUILTIN_BAILED = 1,
    JSBUILTIN_ERROR = 2
} JSBuiltinStatus;

struct InterpState
{
    double        *sp;                  // native stack pointer, stack[0] is spbase[0]
    FrameInfo**   rp;                   // call stack pointer
    JSContext     *cx;                  // current VM context handle
    double        *eos;                 // first unusable word after the native stack
    void          *eor;                 // first unusable word after the call stack
    VMSideExit*    lastTreeExitGuard;   // guard we exited on during a tree call
    VMSideExit*    lastTreeCallGuard;   // guard we want to grow from if the tree
                                        // call exit guard mismatched
    void*          rpAtLastTreeCall;    // value of rp at innermost tree call guard
    TreeInfo*      outermostTree;       // the outermost tree we initially invoked
    double*        stackBase;           // native stack base
    FrameInfo**    callstackBase;       // call stack base
    uintN*         inlineCallCountp;    // inline call count counter
    VMSideExit**   innermostNestedGuardp;
    void*          stackMark;
    VMSideExit*    innermost;
#ifdef EXECUTE_TREE_TIMER
    uint64         startTime;
#endif
    InterpState*   prev;

    /*
     * Used by _FAIL builtins; see jsbuiltins.h. The builtin sets the
     * JSBUILTIN_BAILED bit if it bails off trace and the JSBUILTIN_ERROR bit
     * if an error or exception occurred.
     */
    uint32         builtinStatus;

    // Used to communicate the location of the return value in case of a deep bail.
    double*        deepBailSp;
};

static JS_INLINE void
js_SetBuiltinError(JSContext *cx)
{
    cx->interpState->builtinStatus |= JSBUILTIN_ERROR;
}

#ifdef DEBUG_JSRS_NOT_BOOL
struct JSRecordingStatus {
    int code;
    bool operator==(JSRecordingStatus &s) { return this->code == s.code; };
    bool operator!=(JSRecordingStatus &s) { return this->code != s.code; };
};
enum JSRScodes {
    JSRS_ERROR_code,
    JSRS_STOP_code,
    JSRS_CONTINUE_code,
    JSRS_IMACRO_code
};
struct JSRecordingStatus JSRS_CONTINUE = { JSRS_CONTINUE_code };
struct JSRecordingStatus JSRS_STOP     = { JSRS_STOP_code };
struct JSRecordingStatus JSRS_IMACRO   = { JSRS_IMACRO_code };
struct JSRecordingStatus JSRS_ERROR    = { JSRS_ERROR_code };
#define STATUS_ABORTS_RECORDING(s) ((s) == JSRS_STOP || (s) == JSRS_ERROR)
#else
enum JSRecordingStatus {
    JSRS_ERROR,        // Error; propagate to interpreter. 
    JSRS_STOP,         // Abort recording.
    JSRS_CONTINUE,     // Continue recording.
    JSRS_IMACRO        // Entered imacro; continue recording.
                       // Only JSOP_IS_IMACOP opcodes may return this.
};
#define STATUS_ABORTS_RECORDING(s) ((s) <= JSRS_STOP)
#endif



class TraceRecorder : public avmplus::GCObject {
    JSContext*              cx;
    JSTraceMonitor*         traceMonitor;
    JSObject*               globalObj;
    JSObject*               lexicalBlock;
    Tracker                 tracker;
    Tracker                 nativeFrameTracker;
    char*                   entryTypeMap;
    unsigned                callDepth;
    JSAtom**                atoms;
    VMSideExit*             anchor;
    nanojit::Fragment*      fragment;
    TreeInfo*               treeInfo;
    nanojit::LirBuffer*     lirbuf;
    nanojit::LirWriter*     lir;
    nanojit::LirBufWriter*  lir_buf_writer;
    nanojit::LirWriter*     verbose_filter;
    nanojit::LirWriter*     cse_filter;
    nanojit::LirWriter*     expr_filter;
    nanojit::LirWriter*     func_filter;
    nanojit::LirWriter*     float_filter;
    nanojit::LIns*          cx_ins;
    nanojit::LIns*          eos_ins;
    nanojit::LIns*          eor_ins;
    nanojit::LIns*          rval_ins;
    nanojit::LIns*          inner_sp_ins;
    nanojit::LIns*          native_rval_ins;
    nanojit::LIns*          newobj_ins;
    bool                    deepAborted;
    bool                    trashSelf;
    Queue<nanojit::Fragment*> whichTreesToTrash;
    Queue<jsbytecode*>      cfgMerges;
    jsval*                  global_dslots;
    JSTraceableNative*      generatedTraceableNative;
    JSTraceableNative*      pendingTraceableNative;
    TraceRecorder*          nextRecorderToAbort;
    bool                    wasRootFragment;
    jsbytecode*             outer;     /* outer trace header PC */
    uint32                  outerArgc; /* outer trace deepest frame argc */
    bool                    loop;

    bool isGlobal(jsval* p) const;
    ptrdiff_t nativeGlobalOffset(jsval* p) const;
    JS_REQUIRES_STACK ptrdiff_t nativeStackOffset(jsval* p) const;
    JS_REQUIRES_STACK void import(nanojit::LIns* base, ptrdiff_t offset, jsval* p, uint8 t,
                                  const char *prefix, uintN index, JSStackFrame *fp);
    JS_REQUIRES_STACK void import(TreeInfo* treeInfo, nanojit::LIns* sp, unsigned stackSlots,
                                  unsigned callDepth, unsigned ngslots, uint8* typeMap);
    void trackNativeStackUse(unsigned slots);

    JS_REQUIRES_STACK bool isValidSlot(JSScope* scope, JSScopeProperty* sprop);
    JS_REQUIRES_STACK bool lazilyImportGlobalSlot(unsigned slot);

    JS_REQUIRES_STACK void guard(bool expected, nanojit::LIns* cond, ExitType exitType);
    JS_REQUIRES_STACK void guard(bool expected, nanojit::LIns* cond, VMSideExit* exit);

    nanojit::LIns* addName(nanojit::LIns* ins, const char* name);

    nanojit::LIns* writeBack(nanojit::LIns* i, nanojit::LIns* base, ptrdiff_t offset);
    JS_REQUIRES_STACK void set(jsval* p, nanojit::LIns* l, bool initializing = false);
    JS_REQUIRES_STACK nanojit::LIns* get(jsval* p);
    JS_REQUIRES_STACK bool known(jsval* p);
    JS_REQUIRES_STACK void checkForGlobalObjectReallocation();

    JS_REQUIRES_STACK bool checkType(jsval& v, uint8 t, jsval*& stage_val,
                                     nanojit::LIns*& stage_ins, unsigned& stage_count);
    JS_REQUIRES_STACK bool deduceTypeStability(nanojit::Fragment* root_peer,
                                               nanojit::Fragment** stable_peer,
                                               bool& demote);

    JS_REQUIRES_STACK jsval& argval(unsigned n) const;
    JS_REQUIRES_STACK jsval& varval(unsigned n) const;
    JS_REQUIRES_STACK jsval& stackval(int n) const;

    JS_REQUIRES_STACK nanojit::LIns* scopeChain() const;
    JS_REQUIRES_STACK JSRecordingStatus activeCallOrGlobalSlot(JSObject* obj, jsval*& vp);

    JS_REQUIRES_STACK nanojit::LIns* arg(unsigned n);
    JS_REQUIRES_STACK void arg(unsigned n, nanojit::LIns* i);
    JS_REQUIRES_STACK nanojit::LIns* var(unsigned n);
    JS_REQUIRES_STACK void var(unsigned n, nanojit::LIns* i);
    JS_REQUIRES_STACK nanojit::LIns* upvar(JSScript* script, JSUpvarArray* uva, uintN index, jsval& v);
    JS_REQUIRES_STACK nanojit::LIns* stack(int n);
    JS_REQUIRES_STACK void stack(int n, nanojit::LIns* i);

    JS_REQUIRES_STACK nanojit::LIns* alu(nanojit::LOpcode op, jsdouble v0, jsdouble v1,
                                         nanojit::LIns* s0, nanojit::LIns* s1);
    nanojit::LIns* f2i(nanojit::LIns* f);
    JS_REQUIRES_STACK nanojit::LIns* makeNumberInt32(nanojit::LIns* f);
    JS_REQUIRES_STACK nanojit::LIns* stringify(jsval& v);

    JS_REQUIRES_STACK JSRecordingStatus call_imacro(jsbytecode* imacro);

    JS_REQUIRES_STACK JSRecordingStatus ifop();
    JS_REQUIRES_STACK JSRecordingStatus switchop();
#ifdef NANOJIT_IA32
    JS_REQUIRES_STACK nanojit::LIns* tableswitch();
#endif
    JS_REQUIRES_STACK JSRecordingStatus inc(jsval& v, jsint incr, bool pre = true);
    JS_REQUIRES_STACK JSRecordingStatus inc(jsval& v, nanojit::LIns*& v_ins, jsint incr,
                                                   bool pre = true);
    JS_REQUIRES_STACK JSRecordingStatus incProp(jsint incr, bool pre = true);
    JS_REQUIRES_STACK JSRecordingStatus incElem(jsint incr, bool pre = true);
    JS_REQUIRES_STACK JSRecordingStatus incName(jsint incr, bool pre = true);

    JS_REQUIRES_STACK void strictEquality(bool equal, bool cmpCase);
    JS_REQUIRES_STACK JSRecordingStatus equality(bool negate, bool tryBranchAfterCond);
    JS_REQUIRES_STACK JSRecordingStatus equalityHelper(jsval l, jsval r,
                                                       nanojit::LIns* l_ins, nanojit::LIns* r_ins,
                                                       bool negate, bool tryBranchAfterCond,
                                                       jsval& rval);
    JS_REQUIRES_STACK JSRecordingStatus relational(nanojit::LOpcode op, bool tryBranchAfterCond);

    JS_REQUIRES_STACK JSRecordingStatus unary(nanojit::LOpcode op);
    JS_REQUIRES_STACK JSRecordingStatus binary(nanojit::LOpcode op);

    bool ibinary(nanojit::LOpcode op);
    bool iunary(nanojit::LOpcode op);
    bool bbinary(nanojit::LOpcode op);
    void demote(jsval& v, jsdouble result);

    JS_REQUIRES_STACK bool map_is_native(JSObjectMap* map, nanojit::LIns* map_ins,
                                         nanojit::LIns*& ops_ins, size_t op_offset = 0);
    JS_REQUIRES_STACK JSRecordingStatus test_property_cache(JSObject* obj, nanojit::LIns* obj_ins,
                                                            JSObject*& obj2, jsuword& pcval);
    void stobj_set_fslot(nanojit::LIns *obj_ins, unsigned slot,
                         nanojit::LIns* v_ins, const char *name);
    void stobj_set_dslot(nanojit::LIns *obj_ins, unsigned slot, nanojit::LIns*& dslots_ins,
                         nanojit::LIns* v_ins, const char *name);
    void stobj_set_slot(nanojit::LIns* obj_ins, unsigned slot, nanojit::LIns*& dslots_ins,
                        nanojit::LIns* v_ins);

    nanojit::LIns* stobj_get_fslot(nanojit::LIns* obj_ins, unsigned slot);
    nanojit::LIns* stobj_get_dslot(nanojit::LIns* obj_ins, unsigned index,
                                   nanojit::LIns*& dslots_ins);
    nanojit::LIns* stobj_get_slot(nanojit::LIns* obj_ins, unsigned slot,
                                  nanojit::LIns*& dslots_ins);
    JSRecordingStatus native_set(nanojit::LIns* obj_ins, JSScopeProperty* sprop,
                                 nanojit::LIns*& dslots_ins, nanojit::LIns* v_ins);
    JSRecordingStatus native_get(nanojit::LIns* obj_ins, nanojit::LIns* pobj_ins,
                                 JSScopeProperty* sprop, nanojit::LIns*& dslots_ins,
                                 nanojit::LIns*& v_ins);

    nanojit::LIns* getStringLength(nanojit::LIns* str_ins);

    JS_REQUIRES_STACK JSRecordingStatus name(jsval*& vp);
    JS_REQUIRES_STACK JSRecordingStatus prop(JSObject* obj, nanojit::LIns* obj_ins, uint32& slot,
                                             nanojit::LIns*& v_ins);
    JS_REQUIRES_STACK JSRecordingStatus denseArrayElement(jsval& oval, jsval& idx, jsval*& vp,
                                                          nanojit::LIns*& v_ins,
                                                          nanojit::LIns*& addr_ins);
    JS_REQUIRES_STACK JSRecordingStatus getProp(JSObject* obj, nanojit::LIns* obj_ins);
    JS_REQUIRES_STACK JSRecordingStatus getProp(jsval& v);
    JS_REQUIRES_STACK JSRecordingStatus getThis(nanojit::LIns*& this_ins);

    JS_REQUIRES_STACK void box_jsval(jsval v, nanojit::LIns*& v_ins);
    JS_REQUIRES_STACK void unbox_jsval(jsval v, nanojit::LIns*& v_ins, VMSideExit* exit);
    JS_REQUIRES_STACK bool guardClass(JSObject* obj, nanojit::LIns* obj_ins, JSClass* clasp,
                                      VMSideExit* exit);
    JS_REQUIRES_STACK bool guardDenseArray(JSObject* obj, nanojit::LIns* obj_ins,
                                           ExitType exitType = MISMATCH_EXIT);
    JS_REQUIRES_STACK bool guardHasPrototype(JSObject* obj, nanojit::LIns* obj_ins,
                                             JSObject** pobj, nanojit::LIns** pobj_ins,
                                             VMSideExit* exit);
    JS_REQUIRES_STACK JSRecordingStatus guardPrototypeHasNoIndexedProperties(JSObject* obj,
                                                                             nanojit::LIns* obj_ins,
                                                                             ExitType exitType);
    JS_REQUIRES_STACK JSRecordingStatus guardNotGlobalObject(JSObject* obj,
                                                             nanojit::LIns* obj_ins);
    void clearFrameSlotsFromCache();
    JS_REQUIRES_STACK JSRecordingStatus guardCallee(jsval& callee);
    JS_REQUIRES_STACK JSRecordingStatus getClassPrototype(JSObject* ctor,
                                                          nanojit::LIns*& proto_ins);
    JS_REQUIRES_STACK JSRecordingStatus getClassPrototype(JSProtoKey key,
                                                          nanojit::LIns*& proto_ins);
    JS_REQUIRES_STACK JSRecordingStatus newArray(JSObject* ctor, uint32 argc, jsval* argv,
                                                 jsval* rval);
    JS_REQUIRES_STACK JSRecordingStatus newString(JSObject* ctor, uint32 argc, jsval* argv,
                                                  jsval* rval);
    JS_REQUIRES_STACK JSRecordingStatus interpretedFunctionCall(jsval& fval, JSFunction* fun,
                                                                uintN argc, bool constructing);
    JS_REQUIRES_STACK JSRecordingStatus emitNativeCall(JSTraceableNative* known, uintN argc,
                                                       nanojit::LIns* args[]);
    JS_REQUIRES_STACK JSRecordingStatus callTraceableNative(JSFunction* fun, uintN argc,
                                                            bool constructing);
    JS_REQUIRES_STACK JSRecordingStatus callNative(uintN argc, JSOp mode);
    JS_REQUIRES_STACK JSRecordingStatus functionCall(uintN argc, JSOp mode);

    JS_REQUIRES_STACK void trackCfgMerges(jsbytecode* pc);
    JS_REQUIRES_STACK void emitIf(jsbytecode* pc, bool cond, nanojit::LIns* x);
    JS_REQUIRES_STACK void fuseIf(jsbytecode* pc, bool cond, nanojit::LIns* x);
    JS_REQUIRES_STACK JSRecordingStatus checkTraceEnd(jsbytecode* pc);

    bool hasMethod(JSObject* obj, jsid id);
    JS_REQUIRES_STACK bool hasIteratorMethod(JSObject* obj);

    JS_REQUIRES_STACK jsatomid getFullIndex(ptrdiff_t pcoff = 0);

public:
    JS_REQUIRES_STACK
    TraceRecorder(JSContext* cx, VMSideExit*, nanojit::Fragment*, TreeInfo*,
                  unsigned stackSlots, unsigned ngslots, uint8* typeMap,
                  VMSideExit* expectedInnerExit, jsbytecode* outerTree,
                  uint32 outerArgc);
    ~TraceRecorder();

    static JS_REQUIRES_STACK JSRecordingStatus monitorRecording(JSContext* cx, TraceRecorder* tr,
                                                                JSOp op);

    JS_REQUIRES_STACK uint8 determineSlotType(jsval* vp);

    /*
     * Examines current interpreter state to record information suitable for
     * returning to the interpreter through a side exit of the given type.
     */
    JS_REQUIRES_STACK VMSideExit* snapshot(ExitType exitType);

    /*
     * Creates a separate but identical copy of the given side exit, allowing
     * the guards associated with each to be entirely separate even after
     * subsequent patching.
     */
    JS_REQUIRES_STACK VMSideExit* copy(VMSideExit* exit);

    /*
     * Creates an instruction whose payload is a GuardRecord for the given exit.
     * The instruction is suitable for use as the final argument of a single
     * call to LirBuffer::insGuard; do not reuse the returned value.
     */
    JS_REQUIRES_STACK nanojit::LIns* createGuardRecord(VMSideExit* exit);

    nanojit::Fragment* getFragment() const { return fragment; }
    TreeInfo* getTreeInfo() const { return treeInfo; }
    JS_REQUIRES_STACK void compile(JSTraceMonitor* tm);
    JS_REQUIRES_STACK void closeLoop(JSTraceMonitor* tm, bool& demote);
    JS_REQUIRES_STACK void endLoop(JSTraceMonitor* tm);
    JS_REQUIRES_STACK void joinEdgesToEntry(nanojit::Fragmento* fragmento,
                                            VMFragment* peer_root);
    void blacklist() { fragment->blacklist(); }
    JS_REQUIRES_STACK void adjustCallerTypes(nanojit::Fragment* f);
    JS_REQUIRES_STACK nanojit::Fragment* findNestedCompatiblePeer(nanojit::Fragment* f);
    JS_REQUIRES_STACK void prepareTreeCall(nanojit::Fragment* inner);
    JS_REQUIRES_STACK void emitTreeCall(nanojit::Fragment* inner, VMSideExit* exit);
    unsigned getCallDepth() const;
    void pushAbortStack();
    void popAbortStack();
    void removeFragmentoReferences();
    void deepAbort();

    JS_REQUIRES_STACK JSRecordingStatus record_EnterFrame();
    JS_REQUIRES_STACK JSRecordingStatus record_LeaveFrame();
    JS_REQUIRES_STACK JSRecordingStatus record_SetPropHit(JSPropCacheEntry* entry,
                                                          JSScopeProperty* sprop);
    JS_REQUIRES_STACK JSRecordingStatus record_DefLocalFunSetSlot(uint32 slot, JSObject* obj);
    JS_REQUIRES_STACK JSRecordingStatus record_NativeCallComplete();

    bool wasDeepAborted() { return deepAborted; }
    TreeInfo* getTreeInfo() { return treeInfo; }

#define OPDEF(op,val,name,token,length,nuses,ndefs,prec,format)               \
    JS_REQUIRES_STACK JSRecordingStatus record_##op();
# include "jsopcode.tbl"
#undef OPDEF
};
#define TRACING_ENABLED(cx)       JS_HAS_OPTION(cx, JSOPTION_JIT)
#define TRACE_RECORDER(cx)        (JS_TRACE_MONITOR(cx).recorder)
#define SET_TRACE_RECORDER(cx,tr) (JS_TRACE_MONITOR(cx).recorder = (tr))

#define JSOP_IN_RANGE(op,lo,hi)   (uintN((op) - (lo)) <= uintN((hi) - (lo)))
#define JSOP_IS_BINARY(op)        JSOP_IN_RANGE(op, JSOP_BITOR, JSOP_MOD)
#define JSOP_IS_UNARY(op)         JSOP_IN_RANGE(op, JSOP_NEG, JSOP_POS)
#define JSOP_IS_EQUALITY(op)      JSOP_IN_RANGE(op, JSOP_EQ, JSOP_NE)

#define TRACE_ARGS_(x,args)                                                   \
    JS_BEGIN_MACRO                                                            \
        TraceRecorder* tr_ = TRACE_RECORDER(cx);                              \
        if (tr_ && !tr_->wasDeepAborted()) {                                  \
            JSRecordingStatus status = tr_->record_##x args;                  \
            if (STATUS_ABORTS_RECORDING(status)) {                            \
                js_AbortRecording(cx, #x);                                    \
                if (status == JSRS_ERROR)                                     \
                    goto error;                                               \
            }                                                                 \
            JS_ASSERT(status != JSRS_IMACRO);                                 \
        }                                                                     \
    JS_END_MACRO

#define TRACE_ARGS(x,args)      TRACE_ARGS_(x, args)
#define TRACE_0(x)              TRACE_ARGS(x, ())
#define TRACE_1(x,a)            TRACE_ARGS(x, (a))
#define TRACE_2(x,a,b)          TRACE_ARGS(x, (a, b))

extern JS_REQUIRES_STACK bool
js_MonitorLoopEdge(JSContext* cx, uintN& inlineCallCount);

#ifdef DEBUG
# define js_AbortRecording(cx, reason) js_AbortRecordingImpl(cx, reason)
#else
# define js_AbortRecording(cx, reason) js_AbortRecordingImpl(cx)
#endif

extern JS_REQUIRES_STACK void
js_AbortRecording(JSContext* cx, const char* reason);

extern void
js_InitJIT(JSTraceMonitor *tm);

extern void
js_FinishJIT(JSTraceMonitor *tm);

extern void
js_PurgeScriptFragments(JSContext* cx, JSScript* script);

extern bool
js_OverfullFragmento(JSTraceMonitor* tm, nanojit::Fragmento *frago);

extern void
js_PurgeJITOracle();

extern JSObject *
js_GetBuiltinFunction(JSContext *cx, uintN index);

extern void
js_SetMaxCodeCacheBytes(JSContext* cx, uint32 bytes);

#else  /* !JS_TRACER */

#define TRACE_0(x)              ((void)0)
#define TRACE_1(x,a)            ((void)0)
#define TRACE_2(x,a,b)          ((void)0)

#endif /* !JS_TRACER */

#endif /* jstracer_h___ */
