#ifdef IROP_ENUM
enum IROps : uint16_t {
	OP_DUMMY,
	OP_IRHEADER,
	OP_CODEBLOCK,
	OP_BEGINBLOCK,
	OP_INVALIDATEFLAGS,
	OP_ENDBLOCK,
	OP_GUESTOPCODE,
	OP_VALIDATECODE,
	OP_THREADREMOVECODEENTRY,
	OP_PROCESSORID,
	OP_GETROUNDINGMODE,
	OP_SETROUNDINGMODE,
	OP_PUSHROUNDINGMODE,
	OP_POPROUNDINGMODE,
	OP_PRINT,
	OP_PRINTMSG,
	OP_ALLOCATEGPR,
	OP_ALLOCATEFPR,
	OP_ALLOCATEGPRAFTER,
	OP_RDRAND,
	OP_YIELD,
	OP_WFET,
	OP_MONOBACKPATCHERWRITE,
	OP_JUMP,
	OP_CONDJUMP,
	OP_EXITFUNCTION,
	OP_BREAK,
	OP_CALLBACKRETURN,
	OP_SYSCALL,
	OP_THUNK,
	OP_CPUID,
	OP_XGETBV,
	OP_COPY,
	OP_LOADREGISTER,
	OP_LOADPF,
	OP_LOADAF,
	OP_STOREREGISTER,
	OP_STOREPF,
	OP_STOREAF,
	OP_LOADCONTEXT,
	OP_LOADCONTEXTPAIR,
	OP_STORECONTEXT,
	OP_STORECONTEXTPAIR,
	OP_LOADCONTEXTINDEXED,
	OP_STORECONTEXTINDEXED,
	OP_FORMCONTEXTADDRESS,
	OP_SPILLREGISTER,
	OP_FILLREGISTER,
	OP_LOADNZCV,
	OP_STORENZCV,
	OP_LOADDF,
	OP_LOADMEM,
	OP_LOADMEMPAIR,
	OP_STOREMEM,
	OP_STOREMEMPAIR,
	OP_STOREMEMX87SVEOPTPREDICATE,
	OP_LOADMEMX87SVEOPTPREDICATE,
	OP_LOADMEMTSO,
	OP_STOREMEMTSO,
	OP_VLOADVECTORMASKED,
	OP_VSTOREVECTORMASKED,
	OP_VLOADVECTORGATHERMASKED,
	OP_VLOADVECTORGATHERMASKEDQPS,
	OP_VLOADVECTORELEMENT,
	OP_VSTOREVECTORELEMENT,
	OP_VBROADCASTFROMMEM,
	OP_PUSH,
	OP_PUSHTWO,
	OP_RMWHANDLE,
	OP_POP,
	OP_POPTWO,
	OP_MEMSET,
	OP_MEMCPY,
	OP_CACHELINECLEAR,
	OP_CACHELINECLEAN,
	OP_CACHELINEZERO,
	OP_FENCE,
	OP_PREFETCH,
	OP_VSTORENONTEMPORAL,
	OP_VSTORENONTEMPORALPAIR,
	OP_VLOADNONTEMPORAL,
	OP_CONTEXTCLEAR,
	OP_CAS,
	OP_CASPAIR,
	OP_ATOMICSWAP,
	OP_ATOMICFETCHADD,
	OP_ATOMICFETCHSUB,
	OP_ATOMICFETCHAND,
	OP_ATOMICFETCHCLR,
	OP_ATOMICFETCHOR,
	OP_ATOMICFETCHXOR,
	OP_ATOMICFETCHNEG,
	OP_TELEMETRYSETVALUE,
	OP_ENTRYPOINTOFFSET,
	OP_INLINEENTRYPOINTOFFSET,
	OP_CONSTANT,
	OP_INLINECONSTANT,
	OP_CYCLECOUNTER,
	OP_NEG,
	OP_NOT,
	OP_POPCOUNT,
	OP_FINDLSB,
	OP_FINDMSB,
	OP_FINDTRAILINGZEROES,
	OP_COUNTLEADINGZEROES,
	OP_REV,
	OP_RBIT,
	OP_ADD,
	OP_ADC,
	OP_SBB,
	OP_ADDSHIFT,
	OP_ADDWITHFLAGS,
	OP_ADDNZCV,
	OP_SETSMALLNZV,
	OP_CARRYINVERT,
	OP_AXFLAG,
	OP_PARITY,
	OP_RMIFNZCV,
	OP_CONDADDNZCV,
	OP_CONDSUBNZCV,
	OP_ADCWITHFLAGS,
	OP_ADCZERO,
	OP_ADCZEROWITHFLAGS,
	OP_SBBWITHFLAGS,
	OP_ADCNZCV,
	OP_SBBNZCV,
	OP_SUB,
	OP_SUBSHIFT,
	OP_SUBWITHFLAGS,
	OP_CMPPAIRZ,
	OP_SUBNZCV,
	OP_OR,
	OP_ORLSHL,
	OP_ORLSHR,
	OP_ORNROR,
	OP_XOR,
	OP_XORSHIFT,
	OP_XORNSHIFT,
	OP_AND,
	OP_ANDSHIFT,
	OP_ANDWITHFLAGS,
	OP_ANDN,
	OP_TESTNZ,
	OP_TESTZ,
	OP_LSHL,
	OP_LSHR,
	OP_ASHR,
	OP_SHIFTFLAGS,
	OP_ROTATEFLAGS,
	OP_ROR,
	OP_MUL,
	OP_UMUL,
	OP_UMULL,
	OP_SMULL,
	OP_MULH,
	OP_UMULH,
	OP_BFI,
	OP_BFXIL,
	OP_BFE,
	OP_SBFE,
	OP_NZCVSELECT,
	OP_NZCVSELECTV,
	OP_NZCVSELECTINCREMENT,
	OP_SELECT,
	OP_MASKGENERATEFROMBITWIDTH,
	OP_EXTR,
	OP_PDEP,
	OP_PEXT,
	OP_DIV,
	OP_UDIV,
	OP_VEXTRACTTOGPR,
	OP_FLOAT_TOGPR_S,
	OP_FLOAT_TOGPR_ZS,
	OP_FCMP,
	OP_VFADDSCALARINSERT,
	OP_VFSUBSCALARINSERT,
	OP_VFMULSCALARINSERT,
	OP_VFDIVSCALARINSERT,
	OP_VFMINSCALARINSERT,
	OP_VFMAXSCALARINSERT,
	OP_VFSQRTSCALARINSERT,
	OP_VFRSQRTSCALARINSERT,
	OP_VFRECPSCALARINSERT,
	OP_VFTOFSCALARINSERT,
	OP_VSTOFVECTORINSERT,
	OP_VSTOFGPRINSERT,
	OP_VFTOISCALARINSERT,
	OP_VFCMPSCALARINSERT,
	OP_VFMLASCALARINSERT,
	OP_VFMLSSCALARINSERT,
	OP_VFNMLASCALARINSERT,
	OP_VFNMLSSCALARINSERT,
	OP_VFCOPYSIGN,
	OP_VMOV,
	OP_VECTORIMM,
	OP_LOADNAMEDVECTORCONSTANT,
	OP_LOADNAMEDVECTORINDEXEDCONSTANT,
	OP_VNEG,
	OP_VNOT,
	OP_VABS,
	OP_VPOPCOUNT,
	OP_VADDV,
	OP_VUMINV,
	OP_VUMAXV,
	OP_VFABS,
	OP_VFNEG,
	OP_VFRECP,
	OP_VFRECPPRECISION,
	OP_VFSQRT,
	OP_VFRSQRT,
	OP_VFRSQRTPRECISION,
	OP_VCMPEQZ,
	OP_VCMPGTZ,
	OP_VCMPLTZ,
	OP_VDUPELEMENT,
	OP_VSHLI,
	OP_VUSHRI,
	OP_VUSHRAI,
	OP_VSSHRI,
	OP_VUSHRNI,
	OP_VUSHRNI2,
	OP_VSXTL,
	OP_VSXTL2,
	OP_VSSHLL,
	OP_VSSHLL2,
	OP_VUXTL,
	OP_VUXTL2,
	OP_VSQXTN,
	OP_VSQXTN2,
	OP_VSQXTNPAIR,
	OP_VSQXTUN,
	OP_VSQXTUN2,
	OP_VSQXTUNPAIR,
	OP_VSRSHR,
	OP_VSQSHL,
	OP_VREV32,
	OP_VREV64,
	OP_VADD,
	OP_VSUB,
	OP_VAND,
	OP_VANDN,
	OP_VORN,
	OP_VOR,
	OP_VXOR,
	OP_VUQADD,
	OP_VUQSUB,
	OP_VSQADD,
	OP_VSQSUB,
	OP_VADDP,
	OP_VURAVG,
	OP_VUMIN,
	OP_VUMAX,
	OP_VSMIN,
	OP_VSMAX,
	OP_VZIP,
	OP_VZIP2,
	OP_VUNZIP,
	OP_VUNZIP2,
	OP_VTRN,
	OP_VTRN2,
	OP_VFADD,
	OP_VFADDP,
	OP_VFADDV,
	OP_VFSUB,
	OP_VFMUL,
	OP_VFDIV,
	OP_VFMIN,
	OP_VFMAX,
	OP_VMUL,
	OP_VUMULL,
	OP_VSMULL,
	OP_VUMULL2,
	OP_VSMULL2,
	OP_VUMULH,
	OP_VSMULH,
	OP_VUABDL,
	OP_VUABDL2,
	OP_VUSHL,
	OP_VUSHR,
	OP_VSSHR,
	OP_VUSHLS,
	OP_VUSHRS,
	OP_VSSHRS,
	OP_VUSHRSWIDE,
	OP_VSSHRSWIDE,
	OP_VUSHLSWIDE,
	OP_VINSELEMENT,
	OP_VINSGPR,
	OP_VEXTR,
	OP_VCMPEQ,
	OP_VCMPGT,
	OP_VFCMPEQ,
	OP_VFCMPNEQ,
	OP_VFCMPLT,
	OP_VFCMPGT,
	OP_VFCMPLE,
	OP_VFCMPORD,
	OP_VFCMPUNO,
	OP_VTBL1,
	OP_VTBL2,
	OP_VTBX1,
	OP_VBSL,
	OP_VPCMPESTRX,
	OP_VPCMPISTRX,
	OP_VFCADD,
	OP_VFMLA,
	OP_VFMLS,
	OP_VFNMLA,
	OP_VFNMLS,
	OP_VCASTFROMGPR,
	OP_VDUPFROMGPR,
	OP_VLOADTWOGPRS,
	OP_FLOAT_FROMGPR_S,
	OP_FLOAT_FTOF,
	OP_VECTOR_STOF,
	OP_VECTOR_FTOS,
	OP_VECTOR_FTOZS,
	OP_VECTOR_FTOF,
	OP_VFCVTL2,
	OP_VFCVTN2,
	OP_VECTOR_FTOI,
	OP_VECTOR_FTOISIZED,
	OP_VECTOR_F64TOI32,
	OP_VAESIMC,
	OP_VAESENC,
	OP_VAESENCLAST,
	OP_VAESDEC,
	OP_VAESDECLAST,
	OP_VAESKEYGENASSIST,
	OP_VSHA1H,
	OP_VSHA1C,
	OP_VSHA1M,
	OP_VSHA1P,
	OP_VSHA1SU1,
	OP_VSHA256U0,
	OP_VSHA256U1,
	OP_VSHA256H,
	OP_VSHA256H2,
	OP_CRC32,
	OP_PCLMUL,
	OP_F64ATAN,
	OP_F64FPREM,
	OP_F64FPREM1,
	OP_F64SCALE,
	OP_F64F2XM1,
	OP_F64FYL2X,
	OP_F64FYL2XP1,
	OP_F64TAN,
	OP_F64SIN,
	OP_F64COS,
	OP_F64SINCOS,
	OP_SYNCSTACKTOSLOW,
	OP_STACKFORCESLOW,
	OP_INITSTACK,
	OP_INCSTACKTOP,
	OP_DECSTACKTOP,
	OP_INVALIDATESTACK,
	OP_PUSHSTACK,
	OP_COPYPUSHSTACK,
	OP_STORESTACKMEM,
	OP_STORESTACKTOSTACK,
	OP_POPSTACKDESTROY,
	OP_READSTACKVALUE,
	OP_STACKVALIDTAG,
	OP_F80ADDSTACK,
	OP_F80ADDVALUE,
	OP_F80ADD,
	OP_F80SUBSTACK,
	OP_F80SUBVALUE,
	OP_F80SUBRVALUE,
	OP_F80SUB,
	OP_F80MULSTACK,
	OP_F80MULVALUE,
	OP_F80MUL,
	OP_F80DIVSTACK,
	OP_F80DIVVALUE,
	OP_F80DIVRVALUE,
	OP_F80DIV,
	OP_F80STACKXCHANGE,
	OP_F80STACKCHANGESIGN,
	OP_F80STACKABS,
	OP_F80PTANSTACK,
	OP_F80ATANSTACK,
	OP_F80ATAN,
	OP_F80FPREMSTACK,
	OP_F80FPREM,
	OP_F80FPREM1STACK,
	OP_F80FPREM1,
	OP_F80SCALESTACK,
	OP_F80SCALE,
	OP_F80CVT,
	OP_F80CVTINT,
	OP_F80CVTTO,
	OP_F80CVTTOINT,
	OP_F80ROUNDSTACK,
	OP_F80ROUND,
	OP_F80F2XM1STACK,
	OP_F80F2XM1,
	OP_F80TAN,
	OP_F80SINSTACK,
	OP_F80SIN,
	OP_F80COSSTACK,
	OP_F80COS,
	OP_F80SINCOS,
	OP_F80SINCOSSTACK,
	OP_F80SQRTSTACK,
	OP_F80SQRT,
	OP_F80XTRACT_EXP,
	OP_F80XTRACT_SIG,
	OP_F80STACKTEST,
	OP_F80CMPSTACK,
	OP_F80CMPVALUE,
	OP_F80CMP,
	OP_F80BCDLOAD,
	OP_F80BCDSTORE,
	OP_F80FYL2XSTACK,
	OP_F80FYL2X,
	OP_F80FYL2XP1STACK,
	OP_F80FYL2XP1,
	OP_F80VBSLSTACK,
	OP_LAST,
};
enum class CondClass : uint8_t {
	EQ    = 0,
	NEQ   = 1,
	UGE   = 2,
	ULT   = 3,
	MI    = 4,
	PL    = 5,
	VS    = 6,
	VC    = 7,
	UGT   = 8,
	ULE   = 9,
	SGE   = 10,
	SLT   = 11,
	SGT   = 12,
	SLE   = 13,
	TSTZ  = 14, /* bit test zero */
	TSTNZ = 15, /* bit test nonzero */

	FLU   = 16, /* float less or unordered */
	FGE   = 17, /* float greater or equal */
	FLEU  = 18, /* float less or equal or unordered */
	FGT   = 19, /* float greater */
	FU    = 20, /* float unordered */
	FNU   = 21, /* float not unordered */

	AL    = 32, /* always */
};

enum class FenceType : uint8_t {
	Load      = 0,
	Store     = 1,
	LoadStore = 2,
	Inst      = 3,
};

enum class MemOffsetType : uint8_t {
	SXTX = 0,
	UXTW = 1,
	SXTW = 2,
};

enum class RegClass : uint32_t {
	Invalid  = 0,
	GPR      = 1,
	GPRFixed = 2,
	FPR      = 3,
	FPRFixed = 4,
	Complex  = 5,
};

enum class RoundMode : uint8_t {
	Nearest     = 0,
	NegInfinity = 1,
	PosInfinity = 2,
	TowardsZero = 3, /* Truncate */
	Host        = 4,
};

enum class ConstPad : uint8_t {
	NoPad = 0,
	DoPad = 1,
	AutoPad = 2,
};

#undef IROP_ENUM
#endif

#ifdef IROP_STRUCTS
	constexpr uint8_t NumClasses {6};
	constexpr uint8_t FCMP_FLAG_EQ        = 0;
	constexpr uint8_t FCMP_FLAG_LT        = 1;
	constexpr uint8_t FCMP_FLAG_UNORDERED = 2;
	struct BreakDefinition {;
	  uint16_t ErrorRegister;;
	  uint8_t Signal;;
	  uint8_t TrapNumber;;
	  uint8_t si_code;;
	};;
// Default structs
struct __attribute__((packed)) IROp_Header {
	void* Data[0];
	IROps Op;

	IR::OpSize Size;
	IR::OpSize ElementSize;
	template<typename T>
	T const* C() const { return reinterpret_cast<T const*>(Data); }
	template<typename T>
	T* CW() { return reinterpret_cast<T*>(Data); }
	OrderedNodeWrapper Args[0];
};

static_assert(sizeof(IROp_Header) == sizeof(uint32_t), "IROp_Header should be 32-bits in size");

// User defined IR Op structs
struct __attribute__((packed)) IROp_Dummy {
	IROp_Header Header;
	static constexpr IROps OPCODE = OP_DUMMY;
};
static_assert(std::is_trivially_copyable_v<IROp_Dummy>);
static_assert(std::is_standard_layout_v<IROp_Dummy>);

struct __attribute__((packed)) IROp_IRHeader {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Blocks;
	// Non-SSA arguments
	uint64_t OriginalRIP;
	uint32_t BlockCount;
	uint32_t NumHostInstructions;
	uint32_t SpillSlots;
	bool PostRA;
	bool HasX87;
	bool ReadsParity;
	static constexpr IROps OPCODE = OP_IRHEADER;
	// Get index of argument by name
	static constexpr size_t Blocks_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_IRHeader>);
static_assert(std::is_standard_layout_v<IROp_IRHeader>);

struct __attribute__((packed)) IROp_CodeBlock {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Begin;
	OrderedNodeWrapper Last;
	// Non-SSA arguments
	uint32_t ID;
	bool EntryPoint;
	uint32_t GuestEntryOffset;
	static constexpr IROps OPCODE = OP_CODEBLOCK;
	// Get index of argument by name
	static constexpr size_t Begin_Index = 0;
	static constexpr size_t Last_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_CodeBlock>);
static_assert(std::is_standard_layout_v<IROp_CodeBlock>);

struct __attribute__((packed)) IROp_BeginBlock {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper BlockHeader;
	static constexpr IROps OPCODE = OP_BEGINBLOCK;
	// Get index of argument by name
	static constexpr size_t BlockHeader_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_BeginBlock>);
static_assert(std::is_standard_layout_v<IROp_BeginBlock>);

struct __attribute__((packed)) IROp_InvalidateFlags {
	IROp_Header Header;
	// Non-SSA arguments
	uint64_t Flags;
	static constexpr IROps OPCODE = OP_INVALIDATEFLAGS;
};
static_assert(std::is_trivially_copyable_v<IROp_InvalidateFlags>);
static_assert(std::is_standard_layout_v<IROp_InvalidateFlags>);

struct __attribute__((packed)) IROp_EndBlock {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper BlockHeader;
	static constexpr IROps OPCODE = OP_ENDBLOCK;
	// Get index of argument by name
	static constexpr size_t BlockHeader_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_EndBlock>);
static_assert(std::is_standard_layout_v<IROp_EndBlock>);

struct __attribute__((packed)) IROp_GuestOpcode {
	IROp_Header Header;
	// Non-SSA arguments
	uint32_t GuestEntryOffset;
	static constexpr IROps OPCODE = OP_GUESTOPCODE;
};
static_assert(std::is_trivially_copyable_v<IROp_GuestOpcode>);
static_assert(std::is_standard_layout_v<IROp_GuestOpcode>);

struct __attribute__((packed)) IROp_ValidateCode {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Address;
	// Non-SSA arguments
	std::array<uint8_t, 16> CodeOriginal;
	uint8_t CodeLength;
	static constexpr IROps OPCODE = OP_VALIDATECODE;
	// Get index of argument by name
	static constexpr size_t Address_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_ValidateCode>);
static_assert(std::is_standard_layout_v<IROp_ValidateCode>);

struct __attribute__((packed)) IROp_ThreadRemoveCodeEntry {
	IROp_Header Header;
	static constexpr IROps OPCODE = OP_THREADREMOVECODEENTRY;
};
static_assert(std::is_trivially_copyable_v<IROp_ThreadRemoveCodeEntry>);
static_assert(std::is_standard_layout_v<IROp_ThreadRemoveCodeEntry>);

struct __attribute__((packed)) IROp_ProcessorID {
	IROp_Header Header;
	static constexpr IROps OPCODE = OP_PROCESSORID;
};
static_assert(std::is_trivially_copyable_v<IROp_ProcessorID>);
static_assert(std::is_standard_layout_v<IROp_ProcessorID>);

struct __attribute__((packed)) IROp_GetRoundingMode {
	IROp_Header Header;
	static constexpr IROps OPCODE = OP_GETROUNDINGMODE;
};
static_assert(std::is_trivially_copyable_v<IROp_GetRoundingMode>);
static_assert(std::is_standard_layout_v<IROp_GetRoundingMode>);

struct __attribute__((packed)) IROp_SetRoundingMode {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper RoundMode;
	OrderedNodeWrapper MXCSR;
	// Non-SSA arguments
	bool SetDAZ;
	static constexpr IROps OPCODE = OP_SETROUNDINGMODE;
	// Get index of argument by name
	static constexpr size_t RoundMode_Index = 0;
	static constexpr size_t MXCSR_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_SetRoundingMode>);
static_assert(std::is_standard_layout_v<IROp_SetRoundingMode>);

struct __attribute__((packed)) IROp_PushRoundingMode {
	IROp_Header Header;
	// Non-SSA arguments
	uint8_t RoundMode;
	static constexpr IROps OPCODE = OP_PUSHROUNDINGMODE;
};
static_assert(std::is_trivially_copyable_v<IROp_PushRoundingMode>);
static_assert(std::is_standard_layout_v<IROp_PushRoundingMode>);

struct __attribute__((packed)) IROp_PopRoundingMode {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper FPCR;
	static constexpr IROps OPCODE = OP_POPROUNDINGMODE;
	// Get index of argument by name
	static constexpr size_t FPCR_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_PopRoundingMode>);
static_assert(std::is_standard_layout_v<IROp_PopRoundingMode>);

struct __attribute__((packed)) IROp_Print {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Value;
	static constexpr IROps OPCODE = OP_PRINT;
	// Get index of argument by name
	static constexpr size_t Value_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_Print>);
static_assert(std::is_standard_layout_v<IROp_Print>);

struct __attribute__((packed)) IROp_PrintMsg {
	IROp_Header Header;
	// Non-SSA arguments
	const char* Value;
	static constexpr IROps OPCODE = OP_PRINTMSG;
};
static_assert(std::is_trivially_copyable_v<IROp_PrintMsg>);
static_assert(std::is_standard_layout_v<IROp_PrintMsg>);

struct __attribute__((packed)) IROp_AllocateGPR {
	IROp_Header Header;
	// Non-SSA arguments
	bool ForPair;
	static constexpr IROps OPCODE = OP_ALLOCATEGPR;
};
static_assert(std::is_trivially_copyable_v<IROp_AllocateGPR>);
static_assert(std::is_standard_layout_v<IROp_AllocateGPR>);

struct __attribute__((packed)) IROp_AllocateFPR {
	IROp_Header Header;
	static constexpr IROps OPCODE = OP_ALLOCATEFPR;
};
static_assert(std::is_trivially_copyable_v<IROp_AllocateFPR>);
static_assert(std::is_standard_layout_v<IROp_AllocateFPR>);

struct __attribute__((packed)) IROp_AllocateGPRAfter {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper After;
	static constexpr IROps OPCODE = OP_ALLOCATEGPRAFTER;
	// Get index of argument by name
	static constexpr size_t After_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_AllocateGPRAfter>);
static_assert(std::is_standard_layout_v<IROp_AllocateGPRAfter>);

struct __attribute__((packed)) IROp_RDRAND {
	IROp_Header Header;
	// Non-SSA arguments
	bool GetReseeded;
	static constexpr IROps OPCODE = OP_RDRAND;
};
static_assert(std::is_trivially_copyable_v<IROp_RDRAND>);
static_assert(std::is_standard_layout_v<IROp_RDRAND>);

struct __attribute__((packed)) IROp_Yield {
	IROp_Header Header;
	static constexpr IROps OPCODE = OP_YIELD;
};
static_assert(std::is_trivially_copyable_v<IROp_Yield>);
static_assert(std::is_standard_layout_v<IROp_Yield>);

struct __attribute__((packed)) IROp_WFET {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Upper;
	OrderedNodeWrapper Lower;
	static constexpr IROps OPCODE = OP_WFET;
	// Get index of argument by name
	static constexpr size_t Upper_Index = 0;
	static constexpr size_t Lower_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_WFET>);
static_assert(std::is_standard_layout_v<IROp_WFET>);

struct __attribute__((packed)) IROp_MonoBackpatcherWrite {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Value;
	OrderedNodeWrapper Addr;
	// Non-SSA arguments
	FEXCore::IR::OpSize Size;
	static constexpr IROps OPCODE = OP_MONOBACKPATCHERWRITE;
	// Get index of argument by name
	static constexpr size_t Value_Index = 0;
	static constexpr size_t Addr_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_MonoBackpatcherWrite>);
static_assert(std::is_standard_layout_v<IROp_MonoBackpatcherWrite>);

struct __attribute__((packed)) IROp_Jump {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper TargetBlock;
	static constexpr IROps OPCODE = OP_JUMP;
	// Get index of argument by name
	static constexpr size_t TargetBlock_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_Jump>);
static_assert(std::is_standard_layout_v<IROp_Jump>);

struct __attribute__((packed)) IROp_CondJump {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Cmp1;
	OrderedNodeWrapper Cmp2;
	OrderedNodeWrapper TrueBlock;
	OrderedNodeWrapper FalseBlock;
	// Non-SSA arguments
	CondClass Cond;
	FEXCore::IR::OpSize CompareSize;
	bool FromNZCV;
	static constexpr IROps OPCODE = OP_CONDJUMP;
	// Get index of argument by name
	static constexpr size_t Cmp1_Index = 0;
	static constexpr size_t Cmp2_Index = 1;
	static constexpr size_t TrueBlock_Index = 2;
	static constexpr size_t FalseBlock_Index = 3;
};
static_assert(std::is_trivially_copyable_v<IROp_CondJump>);
static_assert(std::is_standard_layout_v<IROp_CondJump>);

struct __attribute__((packed)) IROp_ExitFunction {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper NewRIP;
	OrderedNodeWrapper CallReturnAddress;
	OrderedNodeWrapper CallReturnBlock;
	// Non-SSA arguments
	FEXCore::IR::BranchHint Hint;
	static constexpr IROps OPCODE = OP_EXITFUNCTION;
	// Get index of argument by name
	static constexpr size_t NewRIP_Index = 0;
	static constexpr size_t CallReturnAddress_Index = 1;
	static constexpr size_t CallReturnBlock_Index = 2;
};
static_assert(std::is_trivially_copyable_v<IROp_ExitFunction>);
static_assert(std::is_standard_layout_v<IROp_ExitFunction>);

struct __attribute__((packed)) IROp_Break {
	IROp_Header Header;
	// Non-SSA arguments
	BreakDefinition Reason;
	static constexpr IROps OPCODE = OP_BREAK;
};
static_assert(std::is_trivially_copyable_v<IROp_Break>);
static_assert(std::is_standard_layout_v<IROp_Break>);

struct __attribute__((packed)) IROp_CallbackReturn {
	IROp_Header Header;
	static constexpr IROps OPCODE = OP_CALLBACKRETURN;
};
static_assert(std::is_trivially_copyable_v<IROp_CallbackReturn>);
static_assert(std::is_standard_layout_v<IROp_CallbackReturn>);

struct __attribute__((packed)) IROp_Syscall {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper SyscallID;
	OrderedNodeWrapper Arg0;
	OrderedNodeWrapper Arg1;
	OrderedNodeWrapper Arg2;
	OrderedNodeWrapper Arg3;
	OrderedNodeWrapper Arg4;
	OrderedNodeWrapper Arg5;
	static constexpr IROps OPCODE = OP_SYSCALL;
	// Get index of argument by name
	static constexpr size_t SyscallID_Index = 0;
	static constexpr size_t Arg0_Index = 1;
	static constexpr size_t Arg1_Index = 2;
	static constexpr size_t Arg2_Index = 3;
	static constexpr size_t Arg3_Index = 4;
	static constexpr size_t Arg4_Index = 5;
	static constexpr size_t Arg5_Index = 6;
};
static_assert(std::is_trivially_copyable_v<IROp_Syscall>);
static_assert(std::is_standard_layout_v<IROp_Syscall>);

struct __attribute__((packed)) IROp_Thunk {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper ArgPtr;
	// Non-SSA arguments
	SHA256Sum ThunkNameHash;
	static constexpr IROps OPCODE = OP_THUNK;
	// Get index of argument by name
	static constexpr size_t ArgPtr_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_Thunk>);
static_assert(std::is_standard_layout_v<IROp_Thunk>);

struct __attribute__((packed)) IROp_CPUID {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Function;
	OrderedNodeWrapper Leaf;
	OrderedNodeWrapper OutEAX;
	OrderedNodeWrapper OutEBX;
	OrderedNodeWrapper OutECX;
	OrderedNodeWrapper OutEDX;
	static constexpr IROps OPCODE = OP_CPUID;
	// Get index of argument by name
	static constexpr size_t Function_Index = 0;
	static constexpr size_t Leaf_Index = 1;
	static constexpr size_t OutEAX_Index = 2;
	static constexpr size_t OutEBX_Index = 3;
	static constexpr size_t OutECX_Index = 4;
	static constexpr size_t OutEDX_Index = 5;
};
static_assert(std::is_trivially_copyable_v<IROp_CPUID>);
static_assert(std::is_standard_layout_v<IROp_CPUID>);

struct __attribute__((packed)) IROp_XGetBV {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Function;
	OrderedNodeWrapper OutEAX;
	OrderedNodeWrapper OutEDX;
	static constexpr IROps OPCODE = OP_XGETBV;
	// Get index of argument by name
	static constexpr size_t Function_Index = 0;
	static constexpr size_t OutEAX_Index = 1;
	static constexpr size_t OutEDX_Index = 2;
};
static_assert(std::is_trivially_copyable_v<IROp_XGetBV>);
static_assert(std::is_standard_layout_v<IROp_XGetBV>);

struct __attribute__((packed)) IROp_Copy {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Source;
	static constexpr IROps OPCODE = OP_COPY;
	// Get index of argument by name
	static constexpr size_t Source_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_Copy>);
static_assert(std::is_standard_layout_v<IROp_Copy>);

struct __attribute__((packed)) IROp_LoadRegister {
	IROp_Header Header;
	// Non-SSA arguments
	uint32_t Reg;
	RegClass Class;
	static constexpr IROps OPCODE = OP_LOADREGISTER;
};
static_assert(std::is_trivially_copyable_v<IROp_LoadRegister>);
static_assert(std::is_standard_layout_v<IROp_LoadRegister>);

struct __attribute__((packed)) IROp_LoadPF {
	IROp_Header Header;
	static constexpr IROps OPCODE = OP_LOADPF;
};
static_assert(std::is_trivially_copyable_v<IROp_LoadPF>);
static_assert(std::is_standard_layout_v<IROp_LoadPF>);

struct __attribute__((packed)) IROp_LoadAF {
	IROp_Header Header;
	static constexpr IROps OPCODE = OP_LOADAF;
};
static_assert(std::is_trivially_copyable_v<IROp_LoadAF>);
static_assert(std::is_standard_layout_v<IROp_LoadAF>);

struct __attribute__((packed)) IROp_StoreRegister {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Value;
	static constexpr IROps OPCODE = OP_STOREREGISTER;
	// Get index of argument by name
	static constexpr size_t Value_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_StoreRegister>);
static_assert(std::is_standard_layout_v<IROp_StoreRegister>);

struct __attribute__((packed)) IROp_StorePF {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Value;
	static constexpr IROps OPCODE = OP_STOREPF;
	// Get index of argument by name
	static constexpr size_t Value_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_StorePF>);
static_assert(std::is_standard_layout_v<IROp_StorePF>);

struct __attribute__((packed)) IROp_StoreAF {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Value;
	static constexpr IROps OPCODE = OP_STOREAF;
	// Get index of argument by name
	static constexpr size_t Value_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_StoreAF>);
static_assert(std::is_standard_layout_v<IROp_StoreAF>);

struct __attribute__((packed)) IROp_LoadContext {
	IROp_Header Header;
	// Non-SSA arguments
	RegClass Class;
	uint32_t Offset;
	static constexpr IROps OPCODE = OP_LOADCONTEXT;
};
static_assert(std::is_trivially_copyable_v<IROp_LoadContext>);
static_assert(std::is_standard_layout_v<IROp_LoadContext>);

struct __attribute__((packed)) IROp_LoadContextPair {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper OutValue1;
	OrderedNodeWrapper OutValue2;
	// Non-SSA arguments
	RegClass Class;
	uint32_t Offset;
	static constexpr IROps OPCODE = OP_LOADCONTEXTPAIR;
	// Get index of argument by name
	static constexpr size_t OutValue1_Index = 0;
	static constexpr size_t OutValue2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_LoadContextPair>);
static_assert(std::is_standard_layout_v<IROp_LoadContextPair>);

struct __attribute__((packed)) IROp_StoreContext {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Value;
	// Non-SSA arguments
	RegClass Class;
	uint32_t Offset;
	static constexpr IROps OPCODE = OP_STORECONTEXT;
	// Get index of argument by name
	static constexpr size_t Value_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_StoreContext>);
static_assert(std::is_standard_layout_v<IROp_StoreContext>);

struct __attribute__((packed)) IROp_StoreContextPair {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Value1;
	OrderedNodeWrapper Value2;
	// Non-SSA arguments
	RegClass Class;
	uint32_t Offset;
	static constexpr IROps OPCODE = OP_STORECONTEXTPAIR;
	// Get index of argument by name
	static constexpr size_t Value1_Index = 0;
	static constexpr size_t Value2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_StoreContextPair>);
static_assert(std::is_standard_layout_v<IROp_StoreContextPair>);

struct __attribute__((packed)) IROp_LoadContextIndexed {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Index;
	// Non-SSA arguments
	uint32_t BaseOffset;
	uint32_t Stride;
	RegClass Class;
	static constexpr IROps OPCODE = OP_LOADCONTEXTINDEXED;
	// Get index of argument by name
	static constexpr size_t Index_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_LoadContextIndexed>);
static_assert(std::is_standard_layout_v<IROp_LoadContextIndexed>);

struct __attribute__((packed)) IROp_StoreContextIndexed {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Value;
	OrderedNodeWrapper Index;
	// Non-SSA arguments
	uint32_t BaseOffset;
	uint32_t Stride;
	RegClass Class;
	static constexpr IROps OPCODE = OP_STORECONTEXTINDEXED;
	// Get index of argument by name
	static constexpr size_t Value_Index = 0;
	static constexpr size_t Index_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_StoreContextIndexed>);
static_assert(std::is_standard_layout_v<IROp_StoreContextIndexed>);

struct __attribute__((packed)) IROp_FormContextAddress {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Index;
	// Non-SSA arguments
	uint32_t Stride;
	static constexpr IROps OPCODE = OP_FORMCONTEXTADDRESS;
	// Get index of argument by name
	static constexpr size_t Index_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_FormContextAddress>);
static_assert(std::is_standard_layout_v<IROp_FormContextAddress>);

struct __attribute__((packed)) IROp_SpillRegister {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Value;
	// Non-SSA arguments
	uint32_t Slot;
	RegClass Class;
	static constexpr IROps OPCODE = OP_SPILLREGISTER;
	// Get index of argument by name
	static constexpr size_t Value_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_SpillRegister>);
static_assert(std::is_standard_layout_v<IROp_SpillRegister>);

struct __attribute__((packed)) IROp_FillRegister {
	IROp_Header Header;
	// Non-SSA arguments
	uint32_t Slot;
	RegClass Class;
	static constexpr IROps OPCODE = OP_FILLREGISTER;
};
static_assert(std::is_trivially_copyable_v<IROp_FillRegister>);
static_assert(std::is_standard_layout_v<IROp_FillRegister>);

struct __attribute__((packed)) IROp_LoadNZCV {
	IROp_Header Header;
	static constexpr IROps OPCODE = OP_LOADNZCV;
};
static_assert(std::is_trivially_copyable_v<IROp_LoadNZCV>);
static_assert(std::is_standard_layout_v<IROp_LoadNZCV>);

struct __attribute__((packed)) IROp_StoreNZCV {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Value;
	static constexpr IROps OPCODE = OP_STORENZCV;
	// Get index of argument by name
	static constexpr size_t Value_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_StoreNZCV>);
static_assert(std::is_standard_layout_v<IROp_StoreNZCV>);

struct __attribute__((packed)) IROp_LoadDF {
	IROp_Header Header;
	static constexpr IROps OPCODE = OP_LOADDF;
};
static_assert(std::is_trivially_copyable_v<IROp_LoadDF>);
static_assert(std::is_standard_layout_v<IROp_LoadDF>);

struct __attribute__((packed)) IROp_LoadMem {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Addr;
	OrderedNodeWrapper Offset;
	// Non-SSA arguments
	RegClass Class;
	FEXCore::IR::OpSize Align;
	MemOffsetType OffsetType;
	uint8_t OffsetScale;
	static constexpr IROps OPCODE = OP_LOADMEM;
	// Get index of argument by name
	static constexpr size_t Addr_Index = 0;
	static constexpr size_t Offset_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_LoadMem>);
static_assert(std::is_standard_layout_v<IROp_LoadMem>);

struct __attribute__((packed)) IROp_LoadMemPair {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Addr;
	OrderedNodeWrapper OutValue1;
	OrderedNodeWrapper OutValue2;
	// Non-SSA arguments
	RegClass Class;
	uint32_t Offset;
	static constexpr IROps OPCODE = OP_LOADMEMPAIR;
	// Get index of argument by name
	static constexpr size_t Addr_Index = 0;
	static constexpr size_t OutValue1_Index = 1;
	static constexpr size_t OutValue2_Index = 2;
};
static_assert(std::is_trivially_copyable_v<IROp_LoadMemPair>);
static_assert(std::is_standard_layout_v<IROp_LoadMemPair>);

struct __attribute__((packed)) IROp_StoreMem {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Value;
	OrderedNodeWrapper Addr;
	OrderedNodeWrapper Offset;
	// Non-SSA arguments
	RegClass Class;
	FEXCore::IR::OpSize Align;
	MemOffsetType OffsetType;
	uint8_t OffsetScale;
	static constexpr IROps OPCODE = OP_STOREMEM;
	// Get index of argument by name
	static constexpr size_t Value_Index = 0;
	static constexpr size_t Addr_Index = 1;
	static constexpr size_t Offset_Index = 2;
};
static_assert(std::is_trivially_copyable_v<IROp_StoreMem>);
static_assert(std::is_standard_layout_v<IROp_StoreMem>);

struct __attribute__((packed)) IROp_StoreMemPair {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Value1;
	OrderedNodeWrapper Value2;
	OrderedNodeWrapper Addr;
	// Non-SSA arguments
	RegClass Class;
	uint32_t Offset;
	static constexpr IROps OPCODE = OP_STOREMEMPAIR;
	// Get index of argument by name
	static constexpr size_t Value1_Index = 0;
	static constexpr size_t Value2_Index = 1;
	static constexpr size_t Addr_Index = 2;
};
static_assert(std::is_trivially_copyable_v<IROp_StoreMemPair>);
static_assert(std::is_standard_layout_v<IROp_StoreMemPair>);

struct __attribute__((packed)) IROp_StoreMemX87SVEOptPredicate {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Value;
	OrderedNodeWrapper Addr;
	static constexpr IROps OPCODE = OP_STOREMEMX87SVEOPTPREDICATE;
	// Get index of argument by name
	static constexpr size_t Value_Index = 0;
	static constexpr size_t Addr_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_StoreMemX87SVEOptPredicate>);
static_assert(std::is_standard_layout_v<IROp_StoreMemX87SVEOptPredicate>);

struct __attribute__((packed)) IROp_LoadMemX87SVEOptPredicate {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Addr;
	static constexpr IROps OPCODE = OP_LOADMEMX87SVEOPTPREDICATE;
	// Get index of argument by name
	static constexpr size_t Addr_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_LoadMemX87SVEOptPredicate>);
static_assert(std::is_standard_layout_v<IROp_LoadMemX87SVEOptPredicate>);

struct __attribute__((packed)) IROp_LoadMemTSO {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Addr;
	OrderedNodeWrapper Offset;
	// Non-SSA arguments
	RegClass Class;
	FEXCore::IR::OpSize Align;
	MemOffsetType OffsetType;
	uint8_t OffsetScale;
	static constexpr IROps OPCODE = OP_LOADMEMTSO;
	// Get index of argument by name
	static constexpr size_t Addr_Index = 0;
	static constexpr size_t Offset_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_LoadMemTSO>);
static_assert(std::is_standard_layout_v<IROp_LoadMemTSO>);

struct __attribute__((packed)) IROp_StoreMemTSO {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Value;
	OrderedNodeWrapper Addr;
	OrderedNodeWrapper Offset;
	// Non-SSA arguments
	RegClass Class;
	FEXCore::IR::OpSize Align;
	MemOffsetType OffsetType;
	uint8_t OffsetScale;
	static constexpr IROps OPCODE = OP_STOREMEMTSO;
	// Get index of argument by name
	static constexpr size_t Value_Index = 0;
	static constexpr size_t Addr_Index = 1;
	static constexpr size_t Offset_Index = 2;
};
static_assert(std::is_trivially_copyable_v<IROp_StoreMemTSO>);
static_assert(std::is_standard_layout_v<IROp_StoreMemTSO>);

struct __attribute__((packed)) IROp_VLoadVectorMasked {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Mask;
	OrderedNodeWrapper Addr;
	OrderedNodeWrapper Offset;
	// Non-SSA arguments
	MemOffsetType OffsetType;
	uint8_t OffsetScale;
	static constexpr IROps OPCODE = OP_VLOADVECTORMASKED;
	// Get index of argument by name
	static constexpr size_t Mask_Index = 0;
	static constexpr size_t Addr_Index = 1;
	static constexpr size_t Offset_Index = 2;
};
static_assert(std::is_trivially_copyable_v<IROp_VLoadVectorMasked>);
static_assert(std::is_standard_layout_v<IROp_VLoadVectorMasked>);

struct __attribute__((packed)) IROp_VStoreVectorMasked {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Mask;
	OrderedNodeWrapper Data;
	OrderedNodeWrapper Addr;
	OrderedNodeWrapper Offset;
	// Non-SSA arguments
	MemOffsetType OffsetType;
	uint8_t OffsetScale;
	static constexpr IROps OPCODE = OP_VSTOREVECTORMASKED;
	// Get index of argument by name
	static constexpr size_t Mask_Index = 0;
	static constexpr size_t Data_Index = 1;
	static constexpr size_t Addr_Index = 2;
	static constexpr size_t Offset_Index = 3;
};
static_assert(std::is_trivially_copyable_v<IROp_VStoreVectorMasked>);
static_assert(std::is_standard_layout_v<IROp_VStoreVectorMasked>);

struct __attribute__((packed)) IROp_VLoadVectorGatherMasked {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Incoming;
	OrderedNodeWrapper Mask;
	OrderedNodeWrapper AddrBase;
	OrderedNodeWrapper VectorIndexLow;
	OrderedNodeWrapper VectorIndexHigh;
	// Non-SSA arguments
	FEXCore::IR::OpSize VectorIndexElementSize;
	uint8_t OffsetScale;
	uint8_t DataElementOffsetStart;
	uint8_t IndexElementOffsetStart;
	FEXCore::IR::OpSize AddrSize;
	static constexpr IROps OPCODE = OP_VLOADVECTORGATHERMASKED;
	// Get index of argument by name
	static constexpr size_t Incoming_Index = 0;
	static constexpr size_t Mask_Index = 1;
	static constexpr size_t AddrBase_Index = 2;
	static constexpr size_t VectorIndexLow_Index = 3;
	static constexpr size_t VectorIndexHigh_Index = 4;
};
static_assert(std::is_trivially_copyable_v<IROp_VLoadVectorGatherMasked>);
static_assert(std::is_standard_layout_v<IROp_VLoadVectorGatherMasked>);

struct __attribute__((packed)) IROp_VLoadVectorGatherMaskedQPS {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Incoming;
	OrderedNodeWrapper MaskReg;
	OrderedNodeWrapper AddrBase;
	OrderedNodeWrapper VectorIndexLow;
	OrderedNodeWrapper VectorIndexHigh;
	// Non-SSA arguments
	uint8_t OffsetScale;
	FEXCore::IR::OpSize AddrSize;
	static constexpr IROps OPCODE = OP_VLOADVECTORGATHERMASKEDQPS;
	// Get index of argument by name
	static constexpr size_t Incoming_Index = 0;
	static constexpr size_t MaskReg_Index = 1;
	static constexpr size_t AddrBase_Index = 2;
	static constexpr size_t VectorIndexLow_Index = 3;
	static constexpr size_t VectorIndexHigh_Index = 4;
};
static_assert(std::is_trivially_copyable_v<IROp_VLoadVectorGatherMaskedQPS>);
static_assert(std::is_standard_layout_v<IROp_VLoadVectorGatherMaskedQPS>);

struct __attribute__((packed)) IROp_VLoadVectorElement {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper DstSrc;
	OrderedNodeWrapper Addr;
	// Non-SSA arguments
	uint8_t Index;
	static constexpr IROps OPCODE = OP_VLOADVECTORELEMENT;
	// Get index of argument by name
	static constexpr size_t DstSrc_Index = 0;
	static constexpr size_t Addr_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_VLoadVectorElement>);
static_assert(std::is_standard_layout_v<IROp_VLoadVectorElement>);

struct __attribute__((packed)) IROp_VStoreVectorElement {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Value;
	OrderedNodeWrapper Addr;
	// Non-SSA arguments
	uint8_t Index;
	static constexpr IROps OPCODE = OP_VSTOREVECTORELEMENT;
	// Get index of argument by name
	static constexpr size_t Value_Index = 0;
	static constexpr size_t Addr_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_VStoreVectorElement>);
static_assert(std::is_standard_layout_v<IROp_VStoreVectorElement>);

struct __attribute__((packed)) IROp_VBroadcastFromMem {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Address;
	static constexpr IROps OPCODE = OP_VBROADCASTFROMMEM;
	// Get index of argument by name
	static constexpr size_t Address_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_VBroadcastFromMem>);
static_assert(std::is_standard_layout_v<IROp_VBroadcastFromMem>);

struct __attribute__((packed)) IROp_Push {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Value;
	OrderedNodeWrapper Addr;
	// Non-SSA arguments
	FEXCore::IR::OpSize ValueSize;
	static constexpr IROps OPCODE = OP_PUSH;
	// Get index of argument by name
	static constexpr size_t Value_Index = 0;
	static constexpr size_t Addr_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_Push>);
static_assert(std::is_standard_layout_v<IROp_Push>);

struct __attribute__((packed)) IROp_PushTwo {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Value1;
	OrderedNodeWrapper Value2;
	OrderedNodeWrapper Addr;
	// Non-SSA arguments
	FEXCore::IR::OpSize ValueSize;
	static constexpr IROps OPCODE = OP_PUSHTWO;
	// Get index of argument by name
	static constexpr size_t Value1_Index = 0;
	static constexpr size_t Value2_Index = 1;
	static constexpr size_t Addr_Index = 2;
};
static_assert(std::is_trivially_copyable_v<IROp_PushTwo>);
static_assert(std::is_standard_layout_v<IROp_PushTwo>);

struct __attribute__((packed)) IROp_RMWHandle {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Value;
	static constexpr IROps OPCODE = OP_RMWHANDLE;
	// Get index of argument by name
	static constexpr size_t Value_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_RMWHandle>);
static_assert(std::is_standard_layout_v<IROp_RMWHandle>);

struct __attribute__((packed)) IROp_Pop {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper InoutAddr;
	OrderedNodeWrapper OutValue;
	// Non-SSA arguments
	FEXCore::IR::OpSize Size;
	static constexpr IROps OPCODE = OP_POP;
	// Get index of argument by name
	static constexpr size_t InoutAddr_Index = 0;
	static constexpr size_t OutValue_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_Pop>);
static_assert(std::is_standard_layout_v<IROp_Pop>);

struct __attribute__((packed)) IROp_PopTwo {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper InoutAddr;
	OrderedNodeWrapper OutValue1;
	OrderedNodeWrapper OutValue2;
	// Non-SSA arguments
	FEXCore::IR::OpSize Size;
	static constexpr IROps OPCODE = OP_POPTWO;
	// Get index of argument by name
	static constexpr size_t InoutAddr_Index = 0;
	static constexpr size_t OutValue1_Index = 1;
	static constexpr size_t OutValue2_Index = 2;
};
static_assert(std::is_trivially_copyable_v<IROp_PopTwo>);
static_assert(std::is_standard_layout_v<IROp_PopTwo>);

struct __attribute__((packed)) IROp_MemSet {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Prefix;
	OrderedNodeWrapper Addr;
	OrderedNodeWrapper Value;
	OrderedNodeWrapper Length;
	OrderedNodeWrapper Direction;
	// Non-SSA arguments
	bool IsAtomic;
	FEXCore::IR::OpSize Size;
	static constexpr IROps OPCODE = OP_MEMSET;
	// Get index of argument by name
	static constexpr size_t Prefix_Index = 0;
	static constexpr size_t Addr_Index = 1;
	static constexpr size_t Value_Index = 2;
	static constexpr size_t Length_Index = 3;
	static constexpr size_t Direction_Index = 4;
};
static_assert(std::is_trivially_copyable_v<IROp_MemSet>);
static_assert(std::is_standard_layout_v<IROp_MemSet>);

struct __attribute__((packed)) IROp_MemCpy {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Dest;
	OrderedNodeWrapper Src;
	OrderedNodeWrapper Length;
	OrderedNodeWrapper Direction;
	OrderedNodeWrapper OutDstAddress;
	OrderedNodeWrapper OutSrcAddress;
	// Non-SSA arguments
	bool IsAtomic;
	FEXCore::IR::OpSize Size;
	static constexpr IROps OPCODE = OP_MEMCPY;
	// Get index of argument by name
	static constexpr size_t Dest_Index = 0;
	static constexpr size_t Src_Index = 1;
	static constexpr size_t Length_Index = 2;
	static constexpr size_t Direction_Index = 3;
	static constexpr size_t OutDstAddress_Index = 4;
	static constexpr size_t OutSrcAddress_Index = 5;
};
static_assert(std::is_trivially_copyable_v<IROp_MemCpy>);
static_assert(std::is_standard_layout_v<IROp_MemCpy>);

struct __attribute__((packed)) IROp_CacheLineClear {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Addr;
	// Non-SSA arguments
	bool Serialize;
	static constexpr IROps OPCODE = OP_CACHELINECLEAR;
	// Get index of argument by name
	static constexpr size_t Addr_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_CacheLineClear>);
static_assert(std::is_standard_layout_v<IROp_CacheLineClear>);

struct __attribute__((packed)) IROp_CacheLineClean {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Addr;
	static constexpr IROps OPCODE = OP_CACHELINECLEAN;
	// Get index of argument by name
	static constexpr size_t Addr_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_CacheLineClean>);
static_assert(std::is_standard_layout_v<IROp_CacheLineClean>);

struct __attribute__((packed)) IROp_CacheLineZero {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Addr;
	static constexpr IROps OPCODE = OP_CACHELINEZERO;
	// Get index of argument by name
	static constexpr size_t Addr_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_CacheLineZero>);
static_assert(std::is_standard_layout_v<IROp_CacheLineZero>);

struct __attribute__((packed)) IROp_Fence {
	IROp_Header Header;
	// Non-SSA arguments
	FenceType Fence;
	static constexpr IROps OPCODE = OP_FENCE;
};
static_assert(std::is_trivially_copyable_v<IROp_Fence>);
static_assert(std::is_standard_layout_v<IROp_Fence>);

struct __attribute__((packed)) IROp_Prefetch {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Addr;
	OrderedNodeWrapper Offset;
	// Non-SSA arguments
	bool ForStore;
	bool Stream;
	int8_t CacheLevel;
	MemOffsetType OffsetType;
	uint8_t OffsetScale;
	static constexpr IROps OPCODE = OP_PREFETCH;
	// Get index of argument by name
	static constexpr size_t Addr_Index = 0;
	static constexpr size_t Offset_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_Prefetch>);
static_assert(std::is_standard_layout_v<IROp_Prefetch>);

struct __attribute__((packed)) IROp_VStoreNonTemporal {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Value;
	OrderedNodeWrapper Addr;
	// Non-SSA arguments
	int8_t Offset;
	static constexpr IROps OPCODE = OP_VSTORENONTEMPORAL;
	// Get index of argument by name
	static constexpr size_t Value_Index = 0;
	static constexpr size_t Addr_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_VStoreNonTemporal>);
static_assert(std::is_standard_layout_v<IROp_VStoreNonTemporal>);

struct __attribute__((packed)) IROp_VStoreNonTemporalPair {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper ValueLow;
	OrderedNodeWrapper ValueHigh;
	OrderedNodeWrapper Addr;
	// Non-SSA arguments
	int8_t Offset;
	static constexpr IROps OPCODE = OP_VSTORENONTEMPORALPAIR;
	// Get index of argument by name
	static constexpr size_t ValueLow_Index = 0;
	static constexpr size_t ValueHigh_Index = 1;
	static constexpr size_t Addr_Index = 2;
};
static_assert(std::is_trivially_copyable_v<IROp_VStoreNonTemporalPair>);
static_assert(std::is_standard_layout_v<IROp_VStoreNonTemporalPair>);

struct __attribute__((packed)) IROp_VLoadNonTemporal {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Addr;
	// Non-SSA arguments
	int8_t Offset;
	static constexpr IROps OPCODE = OP_VLOADNONTEMPORAL;
	// Get index of argument by name
	static constexpr size_t Addr_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_VLoadNonTemporal>);
static_assert(std::is_standard_layout_v<IROp_VLoadNonTemporal>);

struct __attribute__((packed)) IROp_ContextClear {
	IROp_Header Header;
	// Non-SSA arguments
	uint32_t Offset;
	uint32_t Size;
	static constexpr IROps OPCODE = OP_CONTEXTCLEAR;
};
static_assert(std::is_trivially_copyable_v<IROp_ContextClear>);
static_assert(std::is_standard_layout_v<IROp_ContextClear>);

struct __attribute__((packed)) IROp_CAS {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Expected;
	OrderedNodeWrapper Desired;
	OrderedNodeWrapper Addr;
	static constexpr IROps OPCODE = OP_CAS;
	// Get index of argument by name
	static constexpr size_t Expected_Index = 0;
	static constexpr size_t Desired_Index = 1;
	static constexpr size_t Addr_Index = 2;
};
static_assert(std::is_trivially_copyable_v<IROp_CAS>);
static_assert(std::is_standard_layout_v<IROp_CAS>);

struct __attribute__((packed)) IROp_CASPair {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper ExpectedLo;
	OrderedNodeWrapper ExpectedHi;
	OrderedNodeWrapper DesiredLo;
	OrderedNodeWrapper DesiredHi;
	OrderedNodeWrapper Addr;
	OrderedNodeWrapper OutLo;
	OrderedNodeWrapper OutHi;
	static constexpr IROps OPCODE = OP_CASPAIR;
	// Get index of argument by name
	static constexpr size_t ExpectedLo_Index = 0;
	static constexpr size_t ExpectedHi_Index = 1;
	static constexpr size_t DesiredLo_Index = 2;
	static constexpr size_t DesiredHi_Index = 3;
	static constexpr size_t Addr_Index = 4;
	static constexpr size_t OutLo_Index = 5;
	static constexpr size_t OutHi_Index = 6;
};
static_assert(std::is_trivially_copyable_v<IROp_CASPair>);
static_assert(std::is_standard_layout_v<IROp_CASPair>);

struct __attribute__((packed)) IROp_AtomicSwap {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Value;
	OrderedNodeWrapper Addr;
	static constexpr IROps OPCODE = OP_ATOMICSWAP;
	// Get index of argument by name
	static constexpr size_t Value_Index = 0;
	static constexpr size_t Addr_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_AtomicSwap>);
static_assert(std::is_standard_layout_v<IROp_AtomicSwap>);

struct __attribute__((packed)) IROp_AtomicFetchAdd {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Value;
	OrderedNodeWrapper Addr;
	static constexpr IROps OPCODE = OP_ATOMICFETCHADD;
	// Get index of argument by name
	static constexpr size_t Value_Index = 0;
	static constexpr size_t Addr_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_AtomicFetchAdd>);
static_assert(std::is_standard_layout_v<IROp_AtomicFetchAdd>);

struct __attribute__((packed)) IROp_AtomicFetchSub {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Value;
	OrderedNodeWrapper Addr;
	static constexpr IROps OPCODE = OP_ATOMICFETCHSUB;
	// Get index of argument by name
	static constexpr size_t Value_Index = 0;
	static constexpr size_t Addr_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_AtomicFetchSub>);
static_assert(std::is_standard_layout_v<IROp_AtomicFetchSub>);

struct __attribute__((packed)) IROp_AtomicFetchAnd {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Value;
	OrderedNodeWrapper Addr;
	static constexpr IROps OPCODE = OP_ATOMICFETCHAND;
	// Get index of argument by name
	static constexpr size_t Value_Index = 0;
	static constexpr size_t Addr_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_AtomicFetchAnd>);
static_assert(std::is_standard_layout_v<IROp_AtomicFetchAnd>);

struct __attribute__((packed)) IROp_AtomicFetchCLR {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Value;
	OrderedNodeWrapper Addr;
	static constexpr IROps OPCODE = OP_ATOMICFETCHCLR;
	// Get index of argument by name
	static constexpr size_t Value_Index = 0;
	static constexpr size_t Addr_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_AtomicFetchCLR>);
static_assert(std::is_standard_layout_v<IROp_AtomicFetchCLR>);

struct __attribute__((packed)) IROp_AtomicFetchOr {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Value;
	OrderedNodeWrapper Addr;
	static constexpr IROps OPCODE = OP_ATOMICFETCHOR;
	// Get index of argument by name
	static constexpr size_t Value_Index = 0;
	static constexpr size_t Addr_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_AtomicFetchOr>);
static_assert(std::is_standard_layout_v<IROp_AtomicFetchOr>);

struct __attribute__((packed)) IROp_AtomicFetchXor {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Value;
	OrderedNodeWrapper Addr;
	static constexpr IROps OPCODE = OP_ATOMICFETCHXOR;
	// Get index of argument by name
	static constexpr size_t Value_Index = 0;
	static constexpr size_t Addr_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_AtomicFetchXor>);
static_assert(std::is_standard_layout_v<IROp_AtomicFetchXor>);

struct __attribute__((packed)) IROp_AtomicFetchNeg {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Addr;
	static constexpr IROps OPCODE = OP_ATOMICFETCHNEG;
	// Get index of argument by name
	static constexpr size_t Addr_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_AtomicFetchNeg>);
static_assert(std::is_standard_layout_v<IROp_AtomicFetchNeg>);

struct __attribute__((packed)) IROp_TelemetrySetValue {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Value;
	// Non-SSA arguments
	uint8_t TelemetryValueIndex;
	static constexpr IROps OPCODE = OP_TELEMETRYSETVALUE;
	// Get index of argument by name
	static constexpr size_t Value_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_TelemetrySetValue>);
static_assert(std::is_standard_layout_v<IROp_TelemetrySetValue>);

struct __attribute__((packed)) IROp_EntrypointOffset {
	IROp_Header Header;
	// Non-SSA arguments
	int64_t Offset;
	static constexpr IROps OPCODE = OP_ENTRYPOINTOFFSET;
};
static_assert(std::is_trivially_copyable_v<IROp_EntrypointOffset>);
static_assert(std::is_standard_layout_v<IROp_EntrypointOffset>);

struct __attribute__((packed)) IROp_InlineEntrypointOffset {
	IROp_Header Header;
	// Non-SSA arguments
	int64_t Offset;
	static constexpr IROps OPCODE = OP_INLINEENTRYPOINTOFFSET;
};
static_assert(std::is_trivially_copyable_v<IROp_InlineEntrypointOffset>);
static_assert(std::is_standard_layout_v<IROp_InlineEntrypointOffset>);

struct __attribute__((packed)) IROp_Constant {
	IROp_Header Header;
	// Non-SSA arguments
	int64_t Constant;
	ConstPad Pad;
	int32_t MaxBytes;
	static constexpr IROps OPCODE = OP_CONSTANT;
};
static_assert(std::is_trivially_copyable_v<IROp_Constant>);
static_assert(std::is_standard_layout_v<IROp_Constant>);

struct __attribute__((packed)) IROp_InlineConstant {
	IROp_Header Header;
	// Non-SSA arguments
	int64_t Constant;
	static constexpr IROps OPCODE = OP_INLINECONSTANT;
};
static_assert(std::is_trivially_copyable_v<IROp_InlineConstant>);
static_assert(std::is_standard_layout_v<IROp_InlineConstant>);

struct __attribute__((packed)) IROp_CycleCounter {
	IROp_Header Header;
	// Non-SSA arguments
	bool SelfSynchronizingLoads;
	static constexpr IROps OPCODE = OP_CYCLECOUNTER;
};
static_assert(std::is_trivially_copyable_v<IROp_CycleCounter>);
static_assert(std::is_standard_layout_v<IROp_CycleCounter>);

struct __attribute__((packed)) IROp_Neg {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Src;
	// Non-SSA arguments
	CondClass Cond;
	static constexpr IROps OPCODE = OP_NEG;
	// Get index of argument by name
	static constexpr size_t Src_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_Neg>);
static_assert(std::is_standard_layout_v<IROp_Neg>);

struct __attribute__((packed)) IROp_Not {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Src;
	static constexpr IROps OPCODE = OP_NOT;
	// Get index of argument by name
	static constexpr size_t Src_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_Not>);
static_assert(std::is_standard_layout_v<IROp_Not>);

struct __attribute__((packed)) IROp_Popcount {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Src;
	static constexpr IROps OPCODE = OP_POPCOUNT;
	// Get index of argument by name
	static constexpr size_t Src_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_Popcount>);
static_assert(std::is_standard_layout_v<IROp_Popcount>);

struct __attribute__((packed)) IROp_FindLSB {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Src;
	static constexpr IROps OPCODE = OP_FINDLSB;
	// Get index of argument by name
	static constexpr size_t Src_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_FindLSB>);
static_assert(std::is_standard_layout_v<IROp_FindLSB>);

struct __attribute__((packed)) IROp_FindMSB {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Src;
	static constexpr IROps OPCODE = OP_FINDMSB;
	// Get index of argument by name
	static constexpr size_t Src_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_FindMSB>);
static_assert(std::is_standard_layout_v<IROp_FindMSB>);

struct __attribute__((packed)) IROp_FindTrailingZeroes {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Src;
	static constexpr IROps OPCODE = OP_FINDTRAILINGZEROES;
	// Get index of argument by name
	static constexpr size_t Src_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_FindTrailingZeroes>);
static_assert(std::is_standard_layout_v<IROp_FindTrailingZeroes>);

struct __attribute__((packed)) IROp_CountLeadingZeroes {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Src;
	static constexpr IROps OPCODE = OP_COUNTLEADINGZEROES;
	// Get index of argument by name
	static constexpr size_t Src_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_CountLeadingZeroes>);
static_assert(std::is_standard_layout_v<IROp_CountLeadingZeroes>);

struct __attribute__((packed)) IROp_Rev {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Src;
	static constexpr IROps OPCODE = OP_REV;
	// Get index of argument by name
	static constexpr size_t Src_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_Rev>);
static_assert(std::is_standard_layout_v<IROp_Rev>);

struct __attribute__((packed)) IROp_Rbit {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Src;
	static constexpr IROps OPCODE = OP_RBIT;
	// Get index of argument by name
	static constexpr size_t Src_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_Rbit>);
static_assert(std::is_standard_layout_v<IROp_Rbit>);

struct __attribute__((packed)) IROp_Add {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Src1;
	OrderedNodeWrapper Src2;
	static constexpr IROps OPCODE = OP_ADD;
	// Get index of argument by name
	static constexpr size_t Src1_Index = 0;
	static constexpr size_t Src2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_Add>);
static_assert(std::is_standard_layout_v<IROp_Add>);

struct __attribute__((packed)) IROp_Adc {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Src1;
	OrderedNodeWrapper Src2;
	static constexpr IROps OPCODE = OP_ADC;
	// Get index of argument by name
	static constexpr size_t Src1_Index = 0;
	static constexpr size_t Src2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_Adc>);
static_assert(std::is_standard_layout_v<IROp_Adc>);

struct __attribute__((packed)) IROp_Sbb {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Src1;
	OrderedNodeWrapper Src2;
	static constexpr IROps OPCODE = OP_SBB;
	// Get index of argument by name
	static constexpr size_t Src1_Index = 0;
	static constexpr size_t Src2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_Sbb>);
static_assert(std::is_standard_layout_v<IROp_Sbb>);

struct __attribute__((packed)) IROp_AddShift {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Src1;
	OrderedNodeWrapper Src2;
	// Non-SSA arguments
	FEXCore::IR::ShiftType Shift;
	uint8_t ShiftAmount;
	static constexpr IROps OPCODE = OP_ADDSHIFT;
	// Get index of argument by name
	static constexpr size_t Src1_Index = 0;
	static constexpr size_t Src2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_AddShift>);
static_assert(std::is_standard_layout_v<IROp_AddShift>);

struct __attribute__((packed)) IROp_AddWithFlags {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Src1;
	OrderedNodeWrapper Src2;
	static constexpr IROps OPCODE = OP_ADDWITHFLAGS;
	// Get index of argument by name
	static constexpr size_t Src1_Index = 0;
	static constexpr size_t Src2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_AddWithFlags>);
static_assert(std::is_standard_layout_v<IROp_AddWithFlags>);

struct __attribute__((packed)) IROp_AddNZCV {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Src1;
	OrderedNodeWrapper Src2;
	static constexpr IROps OPCODE = OP_ADDNZCV;
	// Get index of argument by name
	static constexpr size_t Src1_Index = 0;
	static constexpr size_t Src2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_AddNZCV>);
static_assert(std::is_standard_layout_v<IROp_AddNZCV>);

struct __attribute__((packed)) IROp_SetSmallNZV {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Src;
	static constexpr IROps OPCODE = OP_SETSMALLNZV;
	// Get index of argument by name
	static constexpr size_t Src_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_SetSmallNZV>);
static_assert(std::is_standard_layout_v<IROp_SetSmallNZV>);

struct __attribute__((packed)) IROp_CarryInvert {
	IROp_Header Header;
	static constexpr IROps OPCODE = OP_CARRYINVERT;
};
static_assert(std::is_trivially_copyable_v<IROp_CarryInvert>);
static_assert(std::is_standard_layout_v<IROp_CarryInvert>);

struct __attribute__((packed)) IROp_AXFlag {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper V_inv;
	static constexpr IROps OPCODE = OP_AXFLAG;
	// Get index of argument by name
	static constexpr size_t V_inv_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_AXFlag>);
static_assert(std::is_standard_layout_v<IROp_AXFlag>);

struct __attribute__((packed)) IROp_Parity {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Raw;
	// Non-SSA arguments
	bool Mask;
	bool Invert;
	static constexpr IROps OPCODE = OP_PARITY;
	// Get index of argument by name
	static constexpr size_t Raw_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_Parity>);
static_assert(std::is_standard_layout_v<IROp_Parity>);

struct __attribute__((packed)) IROp_RmifNZCV {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Src;
	// Non-SSA arguments
	uint8_t Rotate;
	uint8_t Mask;
	static constexpr IROps OPCODE = OP_RMIFNZCV;
	// Get index of argument by name
	static constexpr size_t Src_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_RmifNZCV>);
static_assert(std::is_standard_layout_v<IROp_RmifNZCV>);

struct __attribute__((packed)) IROp_CondAddNZCV {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Src1;
	OrderedNodeWrapper Src2;
	// Non-SSA arguments
	CondClass Cond;
	uint8_t FalseNZCV;
	static constexpr IROps OPCODE = OP_CONDADDNZCV;
	// Get index of argument by name
	static constexpr size_t Src1_Index = 0;
	static constexpr size_t Src2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_CondAddNZCV>);
static_assert(std::is_standard_layout_v<IROp_CondAddNZCV>);

struct __attribute__((packed)) IROp_CondSubNZCV {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Src1;
	OrderedNodeWrapper Src2;
	// Non-SSA arguments
	CondClass Cond;
	uint8_t FalseNZCV;
	static constexpr IROps OPCODE = OP_CONDSUBNZCV;
	// Get index of argument by name
	static constexpr size_t Src1_Index = 0;
	static constexpr size_t Src2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_CondSubNZCV>);
static_assert(std::is_standard_layout_v<IROp_CondSubNZCV>);

struct __attribute__((packed)) IROp_AdcWithFlags {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Src1;
	OrderedNodeWrapper Src2;
	static constexpr IROps OPCODE = OP_ADCWITHFLAGS;
	// Get index of argument by name
	static constexpr size_t Src1_Index = 0;
	static constexpr size_t Src2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_AdcWithFlags>);
static_assert(std::is_standard_layout_v<IROp_AdcWithFlags>);

struct __attribute__((packed)) IROp_AdcZero {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Src1;
	static constexpr IROps OPCODE = OP_ADCZERO;
	// Get index of argument by name
	static constexpr size_t Src1_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_AdcZero>);
static_assert(std::is_standard_layout_v<IROp_AdcZero>);

struct __attribute__((packed)) IROp_AdcZeroWithFlags {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Src1;
	static constexpr IROps OPCODE = OP_ADCZEROWITHFLAGS;
	// Get index of argument by name
	static constexpr size_t Src1_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_AdcZeroWithFlags>);
static_assert(std::is_standard_layout_v<IROp_AdcZeroWithFlags>);

struct __attribute__((packed)) IROp_SbbWithFlags {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Src1;
	OrderedNodeWrapper Src2;
	static constexpr IROps OPCODE = OP_SBBWITHFLAGS;
	// Get index of argument by name
	static constexpr size_t Src1_Index = 0;
	static constexpr size_t Src2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_SbbWithFlags>);
static_assert(std::is_standard_layout_v<IROp_SbbWithFlags>);

struct __attribute__((packed)) IROp_AdcNZCV {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Src1;
	OrderedNodeWrapper Src2;
	static constexpr IROps OPCODE = OP_ADCNZCV;
	// Get index of argument by name
	static constexpr size_t Src1_Index = 0;
	static constexpr size_t Src2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_AdcNZCV>);
static_assert(std::is_standard_layout_v<IROp_AdcNZCV>);

struct __attribute__((packed)) IROp_SbbNZCV {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Src1;
	OrderedNodeWrapper Src2;
	static constexpr IROps OPCODE = OP_SBBNZCV;
	// Get index of argument by name
	static constexpr size_t Src1_Index = 0;
	static constexpr size_t Src2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_SbbNZCV>);
static_assert(std::is_standard_layout_v<IROp_SbbNZCV>);

struct __attribute__((packed)) IROp_Sub {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Src1;
	OrderedNodeWrapper Src2;
	static constexpr IROps OPCODE = OP_SUB;
	// Get index of argument by name
	static constexpr size_t Src1_Index = 0;
	static constexpr size_t Src2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_Sub>);
static_assert(std::is_standard_layout_v<IROp_Sub>);

struct __attribute__((packed)) IROp_SubShift {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Src1;
	OrderedNodeWrapper Src2;
	// Non-SSA arguments
	FEXCore::IR::ShiftType Shift;
	uint8_t ShiftAmount;
	static constexpr IROps OPCODE = OP_SUBSHIFT;
	// Get index of argument by name
	static constexpr size_t Src1_Index = 0;
	static constexpr size_t Src2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_SubShift>);
static_assert(std::is_standard_layout_v<IROp_SubShift>);

struct __attribute__((packed)) IROp_SubWithFlags {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Src1;
	OrderedNodeWrapper Src2;
	static constexpr IROps OPCODE = OP_SUBWITHFLAGS;
	// Get index of argument by name
	static constexpr size_t Src1_Index = 0;
	static constexpr size_t Src2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_SubWithFlags>);
static_assert(std::is_standard_layout_v<IROp_SubWithFlags>);

struct __attribute__((packed)) IROp_CmpPairZ {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Src1Lo;
	OrderedNodeWrapper Src1Hi;
	OrderedNodeWrapper Src2Lo;
	OrderedNodeWrapper Src2Hi;
	static constexpr IROps OPCODE = OP_CMPPAIRZ;
	// Get index of argument by name
	static constexpr size_t Src1Lo_Index = 0;
	static constexpr size_t Src1Hi_Index = 1;
	static constexpr size_t Src2Lo_Index = 2;
	static constexpr size_t Src2Hi_Index = 3;
};
static_assert(std::is_trivially_copyable_v<IROp_CmpPairZ>);
static_assert(std::is_standard_layout_v<IROp_CmpPairZ>);

struct __attribute__((packed)) IROp_SubNZCV {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Src1;
	OrderedNodeWrapper Src2;
	static constexpr IROps OPCODE = OP_SUBNZCV;
	// Get index of argument by name
	static constexpr size_t Src1_Index = 0;
	static constexpr size_t Src2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_SubNZCV>);
static_assert(std::is_standard_layout_v<IROp_SubNZCV>);

struct __attribute__((packed)) IROp_Or {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Src1;
	OrderedNodeWrapper Src2;
	static constexpr IROps OPCODE = OP_OR;
	// Get index of argument by name
	static constexpr size_t Src1_Index = 0;
	static constexpr size_t Src2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_Or>);
static_assert(std::is_standard_layout_v<IROp_Or>);

struct __attribute__((packed)) IROp_Orlshl {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Src1;
	OrderedNodeWrapper Src2;
	// Non-SSA arguments
	uint8_t BitShift;
	static constexpr IROps OPCODE = OP_ORLSHL;
	// Get index of argument by name
	static constexpr size_t Src1_Index = 0;
	static constexpr size_t Src2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_Orlshl>);
static_assert(std::is_standard_layout_v<IROp_Orlshl>);

struct __attribute__((packed)) IROp_Orlshr {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Src1;
	OrderedNodeWrapper Src2;
	// Non-SSA arguments
	uint8_t BitShift;
	static constexpr IROps OPCODE = OP_ORLSHR;
	// Get index of argument by name
	static constexpr size_t Src1_Index = 0;
	static constexpr size_t Src2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_Orlshr>);
static_assert(std::is_standard_layout_v<IROp_Orlshr>);

struct __attribute__((packed)) IROp_Ornror {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Src1;
	OrderedNodeWrapper Src2;
	// Non-SSA arguments
	uint8_t BitShift;
	static constexpr IROps OPCODE = OP_ORNROR;
	// Get index of argument by name
	static constexpr size_t Src1_Index = 0;
	static constexpr size_t Src2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_Ornror>);
static_assert(std::is_standard_layout_v<IROp_Ornror>);

struct __attribute__((packed)) IROp_Xor {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Src1;
	OrderedNodeWrapper Src2;
	static constexpr IROps OPCODE = OP_XOR;
	// Get index of argument by name
	static constexpr size_t Src1_Index = 0;
	static constexpr size_t Src2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_Xor>);
static_assert(std::is_standard_layout_v<IROp_Xor>);

struct __attribute__((packed)) IROp_XorShift {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Src1;
	OrderedNodeWrapper Src2;
	// Non-SSA arguments
	FEXCore::IR::ShiftType Shift;
	uint8_t ShiftAmount;
	static constexpr IROps OPCODE = OP_XORSHIFT;
	// Get index of argument by name
	static constexpr size_t Src1_Index = 0;
	static constexpr size_t Src2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_XorShift>);
static_assert(std::is_standard_layout_v<IROp_XorShift>);

struct __attribute__((packed)) IROp_XornShift {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Src1;
	OrderedNodeWrapper Src2;
	// Non-SSA arguments
	FEXCore::IR::ShiftType Shift;
	uint8_t ShiftAmount;
	static constexpr IROps OPCODE = OP_XORNSHIFT;
	// Get index of argument by name
	static constexpr size_t Src1_Index = 0;
	static constexpr size_t Src2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_XornShift>);
static_assert(std::is_standard_layout_v<IROp_XornShift>);

struct __attribute__((packed)) IROp_And {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Src1;
	OrderedNodeWrapper Src2;
	static constexpr IROps OPCODE = OP_AND;
	// Get index of argument by name
	static constexpr size_t Src1_Index = 0;
	static constexpr size_t Src2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_And>);
static_assert(std::is_standard_layout_v<IROp_And>);

struct __attribute__((packed)) IROp_AndShift {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Src1;
	OrderedNodeWrapper Src2;
	// Non-SSA arguments
	FEXCore::IR::ShiftType Shift;
	uint8_t ShiftAmount;
	static constexpr IROps OPCODE = OP_ANDSHIFT;
	// Get index of argument by name
	static constexpr size_t Src1_Index = 0;
	static constexpr size_t Src2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_AndShift>);
static_assert(std::is_standard_layout_v<IROp_AndShift>);

struct __attribute__((packed)) IROp_AndWithFlags {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Src1;
	OrderedNodeWrapper Src2;
	static constexpr IROps OPCODE = OP_ANDWITHFLAGS;
	// Get index of argument by name
	static constexpr size_t Src1_Index = 0;
	static constexpr size_t Src2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_AndWithFlags>);
static_assert(std::is_standard_layout_v<IROp_AndWithFlags>);

struct __attribute__((packed)) IROp_Andn {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Src1;
	OrderedNodeWrapper Src2;
	static constexpr IROps OPCODE = OP_ANDN;
	// Get index of argument by name
	static constexpr size_t Src1_Index = 0;
	static constexpr size_t Src2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_Andn>);
static_assert(std::is_standard_layout_v<IROp_Andn>);

struct __attribute__((packed)) IROp_TestNZ {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Src1;
	OrderedNodeWrapper Src2;
	static constexpr IROps OPCODE = OP_TESTNZ;
	// Get index of argument by name
	static constexpr size_t Src1_Index = 0;
	static constexpr size_t Src2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_TestNZ>);
static_assert(std::is_standard_layout_v<IROp_TestNZ>);

struct __attribute__((packed)) IROp_TestZ {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Src1;
	OrderedNodeWrapper Src2;
	static constexpr IROps OPCODE = OP_TESTZ;
	// Get index of argument by name
	static constexpr size_t Src1_Index = 0;
	static constexpr size_t Src2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_TestZ>);
static_assert(std::is_standard_layout_v<IROp_TestZ>);

struct __attribute__((packed)) IROp_Lshl {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Src1;
	OrderedNodeWrapper Src2;
	static constexpr IROps OPCODE = OP_LSHL;
	// Get index of argument by name
	static constexpr size_t Src1_Index = 0;
	static constexpr size_t Src2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_Lshl>);
static_assert(std::is_standard_layout_v<IROp_Lshl>);

struct __attribute__((packed)) IROp_Lshr {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Src1;
	OrderedNodeWrapper Src2;
	static constexpr IROps OPCODE = OP_LSHR;
	// Get index of argument by name
	static constexpr size_t Src1_Index = 0;
	static constexpr size_t Src2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_Lshr>);
static_assert(std::is_standard_layout_v<IROp_Lshr>);

struct __attribute__((packed)) IROp_Ashr {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Src1;
	OrderedNodeWrapper Src2;
	static constexpr IROps OPCODE = OP_ASHR;
	// Get index of argument by name
	static constexpr size_t Src1_Index = 0;
	static constexpr size_t Src2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_Ashr>);
static_assert(std::is_standard_layout_v<IROp_Ashr>);

struct __attribute__((packed)) IROp_ShiftFlags {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Result;
	OrderedNodeWrapper Src1;
	OrderedNodeWrapper Src2;
	OrderedNodeWrapper PFInput;
	// Non-SSA arguments
	FEXCore::IR::OpSize Size;
	FEXCore::IR::ShiftType Shift;
	bool InvertCF;
	static constexpr IROps OPCODE = OP_SHIFTFLAGS;
	// Get index of argument by name
	static constexpr size_t Result_Index = 0;
	static constexpr size_t Src1_Index = 1;
	static constexpr size_t Src2_Index = 2;
	static constexpr size_t PFInput_Index = 3;
};
static_assert(std::is_trivially_copyable_v<IROp_ShiftFlags>);
static_assert(std::is_standard_layout_v<IROp_ShiftFlags>);

struct __attribute__((packed)) IROp_RotateFlags {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Result;
	OrderedNodeWrapper Shift;
	// Non-SSA arguments
	FEXCore::IR::OpSize Size;
	bool Left;
	static constexpr IROps OPCODE = OP_ROTATEFLAGS;
	// Get index of argument by name
	static constexpr size_t Result_Index = 0;
	static constexpr size_t Shift_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_RotateFlags>);
static_assert(std::is_standard_layout_v<IROp_RotateFlags>);

struct __attribute__((packed)) IROp_Ror {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Src1;
	OrderedNodeWrapper Src2;
	static constexpr IROps OPCODE = OP_ROR;
	// Get index of argument by name
	static constexpr size_t Src1_Index = 0;
	static constexpr size_t Src2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_Ror>);
static_assert(std::is_standard_layout_v<IROp_Ror>);

struct __attribute__((packed)) IROp_Mul {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Src1;
	OrderedNodeWrapper Src2;
	static constexpr IROps OPCODE = OP_MUL;
	// Get index of argument by name
	static constexpr size_t Src1_Index = 0;
	static constexpr size_t Src2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_Mul>);
static_assert(std::is_standard_layout_v<IROp_Mul>);

struct __attribute__((packed)) IROp_UMul {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Src1;
	OrderedNodeWrapper Src2;
	static constexpr IROps OPCODE = OP_UMUL;
	// Get index of argument by name
	static constexpr size_t Src1_Index = 0;
	static constexpr size_t Src2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_UMul>);
static_assert(std::is_standard_layout_v<IROp_UMul>);

struct __attribute__((packed)) IROp_UMull {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Src1;
	OrderedNodeWrapper Src2;
	static constexpr IROps OPCODE = OP_UMULL;
	// Get index of argument by name
	static constexpr size_t Src1_Index = 0;
	static constexpr size_t Src2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_UMull>);
static_assert(std::is_standard_layout_v<IROp_UMull>);

struct __attribute__((packed)) IROp_SMull {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Src1;
	OrderedNodeWrapper Src2;
	static constexpr IROps OPCODE = OP_SMULL;
	// Get index of argument by name
	static constexpr size_t Src1_Index = 0;
	static constexpr size_t Src2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_SMull>);
static_assert(std::is_standard_layout_v<IROp_SMull>);

struct __attribute__((packed)) IROp_MulH {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Src1;
	OrderedNodeWrapper Src2;
	static constexpr IROps OPCODE = OP_MULH;
	// Get index of argument by name
	static constexpr size_t Src1_Index = 0;
	static constexpr size_t Src2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_MulH>);
static_assert(std::is_standard_layout_v<IROp_MulH>);

struct __attribute__((packed)) IROp_UMulH {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Src1;
	OrderedNodeWrapper Src2;
	static constexpr IROps OPCODE = OP_UMULH;
	// Get index of argument by name
	static constexpr size_t Src1_Index = 0;
	static constexpr size_t Src2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_UMulH>);
static_assert(std::is_standard_layout_v<IROp_UMulH>);

struct __attribute__((packed)) IROp_Bfi {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Dest;
	OrderedNodeWrapper Src;
	// Non-SSA arguments
	uint8_t Width;
	uint8_t lsb;
	static constexpr IROps OPCODE = OP_BFI;
	// Get index of argument by name
	static constexpr size_t Dest_Index = 0;
	static constexpr size_t Src_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_Bfi>);
static_assert(std::is_standard_layout_v<IROp_Bfi>);

struct __attribute__((packed)) IROp_Bfxil {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Dest;
	OrderedNodeWrapper Src;
	// Non-SSA arguments
	uint8_t Width;
	uint8_t lsb;
	static constexpr IROps OPCODE = OP_BFXIL;
	// Get index of argument by name
	static constexpr size_t Dest_Index = 0;
	static constexpr size_t Src_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_Bfxil>);
static_assert(std::is_standard_layout_v<IROp_Bfxil>);

struct __attribute__((packed)) IROp_Bfe {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Src;
	// Non-SSA arguments
	uint8_t Width;
	uint8_t lsb;
	static constexpr IROps OPCODE = OP_BFE;
	// Get index of argument by name
	static constexpr size_t Src_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_Bfe>);
static_assert(std::is_standard_layout_v<IROp_Bfe>);

struct __attribute__((packed)) IROp_Sbfe {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Src;
	// Non-SSA arguments
	uint8_t Width;
	uint8_t lsb;
	static constexpr IROps OPCODE = OP_SBFE;
	// Get index of argument by name
	static constexpr size_t Src_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_Sbfe>);
static_assert(std::is_standard_layout_v<IROp_Sbfe>);

struct __attribute__((packed)) IROp_NZCVSelect {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper TrueVal;
	OrderedNodeWrapper FalseVal;
	// Non-SSA arguments
	CondClass Cond;
	static constexpr IROps OPCODE = OP_NZCVSELECT;
	// Get index of argument by name
	static constexpr size_t TrueVal_Index = 0;
	static constexpr size_t FalseVal_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_NZCVSelect>);
static_assert(std::is_standard_layout_v<IROp_NZCVSelect>);

struct __attribute__((packed)) IROp_NZCVSelectV {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper TrueVal;
	OrderedNodeWrapper FalseVal;
	// Non-SSA arguments
	CondClass Cond;
	static constexpr IROps OPCODE = OP_NZCVSELECTV;
	// Get index of argument by name
	static constexpr size_t TrueVal_Index = 0;
	static constexpr size_t FalseVal_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_NZCVSelectV>);
static_assert(std::is_standard_layout_v<IROp_NZCVSelectV>);

struct __attribute__((packed)) IROp_NZCVSelectIncrement {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper TrueVal;
	OrderedNodeWrapper FalseVal;
	// Non-SSA arguments
	CondClass Cond;
	static constexpr IROps OPCODE = OP_NZCVSELECTINCREMENT;
	// Get index of argument by name
	static constexpr size_t TrueVal_Index = 0;
	static constexpr size_t FalseVal_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_NZCVSelectIncrement>);
static_assert(std::is_standard_layout_v<IROp_NZCVSelectIncrement>);

struct __attribute__((packed)) IROp_Select {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Cmp1;
	OrderedNodeWrapper Cmp2;
	OrderedNodeWrapper TrueVal;
	OrderedNodeWrapper FalseVal;
	// Non-SSA arguments
	FEXCore::IR::OpSize CompareSize;
	CondClass Cond;
	static constexpr IROps OPCODE = OP_SELECT;
	// Get index of argument by name
	static constexpr size_t Cmp1_Index = 0;
	static constexpr size_t Cmp2_Index = 1;
	static constexpr size_t TrueVal_Index = 2;
	static constexpr size_t FalseVal_Index = 3;
};
static_assert(std::is_trivially_copyable_v<IROp_Select>);
static_assert(std::is_standard_layout_v<IROp_Select>);

struct __attribute__((packed)) IROp_MaskGenerateFromBitWidth {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper BitWidth;
	static constexpr IROps OPCODE = OP_MASKGENERATEFROMBITWIDTH;
	// Get index of argument by name
	static constexpr size_t BitWidth_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_MaskGenerateFromBitWidth>);
static_assert(std::is_standard_layout_v<IROp_MaskGenerateFromBitWidth>);

struct __attribute__((packed)) IROp_Extr {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Upper;
	OrderedNodeWrapper Lower;
	// Non-SSA arguments
	uint8_t LSB;
	static constexpr IROps OPCODE = OP_EXTR;
	// Get index of argument by name
	static constexpr size_t Upper_Index = 0;
	static constexpr size_t Lower_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_Extr>);
static_assert(std::is_standard_layout_v<IROp_Extr>);

struct __attribute__((packed)) IROp_PDep {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Input;
	OrderedNodeWrapper Mask;
	static constexpr IROps OPCODE = OP_PDEP;
	// Get index of argument by name
	static constexpr size_t Input_Index = 0;
	static constexpr size_t Mask_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_PDep>);
static_assert(std::is_standard_layout_v<IROp_PDep>);

struct __attribute__((packed)) IROp_PExt {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Input;
	OrderedNodeWrapper Mask;
	static constexpr IROps OPCODE = OP_PEXT;
	// Get index of argument by name
	static constexpr size_t Input_Index = 0;
	static constexpr size_t Mask_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_PExt>);
static_assert(std::is_standard_layout_v<IROp_PExt>);

struct __attribute__((packed)) IROp_Div {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Lower;
	OrderedNodeWrapper Upper;
	OrderedNodeWrapper Divisor;
	OrderedNodeWrapper OutQuotient;
	OrderedNodeWrapper OutRemainder;
	static constexpr IROps OPCODE = OP_DIV;
	// Get index of argument by name
	static constexpr size_t Lower_Index = 0;
	static constexpr size_t Upper_Index = 1;
	static constexpr size_t Divisor_Index = 2;
	static constexpr size_t OutQuotient_Index = 3;
	static constexpr size_t OutRemainder_Index = 4;
};
static_assert(std::is_trivially_copyable_v<IROp_Div>);
static_assert(std::is_standard_layout_v<IROp_Div>);

struct __attribute__((packed)) IROp_UDiv {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Lower;
	OrderedNodeWrapper Upper;
	OrderedNodeWrapper Divisor;
	OrderedNodeWrapper OutQuotient;
	OrderedNodeWrapper OutRemainder;
	static constexpr IROps OPCODE = OP_UDIV;
	// Get index of argument by name
	static constexpr size_t Lower_Index = 0;
	static constexpr size_t Upper_Index = 1;
	static constexpr size_t Divisor_Index = 2;
	static constexpr size_t OutQuotient_Index = 3;
	static constexpr size_t OutRemainder_Index = 4;
};
static_assert(std::is_trivially_copyable_v<IROp_UDiv>);
static_assert(std::is_standard_layout_v<IROp_UDiv>);

struct __attribute__((packed)) IROp_VExtractToGPR {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Vector;
	// Non-SSA arguments
	uint8_t Index;
	static constexpr IROps OPCODE = OP_VEXTRACTTOGPR;
	// Get index of argument by name
	static constexpr size_t Vector_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_VExtractToGPR>);
static_assert(std::is_standard_layout_v<IROp_VExtractToGPR>);

struct __attribute__((packed)) IROp_Float_ToGPR_S {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Scalar;
	// Non-SSA arguments
	FEXCore::IR::OpSize SrcElementSize;
	static constexpr IROps OPCODE = OP_FLOAT_TOGPR_S;
	// Get index of argument by name
	static constexpr size_t Scalar_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_Float_ToGPR_S>);
static_assert(std::is_standard_layout_v<IROp_Float_ToGPR_S>);

struct __attribute__((packed)) IROp_Float_ToGPR_ZS {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Scalar;
	// Non-SSA arguments
	FEXCore::IR::OpSize SrcElementSize;
	static constexpr IROps OPCODE = OP_FLOAT_TOGPR_ZS;
	// Get index of argument by name
	static constexpr size_t Scalar_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_Float_ToGPR_ZS>);
static_assert(std::is_standard_layout_v<IROp_Float_ToGPR_ZS>);

struct __attribute__((packed)) IROp_FCmp {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Scalar1;
	OrderedNodeWrapper Scalar2;
	// Non-SSA arguments
	FEXCore::IR::OpSize ElementSize;
	static constexpr IROps OPCODE = OP_FCMP;
	// Get index of argument by name
	static constexpr size_t Scalar1_Index = 0;
	static constexpr size_t Scalar2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_FCmp>);
static_assert(std::is_standard_layout_v<IROp_FCmp>);

struct __attribute__((packed)) IROp_VFAddScalarInsert {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Vector1;
	OrderedNodeWrapper Vector2;
	// Non-SSA arguments
	bool ZeroUpperBits;
	static constexpr IROps OPCODE = OP_VFADDSCALARINSERT;
	// Get index of argument by name
	static constexpr size_t Vector1_Index = 0;
	static constexpr size_t Vector2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_VFAddScalarInsert>);
static_assert(std::is_standard_layout_v<IROp_VFAddScalarInsert>);

struct __attribute__((packed)) IROp_VFSubScalarInsert {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Vector1;
	OrderedNodeWrapper Vector2;
	// Non-SSA arguments
	bool ZeroUpperBits;
	static constexpr IROps OPCODE = OP_VFSUBSCALARINSERT;
	// Get index of argument by name
	static constexpr size_t Vector1_Index = 0;
	static constexpr size_t Vector2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_VFSubScalarInsert>);
static_assert(std::is_standard_layout_v<IROp_VFSubScalarInsert>);

struct __attribute__((packed)) IROp_VFMulScalarInsert {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Vector1;
	OrderedNodeWrapper Vector2;
	// Non-SSA arguments
	bool ZeroUpperBits;
	static constexpr IROps OPCODE = OP_VFMULSCALARINSERT;
	// Get index of argument by name
	static constexpr size_t Vector1_Index = 0;
	static constexpr size_t Vector2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_VFMulScalarInsert>);
static_assert(std::is_standard_layout_v<IROp_VFMulScalarInsert>);

struct __attribute__((packed)) IROp_VFDivScalarInsert {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Vector1;
	OrderedNodeWrapper Vector2;
	// Non-SSA arguments
	bool ZeroUpperBits;
	static constexpr IROps OPCODE = OP_VFDIVSCALARINSERT;
	// Get index of argument by name
	static constexpr size_t Vector1_Index = 0;
	static constexpr size_t Vector2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_VFDivScalarInsert>);
static_assert(std::is_standard_layout_v<IROp_VFDivScalarInsert>);

struct __attribute__((packed)) IROp_VFMinScalarInsert {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Vector1;
	OrderedNodeWrapper Vector2;
	// Non-SSA arguments
	bool ZeroUpperBits;
	static constexpr IROps OPCODE = OP_VFMINSCALARINSERT;
	// Get index of argument by name
	static constexpr size_t Vector1_Index = 0;
	static constexpr size_t Vector2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_VFMinScalarInsert>);
static_assert(std::is_standard_layout_v<IROp_VFMinScalarInsert>);

struct __attribute__((packed)) IROp_VFMaxScalarInsert {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Vector1;
	OrderedNodeWrapper Vector2;
	// Non-SSA arguments
	bool ZeroUpperBits;
	static constexpr IROps OPCODE = OP_VFMAXSCALARINSERT;
	// Get index of argument by name
	static constexpr size_t Vector1_Index = 0;
	static constexpr size_t Vector2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_VFMaxScalarInsert>);
static_assert(std::is_standard_layout_v<IROp_VFMaxScalarInsert>);

struct __attribute__((packed)) IROp_VFSqrtScalarInsert {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Vector1;
	OrderedNodeWrapper Vector2;
	// Non-SSA arguments
	bool ZeroUpperBits;
	static constexpr IROps OPCODE = OP_VFSQRTSCALARINSERT;
	// Get index of argument by name
	static constexpr size_t Vector1_Index = 0;
	static constexpr size_t Vector2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_VFSqrtScalarInsert>);
static_assert(std::is_standard_layout_v<IROp_VFSqrtScalarInsert>);

struct __attribute__((packed)) IROp_VFRSqrtScalarInsert {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Vector1;
	OrderedNodeWrapper Vector2;
	// Non-SSA arguments
	bool ZeroUpperBits;
	static constexpr IROps OPCODE = OP_VFRSQRTSCALARINSERT;
	// Get index of argument by name
	static constexpr size_t Vector1_Index = 0;
	static constexpr size_t Vector2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_VFRSqrtScalarInsert>);
static_assert(std::is_standard_layout_v<IROp_VFRSqrtScalarInsert>);

struct __attribute__((packed)) IROp_VFRecpScalarInsert {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Vector1;
	OrderedNodeWrapper Vector2;
	// Non-SSA arguments
	bool ZeroUpperBits;
	static constexpr IROps OPCODE = OP_VFRECPSCALARINSERT;
	// Get index of argument by name
	static constexpr size_t Vector1_Index = 0;
	static constexpr size_t Vector2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_VFRecpScalarInsert>);
static_assert(std::is_standard_layout_v<IROp_VFRecpScalarInsert>);

struct __attribute__((packed)) IROp_VFToFScalarInsert {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Vector1;
	OrderedNodeWrapper Vector2;
	// Non-SSA arguments
	FEXCore::IR::OpSize SrcElementSize;
	bool ZeroUpperBits;
	static constexpr IROps OPCODE = OP_VFTOFSCALARINSERT;
	// Get index of argument by name
	static constexpr size_t Vector1_Index = 0;
	static constexpr size_t Vector2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_VFToFScalarInsert>);
static_assert(std::is_standard_layout_v<IROp_VFToFScalarInsert>);

struct __attribute__((packed)) IROp_VSToFVectorInsert {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Vector1;
	OrderedNodeWrapper Vector2;
	// Non-SSA arguments
	FEXCore::IR::OpSize SrcElementSize;
	int8_t HasTwoElements;
	bool ZeroUpperBits;
	static constexpr IROps OPCODE = OP_VSTOFVECTORINSERT;
	// Get index of argument by name
	static constexpr size_t Vector1_Index = 0;
	static constexpr size_t Vector2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_VSToFVectorInsert>);
static_assert(std::is_standard_layout_v<IROp_VSToFVectorInsert>);

struct __attribute__((packed)) IROp_VSToFGPRInsert {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Vector;
	OrderedNodeWrapper Src;
	// Non-SSA arguments
	FEXCore::IR::OpSize SrcElementSize;
	bool ZeroUpperBits;
	static constexpr IROps OPCODE = OP_VSTOFGPRINSERT;
	// Get index of argument by name
	static constexpr size_t Vector_Index = 0;
	static constexpr size_t Src_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_VSToFGPRInsert>);
static_assert(std::is_standard_layout_v<IROp_VSToFGPRInsert>);

struct __attribute__((packed)) IROp_VFToIScalarInsert {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Vector1;
	OrderedNodeWrapper Vector2;
	// Non-SSA arguments
	RoundMode Round;
	bool ZeroUpperBits;
	static constexpr IROps OPCODE = OP_VFTOISCALARINSERT;
	// Get index of argument by name
	static constexpr size_t Vector1_Index = 0;
	static constexpr size_t Vector2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_VFToIScalarInsert>);
static_assert(std::is_standard_layout_v<IROp_VFToIScalarInsert>);

struct __attribute__((packed)) IROp_VFCMPScalarInsert {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Vector1;
	OrderedNodeWrapper Vector2;
	// Non-SSA arguments
	FloatCompareOp Op;
	bool ZeroUpperBits;
	static constexpr IROps OPCODE = OP_VFCMPSCALARINSERT;
	// Get index of argument by name
	static constexpr size_t Vector1_Index = 0;
	static constexpr size_t Vector2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_VFCMPScalarInsert>);
static_assert(std::is_standard_layout_v<IROp_VFCMPScalarInsert>);

struct __attribute__((packed)) IROp_VFMLAScalarInsert {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Upper;
	OrderedNodeWrapper Vector1;
	OrderedNodeWrapper Vector2;
	OrderedNodeWrapper Addend;
	static constexpr IROps OPCODE = OP_VFMLASCALARINSERT;
	// Get index of argument by name
	static constexpr size_t Upper_Index = 0;
	static constexpr size_t Vector1_Index = 1;
	static constexpr size_t Vector2_Index = 2;
	static constexpr size_t Addend_Index = 3;
};
static_assert(std::is_trivially_copyable_v<IROp_VFMLAScalarInsert>);
static_assert(std::is_standard_layout_v<IROp_VFMLAScalarInsert>);

struct __attribute__((packed)) IROp_VFMLSScalarInsert {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Upper;
	OrderedNodeWrapper Vector1;
	OrderedNodeWrapper Vector2;
	OrderedNodeWrapper Addend;
	static constexpr IROps OPCODE = OP_VFMLSSCALARINSERT;
	// Get index of argument by name
	static constexpr size_t Upper_Index = 0;
	static constexpr size_t Vector1_Index = 1;
	static constexpr size_t Vector2_Index = 2;
	static constexpr size_t Addend_Index = 3;
};
static_assert(std::is_trivially_copyable_v<IROp_VFMLSScalarInsert>);
static_assert(std::is_standard_layout_v<IROp_VFMLSScalarInsert>);

struct __attribute__((packed)) IROp_VFNMLAScalarInsert {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Upper;
	OrderedNodeWrapper Vector1;
	OrderedNodeWrapper Vector2;
	OrderedNodeWrapper Addend;
	static constexpr IROps OPCODE = OP_VFNMLASCALARINSERT;
	// Get index of argument by name
	static constexpr size_t Upper_Index = 0;
	static constexpr size_t Vector1_Index = 1;
	static constexpr size_t Vector2_Index = 2;
	static constexpr size_t Addend_Index = 3;
};
static_assert(std::is_trivially_copyable_v<IROp_VFNMLAScalarInsert>);
static_assert(std::is_standard_layout_v<IROp_VFNMLAScalarInsert>);

struct __attribute__((packed)) IROp_VFNMLSScalarInsert {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Upper;
	OrderedNodeWrapper Vector1;
	OrderedNodeWrapper Vector2;
	OrderedNodeWrapper Addend;
	static constexpr IROps OPCODE = OP_VFNMLSSCALARINSERT;
	// Get index of argument by name
	static constexpr size_t Upper_Index = 0;
	static constexpr size_t Vector1_Index = 1;
	static constexpr size_t Vector2_Index = 2;
	static constexpr size_t Addend_Index = 3;
};
static_assert(std::is_trivially_copyable_v<IROp_VFNMLSScalarInsert>);
static_assert(std::is_standard_layout_v<IROp_VFNMLSScalarInsert>);

struct __attribute__((packed)) IROp_VFCopySign {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Vector1;
	OrderedNodeWrapper Vector2;
	static constexpr IROps OPCODE = OP_VFCOPYSIGN;
	// Get index of argument by name
	static constexpr size_t Vector1_Index = 0;
	static constexpr size_t Vector2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_VFCopySign>);
static_assert(std::is_standard_layout_v<IROp_VFCopySign>);

struct __attribute__((packed)) IROp_VMov {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Source;
	static constexpr IROps OPCODE = OP_VMOV;
	// Get index of argument by name
	static constexpr size_t Source_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_VMov>);
static_assert(std::is_standard_layout_v<IROp_VMov>);

struct __attribute__((packed)) IROp_VectorImm {
	IROp_Header Header;
	// Non-SSA arguments
	uint8_t Immediate;
	uint8_t ShiftAmount;
	static constexpr IROps OPCODE = OP_VECTORIMM;
};
static_assert(std::is_trivially_copyable_v<IROp_VectorImm>);
static_assert(std::is_standard_layout_v<IROp_VectorImm>);

struct __attribute__((packed)) IROp_LoadNamedVectorConstant {
	IROp_Header Header;
	// Non-SSA arguments
	FEXCore::IR::NamedVectorConstant Constant;
	static constexpr IROps OPCODE = OP_LOADNAMEDVECTORCONSTANT;
};
static_assert(std::is_trivially_copyable_v<IROp_LoadNamedVectorConstant>);
static_assert(std::is_standard_layout_v<IROp_LoadNamedVectorConstant>);

struct __attribute__((packed)) IROp_LoadNamedVectorIndexedConstant {
	IROp_Header Header;
	// Non-SSA arguments
	FEXCore::IR::IndexNamedVectorConstant Constant;
	uint32_t Index;
	static constexpr IROps OPCODE = OP_LOADNAMEDVECTORINDEXEDCONSTANT;
};
static_assert(std::is_trivially_copyable_v<IROp_LoadNamedVectorIndexedConstant>);
static_assert(std::is_standard_layout_v<IROp_LoadNamedVectorIndexedConstant>);

struct __attribute__((packed)) IROp_VNeg {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Vector;
	static constexpr IROps OPCODE = OP_VNEG;
	// Get index of argument by name
	static constexpr size_t Vector_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_VNeg>);
static_assert(std::is_standard_layout_v<IROp_VNeg>);

struct __attribute__((packed)) IROp_VNot {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Vector;
	static constexpr IROps OPCODE = OP_VNOT;
	// Get index of argument by name
	static constexpr size_t Vector_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_VNot>);
static_assert(std::is_standard_layout_v<IROp_VNot>);

struct __attribute__((packed)) IROp_VAbs {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Vector;
	static constexpr IROps OPCODE = OP_VABS;
	// Get index of argument by name
	static constexpr size_t Vector_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_VAbs>);
static_assert(std::is_standard_layout_v<IROp_VAbs>);

struct __attribute__((packed)) IROp_VPopcount {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Vector;
	static constexpr IROps OPCODE = OP_VPOPCOUNT;
	// Get index of argument by name
	static constexpr size_t Vector_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_VPopcount>);
static_assert(std::is_standard_layout_v<IROp_VPopcount>);

struct __attribute__((packed)) IROp_VAddV {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Vector;
	static constexpr IROps OPCODE = OP_VADDV;
	// Get index of argument by name
	static constexpr size_t Vector_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_VAddV>);
static_assert(std::is_standard_layout_v<IROp_VAddV>);

struct __attribute__((packed)) IROp_VUMinV {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Vector;
	static constexpr IROps OPCODE = OP_VUMINV;
	// Get index of argument by name
	static constexpr size_t Vector_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_VUMinV>);
static_assert(std::is_standard_layout_v<IROp_VUMinV>);

struct __attribute__((packed)) IROp_VUMaxV {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Vector;
	static constexpr IROps OPCODE = OP_VUMAXV;
	// Get index of argument by name
	static constexpr size_t Vector_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_VUMaxV>);
static_assert(std::is_standard_layout_v<IROp_VUMaxV>);

struct __attribute__((packed)) IROp_VFAbs {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Vector;
	static constexpr IROps OPCODE = OP_VFABS;
	// Get index of argument by name
	static constexpr size_t Vector_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_VFAbs>);
static_assert(std::is_standard_layout_v<IROp_VFAbs>);

struct __attribute__((packed)) IROp_VFNeg {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Vector;
	static constexpr IROps OPCODE = OP_VFNEG;
	// Get index of argument by name
	static constexpr size_t Vector_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_VFNeg>);
static_assert(std::is_standard_layout_v<IROp_VFNeg>);

struct __attribute__((packed)) IROp_VFRecp {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Vector;
	static constexpr IROps OPCODE = OP_VFRECP;
	// Get index of argument by name
	static constexpr size_t Vector_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_VFRecp>);
static_assert(std::is_standard_layout_v<IROp_VFRecp>);

struct __attribute__((packed)) IROp_VFRecpPrecision {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Vector;
	static constexpr IROps OPCODE = OP_VFRECPPRECISION;
	// Get index of argument by name
	static constexpr size_t Vector_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_VFRecpPrecision>);
static_assert(std::is_standard_layout_v<IROp_VFRecpPrecision>);

struct __attribute__((packed)) IROp_VFSqrt {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Vector;
	static constexpr IROps OPCODE = OP_VFSQRT;
	// Get index of argument by name
	static constexpr size_t Vector_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_VFSqrt>);
static_assert(std::is_standard_layout_v<IROp_VFSqrt>);

struct __attribute__((packed)) IROp_VFRSqrt {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Vector;
	static constexpr IROps OPCODE = OP_VFRSQRT;
	// Get index of argument by name
	static constexpr size_t Vector_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_VFRSqrt>);
static_assert(std::is_standard_layout_v<IROp_VFRSqrt>);

struct __attribute__((packed)) IROp_VFRSqrtPrecision {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Vector;
	static constexpr IROps OPCODE = OP_VFRSQRTPRECISION;
	// Get index of argument by name
	static constexpr size_t Vector_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_VFRSqrtPrecision>);
static_assert(std::is_standard_layout_v<IROp_VFRSqrtPrecision>);

struct __attribute__((packed)) IROp_VCMPEQZ {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Vector;
	static constexpr IROps OPCODE = OP_VCMPEQZ;
	// Get index of argument by name
	static constexpr size_t Vector_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_VCMPEQZ>);
static_assert(std::is_standard_layout_v<IROp_VCMPEQZ>);

struct __attribute__((packed)) IROp_VCMPGTZ {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Vector;
	static constexpr IROps OPCODE = OP_VCMPGTZ;
	// Get index of argument by name
	static constexpr size_t Vector_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_VCMPGTZ>);
static_assert(std::is_standard_layout_v<IROp_VCMPGTZ>);

struct __attribute__((packed)) IROp_VCMPLTZ {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Vector;
	static constexpr IROps OPCODE = OP_VCMPLTZ;
	// Get index of argument by name
	static constexpr size_t Vector_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_VCMPLTZ>);
static_assert(std::is_standard_layout_v<IROp_VCMPLTZ>);

struct __attribute__((packed)) IROp_VDupElement {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Vector;
	// Non-SSA arguments
	uint8_t Index;
	static constexpr IROps OPCODE = OP_VDUPELEMENT;
	// Get index of argument by name
	static constexpr size_t Vector_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_VDupElement>);
static_assert(std::is_standard_layout_v<IROp_VDupElement>);

struct __attribute__((packed)) IROp_VShlI {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Vector;
	// Non-SSA arguments
	uint8_t BitShift;
	static constexpr IROps OPCODE = OP_VSHLI;
	// Get index of argument by name
	static constexpr size_t Vector_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_VShlI>);
static_assert(std::is_standard_layout_v<IROp_VShlI>);

struct __attribute__((packed)) IROp_VUShrI {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Vector;
	// Non-SSA arguments
	uint8_t BitShift;
	static constexpr IROps OPCODE = OP_VUSHRI;
	// Get index of argument by name
	static constexpr size_t Vector_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_VUShrI>);
static_assert(std::is_standard_layout_v<IROp_VUShrI>);

struct __attribute__((packed)) IROp_VUShraI {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper DestVector;
	OrderedNodeWrapper Vector;
	// Non-SSA arguments
	uint8_t BitShift;
	static constexpr IROps OPCODE = OP_VUSHRAI;
	// Get index of argument by name
	static constexpr size_t DestVector_Index = 0;
	static constexpr size_t Vector_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_VUShraI>);
static_assert(std::is_standard_layout_v<IROp_VUShraI>);

struct __attribute__((packed)) IROp_VSShrI {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Vector;
	// Non-SSA arguments
	uint8_t BitShift;
	static constexpr IROps OPCODE = OP_VSSHRI;
	// Get index of argument by name
	static constexpr size_t Vector_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_VSShrI>);
static_assert(std::is_standard_layout_v<IROp_VSShrI>);

struct __attribute__((packed)) IROp_VUShrNI {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Vector;
	// Non-SSA arguments
	uint8_t BitShift;
	static constexpr IROps OPCODE = OP_VUSHRNI;
	// Get index of argument by name
	static constexpr size_t Vector_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_VUShrNI>);
static_assert(std::is_standard_layout_v<IROp_VUShrNI>);

struct __attribute__((packed)) IROp_VUShrNI2 {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper VectorLower;
	OrderedNodeWrapper VectorUpper;
	// Non-SSA arguments
	uint8_t BitShift;
	static constexpr IROps OPCODE = OP_VUSHRNI2;
	// Get index of argument by name
	static constexpr size_t VectorLower_Index = 0;
	static constexpr size_t VectorUpper_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_VUShrNI2>);
static_assert(std::is_standard_layout_v<IROp_VUShrNI2>);

struct __attribute__((packed)) IROp_VSXTL {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Vector;
	static constexpr IROps OPCODE = OP_VSXTL;
	// Get index of argument by name
	static constexpr size_t Vector_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_VSXTL>);
static_assert(std::is_standard_layout_v<IROp_VSXTL>);

struct __attribute__((packed)) IROp_VSXTL2 {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Vector;
	static constexpr IROps OPCODE = OP_VSXTL2;
	// Get index of argument by name
	static constexpr size_t Vector_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_VSXTL2>);
static_assert(std::is_standard_layout_v<IROp_VSXTL2>);

struct __attribute__((packed)) IROp_VSSHLL {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Vector;
	// Non-SSA arguments
	uint8_t BitShift;
	static constexpr IROps OPCODE = OP_VSSHLL;
	// Get index of argument by name
	static constexpr size_t Vector_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_VSSHLL>);
static_assert(std::is_standard_layout_v<IROp_VSSHLL>);

struct __attribute__((packed)) IROp_VSSHLL2 {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Vector;
	// Non-SSA arguments
	uint8_t BitShift;
	static constexpr IROps OPCODE = OP_VSSHLL2;
	// Get index of argument by name
	static constexpr size_t Vector_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_VSSHLL2>);
static_assert(std::is_standard_layout_v<IROp_VSSHLL2>);

struct __attribute__((packed)) IROp_VUXTL {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Vector;
	static constexpr IROps OPCODE = OP_VUXTL;
	// Get index of argument by name
	static constexpr size_t Vector_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_VUXTL>);
static_assert(std::is_standard_layout_v<IROp_VUXTL>);

struct __attribute__((packed)) IROp_VUXTL2 {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Vector;
	static constexpr IROps OPCODE = OP_VUXTL2;
	// Get index of argument by name
	static constexpr size_t Vector_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_VUXTL2>);
static_assert(std::is_standard_layout_v<IROp_VUXTL2>);

struct __attribute__((packed)) IROp_VSQXTN {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Vector;
	static constexpr IROps OPCODE = OP_VSQXTN;
	// Get index of argument by name
	static constexpr size_t Vector_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_VSQXTN>);
static_assert(std::is_standard_layout_v<IROp_VSQXTN>);

struct __attribute__((packed)) IROp_VSQXTN2 {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper VectorLower;
	OrderedNodeWrapper VectorUpper;
	static constexpr IROps OPCODE = OP_VSQXTN2;
	// Get index of argument by name
	static constexpr size_t VectorLower_Index = 0;
	static constexpr size_t VectorUpper_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_VSQXTN2>);
static_assert(std::is_standard_layout_v<IROp_VSQXTN2>);

struct __attribute__((packed)) IROp_VSQXTNPair {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper VectorLower;
	OrderedNodeWrapper VectorUpper;
	static constexpr IROps OPCODE = OP_VSQXTNPAIR;
	// Get index of argument by name
	static constexpr size_t VectorLower_Index = 0;
	static constexpr size_t VectorUpper_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_VSQXTNPair>);
static_assert(std::is_standard_layout_v<IROp_VSQXTNPair>);

struct __attribute__((packed)) IROp_VSQXTUN {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Vector;
	static constexpr IROps OPCODE = OP_VSQXTUN;
	// Get index of argument by name
	static constexpr size_t Vector_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_VSQXTUN>);
static_assert(std::is_standard_layout_v<IROp_VSQXTUN>);

struct __attribute__((packed)) IROp_VSQXTUN2 {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper VectorLower;
	OrderedNodeWrapper VectorUpper;
	static constexpr IROps OPCODE = OP_VSQXTUN2;
	// Get index of argument by name
	static constexpr size_t VectorLower_Index = 0;
	static constexpr size_t VectorUpper_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_VSQXTUN2>);
static_assert(std::is_standard_layout_v<IROp_VSQXTUN2>);

struct __attribute__((packed)) IROp_VSQXTUNPair {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper VectorLower;
	OrderedNodeWrapper VectorUpper;
	static constexpr IROps OPCODE = OP_VSQXTUNPAIR;
	// Get index of argument by name
	static constexpr size_t VectorLower_Index = 0;
	static constexpr size_t VectorUpper_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_VSQXTUNPair>);
static_assert(std::is_standard_layout_v<IROp_VSQXTUNPair>);

struct __attribute__((packed)) IROp_VSRSHR {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Vector;
	// Non-SSA arguments
	uint8_t BitShift;
	static constexpr IROps OPCODE = OP_VSRSHR;
	// Get index of argument by name
	static constexpr size_t Vector_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_VSRSHR>);
static_assert(std::is_standard_layout_v<IROp_VSRSHR>);

struct __attribute__((packed)) IROp_VSQSHL {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Vector;
	// Non-SSA arguments
	uint8_t BitShift;
	static constexpr IROps OPCODE = OP_VSQSHL;
	// Get index of argument by name
	static constexpr size_t Vector_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_VSQSHL>);
static_assert(std::is_standard_layout_v<IROp_VSQSHL>);

struct __attribute__((packed)) IROp_VRev32 {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Vector;
	static constexpr IROps OPCODE = OP_VREV32;
	// Get index of argument by name
	static constexpr size_t Vector_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_VRev32>);
static_assert(std::is_standard_layout_v<IROp_VRev32>);

struct __attribute__((packed)) IROp_VRev64 {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Vector;
	static constexpr IROps OPCODE = OP_VREV64;
	// Get index of argument by name
	static constexpr size_t Vector_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_VRev64>);
static_assert(std::is_standard_layout_v<IROp_VRev64>);

struct __attribute__((packed)) IROp_VAdd {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Vector1;
	OrderedNodeWrapper Vector2;
	static constexpr IROps OPCODE = OP_VADD;
	// Get index of argument by name
	static constexpr size_t Vector1_Index = 0;
	static constexpr size_t Vector2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_VAdd>);
static_assert(std::is_standard_layout_v<IROp_VAdd>);

struct __attribute__((packed)) IROp_VSub {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Vector1;
	OrderedNodeWrapper Vector2;
	static constexpr IROps OPCODE = OP_VSUB;
	// Get index of argument by name
	static constexpr size_t Vector1_Index = 0;
	static constexpr size_t Vector2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_VSub>);
static_assert(std::is_standard_layout_v<IROp_VSub>);

struct __attribute__((packed)) IROp_VAnd {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Vector1;
	OrderedNodeWrapper Vector2;
	static constexpr IROps OPCODE = OP_VAND;
	// Get index of argument by name
	static constexpr size_t Vector1_Index = 0;
	static constexpr size_t Vector2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_VAnd>);
static_assert(std::is_standard_layout_v<IROp_VAnd>);

struct __attribute__((packed)) IROp_VAndn {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Vector1;
	OrderedNodeWrapper Vector2;
	static constexpr IROps OPCODE = OP_VANDN;
	// Get index of argument by name
	static constexpr size_t Vector1_Index = 0;
	static constexpr size_t Vector2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_VAndn>);
static_assert(std::is_standard_layout_v<IROp_VAndn>);

struct __attribute__((packed)) IROp_VOrn {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Vector1;
	OrderedNodeWrapper Vector2;
	static constexpr IROps OPCODE = OP_VORN;
	// Get index of argument by name
	static constexpr size_t Vector1_Index = 0;
	static constexpr size_t Vector2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_VOrn>);
static_assert(std::is_standard_layout_v<IROp_VOrn>);

struct __attribute__((packed)) IROp_VOr {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Vector1;
	OrderedNodeWrapper Vector2;
	static constexpr IROps OPCODE = OP_VOR;
	// Get index of argument by name
	static constexpr size_t Vector1_Index = 0;
	static constexpr size_t Vector2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_VOr>);
static_assert(std::is_standard_layout_v<IROp_VOr>);

struct __attribute__((packed)) IROp_VXor {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Vector1;
	OrderedNodeWrapper Vector2;
	static constexpr IROps OPCODE = OP_VXOR;
	// Get index of argument by name
	static constexpr size_t Vector1_Index = 0;
	static constexpr size_t Vector2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_VXor>);
static_assert(std::is_standard_layout_v<IROp_VXor>);

struct __attribute__((packed)) IROp_VUQAdd {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Vector1;
	OrderedNodeWrapper Vector2;
	static constexpr IROps OPCODE = OP_VUQADD;
	// Get index of argument by name
	static constexpr size_t Vector1_Index = 0;
	static constexpr size_t Vector2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_VUQAdd>);
static_assert(std::is_standard_layout_v<IROp_VUQAdd>);

struct __attribute__((packed)) IROp_VUQSub {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Vector1;
	OrderedNodeWrapper Vector2;
	static constexpr IROps OPCODE = OP_VUQSUB;
	// Get index of argument by name
	static constexpr size_t Vector1_Index = 0;
	static constexpr size_t Vector2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_VUQSub>);
static_assert(std::is_standard_layout_v<IROp_VUQSub>);

struct __attribute__((packed)) IROp_VSQAdd {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Vector1;
	OrderedNodeWrapper Vector2;
	static constexpr IROps OPCODE = OP_VSQADD;
	// Get index of argument by name
	static constexpr size_t Vector1_Index = 0;
	static constexpr size_t Vector2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_VSQAdd>);
static_assert(std::is_standard_layout_v<IROp_VSQAdd>);

struct __attribute__((packed)) IROp_VSQSub {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Vector1;
	OrderedNodeWrapper Vector2;
	static constexpr IROps OPCODE = OP_VSQSUB;
	// Get index of argument by name
	static constexpr size_t Vector1_Index = 0;
	static constexpr size_t Vector2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_VSQSub>);
static_assert(std::is_standard_layout_v<IROp_VSQSub>);

struct __attribute__((packed)) IROp_VAddP {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper VectorLower;
	OrderedNodeWrapper VectorUpper;
	static constexpr IROps OPCODE = OP_VADDP;
	// Get index of argument by name
	static constexpr size_t VectorLower_Index = 0;
	static constexpr size_t VectorUpper_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_VAddP>);
static_assert(std::is_standard_layout_v<IROp_VAddP>);

struct __attribute__((packed)) IROp_VURAvg {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Vector1;
	OrderedNodeWrapper Vector2;
	static constexpr IROps OPCODE = OP_VURAVG;
	// Get index of argument by name
	static constexpr size_t Vector1_Index = 0;
	static constexpr size_t Vector2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_VURAvg>);
static_assert(std::is_standard_layout_v<IROp_VURAvg>);

struct __attribute__((packed)) IROp_VUMin {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Vector1;
	OrderedNodeWrapper Vector2;
	static constexpr IROps OPCODE = OP_VUMIN;
	// Get index of argument by name
	static constexpr size_t Vector1_Index = 0;
	static constexpr size_t Vector2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_VUMin>);
static_assert(std::is_standard_layout_v<IROp_VUMin>);

struct __attribute__((packed)) IROp_VUMax {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Vector1;
	OrderedNodeWrapper Vector2;
	static constexpr IROps OPCODE = OP_VUMAX;
	// Get index of argument by name
	static constexpr size_t Vector1_Index = 0;
	static constexpr size_t Vector2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_VUMax>);
static_assert(std::is_standard_layout_v<IROp_VUMax>);

struct __attribute__((packed)) IROp_VSMin {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Vector1;
	OrderedNodeWrapper Vector2;
	static constexpr IROps OPCODE = OP_VSMIN;
	// Get index of argument by name
	static constexpr size_t Vector1_Index = 0;
	static constexpr size_t Vector2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_VSMin>);
static_assert(std::is_standard_layout_v<IROp_VSMin>);

struct __attribute__((packed)) IROp_VSMax {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Vector1;
	OrderedNodeWrapper Vector2;
	static constexpr IROps OPCODE = OP_VSMAX;
	// Get index of argument by name
	static constexpr size_t Vector1_Index = 0;
	static constexpr size_t Vector2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_VSMax>);
static_assert(std::is_standard_layout_v<IROp_VSMax>);

struct __attribute__((packed)) IROp_VZip {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper VectorLower;
	OrderedNodeWrapper VectorUpper;
	static constexpr IROps OPCODE = OP_VZIP;
	// Get index of argument by name
	static constexpr size_t VectorLower_Index = 0;
	static constexpr size_t VectorUpper_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_VZip>);
static_assert(std::is_standard_layout_v<IROp_VZip>);

struct __attribute__((packed)) IROp_VZip2 {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper VectorLower;
	OrderedNodeWrapper VectorUpper;
	static constexpr IROps OPCODE = OP_VZIP2;
	// Get index of argument by name
	static constexpr size_t VectorLower_Index = 0;
	static constexpr size_t VectorUpper_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_VZip2>);
static_assert(std::is_standard_layout_v<IROp_VZip2>);

struct __attribute__((packed)) IROp_VUnZip {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper VectorLower;
	OrderedNodeWrapper VectorUpper;
	static constexpr IROps OPCODE = OP_VUNZIP;
	// Get index of argument by name
	static constexpr size_t VectorLower_Index = 0;
	static constexpr size_t VectorUpper_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_VUnZip>);
static_assert(std::is_standard_layout_v<IROp_VUnZip>);

struct __attribute__((packed)) IROp_VUnZip2 {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper VectorLower;
	OrderedNodeWrapper VectorUpper;
	static constexpr IROps OPCODE = OP_VUNZIP2;
	// Get index of argument by name
	static constexpr size_t VectorLower_Index = 0;
	static constexpr size_t VectorUpper_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_VUnZip2>);
static_assert(std::is_standard_layout_v<IROp_VUnZip2>);

struct __attribute__((packed)) IROp_VTrn {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper VectorLower;
	OrderedNodeWrapper VectorUpper;
	static constexpr IROps OPCODE = OP_VTRN;
	// Get index of argument by name
	static constexpr size_t VectorLower_Index = 0;
	static constexpr size_t VectorUpper_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_VTrn>);
static_assert(std::is_standard_layout_v<IROp_VTrn>);

struct __attribute__((packed)) IROp_VTrn2 {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper VectorLower;
	OrderedNodeWrapper VectorUpper;
	static constexpr IROps OPCODE = OP_VTRN2;
	// Get index of argument by name
	static constexpr size_t VectorLower_Index = 0;
	static constexpr size_t VectorUpper_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_VTrn2>);
static_assert(std::is_standard_layout_v<IROp_VTrn2>);

struct __attribute__((packed)) IROp_VFAdd {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Vector1;
	OrderedNodeWrapper Vector2;
	static constexpr IROps OPCODE = OP_VFADD;
	// Get index of argument by name
	static constexpr size_t Vector1_Index = 0;
	static constexpr size_t Vector2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_VFAdd>);
static_assert(std::is_standard_layout_v<IROp_VFAdd>);

struct __attribute__((packed)) IROp_VFAddP {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper VectorLower;
	OrderedNodeWrapper VectorUpper;
	static constexpr IROps OPCODE = OP_VFADDP;
	// Get index of argument by name
	static constexpr size_t VectorLower_Index = 0;
	static constexpr size_t VectorUpper_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_VFAddP>);
static_assert(std::is_standard_layout_v<IROp_VFAddP>);

struct __attribute__((packed)) IROp_VFAddV {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Vector;
	static constexpr IROps OPCODE = OP_VFADDV;
	// Get index of argument by name
	static constexpr size_t Vector_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_VFAddV>);
static_assert(std::is_standard_layout_v<IROp_VFAddV>);

struct __attribute__((packed)) IROp_VFSub {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Vector1;
	OrderedNodeWrapper Vector2;
	static constexpr IROps OPCODE = OP_VFSUB;
	// Get index of argument by name
	static constexpr size_t Vector1_Index = 0;
	static constexpr size_t Vector2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_VFSub>);
static_assert(std::is_standard_layout_v<IROp_VFSub>);

struct __attribute__((packed)) IROp_VFMul {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Vector1;
	OrderedNodeWrapper Vector2;
	static constexpr IROps OPCODE = OP_VFMUL;
	// Get index of argument by name
	static constexpr size_t Vector1_Index = 0;
	static constexpr size_t Vector2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_VFMul>);
static_assert(std::is_standard_layout_v<IROp_VFMul>);

struct __attribute__((packed)) IROp_VFDiv {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Vector1;
	OrderedNodeWrapper Vector2;
	static constexpr IROps OPCODE = OP_VFDIV;
	// Get index of argument by name
	static constexpr size_t Vector1_Index = 0;
	static constexpr size_t Vector2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_VFDiv>);
static_assert(std::is_standard_layout_v<IROp_VFDiv>);

struct __attribute__((packed)) IROp_VFMin {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Vector1;
	OrderedNodeWrapper Vector2;
	static constexpr IROps OPCODE = OP_VFMIN;
	// Get index of argument by name
	static constexpr size_t Vector1_Index = 0;
	static constexpr size_t Vector2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_VFMin>);
static_assert(std::is_standard_layout_v<IROp_VFMin>);

struct __attribute__((packed)) IROp_VFMax {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Vector1;
	OrderedNodeWrapper Vector2;
	static constexpr IROps OPCODE = OP_VFMAX;
	// Get index of argument by name
	static constexpr size_t Vector1_Index = 0;
	static constexpr size_t Vector2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_VFMax>);
static_assert(std::is_standard_layout_v<IROp_VFMax>);

struct __attribute__((packed)) IROp_VMul {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Vector1;
	OrderedNodeWrapper Vector2;
	static constexpr IROps OPCODE = OP_VMUL;
	// Get index of argument by name
	static constexpr size_t Vector1_Index = 0;
	static constexpr size_t Vector2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_VMul>);
static_assert(std::is_standard_layout_v<IROp_VMul>);

struct __attribute__((packed)) IROp_VUMull {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Vector1;
	OrderedNodeWrapper Vector2;
	static constexpr IROps OPCODE = OP_VUMULL;
	// Get index of argument by name
	static constexpr size_t Vector1_Index = 0;
	static constexpr size_t Vector2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_VUMull>);
static_assert(std::is_standard_layout_v<IROp_VUMull>);

struct __attribute__((packed)) IROp_VSMull {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Vector1;
	OrderedNodeWrapper Vector2;
	static constexpr IROps OPCODE = OP_VSMULL;
	// Get index of argument by name
	static constexpr size_t Vector1_Index = 0;
	static constexpr size_t Vector2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_VSMull>);
static_assert(std::is_standard_layout_v<IROp_VSMull>);

struct __attribute__((packed)) IROp_VUMull2 {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Vector1;
	OrderedNodeWrapper Vector2;
	static constexpr IROps OPCODE = OP_VUMULL2;
	// Get index of argument by name
	static constexpr size_t Vector1_Index = 0;
	static constexpr size_t Vector2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_VUMull2>);
static_assert(std::is_standard_layout_v<IROp_VUMull2>);

struct __attribute__((packed)) IROp_VSMull2 {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Vector1;
	OrderedNodeWrapper Vector2;
	static constexpr IROps OPCODE = OP_VSMULL2;
	// Get index of argument by name
	static constexpr size_t Vector1_Index = 0;
	static constexpr size_t Vector2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_VSMull2>);
static_assert(std::is_standard_layout_v<IROp_VSMull2>);

struct __attribute__((packed)) IROp_VUMulH {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Vector1;
	OrderedNodeWrapper Vector2;
	static constexpr IROps OPCODE = OP_VUMULH;
	// Get index of argument by name
	static constexpr size_t Vector1_Index = 0;
	static constexpr size_t Vector2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_VUMulH>);
static_assert(std::is_standard_layout_v<IROp_VUMulH>);

struct __attribute__((packed)) IROp_VSMulH {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Vector1;
	OrderedNodeWrapper Vector2;
	static constexpr IROps OPCODE = OP_VSMULH;
	// Get index of argument by name
	static constexpr size_t Vector1_Index = 0;
	static constexpr size_t Vector2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_VSMulH>);
static_assert(std::is_standard_layout_v<IROp_VSMulH>);

struct __attribute__((packed)) IROp_VUABDL {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Vector1;
	OrderedNodeWrapper Vector2;
	static constexpr IROps OPCODE = OP_VUABDL;
	// Get index of argument by name
	static constexpr size_t Vector1_Index = 0;
	static constexpr size_t Vector2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_VUABDL>);
static_assert(std::is_standard_layout_v<IROp_VUABDL>);

struct __attribute__((packed)) IROp_VUABDL2 {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Vector1;
	OrderedNodeWrapper Vector2;
	static constexpr IROps OPCODE = OP_VUABDL2;
	// Get index of argument by name
	static constexpr size_t Vector1_Index = 0;
	static constexpr size_t Vector2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_VUABDL2>);
static_assert(std::is_standard_layout_v<IROp_VUABDL2>);

struct __attribute__((packed)) IROp_VUShl {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Vector;
	OrderedNodeWrapper ShiftVector;
	// Non-SSA arguments
	bool RangeCheck;
	static constexpr IROps OPCODE = OP_VUSHL;
	// Get index of argument by name
	static constexpr size_t Vector_Index = 0;
	static constexpr size_t ShiftVector_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_VUShl>);
static_assert(std::is_standard_layout_v<IROp_VUShl>);

struct __attribute__((packed)) IROp_VUShr {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Vector;
	OrderedNodeWrapper ShiftVector;
	// Non-SSA arguments
	bool RangeCheck;
	static constexpr IROps OPCODE = OP_VUSHR;
	// Get index of argument by name
	static constexpr size_t Vector_Index = 0;
	static constexpr size_t ShiftVector_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_VUShr>);
static_assert(std::is_standard_layout_v<IROp_VUShr>);

struct __attribute__((packed)) IROp_VSShr {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Vector;
	OrderedNodeWrapper ShiftVector;
	// Non-SSA arguments
	bool RangeCheck;
	static constexpr IROps OPCODE = OP_VSSHR;
	// Get index of argument by name
	static constexpr size_t Vector_Index = 0;
	static constexpr size_t ShiftVector_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_VSShr>);
static_assert(std::is_standard_layout_v<IROp_VSShr>);

struct __attribute__((packed)) IROp_VUShlS {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Vector;
	OrderedNodeWrapper ShiftScalar;
	static constexpr IROps OPCODE = OP_VUSHLS;
	// Get index of argument by name
	static constexpr size_t Vector_Index = 0;
	static constexpr size_t ShiftScalar_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_VUShlS>);
static_assert(std::is_standard_layout_v<IROp_VUShlS>);

struct __attribute__((packed)) IROp_VUShrS {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Vector;
	OrderedNodeWrapper ShiftScalar;
	static constexpr IROps OPCODE = OP_VUSHRS;
	// Get index of argument by name
	static constexpr size_t Vector_Index = 0;
	static constexpr size_t ShiftScalar_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_VUShrS>);
static_assert(std::is_standard_layout_v<IROp_VUShrS>);

struct __attribute__((packed)) IROp_VSShrS {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Vector;
	OrderedNodeWrapper ShiftScalar;
	static constexpr IROps OPCODE = OP_VSSHRS;
	// Get index of argument by name
	static constexpr size_t Vector_Index = 0;
	static constexpr size_t ShiftScalar_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_VSShrS>);
static_assert(std::is_standard_layout_v<IROp_VSShrS>);

struct __attribute__((packed)) IROp_VUShrSWide {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Vector;
	OrderedNodeWrapper ShiftScalar;
	static constexpr IROps OPCODE = OP_VUSHRSWIDE;
	// Get index of argument by name
	static constexpr size_t Vector_Index = 0;
	static constexpr size_t ShiftScalar_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_VUShrSWide>);
static_assert(std::is_standard_layout_v<IROp_VUShrSWide>);

struct __attribute__((packed)) IROp_VSShrSWide {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Vector;
	OrderedNodeWrapper ShiftScalar;
	static constexpr IROps OPCODE = OP_VSSHRSWIDE;
	// Get index of argument by name
	static constexpr size_t Vector_Index = 0;
	static constexpr size_t ShiftScalar_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_VSShrSWide>);
static_assert(std::is_standard_layout_v<IROp_VSShrSWide>);

struct __attribute__((packed)) IROp_VUShlSWide {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Vector;
	OrderedNodeWrapper ShiftScalar;
	static constexpr IROps OPCODE = OP_VUSHLSWIDE;
	// Get index of argument by name
	static constexpr size_t Vector_Index = 0;
	static constexpr size_t ShiftScalar_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_VUShlSWide>);
static_assert(std::is_standard_layout_v<IROp_VUShlSWide>);

struct __attribute__((packed)) IROp_VInsElement {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper DestVector;
	OrderedNodeWrapper SrcVector;
	// Non-SSA arguments
	uint8_t DestIdx;
	uint8_t SrcIdx;
	static constexpr IROps OPCODE = OP_VINSELEMENT;
	// Get index of argument by name
	static constexpr size_t DestVector_Index = 0;
	static constexpr size_t SrcVector_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_VInsElement>);
static_assert(std::is_standard_layout_v<IROp_VInsElement>);

struct __attribute__((packed)) IROp_VInsGPR {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper DestVector;
	OrderedNodeWrapper Src;
	// Non-SSA arguments
	uint8_t DestIdx;
	static constexpr IROps OPCODE = OP_VINSGPR;
	// Get index of argument by name
	static constexpr size_t DestVector_Index = 0;
	static constexpr size_t Src_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_VInsGPR>);
static_assert(std::is_standard_layout_v<IROp_VInsGPR>);

struct __attribute__((packed)) IROp_VExtr {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper VectorLower;
	OrderedNodeWrapper VectorUpper;
	// Non-SSA arguments
	uint8_t Index;
	static constexpr IROps OPCODE = OP_VEXTR;
	// Get index of argument by name
	static constexpr size_t VectorLower_Index = 0;
	static constexpr size_t VectorUpper_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_VExtr>);
static_assert(std::is_standard_layout_v<IROp_VExtr>);

struct __attribute__((packed)) IROp_VCMPEQ {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Vector1;
	OrderedNodeWrapper Vector2;
	static constexpr IROps OPCODE = OP_VCMPEQ;
	// Get index of argument by name
	static constexpr size_t Vector1_Index = 0;
	static constexpr size_t Vector2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_VCMPEQ>);
static_assert(std::is_standard_layout_v<IROp_VCMPEQ>);

struct __attribute__((packed)) IROp_VCMPGT {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Vector1;
	OrderedNodeWrapper Vector2;
	static constexpr IROps OPCODE = OP_VCMPGT;
	// Get index of argument by name
	static constexpr size_t Vector1_Index = 0;
	static constexpr size_t Vector2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_VCMPGT>);
static_assert(std::is_standard_layout_v<IROp_VCMPGT>);

struct __attribute__((packed)) IROp_VFCMPEQ {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Vector1;
	OrderedNodeWrapper Vector2;
	static constexpr IROps OPCODE = OP_VFCMPEQ;
	// Get index of argument by name
	static constexpr size_t Vector1_Index = 0;
	static constexpr size_t Vector2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_VFCMPEQ>);
static_assert(std::is_standard_layout_v<IROp_VFCMPEQ>);

struct __attribute__((packed)) IROp_VFCMPNEQ {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Vector1;
	OrderedNodeWrapper Vector2;
	static constexpr IROps OPCODE = OP_VFCMPNEQ;
	// Get index of argument by name
	static constexpr size_t Vector1_Index = 0;
	static constexpr size_t Vector2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_VFCMPNEQ>);
static_assert(std::is_standard_layout_v<IROp_VFCMPNEQ>);

struct __attribute__((packed)) IROp_VFCMPLT {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Vector1;
	OrderedNodeWrapper Vector2;
	static constexpr IROps OPCODE = OP_VFCMPLT;
	// Get index of argument by name
	static constexpr size_t Vector1_Index = 0;
	static constexpr size_t Vector2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_VFCMPLT>);
static_assert(std::is_standard_layout_v<IROp_VFCMPLT>);

struct __attribute__((packed)) IROp_VFCMPGT {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Vector1;
	OrderedNodeWrapper Vector2;
	static constexpr IROps OPCODE = OP_VFCMPGT;
	// Get index of argument by name
	static constexpr size_t Vector1_Index = 0;
	static constexpr size_t Vector2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_VFCMPGT>);
static_assert(std::is_standard_layout_v<IROp_VFCMPGT>);

struct __attribute__((packed)) IROp_VFCMPLE {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Vector1;
	OrderedNodeWrapper Vector2;
	static constexpr IROps OPCODE = OP_VFCMPLE;
	// Get index of argument by name
	static constexpr size_t Vector1_Index = 0;
	static constexpr size_t Vector2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_VFCMPLE>);
static_assert(std::is_standard_layout_v<IROp_VFCMPLE>);

struct __attribute__((packed)) IROp_VFCMPORD {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Vector1;
	OrderedNodeWrapper Vector2;
	static constexpr IROps OPCODE = OP_VFCMPORD;
	// Get index of argument by name
	static constexpr size_t Vector1_Index = 0;
	static constexpr size_t Vector2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_VFCMPORD>);
static_assert(std::is_standard_layout_v<IROp_VFCMPORD>);

struct __attribute__((packed)) IROp_VFCMPUNO {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Vector1;
	OrderedNodeWrapper Vector2;
	static constexpr IROps OPCODE = OP_VFCMPUNO;
	// Get index of argument by name
	static constexpr size_t Vector1_Index = 0;
	static constexpr size_t Vector2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_VFCMPUNO>);
static_assert(std::is_standard_layout_v<IROp_VFCMPUNO>);

struct __attribute__((packed)) IROp_VTBL1 {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper VectorTable;
	OrderedNodeWrapper VectorIndices;
	static constexpr IROps OPCODE = OP_VTBL1;
	// Get index of argument by name
	static constexpr size_t VectorTable_Index = 0;
	static constexpr size_t VectorIndices_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_VTBL1>);
static_assert(std::is_standard_layout_v<IROp_VTBL1>);

struct __attribute__((packed)) IROp_VTBL2 {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper VectorTable1;
	OrderedNodeWrapper VectorTable2;
	OrderedNodeWrapper VectorIndices;
	static constexpr IROps OPCODE = OP_VTBL2;
	// Get index of argument by name
	static constexpr size_t VectorTable1_Index = 0;
	static constexpr size_t VectorTable2_Index = 1;
	static constexpr size_t VectorIndices_Index = 2;
};
static_assert(std::is_trivially_copyable_v<IROp_VTBL2>);
static_assert(std::is_standard_layout_v<IROp_VTBL2>);

struct __attribute__((packed)) IROp_VTBX1 {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper VectorSrcDst;
	OrderedNodeWrapper VectorTable;
	OrderedNodeWrapper VectorIndices;
	static constexpr IROps OPCODE = OP_VTBX1;
	// Get index of argument by name
	static constexpr size_t VectorSrcDst_Index = 0;
	static constexpr size_t VectorTable_Index = 1;
	static constexpr size_t VectorIndices_Index = 2;
};
static_assert(std::is_trivially_copyable_v<IROp_VTBX1>);
static_assert(std::is_standard_layout_v<IROp_VTBX1>);

struct __attribute__((packed)) IROp_VBSL {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper VectorMask;
	OrderedNodeWrapper VectorTrue;
	OrderedNodeWrapper VectorFalse;
	static constexpr IROps OPCODE = OP_VBSL;
	// Get index of argument by name
	static constexpr size_t VectorMask_Index = 0;
	static constexpr size_t VectorTrue_Index = 1;
	static constexpr size_t VectorFalse_Index = 2;
};
static_assert(std::is_trivially_copyable_v<IROp_VBSL>);
static_assert(std::is_standard_layout_v<IROp_VBSL>);

struct __attribute__((packed)) IROp_VPCMPESTRX {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper LHS;
	OrderedNodeWrapper RHS;
	OrderedNodeWrapper RAX;
	OrderedNodeWrapper RDX;
	// Non-SSA arguments
	uint16_t Control;
	static constexpr IROps OPCODE = OP_VPCMPESTRX;
	// Get index of argument by name
	static constexpr size_t LHS_Index = 0;
	static constexpr size_t RHS_Index = 1;
	static constexpr size_t RAX_Index = 2;
	static constexpr size_t RDX_Index = 3;
};
static_assert(std::is_trivially_copyable_v<IROp_VPCMPESTRX>);
static_assert(std::is_standard_layout_v<IROp_VPCMPESTRX>);

struct __attribute__((packed)) IROp_VPCMPISTRX {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper LHS;
	OrderedNodeWrapper RHS;
	// Non-SSA arguments
	uint8_t Control;
	static constexpr IROps OPCODE = OP_VPCMPISTRX;
	// Get index of argument by name
	static constexpr size_t LHS_Index = 0;
	static constexpr size_t RHS_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_VPCMPISTRX>);
static_assert(std::is_standard_layout_v<IROp_VPCMPISTRX>);

struct __attribute__((packed)) IROp_VFCADD {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Vector1;
	OrderedNodeWrapper Vector2;
	// Non-SSA arguments
	uint16_t Rotate;
	static constexpr IROps OPCODE = OP_VFCADD;
	// Get index of argument by name
	static constexpr size_t Vector1_Index = 0;
	static constexpr size_t Vector2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_VFCADD>);
static_assert(std::is_standard_layout_v<IROp_VFCADD>);

struct __attribute__((packed)) IROp_VFMLA {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Vector1;
	OrderedNodeWrapper Vector2;
	OrderedNodeWrapper Addend;
	static constexpr IROps OPCODE = OP_VFMLA;
	// Get index of argument by name
	static constexpr size_t Vector1_Index = 0;
	static constexpr size_t Vector2_Index = 1;
	static constexpr size_t Addend_Index = 2;
};
static_assert(std::is_trivially_copyable_v<IROp_VFMLA>);
static_assert(std::is_standard_layout_v<IROp_VFMLA>);

struct __attribute__((packed)) IROp_VFMLS {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Vector1;
	OrderedNodeWrapper Vector2;
	OrderedNodeWrapper Addend;
	static constexpr IROps OPCODE = OP_VFMLS;
	// Get index of argument by name
	static constexpr size_t Vector1_Index = 0;
	static constexpr size_t Vector2_Index = 1;
	static constexpr size_t Addend_Index = 2;
};
static_assert(std::is_trivially_copyable_v<IROp_VFMLS>);
static_assert(std::is_standard_layout_v<IROp_VFMLS>);

struct __attribute__((packed)) IROp_VFNMLA {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Vector1;
	OrderedNodeWrapper Vector2;
	OrderedNodeWrapper Addend;
	static constexpr IROps OPCODE = OP_VFNMLA;
	// Get index of argument by name
	static constexpr size_t Vector1_Index = 0;
	static constexpr size_t Vector2_Index = 1;
	static constexpr size_t Addend_Index = 2;
};
static_assert(std::is_trivially_copyable_v<IROp_VFNMLA>);
static_assert(std::is_standard_layout_v<IROp_VFNMLA>);

struct __attribute__((packed)) IROp_VFNMLS {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Vector1;
	OrderedNodeWrapper Vector2;
	OrderedNodeWrapper Addend;
	static constexpr IROps OPCODE = OP_VFNMLS;
	// Get index of argument by name
	static constexpr size_t Vector1_Index = 0;
	static constexpr size_t Vector2_Index = 1;
	static constexpr size_t Addend_Index = 2;
};
static_assert(std::is_trivially_copyable_v<IROp_VFNMLS>);
static_assert(std::is_standard_layout_v<IROp_VFNMLS>);

struct __attribute__((packed)) IROp_VCastFromGPR {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Src;
	static constexpr IROps OPCODE = OP_VCASTFROMGPR;
	// Get index of argument by name
	static constexpr size_t Src_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_VCastFromGPR>);
static_assert(std::is_standard_layout_v<IROp_VCastFromGPR>);

struct __attribute__((packed)) IROp_VDupFromGPR {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Src;
	static constexpr IROps OPCODE = OP_VDUPFROMGPR;
	// Get index of argument by name
	static constexpr size_t Src_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_VDupFromGPR>);
static_assert(std::is_standard_layout_v<IROp_VDupFromGPR>);

struct __attribute__((packed)) IROp_VLoadTwoGPRs {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Lower;
	OrderedNodeWrapper Upper;
	static constexpr IROps OPCODE = OP_VLOADTWOGPRS;
	// Get index of argument by name
	static constexpr size_t Lower_Index = 0;
	static constexpr size_t Upper_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_VLoadTwoGPRs>);
static_assert(std::is_standard_layout_v<IROp_VLoadTwoGPRs>);

struct __attribute__((packed)) IROp_Float_FromGPR_S {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Src;
	// Non-SSA arguments
	FEXCore::IR::OpSize SrcElementSize;
	static constexpr IROps OPCODE = OP_FLOAT_FROMGPR_S;
	// Get index of argument by name
	static constexpr size_t Src_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_Float_FromGPR_S>);
static_assert(std::is_standard_layout_v<IROp_Float_FromGPR_S>);

struct __attribute__((packed)) IROp_Float_FToF {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Scalar;
	// Non-SSA arguments
	FEXCore::IR::OpSize SrcElementSize;
	static constexpr IROps OPCODE = OP_FLOAT_FTOF;
	// Get index of argument by name
	static constexpr size_t Scalar_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_Float_FToF>);
static_assert(std::is_standard_layout_v<IROp_Float_FToF>);

struct __attribute__((packed)) IROp_Vector_SToF {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Vector;
	static constexpr IROps OPCODE = OP_VECTOR_STOF;
	// Get index of argument by name
	static constexpr size_t Vector_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_Vector_SToF>);
static_assert(std::is_standard_layout_v<IROp_Vector_SToF>);

struct __attribute__((packed)) IROp_Vector_FToS {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Vector;
	static constexpr IROps OPCODE = OP_VECTOR_FTOS;
	// Get index of argument by name
	static constexpr size_t Vector_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_Vector_FToS>);
static_assert(std::is_standard_layout_v<IROp_Vector_FToS>);

struct __attribute__((packed)) IROp_Vector_FToZS {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Vector;
	static constexpr IROps OPCODE = OP_VECTOR_FTOZS;
	// Get index of argument by name
	static constexpr size_t Vector_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_Vector_FToZS>);
static_assert(std::is_standard_layout_v<IROp_Vector_FToZS>);

struct __attribute__((packed)) IROp_Vector_FToF {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Vector;
	// Non-SSA arguments
	FEXCore::IR::OpSize SrcElementSize;
	static constexpr IROps OPCODE = OP_VECTOR_FTOF;
	// Get index of argument by name
	static constexpr size_t Vector_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_Vector_FToF>);
static_assert(std::is_standard_layout_v<IROp_Vector_FToF>);

struct __attribute__((packed)) IROp_VFCVTL2 {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Vector;
	static constexpr IROps OPCODE = OP_VFCVTL2;
	// Get index of argument by name
	static constexpr size_t Vector_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_VFCVTL2>);
static_assert(std::is_standard_layout_v<IROp_VFCVTL2>);

struct __attribute__((packed)) IROp_VFCVTN2 {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper VectorLower;
	OrderedNodeWrapper VectorUpper;
	static constexpr IROps OPCODE = OP_VFCVTN2;
	// Get index of argument by name
	static constexpr size_t VectorLower_Index = 0;
	static constexpr size_t VectorUpper_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_VFCVTN2>);
static_assert(std::is_standard_layout_v<IROp_VFCVTN2>);

struct __attribute__((packed)) IROp_Vector_FToI {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Vector;
	// Non-SSA arguments
	RoundMode Round;
	static constexpr IROps OPCODE = OP_VECTOR_FTOI;
	// Get index of argument by name
	static constexpr size_t Vector_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_Vector_FToI>);
static_assert(std::is_standard_layout_v<IROp_Vector_FToI>);

struct __attribute__((packed)) IROp_Vector_FToISized {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Vector;
	// Non-SSA arguments
	bool HostRound;
	FEXCore::IR::OpSize IntSize;
	static constexpr IROps OPCODE = OP_VECTOR_FTOISIZED;
	// Get index of argument by name
	static constexpr size_t Vector_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_Vector_FToISized>);
static_assert(std::is_standard_layout_v<IROp_Vector_FToISized>);

struct __attribute__((packed)) IROp_Vector_F64ToI32 {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Vector;
	// Non-SSA arguments
	RoundMode Round;
	bool EnsureZeroUpperHalf;
	static constexpr IROps OPCODE = OP_VECTOR_F64TOI32;
	// Get index of argument by name
	static constexpr size_t Vector_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_Vector_F64ToI32>);
static_assert(std::is_standard_layout_v<IROp_Vector_F64ToI32>);

struct __attribute__((packed)) IROp_VAESImc {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Vector;
	static constexpr IROps OPCODE = OP_VAESIMC;
	// Get index of argument by name
	static constexpr size_t Vector_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_VAESImc>);
static_assert(std::is_standard_layout_v<IROp_VAESImc>);

struct __attribute__((packed)) IROp_VAESEnc {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper State;
	OrderedNodeWrapper Key;
	OrderedNodeWrapper ZeroReg;
	static constexpr IROps OPCODE = OP_VAESENC;
	// Get index of argument by name
	static constexpr size_t State_Index = 0;
	static constexpr size_t Key_Index = 1;
	static constexpr size_t ZeroReg_Index = 2;
};
static_assert(std::is_trivially_copyable_v<IROp_VAESEnc>);
static_assert(std::is_standard_layout_v<IROp_VAESEnc>);

struct __attribute__((packed)) IROp_VAESEncLast {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper State;
	OrderedNodeWrapper Key;
	OrderedNodeWrapper ZeroReg;
	static constexpr IROps OPCODE = OP_VAESENCLAST;
	// Get index of argument by name
	static constexpr size_t State_Index = 0;
	static constexpr size_t Key_Index = 1;
	static constexpr size_t ZeroReg_Index = 2;
};
static_assert(std::is_trivially_copyable_v<IROp_VAESEncLast>);
static_assert(std::is_standard_layout_v<IROp_VAESEncLast>);

struct __attribute__((packed)) IROp_VAESDec {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper State;
	OrderedNodeWrapper Key;
	OrderedNodeWrapper ZeroReg;
	static constexpr IROps OPCODE = OP_VAESDEC;
	// Get index of argument by name
	static constexpr size_t State_Index = 0;
	static constexpr size_t Key_Index = 1;
	static constexpr size_t ZeroReg_Index = 2;
};
static_assert(std::is_trivially_copyable_v<IROp_VAESDec>);
static_assert(std::is_standard_layout_v<IROp_VAESDec>);

struct __attribute__((packed)) IROp_VAESDecLast {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper State;
	OrderedNodeWrapper Key;
	OrderedNodeWrapper ZeroReg;
	static constexpr IROps OPCODE = OP_VAESDECLAST;
	// Get index of argument by name
	static constexpr size_t State_Index = 0;
	static constexpr size_t Key_Index = 1;
	static constexpr size_t ZeroReg_Index = 2;
};
static_assert(std::is_trivially_copyable_v<IROp_VAESDecLast>);
static_assert(std::is_standard_layout_v<IROp_VAESDecLast>);

struct __attribute__((packed)) IROp_VAESKeyGenAssist {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Src;
	OrderedNodeWrapper KeyGenTBLSwizzle;
	OrderedNodeWrapper ZeroReg;
	// Non-SSA arguments
	uint8_t RCON;
	static constexpr IROps OPCODE = OP_VAESKEYGENASSIST;
	// Get index of argument by name
	static constexpr size_t Src_Index = 0;
	static constexpr size_t KeyGenTBLSwizzle_Index = 1;
	static constexpr size_t ZeroReg_Index = 2;
};
static_assert(std::is_trivially_copyable_v<IROp_VAESKeyGenAssist>);
static_assert(std::is_standard_layout_v<IROp_VAESKeyGenAssist>);

struct __attribute__((packed)) IROp_VSha1H {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Src;
	static constexpr IROps OPCODE = OP_VSHA1H;
	// Get index of argument by name
	static constexpr size_t Src_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_VSha1H>);
static_assert(std::is_standard_layout_v<IROp_VSha1H>);

struct __attribute__((packed)) IROp_VSha1C {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Src1;
	OrderedNodeWrapper Src2;
	OrderedNodeWrapper Src3;
	static constexpr IROps OPCODE = OP_VSHA1C;
	// Get index of argument by name
	static constexpr size_t Src1_Index = 0;
	static constexpr size_t Src2_Index = 1;
	static constexpr size_t Src3_Index = 2;
};
static_assert(std::is_trivially_copyable_v<IROp_VSha1C>);
static_assert(std::is_standard_layout_v<IROp_VSha1C>);

struct __attribute__((packed)) IROp_VSha1M {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Src1;
	OrderedNodeWrapper Src2;
	OrderedNodeWrapper Src3;
	static constexpr IROps OPCODE = OP_VSHA1M;
	// Get index of argument by name
	static constexpr size_t Src1_Index = 0;
	static constexpr size_t Src2_Index = 1;
	static constexpr size_t Src3_Index = 2;
};
static_assert(std::is_trivially_copyable_v<IROp_VSha1M>);
static_assert(std::is_standard_layout_v<IROp_VSha1M>);

struct __attribute__((packed)) IROp_VSha1P {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Src1;
	OrderedNodeWrapper Src2;
	OrderedNodeWrapper Src3;
	static constexpr IROps OPCODE = OP_VSHA1P;
	// Get index of argument by name
	static constexpr size_t Src1_Index = 0;
	static constexpr size_t Src2_Index = 1;
	static constexpr size_t Src3_Index = 2;
};
static_assert(std::is_trivially_copyable_v<IROp_VSha1P>);
static_assert(std::is_standard_layout_v<IROp_VSha1P>);

struct __attribute__((packed)) IROp_VSha1SU1 {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Src1;
	OrderedNodeWrapper Src2;
	static constexpr IROps OPCODE = OP_VSHA1SU1;
	// Get index of argument by name
	static constexpr size_t Src1_Index = 0;
	static constexpr size_t Src2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_VSha1SU1>);
static_assert(std::is_standard_layout_v<IROp_VSha1SU1>);

struct __attribute__((packed)) IROp_VSha256U0 {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Src1;
	OrderedNodeWrapper Src2;
	static constexpr IROps OPCODE = OP_VSHA256U0;
	// Get index of argument by name
	static constexpr size_t Src1_Index = 0;
	static constexpr size_t Src2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_VSha256U0>);
static_assert(std::is_standard_layout_v<IROp_VSha256U0>);

struct __attribute__((packed)) IROp_VSha256U1 {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Src1;
	OrderedNodeWrapper Src2;
	static constexpr IROps OPCODE = OP_VSHA256U1;
	// Get index of argument by name
	static constexpr size_t Src1_Index = 0;
	static constexpr size_t Src2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_VSha256U1>);
static_assert(std::is_standard_layout_v<IROp_VSha256U1>);

struct __attribute__((packed)) IROp_VSha256H {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Src1;
	OrderedNodeWrapper Src2;
	OrderedNodeWrapper Src3;
	static constexpr IROps OPCODE = OP_VSHA256H;
	// Get index of argument by name
	static constexpr size_t Src1_Index = 0;
	static constexpr size_t Src2_Index = 1;
	static constexpr size_t Src3_Index = 2;
};
static_assert(std::is_trivially_copyable_v<IROp_VSha256H>);
static_assert(std::is_standard_layout_v<IROp_VSha256H>);

struct __attribute__((packed)) IROp_VSha256H2 {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Src1;
	OrderedNodeWrapper Src2;
	OrderedNodeWrapper Src3;
	static constexpr IROps OPCODE = OP_VSHA256H2;
	// Get index of argument by name
	static constexpr size_t Src1_Index = 0;
	static constexpr size_t Src2_Index = 1;
	static constexpr size_t Src3_Index = 2;
};
static_assert(std::is_trivially_copyable_v<IROp_VSha256H2>);
static_assert(std::is_standard_layout_v<IROp_VSha256H2>);

struct __attribute__((packed)) IROp_CRC32 {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Src1;
	OrderedNodeWrapper Src2;
	// Non-SSA arguments
	FEXCore::IR::OpSize SrcSize;
	static constexpr IROps OPCODE = OP_CRC32;
	// Get index of argument by name
	static constexpr size_t Src1_Index = 0;
	static constexpr size_t Src2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_CRC32>);
static_assert(std::is_standard_layout_v<IROp_CRC32>);

struct __attribute__((packed)) IROp_PCLMUL {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Src1;
	OrderedNodeWrapper Src2;
	// Non-SSA arguments
	uint8_t Selector;
	static constexpr IROps OPCODE = OP_PCLMUL;
	// Get index of argument by name
	static constexpr size_t Src1_Index = 0;
	static constexpr size_t Src2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_PCLMUL>);
static_assert(std::is_standard_layout_v<IROp_PCLMUL>);

struct __attribute__((packed)) IROp_F64ATAN {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Src1;
	OrderedNodeWrapper Src2;
	static constexpr IROps OPCODE = OP_F64ATAN;
	// Get index of argument by name
	static constexpr size_t Src1_Index = 0;
	static constexpr size_t Src2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_F64ATAN>);
static_assert(std::is_standard_layout_v<IROp_F64ATAN>);

struct __attribute__((packed)) IROp_F64FPREM {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Src1;
	OrderedNodeWrapper Src2;
	static constexpr IROps OPCODE = OP_F64FPREM;
	// Get index of argument by name
	static constexpr size_t Src1_Index = 0;
	static constexpr size_t Src2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_F64FPREM>);
static_assert(std::is_standard_layout_v<IROp_F64FPREM>);

struct __attribute__((packed)) IROp_F64FPREM1 {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Src1;
	OrderedNodeWrapper Src2;
	static constexpr IROps OPCODE = OP_F64FPREM1;
	// Get index of argument by name
	static constexpr size_t Src1_Index = 0;
	static constexpr size_t Src2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_F64FPREM1>);
static_assert(std::is_standard_layout_v<IROp_F64FPREM1>);

struct __attribute__((packed)) IROp_F64SCALE {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Src1;
	OrderedNodeWrapper Src2;
	static constexpr IROps OPCODE = OP_F64SCALE;
	// Get index of argument by name
	static constexpr size_t Src1_Index = 0;
	static constexpr size_t Src2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_F64SCALE>);
static_assert(std::is_standard_layout_v<IROp_F64SCALE>);

struct __attribute__((packed)) IROp_F64F2XM1 {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Src;
	static constexpr IROps OPCODE = OP_F64F2XM1;
	// Get index of argument by name
	static constexpr size_t Src_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_F64F2XM1>);
static_assert(std::is_standard_layout_v<IROp_F64F2XM1>);

struct __attribute__((packed)) IROp_F64FYL2X {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Src;
	OrderedNodeWrapper Src2;
	static constexpr IROps OPCODE = OP_F64FYL2X;
	// Get index of argument by name
	static constexpr size_t Src_Index = 0;
	static constexpr size_t Src2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_F64FYL2X>);
static_assert(std::is_standard_layout_v<IROp_F64FYL2X>);

struct __attribute__((packed)) IROp_F64FYL2XP1 {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Src;
	OrderedNodeWrapper Src2;
	static constexpr IROps OPCODE = OP_F64FYL2XP1;
	// Get index of argument by name
	static constexpr size_t Src_Index = 0;
	static constexpr size_t Src2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_F64FYL2XP1>);
static_assert(std::is_standard_layout_v<IROp_F64FYL2XP1>);

struct __attribute__((packed)) IROp_F64TAN {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Src;
	static constexpr IROps OPCODE = OP_F64TAN;
	// Get index of argument by name
	static constexpr size_t Src_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_F64TAN>);
static_assert(std::is_standard_layout_v<IROp_F64TAN>);

struct __attribute__((packed)) IROp_F64SIN {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Src;
	static constexpr IROps OPCODE = OP_F64SIN;
	// Get index of argument by name
	static constexpr size_t Src_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_F64SIN>);
static_assert(std::is_standard_layout_v<IROp_F64SIN>);

struct __attribute__((packed)) IROp_F64COS {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Src;
	static constexpr IROps OPCODE = OP_F64COS;
	// Get index of argument by name
	static constexpr size_t Src_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_F64COS>);
static_assert(std::is_standard_layout_v<IROp_F64COS>);

struct __attribute__((packed)) IROp_F64SINCOS {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Src;
	OrderedNodeWrapper OutSin;
	OrderedNodeWrapper OutCos;
	static constexpr IROps OPCODE = OP_F64SINCOS;
	// Get index of argument by name
	static constexpr size_t Src_Index = 0;
	static constexpr size_t OutSin_Index = 1;
	static constexpr size_t OutCos_Index = 2;
};
static_assert(std::is_trivially_copyable_v<IROp_F64SINCOS>);
static_assert(std::is_standard_layout_v<IROp_F64SINCOS>);

struct __attribute__((packed)) IROp_SyncStackToSlow {
	IROp_Header Header;
	static constexpr IROps OPCODE = OP_SYNCSTACKTOSLOW;
};
static_assert(std::is_trivially_copyable_v<IROp_SyncStackToSlow>);
static_assert(std::is_standard_layout_v<IROp_SyncStackToSlow>);

struct __attribute__((packed)) IROp_StackForceSlow {
	IROp_Header Header;
	static constexpr IROps OPCODE = OP_STACKFORCESLOW;
};
static_assert(std::is_trivially_copyable_v<IROp_StackForceSlow>);
static_assert(std::is_standard_layout_v<IROp_StackForceSlow>);

struct __attribute__((packed)) IROp_InitStack {
	IROp_Header Header;
	static constexpr IROps OPCODE = OP_INITSTACK;
};
static_assert(std::is_trivially_copyable_v<IROp_InitStack>);
static_assert(std::is_standard_layout_v<IROp_InitStack>);

struct __attribute__((packed)) IROp_IncStackTop {
	IROp_Header Header;
	static constexpr IROps OPCODE = OP_INCSTACKTOP;
};
static_assert(std::is_trivially_copyable_v<IROp_IncStackTop>);
static_assert(std::is_standard_layout_v<IROp_IncStackTop>);

struct __attribute__((packed)) IROp_DecStackTop {
	IROp_Header Header;
	static constexpr IROps OPCODE = OP_DECSTACKTOP;
};
static_assert(std::is_trivially_copyable_v<IROp_DecStackTop>);
static_assert(std::is_standard_layout_v<IROp_DecStackTop>);

struct __attribute__((packed)) IROp_InvalidateStack {
	IROp_Header Header;
	// Non-SSA arguments
	uint8_t StackLocation;
	static constexpr IROps OPCODE = OP_INVALIDATESTACK;
};
static_assert(std::is_trivially_copyable_v<IROp_InvalidateStack>);
static_assert(std::is_standard_layout_v<IROp_InvalidateStack>);

struct __attribute__((packed)) IROp_PushStack {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper X80Src;
	OrderedNodeWrapper OriginalValue;
	// Non-SSA arguments
	FEXCore::IR::OpSize LoadSize;
	static constexpr IROps OPCODE = OP_PUSHSTACK;
	// Get index of argument by name
	static constexpr size_t X80Src_Index = 0;
	static constexpr size_t OriginalValue_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_PushStack>);
static_assert(std::is_standard_layout_v<IROp_PushStack>);

struct __attribute__((packed)) IROp_CopyPushStack {
	IROp_Header Header;
	// Non-SSA arguments
	uint8_t StackLocation;
	static constexpr IROps OPCODE = OP_COPYPUSHSTACK;
};
static_assert(std::is_trivially_copyable_v<IROp_CopyPushStack>);
static_assert(std::is_standard_layout_v<IROp_CopyPushStack>);

struct __attribute__((packed)) IROp_StoreStackMem {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Addr;
	OrderedNodeWrapper Offset;
	// Non-SSA arguments
	FEXCore::IR::OpSize SourceSize;
	FEXCore::IR::OpSize StoreSize;
	FEXCore::IR::OpSize Align;
	MemOffsetType OffsetType;
	uint8_t OffsetScale;
	static constexpr IROps OPCODE = OP_STORESTACKMEM;
	// Get index of argument by name
	static constexpr size_t Addr_Index = 0;
	static constexpr size_t Offset_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_StoreStackMem>);
static_assert(std::is_standard_layout_v<IROp_StoreStackMem>);

struct __attribute__((packed)) IROp_StoreStackToStack {
	IROp_Header Header;
	// Non-SSA arguments
	uint8_t StackLocation;
	static constexpr IROps OPCODE = OP_STORESTACKTOSTACK;
};
static_assert(std::is_trivially_copyable_v<IROp_StoreStackToStack>);
static_assert(std::is_standard_layout_v<IROp_StoreStackToStack>);

struct __attribute__((packed)) IROp_PopStackDestroy {
	IROp_Header Header;
	static constexpr IROps OPCODE = OP_POPSTACKDESTROY;
};
static_assert(std::is_trivially_copyable_v<IROp_PopStackDestroy>);
static_assert(std::is_standard_layout_v<IROp_PopStackDestroy>);

struct __attribute__((packed)) IROp_ReadStackValue {
	IROp_Header Header;
	// Non-SSA arguments
	uint8_t StackLocation;
	static constexpr IROps OPCODE = OP_READSTACKVALUE;
};
static_assert(std::is_trivially_copyable_v<IROp_ReadStackValue>);
static_assert(std::is_standard_layout_v<IROp_ReadStackValue>);

struct __attribute__((packed)) IROp_StackValidTag {
	IROp_Header Header;
	// Non-SSA arguments
	uint8_t StackLocation;
	static constexpr IROps OPCODE = OP_STACKVALIDTAG;
};
static_assert(std::is_trivially_copyable_v<IROp_StackValidTag>);
static_assert(std::is_standard_layout_v<IROp_StackValidTag>);

struct __attribute__((packed)) IROp_F80AddStack {
	IROp_Header Header;
	// Non-SSA arguments
	uint8_t SrcStack1;
	uint8_t SrcStack2;
	static constexpr IROps OPCODE = OP_F80ADDSTACK;
};
static_assert(std::is_trivially_copyable_v<IROp_F80AddStack>);
static_assert(std::is_standard_layout_v<IROp_F80AddStack>);

struct __attribute__((packed)) IROp_F80AddValue {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper X80Src;
	// Non-SSA arguments
	uint8_t SrcStack;
	static constexpr IROps OPCODE = OP_F80ADDVALUE;
	// Get index of argument by name
	static constexpr size_t X80Src_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_F80AddValue>);
static_assert(std::is_standard_layout_v<IROp_F80AddValue>);

struct __attribute__((packed)) IROp_F80Add {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper X80Src1;
	OrderedNodeWrapper X80Src2;
	static constexpr IROps OPCODE = OP_F80ADD;
	// Get index of argument by name
	static constexpr size_t X80Src1_Index = 0;
	static constexpr size_t X80Src2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_F80Add>);
static_assert(std::is_standard_layout_v<IROp_F80Add>);

struct __attribute__((packed)) IROp_F80SubStack {
	IROp_Header Header;
	// Non-SSA arguments
	uint8_t DstStack;
	uint8_t SrcStack1;
	uint8_t SrcStack2;
	static constexpr IROps OPCODE = OP_F80SUBSTACK;
};
static_assert(std::is_trivially_copyable_v<IROp_F80SubStack>);
static_assert(std::is_standard_layout_v<IROp_F80SubStack>);

struct __attribute__((packed)) IROp_F80SubValue {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper X80Src;
	// Non-SSA arguments
	uint8_t SrcStack;
	static constexpr IROps OPCODE = OP_F80SUBVALUE;
	// Get index of argument by name
	static constexpr size_t X80Src_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_F80SubValue>);
static_assert(std::is_standard_layout_v<IROp_F80SubValue>);

struct __attribute__((packed)) IROp_F80SubRValue {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper X80Src;
	// Non-SSA arguments
	uint8_t SrcStack;
	static constexpr IROps OPCODE = OP_F80SUBRVALUE;
	// Get index of argument by name
	static constexpr size_t X80Src_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_F80SubRValue>);
static_assert(std::is_standard_layout_v<IROp_F80SubRValue>);

struct __attribute__((packed)) IROp_F80Sub {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper X80Src1;
	OrderedNodeWrapper X80Src2;
	static constexpr IROps OPCODE = OP_F80SUB;
	// Get index of argument by name
	static constexpr size_t X80Src1_Index = 0;
	static constexpr size_t X80Src2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_F80Sub>);
static_assert(std::is_standard_layout_v<IROp_F80Sub>);

struct __attribute__((packed)) IROp_F80MulStack {
	IROp_Header Header;
	// Non-SSA arguments
	uint8_t SrcStack1;
	uint8_t SrcStack2;
	static constexpr IROps OPCODE = OP_F80MULSTACK;
};
static_assert(std::is_trivially_copyable_v<IROp_F80MulStack>);
static_assert(std::is_standard_layout_v<IROp_F80MulStack>);

struct __attribute__((packed)) IROp_F80MulValue {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper X80Src;
	// Non-SSA arguments
	uint8_t SrcStack;
	static constexpr IROps OPCODE = OP_F80MULVALUE;
	// Get index of argument by name
	static constexpr size_t X80Src_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_F80MulValue>);
static_assert(std::is_standard_layout_v<IROp_F80MulValue>);

struct __attribute__((packed)) IROp_F80Mul {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper X80Src1;
	OrderedNodeWrapper X80Src2;
	static constexpr IROps OPCODE = OP_F80MUL;
	// Get index of argument by name
	static constexpr size_t X80Src1_Index = 0;
	static constexpr size_t X80Src2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_F80Mul>);
static_assert(std::is_standard_layout_v<IROp_F80Mul>);

struct __attribute__((packed)) IROp_F80DivStack {
	IROp_Header Header;
	// Non-SSA arguments
	uint8_t DstStack;
	uint8_t SrcStack1;
	uint8_t SrcStack2;
	static constexpr IROps OPCODE = OP_F80DIVSTACK;
};
static_assert(std::is_trivially_copyable_v<IROp_F80DivStack>);
static_assert(std::is_standard_layout_v<IROp_F80DivStack>);

struct __attribute__((packed)) IROp_F80DivValue {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper X80Src;
	// Non-SSA arguments
	uint8_t SrcStack;
	static constexpr IROps OPCODE = OP_F80DIVVALUE;
	// Get index of argument by name
	static constexpr size_t X80Src_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_F80DivValue>);
static_assert(std::is_standard_layout_v<IROp_F80DivValue>);

struct __attribute__((packed)) IROp_F80DivRValue {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper X80Src;
	// Non-SSA arguments
	uint8_t SrcStack;
	static constexpr IROps OPCODE = OP_F80DIVRVALUE;
	// Get index of argument by name
	static constexpr size_t X80Src_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_F80DivRValue>);
static_assert(std::is_standard_layout_v<IROp_F80DivRValue>);

struct __attribute__((packed)) IROp_F80Div {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper X80Src1;
	OrderedNodeWrapper X80Src2;
	static constexpr IROps OPCODE = OP_F80DIV;
	// Get index of argument by name
	static constexpr size_t X80Src1_Index = 0;
	static constexpr size_t X80Src2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_F80Div>);
static_assert(std::is_standard_layout_v<IROp_F80Div>);

struct __attribute__((packed)) IROp_F80StackXchange {
	IROp_Header Header;
	// Non-SSA arguments
	uint8_t SrcStack;
	static constexpr IROps OPCODE = OP_F80STACKXCHANGE;
};
static_assert(std::is_trivially_copyable_v<IROp_F80StackXchange>);
static_assert(std::is_standard_layout_v<IROp_F80StackXchange>);

struct __attribute__((packed)) IROp_F80StackChangeSign {
	IROp_Header Header;
	static constexpr IROps OPCODE = OP_F80STACKCHANGESIGN;
};
static_assert(std::is_trivially_copyable_v<IROp_F80StackChangeSign>);
static_assert(std::is_standard_layout_v<IROp_F80StackChangeSign>);

struct __attribute__((packed)) IROp_F80StackAbs {
	IROp_Header Header;
	static constexpr IROps OPCODE = OP_F80STACKABS;
};
static_assert(std::is_trivially_copyable_v<IROp_F80StackAbs>);
static_assert(std::is_standard_layout_v<IROp_F80StackAbs>);

struct __attribute__((packed)) IROp_F80PTANStack {
	IROp_Header Header;
	static constexpr IROps OPCODE = OP_F80PTANSTACK;
};
static_assert(std::is_trivially_copyable_v<IROp_F80PTANStack>);
static_assert(std::is_standard_layout_v<IROp_F80PTANStack>);

struct __attribute__((packed)) IROp_F80ATANStack {
	IROp_Header Header;
	static constexpr IROps OPCODE = OP_F80ATANSTACK;
};
static_assert(std::is_trivially_copyable_v<IROp_F80ATANStack>);
static_assert(std::is_standard_layout_v<IROp_F80ATANStack>);

struct __attribute__((packed)) IROp_F80ATAN {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper X80Src1;
	OrderedNodeWrapper X80Src2;
	static constexpr IROps OPCODE = OP_F80ATAN;
	// Get index of argument by name
	static constexpr size_t X80Src1_Index = 0;
	static constexpr size_t X80Src2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_F80ATAN>);
static_assert(std::is_standard_layout_v<IROp_F80ATAN>);

struct __attribute__((packed)) IROp_F80FPREMStack {
	IROp_Header Header;
	static constexpr IROps OPCODE = OP_F80FPREMSTACK;
};
static_assert(std::is_trivially_copyable_v<IROp_F80FPREMStack>);
static_assert(std::is_standard_layout_v<IROp_F80FPREMStack>);

struct __attribute__((packed)) IROp_F80FPREM {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper X80Src1;
	OrderedNodeWrapper X80Src2;
	static constexpr IROps OPCODE = OP_F80FPREM;
	// Get index of argument by name
	static constexpr size_t X80Src1_Index = 0;
	static constexpr size_t X80Src2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_F80FPREM>);
static_assert(std::is_standard_layout_v<IROp_F80FPREM>);

struct __attribute__((packed)) IROp_F80FPREM1Stack {
	IROp_Header Header;
	static constexpr IROps OPCODE = OP_F80FPREM1STACK;
};
static_assert(std::is_trivially_copyable_v<IROp_F80FPREM1Stack>);
static_assert(std::is_standard_layout_v<IROp_F80FPREM1Stack>);

struct __attribute__((packed)) IROp_F80FPREM1 {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper X80Src1;
	OrderedNodeWrapper X80Src2;
	static constexpr IROps OPCODE = OP_F80FPREM1;
	// Get index of argument by name
	static constexpr size_t X80Src1_Index = 0;
	static constexpr size_t X80Src2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_F80FPREM1>);
static_assert(std::is_standard_layout_v<IROp_F80FPREM1>);

struct __attribute__((packed)) IROp_F80SCALEStack {
	IROp_Header Header;
	static constexpr IROps OPCODE = OP_F80SCALESTACK;
};
static_assert(std::is_trivially_copyable_v<IROp_F80SCALEStack>);
static_assert(std::is_standard_layout_v<IROp_F80SCALEStack>);

struct __attribute__((packed)) IROp_F80SCALE {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper X80Src1;
	OrderedNodeWrapper X80Src2;
	static constexpr IROps OPCODE = OP_F80SCALE;
	// Get index of argument by name
	static constexpr size_t X80Src1_Index = 0;
	static constexpr size_t X80Src2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_F80SCALE>);
static_assert(std::is_standard_layout_v<IROp_F80SCALE>);

struct __attribute__((packed)) IROp_F80CVT {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper X80Src;
	static constexpr IROps OPCODE = OP_F80CVT;
	// Get index of argument by name
	static constexpr size_t X80Src_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_F80CVT>);
static_assert(std::is_standard_layout_v<IROp_F80CVT>);

struct __attribute__((packed)) IROp_F80CVTInt {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper X80Src;
	// Non-SSA arguments
	bool Truncate;
	static constexpr IROps OPCODE = OP_F80CVTINT;
	// Get index of argument by name
	static constexpr size_t X80Src_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_F80CVTInt>);
static_assert(std::is_standard_layout_v<IROp_F80CVTInt>);

struct __attribute__((packed)) IROp_F80CVTTo {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper X80Src;
	// Non-SSA arguments
	FEXCore::IR::OpSize SrcSize;
	static constexpr IROps OPCODE = OP_F80CVTTO;
	// Get index of argument by name
	static constexpr size_t X80Src_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_F80CVTTo>);
static_assert(std::is_standard_layout_v<IROp_F80CVTTo>);

struct __attribute__((packed)) IROp_F80CVTToInt {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper Src;
	// Non-SSA arguments
	FEXCore::IR::OpSize SrcSize;
	static constexpr IROps OPCODE = OP_F80CVTTOINT;
	// Get index of argument by name
	static constexpr size_t Src_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_F80CVTToInt>);
static_assert(std::is_standard_layout_v<IROp_F80CVTToInt>);

struct __attribute__((packed)) IROp_F80RoundStack {
	IROp_Header Header;
	static constexpr IROps OPCODE = OP_F80ROUNDSTACK;
};
static_assert(std::is_trivially_copyable_v<IROp_F80RoundStack>);
static_assert(std::is_standard_layout_v<IROp_F80RoundStack>);

struct __attribute__((packed)) IROp_F80Round {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper X80Src;
	static constexpr IROps OPCODE = OP_F80ROUND;
	// Get index of argument by name
	static constexpr size_t X80Src_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_F80Round>);
static_assert(std::is_standard_layout_v<IROp_F80Round>);

struct __attribute__((packed)) IROp_F80F2XM1Stack {
	IROp_Header Header;
	static constexpr IROps OPCODE = OP_F80F2XM1STACK;
};
static_assert(std::is_trivially_copyable_v<IROp_F80F2XM1Stack>);
static_assert(std::is_standard_layout_v<IROp_F80F2XM1Stack>);

struct __attribute__((packed)) IROp_F80F2XM1 {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper X80Src;
	static constexpr IROps OPCODE = OP_F80F2XM1;
	// Get index of argument by name
	static constexpr size_t X80Src_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_F80F2XM1>);
static_assert(std::is_standard_layout_v<IROp_F80F2XM1>);

struct __attribute__((packed)) IROp_F80TAN {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper X80Src;
	static constexpr IROps OPCODE = OP_F80TAN;
	// Get index of argument by name
	static constexpr size_t X80Src_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_F80TAN>);
static_assert(std::is_standard_layout_v<IROp_F80TAN>);

struct __attribute__((packed)) IROp_F80SINStack {
	IROp_Header Header;
	static constexpr IROps OPCODE = OP_F80SINSTACK;
};
static_assert(std::is_trivially_copyable_v<IROp_F80SINStack>);
static_assert(std::is_standard_layout_v<IROp_F80SINStack>);

struct __attribute__((packed)) IROp_F80SIN {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper X80Src;
	static constexpr IROps OPCODE = OP_F80SIN;
	// Get index of argument by name
	static constexpr size_t X80Src_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_F80SIN>);
static_assert(std::is_standard_layout_v<IROp_F80SIN>);

struct __attribute__((packed)) IROp_F80COSStack {
	IROp_Header Header;
	static constexpr IROps OPCODE = OP_F80COSSTACK;
};
static_assert(std::is_trivially_copyable_v<IROp_F80COSStack>);
static_assert(std::is_standard_layout_v<IROp_F80COSStack>);

struct __attribute__((packed)) IROp_F80COS {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper X80Src;
	static constexpr IROps OPCODE = OP_F80COS;
	// Get index of argument by name
	static constexpr size_t X80Src_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_F80COS>);
static_assert(std::is_standard_layout_v<IROp_F80COS>);

struct __attribute__((packed)) IROp_F80SINCOS {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper X80Src;
	OrderedNodeWrapper OutSin;
	OrderedNodeWrapper OutCos;
	static constexpr IROps OPCODE = OP_F80SINCOS;
	// Get index of argument by name
	static constexpr size_t X80Src_Index = 0;
	static constexpr size_t OutSin_Index = 1;
	static constexpr size_t OutCos_Index = 2;
};
static_assert(std::is_trivially_copyable_v<IROp_F80SINCOS>);
static_assert(std::is_standard_layout_v<IROp_F80SINCOS>);

struct __attribute__((packed)) IROp_F80SINCOSStack {
	IROp_Header Header;
	static constexpr IROps OPCODE = OP_F80SINCOSSTACK;
};
static_assert(std::is_trivially_copyable_v<IROp_F80SINCOSStack>);
static_assert(std::is_standard_layout_v<IROp_F80SINCOSStack>);

struct __attribute__((packed)) IROp_F80SQRTStack {
	IROp_Header Header;
	static constexpr IROps OPCODE = OP_F80SQRTSTACK;
};
static_assert(std::is_trivially_copyable_v<IROp_F80SQRTStack>);
static_assert(std::is_standard_layout_v<IROp_F80SQRTStack>);

struct __attribute__((packed)) IROp_F80SQRT {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper X80Src;
	static constexpr IROps OPCODE = OP_F80SQRT;
	// Get index of argument by name
	static constexpr size_t X80Src_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_F80SQRT>);
static_assert(std::is_standard_layout_v<IROp_F80SQRT>);

struct __attribute__((packed)) IROp_F80XTRACT_EXP {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper X80Src;
	static constexpr IROps OPCODE = OP_F80XTRACT_EXP;
	// Get index of argument by name
	static constexpr size_t X80Src_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_F80XTRACT_EXP>);
static_assert(std::is_standard_layout_v<IROp_F80XTRACT_EXP>);

struct __attribute__((packed)) IROp_F80XTRACT_SIG {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper X80Src;
	static constexpr IROps OPCODE = OP_F80XTRACT_SIG;
	// Get index of argument by name
	static constexpr size_t X80Src_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_F80XTRACT_SIG>);
static_assert(std::is_standard_layout_v<IROp_F80XTRACT_SIG>);

struct __attribute__((packed)) IROp_F80StackTest {
	IROp_Header Header;
	// Non-SSA arguments
	uint8_t SrcStack;
	static constexpr IROps OPCODE = OP_F80STACKTEST;
};
static_assert(std::is_trivially_copyable_v<IROp_F80StackTest>);
static_assert(std::is_standard_layout_v<IROp_F80StackTest>);

struct __attribute__((packed)) IROp_F80CmpStack {
	IROp_Header Header;
	// Non-SSA arguments
	uint8_t SrcStack;
	static constexpr IROps OPCODE = OP_F80CMPSTACK;
};
static_assert(std::is_trivially_copyable_v<IROp_F80CmpStack>);
static_assert(std::is_standard_layout_v<IROp_F80CmpStack>);

struct __attribute__((packed)) IROp_F80CmpValue {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper X80Src;
	static constexpr IROps OPCODE = OP_F80CMPVALUE;
	// Get index of argument by name
	static constexpr size_t X80Src_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_F80CmpValue>);
static_assert(std::is_standard_layout_v<IROp_F80CmpValue>);

struct __attribute__((packed)) IROp_F80Cmp {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper X80Src1;
	OrderedNodeWrapper X80Src2;
	static constexpr IROps OPCODE = OP_F80CMP;
	// Get index of argument by name
	static constexpr size_t X80Src1_Index = 0;
	static constexpr size_t X80Src2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_F80Cmp>);
static_assert(std::is_standard_layout_v<IROp_F80Cmp>);

struct __attribute__((packed)) IROp_F80BCDLoad {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper X80Src;
	static constexpr IROps OPCODE = OP_F80BCDLOAD;
	// Get index of argument by name
	static constexpr size_t X80Src_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_F80BCDLoad>);
static_assert(std::is_standard_layout_v<IROp_F80BCDLoad>);

struct __attribute__((packed)) IROp_F80BCDStore {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper X80Src;
	static constexpr IROps OPCODE = OP_F80BCDSTORE;
	// Get index of argument by name
	static constexpr size_t X80Src_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_F80BCDStore>);
static_assert(std::is_standard_layout_v<IROp_F80BCDStore>);

struct __attribute__((packed)) IROp_F80FYL2XStack {
	IROp_Header Header;
	static constexpr IROps OPCODE = OP_F80FYL2XSTACK;
};
static_assert(std::is_trivially_copyable_v<IROp_F80FYL2XStack>);
static_assert(std::is_standard_layout_v<IROp_F80FYL2XStack>);

struct __attribute__((packed)) IROp_F80FYL2X {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper X80Src1;
	OrderedNodeWrapper X80Src2;
	static constexpr IROps OPCODE = OP_F80FYL2X;
	// Get index of argument by name
	static constexpr size_t X80Src1_Index = 0;
	static constexpr size_t X80Src2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_F80FYL2X>);
static_assert(std::is_standard_layout_v<IROp_F80FYL2X>);

struct __attribute__((packed)) IROp_F80FYL2XP1Stack {
	IROp_Header Header;
	static constexpr IROps OPCODE = OP_F80FYL2XP1STACK;
};
static_assert(std::is_trivially_copyable_v<IROp_F80FYL2XP1Stack>);
static_assert(std::is_standard_layout_v<IROp_F80FYL2XP1Stack>);

struct __attribute__((packed)) IROp_F80FYL2XP1 {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper X80Src1;
	OrderedNodeWrapper X80Src2;
	static constexpr IROps OPCODE = OP_F80FYL2XP1;
	// Get index of argument by name
	static constexpr size_t X80Src1_Index = 0;
	static constexpr size_t X80Src2_Index = 1;
};
static_assert(std::is_trivially_copyable_v<IROp_F80FYL2XP1>);
static_assert(std::is_standard_layout_v<IROp_F80FYL2XP1>);

struct __attribute__((packed)) IROp_F80VBSLStack {
	IROp_Header Header;
	// SSA arguments
	OrderedNodeWrapper VectorMask;
	// Non-SSA arguments
	uint8_t SrcStack1;
	uint8_t SrcStack2;
	static constexpr IROps OPCODE = OP_F80VBSLSTACK;
	// Get index of argument by name
	static constexpr size_t VectorMask_Index = 0;
};
static_assert(std::is_trivially_copyable_v<IROp_F80VBSLStack>);
static_assert(std::is_standard_layout_v<IROp_F80VBSLStack>);

struct __attribute__((packed)) IROp_Last {
	IROp_Header Header;
	static constexpr IROps OPCODE = OP_LAST;
};
static_assert(std::is_trivially_copyable_v<IROp_Last>);
static_assert(std::is_standard_layout_v<IROp_Last>);

#undef IROP_STRUCTS
#endif

#ifdef IROP_SIZES
constexpr std::array<size_t, IROps::OP_LAST + 1> IRSizes = {
	sizeof(IROp_Dummy),
	sizeof(IROp_IRHeader),
	sizeof(IROp_CodeBlock),
	sizeof(IROp_BeginBlock),
	sizeof(IROp_InvalidateFlags),
	sizeof(IROp_EndBlock),
	sizeof(IROp_GuestOpcode),
	sizeof(IROp_ValidateCode),
	sizeof(IROp_ThreadRemoveCodeEntry),
	sizeof(IROp_ProcessorID),
	sizeof(IROp_GetRoundingMode),
	sizeof(IROp_SetRoundingMode),
	sizeof(IROp_PushRoundingMode),
	sizeof(IROp_PopRoundingMode),
	sizeof(IROp_Print),
	sizeof(IROp_PrintMsg),
	sizeof(IROp_AllocateGPR),
	sizeof(IROp_AllocateFPR),
	sizeof(IROp_AllocateGPRAfter),
	sizeof(IROp_RDRAND),
	sizeof(IROp_Yield),
	sizeof(IROp_WFET),
	sizeof(IROp_MonoBackpatcherWrite),
	sizeof(IROp_Jump),
	sizeof(IROp_CondJump),
	sizeof(IROp_ExitFunction),
	sizeof(IROp_Break),
	sizeof(IROp_CallbackReturn),
	sizeof(IROp_Syscall),
	sizeof(IROp_Thunk),
	sizeof(IROp_CPUID),
	sizeof(IROp_XGetBV),
	sizeof(IROp_Copy),
	sizeof(IROp_LoadRegister),
	sizeof(IROp_LoadPF),
	sizeof(IROp_LoadAF),
	sizeof(IROp_StoreRegister),
	sizeof(IROp_StorePF),
	sizeof(IROp_StoreAF),
	sizeof(IROp_LoadContext),
	sizeof(IROp_LoadContextPair),
	sizeof(IROp_StoreContext),
	sizeof(IROp_StoreContextPair),
	sizeof(IROp_LoadContextIndexed),
	sizeof(IROp_StoreContextIndexed),
	sizeof(IROp_FormContextAddress),
	sizeof(IROp_SpillRegister),
	sizeof(IROp_FillRegister),
	sizeof(IROp_LoadNZCV),
	sizeof(IROp_StoreNZCV),
	sizeof(IROp_LoadDF),
	sizeof(IROp_LoadMem),
	sizeof(IROp_LoadMemPair),
	sizeof(IROp_StoreMem),
	sizeof(IROp_StoreMemPair),
	sizeof(IROp_StoreMemX87SVEOptPredicate),
	sizeof(IROp_LoadMemX87SVEOptPredicate),
	sizeof(IROp_LoadMemTSO),
	sizeof(IROp_StoreMemTSO),
	sizeof(IROp_VLoadVectorMasked),
	sizeof(IROp_VStoreVectorMasked),
	sizeof(IROp_VLoadVectorGatherMasked),
	sizeof(IROp_VLoadVectorGatherMaskedQPS),
	sizeof(IROp_VLoadVectorElement),
	sizeof(IROp_VStoreVectorElement),
	sizeof(IROp_VBroadcastFromMem),
	sizeof(IROp_Push),
	sizeof(IROp_PushTwo),
	sizeof(IROp_RMWHandle),
	sizeof(IROp_Pop),
	sizeof(IROp_PopTwo),
	sizeof(IROp_MemSet),
	sizeof(IROp_MemCpy),
	sizeof(IROp_CacheLineClear),
	sizeof(IROp_CacheLineClean),
	sizeof(IROp_CacheLineZero),
	sizeof(IROp_Fence),
	sizeof(IROp_Prefetch),
	sizeof(IROp_VStoreNonTemporal),
	sizeof(IROp_VStoreNonTemporalPair),
	sizeof(IROp_VLoadNonTemporal),
	sizeof(IROp_ContextClear),
	sizeof(IROp_CAS),
	sizeof(IROp_CASPair),
	sizeof(IROp_AtomicSwap),
	sizeof(IROp_AtomicFetchAdd),
	sizeof(IROp_AtomicFetchSub),
	sizeof(IROp_AtomicFetchAnd),
	sizeof(IROp_AtomicFetchCLR),
	sizeof(IROp_AtomicFetchOr),
	sizeof(IROp_AtomicFetchXor),
	sizeof(IROp_AtomicFetchNeg),
	sizeof(IROp_TelemetrySetValue),
	sizeof(IROp_EntrypointOffset),
	sizeof(IROp_InlineEntrypointOffset),
	sizeof(IROp_Constant),
	sizeof(IROp_InlineConstant),
	sizeof(IROp_CycleCounter),
	sizeof(IROp_Neg),
	sizeof(IROp_Not),
	sizeof(IROp_Popcount),
	sizeof(IROp_FindLSB),
	sizeof(IROp_FindMSB),
	sizeof(IROp_FindTrailingZeroes),
	sizeof(IROp_CountLeadingZeroes),
	sizeof(IROp_Rev),
	sizeof(IROp_Rbit),
	sizeof(IROp_Add),
	sizeof(IROp_Adc),
	sizeof(IROp_Sbb),
	sizeof(IROp_AddShift),
	sizeof(IROp_AddWithFlags),
	sizeof(IROp_AddNZCV),
	sizeof(IROp_SetSmallNZV),
	sizeof(IROp_CarryInvert),
	sizeof(IROp_AXFlag),
	sizeof(IROp_Parity),
	sizeof(IROp_RmifNZCV),
	sizeof(IROp_CondAddNZCV),
	sizeof(IROp_CondSubNZCV),
	sizeof(IROp_AdcWithFlags),
	sizeof(IROp_AdcZero),
	sizeof(IROp_AdcZeroWithFlags),
	sizeof(IROp_SbbWithFlags),
	sizeof(IROp_AdcNZCV),
	sizeof(IROp_SbbNZCV),
	sizeof(IROp_Sub),
	sizeof(IROp_SubShift),
	sizeof(IROp_SubWithFlags),
	sizeof(IROp_CmpPairZ),
	sizeof(IROp_SubNZCV),
	sizeof(IROp_Or),
	sizeof(IROp_Orlshl),
	sizeof(IROp_Orlshr),
	sizeof(IROp_Ornror),
	sizeof(IROp_Xor),
	sizeof(IROp_XorShift),
	sizeof(IROp_XornShift),
	sizeof(IROp_And),
	sizeof(IROp_AndShift),
	sizeof(IROp_AndWithFlags),
	sizeof(IROp_Andn),
	sizeof(IROp_TestNZ),
	sizeof(IROp_TestZ),
	sizeof(IROp_Lshl),
	sizeof(IROp_Lshr),
	sizeof(IROp_Ashr),
	sizeof(IROp_ShiftFlags),
	sizeof(IROp_RotateFlags),
	sizeof(IROp_Ror),
	sizeof(IROp_Mul),
	sizeof(IROp_UMul),
	sizeof(IROp_UMull),
	sizeof(IROp_SMull),
	sizeof(IROp_MulH),
	sizeof(IROp_UMulH),
	sizeof(IROp_Bfi),
	sizeof(IROp_Bfxil),
	sizeof(IROp_Bfe),
	sizeof(IROp_Sbfe),
	sizeof(IROp_NZCVSelect),
	sizeof(IROp_NZCVSelectV),
	sizeof(IROp_NZCVSelectIncrement),
	sizeof(IROp_Select),
	sizeof(IROp_MaskGenerateFromBitWidth),
	sizeof(IROp_Extr),
	sizeof(IROp_PDep),
	sizeof(IROp_PExt),
	sizeof(IROp_Div),
	sizeof(IROp_UDiv),
	sizeof(IROp_VExtractToGPR),
	sizeof(IROp_Float_ToGPR_S),
	sizeof(IROp_Float_ToGPR_ZS),
	sizeof(IROp_FCmp),
	sizeof(IROp_VFAddScalarInsert),
	sizeof(IROp_VFSubScalarInsert),
	sizeof(IROp_VFMulScalarInsert),
	sizeof(IROp_VFDivScalarInsert),
	sizeof(IROp_VFMinScalarInsert),
	sizeof(IROp_VFMaxScalarInsert),
	sizeof(IROp_VFSqrtScalarInsert),
	sizeof(IROp_VFRSqrtScalarInsert),
	sizeof(IROp_VFRecpScalarInsert),
	sizeof(IROp_VFToFScalarInsert),
	sizeof(IROp_VSToFVectorInsert),
	sizeof(IROp_VSToFGPRInsert),
	sizeof(IROp_VFToIScalarInsert),
	sizeof(IROp_VFCMPScalarInsert),
	sizeof(IROp_VFMLAScalarInsert),
	sizeof(IROp_VFMLSScalarInsert),
	sizeof(IROp_VFNMLAScalarInsert),
	sizeof(IROp_VFNMLSScalarInsert),
	sizeof(IROp_VFCopySign),
	sizeof(IROp_VMov),
	sizeof(IROp_VectorImm),
	sizeof(IROp_LoadNamedVectorConstant),
	sizeof(IROp_LoadNamedVectorIndexedConstant),
	sizeof(IROp_VNeg),
	sizeof(IROp_VNot),
	sizeof(IROp_VAbs),
	sizeof(IROp_VPopcount),
	sizeof(IROp_VAddV),
	sizeof(IROp_VUMinV),
	sizeof(IROp_VUMaxV),
	sizeof(IROp_VFAbs),
	sizeof(IROp_VFNeg),
	sizeof(IROp_VFRecp),
	sizeof(IROp_VFRecpPrecision),
	sizeof(IROp_VFSqrt),
	sizeof(IROp_VFRSqrt),
	sizeof(IROp_VFRSqrtPrecision),
	sizeof(IROp_VCMPEQZ),
	sizeof(IROp_VCMPGTZ),
	sizeof(IROp_VCMPLTZ),
	sizeof(IROp_VDupElement),
	sizeof(IROp_VShlI),
	sizeof(IROp_VUShrI),
	sizeof(IROp_VUShraI),
	sizeof(IROp_VSShrI),
	sizeof(IROp_VUShrNI),
	sizeof(IROp_VUShrNI2),
	sizeof(IROp_VSXTL),
	sizeof(IROp_VSXTL2),
	sizeof(IROp_VSSHLL),
	sizeof(IROp_VSSHLL2),
	sizeof(IROp_VUXTL),
	sizeof(IROp_VUXTL2),
	sizeof(IROp_VSQXTN),
	sizeof(IROp_VSQXTN2),
	sizeof(IROp_VSQXTNPair),
	sizeof(IROp_VSQXTUN),
	sizeof(IROp_VSQXTUN2),
	sizeof(IROp_VSQXTUNPair),
	sizeof(IROp_VSRSHR),
	sizeof(IROp_VSQSHL),
	sizeof(IROp_VRev32),
	sizeof(IROp_VRev64),
	sizeof(IROp_VAdd),
	sizeof(IROp_VSub),
	sizeof(IROp_VAnd),
	sizeof(IROp_VAndn),
	sizeof(IROp_VOrn),
	sizeof(IROp_VOr),
	sizeof(IROp_VXor),
	sizeof(IROp_VUQAdd),
	sizeof(IROp_VUQSub),
	sizeof(IROp_VSQAdd),
	sizeof(IROp_VSQSub),
	sizeof(IROp_VAddP),
	sizeof(IROp_VURAvg),
	sizeof(IROp_VUMin),
	sizeof(IROp_VUMax),
	sizeof(IROp_VSMin),
	sizeof(IROp_VSMax),
	sizeof(IROp_VZip),
	sizeof(IROp_VZip2),
	sizeof(IROp_VUnZip),
	sizeof(IROp_VUnZip2),
	sizeof(IROp_VTrn),
	sizeof(IROp_VTrn2),
	sizeof(IROp_VFAdd),
	sizeof(IROp_VFAddP),
	sizeof(IROp_VFAddV),
	sizeof(IROp_VFSub),
	sizeof(IROp_VFMul),
	sizeof(IROp_VFDiv),
	sizeof(IROp_VFMin),
	sizeof(IROp_VFMax),
	sizeof(IROp_VMul),
	sizeof(IROp_VUMull),
	sizeof(IROp_VSMull),
	sizeof(IROp_VUMull2),
	sizeof(IROp_VSMull2),
	sizeof(IROp_VUMulH),
	sizeof(IROp_VSMulH),
	sizeof(IROp_VUABDL),
	sizeof(IROp_VUABDL2),
	sizeof(IROp_VUShl),
	sizeof(IROp_VUShr),
	sizeof(IROp_VSShr),
	sizeof(IROp_VUShlS),
	sizeof(IROp_VUShrS),
	sizeof(IROp_VSShrS),
	sizeof(IROp_VUShrSWide),
	sizeof(IROp_VSShrSWide),
	sizeof(IROp_VUShlSWide),
	sizeof(IROp_VInsElement),
	sizeof(IROp_VInsGPR),
	sizeof(IROp_VExtr),
	sizeof(IROp_VCMPEQ),
	sizeof(IROp_VCMPGT),
	sizeof(IROp_VFCMPEQ),
	sizeof(IROp_VFCMPNEQ),
	sizeof(IROp_VFCMPLT),
	sizeof(IROp_VFCMPGT),
	sizeof(IROp_VFCMPLE),
	sizeof(IROp_VFCMPORD),
	sizeof(IROp_VFCMPUNO),
	sizeof(IROp_VTBL1),
	sizeof(IROp_VTBL2),
	sizeof(IROp_VTBX1),
	sizeof(IROp_VBSL),
	sizeof(IROp_VPCMPESTRX),
	sizeof(IROp_VPCMPISTRX),
	sizeof(IROp_VFCADD),
	sizeof(IROp_VFMLA),
	sizeof(IROp_VFMLS),
	sizeof(IROp_VFNMLA),
	sizeof(IROp_VFNMLS),
	sizeof(IROp_VCastFromGPR),
	sizeof(IROp_VDupFromGPR),
	sizeof(IROp_VLoadTwoGPRs),
	sizeof(IROp_Float_FromGPR_S),
	sizeof(IROp_Float_FToF),
	sizeof(IROp_Vector_SToF),
	sizeof(IROp_Vector_FToS),
	sizeof(IROp_Vector_FToZS),
	sizeof(IROp_Vector_FToF),
	sizeof(IROp_VFCVTL2),
	sizeof(IROp_VFCVTN2),
	sizeof(IROp_Vector_FToI),
	sizeof(IROp_Vector_FToISized),
	sizeof(IROp_Vector_F64ToI32),
	sizeof(IROp_VAESImc),
	sizeof(IROp_VAESEnc),
	sizeof(IROp_VAESEncLast),
	sizeof(IROp_VAESDec),
	sizeof(IROp_VAESDecLast),
	sizeof(IROp_VAESKeyGenAssist),
	sizeof(IROp_VSha1H),
	sizeof(IROp_VSha1C),
	sizeof(IROp_VSha1M),
	sizeof(IROp_VSha1P),
	sizeof(IROp_VSha1SU1),
	sizeof(IROp_VSha256U0),
	sizeof(IROp_VSha256U1),
	sizeof(IROp_VSha256H),
	sizeof(IROp_VSha256H2),
	sizeof(IROp_CRC32),
	sizeof(IROp_PCLMUL),
	sizeof(IROp_F64ATAN),
	sizeof(IROp_F64FPREM),
	sizeof(IROp_F64FPREM1),
	sizeof(IROp_F64SCALE),
	sizeof(IROp_F64F2XM1),
	sizeof(IROp_F64FYL2X),
	sizeof(IROp_F64FYL2XP1),
	sizeof(IROp_F64TAN),
	sizeof(IROp_F64SIN),
	sizeof(IROp_F64COS),
	sizeof(IROp_F64SINCOS),
	sizeof(IROp_SyncStackToSlow),
	sizeof(IROp_StackForceSlow),
	sizeof(IROp_InitStack),
	sizeof(IROp_IncStackTop),
	sizeof(IROp_DecStackTop),
	sizeof(IROp_InvalidateStack),
	sizeof(IROp_PushStack),
	sizeof(IROp_CopyPushStack),
	sizeof(IROp_StoreStackMem),
	sizeof(IROp_StoreStackToStack),
	sizeof(IROp_PopStackDestroy),
	sizeof(IROp_ReadStackValue),
	sizeof(IROp_StackValidTag),
	sizeof(IROp_F80AddStack),
	sizeof(IROp_F80AddValue),
	sizeof(IROp_F80Add),
	sizeof(IROp_F80SubStack),
	sizeof(IROp_F80SubValue),
	sizeof(IROp_F80SubRValue),
	sizeof(IROp_F80Sub),
	sizeof(IROp_F80MulStack),
	sizeof(IROp_F80MulValue),
	sizeof(IROp_F80Mul),
	sizeof(IROp_F80DivStack),
	sizeof(IROp_F80DivValue),
	sizeof(IROp_F80DivRValue),
	sizeof(IROp_F80Div),
	sizeof(IROp_F80StackXchange),
	sizeof(IROp_F80StackChangeSign),
	sizeof(IROp_F80StackAbs),
	sizeof(IROp_F80PTANStack),
	sizeof(IROp_F80ATANStack),
	sizeof(IROp_F80ATAN),
	sizeof(IROp_F80FPREMStack),
	sizeof(IROp_F80FPREM),
	sizeof(IROp_F80FPREM1Stack),
	sizeof(IROp_F80FPREM1),
	sizeof(IROp_F80SCALEStack),
	sizeof(IROp_F80SCALE),
	sizeof(IROp_F80CVT),
	sizeof(IROp_F80CVTInt),
	sizeof(IROp_F80CVTTo),
	sizeof(IROp_F80CVTToInt),
	sizeof(IROp_F80RoundStack),
	sizeof(IROp_F80Round),
	sizeof(IROp_F80F2XM1Stack),
	sizeof(IROp_F80F2XM1),
	sizeof(IROp_F80TAN),
	sizeof(IROp_F80SINStack),
	sizeof(IROp_F80SIN),
	sizeof(IROp_F80COSStack),
	sizeof(IROp_F80COS),
	sizeof(IROp_F80SINCOS),
	sizeof(IROp_F80SINCOSStack),
	sizeof(IROp_F80SQRTStack),
	sizeof(IROp_F80SQRT),
	sizeof(IROp_F80XTRACT_EXP),
	sizeof(IROp_F80XTRACT_SIG),
	sizeof(IROp_F80StackTest),
	sizeof(IROp_F80CmpStack),
	sizeof(IROp_F80CmpValue),
	sizeof(IROp_F80Cmp),
	sizeof(IROp_F80BCDLoad),
	sizeof(IROp_F80BCDStore),
	sizeof(IROp_F80FYL2XStack),
	sizeof(IROp_F80FYL2X),
	sizeof(IROp_F80FYL2XP1Stack),
	sizeof(IROp_F80FYL2XP1),
	sizeof(IROp_F80VBSLStack),
	-1ULL,

};

// Make sure our array maps directly to the IROps enum
static_assert(IRSizes[IROps::OP_LAST] == -1ULL);

[[nodiscard]] inline size_t GetSize(IROps Op) { return IRSizes[Op]; }
[[nodiscard, gnu::const]] std::string_view const& GetName(IROps Op);
[[nodiscard, gnu::const]] uint8_t GetArgs(IROps Op);
[[nodiscard, gnu::const]] uint8_t GetRAArgs(IROps Op);
[[nodiscard, gnu::const]] FEXCore::IR::RegClass GetRegClass(IROps Op);
[[nodiscard, gnu::const]] bool HasSideEffects(IROps Op);
[[nodiscard, gnu::const]] bool ImplicitFlagClobber(IROps Op);
[[nodiscard, gnu::const]] bool GetHasDest(IROps Op);
[[nodiscard, gnu::const]] bool LoweredX87(IROps Op);
[[nodiscard, gnu::const]] int8_t TiedSource(IROps Op);

#undef IROP_SIZES
#endif
#ifdef IROP_REG_CLASSES_IMPL
constexpr std::array<FEXCore::IR::RegClass, IROps::OP_LAST + 1> IRRegClasses = {
	RegClass::Invalid, // No destination
	RegClass::Invalid, // No destination
	RegClass::Invalid, // No destination
	RegClass::Invalid, // No destination
	RegClass::Invalid, // No destination
	RegClass::Invalid, // No destination
	RegClass::Invalid, // No destination
	RegClass::GPR,
	RegClass::Invalid, // No destination
	RegClass::GPR,
	RegClass::GPR,
	RegClass::Invalid, // No destination
	RegClass::GPR,
	RegClass::Invalid, // No destination
	RegClass::Invalid, // No destination
	RegClass::Invalid, // No destination
	RegClass::GPR,
	RegClass::FPR,
	RegClass::GPR,
	RegClass::GPR,
	RegClass::Invalid, // No destination
	RegClass::Invalid, // No destination
	RegClass::Invalid, // No destination
	RegClass::Invalid, // No destination
	RegClass::Invalid, // No destination
	RegClass::Invalid, // No destination
	RegClass::Invalid, // No destination
	RegClass::Invalid, // No destination
	RegClass::GPR,
	RegClass::Invalid, // No destination
	RegClass::Invalid, // No destination
	RegClass::Invalid, // No destination
	RegClass::GPR,
	RegClass::Complex,
	RegClass::GPR,
	RegClass::GPR,
	RegClass::Complex,
	RegClass::Invalid, // No destination
	RegClass::Invalid, // No destination
	RegClass::Complex,
	RegClass::Invalid, // No destination
	RegClass::Invalid, // No destination
	RegClass::Invalid, // No destination
	RegClass::Complex,
	RegClass::Invalid, // No destination
	RegClass::GPR,
	RegClass::Invalid, // No destination
	RegClass::Complex,
	RegClass::GPR,
	RegClass::Invalid, // No destination
	RegClass::GPR,
	RegClass::Complex,
	RegClass::Invalid, // No destination
	RegClass::Invalid, // No destination
	RegClass::Invalid, // No destination
	RegClass::Invalid, // No destination
	RegClass::FPR,
	RegClass::Complex,
	RegClass::Invalid, // No destination
	RegClass::FPR,
	RegClass::Invalid, // No destination
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::Invalid, // No destination
	RegClass::FPR,
	RegClass::GPR,
	RegClass::Invalid, // No destination
	RegClass::GPR,
	RegClass::Invalid, // No destination
	RegClass::Invalid, // No destination
	RegClass::GPR,
	RegClass::Invalid, // No destination
	RegClass::Invalid, // No destination
	RegClass::Invalid, // No destination
	RegClass::Invalid, // No destination
	RegClass::Invalid, // No destination
	RegClass::Invalid, // No destination
	RegClass::Invalid, // No destination
	RegClass::Invalid, // No destination
	RegClass::FPR,
	RegClass::Invalid, // No destination
	RegClass::GPR,
	RegClass::Invalid, // No destination
	RegClass::GPR,
	RegClass::GPR,
	RegClass::GPR,
	RegClass::GPR,
	RegClass::GPR,
	RegClass::GPR,
	RegClass::GPR,
	RegClass::GPR,
	RegClass::Invalid, // No destination
	RegClass::GPR,
	RegClass::Invalid, // No destination
	RegClass::GPR,
	RegClass::Invalid, // No destination
	RegClass::GPR,
	RegClass::GPR,
	RegClass::GPR,
	RegClass::GPR,
	RegClass::GPR,
	RegClass::GPR,
	RegClass::GPR,
	RegClass::GPR,
	RegClass::GPR,
	RegClass::GPR,
	RegClass::GPR,
	RegClass::GPR,
	RegClass::GPR,
	RegClass::GPR,
	RegClass::GPR,
	RegClass::Invalid, // No destination
	RegClass::Invalid, // No destination
	RegClass::Invalid, // No destination
	RegClass::Invalid, // No destination
	RegClass::GPR,
	RegClass::Invalid, // No destination
	RegClass::Invalid, // No destination
	RegClass::Invalid, // No destination
	RegClass::GPR,
	RegClass::GPR,
	RegClass::GPR,
	RegClass::GPR,
	RegClass::Invalid, // No destination
	RegClass::Invalid, // No destination
	RegClass::GPR,
	RegClass::GPR,
	RegClass::GPR,
	RegClass::Invalid, // No destination
	RegClass::Invalid, // No destination
	RegClass::GPR,
	RegClass::GPR,
	RegClass::GPR,
	RegClass::GPR,
	RegClass::GPR,
	RegClass::GPR,
	RegClass::GPR,
	RegClass::GPR,
	RegClass::GPR,
	RegClass::GPR,
	RegClass::GPR,
	RegClass::Invalid, // No destination
	RegClass::Invalid, // No destination
	RegClass::GPR,
	RegClass::GPR,
	RegClass::GPR,
	RegClass::GPR,
	RegClass::Invalid, // No destination
	RegClass::GPR,
	RegClass::GPR,
	RegClass::GPR,
	RegClass::GPR,
	RegClass::GPR,
	RegClass::GPR,
	RegClass::GPR,
	RegClass::GPR,
	RegClass::GPR,
	RegClass::GPR,
	RegClass::GPR,
	RegClass::GPR,
	RegClass::FPR,
	RegClass::GPR,
	RegClass::GPR,
	RegClass::GPR,
	RegClass::GPR,
	RegClass::GPR,
	RegClass::GPR,
	RegClass::Invalid, // No destination
	RegClass::Invalid, // No destination
	RegClass::GPR,
	RegClass::GPR,
	RegClass::GPR,
	RegClass::Invalid, // No destination
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::GPR,
	RegClass::GPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::GPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::Invalid, // No destination
	RegClass::GPR,
	RegClass::Invalid, // No destination
	RegClass::Invalid, // No destination
	RegClass::Invalid, // No destination
	RegClass::Invalid, // No destination
	RegClass::Invalid, // No destination
	RegClass::Invalid, // No destination
	RegClass::Invalid, // No destination
	RegClass::Invalid, // No destination
	RegClass::Invalid, // No destination
	RegClass::Invalid, // No destination
	RegClass::FPR,
	RegClass::GPR,
	RegClass::Invalid, // No destination
	RegClass::Invalid, // No destination
	RegClass::FPR,
	RegClass::Invalid, // No destination
	RegClass::Invalid, // No destination
	RegClass::Invalid, // No destination
	RegClass::FPR,
	RegClass::Invalid, // No destination
	RegClass::Invalid, // No destination
	RegClass::FPR,
	RegClass::Invalid, // No destination
	RegClass::Invalid, // No destination
	RegClass::Invalid, // No destination
	RegClass::FPR,
	RegClass::Invalid, // No destination
	RegClass::FPR,
	RegClass::FPR,
	RegClass::Invalid, // No destination
	RegClass::FPR,
	RegClass::FPR,
	RegClass::Invalid, // No destination
	RegClass::FPR,
	RegClass::Invalid, // No destination
	RegClass::FPR,
	RegClass::Invalid, // No destination
	RegClass::FPR,
	RegClass::FPR,
	RegClass::GPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::Invalid, // No destination
	RegClass::FPR,
	RegClass::Invalid, // No destination
	RegClass::FPR,
	RegClass::FPR,
	RegClass::Invalid, // No destination
	RegClass::FPR,
	RegClass::Invalid, // No destination
	RegClass::FPR,
	RegClass::Invalid, // No destination
	RegClass::Invalid, // No destination
	RegClass::Invalid, // No destination
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::GPR,
	RegClass::GPR,
	RegClass::GPR,
	RegClass::GPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::FPR,
	RegClass::Invalid, // No destination
	RegClass::Invalid,
};

// Make sure our array maps directly to the IROps enum
static_assert(IRRegClasses[IROps::OP_LAST] == RegClass::Invalid);

FEXCore::IR::RegClass GetRegClass(IROps Op) { return IRRegClasses[Op]; }

#undef IROP_REG_CLASSES_IMPL
#endif

#ifdef IROP_GETNAME_IMPL
constexpr std::array<std::string_view const, OP_LAST + 1> IRNames = {
	"Dummy",
	"IRHeader",
	"CodeBlock",
	"BeginBlock",
	"InvalidateFlags",
	"EndBlock",
	"GuestOpcode",
	"ValidateCode",
	"ThreadRemoveCodeEntry",
	"ProcessorID",
	"GetRoundingMode",
	"SetRoundingMode",
	"PushRoundingMode",
	"PopRoundingMode",
	"Print",
	"PrintMsg",
	"AllocateGPR",
	"AllocateFPR",
	"AllocateGPRAfter",
	"RDRAND",
	"Yield",
	"WFET",
	"MonoBackpatcherWrite",
	"Jump",
	"CondJump",
	"ExitFunction",
	"Break",
	"CallbackReturn",
	"Syscall",
	"Thunk",
	"CPUID",
	"XGetBV",
	"Copy",
	"LoadRegister",
	"LoadPF",
	"LoadAF",
	"StoreRegister",
	"StorePF",
	"StoreAF",
	"LoadContext",
	"LoadContextPair",
	"StoreContext",
	"StoreContextPair",
	"LoadContextIndexed",
	"StoreContextIndexed",
	"FormContextAddress",
	"SpillRegister",
	"FillRegister",
	"LoadNZCV",
	"StoreNZCV",
	"LoadDF",
	"LoadMem",
	"LoadMemPair",
	"StoreMem",
	"StoreMemPair",
	"StoreMemX87SVEOptPredicate",
	"LoadMemX87SVEOptPredicate",
	"LoadMemTSO",
	"StoreMemTSO",
	"VLoadVectorMasked",
	"VStoreVectorMasked",
	"VLoadVectorGatherMasked",
	"VLoadVectorGatherMaskedQPS",
	"VLoadVectorElement",
	"VStoreVectorElement",
	"VBroadcastFromMem",
	"Push",
	"PushTwo",
	"RMWHandle",
	"Pop",
	"PopTwo",
	"MemSet",
	"MemCpy",
	"CacheLineClear",
	"CacheLineClean",
	"CacheLineZero",
	"Fence",
	"Prefetch",
	"VStoreNonTemporal",
	"VStoreNonTemporalPair",
	"VLoadNonTemporal",
	"ContextClear",
	"CAS",
	"CASPair",
	"AtomicSwap",
	"AtomicFetchAdd",
	"AtomicFetchSub",
	"AtomicFetchAnd",
	"AtomicFetchCLR",
	"AtomicFetchOr",
	"AtomicFetchXor",
	"AtomicFetchNeg",
	"TelemetrySetValue",
	"EntrypointOffset",
	"InlineEntrypointOffset",
	"Constant",
	"InlineConstant",
	"CycleCounter",
	"Neg",
	"Not",
	"Popcount",
	"FindLSB",
	"FindMSB",
	"FindTrailingZeroes",
	"CountLeadingZeroes",
	"Rev",
	"Rbit",
	"Add",
	"Adc",
	"Sbb",
	"AddShift",
	"AddWithFlags",
	"AddNZCV",
	"SetSmallNZV",
	"CarryInvert",
	"AXFlag",
	"Parity",
	"RmifNZCV",
	"CondAddNZCV",
	"CondSubNZCV",
	"AdcWithFlags",
	"AdcZero",
	"AdcZeroWithFlags",
	"SbbWithFlags",
	"AdcNZCV",
	"SbbNZCV",
	"Sub",
	"SubShift",
	"SubWithFlags",
	"CmpPairZ",
	"SubNZCV",
	"Or",
	"Orlshl",
	"Orlshr",
	"Ornror",
	"Xor",
	"XorShift",
	"XornShift",
	"And",
	"AndShift",
	"AndWithFlags",
	"Andn",
	"TestNZ",
	"TestZ",
	"Lshl",
	"Lshr",
	"Ashr",
	"ShiftFlags",
	"RotateFlags",
	"Ror",
	"Mul",
	"UMul",
	"UMull",
	"SMull",
	"MulH",
	"UMulH",
	"Bfi",
	"Bfxil",
	"Bfe",
	"Sbfe",
	"NZCVSelect",
	"NZCVSelectV",
	"NZCVSelectIncrement",
	"Select",
	"MaskGenerateFromBitWidth",
	"Extr",
	"PDep",
	"PExt",
	"Div",
	"UDiv",
	"VExtractToGPR",
	"Float_ToGPR_S",
	"Float_ToGPR_ZS",
	"FCmp",
	"VFAddScalarInsert",
	"VFSubScalarInsert",
	"VFMulScalarInsert",
	"VFDivScalarInsert",
	"VFMinScalarInsert",
	"VFMaxScalarInsert",
	"VFSqrtScalarInsert",
	"VFRSqrtScalarInsert",
	"VFRecpScalarInsert",
	"VFToFScalarInsert",
	"VSToFVectorInsert",
	"VSToFGPRInsert",
	"VFToIScalarInsert",
	"VFCMPScalarInsert",
	"VFMLAScalarInsert",
	"VFMLSScalarInsert",
	"VFNMLAScalarInsert",
	"VFNMLSScalarInsert",
	"VFCopySign",
	"VMov",
	"VectorImm",
	"LoadNamedVectorConstant",
	"LoadNamedVectorIndexedConstant",
	"VNeg",
	"VNot",
	"VAbs",
	"VPopcount",
	"VAddV",
	"VUMinV",
	"VUMaxV",
	"VFAbs",
	"VFNeg",
	"VFRecp",
	"VFRecpPrecision",
	"VFSqrt",
	"VFRSqrt",
	"VFRSqrtPrecision",
	"VCMPEQZ",
	"VCMPGTZ",
	"VCMPLTZ",
	"VDupElement",
	"VShlI",
	"VUShrI",
	"VUShraI",
	"VSShrI",
	"VUShrNI",
	"VUShrNI2",
	"VSXTL",
	"VSXTL2",
	"VSSHLL",
	"VSSHLL2",
	"VUXTL",
	"VUXTL2",
	"VSQXTN",
	"VSQXTN2",
	"VSQXTNPair",
	"VSQXTUN",
	"VSQXTUN2",
	"VSQXTUNPair",
	"VSRSHR",
	"VSQSHL",
	"VRev32",
	"VRev64",
	"VAdd",
	"VSub",
	"VAnd",
	"VAndn",
	"VOrn",
	"VOr",
	"VXor",
	"VUQAdd",
	"VUQSub",
	"VSQAdd",
	"VSQSub",
	"VAddP",
	"VURAvg",
	"VUMin",
	"VUMax",
	"VSMin",
	"VSMax",
	"VZip",
	"VZip2",
	"VUnZip",
	"VUnZip2",
	"VTrn",
	"VTrn2",
	"VFAdd",
	"VFAddP",
	"VFAddV",
	"VFSub",
	"VFMul",
	"VFDiv",
	"VFMin",
	"VFMax",
	"VMul",
	"VUMull",
	"VSMull",
	"VUMull2",
	"VSMull2",
	"VUMulH",
	"VSMulH",
	"VUABDL",
	"VUABDL2",
	"VUShl",
	"VUShr",
	"VSShr",
	"VUShlS",
	"VUShrS",
	"VSShrS",
	"VUShrSWide",
	"VSShrSWide",
	"VUShlSWide",
	"VInsElement",
	"VInsGPR",
	"VExtr",
	"VCMPEQ",
	"VCMPGT",
	"VFCMPEQ",
	"VFCMPNEQ",
	"VFCMPLT",
	"VFCMPGT",
	"VFCMPLE",
	"VFCMPORD",
	"VFCMPUNO",
	"VTBL1",
	"VTBL2",
	"VTBX1",
	"VBSL",
	"VPCMPESTRX",
	"VPCMPISTRX",
	"VFCADD",
	"VFMLA",
	"VFMLS",
	"VFNMLA",
	"VFNMLS",
	"VCastFromGPR",
	"VDupFromGPR",
	"VLoadTwoGPRs",
	"Float_FromGPR_S",
	"Float_FToF",
	"Vector_SToF",
	"Vector_FToS",
	"Vector_FToZS",
	"Vector_FToF",
	"VFCVTL2",
	"VFCVTN2",
	"Vector_FToI",
	"Vector_FToISized",
	"Vector_F64ToI32",
	"VAESImc",
	"VAESEnc",
	"VAESEncLast",
	"VAESDec",
	"VAESDecLast",
	"VAESKeyGenAssist",
	"VSha1H",
	"VSha1C",
	"VSha1M",
	"VSha1P",
	"VSha1SU1",
	"VSha256U0",
	"VSha256U1",
	"VSha256H",
	"VSha256H2",
	"CRC32",
	"PCLMUL",
	"F64ATAN",
	"F64FPREM",
	"F64FPREM1",
	"F64SCALE",
	"F64F2XM1",
	"F64FYL2X",
	"F64FYL2XP1",
	"F64TAN",
	"F64SIN",
	"F64COS",
	"F64SINCOS",
	"SyncStackToSlow",
	"StackForceSlow",
	"InitStack",
	"IncStackTop",
	"DecStackTop",
	"InvalidateStack",
	"PushStack",
	"CopyPushStack",
	"StoreStackMem",
	"StoreStackToStack",
	"PopStackDestroy",
	"ReadStackValue",
	"StackValidTag",
	"F80AddStack",
	"F80AddValue",
	"F80Add",
	"F80SubStack",
	"F80SubValue",
	"F80SubRValue",
	"F80Sub",
	"F80MulStack",
	"F80MulValue",
	"F80Mul",
	"F80DivStack",
	"F80DivValue",
	"F80DivRValue",
	"F80Div",
	"F80StackXchange",
	"F80StackChangeSign",
	"F80StackAbs",
	"F80PTANStack",
	"F80ATANStack",
	"F80ATAN",
	"F80FPREMStack",
	"F80FPREM",
	"F80FPREM1Stack",
	"F80FPREM1",
	"F80SCALEStack",
	"F80SCALE",
	"F80CVT",
	"F80CVTInt",
	"F80CVTTo",
	"F80CVTToInt",
	"F80RoundStack",
	"F80Round",
	"F80F2XM1Stack",
	"F80F2XM1",
	"F80TAN",
	"F80SINStack",
	"F80SIN",
	"F80COSStack",
	"F80COS",
	"F80SINCOS",
	"F80SINCOSStack",
	"F80SQRTStack",
	"F80SQRT",
	"F80XTRACT_EXP",
	"F80XTRACT_SIG",
	"F80StackTest",
	"F80CmpStack",
	"F80CmpValue",
	"F80Cmp",
	"F80BCDLoad",
	"F80BCDStore",
	"F80FYL2XStack",
	"F80FYL2X",
	"F80FYL2XP1Stack",
	"F80FYL2XP1",
	"F80VBSLStack",
	"Last",
};

static_assert(IRNames[OP_LAST] == "Last");

std::string_view const& GetName(IROps Op) {
  return IRNames[Op];
}
#undef IROP_GETNAME_IMPL
#endif

#ifdef IROP_GETRAARGS_IMPL
constexpr std::array<uint8_t, OP_LAST + 1> IRRAArgs = {
	0,
	1,
	0,
	0,
	0,
	0,
	0,
	1,
	0,
	0,
	0,
	2,
	0,
	1,
	1,
	0,
	0,
	0,
	1,
	0,
	0,
	2,
	2,
	0,
	2,
	2,
	0,
	0,
	7,
	1,
	6,
	3,
	1,
	0,
	0,
	0,
	1,
	1,
	1,
	0,
	2,
	1,
	2,
	1,
	2,
	1,
	1,
	0,
	0,
	1,
	0,
	2,
	3,
	3,
	3,
	2,
	1,
	2,
	3,
	3,
	4,
	5,
	5,
	2,
	2,
	1,
	2,
	3,
	1,
	2,
	3,
	5,
	6,
	1,
	1,
	1,
	0,
	2,
	2,
	3,
	1,
	0,
	3,
	7,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	1,
	1,
	0,
	0,
	0,
	0,
	0,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	2,
	2,
	2,
	2,
	2,
	2,
	1,
	0,
	1,
	1,
	1,
	2,
	2,
	2,
	1,
	1,
	2,
	2,
	2,
	2,
	2,
	2,
	4,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	4,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	1,
	1,
	2,
	2,
	2,
	4,
	1,
	2,
	2,
	2,
	5,
	5,
	1,
	1,
	1,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	4,
	4,
	4,
	4,
	2,
	1,
	0,
	0,
	0,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	2,
	1,
	1,
	2,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	2,
	2,
	1,
	2,
	2,
	1,
	1,
	1,
	1,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	1,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	3,
	3,
	3,
	4,
	2,
	2,
	3,
	3,
	3,
	3,
	1,
	1,
	2,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	2,
	1,
	1,
	1,
	1,
	3,
	3,
	3,
	3,
	3,
	1,
	3,
	3,
	3,
	2,
	2,
	2,
	3,
	3,
	2,
	2,
	2,
	2,
	2,
	2,
	1,
	2,
	2,
	1,
	1,
	1,
	3,
	0,
	0,
	0,
	0,
	0,
	0,
	2,
	0,
	2,
	0,
	0,
	0,
	0,
	0,
	1,
	2,
	0,
	1,
	1,
	2,
	0,
	1,
	2,
	0,
	1,
	1,
	2,
	0,
	0,
	0,
	0,
	0,
	2,
	0,
	2,
	0,
	2,
	0,
	2,
	1,
	1,
	1,
	1,
	0,
	1,
	0,
	1,
	1,
	0,
	1,
	0,
	1,
	3,
	0,
	0,
	1,
	1,
	1,
	0,
	0,
	1,
	2,
	1,
	1,
	0,
	2,
	0,
	2,
	1,
	0,
};

constexpr std::array<uint8_t, OP_LAST + 1> IRArgs = {
	0,
	1,
	2,
	1,
	0,
	1,
	0,
	1,
	0,
	0,
	0,
	2,
	0,
	1,
	1,
	0,
	0,
	0,
	1,
	0,
	0,
	2,
	2,
	1,
	4,
	3,
	0,
	0,
	7,
	1,
	6,
	3,
	1,
	0,
	0,
	0,
	1,
	1,
	1,
	0,
	2,
	1,
	2,
	1,
	2,
	1,
	1,
	0,
	0,
	1,
	0,
	2,
	3,
	3,
	3,
	2,
	1,
	2,
	3,
	3,
	4,
	5,
	5,
	2,
	2,
	1,
	2,
	3,
	1,
	2,
	3,
	5,
	6,
	1,
	1,
	1,
	0,
	2,
	2,
	3,
	1,
	0,
	3,
	7,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	1,
	1,
	0,
	0,
	0,
	0,
	0,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	2,
	2,
	2,
	2,
	2,
	2,
	1,
	0,
	1,
	1,
	1,
	2,
	2,
	2,
	1,
	1,
	2,
	2,
	2,
	2,
	2,
	2,
	4,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	4,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	1,
	1,
	2,
	2,
	2,
	4,
	1,
	2,
	2,
	2,
	5,
	5,
	1,
	1,
	1,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	4,
	4,
	4,
	4,
	2,
	1,
	0,
	0,
	0,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	2,
	1,
	1,
	2,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	2,
	2,
	1,
	2,
	2,
	1,
	1,
	1,
	1,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	1,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	3,
	3,
	3,
	4,
	2,
	2,
	3,
	3,
	3,
	3,
	1,
	1,
	2,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	2,
	1,
	1,
	1,
	1,
	3,
	3,
	3,
	3,
	3,
	1,
	3,
	3,
	3,
	2,
	2,
	2,
	3,
	3,
	2,
	2,
	2,
	2,
	2,
	2,
	1,
	2,
	2,
	1,
	1,
	1,
	3,
	0,
	0,
	0,
	0,
	0,
	0,
	2,
	0,
	2,
	0,
	0,
	0,
	0,
	0,
	1,
	2,
	0,
	1,
	1,
	2,
	0,
	1,
	2,
	0,
	1,
	1,
	2,
	0,
	0,
	0,
	0,
	0,
	2,
	0,
	2,
	0,
	2,
	0,
	2,
	1,
	1,
	1,
	1,
	0,
	1,
	0,
	1,
	1,
	0,
	1,
	0,
	1,
	3,
	0,
	0,
	1,
	1,
	1,
	0,
	0,
	1,
	2,
	1,
	1,
	0,
	2,
	0,
	2,
	1,
	0,
};

uint8_t GetRAArgs(IROps Op) {
  return IRRAArgs[Op];
}
uint8_t GetArgs(IROps Op) {
  return IRArgs[Op];
}
#undef IROP_GETRAARGS_IMPL
#endif

#ifdef IROP_HASSIDEEFFECTS_IMPL
constexpr std::array<uint8_t, OP_LAST + 1> HasSideEffects_ = {
	true,
	false,
	false,
	true,
	true,
	true,
	true,
	true,
	true,
	false,
	false,
	true,
	true,
	true,
	true,
	true,
	false,
	false,
	false,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	false,
	false,
	false,
	false,
	true,
	true,
	true,
	false,
	true,
	true,
	true,
	false,
	true,
	false,
	true,
	false,
	false,
	true,
	false,
	false,
	true,
	true,
	true,
	true,
	false,
	false,
	true,
	false,
	true,
	false,
	false,
	false,
	true,
	false,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	false,
	true,
	false,
	true,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	true,
	true,
	true,
	true,
	true,
	false,
	true,
	true,
	true,
	true,
	false,
	true,
	true,
	true,
	true,
	false,
	false,
	true,
	true,
	true,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	true,
	false,
	true,
	true,
	false,
	false,
	false,
	true,
	true,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	true,
	true,
	false,
	false,
	false,
	true,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	false,
	false,
	true,
	true,
	false,
	true,
	true,
	true,
	false,
	true,
	true,
	false,
	true,
	true,
	true,
	false,
	true,
	true,
	true,
	true,
	true,
	false,
	true,
	false,
	true,
	false,
	true,
	false,
	false,
	false,
	false,
	false,
	true,
	false,
	true,
	false,
	false,
	true,
	false,
	true,
	false,
	true,
	true,
	true,
	false,
	false,
	false,
	false,
	false,
	true,
	false,
	false,
	false,
	true,
	false,
	true,
	false,
	true,
	true,
};

bool HasSideEffects(IROps Op) {
  return HasSideEffects_[Op];
}
constexpr std::array<uint8_t, OP_LAST + 1> ImplicitFlagClobber_ = {
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	true,
	true,
	true,
	true,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	true,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	true,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	true,
	true,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	true,
	true,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
};

bool ImplicitFlagClobber(IROps Op) {
  return ImplicitFlagClobber_[Op];
}
constexpr std::array<uint8_t, OP_LAST + 1> LoweredX87_ = {
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	false,
	true,
	true,
	true,
	false,
	true,
	true,
	false,
	true,
	true,
	true,
	false,
	true,
	true,
	true,
	true,
	true,
	false,
	true,
	false,
	true,
	false,
	true,
	false,
	false,
	false,
	false,
	false,
	true,
	false,
	true,
	false,
	false,
	true,
	false,
	true,
	false,
	false,
	true,
	true,
	false,
	false,
	false,
	true,
	true,
	true,
	false,
	false,
	false,
	true,
	false,
	true,
	false,
	true,
	false,
};

bool LoweredX87(IROps Op) {
  return LoweredX87_[Op];
}
constexpr std::array<int8_t, OP_LAST + 1> TiedSource_ = {
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	0,
	0,
	0,
	-1,
	-1,
	1,
	-1,
	0,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	0,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	0,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	0,
	0,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	0,
	0,
	0,
	0,
	0,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	0,
	0,
	0,
	0,
	0,
	0,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	0,
	0,
	-1,
	-1,
	-1,
	-1,
	0,
	0,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	0,
	0,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	0,
	0,
	-1,
	-1,
	-1,
	2,
	2,
	2,
	2,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	0,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	0,
	0,
	0,
	0,
	0,
	-1,
	0,
	0,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
};

int8_t TiedSource(IROps Op) {
  return TiedSource_[Op];
}
#undef IROP_HASSIDEEFFECTS_IMPL
#endif

#ifdef IROP_GETHASDEST_IMPL
constexpr std::array<bool, OP_LAST + 1> IRDest = {
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	true,
	false,
	true,
	true,
	false,
	true,
	false,
	false,
	false,
	true,
	true,
	true,
	true,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	true,
	false,
	false,
	false,
	true,
	true,
	true,
	true,
	true,
	false,
	false,
	true,
	false,
	false,
	false,
	true,
	false,
	true,
	false,
	true,
	true,
	false,
	true,
	true,
	false,
	false,
	false,
	false,
	true,
	true,
	false,
	true,
	false,
	true,
	true,
	true,
	false,
	true,
	true,
	false,
	true,
	false,
	false,
	true,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	true,
	false,
	true,
	false,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	false,
	true,
	false,
	true,
	false,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	false,
	false,
	false,
	false,
	true,
	false,
	false,
	false,
	true,
	true,
	true,
	true,
	false,
	false,
	true,
	true,
	true,
	false,
	false,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	false,
	false,
	true,
	true,
	true,
	true,
	false,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	false,
	false,
	true,
	true,
	true,
	false,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	false,
	true,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	true,
	true,
	false,
	false,
	true,
	false,
	false,
	false,
	true,
	false,
	false,
	true,
	false,
	false,
	false,
	true,
	false,
	true,
	true,
	false,
	true,
	true,
	false,
	true,
	false,
	true,
	false,
	true,
	true,
	true,
	true,
	true,
	false,
	true,
	false,
	true,
	true,
	false,
	true,
	false,
	true,
	false,
	false,
	false,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	false,
	false,
};

bool GetHasDest(IROps Op) {
  return IRDest[Op];
}
#undef IROP_GETHASDEST_IMPL
#endif

#ifdef IROP_ARGPRINTER_HELPER
switch (IROp->Op) {
case IROps::OP_DUMMY: {
break;
}
case IROps::OP_IRHEADER: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_IRHeader>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->OriginalRIP);
	*out << ", ";
	PrintArg(out, IR, Op->BlockCount);
	*out << ", ";
	PrintArg(out, IR, Op->NumHostInstructions);
	*out << ", ";
	PrintArg(out, IR, Op->SpillSlots);
	*out << ", ";
	PrintArg(out, IR, Op->PostRA);
	*out << ", ";
	PrintArg(out, IR, Op->HasX87);
	*out << ", ";
	PrintArg(out, IR, Op->ReadsParity);
break;
}
case IROps::OP_CODEBLOCK: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_CodeBlock>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
	*out << ", ";
	PrintArg(out, IR, Op->ID);
	*out << ", ";
	PrintArg(out, IR, Op->EntryPoint);
	*out << ", ";
	PrintArg(out, IR, Op->GuestEntryOffset);
break;
}
case IROps::OP_BEGINBLOCK: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_BeginBlock>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
break;
}
case IROps::OP_INVALIDATEFLAGS: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_InvalidateFlags>();
	*out << " ";
	PrintArg(out, IR, Op->Flags);
break;
}
case IROps::OP_ENDBLOCK: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_EndBlock>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
break;
}
case IROps::OP_GUESTOPCODE: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_GuestOpcode>();
	*out << " ";
	PrintArg(out, IR, Op->GuestEntryOffset);
break;
}
case IROps::OP_VALIDATECODE: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_ValidateCode>();
	*out << " ";
	PrintArg(out, IR, Op->CodeOriginal);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->CodeLength);
break;
}
case IROps::OP_THREADREMOVECODEENTRY: {
break;
}
case IROps::OP_PROCESSORID: {
break;
}
case IROps::OP_GETROUNDINGMODE: {
break;
}
case IROps::OP_SETROUNDINGMODE: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_SetRoundingMode>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->SetDAZ);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_PUSHROUNDINGMODE: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_PushRoundingMode>();
	*out << " ";
	PrintArg(out, IR, Op->RoundMode);
break;
}
case IROps::OP_POPROUNDINGMODE: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_PopRoundingMode>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
break;
}
case IROps::OP_PRINT: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_Print>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
break;
}
case IROps::OP_PRINTMSG: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_PrintMsg>();
	*out << " ";
	PrintArg(out, IR, Op->Value);
break;
}
case IROps::OP_ALLOCATEGPR: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_AllocateGPR>();
	*out << " ";
	PrintArg(out, IR, Op->ForPair);
break;
}
case IROps::OP_ALLOCATEFPR: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_AllocateFPR>();
	*out << " ";
break;
}
case IROps::OP_ALLOCATEGPRAFTER: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_AllocateGPRAfter>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
break;
}
case IROps::OP_RDRAND: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_RDRAND>();
	*out << " ";
	PrintArg(out, IR, Op->GetReseeded);
break;
}
case IROps::OP_YIELD: {
break;
}
case IROps::OP_WFET: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_WFET>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_MONOBACKPATCHERWRITE: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_MonoBackpatcherWrite>();
	*out << " ";
	PrintArg(out, IR, Op->Size);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_JUMP: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_Jump>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
break;
}
case IROps::OP_CONDJUMP: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_CondJump>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[2]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[3]);
	*out << ", ";
	PrintArg(out, IR, Op->Cond);
	*out << ", ";
	PrintArg(out, IR, Op->CompareSize);
	*out << ", ";
	PrintArg(out, IR, Op->FromNZCV);
break;
}
case IROps::OP_EXITFUNCTION: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_ExitFunction>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Hint);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[2]);
break;
}
case IROps::OP_BREAK: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_Break>();
	*out << " ";
	PrintArg(out, IR, Op->Reason);
break;
}
case IROps::OP_CALLBACKRETURN: {
break;
}
case IROps::OP_SYSCALL: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_Syscall>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[2]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[3]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[4]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[5]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[6]);
break;
}
case IROps::OP_THUNK: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_Thunk>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->ThunkNameHash);
break;
}
case IROps::OP_CPUID: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_CPUID>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[2]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[3]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[4]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[5]);
break;
}
case IROps::OP_XGETBV: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_XGetBV>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[2]);
break;
}
case IROps::OP_COPY: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_Copy>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
break;
}
case IROps::OP_LOADREGISTER: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_LoadRegister>();
	*out << " ";
	PrintArg(out, IR, Op->Reg);
	*out << ", ";
	PrintArg(out, IR, Op->Class);
break;
}
case IROps::OP_LOADPF: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_LoadPF>();
	*out << " ";
break;
}
case IROps::OP_LOADAF: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_LoadAF>();
	*out << " ";
break;
}
case IROps::OP_STOREREGISTER: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_StoreRegister>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
break;
}
case IROps::OP_STOREPF: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_StorePF>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
break;
}
case IROps::OP_STOREAF: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_StoreAF>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
break;
}
case IROps::OP_LOADCONTEXT: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_LoadContext>();
	*out << " ";
	PrintArg(out, IR, Op->Class);
	*out << ", ";
	PrintArg(out, IR, Op->Offset);
break;
}
case IROps::OP_LOADCONTEXTPAIR: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_LoadContextPair>();
	*out << " ";
	PrintArg(out, IR, Op->Class);
	*out << ", ";
	PrintArg(out, IR, Op->Offset);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_STORECONTEXT: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_StoreContext>();
	*out << " ";
	PrintArg(out, IR, Op->Class);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Offset);
break;
}
case IROps::OP_STORECONTEXTPAIR: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_StoreContextPair>();
	*out << " ";
	PrintArg(out, IR, Op->Class);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
	*out << ", ";
	PrintArg(out, IR, Op->Offset);
break;
}
case IROps::OP_LOADCONTEXTINDEXED: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_LoadContextIndexed>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->BaseOffset);
	*out << ", ";
	PrintArg(out, IR, Op->Stride);
	*out << ", ";
	PrintArg(out, IR, Op->Class);
break;
}
case IROps::OP_STORECONTEXTINDEXED: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_StoreContextIndexed>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
	*out << ", ";
	PrintArg(out, IR, Op->BaseOffset);
	*out << ", ";
	PrintArg(out, IR, Op->Stride);
	*out << ", ";
	PrintArg(out, IR, Op->Class);
break;
}
case IROps::OP_FORMCONTEXTADDRESS: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_FormContextAddress>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Stride);
break;
}
case IROps::OP_SPILLREGISTER: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_SpillRegister>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Slot);
	*out << ", ";
	PrintArg(out, IR, Op->Class);
break;
}
case IROps::OP_FILLREGISTER: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_FillRegister>();
	*out << " ";
	PrintArg(out, IR, Op->Slot);
	*out << ", ";
	PrintArg(out, IR, Op->Class);
break;
}
case IROps::OP_LOADNZCV: {
break;
}
case IROps::OP_STORENZCV: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_StoreNZCV>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
break;
}
case IROps::OP_LOADDF: {
break;
}
case IROps::OP_LOADMEM: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_LoadMem>();
	*out << " ";
	PrintArg(out, IR, Op->Class);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
	*out << ", ";
	PrintArg(out, IR, Op->Align);
	*out << ", ";
	PrintArg(out, IR, Op->OffsetType);
	*out << ", ";
	PrintArg(out, IR, Op->OffsetScale);
break;
}
case IROps::OP_LOADMEMPAIR: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_LoadMemPair>();
	*out << " ";
	PrintArg(out, IR, Op->Class);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Offset);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[2]);
break;
}
case IROps::OP_STOREMEM: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_StoreMem>();
	*out << " ";
	PrintArg(out, IR, Op->Class);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[2]);
	*out << ", ";
	PrintArg(out, IR, Op->Align);
	*out << ", ";
	PrintArg(out, IR, Op->OffsetType);
	*out << ", ";
	PrintArg(out, IR, Op->OffsetScale);
break;
}
case IROps::OP_STOREMEMPAIR: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_StoreMemPair>();
	*out << " ";
	PrintArg(out, IR, Op->Class);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[2]);
	*out << ", ";
	PrintArg(out, IR, Op->Offset);
break;
}
case IROps::OP_STOREMEMX87SVEOPTPREDICATE: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_StoreMemX87SVEOptPredicate>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_LOADMEMX87SVEOPTPREDICATE: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_LoadMemX87SVEOptPredicate>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
break;
}
case IROps::OP_LOADMEMTSO: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_LoadMemTSO>();
	*out << " ";
	PrintArg(out, IR, Op->Class);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
	*out << ", ";
	PrintArg(out, IR, Op->Align);
	*out << ", ";
	PrintArg(out, IR, Op->OffsetType);
	*out << ", ";
	PrintArg(out, IR, Op->OffsetScale);
break;
}
case IROps::OP_STOREMEMTSO: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_StoreMemTSO>();
	*out << " ";
	PrintArg(out, IR, Op->Class);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[2]);
	*out << ", ";
	PrintArg(out, IR, Op->Align);
	*out << ", ";
	PrintArg(out, IR, Op->OffsetType);
	*out << ", ";
	PrintArg(out, IR, Op->OffsetScale);
break;
}
case IROps::OP_VLOADVECTORMASKED: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VLoadVectorMasked>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[2]);
	*out << ", ";
	PrintArg(out, IR, Op->OffsetType);
	*out << ", ";
	PrintArg(out, IR, Op->OffsetScale);
break;
}
case IROps::OP_VSTOREVECTORMASKED: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VStoreVectorMasked>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[2]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[3]);
	*out << ", ";
	PrintArg(out, IR, Op->OffsetType);
	*out << ", ";
	PrintArg(out, IR, Op->OffsetScale);
break;
}
case IROps::OP_VLOADVECTORGATHERMASKED: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VLoadVectorGatherMasked>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[2]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[3]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[4]);
	*out << ", ";
	PrintArg(out, IR, Op->VectorIndexElementSize);
	*out << ", ";
	PrintArg(out, IR, Op->OffsetScale);
	*out << ", ";
	PrintArg(out, IR, Op->DataElementOffsetStart);
	*out << ", ";
	PrintArg(out, IR, Op->IndexElementOffsetStart);
	*out << ", ";
	PrintArg(out, IR, Op->AddrSize);
break;
}
case IROps::OP_VLOADVECTORGATHERMASKEDQPS: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VLoadVectorGatherMaskedQPS>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[2]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[3]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[4]);
	*out << ", ";
	PrintArg(out, IR, Op->OffsetScale);
	*out << ", ";
	PrintArg(out, IR, Op->AddrSize);
break;
}
case IROps::OP_VLOADVECTORELEMENT: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VLoadVectorElement>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Index);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_VSTOREVECTORELEMENT: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VStoreVectorElement>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Index);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_VBROADCASTFROMMEM: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VBroadcastFromMem>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
break;
}
case IROps::OP_PUSH: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_Push>();
	*out << " ";
	PrintArg(out, IR, Op->ValueSize);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_PUSHTWO: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_PushTwo>();
	*out << " ";
	PrintArg(out, IR, Op->ValueSize);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[2]);
break;
}
case IROps::OP_RMWHANDLE: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_RMWHandle>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
break;
}
case IROps::OP_POP: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_Pop>();
	*out << " ";
	PrintArg(out, IR, Op->Size);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_POPTWO: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_PopTwo>();
	*out << " ";
	PrintArg(out, IR, Op->Size);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[2]);
break;
}
case IROps::OP_MEMSET: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_MemSet>();
	*out << " ";
	PrintArg(out, IR, Op->IsAtomic);
	*out << ", ";
	PrintArg(out, IR, Op->Size);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[2]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[3]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[4]);
break;
}
case IROps::OP_MEMCPY: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_MemCpy>();
	*out << " ";
	PrintArg(out, IR, Op->IsAtomic);
	*out << ", ";
	PrintArg(out, IR, Op->Size);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[2]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[3]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[4]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[5]);
break;
}
case IROps::OP_CACHELINECLEAR: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_CacheLineClear>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Serialize);
break;
}
case IROps::OP_CACHELINECLEAN: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_CacheLineClean>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
break;
}
case IROps::OP_CACHELINEZERO: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_CacheLineZero>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
break;
}
case IROps::OP_FENCE: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_Fence>();
	*out << " ";
	PrintArg(out, IR, Op->Fence);
break;
}
case IROps::OP_PREFETCH: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_Prefetch>();
	*out << " ";
	PrintArg(out, IR, Op->ForStore);
	*out << ", ";
	PrintArg(out, IR, Op->Stream);
	*out << ", ";
	PrintArg(out, IR, Op->CacheLevel);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
	*out << ", ";
	PrintArg(out, IR, Op->OffsetType);
	*out << ", ";
	PrintArg(out, IR, Op->OffsetScale);
break;
}
case IROps::OP_VSTORENONTEMPORAL: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VStoreNonTemporal>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
	*out << ", ";
	PrintArg(out, IR, Op->Offset);
break;
}
case IROps::OP_VSTORENONTEMPORALPAIR: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VStoreNonTemporalPair>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[2]);
	*out << ", ";
	PrintArg(out, IR, Op->Offset);
break;
}
case IROps::OP_VLOADNONTEMPORAL: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VLoadNonTemporal>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Offset);
break;
}
case IROps::OP_CONTEXTCLEAR: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_ContextClear>();
	*out << " ";
	PrintArg(out, IR, Op->Offset);
	*out << ", ";
	PrintArg(out, IR, Op->Size);
break;
}
case IROps::OP_CAS: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_CAS>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[2]);
break;
}
case IROps::OP_CASPAIR: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_CASPair>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[2]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[3]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[4]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[5]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[6]);
break;
}
case IROps::OP_ATOMICSWAP: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_AtomicSwap>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_ATOMICFETCHADD: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_AtomicFetchAdd>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_ATOMICFETCHSUB: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_AtomicFetchSub>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_ATOMICFETCHAND: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_AtomicFetchAnd>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_ATOMICFETCHCLR: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_AtomicFetchCLR>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_ATOMICFETCHOR: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_AtomicFetchOr>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_ATOMICFETCHXOR: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_AtomicFetchXor>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_ATOMICFETCHNEG: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_AtomicFetchNeg>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
break;
}
case IROps::OP_TELEMETRYSETVALUE: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_TelemetrySetValue>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->TelemetryValueIndex);
break;
}
case IROps::OP_ENTRYPOINTOFFSET: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_EntrypointOffset>();
	*out << " ";
	PrintArg(out, IR, Op->Offset);
break;
}
case IROps::OP_INLINEENTRYPOINTOFFSET: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_InlineEntrypointOffset>();
	*out << " ";
	PrintArg(out, IR, Op->Offset);
break;
}
case IROps::OP_CONSTANT: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_Constant>();
	*out << " ";
	PrintArg(out, IR, Op->Constant);
	*out << ", ";
	PrintArg(out, IR, Op->Pad);
	*out << ", ";
	PrintArg(out, IR, Op->MaxBytes);
break;
}
case IROps::OP_INLINECONSTANT: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_InlineConstant>();
	*out << " ";
	PrintArg(out, IR, Op->Constant);
break;
}
case IROps::OP_CYCLECOUNTER: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_CycleCounter>();
	*out << " ";
	PrintArg(out, IR, Op->SelfSynchronizingLoads);
break;
}
case IROps::OP_NEG: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_Neg>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Cond);
break;
}
case IROps::OP_NOT: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_Not>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
break;
}
case IROps::OP_POPCOUNT: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_Popcount>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
break;
}
case IROps::OP_FINDLSB: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_FindLSB>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
break;
}
case IROps::OP_FINDMSB: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_FindMSB>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
break;
}
case IROps::OP_FINDTRAILINGZEROES: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_FindTrailingZeroes>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
break;
}
case IROps::OP_COUNTLEADINGZEROES: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_CountLeadingZeroes>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
break;
}
case IROps::OP_REV: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_Rev>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
break;
}
case IROps::OP_RBIT: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_Rbit>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
break;
}
case IROps::OP_ADD: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_Add>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_ADC: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_Adc>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_SBB: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_Sbb>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_ADDSHIFT: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_AddShift>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
	*out << ", ";
	PrintArg(out, IR, Op->Shift);
	*out << ", ";
	PrintArg(out, IR, Op->ShiftAmount);
break;
}
case IROps::OP_ADDWITHFLAGS: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_AddWithFlags>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_ADDNZCV: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_AddNZCV>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_SETSMALLNZV: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_SetSmallNZV>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
break;
}
case IROps::OP_CARRYINVERT: {
break;
}
case IROps::OP_AXFLAG: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_AXFlag>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
break;
}
case IROps::OP_PARITY: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_Parity>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Mask);
	*out << ", ";
	PrintArg(out, IR, Op->Invert);
break;
}
case IROps::OP_RMIFNZCV: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_RmifNZCV>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Rotate);
	*out << ", ";
	PrintArg(out, IR, Op->Mask);
break;
}
case IROps::OP_CONDADDNZCV: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_CondAddNZCV>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
	*out << ", ";
	PrintArg(out, IR, Op->Cond);
	*out << ", ";
	PrintArg(out, IR, Op->FalseNZCV);
break;
}
case IROps::OP_CONDSUBNZCV: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_CondSubNZCV>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
	*out << ", ";
	PrintArg(out, IR, Op->Cond);
	*out << ", ";
	PrintArg(out, IR, Op->FalseNZCV);
break;
}
case IROps::OP_ADCWITHFLAGS: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_AdcWithFlags>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_ADCZERO: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_AdcZero>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
break;
}
case IROps::OP_ADCZEROWITHFLAGS: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_AdcZeroWithFlags>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
break;
}
case IROps::OP_SBBWITHFLAGS: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_SbbWithFlags>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_ADCNZCV: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_AdcNZCV>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_SBBNZCV: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_SbbNZCV>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_SUB: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_Sub>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_SUBSHIFT: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_SubShift>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
	*out << ", ";
	PrintArg(out, IR, Op->Shift);
	*out << ", ";
	PrintArg(out, IR, Op->ShiftAmount);
break;
}
case IROps::OP_SUBWITHFLAGS: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_SubWithFlags>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_CMPPAIRZ: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_CmpPairZ>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[2]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[3]);
break;
}
case IROps::OP_SUBNZCV: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_SubNZCV>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_OR: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_Or>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_ORLSHL: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_Orlshl>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
	*out << ", ";
	PrintArg(out, IR, Op->BitShift);
break;
}
case IROps::OP_ORLSHR: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_Orlshr>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
	*out << ", ";
	PrintArg(out, IR, Op->BitShift);
break;
}
case IROps::OP_ORNROR: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_Ornror>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
	*out << ", ";
	PrintArg(out, IR, Op->BitShift);
break;
}
case IROps::OP_XOR: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_Xor>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_XORSHIFT: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_XorShift>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
	*out << ", ";
	PrintArg(out, IR, Op->Shift);
	*out << ", ";
	PrintArg(out, IR, Op->ShiftAmount);
break;
}
case IROps::OP_XORNSHIFT: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_XornShift>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
	*out << ", ";
	PrintArg(out, IR, Op->Shift);
	*out << ", ";
	PrintArg(out, IR, Op->ShiftAmount);
break;
}
case IROps::OP_AND: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_And>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_ANDSHIFT: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_AndShift>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
	*out << ", ";
	PrintArg(out, IR, Op->Shift);
	*out << ", ";
	PrintArg(out, IR, Op->ShiftAmount);
break;
}
case IROps::OP_ANDWITHFLAGS: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_AndWithFlags>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_ANDN: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_Andn>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_TESTNZ: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_TestNZ>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_TESTZ: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_TestZ>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_LSHL: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_Lshl>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_LSHR: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_Lshr>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_ASHR: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_Ashr>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_SHIFTFLAGS: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_ShiftFlags>();
	*out << " ";
	PrintArg(out, IR, Op->Size);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
	*out << ", ";
	PrintArg(out, IR, Op->Shift);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[2]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[3]);
	*out << ", ";
	PrintArg(out, IR, Op->InvertCF);
break;
}
case IROps::OP_ROTATEFLAGS: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_RotateFlags>();
	*out << " ";
	PrintArg(out, IR, Op->Size);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
	*out << ", ";
	PrintArg(out, IR, Op->Left);
break;
}
case IROps::OP_ROR: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_Ror>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_MUL: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_Mul>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_UMUL: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_UMul>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_UMULL: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_UMull>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_SMULL: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_SMull>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_MULH: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_MulH>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_UMULH: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_UMulH>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_BFI: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_Bfi>();
	*out << " ";
	PrintArg(out, IR, Op->Width);
	*out << ", ";
	PrintArg(out, IR, Op->lsb);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_BFXIL: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_Bfxil>();
	*out << " ";
	PrintArg(out, IR, Op->Width);
	*out << ", ";
	PrintArg(out, IR, Op->lsb);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_BFE: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_Bfe>();
	*out << " ";
	PrintArg(out, IR, Op->Width);
	*out << ", ";
	PrintArg(out, IR, Op->lsb);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[0]);
break;
}
case IROps::OP_SBFE: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_Sbfe>();
	*out << " ";
	PrintArg(out, IR, Op->Width);
	*out << ", ";
	PrintArg(out, IR, Op->lsb);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[0]);
break;
}
case IROps::OP_NZCVSELECT: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_NZCVSelect>();
	*out << " ";
	PrintArg(out, IR, Op->Cond);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_NZCVSELECTV: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_NZCVSelectV>();
	*out << " ";
	PrintArg(out, IR, Op->Cond);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_NZCVSELECTINCREMENT: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_NZCVSelectIncrement>();
	*out << " ";
	PrintArg(out, IR, Op->Cond);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_SELECT: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_Select>();
	*out << " ";
	PrintArg(out, IR, Op->CompareSize);
	*out << ", ";
	PrintArg(out, IR, Op->Cond);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[2]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[3]);
break;
}
case IROps::OP_MASKGENERATEFROMBITWIDTH: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_MaskGenerateFromBitWidth>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
break;
}
case IROps::OP_EXTR: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_Extr>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
	*out << ", ";
	PrintArg(out, IR, Op->LSB);
break;
}
case IROps::OP_PDEP: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_PDep>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_PEXT: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_PExt>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_DIV: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_Div>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[2]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[3]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[4]);
break;
}
case IROps::OP_UDIV: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_UDiv>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[2]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[3]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[4]);
break;
}
case IROps::OP_VEXTRACTTOGPR: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VExtractToGPR>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Index);
break;
}
case IROps::OP_FLOAT_TOGPR_S: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_Float_ToGPR_S>();
	*out << " ";
	PrintArg(out, IR, Op->SrcElementSize);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[0]);
break;
}
case IROps::OP_FLOAT_TOGPR_ZS: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_Float_ToGPR_ZS>();
	*out << " ";
	PrintArg(out, IR, Op->SrcElementSize);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[0]);
break;
}
case IROps::OP_FCMP: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_FCmp>();
	*out << " ";
	PrintArg(out, IR, Op->ElementSize);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_VFADDSCALARINSERT: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VFAddScalarInsert>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
	*out << ", ";
	PrintArg(out, IR, Op->ZeroUpperBits);
break;
}
case IROps::OP_VFSUBSCALARINSERT: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VFSubScalarInsert>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
	*out << ", ";
	PrintArg(out, IR, Op->ZeroUpperBits);
break;
}
case IROps::OP_VFMULSCALARINSERT: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VFMulScalarInsert>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
	*out << ", ";
	PrintArg(out, IR, Op->ZeroUpperBits);
break;
}
case IROps::OP_VFDIVSCALARINSERT: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VFDivScalarInsert>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
	*out << ", ";
	PrintArg(out, IR, Op->ZeroUpperBits);
break;
}
case IROps::OP_VFMINSCALARINSERT: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VFMinScalarInsert>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
	*out << ", ";
	PrintArg(out, IR, Op->ZeroUpperBits);
break;
}
case IROps::OP_VFMAXSCALARINSERT: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VFMaxScalarInsert>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
	*out << ", ";
	PrintArg(out, IR, Op->ZeroUpperBits);
break;
}
case IROps::OP_VFSQRTSCALARINSERT: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VFSqrtScalarInsert>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
	*out << ", ";
	PrintArg(out, IR, Op->ZeroUpperBits);
break;
}
case IROps::OP_VFRSQRTSCALARINSERT: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VFRSqrtScalarInsert>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
	*out << ", ";
	PrintArg(out, IR, Op->ZeroUpperBits);
break;
}
case IROps::OP_VFRECPSCALARINSERT: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VFRecpScalarInsert>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
	*out << ", ";
	PrintArg(out, IR, Op->ZeroUpperBits);
break;
}
case IROps::OP_VFTOFSCALARINSERT: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VFToFScalarInsert>();
	*out << " ";
	PrintArg(out, IR, Op->SrcElementSize);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
	*out << ", ";
	PrintArg(out, IR, Op->ZeroUpperBits);
break;
}
case IROps::OP_VSTOFVECTORINSERT: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VSToFVectorInsert>();
	*out << " ";
	PrintArg(out, IR, Op->SrcElementSize);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
	*out << ", ";
	PrintArg(out, IR, Op->HasTwoElements);
	*out << ", ";
	PrintArg(out, IR, Op->ZeroUpperBits);
break;
}
case IROps::OP_VSTOFGPRINSERT: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VSToFGPRInsert>();
	*out << " ";
	PrintArg(out, IR, Op->SrcElementSize);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
	*out << ", ";
	PrintArg(out, IR, Op->ZeroUpperBits);
break;
}
case IROps::OP_VFTOISCALARINSERT: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VFToIScalarInsert>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
	*out << ", ";
	PrintArg(out, IR, Op->Round);
	*out << ", ";
	PrintArg(out, IR, Op->ZeroUpperBits);
break;
}
case IROps::OP_VFCMPSCALARINSERT: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VFCMPScalarInsert>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
	*out << ", ";
	PrintArg(out, IR, Op->Op);
	*out << ", ";
	PrintArg(out, IR, Op->ZeroUpperBits);
break;
}
case IROps::OP_VFMLASCALARINSERT: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VFMLAScalarInsert>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[2]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[3]);
break;
}
case IROps::OP_VFMLSSCALARINSERT: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VFMLSScalarInsert>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[2]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[3]);
break;
}
case IROps::OP_VFNMLASCALARINSERT: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VFNMLAScalarInsert>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[2]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[3]);
break;
}
case IROps::OP_VFNMLSSCALARINSERT: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VFNMLSScalarInsert>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[2]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[3]);
break;
}
case IROps::OP_VFCOPYSIGN: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VFCopySign>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_VMOV: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VMov>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
break;
}
case IROps::OP_VECTORIMM: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VectorImm>();
	*out << " ";
	PrintArg(out, IR, Op->Immediate);
	*out << ", ";
	PrintArg(out, IR, Op->ShiftAmount);
break;
}
case IROps::OP_LOADNAMEDVECTORCONSTANT: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_LoadNamedVectorConstant>();
	*out << " ";
	PrintArg(out, IR, Op->Constant);
break;
}
case IROps::OP_LOADNAMEDVECTORINDEXEDCONSTANT: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_LoadNamedVectorIndexedConstant>();
	*out << " ";
	PrintArg(out, IR, Op->Constant);
	*out << ", ";
	PrintArg(out, IR, Op->Index);
break;
}
case IROps::OP_VNEG: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VNeg>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
break;
}
case IROps::OP_VNOT: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VNot>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
break;
}
case IROps::OP_VABS: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VAbs>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
break;
}
case IROps::OP_VPOPCOUNT: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VPopcount>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
break;
}
case IROps::OP_VADDV: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VAddV>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
break;
}
case IROps::OP_VUMINV: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VUMinV>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
break;
}
case IROps::OP_VUMAXV: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VUMaxV>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
break;
}
case IROps::OP_VFABS: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VFAbs>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
break;
}
case IROps::OP_VFNEG: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VFNeg>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
break;
}
case IROps::OP_VFRECP: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VFRecp>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
break;
}
case IROps::OP_VFRECPPRECISION: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VFRecpPrecision>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
break;
}
case IROps::OP_VFSQRT: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VFSqrt>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
break;
}
case IROps::OP_VFRSQRT: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VFRSqrt>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
break;
}
case IROps::OP_VFRSQRTPRECISION: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VFRSqrtPrecision>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
break;
}
case IROps::OP_VCMPEQZ: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VCMPEQZ>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
break;
}
case IROps::OP_VCMPGTZ: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VCMPGTZ>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
break;
}
case IROps::OP_VCMPLTZ: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VCMPLTZ>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
break;
}
case IROps::OP_VDUPELEMENT: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VDupElement>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Index);
break;
}
case IROps::OP_VSHLI: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VShlI>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->BitShift);
break;
}
case IROps::OP_VUSHRI: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VUShrI>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->BitShift);
break;
}
case IROps::OP_VUSHRAI: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VUShraI>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
	*out << ", ";
	PrintArg(out, IR, Op->BitShift);
break;
}
case IROps::OP_VSSHRI: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VSShrI>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->BitShift);
break;
}
case IROps::OP_VUSHRNI: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VUShrNI>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->BitShift);
break;
}
case IROps::OP_VUSHRNI2: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VUShrNI2>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
	*out << ", ";
	PrintArg(out, IR, Op->BitShift);
break;
}
case IROps::OP_VSXTL: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VSXTL>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
break;
}
case IROps::OP_VSXTL2: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VSXTL2>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
break;
}
case IROps::OP_VSSHLL: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VSSHLL>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->BitShift);
break;
}
case IROps::OP_VSSHLL2: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VSSHLL2>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->BitShift);
break;
}
case IROps::OP_VUXTL: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VUXTL>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
break;
}
case IROps::OP_VUXTL2: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VUXTL2>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
break;
}
case IROps::OP_VSQXTN: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VSQXTN>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
break;
}
case IROps::OP_VSQXTN2: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VSQXTN2>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_VSQXTNPAIR: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VSQXTNPair>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_VSQXTUN: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VSQXTUN>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
break;
}
case IROps::OP_VSQXTUN2: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VSQXTUN2>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_VSQXTUNPAIR: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VSQXTUNPair>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_VSRSHR: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VSRSHR>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->BitShift);
break;
}
case IROps::OP_VSQSHL: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VSQSHL>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->BitShift);
break;
}
case IROps::OP_VREV32: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VRev32>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
break;
}
case IROps::OP_VREV64: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VRev64>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
break;
}
case IROps::OP_VADD: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VAdd>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_VSUB: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VSub>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_VAND: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VAnd>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_VANDN: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VAndn>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_VORN: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VOrn>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_VOR: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VOr>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_VXOR: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VXor>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_VUQADD: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VUQAdd>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_VUQSUB: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VUQSub>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_VSQADD: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VSQAdd>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_VSQSUB: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VSQSub>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_VADDP: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VAddP>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_VURAVG: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VURAvg>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_VUMIN: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VUMin>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_VUMAX: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VUMax>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_VSMIN: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VSMin>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_VSMAX: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VSMax>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_VZIP: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VZip>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_VZIP2: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VZip2>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_VUNZIP: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VUnZip>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_VUNZIP2: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VUnZip2>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_VTRN: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VTrn>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_VTRN2: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VTrn2>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_VFADD: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VFAdd>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_VFADDP: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VFAddP>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_VFADDV: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VFAddV>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
break;
}
case IROps::OP_VFSUB: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VFSub>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_VFMUL: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VFMul>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_VFDIV: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VFDiv>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_VFMIN: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VFMin>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_VFMAX: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VFMax>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_VMUL: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VMul>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_VUMULL: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VUMull>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_VSMULL: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VSMull>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_VUMULL2: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VUMull2>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_VSMULL2: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VSMull2>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_VUMULH: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VUMulH>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_VSMULH: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VSMulH>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_VUABDL: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VUABDL>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_VUABDL2: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VUABDL2>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_VUSHL: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VUShl>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
	*out << ", ";
	PrintArg(out, IR, Op->RangeCheck);
break;
}
case IROps::OP_VUSHR: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VUShr>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
	*out << ", ";
	PrintArg(out, IR, Op->RangeCheck);
break;
}
case IROps::OP_VSSHR: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VSShr>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
	*out << ", ";
	PrintArg(out, IR, Op->RangeCheck);
break;
}
case IROps::OP_VUSHLS: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VUShlS>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_VUSHRS: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VUShrS>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_VSSHRS: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VSShrS>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_VUSHRSWIDE: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VUShrSWide>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_VSSHRSWIDE: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VSShrSWide>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_VUSHLSWIDE: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VUShlSWide>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_VINSELEMENT: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VInsElement>();
	*out << " ";
	PrintArg(out, IR, Op->DestIdx);
	*out << ", ";
	PrintArg(out, IR, Op->SrcIdx);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_VINSGPR: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VInsGPR>();
	*out << " ";
	PrintArg(out, IR, Op->DestIdx);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_VEXTR: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VExtr>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
	*out << ", ";
	PrintArg(out, IR, Op->Index);
break;
}
case IROps::OP_VCMPEQ: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VCMPEQ>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_VCMPGT: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VCMPGT>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_VFCMPEQ: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VFCMPEQ>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_VFCMPNEQ: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VFCMPNEQ>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_VFCMPLT: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VFCMPLT>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_VFCMPGT: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VFCMPGT>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_VFCMPLE: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VFCMPLE>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_VFCMPORD: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VFCMPORD>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_VFCMPUNO: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VFCMPUNO>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_VTBL1: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VTBL1>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_VTBL2: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VTBL2>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[2]);
break;
}
case IROps::OP_VTBX1: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VTBX1>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[2]);
break;
}
case IROps::OP_VBSL: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VBSL>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[2]);
break;
}
case IROps::OP_VPCMPESTRX: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VPCMPESTRX>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[2]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[3]);
	*out << ", ";
	PrintArg(out, IR, Op->Control);
break;
}
case IROps::OP_VPCMPISTRX: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VPCMPISTRX>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
	*out << ", ";
	PrintArg(out, IR, Op->Control);
break;
}
case IROps::OP_VFCADD: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VFCADD>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
	*out << ", ";
	PrintArg(out, IR, Op->Rotate);
break;
}
case IROps::OP_VFMLA: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VFMLA>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[2]);
break;
}
case IROps::OP_VFMLS: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VFMLS>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[2]);
break;
}
case IROps::OP_VFNMLA: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VFNMLA>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[2]);
break;
}
case IROps::OP_VFNMLS: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VFNMLS>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[2]);
break;
}
case IROps::OP_VCASTFROMGPR: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VCastFromGPR>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
break;
}
case IROps::OP_VDUPFROMGPR: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VDupFromGPR>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
break;
}
case IROps::OP_VLOADTWOGPRS: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VLoadTwoGPRs>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_FLOAT_FROMGPR_S: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_Float_FromGPR_S>();
	*out << " ";
	PrintArg(out, IR, Op->SrcElementSize);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[0]);
break;
}
case IROps::OP_FLOAT_FTOF: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_Float_FToF>();
	*out << " ";
	PrintArg(out, IR, Op->SrcElementSize);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[0]);
break;
}
case IROps::OP_VECTOR_STOF: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_Vector_SToF>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
break;
}
case IROps::OP_VECTOR_FTOS: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_Vector_FToS>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
break;
}
case IROps::OP_VECTOR_FTOZS: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_Vector_FToZS>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
break;
}
case IROps::OP_VECTOR_FTOF: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_Vector_FToF>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->SrcElementSize);
break;
}
case IROps::OP_VFCVTL2: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VFCVTL2>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
break;
}
case IROps::OP_VFCVTN2: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VFCVTN2>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_VECTOR_FTOI: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_Vector_FToI>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Round);
break;
}
case IROps::OP_VECTOR_FTOISIZED: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_Vector_FToISized>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->HostRound);
	*out << ", ";
	PrintArg(out, IR, Op->IntSize);
break;
}
case IROps::OP_VECTOR_F64TOI32: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_Vector_F64ToI32>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Round);
	*out << ", ";
	PrintArg(out, IR, Op->EnsureZeroUpperHalf);
break;
}
case IROps::OP_VAESIMC: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VAESImc>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
break;
}
case IROps::OP_VAESENC: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VAESEnc>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[2]);
break;
}
case IROps::OP_VAESENCLAST: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VAESEncLast>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[2]);
break;
}
case IROps::OP_VAESDEC: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VAESDec>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[2]);
break;
}
case IROps::OP_VAESDECLAST: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VAESDecLast>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[2]);
break;
}
case IROps::OP_VAESKEYGENASSIST: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VAESKeyGenAssist>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[2]);
	*out << ", ";
	PrintArg(out, IR, Op->RCON);
break;
}
case IROps::OP_VSHA1H: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VSha1H>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
break;
}
case IROps::OP_VSHA1C: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VSha1C>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[2]);
break;
}
case IROps::OP_VSHA1M: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VSha1M>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[2]);
break;
}
case IROps::OP_VSHA1P: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VSha1P>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[2]);
break;
}
case IROps::OP_VSHA1SU1: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VSha1SU1>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_VSHA256U0: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VSha256U0>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_VSHA256U1: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VSha256U1>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_VSHA256H: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VSha256H>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[2]);
break;
}
case IROps::OP_VSHA256H2: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_VSha256H2>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[2]);
break;
}
case IROps::OP_CRC32: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_CRC32>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
	*out << ", ";
	PrintArg(out, IR, Op->SrcSize);
break;
}
case IROps::OP_PCLMUL: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_PCLMUL>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
	*out << ", ";
	PrintArg(out, IR, Op->Selector);
break;
}
case IROps::OP_F64ATAN: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_F64ATAN>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_F64FPREM: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_F64FPREM>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_F64FPREM1: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_F64FPREM1>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_F64SCALE: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_F64SCALE>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_F64F2XM1: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_F64F2XM1>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
break;
}
case IROps::OP_F64FYL2X: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_F64FYL2X>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_F64FYL2XP1: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_F64FYL2XP1>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_F64TAN: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_F64TAN>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
break;
}
case IROps::OP_F64SIN: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_F64SIN>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
break;
}
case IROps::OP_F64COS: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_F64COS>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
break;
}
case IROps::OP_F64SINCOS: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_F64SINCOS>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[2]);
break;
}
case IROps::OP_SYNCSTACKTOSLOW: {
break;
}
case IROps::OP_STACKFORCESLOW: {
break;
}
case IROps::OP_INITSTACK: {
break;
}
case IROps::OP_INCSTACKTOP: {
break;
}
case IROps::OP_DECSTACKTOP: {
break;
}
case IROps::OP_INVALIDATESTACK: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_InvalidateStack>();
	*out << " ";
	PrintArg(out, IR, Op->StackLocation);
break;
}
case IROps::OP_PUSHSTACK: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_PushStack>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
	*out << ", ";
	PrintArg(out, IR, Op->LoadSize);
break;
}
case IROps::OP_COPYPUSHSTACK: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_CopyPushStack>();
	*out << " ";
	PrintArg(out, IR, Op->StackLocation);
break;
}
case IROps::OP_STORESTACKMEM: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_StoreStackMem>();
	*out << " ";
	PrintArg(out, IR, Op->SourceSize);
	*out << ", ";
	PrintArg(out, IR, Op->StoreSize);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
	*out << ", ";
	PrintArg(out, IR, Op->Align);
	*out << ", ";
	PrintArg(out, IR, Op->OffsetType);
	*out << ", ";
	PrintArg(out, IR, Op->OffsetScale);
break;
}
case IROps::OP_STORESTACKTOSTACK: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_StoreStackToStack>();
	*out << " ";
	PrintArg(out, IR, Op->StackLocation);
break;
}
case IROps::OP_POPSTACKDESTROY: {
break;
}
case IROps::OP_READSTACKVALUE: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_ReadStackValue>();
	*out << " ";
	PrintArg(out, IR, Op->StackLocation);
break;
}
case IROps::OP_STACKVALIDTAG: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_StackValidTag>();
	*out << " ";
	PrintArg(out, IR, Op->StackLocation);
break;
}
case IROps::OP_F80ADDSTACK: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_F80AddStack>();
	*out << " ";
	PrintArg(out, IR, Op->SrcStack1);
	*out << ", ";
	PrintArg(out, IR, Op->SrcStack2);
break;
}
case IROps::OP_F80ADDVALUE: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_F80AddValue>();
	*out << " ";
	PrintArg(out, IR, Op->SrcStack);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[0]);
break;
}
case IROps::OP_F80ADD: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_F80Add>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_F80SUBSTACK: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_F80SubStack>();
	*out << " ";
	PrintArg(out, IR, Op->DstStack);
	*out << ", ";
	PrintArg(out, IR, Op->SrcStack1);
	*out << ", ";
	PrintArg(out, IR, Op->SrcStack2);
break;
}
case IROps::OP_F80SUBVALUE: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_F80SubValue>();
	*out << " ";
	PrintArg(out, IR, Op->SrcStack);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[0]);
break;
}
case IROps::OP_F80SUBRVALUE: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_F80SubRValue>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->SrcStack);
break;
}
case IROps::OP_F80SUB: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_F80Sub>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_F80MULSTACK: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_F80MulStack>();
	*out << " ";
	PrintArg(out, IR, Op->SrcStack1);
	*out << ", ";
	PrintArg(out, IR, Op->SrcStack2);
break;
}
case IROps::OP_F80MULVALUE: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_F80MulValue>();
	*out << " ";
	PrintArg(out, IR, Op->SrcStack);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[0]);
break;
}
case IROps::OP_F80MUL: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_F80Mul>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_F80DIVSTACK: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_F80DivStack>();
	*out << " ";
	PrintArg(out, IR, Op->DstStack);
	*out << ", ";
	PrintArg(out, IR, Op->SrcStack1);
	*out << ", ";
	PrintArg(out, IR, Op->SrcStack2);
break;
}
case IROps::OP_F80DIVVALUE: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_F80DivValue>();
	*out << " ";
	PrintArg(out, IR, Op->SrcStack);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[0]);
break;
}
case IROps::OP_F80DIVRVALUE: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_F80DivRValue>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->SrcStack);
break;
}
case IROps::OP_F80DIV: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_F80Div>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_F80STACKXCHANGE: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_F80StackXchange>();
	*out << " ";
	PrintArg(out, IR, Op->SrcStack);
break;
}
case IROps::OP_F80STACKCHANGESIGN: {
break;
}
case IROps::OP_F80STACKABS: {
break;
}
case IROps::OP_F80PTANSTACK: {
break;
}
case IROps::OP_F80ATANSTACK: {
break;
}
case IROps::OP_F80ATAN: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_F80ATAN>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_F80FPREMSTACK: {
break;
}
case IROps::OP_F80FPREM: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_F80FPREM>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_F80FPREM1STACK: {
break;
}
case IROps::OP_F80FPREM1: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_F80FPREM1>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_F80SCALESTACK: {
break;
}
case IROps::OP_F80SCALE: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_F80SCALE>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_F80CVT: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_F80CVT>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
break;
}
case IROps::OP_F80CVTINT: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_F80CVTInt>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Truncate);
break;
}
case IROps::OP_F80CVTTO: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_F80CVTTo>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->SrcSize);
break;
}
case IROps::OP_F80CVTTOINT: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_F80CVTToInt>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->SrcSize);
break;
}
case IROps::OP_F80ROUNDSTACK: {
break;
}
case IROps::OP_F80ROUND: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_F80Round>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
break;
}
case IROps::OP_F80F2XM1STACK: {
break;
}
case IROps::OP_F80F2XM1: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_F80F2XM1>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
break;
}
case IROps::OP_F80TAN: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_F80TAN>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
break;
}
case IROps::OP_F80SINSTACK: {
break;
}
case IROps::OP_F80SIN: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_F80SIN>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
break;
}
case IROps::OP_F80COSSTACK: {
break;
}
case IROps::OP_F80COS: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_F80COS>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
break;
}
case IROps::OP_F80SINCOS: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_F80SINCOS>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[2]);
break;
}
case IROps::OP_F80SINCOSSTACK: {
break;
}
case IROps::OP_F80SQRTSTACK: {
break;
}
case IROps::OP_F80SQRT: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_F80SQRT>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
break;
}
case IROps::OP_F80XTRACT_EXP: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_F80XTRACT_EXP>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
break;
}
case IROps::OP_F80XTRACT_SIG: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_F80XTRACT_SIG>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
break;
}
case IROps::OP_F80STACKTEST: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_F80StackTest>();
	*out << " ";
	PrintArg(out, IR, Op->SrcStack);
break;
}
case IROps::OP_F80CMPSTACK: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_F80CmpStack>();
	*out << " ";
	PrintArg(out, IR, Op->SrcStack);
break;
}
case IROps::OP_F80CMPVALUE: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_F80CmpValue>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
break;
}
case IROps::OP_F80CMP: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_F80Cmp>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_F80BCDLOAD: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_F80BCDLoad>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
break;
}
case IROps::OP_F80BCDSTORE: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_F80BCDStore>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
break;
}
case IROps::OP_F80FYL2XSTACK: {
break;
}
case IROps::OP_F80FYL2X: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_F80FYL2X>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_F80FYL2XP1STACK: {
break;
}
case IROps::OP_F80FYL2XP1: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_F80FYL2XP1>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->Header.Args[1]);
break;
}
case IROps::OP_F80VBSLSTACK: {
	[[maybe_unused]] auto Op = IROp->C<IR::IROp_F80VBSLStack>();
	*out << " ";
	PrintArg(out, IR, Op->Header.Args[0]);
	*out << ", ";
	PrintArg(out, IR, Op->SrcStack1);
	*out << ", ";
	PrintArg(out, IR, Op->SrcStack2);
break;
}
case IROps::OP_LAST: {
break;
}
#undef IROP_ARGPRINTER_HELPER
#endif
#ifdef IROP_ALLOCATE_HELPERS
	template <class T>
	struct Wrapper final {
		T *first;
		OrderedNode *Node; ///< Actual offset of this IR in ths list

		operator Wrapper<IROp_Header>() const { return Wrapper<IROp_Header> {reinterpret_cast<IROp_Header*>(first), Node}; }
		operator OrderedNode *() { return Node; }
		operator const OrderedNode *() const { return Node; }
		operator OpNodeWrapper () const { return Node->Header.Value; }
	};
	template <class T>
	using IRPair = Wrapper<T>;

	IRPair<IROp_Header> AllocateRawOp(size_t HeaderSize) {
		auto Op = reinterpret_cast<IROp_Header*>(DualListData.DataAllocate(HeaderSize));
		memset(Op, 0, HeaderSize);
		Op->Op = IROps::OP_DUMMY;
		return IRPair<IROp_Header>{Op, CreateNode(Op)};
	}

	template<class T, IROps T2>
	T *AllocateOrphanOp() {
		size_t Size = FEXCore::IR::GetSize(T2);
		auto Op = reinterpret_cast<T*>(DualListData.DataAllocate(Size));
		memset(Op, 0, Size);
		Op->Header.Op = T2;
		return Op;
	}

	template<class T, IROps T2>
	IRPair<T> AllocateOp() {
		size_t Size = FEXCore::IR::GetSize(T2);
		auto Op = reinterpret_cast<T*>(DualListData.DataAllocate(Size));
		memset(Op, 0, Size);
		Op->Header.Op = T2;
		return IRPair<T>{Op, CreateNode(&Op->Header)};
	}

	IR::OpSize GetOpSize(const OrderedNode *Op) const {
		auto HeaderOp = Op->Header.Value.GetNode(DualListData.DataBegin());
		return HeaderOp->Size;
	}

	IR::OpSize GetOpElementSize(const OrderedNode *Op) const {
		auto HeaderOp = Op->Header.Value.GetNode(DualListData.DataBegin());
		return HeaderOp->ElementSize;
	}

	uint8_t GetOpElements(const OrderedNode *Op) const {
		LOGMAN_THROW_A_FMT(OpHasDest(Op), "Op {} has no dest\n", GetOpName(Op));
		return IR::OpSizeToSize(GetOpSize(Op)) / IR::OpSizeToSize(GetOpElementSize(Op));
	}

	bool OpHasDest(const OrderedNode *Op) const {
		auto HeaderOp = Op->Header.Value.GetNode(DualListData.DataBegin());
		return GetHasDest(HeaderOp->Op);
	}

	IROps GetOpType(const OrderedNode *Op) const {
		auto HeaderOp = Op->Header.Value.GetNode(DualListData.DataBegin());
		return HeaderOp->Op;
	}

	FEXCore::IR::RegClass GetOpRegClass(const OrderedNode *Op) const {
		return GetRegClass(GetOpType(Op));
	}

	std::string_view const& GetOpName(const OrderedNode *Op) const {
		return IR::GetName(GetOpType(Op));
	}

	IRPair<IROp_Dummy> _Dummy() {
		auto _Op = AllocateOp<IROp_Dummy, IROps::OP_DUMMY>();
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	IRPair<IROp_IRHeader> _IRHeader(OrderedNodeWrapper Blocks, uint64_t OriginalRIP, uint32_t BlockCount, uint32_t NumHostInstructions, uint32_t SpillSlots, bool PostRA = false, bool HasX87 = false, bool ReadsParity = false) {
		auto _Op = AllocateOp<IROp_IRHeader, IROps::OP_IRHEADER>();
		_Op.first->Blocks = Blocks;
		_Op.first->OriginalRIP = OriginalRIP;
		_Op.first->BlockCount = BlockCount;
		_Op.first->NumHostInstructions = NumHostInstructions;
		_Op.first->SpillSlots = SpillSlots;
		_Op.first->PostRA = PostRA;
		_Op.first->HasX87 = HasX87;
		_Op.first->ReadsParity = ReadsParity;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	IRPair<IROp_IRHeader> _IRHeader(OrderedNode *Blocks, uint64_t OriginalRIP, uint32_t BlockCount, uint32_t NumHostInstructions, uint32_t SpillSlots, bool PostRA = false, bool HasX87 = false, bool ReadsParity = false) {
		auto ListDataBegin = DualListData.ListBegin();
		Blocks->AddUse();
		return _IRHeader(Blocks->Wrapped(ListDataBegin), OriginalRIP, BlockCount, NumHostInstructions, SpillSlots, PostRA, HasX87, ReadsParity);
	}

	IRPair<IROp_CodeBlock> _CodeBlock(OrderedNodeWrapper Begin, OrderedNodeWrapper Last, uint32_t ID, bool EntryPoint = false, uint32_t GuestEntryOffset = 0) {
		auto _Op = AllocateOp<IROp_CodeBlock, IROps::OP_CODEBLOCK>();
		_Op.first->Begin = Begin;
		_Op.first->Last = Last;
		_Op.first->ID = ID;
		_Op.first->EntryPoint = EntryPoint;
		_Op.first->GuestEntryOffset = GuestEntryOffset;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	IRPair<IROp_CodeBlock> _CodeBlock(OrderedNode *Begin, OrderedNode *Last, uint32_t ID, bool EntryPoint = false, uint32_t GuestEntryOffset = 0) {
		auto ListDataBegin = DualListData.ListBegin();
		Begin->AddUse();
		Last->AddUse();
		return _CodeBlock(Begin->Wrapped(ListDataBegin), Last->Wrapped(ListDataBegin), ID, EntryPoint, GuestEntryOffset);
	}

	IRPair<IROp_BeginBlock> _BeginBlock(OrderedNodeWrapper BlockHeader) {
		auto _Op = AllocateOp<IROp_BeginBlock, IROps::OP_BEGINBLOCK>();
		_Op.first->BlockHeader = BlockHeader;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	IRPair<IROp_BeginBlock> _BeginBlock(OrderedNode *BlockHeader) {
		auto ListDataBegin = DualListData.ListBegin();
		BlockHeader->AddUse();
		return _BeginBlock(BlockHeader->Wrapped(ListDataBegin));
	}

	IRPair<IROp_InvalidateFlags> _InvalidateFlags(uint64_t Flags) {
		auto _Op = AllocateOp<IROp_InvalidateFlags, IROps::OP_INVALIDATEFLAGS>();
		_Op.first->Flags = Flags;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	IRPair<IROp_EndBlock> _EndBlock(OrderedNodeWrapper BlockHeader) {
		auto _Op = AllocateOp<IROp_EndBlock, IROps::OP_ENDBLOCK>();
		_Op.first->BlockHeader = BlockHeader;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	IRPair<IROp_EndBlock> _EndBlock(OrderedNode *BlockHeader) {
		auto ListDataBegin = DualListData.ListBegin();
		BlockHeader->AddUse();
		return _EndBlock(BlockHeader->Wrapped(ListDataBegin));
	}

	/// Marks the beginning of a guest opcode
	IRPair<IROp_GuestOpcode> _GuestOpcode(uint32_t GuestEntryOffset) {
		auto _Op = AllocateOp<IROp_GuestOpcode, IROps::OP_GUESTOPCODE>();
		_Op.first->GuestEntryOffset = GuestEntryOffset;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	IRPair<IROp_ValidateCode> _ValidateCode(std::array<uint8_t, 16> CodeOriginal, OrderedNodeWrapper Address, uint8_t CodeLength) {
		auto _Op = AllocateOp<IROp_ValidateCode, IROps::OP_VALIDATECODE>();
		_Op.first->Address = Address;
		_Op.first->CodeOriginal = CodeOriginal;
		_Op.first->CodeLength = CodeLength;
		_Op.first->Header.Size = OpSize::i64Bit;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	IRPair<IROp_ValidateCode> _ValidateCode(std::array<uint8_t, 16> CodeOriginal, OrderedNode *Address, uint8_t CodeLength) {
		auto ListDataBegin = DualListData.ListBegin();
		Address->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Address) == RegClass::Invalid || WalkFindRegClass(Address) == RegClass::GPR, "GetOpRegClass(Address) == RegClass::Invalid || WalkFindRegClass(Address) == RegClass::GPR");
#endif
		return _ValidateCode(CodeOriginal, Address->Wrapped(ListDataBegin), CodeLength);
	}

	IRPair<IROp_ThreadRemoveCodeEntry> _ThreadRemoveCodeEntry() {
		auto _Op = AllocateOp<IROp_ThreadRemoveCodeEntry, IROps::OP_THREADREMOVECODEENTRY>();
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Returns the processor ID correlating to the current running CPU
	///
	/// This may be out of date by time this instruction is executed so care must be taken
	///
	/// This same information can be gotten from syscall getcpu(&cpu, &node)
	///
	/// uint32_t Res = (node << 12) | cpu;
	///
	/// This means it has a limitation of 4096 CPU cores. Which is fine and matches x86 behaviour
	IRPair<IROp_ProcessorID> _ProcessorID() {
		auto _Op = AllocateOp<IROp_ProcessorID, IROps::OP_PROCESSORID>();
		_Op.first->Header.Size = OpSize::i64Bit;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Gets the current rounding mode options
	IRPair<IROp_GetRoundingMode> _GetRoundingMode() {
		auto _Op = AllocateOp<IROp_GetRoundingMode, IROps::OP_GETROUNDINGMODE>();
		_Op.first->Header.Size = OpSize::i32Bit;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Sets the current rounding mode options for the thread
	IRPair<IROp_SetRoundingMode> _SetRoundingMode(OrderedNodeWrapper RoundMode, bool SetDAZ, OrderedNodeWrapper MXCSR) {
		auto _Op = AllocateOp<IROp_SetRoundingMode, IROps::OP_SETROUNDINGMODE>();
		_Op.first->RoundMode = RoundMode;
		_Op.first->MXCSR = MXCSR;
		_Op.first->SetDAZ = SetDAZ;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Sets the current rounding mode options for the thread
	IRPair<IROp_SetRoundingMode> _SetRoundingMode(OrderedNode *RoundMode, bool SetDAZ, OrderedNode *MXCSR) {
		auto ListDataBegin = DualListData.ListBegin();
		RoundMode->AddUse();
		MXCSR->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(RoundMode) == RegClass::Invalid || WalkFindRegClass(RoundMode) == RegClass::GPR, "GetOpRegClass(RoundMode) == RegClass::Invalid || WalkFindRegClass(RoundMode) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(MXCSR) == RegClass::Invalid || WalkFindRegClass(MXCSR) == RegClass::GPR, "GetOpRegClass(MXCSR) == RegClass::Invalid || WalkFindRegClass(MXCSR) == RegClass::GPR");
#endif
		return _SetRoundingMode(RoundMode->Wrapped(ListDataBegin), SetDAZ, MXCSR->Wrapped(ListDataBegin));
	}

	/// Override the current rounding mode options for the thread, returning old FPCR
	IRPair<IROp_PushRoundingMode> _PushRoundingMode(uint8_t RoundMode) {
		auto _Op = AllocateOp<IROp_PushRoundingMode, IROps::OP_PUSHROUNDINGMODE>();
		_Op.first->RoundMode = RoundMode;
		_Op.first->Header.Size = OpSize::i64Bit;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Resets rounding mode after PushRoundingMode operation
	IRPair<IROp_PopRoundingMode> _PopRoundingMode(OrderedNodeWrapper FPCR) {
		auto _Op = AllocateOp<IROp_PopRoundingMode, IROps::OP_POPROUNDINGMODE>();
		_Op.first->FPCR = FPCR;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Resets rounding mode after PushRoundingMode operation
	IRPair<IROp_PopRoundingMode> _PopRoundingMode(OrderedNode *FPCR) {
		auto ListDataBegin = DualListData.ListBegin();
		FPCR->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(FPCR) == RegClass::Invalid || WalkFindRegClass(FPCR) == RegClass::GPR, "GetOpRegClass(FPCR) == RegClass::Invalid || WalkFindRegClass(FPCR) == RegClass::GPR");
#endif
		return _PopRoundingMode(FPCR->Wrapped(ListDataBegin));
	}

	/// Debug operation that prints an SSA value to the console
	///
	/// May only print 64bits of the value
	IRPair<IROp_Print> _Print(OrderedNodeWrapper Value) {
		auto _Op = AllocateOp<IROp_Print, IROps::OP_PRINT>();
		_Op.first->Value = Value;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Debug operation that prints an SSA value to the console
	///
	/// May only print 64bits of the value
	IRPair<IROp_Print> _Print(OrderedNode *Value) {
		auto ListDataBegin = DualListData.ListBegin();
		Value->AddUse();
		return _Print(Value->Wrapped(ListDataBegin));
	}

	/// Debug operation that prints an string to the console.
	///
	/// This is for debug only! Will break code caching!
	IRPair<IROp_PrintMsg> _PrintMsg(const char* Value) {
		auto _Op = AllocateOp<IROp_PrintMsg, IROps::OP_PRINTMSG>();
		_Op.first->Value = Value;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Silly pseudo-instruction to allocate a register for a future destination
	///
	/// Note: if an instruction uses allocated destinations-as-sources,
	///
	/// it cannot use a regular destination too. This ensures RA correctness.
	///
	/// This is a kludge to deal with the IR's lack of multiple destinations
	///
	/// If ForPair is set, RA will try to allocate the base of a register pair
	IRPair<IROp_AllocateGPR> _AllocateGPR(bool ForPair) {
		auto _Op = AllocateOp<IROp_AllocateGPR, IROps::OP_ALLOCATEGPR>();
		_Op.first->ForPair = ForPair;
		_Op.first->Header.Size = OpSize::i64Bit;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Like AllocateGPR, but for FPR
	IRPair<IROp_AllocateFPR> _AllocateFPR(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize) {
		auto _Op = AllocateOp<IROp_AllocateFPR, IROps::OP_ALLOCATEFPR>();
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	/// Silly pseudo-instruction to allocate a register for a future destination
	///
	/// This is a kludge to deal with the IR's lack of multiple destinations
	///
	/// RA will attempt to allocate to the register after $After.
	///
	/// It may not succeed.
	IRPair<IROp_AllocateGPRAfter> _AllocateGPRAfter(OrderedNodeWrapper After) {
		auto _Op = AllocateOp<IROp_AllocateGPRAfter, IROps::OP_ALLOCATEGPRAFTER>();
		_Op.first->After = After;
		_Op.first->Header.Size = OpSize::i64Bit;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Silly pseudo-instruction to allocate a register for a future destination
	///
	/// This is a kludge to deal with the IR's lack of multiple destinations
	///
	/// RA will attempt to allocate to the register after $After.
	///
	/// It may not succeed.
	IRPair<IROp_AllocateGPRAfter> _AllocateGPRAfter(OrderedNode *After) {
		auto ListDataBegin = DualListData.ListBegin();
		After->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(After) == RegClass::Invalid || WalkFindRegClass(After) == RegClass::GPR, "GetOpRegClass(After) == RegClass::Invalid || WalkFindRegClass(After) == RegClass::GPR");
#endif
		return _AllocateGPRAfter(After->Wrapped(ListDataBegin));
	}

	/// Uses the hardware random number generator to generate a 64bit number
	///
	/// The boolean argument asks if we should be reading the reseeded number or not
	///
	/// Reseeded RNG calculation is more expensive and will be heavier to use
	///
	/// Returns the 64-bit number
	///
	/// Sets the Z flag if the number is valid.
	///
	/// RNG hardware is allowed to fail early and return. Software must always check this
	IRPair<IROp_RDRAND> _RDRAND(bool GetReseeded) {
		auto _Op = AllocateOp<IROp_RDRAND, IROps::OP_RDRAND>();
		_Op.first->GetReseeded = GetReseeded;
		_Op.first->Header.Size = OpSize::i64Bit;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// This is a hint instruction that the CPU is likely to do a spin so it might want to pause to help out SMP
	///
	/// Can be implemented as a NOP if necessary
	IRPair<IROp_Yield> _Yield() {
		auto _Op = AllocateOp<IROp_Yield, IROps::OP_YIELD>();
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Implement a low power wait attempting to sleep until RDTSC >= Upper:Lower.
	///
	/// Will spuriously wake up.
	IRPair<IROp_WFET> _WFET(OrderedNodeWrapper Upper, OrderedNodeWrapper Lower) {
		auto _Op = AllocateOp<IROp_WFET, IROps::OP_WFET>();
		_Op.first->Upper = Upper;
		_Op.first->Lower = Lower;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Implement a low power wait attempting to sleep until RDTSC >= Upper:Lower.
	///
	/// Will spuriously wake up.
	IRPair<IROp_WFET> _WFET(OrderedNode *Upper, OrderedNode *Lower) {
		auto ListDataBegin = DualListData.ListBegin();
		Upper->AddUse();
		Lower->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Upper) == RegClass::Invalid || WalkFindRegClass(Upper) == RegClass::GPR, "GetOpRegClass(Upper) == RegClass::Invalid || WalkFindRegClass(Upper) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Lower) == RegClass::Invalid || WalkFindRegClass(Lower) == RegClass::GPR, "GetOpRegClass(Lower) == RegClass::Invalid || WalkFindRegClass(Lower) == RegClass::GPR");
#endif
		return _WFET(Upper->Wrapped(ListDataBegin), Lower->Wrapped(ListDataBegin));
	}

	/// Writes and invalidates the target address with the invalidation mutex locked. This is a fault-avoiding
	///
	/// replacement for the atomic SMC writes used in the mono callsite backpatcher.
	IRPair<IROp_MonoBackpatcherWrite> _MonoBackpatcherWrite(FEXCore::IR::OpSize Size, OrderedNodeWrapper Value, OrderedNodeWrapper Addr) {
		auto _Op = AllocateOp<IROp_MonoBackpatcherWrite, IROps::OP_MONOBACKPATCHERWRITE>();
		_Op.first->Value = Value;
		_Op.first->Addr = Addr;
		_Op.first->Size = Size;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Writes and invalidates the target address with the invalidation mutex locked. This is a fault-avoiding
	///
	/// replacement for the atomic SMC writes used in the mono callsite backpatcher.
	IRPair<IROp_MonoBackpatcherWrite> _MonoBackpatcherWrite(FEXCore::IR::OpSize Size, OrderedNode *Value, OrderedNode *Addr) {
		auto ListDataBegin = DualListData.ListBegin();
		Value->AddUse();
		Addr->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Value) == RegClass::Invalid || WalkFindRegClass(Value) == RegClass::GPR, "GetOpRegClass(Value) == RegClass::Invalid || WalkFindRegClass(Value) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Addr) == RegClass::Invalid || WalkFindRegClass(Addr) == RegClass::GPR, "GetOpRegClass(Addr) == RegClass::Invalid || WalkFindRegClass(Addr) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit, "Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit");
#endif
		return _MonoBackpatcherWrite(Size, Value->Wrapped(ListDataBegin), Addr->Wrapped(ListDataBegin));
	}

	IRPair<IROp_Jump> _Jump(OrderedNodeWrapper TargetBlock) {
		auto _Op = AllocateOp<IROp_Jump, IROps::OP_JUMP>();
		_Op.first->TargetBlock = TargetBlock;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	IRPair<IROp_Jump> _Jump(OrderedNode *TargetBlock) {
		auto ListDataBegin = DualListData.ListBegin();
		TargetBlock->AddUse();
		return _Jump(TargetBlock->Wrapped(ListDataBegin));
	}

	IRPair<IROp_CondJump> _CondJump(OrderedNodeWrapper Cmp1, OrderedNodeWrapper Cmp2, OrderedNodeWrapper TrueBlock, OrderedNodeWrapper FalseBlock, CondClass Cond = CondClass::NEQ, FEXCore::IR::OpSize CompareSize = OpSize::iInvalid, bool FromNZCV = false) {
		auto _Op = AllocateOp<IROp_CondJump, IROps::OP_CONDJUMP>();
		_Op.first->Cmp1 = Cmp1;
		_Op.first->Cmp2 = Cmp2;
		_Op.first->TrueBlock = TrueBlock;
		_Op.first->FalseBlock = FalseBlock;
		_Op.first->Cond = Cond;
		_Op.first->CompareSize = CompareSize;
		_Op.first->FromNZCV = FromNZCV;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	IRPair<IROp_CondJump> _CondJump(OrderedNode *Cmp1, OrderedNode *Cmp2, OrderedNode *TrueBlock, OrderedNode *FalseBlock, CondClass Cond = CondClass::NEQ, FEXCore::IR::OpSize CompareSize = OpSize::iInvalid, bool FromNZCV = false) {
		auto ListDataBegin = DualListData.ListBegin();
		Cmp1->AddUse();
		Cmp2 = InlineAddSub(OpSize::i64Bit, Cmp2);
		Cmp2->AddUse();
		TrueBlock->AddUse();
		FalseBlock->AddUse();
		return _CondJump(Cmp1->Wrapped(ListDataBegin), Cmp2->Wrapped(ListDataBegin), TrueBlock->Wrapped(ListDataBegin), FalseBlock->Wrapped(ListDataBegin), Cond, CompareSize, FromNZCV);
	}

	/// Exits the current JIT function with a target RIP
	IRPair<IROp_ExitFunction> _ExitFunction(FEXCore::IR::OpSize Size, OrderedNodeWrapper NewRIP, FEXCore::IR::BranchHint Hint, OrderedNodeWrapper CallReturnAddress, OrderedNodeWrapper CallReturnBlock) {
		auto _Op = AllocateOp<IROp_ExitFunction, IROps::OP_EXITFUNCTION>();
		_Op.first->NewRIP = NewRIP;
		_Op.first->CallReturnAddress = CallReturnAddress;
		_Op.first->CallReturnBlock = CallReturnBlock;
		_Op.first->Hint = Hint;
		_Op.first->Header.Size = Size;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Exits the current JIT function with a target RIP
	IRPair<IROp_ExitFunction> _ExitFunction(FEXCore::IR::OpSize Size, OrderedNode *NewRIP, FEXCore::IR::BranchHint Hint, OrderedNode *CallReturnAddress, OrderedNode *CallReturnBlock) {
		auto ListDataBegin = DualListData.ListBegin();
		NewRIP = InlineAny(Size, NewRIP);
		NewRIP->AddUse();
		CallReturnAddress->AddUse();
		CallReturnBlock->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(NewRIP) == RegClass::Invalid || WalkFindRegClass(NewRIP) == RegClass::GPR, "GetOpRegClass(NewRIP) == RegClass::Invalid || WalkFindRegClass(NewRIP) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(CallReturnAddress) == RegClass::Invalid || WalkFindRegClass(CallReturnAddress) == RegClass::GPR, "GetOpRegClass(CallReturnAddress) == RegClass::Invalid || WalkFindRegClass(CallReturnAddress) == RegClass::GPR");
#endif
		return _ExitFunction(Size, NewRIP->Wrapped(ListDataBegin), Hint, CallReturnAddress->Wrapped(ListDataBegin), CallReturnBlock->Wrapped(ListDataBegin));
	}

	IRPair<IROp_Break> _Break(BreakDefinition Reason) {
		auto _Op = AllocateOp<IROp_Break, IROps::OP_BREAK>();
		_Op.first->Reason = Reason;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	IRPair<IROp_CallbackReturn> _CallbackReturn() {
		auto _Op = AllocateOp<IROp_CallbackReturn, IROps::OP_CALLBACKRETURN>();
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Dispatches a guest syscall through to the SyscallHandler class
	IRPair<IROp_Syscall> _Syscall(OrderedNodeWrapper SyscallID, OrderedNodeWrapper Arg0, OrderedNodeWrapper Arg1, OrderedNodeWrapper Arg2, OrderedNodeWrapper Arg3, OrderedNodeWrapper Arg4, OrderedNodeWrapper Arg5) {
		auto _Op = AllocateOp<IROp_Syscall, IROps::OP_SYSCALL>();
		_Op.first->SyscallID = SyscallID;
		_Op.first->Arg0 = Arg0;
		_Op.first->Arg1 = Arg1;
		_Op.first->Arg2 = Arg2;
		_Op.first->Arg3 = Arg3;
		_Op.first->Arg4 = Arg4;
		_Op.first->Arg5 = Arg5;
		_Op.first->Header.Size = OpSize::i64Bit;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Dispatches a guest syscall through to the SyscallHandler class
	IRPair<IROp_Syscall> _Syscall(OrderedNode *SyscallID, OrderedNode *Arg0, OrderedNode *Arg1, OrderedNode *Arg2, OrderedNode *Arg3, OrderedNode *Arg4, OrderedNode *Arg5) {
		auto ListDataBegin = DualListData.ListBegin();
		SyscallID->AddUse();
		Arg0->AddUse();
		Arg1->AddUse();
		Arg2->AddUse();
		Arg3->AddUse();
		Arg4->AddUse();
		Arg5->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(SyscallID) == RegClass::Invalid || WalkFindRegClass(SyscallID) == RegClass::GPR, "GetOpRegClass(SyscallID) == RegClass::Invalid || WalkFindRegClass(SyscallID) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Arg0) == RegClass::Invalid || WalkFindRegClass(Arg0) == RegClass::GPR, "GetOpRegClass(Arg0) == RegClass::Invalid || WalkFindRegClass(Arg0) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Arg1) == RegClass::Invalid || WalkFindRegClass(Arg1) == RegClass::GPR, "GetOpRegClass(Arg1) == RegClass::Invalid || WalkFindRegClass(Arg1) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Arg2) == RegClass::Invalid || WalkFindRegClass(Arg2) == RegClass::GPR, "GetOpRegClass(Arg2) == RegClass::Invalid || WalkFindRegClass(Arg2) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Arg3) == RegClass::Invalid || WalkFindRegClass(Arg3) == RegClass::GPR, "GetOpRegClass(Arg3) == RegClass::Invalid || WalkFindRegClass(Arg3) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Arg4) == RegClass::Invalid || WalkFindRegClass(Arg4) == RegClass::GPR, "GetOpRegClass(Arg4) == RegClass::Invalid || WalkFindRegClass(Arg4) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Arg5) == RegClass::Invalid || WalkFindRegClass(Arg5) == RegClass::GPR, "GetOpRegClass(Arg5) == RegClass::Invalid || WalkFindRegClass(Arg5) == RegClass::GPR");
#endif
		return _Syscall(SyscallID->Wrapped(ListDataBegin), Arg0->Wrapped(ListDataBegin), Arg1->Wrapped(ListDataBegin), Arg2->Wrapped(ListDataBegin), Arg3->Wrapped(ListDataBegin), Arg4->Wrapped(ListDataBegin), Arg5->Wrapped(ListDataBegin));
	}

	IRPair<IROp_Thunk> _Thunk(OrderedNodeWrapper ArgPtr, SHA256Sum ThunkNameHash) {
		auto _Op = AllocateOp<IROp_Thunk, IROps::OP_THUNK>();
		_Op.first->ArgPtr = ArgPtr;
		_Op.first->ThunkNameHash = ThunkNameHash;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	IRPair<IROp_Thunk> _Thunk(OrderedNode *ArgPtr, SHA256Sum ThunkNameHash) {
		auto ListDataBegin = DualListData.ListBegin();
		ArgPtr->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(ArgPtr) == RegClass::Invalid || WalkFindRegClass(ArgPtr) == RegClass::GPR, "GetOpRegClass(ArgPtr) == RegClass::Invalid || WalkFindRegClass(ArgPtr) == RegClass::GPR");
#endif
		return _Thunk(ArgPtr->Wrapped(ListDataBegin), ThunkNameHash);
	}

	/// Calls in to the CPUID handler function to return emulated CPUID
	IRPair<IROp_CPUID> _CPUID(OrderedNodeWrapper Function, OrderedNodeWrapper Leaf, OrderedNodeWrapper OutEAX, OrderedNodeWrapper OutEBX, OrderedNodeWrapper OutECX, OrderedNodeWrapper OutEDX) {
		auto _Op = AllocateOp<IROp_CPUID, IROps::OP_CPUID>();
		_Op.first->Function = Function;
		_Op.first->Leaf = Leaf;
		_Op.first->OutEAX = OutEAX;
		_Op.first->OutEBX = OutEBX;
		_Op.first->OutECX = OutECX;
		_Op.first->OutEDX = OutEDX;
		_Op.first->Header.Size = OpSize::i32Bit;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Calls in to the CPUID handler function to return emulated CPUID
	IRPair<IROp_CPUID> _CPUID(OrderedNode *Function, OrderedNode *Leaf, OrderedNode *OutEAX, OrderedNode *OutEBX, OrderedNode *OutECX, OrderedNode *OutEDX) {
		auto ListDataBegin = DualListData.ListBegin();
		Function->AddUse();
		Leaf->AddUse();
		OutEAX->AddUse();
		OutEBX->AddUse();
		OutECX->AddUse();
		OutEDX->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Function) == RegClass::Invalid || WalkFindRegClass(Function) == RegClass::GPR, "GetOpRegClass(Function) == RegClass::Invalid || WalkFindRegClass(Function) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Leaf) == RegClass::Invalid || WalkFindRegClass(Leaf) == RegClass::GPR, "GetOpRegClass(Leaf) == RegClass::Invalid || WalkFindRegClass(Leaf) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(OutEAX) == RegClass::Invalid || WalkFindRegClass(OutEAX) == RegClass::GPR, "GetOpRegClass(OutEAX) == RegClass::Invalid || WalkFindRegClass(OutEAX) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(OutEBX) == RegClass::Invalid || WalkFindRegClass(OutEBX) == RegClass::GPR, "GetOpRegClass(OutEBX) == RegClass::Invalid || WalkFindRegClass(OutEBX) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(OutECX) == RegClass::Invalid || WalkFindRegClass(OutECX) == RegClass::GPR, "GetOpRegClass(OutECX) == RegClass::Invalid || WalkFindRegClass(OutECX) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(OutEDX) == RegClass::Invalid || WalkFindRegClass(OutEDX) == RegClass::GPR, "GetOpRegClass(OutEDX) == RegClass::Invalid || WalkFindRegClass(OutEDX) == RegClass::GPR");
#endif
		return _CPUID(Function->Wrapped(ListDataBegin), Leaf->Wrapped(ListDataBegin), OutEAX->Wrapped(ListDataBegin), OutEBX->Wrapped(ListDataBegin), OutECX->Wrapped(ListDataBegin), OutEDX->Wrapped(ListDataBegin));
	}

	/// Calls in to the XCR handler function to return emulated XCR
	IRPair<IROp_XGetBV> _XGetBV(OrderedNodeWrapper Function, OrderedNodeWrapper OutEAX, OrderedNodeWrapper OutEDX) {
		auto _Op = AllocateOp<IROp_XGetBV, IROps::OP_XGETBV>();
		_Op.first->Function = Function;
		_Op.first->OutEAX = OutEAX;
		_Op.first->OutEDX = OutEDX;
		_Op.first->Header.Size = OpSize::i32Bit;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Calls in to the XCR handler function to return emulated XCR
	IRPair<IROp_XGetBV> _XGetBV(OrderedNode *Function, OrderedNode *OutEAX, OrderedNode *OutEDX) {
		auto ListDataBegin = DualListData.ListBegin();
		Function->AddUse();
		OutEAX->AddUse();
		OutEDX->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Function) == RegClass::Invalid || WalkFindRegClass(Function) == RegClass::GPR, "GetOpRegClass(Function) == RegClass::Invalid || WalkFindRegClass(Function) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(OutEAX) == RegClass::Invalid || WalkFindRegClass(OutEAX) == RegClass::GPR, "GetOpRegClass(OutEAX) == RegClass::Invalid || WalkFindRegClass(OutEAX) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(OutEDX) == RegClass::Invalid || WalkFindRegClass(OutEDX) == RegClass::GPR, "GetOpRegClass(OutEDX) == RegClass::Invalid || WalkFindRegClass(OutEDX) == RegClass::GPR");
#endif
		return _XGetBV(Function->Wrapped(ListDataBegin), OutEAX->Wrapped(ListDataBegin), OutEDX->Wrapped(ListDataBegin));
	}

	/// GPR copy, generated by RA to split live ranges
	IRPair<IROp_Copy> _Copy(OrderedNodeWrapper Source) {
		auto _Op = AllocateOp<IROp_Copy, IROps::OP_COPY>();
		_Op.first->Source = Source;
		_Op.first->Header.Size = OpSize::i64Bit;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// GPR copy, generated by RA to split live ranges
	IRPair<IROp_Copy> _Copy(OrderedNode *Source) {
		auto ListDataBegin = DualListData.ListBegin();
		Source->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Source) == RegClass::Invalid || WalkFindRegClass(Source) == RegClass::GPR, "GetOpRegClass(Source) == RegClass::Invalid || WalkFindRegClass(Source) == RegClass::GPR");
#endif
		return _Copy(Source->Wrapped(ListDataBegin));
	}

	/// Loads a value from the given register
	///
	/// Size must match the execution mode.
	IRPair<IROp_LoadRegister> _LoadRegister(uint32_t Reg, RegClass Class, FEXCore::IR::OpSize Size) {
		auto _Op = AllocateOp<IROp_LoadRegister, IROps::OP_LOADREGISTER>();
		_Op.first->Reg = Reg;
		_Op.first->Class = Class;
		_Op.first->Header.Size = Size;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Loads raw PF
	IRPair<IROp_LoadPF> _LoadPF(FEXCore::IR::OpSize Size) {
		auto _Op = AllocateOp<IROp_LoadPF, IROps::OP_LOADPF>();
		_Op.first->Header.Size = Size;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Loads raw PF
	IRPair<IROp_LoadAF> _LoadAF(FEXCore::IR::OpSize Size) {
		auto _Op = AllocateOp<IROp_LoadAF, IROps::OP_LOADAF>();
		_Op.first->Header.Size = Size;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Stores a value to a given register.
	///
	/// Size must match the execution mode.
	IRPair<IROp_StoreRegister> _StoreRegister(OrderedNodeWrapper Value, FEXCore::IR::OpSize Size) {
		auto _Op = AllocateOp<IROp_StoreRegister, IROps::OP_STOREREGISTER>();
		_Op.first->Value = Value;
		_Op.first->Header.Size = Size;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Stores a value to a given register.
	///
	/// Size must match the execution mode.
	IRPair<IROp_StoreRegister> _StoreRegister(OrderedNode *Value, FEXCore::IR::OpSize Size) {
		auto ListDataBegin = DualListData.ListBegin();
		Value->AddUse();
		return _StoreRegister(Value->Wrapped(ListDataBegin), Size);
	}

	/// Stores raw PF
	IRPair<IROp_StorePF> _StorePF(OrderedNodeWrapper Value, FEXCore::IR::OpSize Size) {
		auto _Op = AllocateOp<IROp_StorePF, IROps::OP_STOREPF>();
		_Op.first->Value = Value;
		_Op.first->Header.Size = Size;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Stores raw PF
	IRPair<IROp_StorePF> _StorePF(OrderedNode *Value, FEXCore::IR::OpSize Size) {
		auto ListDataBegin = DualListData.ListBegin();
		Value->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Value) == RegClass::Invalid || WalkFindRegClass(Value) == RegClass::GPR, "GetOpRegClass(Value) == RegClass::Invalid || WalkFindRegClass(Value) == RegClass::GPR");
#endif
		return _StorePF(Value->Wrapped(ListDataBegin), Size);
	}

	/// Stores raw AF
	IRPair<IROp_StoreAF> _StoreAF(OrderedNodeWrapper Value, FEXCore::IR::OpSize Size) {
		auto _Op = AllocateOp<IROp_StoreAF, IROps::OP_STOREAF>();
		_Op.first->Value = Value;
		_Op.first->Header.Size = Size;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Stores raw AF
	IRPair<IROp_StoreAF> _StoreAF(OrderedNode *Value, FEXCore::IR::OpSize Size) {
		auto ListDataBegin = DualListData.ListBegin();
		Value->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Value) == RegClass::Invalid || WalkFindRegClass(Value) == RegClass::GPR, "GetOpRegClass(Value) == RegClass::Invalid || WalkFindRegClass(Value) == RegClass::GPR");
#endif
		return _StoreAF(Value->Wrapped(ListDataBegin), Size);
	}

	/// Loads a value from the context with offset
	///
	/// Dest = Ctx[Offset]
	IRPair<IROp_LoadContext> _LoadContext(FEXCore::IR::OpSize ByteSize, RegClass Class, uint32_t Offset) {
		auto _Op = AllocateOp<IROp_LoadContext, IROps::OP_LOADCONTEXT>();
		_Op.first->Class = Class;
		_Op.first->Offset = Offset;
		_Op.first->Header.Size = ByteSize;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT((Class == RegClass::GPR && (ByteSize == IR::OpSize::i8Bit || ByteSize == IR::OpSize::i16Bit || ByteSize == IR::OpSize::i32Bit || ByteSize == IR::OpSize::i64Bit)) || Class == RegClass::FPR, "(Class == RegClass::GPR && (ByteSize == IR::OpSize::i8Bit || ByteSize == IR::OpSize::i16Bit || ByteSize == IR::OpSize::i32Bit || ByteSize == IR::OpSize::i64Bit)) || Class == RegClass::FPR");
		LOGMAN_THROW_A_FMT((Class == RegClass::FPR && (ByteSize == IR::OpSize::i8Bit || ByteSize == IR::OpSize::i16Bit || ByteSize == IR::OpSize::i32Bit || ByteSize == IR::OpSize::i64Bit || ByteSize == IR::OpSize::i128Bit || ByteSize == IR::OpSize::i256Bit)) || Class == RegClass::GPR, "(Class == RegClass::FPR && (ByteSize == IR::OpSize::i8Bit || ByteSize == IR::OpSize::i16Bit || ByteSize == IR::OpSize::i32Bit || ByteSize == IR::OpSize::i64Bit || ByteSize == IR::OpSize::i128Bit || ByteSize == IR::OpSize::i256Bit)) || Class == RegClass::GPR");
		LOGMAN_THROW_A_FMT(!(Offset >= offsetof(Core::CPUState, gregs[0]) && Offset < offsetof(Core::CPUState, gregs[16])) && "Can't LoadContext to GPR", "!(Offset >= offsetof(Core::CPUState, gregs[0]) && Offset < offsetof(Core::CPUState, gregs[16])) && \"Can't LoadContext to GPR\"");
		LOGMAN_THROW_A_FMT(!(Offset >= offsetof(Core::CPUState, xmm.avx.data[0]) && Offset < offsetof(Core::CPUState, xmm.avx.data[16])) && "Can't LoadContext to XMM", "!(Offset >= offsetof(Core::CPUState, xmm.avx.data[0]) && Offset < offsetof(Core::CPUState, xmm.avx.data[16])) && \"Can't LoadContext to XMM\"");
#endif
		return _Op;
	}

	/// Loads a pair of values from the context with offset
	///
	/// Value0 = Ctx[Offset], Value1 = Ctx[Offset + ByteSize]
	IRPair<IROp_LoadContextPair> _LoadContextPair(FEXCore::IR::OpSize ByteSize, RegClass Class, uint32_t Offset, OrderedNodeWrapper OutValue1, OrderedNodeWrapper OutValue2) {
		auto _Op = AllocateOp<IROp_LoadContextPair, IROps::OP_LOADCONTEXTPAIR>();
		_Op.first->OutValue1 = OutValue1;
		_Op.first->OutValue2 = OutValue2;
		_Op.first->Class = Class;
		_Op.first->Offset = Offset;
		_Op.first->Header.Size = ByteSize;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Loads a pair of values from the context with offset
	///
	/// Value0 = Ctx[Offset], Value1 = Ctx[Offset + ByteSize]
	IRPair<IROp_LoadContextPair> _LoadContextPair(FEXCore::IR::OpSize ByteSize, RegClass Class, uint32_t Offset, OrderedNode *OutValue1, OrderedNode *OutValue2) {
		auto ListDataBegin = DualListData.ListBegin();
		OutValue1->AddUse();
		OutValue2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT((Class == RegClass::GPR && (ByteSize == IR::OpSize::i8Bit || ByteSize == IR::OpSize::i16Bit || ByteSize == IR::OpSize::i32Bit || ByteSize == IR::OpSize::i64Bit)) || Class == RegClass::FPR, "(Class == RegClass::GPR && (ByteSize == IR::OpSize::i8Bit || ByteSize == IR::OpSize::i16Bit || ByteSize == IR::OpSize::i32Bit || ByteSize == IR::OpSize::i64Bit)) || Class == RegClass::FPR");
		LOGMAN_THROW_A_FMT((Class == RegClass::FPR && (ByteSize == IR::OpSize::i8Bit || ByteSize == IR::OpSize::i16Bit || ByteSize == IR::OpSize::i32Bit || ByteSize == IR::OpSize::i64Bit || ByteSize == IR::OpSize::i128Bit || ByteSize == IR::OpSize::i256Bit)) || Class == RegClass::GPR, "(Class == RegClass::FPR && (ByteSize == IR::OpSize::i8Bit || ByteSize == IR::OpSize::i16Bit || ByteSize == IR::OpSize::i32Bit || ByteSize == IR::OpSize::i64Bit || ByteSize == IR::OpSize::i128Bit || ByteSize == IR::OpSize::i256Bit)) || Class == RegClass::GPR");
		LOGMAN_THROW_A_FMT(!(Offset >= offsetof(Core::CPUState, gregs[0]) && Offset < offsetof(Core::CPUState, gregs[16])) && "Can't LoadContext to GPR", "!(Offset >= offsetof(Core::CPUState, gregs[0]) && Offset < offsetof(Core::CPUState, gregs[16])) && \"Can't LoadContext to GPR\"");
		LOGMAN_THROW_A_FMT(!(Offset >= offsetof(Core::CPUState, xmm.avx.data[0]) && Offset < offsetof(Core::CPUState, xmm.avx.data[16])) && "Can't LoadContext to XMM", "!(Offset >= offsetof(Core::CPUState, xmm.avx.data[0]) && Offset < offsetof(Core::CPUState, xmm.avx.data[16])) && \"Can't LoadContext to XMM\"");
#endif
		return _LoadContextPair(ByteSize, Class, Offset, OutValue1->Wrapped(ListDataBegin), OutValue2->Wrapped(ListDataBegin));
	}

	/// Stores a value to the context with offset
	///
	/// Ctx[Offset] = Value
	///
	/// Zero Extends if value's type is too small
	///
	/// Truncates if value's type is too large
	IRPair<IROp_StoreContext> _StoreContext(FEXCore::IR::OpSize ByteSize, RegClass Class, OrderedNodeWrapper Value, uint32_t Offset) {
		auto _Op = AllocateOp<IROp_StoreContext, IROps::OP_STORECONTEXT>();
		_Op.first->Value = Value;
		_Op.first->Class = Class;
		_Op.first->Offset = Offset;
		_Op.first->Header.Size = ByteSize;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Stores a value to the context with offset
	///
	/// Ctx[Offset] = Value
	///
	/// Zero Extends if value's type is too small
	///
	/// Truncates if value's type is too large
	IRPair<IROp_StoreContext> _StoreContext(FEXCore::IR::OpSize ByteSize, RegClass Class, OrderedNode *Value, uint32_t Offset) {
		auto ListDataBegin = DualListData.ListBegin();
		Value = InlineZero(OpSize::i64Bit, Value);
		Value->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT((Class == RegClass::GPR && (ByteSize == IR::OpSize::i8Bit || ByteSize == IR::OpSize::i16Bit || ByteSize == IR::OpSize::i32Bit || ByteSize == IR::OpSize::i64Bit)) || Class == RegClass::FPR, "(Class == RegClass::GPR && (ByteSize == IR::OpSize::i8Bit || ByteSize == IR::OpSize::i16Bit || ByteSize == IR::OpSize::i32Bit || ByteSize == IR::OpSize::i64Bit)) || Class == RegClass::FPR");
		LOGMAN_THROW_A_FMT((Class == RegClass::FPR && (ByteSize == IR::OpSize::i8Bit || ByteSize == IR::OpSize::i16Bit || ByteSize == IR::OpSize::i32Bit || ByteSize == IR::OpSize::i64Bit || ByteSize == IR::OpSize::i128Bit || ByteSize == IR::OpSize::i256Bit)) || Class == RegClass::GPR, "(Class == RegClass::FPR && (ByteSize == IR::OpSize::i8Bit || ByteSize == IR::OpSize::i16Bit || ByteSize == IR::OpSize::i32Bit || ByteSize == IR::OpSize::i64Bit || ByteSize == IR::OpSize::i128Bit || ByteSize == IR::OpSize::i256Bit)) || Class == RegClass::GPR");
		LOGMAN_THROW_A_FMT(!(Offset >= offsetof(Core::CPUState, gregs[0]) && Offset < offsetof(Core::CPUState, gregs[16])) && "Can't StoreContext to GPR", "!(Offset >= offsetof(Core::CPUState, gregs[0]) && Offset < offsetof(Core::CPUState, gregs[16])) && \"Can't StoreContext to GPR\"");
		LOGMAN_THROW_A_FMT(!(Offset >= offsetof(Core::CPUState, xmm.avx.data[0]) && Offset < offsetof(Core::CPUState, xmm.avx.data[16])) && "Can't StoreContext to XMM", "!(Offset >= offsetof(Core::CPUState, xmm.avx.data[0]) && Offset < offsetof(Core::CPUState, xmm.avx.data[16])) && \"Can't StoreContext to XMM\"");
#endif
		return _StoreContext(ByteSize, Class, Value->Wrapped(ListDataBegin), Offset);
	}

	/// Stores a pair of values to the context with offset
	///
	/// Ctx[Offset] = Value1, Ctx[Offset + ByteSize] = Value2
	///
	/// Zero Extends if value's type is too small
	///
	/// Truncates if value's type is too large
	IRPair<IROp_StoreContextPair> _StoreContextPair(FEXCore::IR::OpSize ByteSize, RegClass Class, OrderedNodeWrapper Value1, OrderedNodeWrapper Value2, uint32_t Offset) {
		auto _Op = AllocateOp<IROp_StoreContextPair, IROps::OP_STORECONTEXTPAIR>();
		_Op.first->Value1 = Value1;
		_Op.first->Value2 = Value2;
		_Op.first->Class = Class;
		_Op.first->Offset = Offset;
		_Op.first->Header.Size = ByteSize;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Stores a pair of values to the context with offset
	///
	/// Ctx[Offset] = Value1, Ctx[Offset + ByteSize] = Value2
	///
	/// Zero Extends if value's type is too small
	///
	/// Truncates if value's type is too large
	IRPair<IROp_StoreContextPair> _StoreContextPair(FEXCore::IR::OpSize ByteSize, RegClass Class, OrderedNode *Value1, OrderedNode *Value2, uint32_t Offset) {
		auto ListDataBegin = DualListData.ListBegin();
		Value1->AddUse();
		Value2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(WalkFindRegClass(Value1) == Class, "WalkFindRegClass(Value1) == Class");
		LOGMAN_THROW_A_FMT(WalkFindRegClass(Value2) == Class, "WalkFindRegClass(Value2) == Class");
		LOGMAN_THROW_A_FMT((Class == RegClass::GPR && (ByteSize == IR::OpSize::i8Bit || ByteSize == IR::OpSize::i16Bit || ByteSize == IR::OpSize::i32Bit || ByteSize == IR::OpSize::i64Bit)) || Class == RegClass::FPR, "(Class == RegClass::GPR && (ByteSize == IR::OpSize::i8Bit || ByteSize == IR::OpSize::i16Bit || ByteSize == IR::OpSize::i32Bit || ByteSize == IR::OpSize::i64Bit)) || Class == RegClass::FPR");
		LOGMAN_THROW_A_FMT((Class == RegClass::FPR && (ByteSize == IR::OpSize::i8Bit || ByteSize == IR::OpSize::i16Bit || ByteSize == IR::OpSize::i32Bit || ByteSize == IR::OpSize::i64Bit || ByteSize == IR::OpSize::i128Bit || ByteSize == IR::OpSize::i256Bit)) || Class == RegClass::GPR, "(Class == RegClass::FPR && (ByteSize == IR::OpSize::i8Bit || ByteSize == IR::OpSize::i16Bit || ByteSize == IR::OpSize::i32Bit || ByteSize == IR::OpSize::i64Bit || ByteSize == IR::OpSize::i128Bit || ByteSize == IR::OpSize::i256Bit)) || Class == RegClass::GPR");
		LOGMAN_THROW_A_FMT(!(Offset >= offsetof(Core::CPUState, gregs[0]) && Offset < offsetof(Core::CPUState, gregs[16])) && "Can't StoreContext to GPR", "!(Offset >= offsetof(Core::CPUState, gregs[0]) && Offset < offsetof(Core::CPUState, gregs[16])) && \"Can't StoreContext to GPR\"");
		LOGMAN_THROW_A_FMT(!(Offset >= offsetof(Core::CPUState, xmm.avx.data[0]) && Offset < offsetof(Core::CPUState, xmm.avx.data[16])) && "Can't StoreContext to XMM", "!(Offset >= offsetof(Core::CPUState, xmm.avx.data[0]) && Offset < offsetof(Core::CPUState, xmm.avx.data[16])) && \"Can't StoreContext to XMM\"");
#endif
		return _StoreContextPair(ByteSize, Class, Value1->Wrapped(ListDataBegin), Value2->Wrapped(ListDataBegin), Offset);
	}

	/// Loads a value from the context with offset and indexed by SSA value
	///
	/// Dest = Ctx[BaseOffset + Index * Stride]
	IRPair<IROp_LoadContextIndexed> _LoadContextIndexed(OrderedNodeWrapper Index, FEXCore::IR::OpSize ByteSize, uint32_t BaseOffset, uint32_t Stride, RegClass Class) {
		auto _Op = AllocateOp<IROp_LoadContextIndexed, IROps::OP_LOADCONTEXTINDEXED>();
		_Op.first->Index = Index;
		_Op.first->BaseOffset = BaseOffset;
		_Op.first->Stride = Stride;
		_Op.first->Class = Class;
		_Op.first->Header.Size = ByteSize;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Loads a value from the context with offset and indexed by SSA value
	///
	/// Dest = Ctx[BaseOffset + Index * Stride]
	IRPair<IROp_LoadContextIndexed> _LoadContextIndexed(OrderedNode *Index, FEXCore::IR::OpSize ByteSize, uint32_t BaseOffset, uint32_t Stride, RegClass Class) {
		auto ListDataBegin = DualListData.ListBegin();
		Index->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Index) == RegClass::Invalid || WalkFindRegClass(Index) == RegClass::GPR, "GetOpRegClass(Index) == RegClass::Invalid || WalkFindRegClass(Index) == RegClass::GPR");
		LOGMAN_THROW_A_FMT((Class == RegClass::GPR && (ByteSize == IR::OpSize::i8Bit || ByteSize == IR::OpSize::i16Bit || ByteSize == IR::OpSize::i32Bit || ByteSize == IR::OpSize::i64Bit)) || Class == RegClass::FPR, "(Class == RegClass::GPR && (ByteSize == IR::OpSize::i8Bit || ByteSize == IR::OpSize::i16Bit || ByteSize == IR::OpSize::i32Bit || ByteSize == IR::OpSize::i64Bit)) || Class == RegClass::FPR");
		LOGMAN_THROW_A_FMT((Class == RegClass::FPR && (ByteSize == IR::OpSize::i8Bit || ByteSize == IR::OpSize::i16Bit || ByteSize == IR::OpSize::i32Bit || ByteSize == IR::OpSize::i64Bit || ByteSize == IR::OpSize::i128Bit || ByteSize == IR::OpSize::i256Bit)) || Class == RegClass::GPR, "(Class == RegClass::FPR && (ByteSize == IR::OpSize::i8Bit || ByteSize == IR::OpSize::i16Bit || ByteSize == IR::OpSize::i32Bit || ByteSize == IR::OpSize::i64Bit || ByteSize == IR::OpSize::i128Bit || ByteSize == IR::OpSize::i256Bit)) || Class == RegClass::GPR");
		LOGMAN_THROW_A_FMT(!(BaseOffset >= offsetof(Core::CPUState, gregs[0]) && BaseOffset < offsetof(Core::CPUState, gregs[16])) && "Can't LoadContextIndexed to GPR", "!(BaseOffset >= offsetof(Core::CPUState, gregs[0]) && BaseOffset < offsetof(Core::CPUState, gregs[16])) && \"Can't LoadContextIndexed to GPR\"");
		LOGMAN_THROW_A_FMT(!(BaseOffset >= offsetof(Core::CPUState, xmm.avx.data[0]) && BaseOffset < offsetof(Core::CPUState, xmm.avx.data[16])) && "Can't LoadContextIndexed to XMM", "!(BaseOffset >= offsetof(Core::CPUState, xmm.avx.data[0]) && BaseOffset < offsetof(Core::CPUState, xmm.avx.data[16])) && \"Can't LoadContextIndexed to XMM\"");
#endif
		return _LoadContextIndexed(Index->Wrapped(ListDataBegin), ByteSize, BaseOffset, Stride, Class);
	}

	/// Stores a value to the context with offset and indexed by SSA value
	///
	/// Ctx[BaseOffset + Index * Stride] = Value
	IRPair<IROp_StoreContextIndexed> _StoreContextIndexed(OrderedNodeWrapper Value, OrderedNodeWrapper Index, FEXCore::IR::OpSize ByteSize, uint32_t BaseOffset, uint32_t Stride, RegClass Class) {
		auto _Op = AllocateOp<IROp_StoreContextIndexed, IROps::OP_STORECONTEXTINDEXED>();
		_Op.first->Value = Value;
		_Op.first->Index = Index;
		_Op.first->BaseOffset = BaseOffset;
		_Op.first->Stride = Stride;
		_Op.first->Class = Class;
		_Op.first->Header.Size = ByteSize;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Stores a value to the context with offset and indexed by SSA value
	///
	/// Ctx[BaseOffset + Index * Stride] = Value
	IRPair<IROp_StoreContextIndexed> _StoreContextIndexed(OrderedNode *Value, OrderedNode *Index, FEXCore::IR::OpSize ByteSize, uint32_t BaseOffset, uint32_t Stride, RegClass Class) {
		auto ListDataBegin = DualListData.ListBegin();
		Value->AddUse();
		Index->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Index) == RegClass::Invalid || WalkFindRegClass(Index) == RegClass::GPR, "GetOpRegClass(Index) == RegClass::Invalid || WalkFindRegClass(Index) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(WalkFindRegClass(Value) == Class, "WalkFindRegClass(Value) == Class");
		LOGMAN_THROW_A_FMT((Class == RegClass::GPR && (ByteSize == IR::OpSize::i8Bit || ByteSize == IR::OpSize::i16Bit || ByteSize == IR::OpSize::i32Bit || ByteSize == IR::OpSize::i64Bit)) || Class == RegClass::FPR, "(Class == RegClass::GPR && (ByteSize == IR::OpSize::i8Bit || ByteSize == IR::OpSize::i16Bit || ByteSize == IR::OpSize::i32Bit || ByteSize == IR::OpSize::i64Bit)) || Class == RegClass::FPR");
		LOGMAN_THROW_A_FMT((Class == RegClass::FPR && (ByteSize == IR::OpSize::i8Bit || ByteSize == IR::OpSize::i16Bit || ByteSize == IR::OpSize::i32Bit || ByteSize == IR::OpSize::i64Bit || ByteSize == IR::OpSize::i128Bit || ByteSize == IR::OpSize::i256Bit)) || Class == RegClass::GPR, "(Class == RegClass::FPR && (ByteSize == IR::OpSize::i8Bit || ByteSize == IR::OpSize::i16Bit || ByteSize == IR::OpSize::i32Bit || ByteSize == IR::OpSize::i64Bit || ByteSize == IR::OpSize::i128Bit || ByteSize == IR::OpSize::i256Bit)) || Class == RegClass::GPR");
		LOGMAN_THROW_A_FMT(!(BaseOffset >= offsetof(Core::CPUState, gregs[0]) && BaseOffset < offsetof(Core::CPUState, gregs[16])) && "Can't StoreContextIndexed to GPR", "!(BaseOffset >= offsetof(Core::CPUState, gregs[0]) && BaseOffset < offsetof(Core::CPUState, gregs[16])) && \"Can't StoreContextIndexed to GPR\"");
		LOGMAN_THROW_A_FMT(!(BaseOffset >= offsetof(Core::CPUState, xmm.avx.data[0]) && BaseOffset < offsetof(Core::CPUState, xmm.avx.data[16])) && "Can't StoreContextIndexed to XMM", "!(BaseOffset >= offsetof(Core::CPUState, xmm.avx.data[0]) && BaseOffset < offsetof(Core::CPUState, xmm.avx.data[16])) && \"Can't StoreContextIndexed to XMM\"");
#endif
		return _StoreContextIndexed(Value->Wrapped(ListDataBegin), Index->Wrapped(ListDataBegin), ByteSize, BaseOffset, Stride, Class);
	}

	/// Forms an address into the context structure indexed by SSA value
	///
	/// Dest = Ctx + Index * Stride
	///
	/// This allows backends to compute the address once and reuse it for multiple memory operations
	///
	/// Stride must be a power of 2
	IRPair<IROp_FormContextAddress> _FormContextAddress(FEXCore::IR::OpSize Size, OrderedNodeWrapper Index, uint32_t Stride) {
		auto _Op = AllocateOp<IROp_FormContextAddress, IROps::OP_FORMCONTEXTADDRESS>();
		_Op.first->Index = Index;
		_Op.first->Stride = Stride;
		_Op.first->Header.Size = Size;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Forms an address into the context structure indexed by SSA value
	///
	/// Dest = Ctx + Index * Stride
	///
	/// This allows backends to compute the address once and reuse it for multiple memory operations
	///
	/// Stride must be a power of 2
	IRPair<IROp_FormContextAddress> _FormContextAddress(FEXCore::IR::OpSize Size, OrderedNode *Index, uint32_t Stride) {
		auto ListDataBegin = DualListData.ListBegin();
		Index->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Index) == RegClass::Invalid || WalkFindRegClass(Index) == RegClass::GPR, "GetOpRegClass(Index) == RegClass::Invalid || WalkFindRegClass(Index) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(Size == IR::OpSize::i64Bit, "Size == IR::OpSize::i64Bit");
#endif
		return _FormContextAddress(Size, Index->Wrapped(ListDataBegin), Stride);
	}

	/// Spills an SSA value to memory
	///
	/// Spill slots are register allocated and has live ranges calculated to handle slot calculation
	///
	/// !Don't use this op. It is for RA to handle spilling and filling!
	IRPair<IROp_SpillRegister> _SpillRegister(OrderedNodeWrapper Value, uint32_t Slot, RegClass Class) {
		auto _Op = AllocateOp<IROp_SpillRegister, IROps::OP_SPILLREGISTER>();
		_Op.first->Value = Value;
		_Op.first->Slot = Slot;
		_Op.first->Class = Class;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Spills an SSA value to memory
	///
	/// Spill slots are register allocated and has live ranges calculated to handle slot calculation
	///
	/// !Don't use this op. It is for RA to handle spilling and filling!
	IRPair<IROp_SpillRegister> _SpillRegister(OrderedNode *Value, uint32_t Slot, RegClass Class) {
		auto ListDataBegin = DualListData.ListBegin();
		Value->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(WalkFindRegClass(Value) == Class, "WalkFindRegClass(Value) == Class");
#endif
		return _SpillRegister(Value->Wrapped(ListDataBegin), Slot, Class);
	}

	/// Fills a register from a spill slot
	///
	/// Spill slots are register allocated and has live ranges calculated to handle slot calculation
	///
	/// !Don't use this op. It is for RA to handle spilling and filling!
	IRPair<IROp_FillRegister> _FillRegister(FEXCore::IR::OpSize Size, FEXCore::IR::OpSize ElementSize, uint32_t Slot, RegClass Class) {
		auto _Op = AllocateOp<IROp_FillRegister, IROps::OP_FILLREGISTER>();
		_Op.first->Slot = Slot;
		_Op.first->Class = Class;
		_Op.first->Header.Size = Size;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	/// Loads value of NZCV register
	IRPair<IROp_LoadNZCV> _LoadNZCV() {
		auto _Op = AllocateOp<IROp_LoadNZCV, IROps::OP_LOADNZCV>();
		_Op.first->Header.Size = OpSize::i32Bit;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Stores value to NZCV register
	IRPair<IROp_StoreNZCV> _StoreNZCV(OrderedNodeWrapper Value) {
		auto _Op = AllocateOp<IROp_StoreNZCV, IROps::OP_STORENZCV>();
		_Op.first->Value = Value;
		_Op.first->Header.Size = OpSize::i32Bit;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Stores value to NZCV register
	IRPair<IROp_StoreNZCV> _StoreNZCV(OrderedNode *Value) {
		auto ListDataBegin = DualListData.ListBegin();
		Value->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Value) == RegClass::Invalid || WalkFindRegClass(Value) == RegClass::GPR, "GetOpRegClass(Value) == RegClass::Invalid || WalkFindRegClass(Value) == RegClass::GPR");
#endif
		return _StoreNZCV(Value->Wrapped(ListDataBegin));
	}

	/// Loads the decimal flag from the context object in -1/1
	///
	/// representation for easy consumption
	IRPair<IROp_LoadDF> _LoadDF() {
		auto _Op = AllocateOp<IROp_LoadDF, IROps::OP_LOADDF>();
		_Op.first->Header.Size = OpSize::i64Bit;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	IRPair<IROp_LoadMem> _LoadMem(RegClass Class, FEXCore::IR::OpSize Size, OrderedNodeWrapper Addr, OrderedNodeWrapper Offset, FEXCore::IR::OpSize Align, MemOffsetType OffsetType, uint8_t OffsetScale) {
		auto _Op = AllocateOp<IROp_LoadMem, IROps::OP_LOADMEM>();
		_Op.first->Addr = Addr;
		_Op.first->Offset = Offset;
		_Op.first->Class = Class;
		_Op.first->Align = Align;
		_Op.first->OffsetType = OffsetType;
		_Op.first->OffsetScale = OffsetScale;
		_Op.first->Header.Size = Size;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	IRPair<IROp_LoadMem> _LoadMem(RegClass Class, FEXCore::IR::OpSize Size, OrderedNode *Addr, OrderedNode *Offset, FEXCore::IR::OpSize Align, MemOffsetType OffsetType, uint8_t OffsetScale) {
		auto ListDataBegin = DualListData.ListBegin();
		Addr->AddUse();
		Offset = InlineMem(Size, Offset, OffsetType, OffsetScale);
		Offset->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Addr) == RegClass::Invalid || WalkFindRegClass(Addr) == RegClass::GPR, "GetOpRegClass(Addr) == RegClass::Invalid || WalkFindRegClass(Addr) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Offset) == RegClass::Invalid || WalkFindRegClass(Offset) == RegClass::GPR, "GetOpRegClass(Offset) == RegClass::Invalid || WalkFindRegClass(Offset) == RegClass::GPR");
#endif
		return _LoadMem(Class, Size, Addr->Wrapped(ListDataBegin), Offset->Wrapped(ListDataBegin), Align, OffsetType, OffsetScale);
	}

	/// Load a pair of values from memory.
	IRPair<IROp_LoadMemPair> _LoadMemPair(RegClass Class, FEXCore::IR::OpSize Size, OrderedNodeWrapper Addr, uint32_t Offset, OrderedNodeWrapper OutValue1, OrderedNodeWrapper OutValue2) {
		auto _Op = AllocateOp<IROp_LoadMemPair, IROps::OP_LOADMEMPAIR>();
		_Op.first->Addr = Addr;
		_Op.first->OutValue1 = OutValue1;
		_Op.first->OutValue2 = OutValue2;
		_Op.first->Class = Class;
		_Op.first->Offset = Offset;
		_Op.first->Header.Size = Size;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Load a pair of values from memory.
	IRPair<IROp_LoadMemPair> _LoadMemPair(RegClass Class, FEXCore::IR::OpSize Size, OrderedNode *Addr, uint32_t Offset, OrderedNode *OutValue1, OrderedNode *OutValue2) {
		auto ListDataBegin = DualListData.ListBegin();
		Addr->AddUse();
		OutValue1->AddUse();
		OutValue2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Addr) == RegClass::Invalid || WalkFindRegClass(Addr) == RegClass::GPR, "GetOpRegClass(Addr) == RegClass::Invalid || WalkFindRegClass(Addr) == RegClass::GPR");
#endif
		return _LoadMemPair(Class, Size, Addr->Wrapped(ListDataBegin), Offset, OutValue1->Wrapped(ListDataBegin), OutValue2->Wrapped(ListDataBegin));
	}

	/// Stores a value to memory.
	///
	/// Zero Extends if value's type is too small
	///
	/// Truncates if value's type is too large
	IRPair<IROp_StoreMem> _StoreMem(RegClass Class, FEXCore::IR::OpSize Size, OrderedNodeWrapper Value, OrderedNodeWrapper Addr, OrderedNodeWrapper Offset, FEXCore::IR::OpSize Align, MemOffsetType OffsetType, uint8_t OffsetScale) {
		auto _Op = AllocateOp<IROp_StoreMem, IROps::OP_STOREMEM>();
		_Op.first->Value = Value;
		_Op.first->Addr = Addr;
		_Op.first->Offset = Offset;
		_Op.first->Class = Class;
		_Op.first->Align = Align;
		_Op.first->OffsetType = OffsetType;
		_Op.first->OffsetScale = OffsetScale;
		_Op.first->Header.Size = Size;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Stores a value to memory.
	///
	/// Zero Extends if value's type is too small
	///
	/// Truncates if value's type is too large
	IRPair<IROp_StoreMem> _StoreMem(RegClass Class, FEXCore::IR::OpSize Size, OrderedNode *Value, OrderedNode *Addr, OrderedNode *Offset, FEXCore::IR::OpSize Align, MemOffsetType OffsetType, uint8_t OffsetScale) {
		auto ListDataBegin = DualListData.ListBegin();
		Value = InlineZero(Size, Value);
		Value->AddUse();
		Addr->AddUse();
		Offset = InlineMem(Size, Offset, OffsetType, OffsetScale);
		Offset->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Addr) == RegClass::Invalid || WalkFindRegClass(Addr) == RegClass::GPR, "GetOpRegClass(Addr) == RegClass::Invalid || WalkFindRegClass(Addr) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Offset) == RegClass::Invalid || WalkFindRegClass(Offset) == RegClass::GPR, "GetOpRegClass(Offset) == RegClass::Invalid || WalkFindRegClass(Offset) == RegClass::GPR");
#endif
		return _StoreMem(Class, Size, Value->Wrapped(ListDataBegin), Addr->Wrapped(ListDataBegin), Offset->Wrapped(ListDataBegin), Align, OffsetType, OffsetScale);
	}

	/// Stores a pair of values to memory.
	///
	/// Zero Extends if value's type is too small
	///
	/// Truncates if value's type is too large
	IRPair<IROp_StoreMemPair> _StoreMemPair(RegClass Class, FEXCore::IR::OpSize Size, OrderedNodeWrapper Value1, OrderedNodeWrapper Value2, OrderedNodeWrapper Addr, uint32_t Offset) {
		auto _Op = AllocateOp<IROp_StoreMemPair, IROps::OP_STOREMEMPAIR>();
		_Op.first->Value1 = Value1;
		_Op.first->Value2 = Value2;
		_Op.first->Addr = Addr;
		_Op.first->Class = Class;
		_Op.first->Offset = Offset;
		_Op.first->Header.Size = Size;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Stores a pair of values to memory.
	///
	/// Zero Extends if value's type is too small
	///
	/// Truncates if value's type is too large
	IRPair<IROp_StoreMemPair> _StoreMemPair(RegClass Class, FEXCore::IR::OpSize Size, OrderedNode *Value1, OrderedNode *Value2, OrderedNode *Addr, uint32_t Offset) {
		auto ListDataBegin = DualListData.ListBegin();
		Value1 = InlineZero(Size, Value1);
		Value1->AddUse();
		Value2 = InlineZero(Size, Value2);
		Value2->AddUse();
		Addr->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Addr) == RegClass::Invalid || WalkFindRegClass(Addr) == RegClass::GPR, "GetOpRegClass(Addr) == RegClass::Invalid || WalkFindRegClass(Addr) == RegClass::GPR");
#endif
		return _StoreMemPair(Class, Size, Value1->Wrapped(ListDataBegin), Value2->Wrapped(ListDataBegin), Addr->Wrapped(ListDataBegin), Offset);
	}

	/// Stores a value to memory using SVE predicate mask that's designed
	///
	/// specifically for use in the X87 SVE Ldst optimization.
	IRPair<IROp_StoreMemX87SVEOptPredicate> _StoreMemX87SVEOptPredicate(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Value, OrderedNodeWrapper Addr) {
		auto _Op = AllocateOp<IROp_StoreMemX87SVEOptPredicate, IROps::OP_STOREMEMX87SVEOPTPREDICATE>();
		_Op.first->Value = Value;
		_Op.first->Addr = Addr;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	/// Stores a value to memory using SVE predicate mask that's designed
	///
	/// specifically for use in the X87 SVE Ldst optimization.
	IRPair<IROp_StoreMemX87SVEOptPredicate> _StoreMemX87SVEOptPredicate(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Value, OrderedNode *Addr) {
		auto ListDataBegin = DualListData.ListBegin();
		Value->AddUse();
		Addr->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Value) == RegClass::Invalid || WalkFindRegClass(Value) == RegClass::FPR, "GetOpRegClass(Value) == RegClass::Invalid || WalkFindRegClass(Value) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Addr) == RegClass::Invalid || WalkFindRegClass(Addr) == RegClass::GPR, "GetOpRegClass(Addr) == RegClass::Invalid || WalkFindRegClass(Addr) == RegClass::GPR");
#endif
		return _StoreMemX87SVEOptPredicate(RegisterSize, ElementSize, Value->Wrapped(ListDataBegin), Addr->Wrapped(ListDataBegin));
	}

	/// Loads a value to memory using SVE predicate mask that's designed
	///
	/// specifically for use in the X87 SVE Ldst optimization.
	IRPair<IROp_LoadMemX87SVEOptPredicate> _LoadMemX87SVEOptPredicate(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Addr) {
		auto _Op = AllocateOp<IROp_LoadMemX87SVEOptPredicate, IROps::OP_LOADMEMX87SVEOPTPREDICATE>();
		_Op.first->Addr = Addr;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	/// Loads a value to memory using SVE predicate mask that's designed
	///
	/// specifically for use in the X87 SVE Ldst optimization.
	IRPair<IROp_LoadMemX87SVEOptPredicate> _LoadMemX87SVEOptPredicate(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Addr) {
		auto ListDataBegin = DualListData.ListBegin();
		Addr->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Addr) == RegClass::Invalid || WalkFindRegClass(Addr) == RegClass::GPR, "GetOpRegClass(Addr) == RegClass::Invalid || WalkFindRegClass(Addr) == RegClass::GPR");
#endif
		return _LoadMemX87SVEOptPredicate(RegisterSize, ElementSize, Addr->Wrapped(ListDataBegin));
	}

	/// Does a x86 TSO compatible load from memory. Offset must be Invalid().
	IRPair<IROp_LoadMemTSO> _LoadMemTSO(RegClass Class, FEXCore::IR::OpSize Size, OrderedNodeWrapper Addr, OrderedNodeWrapper Offset, FEXCore::IR::OpSize Align, MemOffsetType OffsetType, uint8_t OffsetScale) {
		auto _Op = AllocateOp<IROp_LoadMemTSO, IROps::OP_LOADMEMTSO>();
		_Op.first->Addr = Addr;
		_Op.first->Offset = Offset;
		_Op.first->Class = Class;
		_Op.first->Align = Align;
		_Op.first->OffsetType = OffsetType;
		_Op.first->OffsetScale = OffsetScale;
		_Op.first->Header.Size = Size;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Does a x86 TSO compatible load from memory. Offset must be Invalid().
	IRPair<IROp_LoadMemTSO> _LoadMemTSO(RegClass Class, FEXCore::IR::OpSize Size, OrderedNode *Addr, OrderedNode *Offset, FEXCore::IR::OpSize Align, MemOffsetType OffsetType, uint8_t OffsetScale) {
		auto ListDataBegin = DualListData.ListBegin();
		Addr->AddUse();
		Offset = InlineMem(Size, Offset, OffsetType, OffsetScale, true /* TSO */);
		Offset->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Addr) == RegClass::Invalid || WalkFindRegClass(Addr) == RegClass::GPR, "GetOpRegClass(Addr) == RegClass::Invalid || WalkFindRegClass(Addr) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Offset) == RegClass::Invalid || WalkFindRegClass(Offset) == RegClass::GPR, "GetOpRegClass(Offset) == RegClass::Invalid || WalkFindRegClass(Offset) == RegClass::GPR");
#endif
		return _LoadMemTSO(Class, Size, Addr->Wrapped(ListDataBegin), Offset->Wrapped(ListDataBegin), Align, OffsetType, OffsetScale);
	}

	/// Does a x86 TSO compatible store to memory. Offset must be Invalid().
	IRPair<IROp_StoreMemTSO> _StoreMemTSO(RegClass Class, FEXCore::IR::OpSize Size, OrderedNodeWrapper Value, OrderedNodeWrapper Addr, OrderedNodeWrapper Offset, FEXCore::IR::OpSize Align, MemOffsetType OffsetType, uint8_t OffsetScale) {
		auto _Op = AllocateOp<IROp_StoreMemTSO, IROps::OP_STOREMEMTSO>();
		_Op.first->Value = Value;
		_Op.first->Addr = Addr;
		_Op.first->Offset = Offset;
		_Op.first->Class = Class;
		_Op.first->Align = Align;
		_Op.first->OffsetType = OffsetType;
		_Op.first->OffsetScale = OffsetScale;
		_Op.first->Header.Size = Size;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Does a x86 TSO compatible store to memory. Offset must be Invalid().
	IRPair<IROp_StoreMemTSO> _StoreMemTSO(RegClass Class, FEXCore::IR::OpSize Size, OrderedNode *Value, OrderedNode *Addr, OrderedNode *Offset, FEXCore::IR::OpSize Align, MemOffsetType OffsetType, uint8_t OffsetScale) {
		auto ListDataBegin = DualListData.ListBegin();
		Value = InlineZero(Size, Value);
		Value->AddUse();
		Addr->AddUse();
		Offset = InlineMem(Size, Offset, OffsetType, OffsetScale, true /* TSO */);
		Offset->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Addr) == RegClass::Invalid || WalkFindRegClass(Addr) == RegClass::GPR, "GetOpRegClass(Addr) == RegClass::Invalid || WalkFindRegClass(Addr) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Offset) == RegClass::Invalid || WalkFindRegClass(Offset) == RegClass::GPR, "GetOpRegClass(Offset) == RegClass::Invalid || WalkFindRegClass(Offset) == RegClass::GPR");
#endif
		return _StoreMemTSO(Class, Size, Value->Wrapped(ListDataBegin), Addr->Wrapped(ListDataBegin), Offset->Wrapped(ListDataBegin), Align, OffsetType, OffsetScale);
	}

	/// Does a masked load similar to VPMASKMOV/VMASKMOV where the upper bit of each element
	///
	/// determines whether or not that element will be loaded from memory
	IRPair<IROp_VLoadVectorMasked> _VLoadVectorMasked(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Mask, OrderedNodeWrapper Addr, OrderedNodeWrapper Offset, MemOffsetType OffsetType, uint8_t OffsetScale) {
		SaveNZCV(IROps::OP_VLOADVECTORMASKED);		auto _Op = AllocateOp<IROp_VLoadVectorMasked, IROps::OP_VLOADVECTORMASKED>();
		_Op.first->Mask = Mask;
		_Op.first->Addr = Addr;
		_Op.first->Offset = Offset;
		_Op.first->OffsetType = OffsetType;
		_Op.first->OffsetScale = OffsetScale;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	/// Does a masked load similar to VPMASKMOV/VMASKMOV where the upper bit of each element
	///
	/// determines whether or not that element will be loaded from memory
	IRPair<IROp_VLoadVectorMasked> _VLoadVectorMasked(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Mask, OrderedNode *Addr, OrderedNode *Offset, MemOffsetType OffsetType, uint8_t OffsetScale) {
		auto ListDataBegin = DualListData.ListBegin();
		Mask->AddUse();
		Addr->AddUse();
		Offset->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Mask) == RegClass::Invalid || WalkFindRegClass(Mask) == RegClass::FPR, "GetOpRegClass(Mask) == RegClass::Invalid || WalkFindRegClass(Mask) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Addr) == RegClass::Invalid || WalkFindRegClass(Addr) == RegClass::GPR, "GetOpRegClass(Addr) == RegClass::Invalid || WalkFindRegClass(Addr) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Offset) == RegClass::Invalid || WalkFindRegClass(Offset) == RegClass::GPR, "GetOpRegClass(Offset) == RegClass::Invalid || WalkFindRegClass(Offset) == RegClass::GPR");
#endif
		return _VLoadVectorMasked(RegisterSize, ElementSize, Mask->Wrapped(ListDataBegin), Addr->Wrapped(ListDataBegin), Offset->Wrapped(ListDataBegin), OffsetType, OffsetScale);
	}

	/// Does a masked store similar to VPMASKMOV/VMASKMOV where the upper bit of each element
	///
	/// determines whether or not that element will be stored to memory
	IRPair<IROp_VStoreVectorMasked> _VStoreVectorMasked(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Mask, OrderedNodeWrapper Data, OrderedNodeWrapper Addr, OrderedNodeWrapper Offset, MemOffsetType OffsetType, uint8_t OffsetScale) {
		SaveNZCV(IROps::OP_VSTOREVECTORMASKED);		auto _Op = AllocateOp<IROp_VStoreVectorMasked, IROps::OP_VSTOREVECTORMASKED>();
		_Op.first->Mask = Mask;
		_Op.first->Data = Data;
		_Op.first->Addr = Addr;
		_Op.first->Offset = Offset;
		_Op.first->OffsetType = OffsetType;
		_Op.first->OffsetScale = OffsetScale;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	/// Does a masked store similar to VPMASKMOV/VMASKMOV where the upper bit of each element
	///
	/// determines whether or not that element will be stored to memory
	IRPair<IROp_VStoreVectorMasked> _VStoreVectorMasked(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Mask, OrderedNode *Data, OrderedNode *Addr, OrderedNode *Offset, MemOffsetType OffsetType, uint8_t OffsetScale) {
		auto ListDataBegin = DualListData.ListBegin();
		Mask->AddUse();
		Data->AddUse();
		Addr->AddUse();
		Offset->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Mask) == RegClass::Invalid || WalkFindRegClass(Mask) == RegClass::FPR, "GetOpRegClass(Mask) == RegClass::Invalid || WalkFindRegClass(Mask) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Data) == RegClass::Invalid || WalkFindRegClass(Data) == RegClass::FPR, "GetOpRegClass(Data) == RegClass::Invalid || WalkFindRegClass(Data) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Addr) == RegClass::Invalid || WalkFindRegClass(Addr) == RegClass::GPR, "GetOpRegClass(Addr) == RegClass::Invalid || WalkFindRegClass(Addr) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Offset) == RegClass::Invalid || WalkFindRegClass(Offset) == RegClass::GPR, "GetOpRegClass(Offset) == RegClass::Invalid || WalkFindRegClass(Offset) == RegClass::GPR");
#endif
		return _VStoreVectorMasked(RegisterSize, ElementSize, Mask->Wrapped(ListDataBegin), Data->Wrapped(ListDataBegin), Addr->Wrapped(ListDataBegin), Offset->Wrapped(ListDataBegin), OffsetType, OffsetScale);
	}

	/// Does a masked load similar to VPGATHERD* where the upper bit of each element
	///
	/// determines whether or not that element will be loaded from memory.
	///
	/// Most of VSIB encoding is passed directly through to the IR operation.
	IRPair<IROp_VLoadVectorGatherMasked> _VLoadVectorGatherMasked(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Incoming, OrderedNodeWrapper Mask, OrderedNodeWrapper AddrBase, OrderedNodeWrapper VectorIndexLow, OrderedNodeWrapper VectorIndexHigh, FEXCore::IR::OpSize VectorIndexElementSize, uint8_t OffsetScale, uint8_t DataElementOffsetStart, uint8_t IndexElementOffsetStart, FEXCore::IR::OpSize AddrSize) {
		SaveNZCV(IROps::OP_VLOADVECTORGATHERMASKED);		auto _Op = AllocateOp<IROp_VLoadVectorGatherMasked, IROps::OP_VLOADVECTORGATHERMASKED>();
		_Op.first->Incoming = Incoming;
		_Op.first->Mask = Mask;
		_Op.first->AddrBase = AddrBase;
		_Op.first->VectorIndexLow = VectorIndexLow;
		_Op.first->VectorIndexHigh = VectorIndexHigh;
		_Op.first->VectorIndexElementSize = VectorIndexElementSize;
		_Op.first->OffsetScale = OffsetScale;
		_Op.first->DataElementOffsetStart = DataElementOffsetStart;
		_Op.first->IndexElementOffsetStart = IndexElementOffsetStart;
		_Op.first->AddrSize = AddrSize;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	/// Does a masked load similar to VPGATHERD* where the upper bit of each element
	///
	/// determines whether or not that element will be loaded from memory.
	///
	/// Most of VSIB encoding is passed directly through to the IR operation.
	IRPair<IROp_VLoadVectorGatherMasked> _VLoadVectorGatherMasked(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Incoming, OrderedNode *Mask, OrderedNode *AddrBase, OrderedNode *VectorIndexLow, OrderedNode *VectorIndexHigh, FEXCore::IR::OpSize VectorIndexElementSize, uint8_t OffsetScale, uint8_t DataElementOffsetStart, uint8_t IndexElementOffsetStart, FEXCore::IR::OpSize AddrSize) {
		auto ListDataBegin = DualListData.ListBegin();
		Incoming->AddUse();
		Mask->AddUse();
		AddrBase->AddUse();
		VectorIndexLow->AddUse();
		VectorIndexHigh->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Incoming) == RegClass::Invalid || WalkFindRegClass(Incoming) == RegClass::FPR, "GetOpRegClass(Incoming) == RegClass::Invalid || WalkFindRegClass(Incoming) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Mask) == RegClass::Invalid || WalkFindRegClass(Mask) == RegClass::FPR, "GetOpRegClass(Mask) == RegClass::Invalid || WalkFindRegClass(Mask) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(AddrBase) == RegClass::Invalid || WalkFindRegClass(AddrBase) == RegClass::GPR, "GetOpRegClass(AddrBase) == RegClass::Invalid || WalkFindRegClass(AddrBase) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(VectorIndexLow) == RegClass::Invalid || WalkFindRegClass(VectorIndexLow) == RegClass::FPR, "GetOpRegClass(VectorIndexLow) == RegClass::Invalid || WalkFindRegClass(VectorIndexLow) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(VectorIndexHigh) == RegClass::Invalid || WalkFindRegClass(VectorIndexHigh) == RegClass::FPR, "GetOpRegClass(VectorIndexHigh) == RegClass::Invalid || WalkFindRegClass(VectorIndexHigh) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(VectorIndexElementSize == OpSize::i32Bit || VectorIndexElementSize == OpSize::i64Bit, "VectorIndexElementSize == OpSize::i32Bit || VectorIndexElementSize == OpSize::i64Bit");
#endif
		return _VLoadVectorGatherMasked(RegisterSize, ElementSize, Incoming->Wrapped(ListDataBegin), Mask->Wrapped(ListDataBegin), AddrBase->Wrapped(ListDataBegin), VectorIndexLow->Wrapped(ListDataBegin), VectorIndexHigh->Wrapped(ListDataBegin), VectorIndexElementSize, OffsetScale, DataElementOffsetStart, IndexElementOffsetStart, AddrSize);
	}

	/// Does a masked load similar to VPGATHERQPS where the upper bit of each element
	///
	/// determines whether or not that element will be loaded from memory.
	///
	/// Most of VSIB encoding is passed directly through to the IR operation.
	///
	/// Only supports the case of 32-bit data element sizes from 64-bit addresses
	IRPair<IROp_VLoadVectorGatherMaskedQPS> _VLoadVectorGatherMaskedQPS(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Incoming, OrderedNodeWrapper MaskReg, OrderedNodeWrapper AddrBase, OrderedNodeWrapper VectorIndexLow, OrderedNodeWrapper VectorIndexHigh, uint8_t OffsetScale, FEXCore::IR::OpSize AddrSize) {
		SaveNZCV(IROps::OP_VLOADVECTORGATHERMASKEDQPS);		auto _Op = AllocateOp<IROp_VLoadVectorGatherMaskedQPS, IROps::OP_VLOADVECTORGATHERMASKEDQPS>();
		_Op.first->Incoming = Incoming;
		_Op.first->MaskReg = MaskReg;
		_Op.first->AddrBase = AddrBase;
		_Op.first->VectorIndexLow = VectorIndexLow;
		_Op.first->VectorIndexHigh = VectorIndexHigh;
		_Op.first->OffsetScale = OffsetScale;
		_Op.first->AddrSize = AddrSize;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	/// Does a masked load similar to VPGATHERQPS where the upper bit of each element
	///
	/// determines whether or not that element will be loaded from memory.
	///
	/// Most of VSIB encoding is passed directly through to the IR operation.
	///
	/// Only supports the case of 32-bit data element sizes from 64-bit addresses
	IRPair<IROp_VLoadVectorGatherMaskedQPS> _VLoadVectorGatherMaskedQPS(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Incoming, OrderedNode *MaskReg, OrderedNode *AddrBase, OrderedNode *VectorIndexLow, OrderedNode *VectorIndexHigh, uint8_t OffsetScale, FEXCore::IR::OpSize AddrSize) {
		auto ListDataBegin = DualListData.ListBegin();
		Incoming->AddUse();
		MaskReg->AddUse();
		AddrBase->AddUse();
		VectorIndexLow->AddUse();
		VectorIndexHigh->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Incoming) == RegClass::Invalid || WalkFindRegClass(Incoming) == RegClass::FPR, "GetOpRegClass(Incoming) == RegClass::Invalid || WalkFindRegClass(Incoming) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(MaskReg) == RegClass::Invalid || WalkFindRegClass(MaskReg) == RegClass::FPR, "GetOpRegClass(MaskReg) == RegClass::Invalid || WalkFindRegClass(MaskReg) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(AddrBase) == RegClass::Invalid || WalkFindRegClass(AddrBase) == RegClass::GPR, "GetOpRegClass(AddrBase) == RegClass::Invalid || WalkFindRegClass(AddrBase) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(VectorIndexLow) == RegClass::Invalid || WalkFindRegClass(VectorIndexLow) == RegClass::FPR, "GetOpRegClass(VectorIndexLow) == RegClass::Invalid || WalkFindRegClass(VectorIndexLow) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(VectorIndexHigh) == RegClass::Invalid || WalkFindRegClass(VectorIndexHigh) == RegClass::FPR, "GetOpRegClass(VectorIndexHigh) == RegClass::Invalid || WalkFindRegClass(VectorIndexHigh) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(ElementSize == OpSize::i32Bit, "ElementSize == OpSize::i32Bit");
		LOGMAN_THROW_A_FMT(RegisterSize != FEXCore::IR::OpSize::i256Bit && "What does 256-bit mean in this context?", "RegisterSize != FEXCore::IR::OpSize::i256Bit && \"What does 256-bit mean in this context?\"");
#endif
		return _VLoadVectorGatherMaskedQPS(RegisterSize, ElementSize, Incoming->Wrapped(ListDataBegin), MaskReg->Wrapped(ListDataBegin), AddrBase->Wrapped(ListDataBegin), VectorIndexLow->Wrapped(ListDataBegin), VectorIndexHigh->Wrapped(ListDataBegin), OffsetScale, AddrSize);
	}

	/// Does a memory load to a single element of a vector.
	///
	/// Leaves the rest of the vector's data intact.
	///
	/// Matches arm64 ld1 semantics
	IRPair<IROp_VLoadVectorElement> _VLoadVectorElement(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper DstSrc, uint8_t Index, OrderedNodeWrapper Addr) {
		auto _Op = AllocateOp<IROp_VLoadVectorElement, IROps::OP_VLOADVECTORELEMENT>();
		_Op.first->DstSrc = DstSrc;
		_Op.first->Addr = Addr;
		_Op.first->Index = Index;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	/// Does a memory load to a single element of a vector.
	///
	/// Leaves the rest of the vector's data intact.
	///
	/// Matches arm64 ld1 semantics
	IRPair<IROp_VLoadVectorElement> _VLoadVectorElement(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *DstSrc, uint8_t Index, OrderedNode *Addr) {
		auto ListDataBegin = DualListData.ListBegin();
		DstSrc->AddUse();
		Addr->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(DstSrc) == RegClass::Invalid || WalkFindRegClass(DstSrc) == RegClass::FPR, "GetOpRegClass(DstSrc) == RegClass::Invalid || WalkFindRegClass(DstSrc) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Addr) == RegClass::Invalid || WalkFindRegClass(Addr) == RegClass::GPR, "GetOpRegClass(Addr) == RegClass::Invalid || WalkFindRegClass(Addr) == RegClass::GPR");
#endif
		return _VLoadVectorElement(RegisterSize, ElementSize, DstSrc->Wrapped(ListDataBegin), Index, Addr->Wrapped(ListDataBegin));
	}

	/// Does a memory store of a single element of a vector.
	///
	/// Matches arm64 st1 semantics
	IRPair<IROp_VStoreVectorElement> _VStoreVectorElement(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Value, uint8_t Index, OrderedNodeWrapper Addr) {
		auto _Op = AllocateOp<IROp_VStoreVectorElement, IROps::OP_VSTOREVECTORELEMENT>();
		_Op.first->Value = Value;
		_Op.first->Addr = Addr;
		_Op.first->Index = Index;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	/// Does a memory store of a single element of a vector.
	///
	/// Matches arm64 st1 semantics
	IRPair<IROp_VStoreVectorElement> _VStoreVectorElement(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Value, uint8_t Index, OrderedNode *Addr) {
		auto ListDataBegin = DualListData.ListBegin();
		Value->AddUse();
		Addr->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Value) == RegClass::Invalid || WalkFindRegClass(Value) == RegClass::FPR, "GetOpRegClass(Value) == RegClass::Invalid || WalkFindRegClass(Value) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Addr) == RegClass::Invalid || WalkFindRegClass(Addr) == RegClass::GPR, "GetOpRegClass(Addr) == RegClass::Invalid || WalkFindRegClass(Addr) == RegClass::GPR");
#endif
		return _VStoreVectorElement(RegisterSize, ElementSize, Value->Wrapped(ListDataBegin), Index, Addr->Wrapped(ListDataBegin));
	}

	/// Broadcasts an ElementSize value from memory into each element of a vector.
	IRPair<IROp_VBroadcastFromMem> _VBroadcastFromMem(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Address) {
		auto _Op = AllocateOp<IROp_VBroadcastFromMem, IROps::OP_VBROADCASTFROMMEM>();
		_Op.first->Address = Address;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	/// Broadcasts an ElementSize value from memory into each element of a vector.
	IRPair<IROp_VBroadcastFromMem> _VBroadcastFromMem(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Address) {
		auto ListDataBegin = DualListData.ListBegin();
		Address->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Address) == RegClass::Invalid || WalkFindRegClass(Address) == RegClass::GPR, "GetOpRegClass(Address) == RegClass::Invalid || WalkFindRegClass(Address) == RegClass::GPR");
#endif
		return _VBroadcastFromMem(RegisterSize, ElementSize, Address->Wrapped(ListDataBegin));
	}

	/// Pushes a value to the address, returning the new pointer after incrementing.
	///
	/// The address is decremented by the value size while.
	///
	/// The return value size is the size of the current operating mode
	IRPair<IROp_Push> _Push(FEXCore::IR::OpSize Size, FEXCore::IR::OpSize ValueSize, OrderedNodeWrapper Value, OrderedNodeWrapper Addr) {
		auto _Op = AllocateOp<IROp_Push, IROps::OP_PUSH>();
		_Op.first->Value = Value;
		_Op.first->Addr = Addr;
		_Op.first->ValueSize = ValueSize;
		_Op.first->Header.Size = Size;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Pushes a value to the address, returning the new pointer after incrementing.
	///
	/// The address is decremented by the value size while.
	///
	/// The return value size is the size of the current operating mode
	IRPair<IROp_Push> _Push(FEXCore::IR::OpSize Size, FEXCore::IR::OpSize ValueSize, OrderedNode *Value, OrderedNode *Addr) {
		auto ListDataBegin = DualListData.ListBegin();
		Value->AddUse();
		Addr->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Value) == RegClass::Invalid || WalkFindRegClass(Value) == RegClass::GPR, "GetOpRegClass(Value) == RegClass::Invalid || WalkFindRegClass(Value) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Addr) == RegClass::Invalid || WalkFindRegClass(Addr) == RegClass::GPR, "GetOpRegClass(Addr) == RegClass::Invalid || WalkFindRegClass(Addr) == RegClass::GPR");
#endif
		return _Push(Size, ValueSize, Value->Wrapped(ListDataBegin), Addr->Wrapped(ListDataBegin));
	}

	/// Push two values to the address, incrementing the pointer in the place.
	///
	/// Fused post-RA so doesn't have a destination.
	IRPair<IROp_PushTwo> _PushTwo(FEXCore::IR::OpSize Size, FEXCore::IR::OpSize ValueSize, OrderedNodeWrapper Value1, OrderedNodeWrapper Value2, OrderedNodeWrapper Addr) {
		auto _Op = AllocateOp<IROp_PushTwo, IROps::OP_PUSHTWO>();
		_Op.first->Value1 = Value1;
		_Op.first->Value2 = Value2;
		_Op.first->Addr = Addr;
		_Op.first->ValueSize = ValueSize;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Push two values to the address, incrementing the pointer in the place.
	///
	/// Fused post-RA so doesn't have a destination.
	IRPair<IROp_PushTwo> _PushTwo(FEXCore::IR::OpSize Size, FEXCore::IR::OpSize ValueSize, OrderedNode *Value1, OrderedNode *Value2, OrderedNode *Addr) {
		auto ListDataBegin = DualListData.ListBegin();
		Value1->AddUse();
		Value2->AddUse();
		Addr->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Value1) == RegClass::Invalid || WalkFindRegClass(Value1) == RegClass::GPR, "GetOpRegClass(Value1) == RegClass::Invalid || WalkFindRegClass(Value1) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Value2) == RegClass::Invalid || WalkFindRegClass(Value2) == RegClass::GPR, "GetOpRegClass(Value2) == RegClass::Invalid || WalkFindRegClass(Value2) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Addr) == RegClass::Invalid || WalkFindRegClass(Addr) == RegClass::GPR, "GetOpRegClass(Addr) == RegClass::Invalid || WalkFindRegClass(Addr) == RegClass::GPR");
#endif
		return _PushTwo(Size, ValueSize, Value1->Wrapped(ListDataBegin), Value2->Wrapped(ListDataBegin), Addr->Wrapped(ListDataBegin));
	}

	/// This is a special move that indicates the result will be poisoned by a non-SSA instruction writing to its result.
	///
	/// In effect, it serves to prevent invalid optimizations with non-SSA instructions.
	IRPair<IROp_RMWHandle> _RMWHandle(OrderedNodeWrapper Value) {
		auto _Op = AllocateOp<IROp_RMWHandle, IROps::OP_RMWHANDLE>();
		_Op.first->Value = Value;
		_Op.first->Header.Size = OpSize::i64Bit;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// This is a special move that indicates the result will be poisoned by a non-SSA instruction writing to its result.
	///
	/// In effect, it serves to prevent invalid optimizations with non-SSA instructions.
	IRPair<IROp_RMWHandle> _RMWHandle(OrderedNode *Value) {
		auto ListDataBegin = DualListData.ListBegin();
		Value->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Value) == RegClass::Invalid || WalkFindRegClass(Value) == RegClass::GPR, "GetOpRegClass(Value) == RegClass::Invalid || WalkFindRegClass(Value) == RegClass::GPR");
#endif
		return _RMWHandle(Value->Wrapped(ListDataBegin));
	}

	/// Pops a value from the address, updating the new pointer after incrementing.
	///
	/// The address is incremented by the size via an RMW source/destintaion.
	IRPair<IROp_Pop> _Pop(FEXCore::IR::OpSize Size, OrderedNodeWrapper InoutAddr, OrderedNodeWrapper OutValue) {
		auto _Op = AllocateOp<IROp_Pop, IROps::OP_POP>();
		_Op.first->InoutAddr = InoutAddr;
		_Op.first->OutValue = OutValue;
		_Op.first->Size = Size;
		_Op.first->Header.Size = Size;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Pops a value from the address, updating the new pointer after incrementing.
	///
	/// The address is incremented by the size via an RMW source/destintaion.
	IRPair<IROp_Pop> _Pop(FEXCore::IR::OpSize Size, OrderedNode *InoutAddr, OrderedNode *OutValue) {
		auto ListDataBegin = DualListData.ListBegin();
		InoutAddr->AddUse();
		OutValue->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(InoutAddr) == RegClass::Invalid || WalkFindRegClass(InoutAddr) == RegClass::GPR, "GetOpRegClass(InoutAddr) == RegClass::Invalid || WalkFindRegClass(InoutAddr) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(OutValue) == RegClass::Invalid || WalkFindRegClass(OutValue) == RegClass::GPR, "GetOpRegClass(OutValue) == RegClass::Invalid || WalkFindRegClass(OutValue) == RegClass::GPR");
#endif
		return _Pop(Size, InoutAddr->Wrapped(ListDataBegin), OutValue->Wrapped(ListDataBegin));
	}

	/// Pop two values from the address. Fused post-RA.
	IRPair<IROp_PopTwo> _PopTwo(FEXCore::IR::OpSize Size, OrderedNodeWrapper InoutAddr, OrderedNodeWrapper OutValue1, OrderedNodeWrapper OutValue2) {
		auto _Op = AllocateOp<IROp_PopTwo, IROps::OP_POPTWO>();
		_Op.first->InoutAddr = InoutAddr;
		_Op.first->OutValue1 = OutValue1;
		_Op.first->OutValue2 = OutValue2;
		_Op.first->Size = Size;
		_Op.first->Header.Size = Size;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Pop two values from the address. Fused post-RA.
	IRPair<IROp_PopTwo> _PopTwo(FEXCore::IR::OpSize Size, OrderedNode *InoutAddr, OrderedNode *OutValue1, OrderedNode *OutValue2) {
		auto ListDataBegin = DualListData.ListBegin();
		InoutAddr->AddUse();
		OutValue1->AddUse();
		OutValue2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(InoutAddr) == RegClass::Invalid || WalkFindRegClass(InoutAddr) == RegClass::GPR, "GetOpRegClass(InoutAddr) == RegClass::Invalid || WalkFindRegClass(InoutAddr) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(OutValue1) == RegClass::Invalid || WalkFindRegClass(OutValue1) == RegClass::GPR, "GetOpRegClass(OutValue1) == RegClass::Invalid || WalkFindRegClass(OutValue1) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(OutValue2) == RegClass::Invalid || WalkFindRegClass(OutValue2) == RegClass::GPR, "GetOpRegClass(OutValue2) == RegClass::Invalid || WalkFindRegClass(OutValue2) == RegClass::GPR");
#endif
		return _PopTwo(Size, InoutAddr->Wrapped(ListDataBegin), OutValue1->Wrapped(ListDataBegin), OutValue2->Wrapped(ListDataBegin));
	}

	/// Duplicates behaviour of x86 STOS repeat
	///
	/// Returns the final address that gets generated without the prefix appended.
	IRPair<IROp_MemSet> _MemSet(bool IsAtomic, FEXCore::IR::OpSize Size, OrderedNodeWrapper Prefix, OrderedNodeWrapper Addr, OrderedNodeWrapper Value, OrderedNodeWrapper Length, OrderedNodeWrapper Direction) {
		auto _Op = AllocateOp<IROp_MemSet, IROps::OP_MEMSET>();
		_Op.first->Prefix = Prefix;
		_Op.first->Addr = Addr;
		_Op.first->Value = Value;
		_Op.first->Length = Length;
		_Op.first->Direction = Direction;
		_Op.first->IsAtomic = IsAtomic;
		_Op.first->Size = Size;
		_Op.first->Header.Size = OpSize::i64Bit;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Duplicates behaviour of x86 STOS repeat
	///
	/// Returns the final address that gets generated without the prefix appended.
	IRPair<IROp_MemSet> _MemSet(bool IsAtomic, FEXCore::IR::OpSize Size, OrderedNode *Prefix, OrderedNode *Addr, OrderedNode *Value, OrderedNode *Length, OrderedNode *Direction) {
		auto ListDataBegin = DualListData.ListBegin();
		Prefix->AddUse();
		Addr->AddUse();
		Value = InlineZero(Size, Value);
		Value->AddUse();
		Length->AddUse();
		Direction = InlineAny(Size, Direction);
		Direction->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Prefix) == RegClass::Invalid || WalkFindRegClass(Prefix) == RegClass::GPR, "GetOpRegClass(Prefix) == RegClass::Invalid || WalkFindRegClass(Prefix) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Addr) == RegClass::Invalid || WalkFindRegClass(Addr) == RegClass::GPR, "GetOpRegClass(Addr) == RegClass::Invalid || WalkFindRegClass(Addr) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Value) == RegClass::Invalid || WalkFindRegClass(Value) == RegClass::GPR, "GetOpRegClass(Value) == RegClass::Invalid || WalkFindRegClass(Value) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Length) == RegClass::Invalid || WalkFindRegClass(Length) == RegClass::GPR, "GetOpRegClass(Length) == RegClass::Invalid || WalkFindRegClass(Length) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Direction) == RegClass::Invalid || WalkFindRegClass(Direction) == RegClass::GPR, "GetOpRegClass(Direction) == RegClass::Invalid || WalkFindRegClass(Direction) == RegClass::GPR");
#endif
		return _MemSet(IsAtomic, Size, Prefix->Wrapped(ListDataBegin), Addr->Wrapped(ListDataBegin), Value->Wrapped(ListDataBegin), Length->Wrapped(ListDataBegin), Direction->Wrapped(ListDataBegin));
	}

	/// Duplicates behaviour of x86 MOVS repeat
	///
	/// Returns the final addresses after they have been incremented or decremented
	IRPair<IROp_MemCpy> _MemCpy(bool IsAtomic, FEXCore::IR::OpSize Size, OrderedNodeWrapper Dest, OrderedNodeWrapper Src, OrderedNodeWrapper Length, OrderedNodeWrapper Direction, OrderedNodeWrapper OutDstAddress, OrderedNodeWrapper OutSrcAddress) {
		auto _Op = AllocateOp<IROp_MemCpy, IROps::OP_MEMCPY>();
		_Op.first->Dest = Dest;
		_Op.first->Src = Src;
		_Op.first->Length = Length;
		_Op.first->Direction = Direction;
		_Op.first->OutDstAddress = OutDstAddress;
		_Op.first->OutSrcAddress = OutSrcAddress;
		_Op.first->IsAtomic = IsAtomic;
		_Op.first->Size = Size;
		_Op.first->Header.Size = OpSize::i64Bit;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Duplicates behaviour of x86 MOVS repeat
	///
	/// Returns the final addresses after they have been incremented or decremented
	IRPair<IROp_MemCpy> _MemCpy(bool IsAtomic, FEXCore::IR::OpSize Size, OrderedNode *Dest, OrderedNode *Src, OrderedNode *Length, OrderedNode *Direction, OrderedNode *OutDstAddress, OrderedNode *OutSrcAddress) {
		auto ListDataBegin = DualListData.ListBegin();
		Dest->AddUse();
		Src->AddUse();
		Length->AddUse();
		Direction = InlineAny(Size, Direction);
		Direction->AddUse();
		OutDstAddress->AddUse();
		OutSrcAddress->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Dest) == RegClass::Invalid || WalkFindRegClass(Dest) == RegClass::GPR, "GetOpRegClass(Dest) == RegClass::Invalid || WalkFindRegClass(Dest) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src) == RegClass::Invalid || WalkFindRegClass(Src) == RegClass::GPR, "GetOpRegClass(Src) == RegClass::Invalid || WalkFindRegClass(Src) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Length) == RegClass::Invalid || WalkFindRegClass(Length) == RegClass::GPR, "GetOpRegClass(Length) == RegClass::Invalid || WalkFindRegClass(Length) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Direction) == RegClass::Invalid || WalkFindRegClass(Direction) == RegClass::GPR, "GetOpRegClass(Direction) == RegClass::Invalid || WalkFindRegClass(Direction) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(OutDstAddress) == RegClass::Invalid || WalkFindRegClass(OutDstAddress) == RegClass::GPR, "GetOpRegClass(OutDstAddress) == RegClass::Invalid || WalkFindRegClass(OutDstAddress) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(OutSrcAddress) == RegClass::Invalid || WalkFindRegClass(OutSrcAddress) == RegClass::GPR, "GetOpRegClass(OutSrcAddress) == RegClass::Invalid || WalkFindRegClass(OutSrcAddress) == RegClass::GPR");
#endif
		return _MemCpy(IsAtomic, Size, Dest->Wrapped(ListDataBegin), Src->Wrapped(ListDataBegin), Length->Wrapped(ListDataBegin), Direction->Wrapped(ListDataBegin), OutDstAddress->Wrapped(ListDataBegin), OutSrcAddress->Wrapped(ListDataBegin));
	}

	/// Does a 64 byte cacheline clear at the address specified
	///
	/// Only clears the data cachelines. Doesn't do any zeroing
	///
	/// Can skip serialization if requested.
	IRPair<IROp_CacheLineClear> _CacheLineClear(OrderedNodeWrapper Addr, bool Serialize) {
		auto _Op = AllocateOp<IROp_CacheLineClear, IROps::OP_CACHELINECLEAR>();
		_Op.first->Addr = Addr;
		_Op.first->Serialize = Serialize;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Does a 64 byte cacheline clear at the address specified
	///
	/// Only clears the data cachelines. Doesn't do any zeroing
	///
	/// Can skip serialization if requested.
	IRPair<IROp_CacheLineClear> _CacheLineClear(OrderedNode *Addr, bool Serialize) {
		auto ListDataBegin = DualListData.ListBegin();
		Addr->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Addr) == RegClass::Invalid || WalkFindRegClass(Addr) == RegClass::GPR, "GetOpRegClass(Addr) == RegClass::Invalid || WalkFindRegClass(Addr) == RegClass::GPR");
#endif
		return _CacheLineClear(Addr->Wrapped(ListDataBegin), Serialize);
	}

	/// Does a 64 byte cacheline clean at the address specified
	///
	/// Only cleans the data cachelines. Doesn't do any zeroing
	///
	/// Skips the invalidation step of the CacheLineClear operation
	IRPair<IROp_CacheLineClean> _CacheLineClean(OrderedNodeWrapper Addr) {
		auto _Op = AllocateOp<IROp_CacheLineClean, IROps::OP_CACHELINECLEAN>();
		_Op.first->Addr = Addr;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Does a 64 byte cacheline clean at the address specified
	///
	/// Only cleans the data cachelines. Doesn't do any zeroing
	///
	/// Skips the invalidation step of the CacheLineClear operation
	IRPair<IROp_CacheLineClean> _CacheLineClean(OrderedNode *Addr) {
		auto ListDataBegin = DualListData.ListBegin();
		Addr->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Addr) == RegClass::Invalid || WalkFindRegClass(Addr) == RegClass::GPR, "GetOpRegClass(Addr) == RegClass::Invalid || WalkFindRegClass(Addr) == RegClass::GPR");
#endif
		return _CacheLineClean(Addr->Wrapped(ListDataBegin));
	}

	/// Does a 64 byte zero at the address specified
	///
	/// Writing zeroes to memory
	///
	/// It is specifically non-temporal and weakly ordered
	///
	/// This matches CLZero behaviour
	IRPair<IROp_CacheLineZero> _CacheLineZero(OrderedNodeWrapper Addr) {
		auto _Op = AllocateOp<IROp_CacheLineZero, IROps::OP_CACHELINEZERO>();
		_Op.first->Addr = Addr;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Does a 64 byte zero at the address specified
	///
	/// Writing zeroes to memory
	///
	/// It is specifically non-temporal and weakly ordered
	///
	/// This matches CLZero behaviour
	IRPair<IROp_CacheLineZero> _CacheLineZero(OrderedNode *Addr) {
		auto ListDataBegin = DualListData.ListBegin();
		Addr->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Addr) == RegClass::Invalid || WalkFindRegClass(Addr) == RegClass::GPR, "GetOpRegClass(Addr) == RegClass::Invalid || WalkFindRegClass(Addr) == RegClass::GPR");
#endif
		return _CacheLineZero(Addr->Wrapped(ListDataBegin));
	}

	/// Does a memory fence operation of the desired type
	///
	/// FenceType::Load: Ensures load memory operations are serialized
	///
	/// FenceType::Store: Ensures store memory operations are serialized
	///
	/// FenceType::LoadStore: Ensures loads and store memory operations are serialized
	///
	/// FenceType::Inst: Instruction barrier. Ensures all instructions after this point will be explicitly fetched
	///
	/// Ensures the memory operations are globally visible
	IRPair<IROp_Fence> _Fence(FenceType Fence) {
		auto _Op = AllocateOp<IROp_Fence, IROps::OP_FENCE>();
		_Op.first->Fence = Fence;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Does a cacheline prefetch operation
	IRPair<IROp_Prefetch> _Prefetch(bool ForStore, bool Stream, int8_t CacheLevel, OrderedNodeWrapper Addr, OrderedNodeWrapper Offset, MemOffsetType OffsetType, uint8_t OffsetScale) {
		auto _Op = AllocateOp<IROp_Prefetch, IROps::OP_PREFETCH>();
		_Op.first->Addr = Addr;
		_Op.first->Offset = Offset;
		_Op.first->ForStore = ForStore;
		_Op.first->Stream = Stream;
		_Op.first->CacheLevel = CacheLevel;
		_Op.first->OffsetType = OffsetType;
		_Op.first->OffsetScale = OffsetScale;
		_Op.first->Header.Size = OpSize::i64Bit;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Does a cacheline prefetch operation
	IRPair<IROp_Prefetch> _Prefetch(bool ForStore, bool Stream, int8_t CacheLevel, OrderedNode *Addr, OrderedNode *Offset, MemOffsetType OffsetType, uint8_t OffsetScale) {
		auto ListDataBegin = DualListData.ListBegin();
		Addr->AddUse();
		Offset = InlineMem(OpSize::i64Bit, Offset, OffsetType, OffsetScale);
		Offset->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Addr) == RegClass::Invalid || WalkFindRegClass(Addr) == RegClass::GPR, "GetOpRegClass(Addr) == RegClass::Invalid || WalkFindRegClass(Addr) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Offset) == RegClass::Invalid || WalkFindRegClass(Offset) == RegClass::GPR, "GetOpRegClass(Offset) == RegClass::Invalid || WalkFindRegClass(Offset) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(CacheLevel > 0 && CacheLevel < 4, "CacheLevel > 0 && CacheLevel < 4");
#endif
		return _Prefetch(ForStore, Stream, CacheLevel, Addr->Wrapped(ListDataBegin), Offset->Wrapped(ListDataBegin), OffsetType, OffsetScale);
	}

	/// Does a non-temporal memory store of a vector.
	///
	/// Matches arm64 SVE stnt1b semantics.
	///
	/// Specifically weak-memory model ordered to match x86 non-temporal stores.
	IRPair<IROp_VStoreNonTemporal> _VStoreNonTemporal(FEXCore::IR::OpSize RegisterSize, OrderedNodeWrapper Value, OrderedNodeWrapper Addr, int8_t Offset) {
		auto _Op = AllocateOp<IROp_VStoreNonTemporal, IROps::OP_VSTORENONTEMPORAL>();
		_Op.first->Value = Value;
		_Op.first->Addr = Addr;
		_Op.first->Offset = Offset;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Does a non-temporal memory store of a vector.
	///
	/// Matches arm64 SVE stnt1b semantics.
	///
	/// Specifically weak-memory model ordered to match x86 non-temporal stores.
	IRPair<IROp_VStoreNonTemporal> _VStoreNonTemporal(FEXCore::IR::OpSize RegisterSize, OrderedNode *Value, OrderedNode *Addr, int8_t Offset) {
		auto ListDataBegin = DualListData.ListBegin();
		Value->AddUse();
		Addr->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Value) == RegClass::Invalid || WalkFindRegClass(Value) == RegClass::FPR, "GetOpRegClass(Value) == RegClass::Invalid || WalkFindRegClass(Value) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Addr) == RegClass::Invalid || WalkFindRegClass(Addr) == RegClass::GPR, "GetOpRegClass(Addr) == RegClass::Invalid || WalkFindRegClass(Addr) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(RegisterSize == FEXCore::IR::OpSize::i128Bit || RegisterSize == FEXCore::IR::OpSize::i256Bit, "RegisterSize == FEXCore::IR::OpSize::i128Bit || RegisterSize == FEXCore::IR::OpSize::i256Bit");
		LOGMAN_THROW_A_FMT(Offset % IR::OpSizeToSize(RegisterSize) == 0, "Offset % IR::OpSizeToSize(RegisterSize) == 0");
#endif
		return _VStoreNonTemporal(RegisterSize, Value->Wrapped(ListDataBegin), Addr->Wrapped(ListDataBegin), Offset);
	}

	/// Does a non-temporal memory store of two vector registers.
	///
	/// Matches arm64 stnp semantics.
	///
	/// Specifically weak-memory model ordered to match x86 non-temporal stores.
	IRPair<IROp_VStoreNonTemporalPair> _VStoreNonTemporalPair(FEXCore::IR::OpSize RegisterSize, OrderedNodeWrapper ValueLow, OrderedNodeWrapper ValueHigh, OrderedNodeWrapper Addr, int8_t Offset) {
		auto _Op = AllocateOp<IROp_VStoreNonTemporalPair, IROps::OP_VSTORENONTEMPORALPAIR>();
		_Op.first->ValueLow = ValueLow;
		_Op.first->ValueHigh = ValueHigh;
		_Op.first->Addr = Addr;
		_Op.first->Offset = Offset;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Does a non-temporal memory store of two vector registers.
	///
	/// Matches arm64 stnp semantics.
	///
	/// Specifically weak-memory model ordered to match x86 non-temporal stores.
	IRPair<IROp_VStoreNonTemporalPair> _VStoreNonTemporalPair(FEXCore::IR::OpSize RegisterSize, OrderedNode *ValueLow, OrderedNode *ValueHigh, OrderedNode *Addr, int8_t Offset) {
		auto ListDataBegin = DualListData.ListBegin();
		ValueLow->AddUse();
		ValueHigh->AddUse();
		Addr->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(ValueLow) == RegClass::Invalid || WalkFindRegClass(ValueLow) == RegClass::FPR, "GetOpRegClass(ValueLow) == RegClass::Invalid || WalkFindRegClass(ValueLow) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(ValueHigh) == RegClass::Invalid || WalkFindRegClass(ValueHigh) == RegClass::FPR, "GetOpRegClass(ValueHigh) == RegClass::Invalid || WalkFindRegClass(ValueHigh) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Addr) == RegClass::Invalid || WalkFindRegClass(Addr) == RegClass::GPR, "GetOpRegClass(Addr) == RegClass::Invalid || WalkFindRegClass(Addr) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(RegisterSize == FEXCore::IR::OpSize::i128Bit, "RegisterSize == FEXCore::IR::OpSize::i128Bit");
		LOGMAN_THROW_A_FMT(Offset % IR::OpSizeToSize(RegisterSize) == 0, "Offset % IR::OpSizeToSize(RegisterSize) == 0");
#endif
		return _VStoreNonTemporalPair(RegisterSize, ValueLow->Wrapped(ListDataBegin), ValueHigh->Wrapped(ListDataBegin), Addr->Wrapped(ListDataBegin), Offset);
	}

	/// Does a non-temporal memory load of a vector.
	///
	/// Matches arm64 SVE ldnt1b semantics.
	///
	/// Specifically weak-memory model ordered to match x86 non-temporal stores.
	IRPair<IROp_VLoadNonTemporal> _VLoadNonTemporal(FEXCore::IR::OpSize RegisterSize, OrderedNodeWrapper Addr, int8_t Offset) {
		auto _Op = AllocateOp<IROp_VLoadNonTemporal, IROps::OP_VLOADNONTEMPORAL>();
		_Op.first->Addr = Addr;
		_Op.first->Offset = Offset;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Does a non-temporal memory load of a vector.
	///
	/// Matches arm64 SVE ldnt1b semantics.
	///
	/// Specifically weak-memory model ordered to match x86 non-temporal stores.
	IRPair<IROp_VLoadNonTemporal> _VLoadNonTemporal(FEXCore::IR::OpSize RegisterSize, OrderedNode *Addr, int8_t Offset) {
		auto ListDataBegin = DualListData.ListBegin();
		Addr->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Addr) == RegClass::Invalid || WalkFindRegClass(Addr) == RegClass::GPR, "GetOpRegClass(Addr) == RegClass::Invalid || WalkFindRegClass(Addr) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(RegisterSize == FEXCore::IR::OpSize::i128Bit || RegisterSize == FEXCore::IR::OpSize::i256Bit, "RegisterSize == FEXCore::IR::OpSize::i128Bit || RegisterSize == FEXCore::IR::OpSize::i256Bit");
		LOGMAN_THROW_A_FMT(Offset % IR::OpSizeToSize(RegisterSize) == 0, "Offset % IR::OpSizeToSize(RegisterSize) == 0");
#endif
		return _VLoadNonTemporal(RegisterSize, Addr->Wrapped(ListDataBegin), Offset);
	}

	/// Clears a region of the context by CLZero size
	///
	/// Both the offset and size alignment need to be by CLZero size
	IRPair<IROp_ContextClear> _ContextClear(uint32_t Offset, uint32_t Size) {
		auto _Op = AllocateOp<IROp_ContextClear, IROps::OP_CONTEXTCLEAR>();
		_Op.first->Offset = Offset;
		_Op.first->Size = Size;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(Offset % 64 == 0, "Offset % 64 == 0");
		LOGMAN_THROW_A_FMT(Size % 64 == 0, "Size % 64 == 0");
#endif
		return _Op;
	}

	/// Does a compare and swap of values to a memory location
	///
	/// This mostly matches the C++ atomic_compare_exchange_strong function
	///
	/// Dest = atomic_compare_exchange_strong(%Addr, %Expected, %Desired)
	///
	/// Depending on if the value in %Addr is Expected the results destination will be different
	///
	/// Behaves like the following but atomically
	///
	/// Dest = %Expected
	///
	/// if (deref(%Addr) != %Expected) Dest = deref(%Addr)
	IRPair<IROp_CAS> _CAS(FEXCore::IR::OpSize Size, OrderedNodeWrapper Expected, OrderedNodeWrapper Desired, OrderedNodeWrapper Addr) {
		SaveNZCV(IROps::OP_CAS);		auto _Op = AllocateOp<IROp_CAS, IROps::OP_CAS>();
		_Op.first->Expected = Expected;
		_Op.first->Desired = Desired;
		_Op.first->Addr = Addr;
		_Op.first->Header.Size = Size;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Does a compare and swap of values to a memory location
	///
	/// This mostly matches the C++ atomic_compare_exchange_strong function
	///
	/// Dest = atomic_compare_exchange_strong(%Addr, %Expected, %Desired)
	///
	/// Depending on if the value in %Addr is Expected the results destination will be different
	///
	/// Behaves like the following but atomically
	///
	/// Dest = %Expected
	///
	/// if (deref(%Addr) != %Expected) Dest = deref(%Addr)
	IRPair<IROp_CAS> _CAS(FEXCore::IR::OpSize Size, OrderedNode *Expected, OrderedNode *Desired, OrderedNode *Addr) {
		auto ListDataBegin = DualListData.ListBegin();
		Expected->AddUse();
		Desired->AddUse();
		Addr->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Expected) == RegClass::Invalid || WalkFindRegClass(Expected) == RegClass::GPR, "GetOpRegClass(Expected) == RegClass::Invalid || WalkFindRegClass(Expected) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Desired) == RegClass::Invalid || WalkFindRegClass(Desired) == RegClass::GPR, "GetOpRegClass(Desired) == RegClass::Invalid || WalkFindRegClass(Desired) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Addr) == RegClass::Invalid || WalkFindRegClass(Addr) == RegClass::GPR, "GetOpRegClass(Addr) == RegClass::Invalid || WalkFindRegClass(Addr) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(Size == FEXCore::IR::OpSize::i8Bit || Size == FEXCore::IR::OpSize::i16Bit || Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit, "Size == FEXCore::IR::OpSize::i8Bit || Size == FEXCore::IR::OpSize::i16Bit || Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit");
#endif
		return _CAS(Size, Expected->Wrapped(ListDataBegin), Desired->Wrapped(ListDataBegin), Addr->Wrapped(ListDataBegin));
	}

	/// Does a compare and exchange with two pairs of values
	///
	/// ssa0 is the comparison value
	///
	/// ssa1 is the new value
	///
	/// ssa2 is the memory location
	///
	/// Returns the lower & upper halves of the value in memory.
	IRPair<IROp_CASPair> _CASPair(FEXCore::IR::OpSize Size, OrderedNodeWrapper ExpectedLo, OrderedNodeWrapper ExpectedHi, OrderedNodeWrapper DesiredLo, OrderedNodeWrapper DesiredHi, OrderedNodeWrapper Addr, OrderedNodeWrapper OutLo, OrderedNodeWrapper OutHi) {
		auto _Op = AllocateOp<IROp_CASPair, IROps::OP_CASPAIR>();
		_Op.first->ExpectedLo = ExpectedLo;
		_Op.first->ExpectedHi = ExpectedHi;
		_Op.first->DesiredLo = DesiredLo;
		_Op.first->DesiredHi = DesiredHi;
		_Op.first->Addr = Addr;
		_Op.first->OutLo = OutLo;
		_Op.first->OutHi = OutHi;
		_Op.first->Header.Size = Size;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Does a compare and exchange with two pairs of values
	///
	/// ssa0 is the comparison value
	///
	/// ssa1 is the new value
	///
	/// ssa2 is the memory location
	///
	/// Returns the lower & upper halves of the value in memory.
	IRPair<IROp_CASPair> _CASPair(FEXCore::IR::OpSize Size, OrderedNode *ExpectedLo, OrderedNode *ExpectedHi, OrderedNode *DesiredLo, OrderedNode *DesiredHi, OrderedNode *Addr, OrderedNode *OutLo, OrderedNode *OutHi) {
		auto ListDataBegin = DualListData.ListBegin();
		ExpectedLo->AddUse();
		ExpectedHi->AddUse();
		DesiredLo->AddUse();
		DesiredHi->AddUse();
		Addr->AddUse();
		OutLo->AddUse();
		OutHi->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(ExpectedLo) == RegClass::Invalid || WalkFindRegClass(ExpectedLo) == RegClass::GPR, "GetOpRegClass(ExpectedLo) == RegClass::Invalid || WalkFindRegClass(ExpectedLo) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(ExpectedHi) == RegClass::Invalid || WalkFindRegClass(ExpectedHi) == RegClass::GPR, "GetOpRegClass(ExpectedHi) == RegClass::Invalid || WalkFindRegClass(ExpectedHi) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(DesiredLo) == RegClass::Invalid || WalkFindRegClass(DesiredLo) == RegClass::GPR, "GetOpRegClass(DesiredLo) == RegClass::Invalid || WalkFindRegClass(DesiredLo) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(DesiredHi) == RegClass::Invalid || WalkFindRegClass(DesiredHi) == RegClass::GPR, "GetOpRegClass(DesiredHi) == RegClass::Invalid || WalkFindRegClass(DesiredHi) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Addr) == RegClass::Invalid || WalkFindRegClass(Addr) == RegClass::GPR, "GetOpRegClass(Addr) == RegClass::Invalid || WalkFindRegClass(Addr) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(OutLo) == RegClass::Invalid || WalkFindRegClass(OutLo) == RegClass::GPR, "GetOpRegClass(OutLo) == RegClass::Invalid || WalkFindRegClass(OutLo) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(OutHi) == RegClass::Invalid || WalkFindRegClass(OutHi) == RegClass::GPR, "GetOpRegClass(OutHi) == RegClass::Invalid || WalkFindRegClass(OutHi) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit, "Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit");
#endif
		return _CASPair(Size, ExpectedLo->Wrapped(ListDataBegin), ExpectedHi->Wrapped(ListDataBegin), DesiredLo->Wrapped(ListDataBegin), DesiredHi->Wrapped(ListDataBegin), Addr->Wrapped(ListDataBegin), OutLo->Wrapped(ListDataBegin), OutHi->Wrapped(ListDataBegin));
	}

	/// Atomic integer swap
	IRPair<IROp_AtomicSwap> _AtomicSwap(FEXCore::IR::OpSize Size, OrderedNodeWrapper Value, OrderedNodeWrapper Addr) {
		auto _Op = AllocateOp<IROp_AtomicSwap, IROps::OP_ATOMICSWAP>();
		_Op.first->Value = Value;
		_Op.first->Addr = Addr;
		_Op.first->Header.Size = Size;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Atomic integer swap
	IRPair<IROp_AtomicSwap> _AtomicSwap(FEXCore::IR::OpSize Size, OrderedNode *Value, OrderedNode *Addr) {
		auto ListDataBegin = DualListData.ListBegin();
		Value->AddUse();
		Addr->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Value) == RegClass::Invalid || WalkFindRegClass(Value) == RegClass::GPR, "GetOpRegClass(Value) == RegClass::Invalid || WalkFindRegClass(Value) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Addr) == RegClass::Invalid || WalkFindRegClass(Addr) == RegClass::GPR, "GetOpRegClass(Addr) == RegClass::Invalid || WalkFindRegClass(Addr) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(Size == FEXCore::IR::OpSize::i8Bit || Size == FEXCore::IR::OpSize::i16Bit || Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit, "Size == FEXCore::IR::OpSize::i8Bit || Size == FEXCore::IR::OpSize::i16Bit || Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit");
#endif
		return _AtomicSwap(Size, Value->Wrapped(ListDataBegin), Addr->Wrapped(ListDataBegin));
	}

	/// Atomic integer fetch and add
	///
	/// Atomically fetches %Addr and adds %value to the memory location
	///
	/// Dest is the value prior to operating on the value in memory
	///
	/// IR layout must match NonFetch-variant, otherwise DCE IR optimization breaks!
	IRPair<IROp_AtomicFetchAdd> _AtomicFetchAdd(FEXCore::IR::OpSize Size, OrderedNodeWrapper Value, OrderedNodeWrapper Addr) {
		auto _Op = AllocateOp<IROp_AtomicFetchAdd, IROps::OP_ATOMICFETCHADD>();
		_Op.first->Value = Value;
		_Op.first->Addr = Addr;
		_Op.first->Header.Size = Size;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Atomic integer fetch and add
	///
	/// Atomically fetches %Addr and adds %value to the memory location
	///
	/// Dest is the value prior to operating on the value in memory
	///
	/// IR layout must match NonFetch-variant, otherwise DCE IR optimization breaks!
	IRPair<IROp_AtomicFetchAdd> _AtomicFetchAdd(FEXCore::IR::OpSize Size, OrderedNode *Value, OrderedNode *Addr) {
		auto ListDataBegin = DualListData.ListBegin();
		Value->AddUse();
		Addr->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Value) == RegClass::Invalid || WalkFindRegClass(Value) == RegClass::GPR, "GetOpRegClass(Value) == RegClass::Invalid || WalkFindRegClass(Value) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Addr) == RegClass::Invalid || WalkFindRegClass(Addr) == RegClass::GPR, "GetOpRegClass(Addr) == RegClass::Invalid || WalkFindRegClass(Addr) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(Size == FEXCore::IR::OpSize::i8Bit || Size == FEXCore::IR::OpSize::i16Bit || Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit, "Size == FEXCore::IR::OpSize::i8Bit || Size == FEXCore::IR::OpSize::i16Bit || Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit");
#endif
		return _AtomicFetchAdd(Size, Value->Wrapped(ListDataBegin), Addr->Wrapped(ListDataBegin));
	}

	/// Atomic integer fetch and sub
	///
	/// Atomically fetches %Addr and subtracts %value to the memory location
	///
	/// Dest is the value prior to operating on the value in memory
	///
	/// IR layout must match NonFetch-variant, otherwise DCE IR optimization breaks!
	IRPair<IROp_AtomicFetchSub> _AtomicFetchSub(FEXCore::IR::OpSize Size, OrderedNodeWrapper Value, OrderedNodeWrapper Addr) {
		auto _Op = AllocateOp<IROp_AtomicFetchSub, IROps::OP_ATOMICFETCHSUB>();
		_Op.first->Value = Value;
		_Op.first->Addr = Addr;
		_Op.first->Header.Size = Size;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Atomic integer fetch and sub
	///
	/// Atomically fetches %Addr and subtracts %value to the memory location
	///
	/// Dest is the value prior to operating on the value in memory
	///
	/// IR layout must match NonFetch-variant, otherwise DCE IR optimization breaks!
	IRPair<IROp_AtomicFetchSub> _AtomicFetchSub(FEXCore::IR::OpSize Size, OrderedNode *Value, OrderedNode *Addr) {
		auto ListDataBegin = DualListData.ListBegin();
		Value->AddUse();
		Addr->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Value) == RegClass::Invalid || WalkFindRegClass(Value) == RegClass::GPR, "GetOpRegClass(Value) == RegClass::Invalid || WalkFindRegClass(Value) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Addr) == RegClass::Invalid || WalkFindRegClass(Addr) == RegClass::GPR, "GetOpRegClass(Addr) == RegClass::Invalid || WalkFindRegClass(Addr) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(Size == FEXCore::IR::OpSize::i8Bit || Size == FEXCore::IR::OpSize::i16Bit || Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit, "Size == FEXCore::IR::OpSize::i8Bit || Size == FEXCore::IR::OpSize::i16Bit || Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit");
#endif
		return _AtomicFetchSub(Size, Value->Wrapped(ListDataBegin), Addr->Wrapped(ListDataBegin));
	}

	/// Atomic integer fetch and binary and
	///
	/// Atomically fetches %Addr and binary ands %value to the memory location
	///
	/// Dest is the value prior to operating on the value in memory
	///
	/// IR layout must match NonFetch-variant, otherwise DCE IR optimization breaks!
	IRPair<IROp_AtomicFetchAnd> _AtomicFetchAnd(FEXCore::IR::OpSize Size, OrderedNodeWrapper Value, OrderedNodeWrapper Addr) {
		auto _Op = AllocateOp<IROp_AtomicFetchAnd, IROps::OP_ATOMICFETCHAND>();
		_Op.first->Value = Value;
		_Op.first->Addr = Addr;
		_Op.first->Header.Size = Size;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Atomic integer fetch and binary and
	///
	/// Atomically fetches %Addr and binary ands %value to the memory location
	///
	/// Dest is the value prior to operating on the value in memory
	///
	/// IR layout must match NonFetch-variant, otherwise DCE IR optimization breaks!
	IRPair<IROp_AtomicFetchAnd> _AtomicFetchAnd(FEXCore::IR::OpSize Size, OrderedNode *Value, OrderedNode *Addr) {
		auto ListDataBegin = DualListData.ListBegin();
		Value->AddUse();
		Addr->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Value) == RegClass::Invalid || WalkFindRegClass(Value) == RegClass::GPR, "GetOpRegClass(Value) == RegClass::Invalid || WalkFindRegClass(Value) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Addr) == RegClass::Invalid || WalkFindRegClass(Addr) == RegClass::GPR, "GetOpRegClass(Addr) == RegClass::Invalid || WalkFindRegClass(Addr) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(Size == FEXCore::IR::OpSize::i8Bit || Size == FEXCore::IR::OpSize::i16Bit || Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit, "Size == FEXCore::IR::OpSize::i8Bit || Size == FEXCore::IR::OpSize::i16Bit || Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit");
#endif
		return _AtomicFetchAnd(Size, Value->Wrapped(ListDataBegin), Addr->Wrapped(ListDataBegin));
	}

	/// Atomic integer fetch and binary clear
	///
	/// Atomically fetches %Addr and binary clears %value to the memory location
	///
	/// Dest is the value prior to operating on the value in memory
	///
	/// Matches ARM ldclral semantics
	///
	/// eg: Dest[Addr] &= ~Value
	///
	/// IR layout must match NonFetch-variant, otherwise DCE IR optimization breaks!
	IRPair<IROp_AtomicFetchCLR> _AtomicFetchCLR(FEXCore::IR::OpSize Size, OrderedNodeWrapper Value, OrderedNodeWrapper Addr) {
		auto _Op = AllocateOp<IROp_AtomicFetchCLR, IROps::OP_ATOMICFETCHCLR>();
		_Op.first->Value = Value;
		_Op.first->Addr = Addr;
		_Op.first->Header.Size = Size;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Atomic integer fetch and binary clear
	///
	/// Atomically fetches %Addr and binary clears %value to the memory location
	///
	/// Dest is the value prior to operating on the value in memory
	///
	/// Matches ARM ldclral semantics
	///
	/// eg: Dest[Addr] &= ~Value
	///
	/// IR layout must match NonFetch-variant, otherwise DCE IR optimization breaks!
	IRPair<IROp_AtomicFetchCLR> _AtomicFetchCLR(FEXCore::IR::OpSize Size, OrderedNode *Value, OrderedNode *Addr) {
		auto ListDataBegin = DualListData.ListBegin();
		Value->AddUse();
		Addr->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Value) == RegClass::Invalid || WalkFindRegClass(Value) == RegClass::GPR, "GetOpRegClass(Value) == RegClass::Invalid || WalkFindRegClass(Value) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Addr) == RegClass::Invalid || WalkFindRegClass(Addr) == RegClass::GPR, "GetOpRegClass(Addr) == RegClass::Invalid || WalkFindRegClass(Addr) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(Size == FEXCore::IR::OpSize::i8Bit || Size == FEXCore::IR::OpSize::i16Bit || Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit, "Size == FEXCore::IR::OpSize::i8Bit || Size == FEXCore::IR::OpSize::i16Bit || Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit");
#endif
		return _AtomicFetchCLR(Size, Value->Wrapped(ListDataBegin), Addr->Wrapped(ListDataBegin));
	}

	/// Atomic integer fetch and binary or
	///
	/// Atomically fetches %Addr and binary ors %value to the memory location
	///
	/// Dest is the value prior to operating on the value in memory
	///
	/// IR layout must match NonFetch-variant, otherwise DCE IR optimization breaks!
	IRPair<IROp_AtomicFetchOr> _AtomicFetchOr(FEXCore::IR::OpSize Size, OrderedNodeWrapper Value, OrderedNodeWrapper Addr) {
		auto _Op = AllocateOp<IROp_AtomicFetchOr, IROps::OP_ATOMICFETCHOR>();
		_Op.first->Value = Value;
		_Op.first->Addr = Addr;
		_Op.first->Header.Size = Size;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Atomic integer fetch and binary or
	///
	/// Atomically fetches %Addr and binary ors %value to the memory location
	///
	/// Dest is the value prior to operating on the value in memory
	///
	/// IR layout must match NonFetch-variant, otherwise DCE IR optimization breaks!
	IRPair<IROp_AtomicFetchOr> _AtomicFetchOr(FEXCore::IR::OpSize Size, OrderedNode *Value, OrderedNode *Addr) {
		auto ListDataBegin = DualListData.ListBegin();
		Value->AddUse();
		Addr->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Value) == RegClass::Invalid || WalkFindRegClass(Value) == RegClass::GPR, "GetOpRegClass(Value) == RegClass::Invalid || WalkFindRegClass(Value) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Addr) == RegClass::Invalid || WalkFindRegClass(Addr) == RegClass::GPR, "GetOpRegClass(Addr) == RegClass::Invalid || WalkFindRegClass(Addr) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(Size == FEXCore::IR::OpSize::i8Bit || Size == FEXCore::IR::OpSize::i16Bit || Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit, "Size == FEXCore::IR::OpSize::i8Bit || Size == FEXCore::IR::OpSize::i16Bit || Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit");
#endif
		return _AtomicFetchOr(Size, Value->Wrapped(ListDataBegin), Addr->Wrapped(ListDataBegin));
	}

	/// Atomic integer fetch and binary exclusive or
	///
	/// Atomically fetches %Addr and binary exclusive ors %value to the memory location
	///
	/// Dest is the value prior to operating on the value in memory
	///
	/// IR layout must match NonFetch-variant, otherwise DCE IR optimization breaks!
	IRPair<IROp_AtomicFetchXor> _AtomicFetchXor(FEXCore::IR::OpSize Size, OrderedNodeWrapper Value, OrderedNodeWrapper Addr) {
		auto _Op = AllocateOp<IROp_AtomicFetchXor, IROps::OP_ATOMICFETCHXOR>();
		_Op.first->Value = Value;
		_Op.first->Addr = Addr;
		_Op.first->Header.Size = Size;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Atomic integer fetch and binary exclusive or
	///
	/// Atomically fetches %Addr and binary exclusive ors %value to the memory location
	///
	/// Dest is the value prior to operating on the value in memory
	///
	/// IR layout must match NonFetch-variant, otherwise DCE IR optimization breaks!
	IRPair<IROp_AtomicFetchXor> _AtomicFetchXor(FEXCore::IR::OpSize Size, OrderedNode *Value, OrderedNode *Addr) {
		auto ListDataBegin = DualListData.ListBegin();
		Value->AddUse();
		Addr->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Value) == RegClass::Invalid || WalkFindRegClass(Value) == RegClass::GPR, "GetOpRegClass(Value) == RegClass::Invalid || WalkFindRegClass(Value) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Addr) == RegClass::Invalid || WalkFindRegClass(Addr) == RegClass::GPR, "GetOpRegClass(Addr) == RegClass::Invalid || WalkFindRegClass(Addr) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(Size == FEXCore::IR::OpSize::i8Bit || Size == FEXCore::IR::OpSize::i16Bit || Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit, "Size == FEXCore::IR::OpSize::i8Bit || Size == FEXCore::IR::OpSize::i16Bit || Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit");
#endif
		return _AtomicFetchXor(Size, Value->Wrapped(ListDataBegin), Addr->Wrapped(ListDataBegin));
	}

	/// Atomic integer fetch and two's complement negate
	///
	/// Dest is the value prior to operating on the value in memory
	///
	/// IR layout must match NonFetch-variant, otherwise DCE IR optimization breaks!
	IRPair<IROp_AtomicFetchNeg> _AtomicFetchNeg(FEXCore::IR::OpSize Size, OrderedNodeWrapper Addr) {
		auto _Op = AllocateOp<IROp_AtomicFetchNeg, IROps::OP_ATOMICFETCHNEG>();
		_Op.first->Addr = Addr;
		_Op.first->Header.Size = Size;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Atomic integer fetch and two's complement negate
	///
	/// Dest is the value prior to operating on the value in memory
	///
	/// IR layout must match NonFetch-variant, otherwise DCE IR optimization breaks!
	IRPair<IROp_AtomicFetchNeg> _AtomicFetchNeg(FEXCore::IR::OpSize Size, OrderedNode *Addr) {
		auto ListDataBegin = DualListData.ListBegin();
		Addr->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Addr) == RegClass::Invalid || WalkFindRegClass(Addr) == RegClass::GPR, "GetOpRegClass(Addr) == RegClass::Invalid || WalkFindRegClass(Addr) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(Size == FEXCore::IR::OpSize::i8Bit || Size == FEXCore::IR::OpSize::i16Bit || Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit, "Size == FEXCore::IR::OpSize::i8Bit || Size == FEXCore::IR::OpSize::i16Bit || Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit");
#endif
		return _AtomicFetchNeg(Size, Addr->Wrapped(ListDataBegin));
	}

	/// Set Telemetry value if the passed in 32-bit value isn't zero.
	///
	/// Only useful for 32-bit applications.
	IRPair<IROp_TelemetrySetValue> _TelemetrySetValue(OrderedNodeWrapper Value, uint8_t TelemetryValueIndex) {
		SaveNZCV(IROps::OP_TELEMETRYSETVALUE);		auto _Op = AllocateOp<IROp_TelemetrySetValue, IROps::OP_TELEMETRYSETVALUE>();
		_Op.first->Value = Value;
		_Op.first->TelemetryValueIndex = TelemetryValueIndex;
		_Op.first->Header.Size = OpSize::i64Bit;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Set Telemetry value if the passed in 32-bit value isn't zero.
	///
	/// Only useful for 32-bit applications.
	IRPair<IROp_TelemetrySetValue> _TelemetrySetValue(OrderedNode *Value, uint8_t TelemetryValueIndex) {
		auto ListDataBegin = DualListData.ListBegin();
		Value->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Value) == RegClass::Invalid || WalkFindRegClass(Value) == RegClass::GPR, "GetOpRegClass(Value) == RegClass::Invalid || WalkFindRegClass(Value) == RegClass::GPR");
#endif
		return _TelemetrySetValue(Value->Wrapped(ListDataBegin), TelemetryValueIndex);
	}

	/// Returns the <entrypoint> + Offset address
	///
	/// When the size is 4 bytes then 32-bit overflow and underflow needs to work
	IRPair<IROp_EntrypointOffset> _EntrypointOffset(FEXCore::IR::OpSize Size, int64_t Offset) {
		auto _Op = AllocateOp<IROp_EntrypointOffset, IROps::OP_ENTRYPOINTOFFSET>();
		_Op.first->Offset = Offset;
		_Op.first->Header.Size = Size;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit, "Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit");
#endif
		return _Op;
	}

	/// Returns the <entrypoint> + Offset address
	///
	/// When the size is 4 bytes then 32-bit overflow and underflow needs to work
	IRPair<IROp_InlineEntrypointOffset> _InlineEntrypointOffset(FEXCore::IR::OpSize Size, int64_t Offset) {
		auto _Op = AllocateOp<IROp_InlineEntrypointOffset, IROps::OP_INLINEENTRYPOINTOFFSET>();
		_Op.first->Offset = Offset;
		_Op.first->Header.Size = Size;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit, "Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit");
#endif
		return _Op;
	}

	/// Generates a 64bit constant inside of a GPR
	///
	/// Unsupported to create a constant in FPR
	IRPair<IROp_Constant> _Constant(int64_t Constant, ConstPad Pad = IR::ConstPad::NoPad, int32_t MaxBytes = 0) {
		auto _Op = AllocateOp<IROp_Constant, IROps::OP_CONSTANT>();
		_Op.first->Constant = Constant;
		_Op.first->Pad = Pad;
		_Op.first->MaxBytes = MaxBytes;
		_Op.first->Header.Size = OpSize::i64Bit;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(MaxBytes >= 0 && MaxBytes <= 8 && (MaxBytes & 1) == 0, "MaxBytes >= 0 && MaxBytes <= 8 && (MaxBytes & 1) == 0");
		LOGMAN_THROW_A_FMT(MaxBytes == 0 || (Constant >> (MaxBytes * 8)) == 0, "MaxBytes == 0 || (Constant >> (MaxBytes * 8)) == 0");
#endif
		return _Op;
	}

	/// Generates a 64bit constant to be used directly, non-FPR
	IRPair<IROp_InlineConstant> _InlineConstant(int64_t Constant) {
		auto _Op = AllocateOp<IROp_InlineConstant, IROps::OP_INLINECONSTANT>();
		_Op.first->Constant = Constant;
		_Op.first->Header.Size = OpSize::i64Bit;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Returns the host 64bit cycle counter
	///
	/// Useful when emulating rdtsc
	///
	/// Be careful, the frequency of this counter changes based on host
	///
	/// On AArch64 make sure to query the CNTFRQ_EL0 system register to get the frequency
	///
	/// On x86-64 make sure to query CPUID fn8000_0008[EDX_8] for constant TSC
	///
	/// x86-64 constant frequency lives in MSR_PLATFORM_INFO. Which is only available to kernel
	///
	/// Part of the ART frequency equation can be pulled from CPUID fn0000_0015[EBX & EAX]
	///
	/// But it's missing the ART multiplier still?
	///
	/// If the self-synchronizing flag is toggled then all instructions and loads must be completed before the cycle counter read
	IRPair<IROp_CycleCounter> _CycleCounter(bool SelfSynchronizingLoads) {
		auto _Op = AllocateOp<IROp_CycleCounter, IROps::OP_CYCLECOUNTER>();
		_Op.first->SelfSynchronizingLoads = SelfSynchronizingLoads;
		_Op.first->Header.Size = OpSize::i64Bit;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Integer negation, with optional predication
	///
	/// Dest = Cond ? -Src : Src
	///
	/// Will truncate to 64 or 32bits
	IRPair<IROp_Neg> _Neg(FEXCore::IR::OpSize Size, OrderedNodeWrapper Src, CondClass Cond = CondClass::AL) {
		auto _Op = AllocateOp<IROp_Neg, IROps::OP_NEG>();
		_Op.first->Src = Src;
		_Op.first->Cond = Cond;
		_Op.first->Header.Size = Size;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Integer negation, with optional predication
	///
	/// Dest = Cond ? -Src : Src
	///
	/// Will truncate to 64 or 32bits
	IRPair<IROp_Neg> _Neg(FEXCore::IR::OpSize Size, OrderedNode *Src, CondClass Cond = CondClass::AL) {
		auto ListDataBegin = DualListData.ListBegin();
		Src->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src) == RegClass::Invalid || WalkFindRegClass(Src) == RegClass::GPR, "GetOpRegClass(Src) == RegClass::Invalid || WalkFindRegClass(Src) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit, "Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit");
#endif
		return _Neg(Size, Src->Wrapped(ListDataBegin), Cond);
	}

	/// Integer binary not
	///
	/// op:
	///
	/// Dest = ~Src
	IRPair<IROp_Not> _Not(FEXCore::IR::OpSize Size, OrderedNodeWrapper Src) {
		auto _Op = AllocateOp<IROp_Not, IROps::OP_NOT>();
		_Op.first->Src = Src;
		_Op.first->Header.Size = Size;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Integer binary not
	///
	/// op:
	///
	/// Dest = ~Src
	IRPair<IROp_Not> _Not(FEXCore::IR::OpSize Size, OrderedNode *Src) {
		auto ListDataBegin = DualListData.ListBegin();
		Src->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src) == RegClass::Invalid || WalkFindRegClass(Src) == RegClass::GPR, "GetOpRegClass(Src) == RegClass::Invalid || WalkFindRegClass(Src) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit, "Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit");
#endif
		return _Not(Size, Src->Wrapped(ListDataBegin));
	}

	/// Population count of source register
	///
	/// Returns the number of bits set
	IRPair<IROp_Popcount> _Popcount(FEXCore::IR::OpSize Size, OrderedNodeWrapper Src) {
		auto _Op = AllocateOp<IROp_Popcount, IROps::OP_POPCOUNT>();
		_Op.first->Src = Src;
		_Op.first->Header.Size = Size;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Population count of source register
	///
	/// Returns the number of bits set
	IRPair<IROp_Popcount> _Popcount(FEXCore::IR::OpSize Size, OrderedNode *Src) {
		auto ListDataBegin = DualListData.ListBegin();
		Src->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src) == RegClass::Invalid || WalkFindRegClass(Src) == RegClass::GPR, "GetOpRegClass(Src) == RegClass::Invalid || WalkFindRegClass(Src) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(Size == FEXCore::IR::OpSize::i8Bit || Size == FEXCore::IR::OpSize::i16Bit || Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit, "Size == FEXCore::IR::OpSize::i8Bit || Size == FEXCore::IR::OpSize::i16Bit || Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit");
#endif
		return _Popcount(Size, Src->Wrapped(ListDataBegin));
	}

	/// Find least-significant-bit set
	///
	/// Returns the index of the least significant bit set
	///
	/// Undefined result if Src is zero.
	IRPair<IROp_FindLSB> _FindLSB(FEXCore::IR::OpSize Size, OrderedNodeWrapper Src) {
		auto _Op = AllocateOp<IROp_FindLSB, IROps::OP_FINDLSB>();
		_Op.first->Src = Src;
		_Op.first->Header.Size = Size;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Find least-significant-bit set
	///
	/// Returns the index of the least significant bit set
	///
	/// Undefined result if Src is zero.
	IRPair<IROp_FindLSB> _FindLSB(FEXCore::IR::OpSize Size, OrderedNode *Src) {
		auto ListDataBegin = DualListData.ListBegin();
		Src->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src) == RegClass::Invalid || WalkFindRegClass(Src) == RegClass::GPR, "GetOpRegClass(Src) == RegClass::Invalid || WalkFindRegClass(Src) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(Size == FEXCore::IR::OpSize::i8Bit || Size == FEXCore::IR::OpSize::i16Bit || Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit, "Size == FEXCore::IR::OpSize::i8Bit || Size == FEXCore::IR::OpSize::i16Bit || Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit");
#endif
		return _FindLSB(Size, Src->Wrapped(ListDataBegin));
	}

	/// Find most-significant-bit set
	///
	/// Returns the index of the most significant bit set
	///
	/// Undefined result if Src is zero.
	IRPair<IROp_FindMSB> _FindMSB(FEXCore::IR::OpSize Size, OrderedNodeWrapper Src) {
		auto _Op = AllocateOp<IROp_FindMSB, IROps::OP_FINDMSB>();
		_Op.first->Src = Src;
		_Op.first->Header.Size = Size;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Find most-significant-bit set
	///
	/// Returns the index of the most significant bit set
	///
	/// Undefined result if Src is zero.
	IRPair<IROp_FindMSB> _FindMSB(FEXCore::IR::OpSize Size, OrderedNode *Src) {
		auto ListDataBegin = DualListData.ListBegin();
		Src->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src) == RegClass::Invalid || WalkFindRegClass(Src) == RegClass::GPR, "GetOpRegClass(Src) == RegClass::Invalid || WalkFindRegClass(Src) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(Size == FEXCore::IR::OpSize::i16Bit || Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit, "Size == FEXCore::IR::OpSize::i16Bit || Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit");
#endif
		return _FindMSB(Size, Src->Wrapped(ListDataBegin));
	}

	/// Counts the number of trailing zero bits in a GPR
	///
	/// Returns the number of bits that are zero trailing
	///
	/// In the case of zero returns the size in bits of the input
	IRPair<IROp_FindTrailingZeroes> _FindTrailingZeroes(FEXCore::IR::OpSize Size, OrderedNodeWrapper Src) {
		auto _Op = AllocateOp<IROp_FindTrailingZeroes, IROps::OP_FINDTRAILINGZEROES>();
		_Op.first->Src = Src;
		_Op.first->Header.Size = Size;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Counts the number of trailing zero bits in a GPR
	///
	/// Returns the number of bits that are zero trailing
	///
	/// In the case of zero returns the size in bits of the input
	IRPair<IROp_FindTrailingZeroes> _FindTrailingZeroes(FEXCore::IR::OpSize Size, OrderedNode *Src) {
		auto ListDataBegin = DualListData.ListBegin();
		Src->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src) == RegClass::Invalid || WalkFindRegClass(Src) == RegClass::GPR, "GetOpRegClass(Src) == RegClass::Invalid || WalkFindRegClass(Src) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(Size == FEXCore::IR::OpSize::i16Bit || Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit, "Size == FEXCore::IR::OpSize::i16Bit || Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit");
#endif
		return _FindTrailingZeroes(Size, Src->Wrapped(ListDataBegin));
	}

	/// Counts the number of leading zero bits in a GPR
	///
	/// Returns the number of bits that are zero leading
	///
	/// In the case of zero returns the size in bits of the input
	IRPair<IROp_CountLeadingZeroes> _CountLeadingZeroes(FEXCore::IR::OpSize Size, OrderedNodeWrapper Src) {
		auto _Op = AllocateOp<IROp_CountLeadingZeroes, IROps::OP_COUNTLEADINGZEROES>();
		_Op.first->Src = Src;
		_Op.first->Header.Size = Size;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Counts the number of leading zero bits in a GPR
	///
	/// Returns the number of bits that are zero leading
	///
	/// In the case of zero returns the size in bits of the input
	IRPair<IROp_CountLeadingZeroes> _CountLeadingZeroes(FEXCore::IR::OpSize Size, OrderedNode *Src) {
		auto ListDataBegin = DualListData.ListBegin();
		Src->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src) == RegClass::Invalid || WalkFindRegClass(Src) == RegClass::GPR, "GetOpRegClass(Src) == RegClass::Invalid || WalkFindRegClass(Src) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(Size == FEXCore::IR::OpSize::i16Bit || Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit, "Size == FEXCore::IR::OpSize::i16Bit || Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit");
#endif
		return _CountLeadingZeroes(Size, Src->Wrapped(ListDataBegin));
	}

	/// Reverses the byte order of the register
	///
	/// Specifically 8bit byte swap size. (Not 16bit or 32bit word swapping)
	IRPair<IROp_Rev> _Rev(FEXCore::IR::OpSize Size, OrderedNodeWrapper Src) {
		auto _Op = AllocateOp<IROp_Rev, IROps::OP_REV>();
		_Op.first->Src = Src;
		_Op.first->Header.Size = Size;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Reverses the byte order of the register
	///
	/// Specifically 8bit byte swap size. (Not 16bit or 32bit word swapping)
	IRPair<IROp_Rev> _Rev(FEXCore::IR::OpSize Size, OrderedNode *Src) {
		auto ListDataBegin = DualListData.ListBegin();
		Src->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src) == RegClass::Invalid || WalkFindRegClass(Src) == RegClass::GPR, "GetOpRegClass(Src) == RegClass::Invalid || WalkFindRegClass(Src) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(Size == FEXCore::IR::OpSize::i16Bit || Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit, "Size == FEXCore::IR::OpSize::i16Bit || Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit");
#endif
		return _Rev(Size, Src->Wrapped(ListDataBegin));
	}

	/// Reverses the bit order of the register
	IRPair<IROp_Rbit> _Rbit(FEXCore::IR::OpSize Size, OrderedNodeWrapper Src) {
		auto _Op = AllocateOp<IROp_Rbit, IROps::OP_RBIT>();
		_Op.first->Src = Src;
		_Op.first->Header.Size = Size;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Reverses the bit order of the register
	IRPair<IROp_Rbit> _Rbit(FEXCore::IR::OpSize Size, OrderedNode *Src) {
		auto ListDataBegin = DualListData.ListBegin();
		Src->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src) == RegClass::Invalid || WalkFindRegClass(Src) == RegClass::GPR, "GetOpRegClass(Src) == RegClass::Invalid || WalkFindRegClass(Src) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit, "Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit");
#endif
		return _Rbit(Size, Src->Wrapped(ListDataBegin));
	}

	/// Integer Add
	///
	/// Will truncate to 64 or 32bits
	IRPair<IROp_Add> _Add(FEXCore::IR::OpSize Size, OrderedNodeWrapper Src1, OrderedNodeWrapper Src2) {
		auto _Op = AllocateOp<IROp_Add, IROps::OP_ADD>();
		_Op.first->Src1 = Src1;
		_Op.first->Src2 = Src2;
		_Op.first->Header.Size = Size;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Integer Add
	///
	/// Will truncate to 64 or 32bits
	IRPair<IROp_Add> _Add(FEXCore::IR::OpSize Size, OrderedNode *Src1, OrderedNode *Src2) {
		auto ListDataBegin = DualListData.ListBegin();
		Src1->AddUse();
		Src2 = InlineLargeAddSub(Size, Src2);
		Src2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src1) == RegClass::Invalid || WalkFindRegClass(Src1) == RegClass::GPR, "GetOpRegClass(Src1) == RegClass::Invalid || WalkFindRegClass(Src1) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src2) == RegClass::Invalid || WalkFindRegClass(Src2) == RegClass::GPR, "GetOpRegClass(Src2) == RegClass::Invalid || WalkFindRegClass(Src2) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit, "Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit");
#endif
		return _Add(Size, Src1->Wrapped(ListDataBegin), Src2->Wrapped(ListDataBegin));
	}

	/// Integer Add with carry
	///
	/// Will truncate to 64 or 32bits
	IRPair<IROp_Adc> _Adc(FEXCore::IR::OpSize Size, OrderedNodeWrapper Src1, OrderedNodeWrapper Src2) {
		auto _Op = AllocateOp<IROp_Adc, IROps::OP_ADC>();
		_Op.first->Src1 = Src1;
		_Op.first->Src2 = Src2;
		_Op.first->Header.Size = Size;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Integer Add with carry
	///
	/// Will truncate to 64 or 32bits
	IRPair<IROp_Adc> _Adc(FEXCore::IR::OpSize Size, OrderedNode *Src1, OrderedNode *Src2) {
		auto ListDataBegin = DualListData.ListBegin();
		Src1 = InlineZero(Size, Src1);
		Src1->AddUse();
		Src2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src1) == RegClass::Invalid || WalkFindRegClass(Src1) == RegClass::GPR, "GetOpRegClass(Src1) == RegClass::Invalid || WalkFindRegClass(Src1) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src2) == RegClass::Invalid || WalkFindRegClass(Src2) == RegClass::GPR, "GetOpRegClass(Src2) == RegClass::Invalid || WalkFindRegClass(Src2) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit, "Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit");
#endif
		return _Adc(Size, Src1->Wrapped(ListDataBegin), Src2->Wrapped(ListDataBegin));
	}

	/// Integer Subtract with carry/borrow
	///
	/// Will truncate to 64 or 32bits
	IRPair<IROp_Sbb> _Sbb(FEXCore::IR::OpSize Size, OrderedNodeWrapper Src1, OrderedNodeWrapper Src2) {
		auto _Op = AllocateOp<IROp_Sbb, IROps::OP_SBB>();
		_Op.first->Src1 = Src1;
		_Op.first->Src2 = Src2;
		_Op.first->Header.Size = Size;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Integer Subtract with carry/borrow
	///
	/// Will truncate to 64 or 32bits
	IRPair<IROp_Sbb> _Sbb(FEXCore::IR::OpSize Size, OrderedNode *Src1, OrderedNode *Src2) {
		auto ListDataBegin = DualListData.ListBegin();
		Src1->AddUse();
		Src2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src1) == RegClass::Invalid || WalkFindRegClass(Src1) == RegClass::GPR, "GetOpRegClass(Src1) == RegClass::Invalid || WalkFindRegClass(Src1) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src2) == RegClass::Invalid || WalkFindRegClass(Src2) == RegClass::GPR, "GetOpRegClass(Src2) == RegClass::Invalid || WalkFindRegClass(Src2) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit, "Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit");
#endif
		return _Sbb(Size, Src1->Wrapped(ListDataBegin), Src2->Wrapped(ListDataBegin));
	}

	/// Integer Add with shifted register
	///
	/// Will truncate to 64 or 32bits
	///
	/// Dest = Src1 + (Src2 << ShiftAmount)
	IRPair<IROp_AddShift> _AddShift(FEXCore::IR::OpSize Size, OrderedNodeWrapper Src1, OrderedNodeWrapper Src2, FEXCore::IR::ShiftType Shift = ShiftType::LSL, uint8_t ShiftAmount = 0) {
		auto _Op = AllocateOp<IROp_AddShift, IROps::OP_ADDSHIFT>();
		_Op.first->Src1 = Src1;
		_Op.first->Src2 = Src2;
		_Op.first->Shift = Shift;
		_Op.first->ShiftAmount = ShiftAmount;
		_Op.first->Header.Size = Size;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Integer Add with shifted register
	///
	/// Will truncate to 64 or 32bits
	///
	/// Dest = Src1 + (Src2 << ShiftAmount)
	IRPair<IROp_AddShift> _AddShift(FEXCore::IR::OpSize Size, OrderedNode *Src1, OrderedNode *Src2, FEXCore::IR::ShiftType Shift = ShiftType::LSL, uint8_t ShiftAmount = 0) {
		auto ListDataBegin = DualListData.ListBegin();
		Src1->AddUse();
		Src2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src1) == RegClass::Invalid || WalkFindRegClass(Src1) == RegClass::GPR, "GetOpRegClass(Src1) == RegClass::Invalid || WalkFindRegClass(Src1) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src2) == RegClass::Invalid || WalkFindRegClass(Src2) == RegClass::GPR, "GetOpRegClass(Src2) == RegClass::Invalid || WalkFindRegClass(Src2) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit, "Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit");
		LOGMAN_THROW_A_FMT(Shift != ShiftType::ROR, "Shift != ShiftType::ROR");
#endif
		return _AddShift(Size, Src1->Wrapped(ListDataBegin), Src2->Wrapped(ListDataBegin), Shift, ShiftAmount);
	}

	/// Integer add. Truncates and sets NZCV per AddNZCV
	IRPair<IROp_AddWithFlags> _AddWithFlags(FEXCore::IR::OpSize Size, OrderedNodeWrapper Src1, OrderedNodeWrapper Src2) {
		auto _Op = AllocateOp<IROp_AddWithFlags, IROps::OP_ADDWITHFLAGS>();
		_Op.first->Src1 = Src1;
		_Op.first->Src2 = Src2;
		_Op.first->Header.Size = Size;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Integer add. Truncates and sets NZCV per AddNZCV
	IRPair<IROp_AddWithFlags> _AddWithFlags(FEXCore::IR::OpSize Size, OrderedNode *Src1, OrderedNode *Src2) {
		auto ListDataBegin = DualListData.ListBegin();
		Src1->AddUse();
		Src2 = InlineLargeAddSub(Size, Src2);
		Src2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src1) == RegClass::Invalid || WalkFindRegClass(Src1) == RegClass::GPR, "GetOpRegClass(Src1) == RegClass::Invalid || WalkFindRegClass(Src1) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src2) == RegClass::Invalid || WalkFindRegClass(Src2) == RegClass::GPR, "GetOpRegClass(Src2) == RegClass::Invalid || WalkFindRegClass(Src2) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit, "Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit");
#endif
		return _AddWithFlags(Size, Src1->Wrapped(ListDataBegin), Src2->Wrapped(ListDataBegin));
	}

	/// Set NZCV for the sum of two GPRs
	IRPair<IROp_AddNZCV> _AddNZCV(FEXCore::IR::OpSize Size, OrderedNodeWrapper Src1, OrderedNodeWrapper Src2) {
		auto _Op = AllocateOp<IROp_AddNZCV, IROps::OP_ADDNZCV>();
		_Op.first->Src1 = Src1;
		_Op.first->Src2 = Src2;
		_Op.first->Header.Size = Size;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Set NZCV for the sum of two GPRs
	IRPair<IROp_AddNZCV> _AddNZCV(FEXCore::IR::OpSize Size, OrderedNode *Src1, OrderedNode *Src2) {
		auto ListDataBegin = DualListData.ListBegin();
		Src1->AddUse();
		Src2 = InlineLargeAddSub(Size, Src2);
		Src2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src1) == RegClass::Invalid || WalkFindRegClass(Src1) == RegClass::GPR, "GetOpRegClass(Src1) == RegClass::Invalid || WalkFindRegClass(Src1) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src2) == RegClass::Invalid || WalkFindRegClass(Src2) == RegClass::GPR, "GetOpRegClass(Src2) == RegClass::Invalid || WalkFindRegClass(Src2) == RegClass::GPR");
#endif
		return _AddNZCV(Size, Src1->Wrapped(ListDataBegin), Src2->Wrapped(ListDataBegin));
	}

	/// Set NZV with a SETF instruction. Preserves CF.
	IRPair<IROp_SetSmallNZV> _SetSmallNZV(FEXCore::IR::OpSize Size, OrderedNodeWrapper Src) {
		auto _Op = AllocateOp<IROp_SetSmallNZV, IROps::OP_SETSMALLNZV>();
		_Op.first->Src = Src;
		_Op.first->Header.Size = Size;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Set NZV with a SETF instruction. Preserves CF.
	IRPair<IROp_SetSmallNZV> _SetSmallNZV(FEXCore::IR::OpSize Size, OrderedNode *Src) {
		auto ListDataBegin = DualListData.ListBegin();
		Src->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src) == RegClass::Invalid || WalkFindRegClass(Src) == RegClass::GPR, "GetOpRegClass(Src) == RegClass::Invalid || WalkFindRegClass(Src) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(Size == FEXCore::IR::OpSize::i8Bit || Size == FEXCore::IR::OpSize::i16Bit, "Size == FEXCore::IR::OpSize::i8Bit || Size == FEXCore::IR::OpSize::i16Bit");
#endif
		return _SetSmallNZV(Size, Src->Wrapped(ListDataBegin));
	}

	/// Invert carry flag in NZCV
	IRPair<IROp_CarryInvert> _CarryInvert() {
		auto _Op = AllocateOp<IROp_CarryInvert, IROps::OP_CARRYINVERT>();
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// After an FCmp, converts NZCV flags from the Arm format to a mysterious eXternal format
	///
	/// On FlagM2-less platforms, takes the inverted 1/0 overflow flag
	IRPair<IROp_AXFlag> _AXFlag(OrderedNodeWrapper V_inv) {
		auto _Op = AllocateOp<IROp_AXFlag, IROps::OP_AXFLAG>();
		_Op.first->V_inv = V_inv;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// After an FCmp, converts NZCV flags from the Arm format to a mysterious eXternal format
	///
	/// On FlagM2-less platforms, takes the inverted 1/0 overflow flag
	IRPair<IROp_AXFlag> _AXFlag(OrderedNode *V_inv) {
		auto ListDataBegin = DualListData.ListBegin();
		V_inv->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(V_inv) == RegClass::Invalid || WalkFindRegClass(V_inv) == RegClass::GPR, "GetOpRegClass(V_inv) == RegClass::Invalid || WalkFindRegClass(V_inv) == RegClass::GPR");
#endif
		return _AXFlag(V_inv->Wrapped(ListDataBegin));
	}

	/// Calculates PF
	IRPair<IROp_Parity> _Parity(OrderedNodeWrapper Raw, bool Mask, bool Invert) {
		auto _Op = AllocateOp<IROp_Parity, IROps::OP_PARITY>();
		_Op.first->Raw = Raw;
		_Op.first->Mask = Mask;
		_Op.first->Invert = Invert;
		_Op.first->Header.Size = OpSize::i32Bit;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Calculates PF
	IRPair<IROp_Parity> _Parity(OrderedNode *Raw, bool Mask, bool Invert) {
		auto ListDataBegin = DualListData.ListBegin();
		Raw->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Raw) == RegClass::Invalid || WalkFindRegClass(Raw) == RegClass::GPR, "GetOpRegClass(Raw) == RegClass::Invalid || WalkFindRegClass(Raw) == RegClass::GPR");
#endif
		return _Parity(Raw->Wrapped(ListDataBegin), Mask, Invert);
	}

	/// Rotate, mask, and insert into NZCV on FlagM platforms
	IRPair<IROp_RmifNZCV> _RmifNZCV(OrderedNodeWrapper Src, uint8_t Rotate, uint8_t Mask) {
		auto _Op = AllocateOp<IROp_RmifNZCV, IROps::OP_RMIFNZCV>();
		_Op.first->Src = Src;
		_Op.first->Rotate = Rotate;
		_Op.first->Mask = Mask;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Rotate, mask, and insert into NZCV on FlagM platforms
	IRPair<IROp_RmifNZCV> _RmifNZCV(OrderedNode *Src, uint8_t Rotate, uint8_t Mask) {
		auto ListDataBegin = DualListData.ListBegin();
		Src = InlineZero(OpSize::i64Bit, Src);
		Src->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src) == RegClass::Invalid || WalkFindRegClass(Src) == RegClass::GPR, "GetOpRegClass(Src) == RegClass::Invalid || WalkFindRegClass(Src) == RegClass::GPR");
#endif
		return _RmifNZCV(Src->Wrapped(ListDataBegin), Rotate, Mask);
	}

	/// If condition is true, set NZCV per sum of GPRs, else force NZCV to a constant.
	IRPair<IROp_CondAddNZCV> _CondAddNZCV(FEXCore::IR::OpSize Size, OrderedNodeWrapper Src1, OrderedNodeWrapper Src2, CondClass Cond, uint8_t FalseNZCV) {
		auto _Op = AllocateOp<IROp_CondAddNZCV, IROps::OP_CONDADDNZCV>();
		_Op.first->Src1 = Src1;
		_Op.first->Src2 = Src2;
		_Op.first->Cond = Cond;
		_Op.first->FalseNZCV = FalseNZCV;
		_Op.first->Header.Size = Size;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// If condition is true, set NZCV per sum of GPRs, else force NZCV to a constant.
	IRPair<IROp_CondAddNZCV> _CondAddNZCV(FEXCore::IR::OpSize Size, OrderedNode *Src1, OrderedNode *Src2, CondClass Cond, uint8_t FalseNZCV) {
		auto ListDataBegin = DualListData.ListBegin();
		Src1 = InlineZero(Size, Src1);
		Src1->AddUse();
		Src2 = InlineAddSub(Size, Src2);
		Src2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src1) == RegClass::Invalid || WalkFindRegClass(Src1) == RegClass::GPR, "GetOpRegClass(Src1) == RegClass::Invalid || WalkFindRegClass(Src1) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src2) == RegClass::Invalid || WalkFindRegClass(Src2) == RegClass::GPR, "GetOpRegClass(Src2) == RegClass::Invalid || WalkFindRegClass(Src2) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit, "Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit");
#endif
		return _CondAddNZCV(Size, Src1->Wrapped(ListDataBegin), Src2->Wrapped(ListDataBegin), Cond, FalseNZCV);
	}

	/// If condition is true, set NZCV per difference of GPRs, else force NZCV to a constant.
	IRPair<IROp_CondSubNZCV> _CondSubNZCV(FEXCore::IR::OpSize Size, OrderedNodeWrapper Src1, OrderedNodeWrapper Src2, CondClass Cond, uint8_t FalseNZCV) {
		auto _Op = AllocateOp<IROp_CondSubNZCV, IROps::OP_CONDSUBNZCV>();
		_Op.first->Src1 = Src1;
		_Op.first->Src2 = Src2;
		_Op.first->Cond = Cond;
		_Op.first->FalseNZCV = FalseNZCV;
		_Op.first->Header.Size = Size;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// If condition is true, set NZCV per difference of GPRs, else force NZCV to a constant.
	IRPair<IROp_CondSubNZCV> _CondSubNZCV(FEXCore::IR::OpSize Size, OrderedNode *Src1, OrderedNode *Src2, CondClass Cond, uint8_t FalseNZCV) {
		auto ListDataBegin = DualListData.ListBegin();
		Src1 = InlineZero(Size, Src1);
		Src1->AddUse();
		Src2 = InlineAddSub(Size, Src2);
		Src2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src1) == RegClass::Invalid || WalkFindRegClass(Src1) == RegClass::GPR, "GetOpRegClass(Src1) == RegClass::Invalid || WalkFindRegClass(Src1) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src2) == RegClass::Invalid || WalkFindRegClass(Src2) == RegClass::GPR, "GetOpRegClass(Src2) == RegClass::Invalid || WalkFindRegClass(Src2) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit, "Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit");
#endif
		return _CondSubNZCV(Size, Src1->Wrapped(ListDataBegin), Src2->Wrapped(ListDataBegin), Cond, FalseNZCV);
	}

	/// Adds and set NZCV for the sum of two GPRs and carry-in given as NZCV
	IRPair<IROp_AdcWithFlags> _AdcWithFlags(FEXCore::IR::OpSize Size, OrderedNodeWrapper Src1, OrderedNodeWrapper Src2) {
		auto _Op = AllocateOp<IROp_AdcWithFlags, IROps::OP_ADCWITHFLAGS>();
		_Op.first->Src1 = Src1;
		_Op.first->Src2 = Src2;
		_Op.first->Header.Size = Size;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Adds and set NZCV for the sum of two GPRs and carry-in given as NZCV
	IRPair<IROp_AdcWithFlags> _AdcWithFlags(FEXCore::IR::OpSize Size, OrderedNode *Src1, OrderedNode *Src2) {
		auto ListDataBegin = DualListData.ListBegin();
		Src1 = InlineZero(Size, Src1);
		Src1->AddUse();
		Src2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src1) == RegClass::Invalid || WalkFindRegClass(Src1) == RegClass::GPR, "GetOpRegClass(Src1) == RegClass::Invalid || WalkFindRegClass(Src1) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src2) == RegClass::Invalid || WalkFindRegClass(Src2) == RegClass::GPR, "GetOpRegClass(Src2) == RegClass::Invalid || WalkFindRegClass(Src2) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit, "Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit");
#endif
		return _AdcWithFlags(Size, Src1->Wrapped(ListDataBegin), Src2->Wrapped(ListDataBegin));
	}

	/// Adds GPR with inverted carry-in
	IRPair<IROp_AdcZero> _AdcZero(FEXCore::IR::OpSize Size, OrderedNodeWrapper Src1) {
		auto _Op = AllocateOp<IROp_AdcZero, IROps::OP_ADCZERO>();
		_Op.first->Src1 = Src1;
		_Op.first->Header.Size = Size;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Adds GPR with inverted carry-in
	IRPair<IROp_AdcZero> _AdcZero(FEXCore::IR::OpSize Size, OrderedNode *Src1) {
		auto ListDataBegin = DualListData.ListBegin();
		Src1->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src1) == RegClass::Invalid || WalkFindRegClass(Src1) == RegClass::GPR, "GetOpRegClass(Src1) == RegClass::Invalid || WalkFindRegClass(Src1) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit, "Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit");
#endif
		return _AdcZero(Size, Src1->Wrapped(ListDataBegin));
	}

	/// Adds and set NZCV for the sum of GPR and inverted carry-in given as NZCV
	IRPair<IROp_AdcZeroWithFlags> _AdcZeroWithFlags(FEXCore::IR::OpSize Size, OrderedNodeWrapper Src1) {
		auto _Op = AllocateOp<IROp_AdcZeroWithFlags, IROps::OP_ADCZEROWITHFLAGS>();
		_Op.first->Src1 = Src1;
		_Op.first->Header.Size = Size;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Adds and set NZCV for the sum of GPR and inverted carry-in given as NZCV
	IRPair<IROp_AdcZeroWithFlags> _AdcZeroWithFlags(FEXCore::IR::OpSize Size, OrderedNode *Src1) {
		auto ListDataBegin = DualListData.ListBegin();
		Src1->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src1) == RegClass::Invalid || WalkFindRegClass(Src1) == RegClass::GPR, "GetOpRegClass(Src1) == RegClass::Invalid || WalkFindRegClass(Src1) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit, "Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit");
#endif
		return _AdcZeroWithFlags(Size, Src1->Wrapped(ListDataBegin));
	}

	/// Subtracts and set NZCV for the difference of two GPRs and carry-in given as NZCV
	IRPair<IROp_SbbWithFlags> _SbbWithFlags(FEXCore::IR::OpSize Size, OrderedNodeWrapper Src1, OrderedNodeWrapper Src2) {
		auto _Op = AllocateOp<IROp_SbbWithFlags, IROps::OP_SBBWITHFLAGS>();
		_Op.first->Src1 = Src1;
		_Op.first->Src2 = Src2;
		_Op.first->Header.Size = Size;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Subtracts and set NZCV for the difference of two GPRs and carry-in given as NZCV
	IRPair<IROp_SbbWithFlags> _SbbWithFlags(FEXCore::IR::OpSize Size, OrderedNode *Src1, OrderedNode *Src2) {
		auto ListDataBegin = DualListData.ListBegin();
		Src1->AddUse();
		Src2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src1) == RegClass::Invalid || WalkFindRegClass(Src1) == RegClass::GPR, "GetOpRegClass(Src1) == RegClass::Invalid || WalkFindRegClass(Src1) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src2) == RegClass::Invalid || WalkFindRegClass(Src2) == RegClass::GPR, "GetOpRegClass(Src2) == RegClass::Invalid || WalkFindRegClass(Src2) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit, "Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit");
#endif
		return _SbbWithFlags(Size, Src1->Wrapped(ListDataBegin), Src2->Wrapped(ListDataBegin));
	}

	/// Set NZCV for the sum of two GPRs and carry-in given as NZCV
	IRPair<IROp_AdcNZCV> _AdcNZCV(FEXCore::IR::OpSize Size, OrderedNodeWrapper Src1, OrderedNodeWrapper Src2) {
		auto _Op = AllocateOp<IROp_AdcNZCV, IROps::OP_ADCNZCV>();
		_Op.first->Src1 = Src1;
		_Op.first->Src2 = Src2;
		_Op.first->Header.Size = Size;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Set NZCV for the sum of two GPRs and carry-in given as NZCV
	IRPair<IROp_AdcNZCV> _AdcNZCV(FEXCore::IR::OpSize Size, OrderedNode *Src1, OrderedNode *Src2) {
		auto ListDataBegin = DualListData.ListBegin();
		Src1->AddUse();
		Src2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src1) == RegClass::Invalid || WalkFindRegClass(Src1) == RegClass::GPR, "GetOpRegClass(Src1) == RegClass::Invalid || WalkFindRegClass(Src1) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src2) == RegClass::Invalid || WalkFindRegClass(Src2) == RegClass::GPR, "GetOpRegClass(Src2) == RegClass::Invalid || WalkFindRegClass(Src2) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit, "Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit");
#endif
		return _AdcNZCV(Size, Src1->Wrapped(ListDataBegin), Src2->Wrapped(ListDataBegin));
	}

	/// Set NZCV for the difference of two GPRs and carry-in given as NZCV
	IRPair<IROp_SbbNZCV> _SbbNZCV(FEXCore::IR::OpSize Size, OrderedNodeWrapper Src1, OrderedNodeWrapper Src2) {
		auto _Op = AllocateOp<IROp_SbbNZCV, IROps::OP_SBBNZCV>();
		_Op.first->Src1 = Src1;
		_Op.first->Src2 = Src2;
		_Op.first->Header.Size = Size;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Set NZCV for the difference of two GPRs and carry-in given as NZCV
	IRPair<IROp_SbbNZCV> _SbbNZCV(FEXCore::IR::OpSize Size, OrderedNode *Src1, OrderedNode *Src2) {
		auto ListDataBegin = DualListData.ListBegin();
		Src1->AddUse();
		Src2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src1) == RegClass::Invalid || WalkFindRegClass(Src1) == RegClass::GPR, "GetOpRegClass(Src1) == RegClass::Invalid || WalkFindRegClass(Src1) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src2) == RegClass::Invalid || WalkFindRegClass(Src2) == RegClass::GPR, "GetOpRegClass(Src2) == RegClass::Invalid || WalkFindRegClass(Src2) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit, "Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit");
#endif
		return _SbbNZCV(Size, Src1->Wrapped(ListDataBegin), Src2->Wrapped(ListDataBegin));
	}

	/// Integer Sub
	///
	/// Will truncate to 64 or 32bits
	IRPair<IROp_Sub> _Sub(FEXCore::IR::OpSize Size, OrderedNodeWrapper Src1, OrderedNodeWrapper Src2) {
		auto _Op = AllocateOp<IROp_Sub, IROps::OP_SUB>();
		_Op.first->Src1 = Src1;
		_Op.first->Src2 = Src2;
		_Op.first->Header.Size = Size;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Integer Sub
	///
	/// Will truncate to 64 or 32bits
	IRPair<IROp_Sub> _Sub(FEXCore::IR::OpSize Size, OrderedNode *Src1, OrderedNode *Src2) {
		auto ListDataBegin = DualListData.ListBegin();
		Src1 = InlineSubtractZero(Size, Src1, Src2);
		Src1->AddUse();
		Src2 = InlineLargeAddSub(Size, Src2);
		Src2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src1) == RegClass::Invalid || WalkFindRegClass(Src1) == RegClass::GPR, "GetOpRegClass(Src1) == RegClass::Invalid || WalkFindRegClass(Src1) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src2) == RegClass::Invalid || WalkFindRegClass(Src2) == RegClass::GPR, "GetOpRegClass(Src2) == RegClass::Invalid || WalkFindRegClass(Src2) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit, "Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit");
#endif
		return _Sub(Size, Src1->Wrapped(ListDataBegin), Src2->Wrapped(ListDataBegin));
	}

	/// Integer Sub with shifted register
	///
	/// Will truncate to 64 or 32bits
	IRPair<IROp_SubShift> _SubShift(FEXCore::IR::OpSize Size, OrderedNodeWrapper Src1, OrderedNodeWrapper Src2, FEXCore::IR::ShiftType Shift = ShiftType::LSL, uint8_t ShiftAmount = 0) {
		auto _Op = AllocateOp<IROp_SubShift, IROps::OP_SUBSHIFT>();
		_Op.first->Src1 = Src1;
		_Op.first->Src2 = Src2;
		_Op.first->Shift = Shift;
		_Op.first->ShiftAmount = ShiftAmount;
		_Op.first->Header.Size = Size;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Integer Sub with shifted register
	///
	/// Will truncate to 64 or 32bits
	IRPair<IROp_SubShift> _SubShift(FEXCore::IR::OpSize Size, OrderedNode *Src1, OrderedNode *Src2, FEXCore::IR::ShiftType Shift = ShiftType::LSL, uint8_t ShiftAmount = 0) {
		auto ListDataBegin = DualListData.ListBegin();
		Src1->AddUse();
		Src2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src1) == RegClass::Invalid || WalkFindRegClass(Src1) == RegClass::GPR, "GetOpRegClass(Src1) == RegClass::Invalid || WalkFindRegClass(Src1) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src2) == RegClass::Invalid || WalkFindRegClass(Src2) == RegClass::GPR, "GetOpRegClass(Src2) == RegClass::Invalid || WalkFindRegClass(Src2) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit, "Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit");
		LOGMAN_THROW_A_FMT(Shift != ShiftType::ROR, "Shift != ShiftType::ROR");
#endif
		return _SubShift(Size, Src1->Wrapped(ListDataBegin), Src2->Wrapped(ListDataBegin), Shift, ShiftAmount);
	}

	/// Integer Sub. Truncates and sets NZCV per SubNZCV
	IRPair<IROp_SubWithFlags> _SubWithFlags(FEXCore::IR::OpSize Size, OrderedNodeWrapper Src1, OrderedNodeWrapper Src2) {
		auto _Op = AllocateOp<IROp_SubWithFlags, IROps::OP_SUBWITHFLAGS>();
		_Op.first->Src1 = Src1;
		_Op.first->Src2 = Src2;
		_Op.first->Header.Size = Size;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Integer Sub. Truncates and sets NZCV per SubNZCV
	IRPair<IROp_SubWithFlags> _SubWithFlags(FEXCore::IR::OpSize Size, OrderedNode *Src1, OrderedNode *Src2) {
		auto ListDataBegin = DualListData.ListBegin();
		Src1 = InlineSubtractZero(Size, Src1, Src2);
		Src1->AddUse();
		Src2 = InlineLargeAddSub(Size, Src2);
		Src2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src1) == RegClass::Invalid || WalkFindRegClass(Src1) == RegClass::GPR, "GetOpRegClass(Src1) == RegClass::Invalid || WalkFindRegClass(Src1) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src2) == RegClass::Invalid || WalkFindRegClass(Src2) == RegClass::GPR, "GetOpRegClass(Src2) == RegClass::Invalid || WalkFindRegClass(Src2) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit, "Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit");
#endif
		return _SubWithFlags(Size, Src1->Wrapped(ListDataBegin), Src2->Wrapped(ListDataBegin));
	}

	/// Compares register pairs and sets Z accordingly, preserving N/Z/V.
	///
	/// This accelerates cmpxchg.
	IRPair<IROp_CmpPairZ> _CmpPairZ(FEXCore::IR::OpSize Size, OrderedNodeWrapper Src1Lo, OrderedNodeWrapper Src1Hi, OrderedNodeWrapper Src2Lo, OrderedNodeWrapper Src2Hi) {
		auto _Op = AllocateOp<IROp_CmpPairZ, IROps::OP_CMPPAIRZ>();
		_Op.first->Src1Lo = Src1Lo;
		_Op.first->Src1Hi = Src1Hi;
		_Op.first->Src2Lo = Src2Lo;
		_Op.first->Src2Hi = Src2Hi;
		_Op.first->Header.Size = Size;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Compares register pairs and sets Z accordingly, preserving N/Z/V.
	///
	/// This accelerates cmpxchg.
	IRPair<IROp_CmpPairZ> _CmpPairZ(FEXCore::IR::OpSize Size, OrderedNode *Src1Lo, OrderedNode *Src1Hi, OrderedNode *Src2Lo, OrderedNode *Src2Hi) {
		auto ListDataBegin = DualListData.ListBegin();
		Src1Lo->AddUse();
		Src1Hi->AddUse();
		Src2Lo->AddUse();
		Src2Hi->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src1Lo) == RegClass::Invalid || WalkFindRegClass(Src1Lo) == RegClass::GPR, "GetOpRegClass(Src1Lo) == RegClass::Invalid || WalkFindRegClass(Src1Lo) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src1Hi) == RegClass::Invalid || WalkFindRegClass(Src1Hi) == RegClass::GPR, "GetOpRegClass(Src1Hi) == RegClass::Invalid || WalkFindRegClass(Src1Hi) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src2Lo) == RegClass::Invalid || WalkFindRegClass(Src2Lo) == RegClass::GPR, "GetOpRegClass(Src2Lo) == RegClass::Invalid || WalkFindRegClass(Src2Lo) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src2Hi) == RegClass::Invalid || WalkFindRegClass(Src2Hi) == RegClass::GPR, "GetOpRegClass(Src2Hi) == RegClass::Invalid || WalkFindRegClass(Src2Hi) == RegClass::GPR");
#endif
		return _CmpPairZ(Size, Src1Lo->Wrapped(ListDataBegin), Src1Hi->Wrapped(ListDataBegin), Src2Lo->Wrapped(ListDataBegin), Src2Hi->Wrapped(ListDataBegin));
	}

	/// Set NZCV for the difference of two GPRs. 
	///
	/// Carry flag uses arm64 definition, inverted x86.
	///
	/// 
	IRPair<IROp_SubNZCV> _SubNZCV(FEXCore::IR::OpSize Size, OrderedNodeWrapper Src1, OrderedNodeWrapper Src2) {
		auto _Op = AllocateOp<IROp_SubNZCV, IROps::OP_SUBNZCV>();
		_Op.first->Src1 = Src1;
		_Op.first->Src2 = Src2;
		_Op.first->Header.Size = Size;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Set NZCV for the difference of two GPRs. 
	///
	/// Carry flag uses arm64 definition, inverted x86.
	///
	/// 
	IRPair<IROp_SubNZCV> _SubNZCV(FEXCore::IR::OpSize Size, OrderedNode *Src1, OrderedNode *Src2) {
		auto ListDataBegin = DualListData.ListBegin();
		Src1 = InlineZero(Size, Src1);
		Src1->AddUse();
		Src2 = InlineLargeAddSub(Size, Src2);
		Src2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src1) == RegClass::Invalid || WalkFindRegClass(Src1) == RegClass::GPR, "GetOpRegClass(Src1) == RegClass::Invalid || WalkFindRegClass(Src1) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src2) == RegClass::Invalid || WalkFindRegClass(Src2) == RegClass::GPR, "GetOpRegClass(Src2) == RegClass::Invalid || WalkFindRegClass(Src2) == RegClass::GPR");
#endif
		return _SubNZCV(Size, Src1->Wrapped(ListDataBegin), Src2->Wrapped(ListDataBegin));
	}

	/// Integer binary or
	IRPair<IROp_Or> _Or(FEXCore::IR::OpSize Size, OrderedNodeWrapper Src1, OrderedNodeWrapper Src2) {
		auto _Op = AllocateOp<IROp_Or, IROps::OP_OR>();
		_Op.first->Src1 = Src1;
		_Op.first->Src2 = Src2;
		_Op.first->Header.Size = Size;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Integer binary or
	IRPair<IROp_Or> _Or(FEXCore::IR::OpSize Size, OrderedNode *Src1, OrderedNode *Src2) {
		auto ListDataBegin = DualListData.ListBegin();
		Src1->AddUse();
		Src2 = InlineLogical(Size, Src2);
		Src2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src1) == RegClass::Invalid || WalkFindRegClass(Src1) == RegClass::GPR, "GetOpRegClass(Src1) == RegClass::Invalid || WalkFindRegClass(Src1) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src2) == RegClass::Invalid || WalkFindRegClass(Src2) == RegClass::GPR, "GetOpRegClass(Src2) == RegClass::Invalid || WalkFindRegClass(Src2) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit, "Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit");
#endif
		return _Or(Size, Src1->Wrapped(ListDataBegin), Src2->Wrapped(ListDataBegin));
	}

	/// Integer binary or with logical shift left
	IRPair<IROp_Orlshl> _Orlshl(FEXCore::IR::OpSize Size, OrderedNodeWrapper Src1, OrderedNodeWrapper Src2, uint8_t BitShift) {
		auto _Op = AllocateOp<IROp_Orlshl, IROps::OP_ORLSHL>();
		_Op.first->Src1 = Src1;
		_Op.first->Src2 = Src2;
		_Op.first->BitShift = BitShift;
		_Op.first->Header.Size = Size;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Integer binary or with logical shift left
	IRPair<IROp_Orlshl> _Orlshl(FEXCore::IR::OpSize Size, OrderedNode *Src1, OrderedNode *Src2, uint8_t BitShift) {
		auto ListDataBegin = DualListData.ListBegin();
		Src1->AddUse();
		Src2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src1) == RegClass::Invalid || WalkFindRegClass(Src1) == RegClass::GPR, "GetOpRegClass(Src1) == RegClass::Invalid || WalkFindRegClass(Src1) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src2) == RegClass::Invalid || WalkFindRegClass(Src2) == RegClass::GPR, "GetOpRegClass(Src2) == RegClass::Invalid || WalkFindRegClass(Src2) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit, "Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit");
#endif
		return _Orlshl(Size, Src1->Wrapped(ListDataBegin), Src2->Wrapped(ListDataBegin), BitShift);
	}

	/// Integer binary or with logical shift right
	IRPair<IROp_Orlshr> _Orlshr(FEXCore::IR::OpSize Size, OrderedNodeWrapper Src1, OrderedNodeWrapper Src2, uint8_t BitShift) {
		auto _Op = AllocateOp<IROp_Orlshr, IROps::OP_ORLSHR>();
		_Op.first->Src1 = Src1;
		_Op.first->Src2 = Src2;
		_Op.first->BitShift = BitShift;
		_Op.first->Header.Size = Size;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Integer binary or with logical shift right
	IRPair<IROp_Orlshr> _Orlshr(FEXCore::IR::OpSize Size, OrderedNode *Src1, OrderedNode *Src2, uint8_t BitShift) {
		auto ListDataBegin = DualListData.ListBegin();
		Src1->AddUse();
		Src2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src1) == RegClass::Invalid || WalkFindRegClass(Src1) == RegClass::GPR, "GetOpRegClass(Src1) == RegClass::Invalid || WalkFindRegClass(Src1) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src2) == RegClass::Invalid || WalkFindRegClass(Src2) == RegClass::GPR, "GetOpRegClass(Src2) == RegClass::Invalid || WalkFindRegClass(Src2) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit, "Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit");
#endif
		return _Orlshr(Size, Src1->Wrapped(ListDataBegin), Src2->Wrapped(ListDataBegin), BitShift);
	}

	/// Integer binary or with NOT on second source and rotation right
	IRPair<IROp_Ornror> _Ornror(FEXCore::IR::OpSize Size, OrderedNodeWrapper Src1, OrderedNodeWrapper Src2, uint8_t BitShift) {
		auto _Op = AllocateOp<IROp_Ornror, IROps::OP_ORNROR>();
		_Op.first->Src1 = Src1;
		_Op.first->Src2 = Src2;
		_Op.first->BitShift = BitShift;
		_Op.first->Header.Size = Size;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Integer binary or with NOT on second source and rotation right
	IRPair<IROp_Ornror> _Ornror(FEXCore::IR::OpSize Size, OrderedNode *Src1, OrderedNode *Src2, uint8_t BitShift) {
		auto ListDataBegin = DualListData.ListBegin();
		Src1->AddUse();
		Src2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src1) == RegClass::Invalid || WalkFindRegClass(Src1) == RegClass::GPR, "GetOpRegClass(Src1) == RegClass::Invalid || WalkFindRegClass(Src1) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src2) == RegClass::Invalid || WalkFindRegClass(Src2) == RegClass::GPR, "GetOpRegClass(Src2) == RegClass::Invalid || WalkFindRegClass(Src2) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit, "Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit");
#endif
		return _Ornror(Size, Src1->Wrapped(ListDataBegin), Src2->Wrapped(ListDataBegin), BitShift);
	}

	/// Integer binary exclusive or
	IRPair<IROp_Xor> _Xor(FEXCore::IR::OpSize Size, OrderedNodeWrapper Src1, OrderedNodeWrapper Src2) {
		auto _Op = AllocateOp<IROp_Xor, IROps::OP_XOR>();
		_Op.first->Src1 = Src1;
		_Op.first->Src2 = Src2;
		_Op.first->Header.Size = Size;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Integer binary exclusive or
	IRPair<IROp_Xor> _Xor(FEXCore::IR::OpSize Size, OrderedNode *Src1, OrderedNode *Src2) {
		auto ListDataBegin = DualListData.ListBegin();
		Src1->AddUse();
		Src2 = InlineLogical(Size, Src2);
		Src2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src1) == RegClass::Invalid || WalkFindRegClass(Src1) == RegClass::GPR, "GetOpRegClass(Src1) == RegClass::Invalid || WalkFindRegClass(Src1) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src2) == RegClass::Invalid || WalkFindRegClass(Src2) == RegClass::GPR, "GetOpRegClass(Src2) == RegClass::Invalid || WalkFindRegClass(Src2) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit, "Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit");
#endif
		return _Xor(Size, Src1->Wrapped(ListDataBegin), Src2->Wrapped(ListDataBegin));
	}

	/// Integer binary exclusive or with shifted register
	IRPair<IROp_XorShift> _XorShift(FEXCore::IR::OpSize Size, OrderedNodeWrapper Src1, OrderedNodeWrapper Src2, FEXCore::IR::ShiftType Shift = ShiftType::LSL, uint8_t ShiftAmount = 0) {
		auto _Op = AllocateOp<IROp_XorShift, IROps::OP_XORSHIFT>();
		_Op.first->Src1 = Src1;
		_Op.first->Src2 = Src2;
		_Op.first->Shift = Shift;
		_Op.first->ShiftAmount = ShiftAmount;
		_Op.first->Header.Size = Size;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Integer binary exclusive or with shifted register
	IRPair<IROp_XorShift> _XorShift(FEXCore::IR::OpSize Size, OrderedNode *Src1, OrderedNode *Src2, FEXCore::IR::ShiftType Shift = ShiftType::LSL, uint8_t ShiftAmount = 0) {
		auto ListDataBegin = DualListData.ListBegin();
		Src1->AddUse();
		Src2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src1) == RegClass::Invalid || WalkFindRegClass(Src1) == RegClass::GPR, "GetOpRegClass(Src1) == RegClass::Invalid || WalkFindRegClass(Src1) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src2) == RegClass::Invalid || WalkFindRegClass(Src2) == RegClass::GPR, "GetOpRegClass(Src2) == RegClass::Invalid || WalkFindRegClass(Src2) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit, "Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit");
#endif
		return _XorShift(Size, Src1->Wrapped(ListDataBegin), Src2->Wrapped(ListDataBegin), Shift, ShiftAmount);
	}

	/// Integer binary exclusive or not with shifted register
	IRPair<IROp_XornShift> _XornShift(FEXCore::IR::OpSize Size, OrderedNodeWrapper Src1, OrderedNodeWrapper Src2, FEXCore::IR::ShiftType Shift = ShiftType::LSL, uint8_t ShiftAmount = 0) {
		auto _Op = AllocateOp<IROp_XornShift, IROps::OP_XORNSHIFT>();
		_Op.first->Src1 = Src1;
		_Op.first->Src2 = Src2;
		_Op.first->Shift = Shift;
		_Op.first->ShiftAmount = ShiftAmount;
		_Op.first->Header.Size = Size;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Integer binary exclusive or not with shifted register
	IRPair<IROp_XornShift> _XornShift(FEXCore::IR::OpSize Size, OrderedNode *Src1, OrderedNode *Src2, FEXCore::IR::ShiftType Shift = ShiftType::LSL, uint8_t ShiftAmount = 0) {
		auto ListDataBegin = DualListData.ListBegin();
		Src1->AddUse();
		Src2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src1) == RegClass::Invalid || WalkFindRegClass(Src1) == RegClass::GPR, "GetOpRegClass(Src1) == RegClass::Invalid || WalkFindRegClass(Src1) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src2) == RegClass::Invalid || WalkFindRegClass(Src2) == RegClass::GPR, "GetOpRegClass(Src2) == RegClass::Invalid || WalkFindRegClass(Src2) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit, "Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit");
#endif
		return _XornShift(Size, Src1->Wrapped(ListDataBegin), Src2->Wrapped(ListDataBegin), Shift, ShiftAmount);
	}

	/// Integer binary and
	IRPair<IROp_And> _And(FEXCore::IR::OpSize Size, OrderedNodeWrapper Src1, OrderedNodeWrapper Src2) {
		auto _Op = AllocateOp<IROp_And, IROps::OP_AND>();
		_Op.first->Src1 = Src1;
		_Op.first->Src2 = Src2;
		_Op.first->Header.Size = Size;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Integer binary and
	IRPair<IROp_And> _And(FEXCore::IR::OpSize Size, OrderedNode *Src1, OrderedNode *Src2) {
		auto ListDataBegin = DualListData.ListBegin();
		Src1->AddUse();
		Src2 = InlineLogical(Size, Src2);
		Src2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src1) == RegClass::Invalid || WalkFindRegClass(Src1) == RegClass::GPR, "GetOpRegClass(Src1) == RegClass::Invalid || WalkFindRegClass(Src1) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src2) == RegClass::Invalid || WalkFindRegClass(Src2) == RegClass::GPR, "GetOpRegClass(Src2) == RegClass::Invalid || WalkFindRegClass(Src2) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit, "Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit");
#endif
		return _And(Size, Src1->Wrapped(ListDataBegin), Src2->Wrapped(ListDataBegin));
	}

	/// Integer binary and with shifted register
	IRPair<IROp_AndShift> _AndShift(FEXCore::IR::OpSize Size, OrderedNodeWrapper Src1, OrderedNodeWrapper Src2, FEXCore::IR::ShiftType Shift = ShiftType::LSL, uint8_t ShiftAmount = 0) {
		auto _Op = AllocateOp<IROp_AndShift, IROps::OP_ANDSHIFT>();
		_Op.first->Src1 = Src1;
		_Op.first->Src2 = Src2;
		_Op.first->Shift = Shift;
		_Op.first->ShiftAmount = ShiftAmount;
		_Op.first->Header.Size = Size;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Integer binary and with shifted register
	IRPair<IROp_AndShift> _AndShift(FEXCore::IR::OpSize Size, OrderedNode *Src1, OrderedNode *Src2, FEXCore::IR::ShiftType Shift = ShiftType::LSL, uint8_t ShiftAmount = 0) {
		auto ListDataBegin = DualListData.ListBegin();
		Src1->AddUse();
		Src2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src1) == RegClass::Invalid || WalkFindRegClass(Src1) == RegClass::GPR, "GetOpRegClass(Src1) == RegClass::Invalid || WalkFindRegClass(Src1) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src2) == RegClass::Invalid || WalkFindRegClass(Src2) == RegClass::GPR, "GetOpRegClass(Src2) == RegClass::Invalid || WalkFindRegClass(Src2) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit, "Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit");
#endif
		return _AndShift(Size, Src1->Wrapped(ListDataBegin), Src2->Wrapped(ListDataBegin), Shift, ShiftAmount);
	}

	/// Integer binary and
	IRPair<IROp_AndWithFlags> _AndWithFlags(FEXCore::IR::OpSize Size, OrderedNodeWrapper Src1, OrderedNodeWrapper Src2) {
		auto _Op = AllocateOp<IROp_AndWithFlags, IROps::OP_ANDWITHFLAGS>();
		_Op.first->Src1 = Src1;
		_Op.first->Src2 = Src2;
		_Op.first->Header.Size = Size;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Integer binary and
	IRPair<IROp_AndWithFlags> _AndWithFlags(FEXCore::IR::OpSize Size, OrderedNode *Src1, OrderedNode *Src2) {
		auto ListDataBegin = DualListData.ListBegin();
		Src1->AddUse();
		Src2 = InlineLogical(Size, Src2);
		Src2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src1) == RegClass::Invalid || WalkFindRegClass(Src1) == RegClass::GPR, "GetOpRegClass(Src1) == RegClass::Invalid || WalkFindRegClass(Src1) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src2) == RegClass::Invalid || WalkFindRegClass(Src2) == RegClass::GPR, "GetOpRegClass(Src2) == RegClass::Invalid || WalkFindRegClass(Src2) == RegClass::GPR");
#endif
		return _AndWithFlags(Size, Src1->Wrapped(ListDataBegin), Src2->Wrapped(ListDataBegin));
	}

	/// Integer binary AND NOT. Performs the equivalent of Src1 & ~Src2
	IRPair<IROp_Andn> _Andn(FEXCore::IR::OpSize Size, OrderedNodeWrapper Src1, OrderedNodeWrapper Src2) {
		auto _Op = AllocateOp<IROp_Andn, IROps::OP_ANDN>();
		_Op.first->Src1 = Src1;
		_Op.first->Src2 = Src2;
		_Op.first->Header.Size = Size;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Integer binary AND NOT. Performs the equivalent of Src1 & ~Src2
	IRPair<IROp_Andn> _Andn(FEXCore::IR::OpSize Size, OrderedNode *Src1, OrderedNode *Src2) {
		auto ListDataBegin = DualListData.ListBegin();
		Src1->AddUse();
		Src2 = InlineLogical(Size, Src2);
		Src2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src1) == RegClass::Invalid || WalkFindRegClass(Src1) == RegClass::GPR, "GetOpRegClass(Src1) == RegClass::Invalid || WalkFindRegClass(Src1) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src2) == RegClass::Invalid || WalkFindRegClass(Src2) == RegClass::GPR, "GetOpRegClass(Src2) == RegClass::Invalid || WalkFindRegClass(Src2) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit, "Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit");
#endif
		return _Andn(Size, Src1->Wrapped(ListDataBegin), Src2->Wrapped(ListDataBegin));
	}

	/// Set NZCV for the binary AND of two GPRs, setting N and Z accordingly and zeroing C and V
	IRPair<IROp_TestNZ> _TestNZ(FEXCore::IR::OpSize Size, OrderedNodeWrapper Src1, OrderedNodeWrapper Src2) {
		auto _Op = AllocateOp<IROp_TestNZ, IROps::OP_TESTNZ>();
		_Op.first->Src1 = Src1;
		_Op.first->Src2 = Src2;
		_Op.first->Header.Size = Size;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Set NZCV for the binary AND of two GPRs, setting N and Z accordingly and zeroing C and V
	IRPair<IROp_TestNZ> _TestNZ(FEXCore::IR::OpSize Size, OrderedNode *Src1, OrderedNode *Src2) {
		auto ListDataBegin = DualListData.ListBegin();
		Src1->AddUse();
		Src2 = InlineLogical(Size, Src2);
		Src2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src1) == RegClass::Invalid || WalkFindRegClass(Src1) == RegClass::GPR, "GetOpRegClass(Src1) == RegClass::Invalid || WalkFindRegClass(Src1) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src2) == RegClass::Invalid || WalkFindRegClass(Src2) == RegClass::GPR, "GetOpRegClass(Src2) == RegClass::Invalid || WalkFindRegClass(Src2) == RegClass::GPR");
#endif
		return _TestNZ(Size, Src1->Wrapped(ListDataBegin), Src2->Wrapped(ListDataBegin));
	}

	/// Set NZCV for the binary AND of two GPRs, setting Z accordingly and zeroing C and V. N is undefined.
	IRPair<IROp_TestZ> _TestZ(FEXCore::IR::OpSize Size, OrderedNodeWrapper Src1, OrderedNodeWrapper Src2) {
		auto _Op = AllocateOp<IROp_TestZ, IROps::OP_TESTZ>();
		_Op.first->Src1 = Src1;
		_Op.first->Src2 = Src2;
		_Op.first->Header.Size = Size;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Set NZCV for the binary AND of two GPRs, setting Z accordingly and zeroing C and V. N is undefined.
	IRPair<IROp_TestZ> _TestZ(FEXCore::IR::OpSize Size, OrderedNode *Src1, OrderedNode *Src2) {
		auto ListDataBegin = DualListData.ListBegin();
		Src1->AddUse();
		Src2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src1) == RegClass::Invalid || WalkFindRegClass(Src1) == RegClass::GPR, "GetOpRegClass(Src1) == RegClass::Invalid || WalkFindRegClass(Src1) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src2) == RegClass::Invalid || WalkFindRegClass(Src2) == RegClass::GPR, "GetOpRegClass(Src2) == RegClass::Invalid || WalkFindRegClass(Src2) == RegClass::GPR");
#endif
		return _TestZ(Size, Src1->Wrapped(ListDataBegin), Src2->Wrapped(ListDataBegin));
	}

	/// Integer logical shift left
	IRPair<IROp_Lshl> _Lshl(FEXCore::IR::OpSize Size, OrderedNodeWrapper Src1, OrderedNodeWrapper Src2) {
		auto _Op = AllocateOp<IROp_Lshl, IROps::OP_LSHL>();
		_Op.first->Src1 = Src1;
		_Op.first->Src2 = Src2;
		_Op.first->Header.Size = Size;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Integer logical shift left
	IRPair<IROp_Lshl> _Lshl(FEXCore::IR::OpSize Size, OrderedNode *Src1, OrderedNode *Src2) {
		auto ListDataBegin = DualListData.ListBegin();
		Src1->AddUse();
		Src2 = InlineAny(Size, Src2);
		Src2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src1) == RegClass::Invalid || WalkFindRegClass(Src1) == RegClass::GPR, "GetOpRegClass(Src1) == RegClass::Invalid || WalkFindRegClass(Src1) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src2) == RegClass::Invalid || WalkFindRegClass(Src2) == RegClass::GPR, "GetOpRegClass(Src2) == RegClass::Invalid || WalkFindRegClass(Src2) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit, "Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit");
#endif
		return _Lshl(Size, Src1->Wrapped(ListDataBegin), Src2->Wrapped(ListDataBegin));
	}

	/// Integer logical shift right
	IRPair<IROp_Lshr> _Lshr(FEXCore::IR::OpSize Size, OrderedNodeWrapper Src1, OrderedNodeWrapper Src2) {
		auto _Op = AllocateOp<IROp_Lshr, IROps::OP_LSHR>();
		_Op.first->Src1 = Src1;
		_Op.first->Src2 = Src2;
		_Op.first->Header.Size = Size;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Integer logical shift right
	IRPair<IROp_Lshr> _Lshr(FEXCore::IR::OpSize Size, OrderedNode *Src1, OrderedNode *Src2) {
		auto ListDataBegin = DualListData.ListBegin();
		Src1->AddUse();
		Src2 = InlineAny(Size, Src2);
		Src2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src1) == RegClass::Invalid || WalkFindRegClass(Src1) == RegClass::GPR, "GetOpRegClass(Src1) == RegClass::Invalid || WalkFindRegClass(Src1) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src2) == RegClass::Invalid || WalkFindRegClass(Src2) == RegClass::GPR, "GetOpRegClass(Src2) == RegClass::Invalid || WalkFindRegClass(Src2) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit, "Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit");
#endif
		return _Lshr(Size, Src1->Wrapped(ListDataBegin), Src2->Wrapped(ListDataBegin));
	}

	/// Integer arithmetic shift right
	IRPair<IROp_Ashr> _Ashr(FEXCore::IR::OpSize Size, OrderedNodeWrapper Src1, OrderedNodeWrapper Src2) {
		auto _Op = AllocateOp<IROp_Ashr, IROps::OP_ASHR>();
		_Op.first->Src1 = Src1;
		_Op.first->Src2 = Src2;
		_Op.first->Header.Size = Size;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Integer arithmetic shift right
	IRPair<IROp_Ashr> _Ashr(FEXCore::IR::OpSize Size, OrderedNode *Src1, OrderedNode *Src2) {
		auto ListDataBegin = DualListData.ListBegin();
		Src1->AddUse();
		Src2 = InlineAny(Size, Src2);
		Src2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src1) == RegClass::Invalid || WalkFindRegClass(Src1) == RegClass::GPR, "GetOpRegClass(Src1) == RegClass::Invalid || WalkFindRegClass(Src1) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src2) == RegClass::Invalid || WalkFindRegClass(Src2) == RegClass::GPR, "GetOpRegClass(Src2) == RegClass::Invalid || WalkFindRegClass(Src2) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit, "Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit");
#endif
		return _Ashr(Size, Src1->Wrapped(ListDataBegin), Src2->Wrapped(ListDataBegin));
	}

	/// Set NZCV flags for specified variable integer shift with given result.
	///
	/// Returns updated raw PF.
	IRPair<IROp_ShiftFlags> _ShiftFlags(FEXCore::IR::OpSize Size, OrderedNodeWrapper Result, OrderedNodeWrapper Src1, FEXCore::IR::ShiftType Shift, OrderedNodeWrapper Src2, OrderedNodeWrapper PFInput, bool InvertCF) {
		auto _Op = AllocateOp<IROp_ShiftFlags, IROps::OP_SHIFTFLAGS>();
		_Op.first->Result = Result;
		_Op.first->Src1 = Src1;
		_Op.first->Src2 = Src2;
		_Op.first->PFInput = PFInput;
		_Op.first->Size = Size;
		_Op.first->Shift = Shift;
		_Op.first->InvertCF = InvertCF;
		_Op.first->Header.Size = OpSize::i64Bit;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Set NZCV flags for specified variable integer shift with given result.
	///
	/// Returns updated raw PF.
	IRPair<IROp_ShiftFlags> _ShiftFlags(FEXCore::IR::OpSize Size, OrderedNode *Result, OrderedNode *Src1, FEXCore::IR::ShiftType Shift, OrderedNode *Src2, OrderedNode *PFInput, bool InvertCF) {
		auto ListDataBegin = DualListData.ListBegin();
		Result->AddUse();
		Src1->AddUse();
		Src2->AddUse();
		PFInput->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Result) == RegClass::Invalid || WalkFindRegClass(Result) == RegClass::GPR, "GetOpRegClass(Result) == RegClass::Invalid || WalkFindRegClass(Result) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src1) == RegClass::Invalid || WalkFindRegClass(Src1) == RegClass::GPR, "GetOpRegClass(Src1) == RegClass::Invalid || WalkFindRegClass(Src1) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src2) == RegClass::Invalid || WalkFindRegClass(Src2) == RegClass::GPR, "GetOpRegClass(Src2) == RegClass::Invalid || WalkFindRegClass(Src2) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(PFInput) == RegClass::Invalid || WalkFindRegClass(PFInput) == RegClass::GPR, "GetOpRegClass(PFInput) == RegClass::Invalid || WalkFindRegClass(PFInput) == RegClass::GPR");
#endif
		return _ShiftFlags(Size, Result->Wrapped(ListDataBegin), Src1->Wrapped(ListDataBegin), Shift, Src2->Wrapped(ListDataBegin), PFInput->Wrapped(ListDataBegin), InvertCF);
	}

	/// Set NZCV flags for specified variable integer rotate with given result.
	IRPair<IROp_RotateFlags> _RotateFlags(FEXCore::IR::OpSize Size, OrderedNodeWrapper Result, OrderedNodeWrapper Shift, bool Left) {
		auto _Op = AllocateOp<IROp_RotateFlags, IROps::OP_ROTATEFLAGS>();
		_Op.first->Result = Result;
		_Op.first->Shift = Shift;
		_Op.first->Size = Size;
		_Op.first->Left = Left;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Set NZCV flags for specified variable integer rotate with given result.
	IRPair<IROp_RotateFlags> _RotateFlags(FEXCore::IR::OpSize Size, OrderedNode *Result, OrderedNode *Shift, bool Left) {
		auto ListDataBegin = DualListData.ListBegin();
		Result->AddUse();
		Shift->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Result) == RegClass::Invalid || WalkFindRegClass(Result) == RegClass::GPR, "GetOpRegClass(Result) == RegClass::Invalid || WalkFindRegClass(Result) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Shift) == RegClass::Invalid || WalkFindRegClass(Shift) == RegClass::GPR, "GetOpRegClass(Shift) == RegClass::Invalid || WalkFindRegClass(Shift) == RegClass::GPR");
#endif
		return _RotateFlags(Size, Result->Wrapped(ListDataBegin), Shift->Wrapped(ListDataBegin), Left);
	}

	/// Integer rotate right
	IRPair<IROp_Ror> _Ror(FEXCore::IR::OpSize Size, OrderedNodeWrapper Src1, OrderedNodeWrapper Src2) {
		auto _Op = AllocateOp<IROp_Ror, IROps::OP_ROR>();
		_Op.first->Src1 = Src1;
		_Op.first->Src2 = Src2;
		_Op.first->Header.Size = Size;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Integer rotate right
	IRPair<IROp_Ror> _Ror(FEXCore::IR::OpSize Size, OrderedNode *Src1, OrderedNode *Src2) {
		auto ListDataBegin = DualListData.ListBegin();
		Src1->AddUse();
		Src2 = InlineAny(Size, Src2);
		Src2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src1) == RegClass::Invalid || WalkFindRegClass(Src1) == RegClass::GPR, "GetOpRegClass(Src1) == RegClass::Invalid || WalkFindRegClass(Src1) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src2) == RegClass::Invalid || WalkFindRegClass(Src2) == RegClass::GPR, "GetOpRegClass(Src2) == RegClass::Invalid || WalkFindRegClass(Src2) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit, "Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit");
#endif
		return _Ror(Size, Src1->Wrapped(ListDataBegin), Src2->Wrapped(ListDataBegin));
	}

	/// Integer signed multiplication
	IRPair<IROp_Mul> _Mul(FEXCore::IR::OpSize Size, OrderedNodeWrapper Src1, OrderedNodeWrapper Src2) {
		auto _Op = AllocateOp<IROp_Mul, IROps::OP_MUL>();
		_Op.first->Src1 = Src1;
		_Op.first->Src2 = Src2;
		_Op.first->Header.Size = Size;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Integer signed multiplication
	IRPair<IROp_Mul> _Mul(FEXCore::IR::OpSize Size, OrderedNode *Src1, OrderedNode *Src2) {
		auto ListDataBegin = DualListData.ListBegin();
		Src1->AddUse();
		Src2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src1) == RegClass::Invalid || WalkFindRegClass(Src1) == RegClass::GPR, "GetOpRegClass(Src1) == RegClass::Invalid || WalkFindRegClass(Src1) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src2) == RegClass::Invalid || WalkFindRegClass(Src2) == RegClass::GPR, "GetOpRegClass(Src2) == RegClass::Invalid || WalkFindRegClass(Src2) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit, "Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit");
#endif
		return _Mul(Size, Src1->Wrapped(ListDataBegin), Src2->Wrapped(ListDataBegin));
	}

	/// Integer unsigned multiplication
	IRPair<IROp_UMul> _UMul(FEXCore::IR::OpSize Size, OrderedNodeWrapper Src1, OrderedNodeWrapper Src2) {
		auto _Op = AllocateOp<IROp_UMul, IROps::OP_UMUL>();
		_Op.first->Src1 = Src1;
		_Op.first->Src2 = Src2;
		_Op.first->Header.Size = Size;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Integer unsigned multiplication
	IRPair<IROp_UMul> _UMul(FEXCore::IR::OpSize Size, OrderedNode *Src1, OrderedNode *Src2) {
		auto ListDataBegin = DualListData.ListBegin();
		Src1->AddUse();
		Src2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src1) == RegClass::Invalid || WalkFindRegClass(Src1) == RegClass::GPR, "GetOpRegClass(Src1) == RegClass::Invalid || WalkFindRegClass(Src1) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src2) == RegClass::Invalid || WalkFindRegClass(Src2) == RegClass::GPR, "GetOpRegClass(Src2) == RegClass::Invalid || WalkFindRegClass(Src2) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit, "Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit");
#endif
		return _UMul(Size, Src1->Wrapped(ListDataBegin), Src2->Wrapped(ListDataBegin));
	}

	/// Integer unsigned multiplication long
	///
	/// Multiplies two 32-bit numbers, returning a 64-bit destination register.
	IRPair<IROp_UMull> _UMull(OrderedNodeWrapper Src1, OrderedNodeWrapper Src2) {
		auto _Op = AllocateOp<IROp_UMull, IROps::OP_UMULL>();
		_Op.first->Src1 = Src1;
		_Op.first->Src2 = Src2;
		_Op.first->Header.Size = FEXCore::IR::OpSize::i64Bit;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Integer unsigned multiplication long
	///
	/// Multiplies two 32-bit numbers, returning a 64-bit destination register.
	IRPair<IROp_UMull> _UMull(OrderedNode *Src1, OrderedNode *Src2) {
		auto ListDataBegin = DualListData.ListBegin();
		Src1->AddUse();
		Src2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src1) == RegClass::Invalid || WalkFindRegClass(Src1) == RegClass::GPR, "GetOpRegClass(Src1) == RegClass::Invalid || WalkFindRegClass(Src1) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src2) == RegClass::Invalid || WalkFindRegClass(Src2) == RegClass::GPR, "GetOpRegClass(Src2) == RegClass::Invalid || WalkFindRegClass(Src2) == RegClass::GPR");
#endif
		return _UMull(Src1->Wrapped(ListDataBegin), Src2->Wrapped(ListDataBegin));
	}

	/// Integer signed multiplication long
	///
	/// Multiplies two 32-bit numbers, returning a 64-bit destination register.
	IRPair<IROp_SMull> _SMull(OrderedNodeWrapper Src1, OrderedNodeWrapper Src2) {
		auto _Op = AllocateOp<IROp_SMull, IROps::OP_SMULL>();
		_Op.first->Src1 = Src1;
		_Op.first->Src2 = Src2;
		_Op.first->Header.Size = FEXCore::IR::OpSize::i64Bit;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Integer signed multiplication long
	///
	/// Multiplies two 32-bit numbers, returning a 64-bit destination register.
	IRPair<IROp_SMull> _SMull(OrderedNode *Src1, OrderedNode *Src2) {
		auto ListDataBegin = DualListData.ListBegin();
		Src1->AddUse();
		Src2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src1) == RegClass::Invalid || WalkFindRegClass(Src1) == RegClass::GPR, "GetOpRegClass(Src1) == RegClass::Invalid || WalkFindRegClass(Src1) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src2) == RegClass::Invalid || WalkFindRegClass(Src2) == RegClass::GPR, "GetOpRegClass(Src2) == RegClass::Invalid || WalkFindRegClass(Src2) == RegClass::GPR");
#endif
		return _SMull(Src1->Wrapped(ListDataBegin), Src2->Wrapped(ListDataBegin));
	}

	/// Integer signed multiply returning high results
	///
	/// op:
	///
	/// Tmp <size * 2> = Src1 * Src2;
	///
	/// Dest = Tmp >> (size * 8);
	IRPair<IROp_MulH> _MulH(FEXCore::IR::OpSize Size, OrderedNodeWrapper Src1, OrderedNodeWrapper Src2) {
		auto _Op = AllocateOp<IROp_MulH, IROps::OP_MULH>();
		_Op.first->Src1 = Src1;
		_Op.first->Src2 = Src2;
		_Op.first->Header.Size = Size;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Integer signed multiply returning high results
	///
	/// op:
	///
	/// Tmp <size * 2> = Src1 * Src2;
	///
	/// Dest = Tmp >> (size * 8);
	IRPair<IROp_MulH> _MulH(FEXCore::IR::OpSize Size, OrderedNode *Src1, OrderedNode *Src2) {
		auto ListDataBegin = DualListData.ListBegin();
		Src1->AddUse();
		Src2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src1) == RegClass::Invalid || WalkFindRegClass(Src1) == RegClass::GPR, "GetOpRegClass(Src1) == RegClass::Invalid || WalkFindRegClass(Src1) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src2) == RegClass::Invalid || WalkFindRegClass(Src2) == RegClass::GPR, "GetOpRegClass(Src2) == RegClass::Invalid || WalkFindRegClass(Src2) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit, "Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit");
#endif
		return _MulH(Size, Src1->Wrapped(ListDataBegin), Src2->Wrapped(ListDataBegin));
	}

	/// Integer unsigned multiply returning high results
	///
	/// op:
	///
	/// Tmp <size * 2> = Src1 * Src2;
	///
	/// Dest = Tmp >> (size * 8);
	IRPair<IROp_UMulH> _UMulH(FEXCore::IR::OpSize Size, OrderedNodeWrapper Src1, OrderedNodeWrapper Src2) {
		auto _Op = AllocateOp<IROp_UMulH, IROps::OP_UMULH>();
		_Op.first->Src1 = Src1;
		_Op.first->Src2 = Src2;
		_Op.first->Header.Size = Size;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Integer unsigned multiply returning high results
	///
	/// op:
	///
	/// Tmp <size * 2> = Src1 * Src2;
	///
	/// Dest = Tmp >> (size * 8);
	IRPair<IROp_UMulH> _UMulH(FEXCore::IR::OpSize Size, OrderedNode *Src1, OrderedNode *Src2) {
		auto ListDataBegin = DualListData.ListBegin();
		Src1->AddUse();
		Src2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src1) == RegClass::Invalid || WalkFindRegClass(Src1) == RegClass::GPR, "GetOpRegClass(Src1) == RegClass::Invalid || WalkFindRegClass(Src1) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src2) == RegClass::Invalid || WalkFindRegClass(Src2) == RegClass::GPR, "GetOpRegClass(Src2) == RegClass::Invalid || WalkFindRegClass(Src2) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit, "Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit");
#endif
		return _UMulH(Size, Src1->Wrapped(ListDataBegin), Src2->Wrapped(ListDataBegin));
	}

	/// Copies a bitfield from one GPR to another
	///
	/// The source bitfield is from Src[Width:0]
	///
	/// The bitfield is copied in to Dest[(Width + lsb):lsb]
	IRPair<IROp_Bfi> _Bfi(FEXCore::IR::OpSize Size, uint8_t Width, uint8_t lsb, OrderedNodeWrapper Dest, OrderedNodeWrapper Src) {
		auto _Op = AllocateOp<IROp_Bfi, IROps::OP_BFI>();
		_Op.first->Dest = Dest;
		_Op.first->Src = Src;
		_Op.first->Width = Width;
		_Op.first->lsb = lsb;
		_Op.first->Header.Size = Size;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Copies a bitfield from one GPR to another
	///
	/// The source bitfield is from Src[Width:0]
	///
	/// The bitfield is copied in to Dest[(Width + lsb):lsb]
	IRPair<IROp_Bfi> _Bfi(FEXCore::IR::OpSize Size, uint8_t Width, uint8_t lsb, OrderedNode *Dest, OrderedNode *Src) {
		auto ListDataBegin = DualListData.ListBegin();
		Dest->AddUse();
		Src->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Dest) == RegClass::Invalid || WalkFindRegClass(Dest) == RegClass::GPR, "GetOpRegClass(Dest) == RegClass::Invalid || WalkFindRegClass(Dest) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src) == RegClass::Invalid || WalkFindRegClass(Src) == RegClass::GPR, "GetOpRegClass(Src) == RegClass::Invalid || WalkFindRegClass(Src) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit, "Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit");
		LOGMAN_THROW_A_FMT((Width + lsb) <= IR::OpSizeAsBits(Size), "(Width + lsb) <= IR::OpSizeAsBits(Size)");
#endif
		return _Bfi(Size, Width, lsb, Dest->Wrapped(ListDataBegin), Src->Wrapped(ListDataBegin));
	}

	/// Copies a bitfield from one GPR to another
	///
	/// Inserting in to the low bits of the destination
	///
	/// The source bitfield is from Src[(Width + lsb):lsb]
	///
	/// The bitfield is copied in to Dest[Width:0]
	IRPair<IROp_Bfxil> _Bfxil(FEXCore::IR::OpSize Size, uint8_t Width, uint8_t lsb, OrderedNodeWrapper Dest, OrderedNodeWrapper Src) {
		auto _Op = AllocateOp<IROp_Bfxil, IROps::OP_BFXIL>();
		_Op.first->Dest = Dest;
		_Op.first->Src = Src;
		_Op.first->Width = Width;
		_Op.first->lsb = lsb;
		_Op.first->Header.Size = Size;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Copies a bitfield from one GPR to another
	///
	/// Inserting in to the low bits of the destination
	///
	/// The source bitfield is from Src[(Width + lsb):lsb]
	///
	/// The bitfield is copied in to Dest[Width:0]
	IRPair<IROp_Bfxil> _Bfxil(FEXCore::IR::OpSize Size, uint8_t Width, uint8_t lsb, OrderedNode *Dest, OrderedNode *Src) {
		auto ListDataBegin = DualListData.ListBegin();
		Dest->AddUse();
		Src->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Dest) == RegClass::Invalid || WalkFindRegClass(Dest) == RegClass::GPR, "GetOpRegClass(Dest) == RegClass::Invalid || WalkFindRegClass(Dest) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src) == RegClass::Invalid || WalkFindRegClass(Src) == RegClass::GPR, "GetOpRegClass(Src) == RegClass::Invalid || WalkFindRegClass(Src) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit, "Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit");
		LOGMAN_THROW_A_FMT((Width + lsb) <= IR::OpSizeAsBits(Size), "(Width + lsb) <= IR::OpSizeAsBits(Size)");
#endif
		return _Bfxil(Size, Width, lsb, Dest->Wrapped(ListDataBegin), Src->Wrapped(ListDataBegin));
	}

	/// Extracts a bitfield from one GPR with zext
	///
	/// The source bitfield is from Src[Width:0]
	///
	/// The bitfield is then zero extended
	IRPair<IROp_Bfe> _Bfe(FEXCore::IR::OpSize Size, uint8_t Width, uint8_t lsb, OrderedNodeWrapper Src) {
		auto _Op = AllocateOp<IROp_Bfe, IROps::OP_BFE>();
		_Op.first->Src = Src;
		_Op.first->Width = Width;
		_Op.first->lsb = lsb;
		_Op.first->Header.Size = Size;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Extracts a bitfield from one GPR with zext
	///
	/// The source bitfield is from Src[Width:0]
	///
	/// The bitfield is then zero extended
	IRPair<IROp_Bfe> _Bfe(FEXCore::IR::OpSize Size, uint8_t Width, uint8_t lsb, OrderedNode *Src) {
		auto ListDataBegin = DualListData.ListBegin();
		Src->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src) == RegClass::Invalid || WalkFindRegClass(Src) == RegClass::GPR, "GetOpRegClass(Src) == RegClass::Invalid || WalkFindRegClass(Src) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit, "Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit");
		LOGMAN_THROW_A_FMT((Width + lsb) <= IR::OpSizeAsBits(Size), "(Width + lsb) <= IR::OpSizeAsBits(Size)");
#endif
		return _Bfe(Size, Width, lsb, Src->Wrapped(ListDataBegin));
	}

	/// Extracts a bitfield from one GPR with sext
	///
	/// The source bitfield is from Src[Width:0]
	///
	/// The bitfield is then sign extended
	IRPair<IROp_Sbfe> _Sbfe(FEXCore::IR::OpSize Size, uint8_t Width, uint8_t lsb, OrderedNodeWrapper Src) {
		auto _Op = AllocateOp<IROp_Sbfe, IROps::OP_SBFE>();
		_Op.first->Src = Src;
		_Op.first->Width = Width;
		_Op.first->lsb = lsb;
		_Op.first->Header.Size = Size;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Extracts a bitfield from one GPR with sext
	///
	/// The source bitfield is from Src[Width:0]
	///
	/// The bitfield is then sign extended
	IRPair<IROp_Sbfe> _Sbfe(FEXCore::IR::OpSize Size, uint8_t Width, uint8_t lsb, OrderedNode *Src) {
		auto ListDataBegin = DualListData.ListBegin();
		Src->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src) == RegClass::Invalid || WalkFindRegClass(Src) == RegClass::GPR, "GetOpRegClass(Src) == RegClass::Invalid || WalkFindRegClass(Src) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit, "Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit");
		LOGMAN_THROW_A_FMT((Width + lsb) <= IR::OpSizeAsBits(Size), "(Width + lsb) <= IR::OpSizeAsBits(Size)");
#endif
		return _Sbfe(Size, Width, lsb, Src->Wrapped(ListDataBegin));
	}

	/// Select based on value in NZCV flags
	///
	/// op:
	///
	/// Dest = Cond ? TrueVal : FalseVal
	IRPair<IROp_NZCVSelect> _NZCVSelect(FEXCore::IR::OpSize ResultSize, CondClass Cond, OrderedNodeWrapper TrueVal, OrderedNodeWrapper FalseVal) {
		auto _Op = AllocateOp<IROp_NZCVSelect, IROps::OP_NZCVSELECT>();
		_Op.first->TrueVal = TrueVal;
		_Op.first->FalseVal = FalseVal;
		_Op.first->Cond = Cond;
		_Op.first->Header.Size = ResultSize;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Select based on value in NZCV flags
	///
	/// op:
	///
	/// Dest = Cond ? TrueVal : FalseVal
	IRPair<IROp_NZCVSelect> _NZCVSelect(FEXCore::IR::OpSize ResultSize, CondClass Cond, OrderedNode *TrueVal, OrderedNode *FalseVal) {
		auto ListDataBegin = DualListData.ListBegin();
		TrueVal->AddUse();
		FalseVal->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(TrueVal) == RegClass::Invalid || WalkFindRegClass(TrueVal) == RegClass::GPR, "GetOpRegClass(TrueVal) == RegClass::Invalid || WalkFindRegClass(TrueVal) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(FalseVal) == RegClass::Invalid || WalkFindRegClass(FalseVal) == RegClass::GPR, "GetOpRegClass(FalseVal) == RegClass::Invalid || WalkFindRegClass(FalseVal) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(ResultSize == FEXCore::IR::OpSize::i32Bit || ResultSize == FEXCore::IR::OpSize::i64Bit, "ResultSize == FEXCore::IR::OpSize::i32Bit || ResultSize == FEXCore::IR::OpSize::i64Bit");
#endif
		return _NZCVSelect(ResultSize, Cond, TrueVal->Wrapped(ListDataBegin), FalseVal->Wrapped(ListDataBegin));
	}

	/// Select based on value in NZCV flags, where TrueVal and FalseVal are both FPRs.
	///
	/// op:
	///
	/// Dest = Cond ? TrueVal : FalseVal
	IRPair<IROp_NZCVSelectV> _NZCVSelectV(FEXCore::IR::OpSize ResultSize, CondClass Cond, OrderedNodeWrapper TrueVal, OrderedNodeWrapper FalseVal) {
		auto _Op = AllocateOp<IROp_NZCVSelectV, IROps::OP_NZCVSELECTV>();
		_Op.first->TrueVal = TrueVal;
		_Op.first->FalseVal = FalseVal;
		_Op.first->Cond = Cond;
		_Op.first->Header.Size = ResultSize;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Select based on value in NZCV flags, where TrueVal and FalseVal are both FPRs.
	///
	/// op:
	///
	/// Dest = Cond ? TrueVal : FalseVal
	IRPair<IROp_NZCVSelectV> _NZCVSelectV(FEXCore::IR::OpSize ResultSize, CondClass Cond, OrderedNode *TrueVal, OrderedNode *FalseVal) {
		auto ListDataBegin = DualListData.ListBegin();
		TrueVal->AddUse();
		FalseVal->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(TrueVal) == RegClass::Invalid || WalkFindRegClass(TrueVal) == RegClass::FPR, "GetOpRegClass(TrueVal) == RegClass::Invalid || WalkFindRegClass(TrueVal) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(FalseVal) == RegClass::Invalid || WalkFindRegClass(FalseVal) == RegClass::FPR, "GetOpRegClass(FalseVal) == RegClass::Invalid || WalkFindRegClass(FalseVal) == RegClass::FPR");
#endif
		return _NZCVSelectV(ResultSize, Cond, TrueVal->Wrapped(ListDataBegin), FalseVal->Wrapped(ListDataBegin));
	}

	/// Select and increment based on value in NZCV flags
	///
	/// op:
	///
	/// Dest = Cond ? TrueVal : (FalseVal + 1)
	IRPair<IROp_NZCVSelectIncrement> _NZCVSelectIncrement(FEXCore::IR::OpSize ResultSize, CondClass Cond, OrderedNodeWrapper TrueVal, OrderedNodeWrapper FalseVal) {
		auto _Op = AllocateOp<IROp_NZCVSelectIncrement, IROps::OP_NZCVSELECTINCREMENT>();
		_Op.first->TrueVal = TrueVal;
		_Op.first->FalseVal = FalseVal;
		_Op.first->Cond = Cond;
		_Op.first->Header.Size = ResultSize;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Select and increment based on value in NZCV flags
	///
	/// op:
	///
	/// Dest = Cond ? TrueVal : (FalseVal + 1)
	IRPair<IROp_NZCVSelectIncrement> _NZCVSelectIncrement(FEXCore::IR::OpSize ResultSize, CondClass Cond, OrderedNode *TrueVal, OrderedNode *FalseVal) {
		auto ListDataBegin = DualListData.ListBegin();
		TrueVal->AddUse();
		FalseVal->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(TrueVal) == RegClass::Invalid || WalkFindRegClass(TrueVal) == RegClass::GPR, "GetOpRegClass(TrueVal) == RegClass::Invalid || WalkFindRegClass(TrueVal) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(FalseVal) == RegClass::Invalid || WalkFindRegClass(FalseVal) == RegClass::GPR, "GetOpRegClass(FalseVal) == RegClass::Invalid || WalkFindRegClass(FalseVal) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(ResultSize == FEXCore::IR::OpSize::i32Bit || ResultSize == FEXCore::IR::OpSize::i64Bit, "ResultSize == FEXCore::IR::OpSize::i32Bit || ResultSize == FEXCore::IR::OpSize::i64Bit");
#endif
		return _NZCVSelectIncrement(ResultSize, Cond, TrueVal->Wrapped(ListDataBegin), FalseVal->Wrapped(ListDataBegin));
	}

	/// Ternary selection of GPRs
	///
	/// op:
	///
	/// Dest = Cmp1 <Cond> Cmp2 ? TrueVal : FalseVal
	IRPair<IROp_Select> _Select(FEXCore::IR::OpSize ResultSize, FEXCore::IR::OpSize CompareSize, CondClass Cond, OrderedNodeWrapper Cmp1, OrderedNodeWrapper Cmp2, OrderedNodeWrapper TrueVal, OrderedNodeWrapper FalseVal) {
		SaveNZCV(IROps::OP_SELECT);		auto _Op = AllocateOp<IROp_Select, IROps::OP_SELECT>();
		_Op.first->Cmp1 = Cmp1;
		_Op.first->Cmp2 = Cmp2;
		_Op.first->TrueVal = TrueVal;
		_Op.first->FalseVal = FalseVal;
		_Op.first->CompareSize = CompareSize;
		_Op.first->Cond = Cond;
		_Op.first->Header.Size = ResultSize;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Ternary selection of GPRs
	///
	/// op:
	///
	/// Dest = Cmp1 <Cond> Cmp2 ? TrueVal : FalseVal
	IRPair<IROp_Select> _Select(FEXCore::IR::OpSize ResultSize, FEXCore::IR::OpSize CompareSize, CondClass Cond, OrderedNode *Cmp1, OrderedNode *Cmp2, OrderedNode *TrueVal, OrderedNode *FalseVal) {
		auto ListDataBegin = DualListData.ListBegin();
		Cmp1->AddUse();
		Cmp2 = InlineAddSub(OpSize::i64Bit, Cmp2);
		Cmp2->AddUse();
		TrueVal->AddUse();
		FalseVal->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(TrueVal) == RegClass::Invalid || WalkFindRegClass(TrueVal) == RegClass::GPR, "GetOpRegClass(TrueVal) == RegClass::Invalid || WalkFindRegClass(TrueVal) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(FalseVal) == RegClass::Invalid || WalkFindRegClass(FalseVal) == RegClass::GPR, "GetOpRegClass(FalseVal) == RegClass::Invalid || WalkFindRegClass(FalseVal) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(CompareSize == FEXCore::IR::OpSize::i32Bit || CompareSize == FEXCore::IR::OpSize::i64Bit || CompareSize == FEXCore::IR::OpSize::i128Bit, "CompareSize == FEXCore::IR::OpSize::i32Bit || CompareSize == FEXCore::IR::OpSize::i64Bit || CompareSize == FEXCore::IR::OpSize::i128Bit");
		LOGMAN_THROW_A_FMT(ResultSize == FEXCore::IR::OpSize::i32Bit || ResultSize == FEXCore::IR::OpSize::i64Bit, "ResultSize == FEXCore::IR::OpSize::i32Bit || ResultSize == FEXCore::IR::OpSize::i64Bit");
#endif
		return _Select(ResultSize, CompareSize, Cond, Cmp1->Wrapped(ListDataBegin), Cmp2->Wrapped(ListDataBegin), TrueVal->Wrapped(ListDataBegin), FalseVal->Wrapped(ListDataBegin));
	}

	/// Generates a bit mask from with a value from [0, 63]
	///
	/// 0 is special cased to full-mask
	///
	/// Special operation for SSE4a bitmask generation.
	IRPair<IROp_MaskGenerateFromBitWidth> _MaskGenerateFromBitWidth(OrderedNodeWrapper BitWidth) {
		SaveNZCV(IROps::OP_MASKGENERATEFROMBITWIDTH);		auto _Op = AllocateOp<IROp_MaskGenerateFromBitWidth, IROps::OP_MASKGENERATEFROMBITWIDTH>();
		_Op.first->BitWidth = BitWidth;
		_Op.first->Header.Size = FEXCore::IR::OpSize::i64Bit;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Generates a bit mask from with a value from [0, 63]
	///
	/// 0 is special cased to full-mask
	///
	/// Special operation for SSE4a bitmask generation.
	IRPair<IROp_MaskGenerateFromBitWidth> _MaskGenerateFromBitWidth(OrderedNode *BitWidth) {
		auto ListDataBegin = DualListData.ListBegin();
		BitWidth->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(BitWidth) == RegClass::Invalid || WalkFindRegClass(BitWidth) == RegClass::GPR, "GetOpRegClass(BitWidth) == RegClass::Invalid || WalkFindRegClass(BitWidth) == RegClass::GPR");
#endif
		return _MaskGenerateFromBitWidth(BitWidth->Wrapped(ListDataBegin));
	}

	/// Concats the two GPRs to create a value that is the size of the full two GPRs
	///
	/// It then extracts a bitfield width that size of a GPR from the LSB
	///
	/// Valid LSB range is 0-31 for 32bit and 0-63 for 64bit
	///
	/// <Size * 2> ConcatValue = $Upper:$Lower
	///
	/// Result = ConcatValue<LSB+Size - 1: LSB>
	IRPair<IROp_Extr> _Extr(FEXCore::IR::OpSize Size, OrderedNodeWrapper Upper, OrderedNodeWrapper Lower, uint8_t LSB) {
		auto _Op = AllocateOp<IROp_Extr, IROps::OP_EXTR>();
		_Op.first->Upper = Upper;
		_Op.first->Lower = Lower;
		_Op.first->LSB = LSB;
		_Op.first->Header.Size = Size;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Concats the two GPRs to create a value that is the size of the full two GPRs
	///
	/// It then extracts a bitfield width that size of a GPR from the LSB
	///
	/// Valid LSB range is 0-31 for 32bit and 0-63 for 64bit
	///
	/// <Size * 2> ConcatValue = $Upper:$Lower
	///
	/// Result = ConcatValue<LSB+Size - 1: LSB>
	IRPair<IROp_Extr> _Extr(FEXCore::IR::OpSize Size, OrderedNode *Upper, OrderedNode *Lower, uint8_t LSB) {
		auto ListDataBegin = DualListData.ListBegin();
		Upper->AddUse();
		Lower->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Upper) == RegClass::Invalid || WalkFindRegClass(Upper) == RegClass::GPR, "GetOpRegClass(Upper) == RegClass::Invalid || WalkFindRegClass(Upper) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Lower) == RegClass::Invalid || WalkFindRegClass(Lower) == RegClass::GPR, "GetOpRegClass(Lower) == RegClass::Invalid || WalkFindRegClass(Lower) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit, "Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit");
#endif
		return _Extr(Size, Upper->Wrapped(ListDataBegin), Lower->Wrapped(ListDataBegin), LSB);
	}

	/// Performs a parallel bit deposit.
	///
	/// Takes the contiguous low-order bits and deposits them into
	///
	/// the destination at the locations specified by the Mask.
	IRPair<IROp_PDep> _PDep(FEXCore::IR::OpSize Size, OrderedNodeWrapper Input, OrderedNodeWrapper Mask) {
		auto _Op = AllocateOp<IROp_PDep, IROps::OP_PDEP>();
		_Op.first->Input = Input;
		_Op.first->Mask = Mask;
		_Op.first->Header.Size = Size;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Performs a parallel bit deposit.
	///
	/// Takes the contiguous low-order bits and deposits them into
	///
	/// the destination at the locations specified by the Mask.
	IRPair<IROp_PDep> _PDep(FEXCore::IR::OpSize Size, OrderedNode *Input, OrderedNode *Mask) {
		auto ListDataBegin = DualListData.ListBegin();
		Input->AddUse();
		Mask->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Input) == RegClass::Invalid || WalkFindRegClass(Input) == RegClass::GPR, "GetOpRegClass(Input) == RegClass::Invalid || WalkFindRegClass(Input) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Mask) == RegClass::Invalid || WalkFindRegClass(Mask) == RegClass::GPR, "GetOpRegClass(Mask) == RegClass::Invalid || WalkFindRegClass(Mask) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit, "Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit");
#endif
		return _PDep(Size, Input->Wrapped(ListDataBegin), Mask->Wrapped(ListDataBegin));
	}

	/// Performs a parallel bit extract.
	///
	/// Each bit set in the mask will select the corresponding bit in the Input
	///
	/// and transfers them to the lower contiguous bits in the destination.
	IRPair<IROp_PExt> _PExt(FEXCore::IR::OpSize Size, OrderedNodeWrapper Input, OrderedNodeWrapper Mask) {
		auto _Op = AllocateOp<IROp_PExt, IROps::OP_PEXT>();
		_Op.first->Input = Input;
		_Op.first->Mask = Mask;
		_Op.first->Header.Size = Size;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Performs a parallel bit extract.
	///
	/// Each bit set in the mask will select the corresponding bit in the Input
	///
	/// and transfers them to the lower contiguous bits in the destination.
	IRPair<IROp_PExt> _PExt(FEXCore::IR::OpSize Size, OrderedNode *Input, OrderedNode *Mask) {
		auto ListDataBegin = DualListData.ListBegin();
		Input->AddUse();
		Mask->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Input) == RegClass::Invalid || WalkFindRegClass(Input) == RegClass::GPR, "GetOpRegClass(Input) == RegClass::Invalid || WalkFindRegClass(Input) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Mask) == RegClass::Invalid || WalkFindRegClass(Mask) == RegClass::GPR, "GetOpRegClass(Mask) == RegClass::Invalid || WalkFindRegClass(Mask) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit, "Size == FEXCore::IR::OpSize::i32Bit || Size == FEXCore::IR::OpSize::i64Bit");
#endif
		return _PExt(Size, Input->Wrapped(ListDataBegin), Mask->Wrapped(ListDataBegin));
	}

	/// Integer long signed division returning lower bits
	///
	/// The Lower and Upper registers will be concated together to generate a dividend twice the size
	///
	/// Then the divisor divides the temporary dividend and returns the results in the original sized register
	///
	/// If Upper is invalid, this is a non-long division.
	IRPair<IROp_Div> _Div(FEXCore::IR::OpSize Size, OrderedNodeWrapper Lower, OrderedNodeWrapper Upper, OrderedNodeWrapper Divisor, OrderedNodeWrapper OutQuotient, OrderedNodeWrapper OutRemainder) {
		auto _Op = AllocateOp<IROp_Div, IROps::OP_DIV>();
		_Op.first->Lower = Lower;
		_Op.first->Upper = Upper;
		_Op.first->Divisor = Divisor;
		_Op.first->OutQuotient = OutQuotient;
		_Op.first->OutRemainder = OutRemainder;
		_Op.first->Header.Size = Size;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Integer long signed division returning lower bits
	///
	/// The Lower and Upper registers will be concated together to generate a dividend twice the size
	///
	/// Then the divisor divides the temporary dividend and returns the results in the original sized register
	///
	/// If Upper is invalid, this is a non-long division.
	IRPair<IROp_Div> _Div(FEXCore::IR::OpSize Size, OrderedNode *Lower, OrderedNode *Upper, OrderedNode *Divisor, OrderedNode *OutQuotient, OrderedNode *OutRemainder) {
		auto ListDataBegin = DualListData.ListBegin();
		Lower->AddUse();
		Upper->AddUse();
		Divisor->AddUse();
		OutQuotient->AddUse();
		OutRemainder->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Lower) == RegClass::Invalid || WalkFindRegClass(Lower) == RegClass::GPR, "GetOpRegClass(Lower) == RegClass::Invalid || WalkFindRegClass(Lower) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Upper) == RegClass::Invalid || WalkFindRegClass(Upper) == RegClass::GPR, "GetOpRegClass(Upper) == RegClass::Invalid || WalkFindRegClass(Upper) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Divisor) == RegClass::Invalid || WalkFindRegClass(Divisor) == RegClass::GPR, "GetOpRegClass(Divisor) == RegClass::Invalid || WalkFindRegClass(Divisor) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(OutQuotient) == RegClass::Invalid || WalkFindRegClass(OutQuotient) == RegClass::GPR, "GetOpRegClass(OutQuotient) == RegClass::Invalid || WalkFindRegClass(OutQuotient) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(OutRemainder) == RegClass::Invalid || WalkFindRegClass(OutRemainder) == RegClass::GPR, "GetOpRegClass(OutRemainder) == RegClass::Invalid || WalkFindRegClass(OutRemainder) == RegClass::GPR");
#endif
		return _Div(Size, Lower->Wrapped(ListDataBegin), Upper->Wrapped(ListDataBegin), Divisor->Wrapped(ListDataBegin), OutQuotient->Wrapped(ListDataBegin), OutRemainder->Wrapped(ListDataBegin));
	}

	/// Integer long unsigned division returning lower bits
	///
	/// The Lower and Upper registers will be concated together to generate a dividend twice the size
	///
	/// Then the divisor divides the temporary dividend and returns the results in the original sized register
	///
	/// If Upper is invalid, this is a non-long division.
	IRPair<IROp_UDiv> _UDiv(FEXCore::IR::OpSize Size, OrderedNodeWrapper Lower, OrderedNodeWrapper Upper, OrderedNodeWrapper Divisor, OrderedNodeWrapper OutQuotient, OrderedNodeWrapper OutRemainder) {
		auto _Op = AllocateOp<IROp_UDiv, IROps::OP_UDIV>();
		_Op.first->Lower = Lower;
		_Op.first->Upper = Upper;
		_Op.first->Divisor = Divisor;
		_Op.first->OutQuotient = OutQuotient;
		_Op.first->OutRemainder = OutRemainder;
		_Op.first->Header.Size = Size;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Integer long unsigned division returning lower bits
	///
	/// The Lower and Upper registers will be concated together to generate a dividend twice the size
	///
	/// Then the divisor divides the temporary dividend and returns the results in the original sized register
	///
	/// If Upper is invalid, this is a non-long division.
	IRPair<IROp_UDiv> _UDiv(FEXCore::IR::OpSize Size, OrderedNode *Lower, OrderedNode *Upper, OrderedNode *Divisor, OrderedNode *OutQuotient, OrderedNode *OutRemainder) {
		auto ListDataBegin = DualListData.ListBegin();
		Lower->AddUse();
		Upper->AddUse();
		Divisor->AddUse();
		OutQuotient->AddUse();
		OutRemainder->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Lower) == RegClass::Invalid || WalkFindRegClass(Lower) == RegClass::GPR, "GetOpRegClass(Lower) == RegClass::Invalid || WalkFindRegClass(Lower) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Upper) == RegClass::Invalid || WalkFindRegClass(Upper) == RegClass::GPR, "GetOpRegClass(Upper) == RegClass::Invalid || WalkFindRegClass(Upper) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Divisor) == RegClass::Invalid || WalkFindRegClass(Divisor) == RegClass::GPR, "GetOpRegClass(Divisor) == RegClass::Invalid || WalkFindRegClass(Divisor) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(OutQuotient) == RegClass::Invalid || WalkFindRegClass(OutQuotient) == RegClass::GPR, "GetOpRegClass(OutQuotient) == RegClass::Invalid || WalkFindRegClass(OutQuotient) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(OutRemainder) == RegClass::Invalid || WalkFindRegClass(OutRemainder) == RegClass::GPR, "GetOpRegClass(OutRemainder) == RegClass::Invalid || WalkFindRegClass(OutRemainder) == RegClass::GPR");
#endif
		return _UDiv(Size, Lower->Wrapped(ListDataBegin), Upper->Wrapped(ListDataBegin), Divisor->Wrapped(ListDataBegin), OutQuotient->Wrapped(ListDataBegin), OutRemainder->Wrapped(ListDataBegin));
	}

	/// Extracts an element from a vector and places it in a GPR
	///
	/// The element that is extracted from the vector is zero extended to the GPR size
	IRPair<IROp_VExtractToGPR> _VExtractToGPR(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Vector, uint8_t Index) {
		auto _Op = AllocateOp<IROp_VExtractToGPR, IROps::OP_VEXTRACTTOGPR>();
		_Op.first->Vector = Vector;
		_Op.first->Index = Index;
		_Op.first->Header.Size = ElementSize;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Extracts an element from a vector and places it in a GPR
	///
	/// The element that is extracted from the vector is zero extended to the GPR size
	IRPair<IROp_VExtractToGPR> _VExtractToGPR(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Vector, uint8_t Index) {
		auto ListDataBegin = DualListData.ListBegin();
		Vector->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector) == RegClass::Invalid || WalkFindRegClass(Vector) == RegClass::FPR, "GetOpRegClass(Vector) == RegClass::Invalid || WalkFindRegClass(Vector) == RegClass::FPR");
#endif
		return _VExtractToGPR(RegisterSize, ElementSize, Vector->Wrapped(ListDataBegin), Index);
	}

	/// Moves the scalar element to a GPR with conversion
	///
	/// Converts the 32bit or 64bit float to an signed integer
	///
	/// Rounding mode determined by host flag's rounding mode
	IRPair<IROp_Float_ToGPR_S> _Float_ToGPR_S(FEXCore::IR::OpSize DestElementSize, FEXCore::IR::OpSize SrcElementSize, OrderedNodeWrapper Scalar) {
		auto _Op = AllocateOp<IROp_Float_ToGPR_S, IROps::OP_FLOAT_TOGPR_S>();
		_Op.first->Scalar = Scalar;
		_Op.first->SrcElementSize = SrcElementSize;
		_Op.first->Header.Size = DestElementSize;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Moves the scalar element to a GPR with conversion
	///
	/// Converts the 32bit or 64bit float to an signed integer
	///
	/// Rounding mode determined by host flag's rounding mode
	IRPair<IROp_Float_ToGPR_S> _Float_ToGPR_S(FEXCore::IR::OpSize DestElementSize, FEXCore::IR::OpSize SrcElementSize, OrderedNode *Scalar) {
		auto ListDataBegin = DualListData.ListBegin();
		Scalar->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Scalar) == RegClass::Invalid || WalkFindRegClass(Scalar) == RegClass::FPR, "GetOpRegClass(Scalar) == RegClass::Invalid || WalkFindRegClass(Scalar) == RegClass::FPR");
#endif
		return _Float_ToGPR_S(DestElementSize, SrcElementSize, Scalar->Wrapped(ListDataBegin));
	}

	/// Moves the scalar element to a GPR with conversion
	///
	/// Converts the 32bit or 64bit float to an signed integer rounding towards zero (Truncating)
	IRPair<IROp_Float_ToGPR_ZS> _Float_ToGPR_ZS(FEXCore::IR::OpSize DestElementSize, FEXCore::IR::OpSize SrcElementSize, OrderedNodeWrapper Scalar) {
		auto _Op = AllocateOp<IROp_Float_ToGPR_ZS, IROps::OP_FLOAT_TOGPR_ZS>();
		_Op.first->Scalar = Scalar;
		_Op.first->SrcElementSize = SrcElementSize;
		_Op.first->Header.Size = DestElementSize;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Moves the scalar element to a GPR with conversion
	///
	/// Converts the 32bit or 64bit float to an signed integer rounding towards zero (Truncating)
	IRPair<IROp_Float_ToGPR_ZS> _Float_ToGPR_ZS(FEXCore::IR::OpSize DestElementSize, FEXCore::IR::OpSize SrcElementSize, OrderedNode *Scalar) {
		auto ListDataBegin = DualListData.ListBegin();
		Scalar->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Scalar) == RegClass::Invalid || WalkFindRegClass(Scalar) == RegClass::FPR, "GetOpRegClass(Scalar) == RegClass::Invalid || WalkFindRegClass(Scalar) == RegClass::FPR");
#endif
		return _Float_ToGPR_ZS(DestElementSize, SrcElementSize, Scalar->Wrapped(ListDataBegin));
	}

	/// Does a scalar unordered compare and sets NZCV accordingly.
	///
	/// NZCV follows Arm conventions, a separate AXFLAG instruction is required for x86
	///
	/// Ordering flag result is true if either float input is NaN
	IRPair<IROp_FCmp> _FCmp(FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Scalar1, OrderedNodeWrapper Scalar2) {
		auto _Op = AllocateOp<IROp_FCmp, IROps::OP_FCMP>();
		_Op.first->Scalar1 = Scalar1;
		_Op.first->Scalar2 = Scalar2;
		_Op.first->ElementSize = ElementSize;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Does a scalar unordered compare and sets NZCV accordingly.
	///
	/// NZCV follows Arm conventions, a separate AXFLAG instruction is required for x86
	///
	/// Ordering flag result is true if either float input is NaN
	IRPair<IROp_FCmp> _FCmp(FEXCore::IR::OpSize ElementSize, OrderedNode *Scalar1, OrderedNode *Scalar2) {
		auto ListDataBegin = DualListData.ListBegin();
		Scalar1->AddUse();
		Scalar2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Scalar1) == RegClass::Invalid || WalkFindRegClass(Scalar1) == RegClass::FPR, "GetOpRegClass(Scalar1) == RegClass::Invalid || WalkFindRegClass(Scalar1) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Scalar2) == RegClass::Invalid || WalkFindRegClass(Scalar2) == RegClass::FPR, "GetOpRegClass(Scalar2) == RegClass::Invalid || WalkFindRegClass(Scalar2) == RegClass::FPR");
#endif
		return _FCmp(ElementSize, Scalar1->Wrapped(ListDataBegin), Scalar2->Wrapped(ListDataBegin));
	}

	/// Does a scalar 'add' between Vector1 and Vector2.
	///
	/// Inserting the result in to the lower element of Vector1 and returning the results.
	///
	/// If ZeroUpperBits is set then in a 256-bit wide operation it will zero the upper 128-bits of the destination.
	///
	/// For 128-bit operation this matches SSE insert semantics.
	///
	/// For 256-bit operation with ZeroUpperBits, this matches AVX insert semantics.
	IRPair<IROp_VFAddScalarInsert> _VFAddScalarInsert(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Vector1, OrderedNodeWrapper Vector2, bool ZeroUpperBits) {
		auto _Op = AllocateOp<IROp_VFAddScalarInsert, IROps::OP_VFADDSCALARINSERT>();
		_Op.first->Vector1 = Vector1;
		_Op.first->Vector2 = Vector2;
		_Op.first->ZeroUpperBits = ZeroUpperBits;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	/// Does a scalar 'add' between Vector1 and Vector2.
	///
	/// Inserting the result in to the lower element of Vector1 and returning the results.
	///
	/// If ZeroUpperBits is set then in a 256-bit wide operation it will zero the upper 128-bits of the destination.
	///
	/// For 128-bit operation this matches SSE insert semantics.
	///
	/// For 256-bit operation with ZeroUpperBits, this matches AVX insert semantics.
	IRPair<IROp_VFAddScalarInsert> _VFAddScalarInsert(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Vector1, OrderedNode *Vector2, bool ZeroUpperBits) {
		auto ListDataBegin = DualListData.ListBegin();
		Vector1->AddUse();
		Vector2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR, "GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR, "GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR");
#endif
		return _VFAddScalarInsert(RegisterSize, ElementSize, Vector1->Wrapped(ListDataBegin), Vector2->Wrapped(ListDataBegin), ZeroUpperBits);
	}

	/// Does a scalar 'sub' between Vector1 and Vector2.
	///
	/// Inserting the result in to the lower element of Vector1 and returning the results.
	///
	/// If ZeroUpperBits is set then in a 256-bit wide operation it will zero the upper 128-bits of the destination.
	///
	/// For 128-bit operation this matches SSE insert semantics.
	///
	/// For 256-bit operation with ZeroUpperBits, this matches AVX insert semantics.
	IRPair<IROp_VFSubScalarInsert> _VFSubScalarInsert(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Vector1, OrderedNodeWrapper Vector2, bool ZeroUpperBits) {
		auto _Op = AllocateOp<IROp_VFSubScalarInsert, IROps::OP_VFSUBSCALARINSERT>();
		_Op.first->Vector1 = Vector1;
		_Op.first->Vector2 = Vector2;
		_Op.first->ZeroUpperBits = ZeroUpperBits;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	/// Does a scalar 'sub' between Vector1 and Vector2.
	///
	/// Inserting the result in to the lower element of Vector1 and returning the results.
	///
	/// If ZeroUpperBits is set then in a 256-bit wide operation it will zero the upper 128-bits of the destination.
	///
	/// For 128-bit operation this matches SSE insert semantics.
	///
	/// For 256-bit operation with ZeroUpperBits, this matches AVX insert semantics.
	IRPair<IROp_VFSubScalarInsert> _VFSubScalarInsert(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Vector1, OrderedNode *Vector2, bool ZeroUpperBits) {
		auto ListDataBegin = DualListData.ListBegin();
		Vector1->AddUse();
		Vector2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR, "GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR, "GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR");
#endif
		return _VFSubScalarInsert(RegisterSize, ElementSize, Vector1->Wrapped(ListDataBegin), Vector2->Wrapped(ListDataBegin), ZeroUpperBits);
	}

	/// Does a scalar 'mul' between Vector1 and Vector2.
	///
	/// Inserting the result in to the lower element of Vector1 and returning the results.
	///
	/// If ZeroUpperBits is set then in a 256-bit wide operation it will zero the upper 128-bits of the destination.
	///
	/// For 128-bit operation this matches SSE insert semantics.
	///
	/// For 256-bit operation with ZeroUpperBits, this matches AVX insert semantics.
	IRPair<IROp_VFMulScalarInsert> _VFMulScalarInsert(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Vector1, OrderedNodeWrapper Vector2, bool ZeroUpperBits) {
		auto _Op = AllocateOp<IROp_VFMulScalarInsert, IROps::OP_VFMULSCALARINSERT>();
		_Op.first->Vector1 = Vector1;
		_Op.first->Vector2 = Vector2;
		_Op.first->ZeroUpperBits = ZeroUpperBits;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	/// Does a scalar 'mul' between Vector1 and Vector2.
	///
	/// Inserting the result in to the lower element of Vector1 and returning the results.
	///
	/// If ZeroUpperBits is set then in a 256-bit wide operation it will zero the upper 128-bits of the destination.
	///
	/// For 128-bit operation this matches SSE insert semantics.
	///
	/// For 256-bit operation with ZeroUpperBits, this matches AVX insert semantics.
	IRPair<IROp_VFMulScalarInsert> _VFMulScalarInsert(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Vector1, OrderedNode *Vector2, bool ZeroUpperBits) {
		auto ListDataBegin = DualListData.ListBegin();
		Vector1->AddUse();
		Vector2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR, "GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR, "GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR");
#endif
		return _VFMulScalarInsert(RegisterSize, ElementSize, Vector1->Wrapped(ListDataBegin), Vector2->Wrapped(ListDataBegin), ZeroUpperBits);
	}

	/// Does a scalar 'div' between Vector1 and Vector2.
	///
	/// Inserting the result in to the lower element of Vector1 and returning the results.
	///
	/// If ZeroUpperBits is set then in a 256-bit wide operation it will zero the upper 128-bits of the destination.
	///
	/// For 128-bit operation this matches SSE insert semantics.
	///
	/// For 256-bit operation with ZeroUpperBits, this matches AVX insert semantics.
	IRPair<IROp_VFDivScalarInsert> _VFDivScalarInsert(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Vector1, OrderedNodeWrapper Vector2, bool ZeroUpperBits) {
		auto _Op = AllocateOp<IROp_VFDivScalarInsert, IROps::OP_VFDIVSCALARINSERT>();
		_Op.first->Vector1 = Vector1;
		_Op.first->Vector2 = Vector2;
		_Op.first->ZeroUpperBits = ZeroUpperBits;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	/// Does a scalar 'div' between Vector1 and Vector2.
	///
	/// Inserting the result in to the lower element of Vector1 and returning the results.
	///
	/// If ZeroUpperBits is set then in a 256-bit wide operation it will zero the upper 128-bits of the destination.
	///
	/// For 128-bit operation this matches SSE insert semantics.
	///
	/// For 256-bit operation with ZeroUpperBits, this matches AVX insert semantics.
	IRPair<IROp_VFDivScalarInsert> _VFDivScalarInsert(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Vector1, OrderedNode *Vector2, bool ZeroUpperBits) {
		auto ListDataBegin = DualListData.ListBegin();
		Vector1->AddUse();
		Vector2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR, "GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR, "GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR");
#endif
		return _VFDivScalarInsert(RegisterSize, ElementSize, Vector1->Wrapped(ListDataBegin), Vector2->Wrapped(ListDataBegin), ZeroUpperBits);
	}

	/// Does a scalar 'min' between Vector1 and Vector2.
	///
	/// Inserting the result in to the lower element of Vector1 and returning the results.
	///
	/// If ZeroUpperBits is set then in a 256-bit wide operation it will zero the upper 128-bits of the destination.
	///
	/// For 128-bit operation this matches SSE insert semantics.
	///
	/// For 256-bit operation with ZeroUpperBits, this matches AVX insert semantics.
	///
	/// Additionally matches x86 zero and NaN semantics
	///
	/// If both source operands are zero, return the second operand (in the case of negative and positive zero)
	///
	/// If either source operand is NaN then return the second operand.
	IRPair<IROp_VFMinScalarInsert> _VFMinScalarInsert(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Vector1, OrderedNodeWrapper Vector2, bool ZeroUpperBits) {
		SaveNZCV(IROps::OP_VFMINSCALARINSERT);		auto _Op = AllocateOp<IROp_VFMinScalarInsert, IROps::OP_VFMINSCALARINSERT>();
		_Op.first->Vector1 = Vector1;
		_Op.first->Vector2 = Vector2;
		_Op.first->ZeroUpperBits = ZeroUpperBits;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	/// Does a scalar 'min' between Vector1 and Vector2.
	///
	/// Inserting the result in to the lower element of Vector1 and returning the results.
	///
	/// If ZeroUpperBits is set then in a 256-bit wide operation it will zero the upper 128-bits of the destination.
	///
	/// For 128-bit operation this matches SSE insert semantics.
	///
	/// For 256-bit operation with ZeroUpperBits, this matches AVX insert semantics.
	///
	/// Additionally matches x86 zero and NaN semantics
	///
	/// If both source operands are zero, return the second operand (in the case of negative and positive zero)
	///
	/// If either source operand is NaN then return the second operand.
	IRPair<IROp_VFMinScalarInsert> _VFMinScalarInsert(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Vector1, OrderedNode *Vector2, bool ZeroUpperBits) {
		auto ListDataBegin = DualListData.ListBegin();
		Vector1->AddUse();
		Vector2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR, "GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR, "GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR");
#endif
		return _VFMinScalarInsert(RegisterSize, ElementSize, Vector1->Wrapped(ListDataBegin), Vector2->Wrapped(ListDataBegin), ZeroUpperBits);
	}

	/// Does a scalar 'max' between Vector1 and Vector2.
	///
	/// Inserting the result in to the lower element of Vector1 and returning the results.
	///
	/// If ZeroUpperBits is set then in a 256-bit wide operation it will zero the upper 128-bits of the destination.
	///
	/// For 128-bit operation this matches SSE insert semantics.
	///
	/// For 256-bit operation with ZeroUpperBits, this matches AVX insert semantics.
	///
	/// Additionally matches x86 zero and NaN semantics
	///
	/// If both source operands are zero, return the second operand (in the case of negative and positive zero)
	///
	/// If either source operand is NaN then return the second operand.
	IRPair<IROp_VFMaxScalarInsert> _VFMaxScalarInsert(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Vector1, OrderedNodeWrapper Vector2, bool ZeroUpperBits) {
		SaveNZCV(IROps::OP_VFMAXSCALARINSERT);		auto _Op = AllocateOp<IROp_VFMaxScalarInsert, IROps::OP_VFMAXSCALARINSERT>();
		_Op.first->Vector1 = Vector1;
		_Op.first->Vector2 = Vector2;
		_Op.first->ZeroUpperBits = ZeroUpperBits;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	/// Does a scalar 'max' between Vector1 and Vector2.
	///
	/// Inserting the result in to the lower element of Vector1 and returning the results.
	///
	/// If ZeroUpperBits is set then in a 256-bit wide operation it will zero the upper 128-bits of the destination.
	///
	/// For 128-bit operation this matches SSE insert semantics.
	///
	/// For 256-bit operation with ZeroUpperBits, this matches AVX insert semantics.
	///
	/// Additionally matches x86 zero and NaN semantics
	///
	/// If both source operands are zero, return the second operand (in the case of negative and positive zero)
	///
	/// If either source operand is NaN then return the second operand.
	IRPair<IROp_VFMaxScalarInsert> _VFMaxScalarInsert(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Vector1, OrderedNode *Vector2, bool ZeroUpperBits) {
		auto ListDataBegin = DualListData.ListBegin();
		Vector1->AddUse();
		Vector2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR, "GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR, "GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR");
#endif
		return _VFMaxScalarInsert(RegisterSize, ElementSize, Vector1->Wrapped(ListDataBegin), Vector2->Wrapped(ListDataBegin), ZeroUpperBits);
	}

	/// Does a scalar 'sqrt' on Vector2, inserting in to Vector1 and storing in to the destination.
	///
	/// Inserting the result in to the lower element of Vector1 and returning the results.
	///
	/// If ZeroUpperBits is set then in a 256-bit wide operation it will zero the upper 128-bits of the destination.
	///
	/// For 128-bit operation this matches SSE insert semantics.
	///
	/// For 256-bit operation with ZeroUpperBits, this matches AVX insert semantics.
	IRPair<IROp_VFSqrtScalarInsert> _VFSqrtScalarInsert(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Vector1, OrderedNodeWrapper Vector2, bool ZeroUpperBits) {
		auto _Op = AllocateOp<IROp_VFSqrtScalarInsert, IROps::OP_VFSQRTSCALARINSERT>();
		_Op.first->Vector1 = Vector1;
		_Op.first->Vector2 = Vector2;
		_Op.first->ZeroUpperBits = ZeroUpperBits;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	/// Does a scalar 'sqrt' on Vector2, inserting in to Vector1 and storing in to the destination.
	///
	/// Inserting the result in to the lower element of Vector1 and returning the results.
	///
	/// If ZeroUpperBits is set then in a 256-bit wide operation it will zero the upper 128-bits of the destination.
	///
	/// For 128-bit operation this matches SSE insert semantics.
	///
	/// For 256-bit operation with ZeroUpperBits, this matches AVX insert semantics.
	IRPair<IROp_VFSqrtScalarInsert> _VFSqrtScalarInsert(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Vector1, OrderedNode *Vector2, bool ZeroUpperBits) {
		auto ListDataBegin = DualListData.ListBegin();
		Vector1->AddUse();
		Vector2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR, "GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR, "GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR");
#endif
		return _VFSqrtScalarInsert(RegisterSize, ElementSize, Vector1->Wrapped(ListDataBegin), Vector2->Wrapped(ListDataBegin), ZeroUpperBits);
	}

	/// Does a scalar 'rsqrt' on Vector2, inserting in to Vector1 and storing in to the destination.
	///
	/// Inserting the result in to the lower element of Vector1 and returning the results.
	///
	/// If ZeroUpperBits is set then in a 256-bit wide operation it will zero the upper 128-bits of the destination.
	///
	/// For 128-bit operation this matches SSE insert semantics.
	///
	/// For 256-bit operation with ZeroUpperBits, this matches AVX insert semantics.
	IRPair<IROp_VFRSqrtScalarInsert> _VFRSqrtScalarInsert(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Vector1, OrderedNodeWrapper Vector2, bool ZeroUpperBits) {
		auto _Op = AllocateOp<IROp_VFRSqrtScalarInsert, IROps::OP_VFRSQRTSCALARINSERT>();
		_Op.first->Vector1 = Vector1;
		_Op.first->Vector2 = Vector2;
		_Op.first->ZeroUpperBits = ZeroUpperBits;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	/// Does a scalar 'rsqrt' on Vector2, inserting in to Vector1 and storing in to the destination.
	///
	/// Inserting the result in to the lower element of Vector1 and returning the results.
	///
	/// If ZeroUpperBits is set then in a 256-bit wide operation it will zero the upper 128-bits of the destination.
	///
	/// For 128-bit operation this matches SSE insert semantics.
	///
	/// For 256-bit operation with ZeroUpperBits, this matches AVX insert semantics.
	IRPair<IROp_VFRSqrtScalarInsert> _VFRSqrtScalarInsert(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Vector1, OrderedNode *Vector2, bool ZeroUpperBits) {
		auto ListDataBegin = DualListData.ListBegin();
		Vector1->AddUse();
		Vector2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR, "GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR, "GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR");
#endif
		return _VFRSqrtScalarInsert(RegisterSize, ElementSize, Vector1->Wrapped(ListDataBegin), Vector2->Wrapped(ListDataBegin), ZeroUpperBits);
	}

	/// Does a scalar 'recip' on Vector2, inserting in to Vector1 and storing in to the destination.
	///
	/// Inserting the result in to the lower element of Vector1 and returning the results.
	///
	/// If ZeroUpperBits is set then in a 256-bit wide operation it will zero the upper 128-bits of the destination.
	///
	/// For 128-bit operation this matches SSE insert semantics.
	///
	/// For 256-bit operation with ZeroUpperBits, this matches AVX insert semantics.
	IRPair<IROp_VFRecpScalarInsert> _VFRecpScalarInsert(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Vector1, OrderedNodeWrapper Vector2, bool ZeroUpperBits) {
		auto _Op = AllocateOp<IROp_VFRecpScalarInsert, IROps::OP_VFRECPSCALARINSERT>();
		_Op.first->Vector1 = Vector1;
		_Op.first->Vector2 = Vector2;
		_Op.first->ZeroUpperBits = ZeroUpperBits;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	/// Does a scalar 'recip' on Vector2, inserting in to Vector1 and storing in to the destination.
	///
	/// Inserting the result in to the lower element of Vector1 and returning the results.
	///
	/// If ZeroUpperBits is set then in a 256-bit wide operation it will zero the upper 128-bits of the destination.
	///
	/// For 128-bit operation this matches SSE insert semantics.
	///
	/// For 256-bit operation with ZeroUpperBits, this matches AVX insert semantics.
	IRPair<IROp_VFRecpScalarInsert> _VFRecpScalarInsert(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Vector1, OrderedNode *Vector2, bool ZeroUpperBits) {
		auto ListDataBegin = DualListData.ListBegin();
		Vector1->AddUse();
		Vector2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR, "GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR, "GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR");
#endif
		return _VFRecpScalarInsert(RegisterSize, ElementSize, Vector1->Wrapped(ListDataBegin), Vector2->Wrapped(ListDataBegin), ZeroUpperBits);
	}

	/// Does a scalar 'cvt' between Vector1 and Vector2.
	///
	/// Inserting the result in to the lower element of Vector1 and returning the results.
	///
	/// If ZeroUpperBits is set then in a 256-bit wide operation it will zero the upper 128-bits of the destination.
	///
	/// For 128-bit operation this matches SSE insert semantics.
	///
	/// For 256-bit operation with ZeroUpperBits, this matches AVX insert semantics.
	IRPair<IROp_VFToFScalarInsert> _VFToFScalarInsert(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize DstElementSize, FEXCore::IR::OpSize SrcElementSize, OrderedNodeWrapper Vector1, OrderedNodeWrapper Vector2, bool ZeroUpperBits) {
		auto _Op = AllocateOp<IROp_VFToFScalarInsert, IROps::OP_VFTOFSCALARINSERT>();
		_Op.first->Vector1 = Vector1;
		_Op.first->Vector2 = Vector2;
		_Op.first->SrcElementSize = SrcElementSize;
		_Op.first->ZeroUpperBits = ZeroUpperBits;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = DstElementSize;
		return _Op;
	}

	/// Does a scalar 'cvt' between Vector1 and Vector2.
	///
	/// Inserting the result in to the lower element of Vector1 and returning the results.
	///
	/// If ZeroUpperBits is set then in a 256-bit wide operation it will zero the upper 128-bits of the destination.
	///
	/// For 128-bit operation this matches SSE insert semantics.
	///
	/// For 256-bit operation with ZeroUpperBits, this matches AVX insert semantics.
	IRPair<IROp_VFToFScalarInsert> _VFToFScalarInsert(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize DstElementSize, FEXCore::IR::OpSize SrcElementSize, OrderedNode *Vector1, OrderedNode *Vector2, bool ZeroUpperBits) {
		auto ListDataBegin = DualListData.ListBegin();
		Vector1->AddUse();
		Vector2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR, "GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR, "GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR");
#endif
		return _VFToFScalarInsert(RegisterSize, DstElementSize, SrcElementSize, Vector1->Wrapped(ListDataBegin), Vector2->Wrapped(ListDataBegin), ZeroUpperBits);
	}

	/// Does a Vector 'scvt' between Vector1 and Vector2.
	///
	/// Inserting the result in to the lower element of Vector1 and returning the results.
	///
	/// If ZeroUpperBits is set then in a 256-bit wide operation it will zero the upper 128-bits of the destination.
	///
	/// For 128-bit operation this matches SSE insert semantics.
	///
	/// For 256-bit operation with ZeroUpperBits, this matches AVX insert semantics.
	///
	/// HasTwoElements is slightly different than most of these scalar operations.
	///
	/// Handles the edge case of cvtpi2ps xmm0, mm0 which is two elements in the lower 64-bits
	IRPair<IROp_VSToFVectorInsert> _VSToFVectorInsert(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize DstElementSize, FEXCore::IR::OpSize SrcElementSize, OrderedNodeWrapper Vector1, OrderedNodeWrapper Vector2, int8_t HasTwoElements, bool ZeroUpperBits) {
		auto _Op = AllocateOp<IROp_VSToFVectorInsert, IROps::OP_VSTOFVECTORINSERT>();
		_Op.first->Vector1 = Vector1;
		_Op.first->Vector2 = Vector2;
		_Op.first->SrcElementSize = SrcElementSize;
		_Op.first->HasTwoElements = HasTwoElements;
		_Op.first->ZeroUpperBits = ZeroUpperBits;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = DstElementSize;
		return _Op;
	}

	/// Does a Vector 'scvt' between Vector1 and Vector2.
	///
	/// Inserting the result in to the lower element of Vector1 and returning the results.
	///
	/// If ZeroUpperBits is set then in a 256-bit wide operation it will zero the upper 128-bits of the destination.
	///
	/// For 128-bit operation this matches SSE insert semantics.
	///
	/// For 256-bit operation with ZeroUpperBits, this matches AVX insert semantics.
	///
	/// HasTwoElements is slightly different than most of these scalar operations.
	///
	/// Handles the edge case of cvtpi2ps xmm0, mm0 which is two elements in the lower 64-bits
	IRPair<IROp_VSToFVectorInsert> _VSToFVectorInsert(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize DstElementSize, FEXCore::IR::OpSize SrcElementSize, OrderedNode *Vector1, OrderedNode *Vector2, int8_t HasTwoElements, bool ZeroUpperBits) {
		auto ListDataBegin = DualListData.ListBegin();
		Vector1->AddUse();
		Vector2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR, "GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR, "GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR");
#endif
		return _VSToFVectorInsert(RegisterSize, DstElementSize, SrcElementSize, Vector1->Wrapped(ListDataBegin), Vector2->Wrapped(ListDataBegin), HasTwoElements, ZeroUpperBits);
	}

	/// Does a scalar 'cvt' between Vector1 and GPR.
	///
	/// Inserting the result in to the lower element of Vector1 and returning the results.
	///
	/// If ZeroUpperBits is set then in a 256-bit wide operation it will zero the upper 128-bits of the destination.
	///
	/// For 128-bit operation this matches SSE insert semantics.
	///
	/// For 256-bit operation with ZeroUpperBits, this matches AVX insert semantics.
	IRPair<IROp_VSToFGPRInsert> _VSToFGPRInsert(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize DstElementSize, FEXCore::IR::OpSize SrcElementSize, OrderedNodeWrapper Vector, OrderedNodeWrapper Src, bool ZeroUpperBits) {
		auto _Op = AllocateOp<IROp_VSToFGPRInsert, IROps::OP_VSTOFGPRINSERT>();
		_Op.first->Vector = Vector;
		_Op.first->Src = Src;
		_Op.first->SrcElementSize = SrcElementSize;
		_Op.first->ZeroUpperBits = ZeroUpperBits;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = DstElementSize;
		return _Op;
	}

	/// Does a scalar 'cvt' between Vector1 and GPR.
	///
	/// Inserting the result in to the lower element of Vector1 and returning the results.
	///
	/// If ZeroUpperBits is set then in a 256-bit wide operation it will zero the upper 128-bits of the destination.
	///
	/// For 128-bit operation this matches SSE insert semantics.
	///
	/// For 256-bit operation with ZeroUpperBits, this matches AVX insert semantics.
	IRPair<IROp_VSToFGPRInsert> _VSToFGPRInsert(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize DstElementSize, FEXCore::IR::OpSize SrcElementSize, OrderedNode *Vector, OrderedNode *Src, bool ZeroUpperBits) {
		auto ListDataBegin = DualListData.ListBegin();
		Vector->AddUse();
		Src->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector) == RegClass::Invalid || WalkFindRegClass(Vector) == RegClass::FPR, "GetOpRegClass(Vector) == RegClass::Invalid || WalkFindRegClass(Vector) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src) == RegClass::Invalid || WalkFindRegClass(Src) == RegClass::GPR, "GetOpRegClass(Src) == RegClass::Invalid || WalkFindRegClass(Src) == RegClass::GPR");
#endif
		return _VSToFGPRInsert(RegisterSize, DstElementSize, SrcElementSize, Vector->Wrapped(ListDataBegin), Src->Wrapped(ListDataBegin), ZeroUpperBits);
	}

	/// Does a scalar round float to integral on Vector2, inserting in to Vector1 and storing in to the destination.
	///
	/// Rounding mode determined by argument
	///
	/// Inserting the result in to the lower element of Vector1 and returning the results.
	///
	/// If ZeroUpperBits is set then in a 256-bit wide operation it will zero the upper 128-bits of the destination.
	///
	/// For 128-bit operation this matches SSE insert semantics.
	///
	/// For 256-bit operation with ZeroUpperBits, this matches AVX insert semantics.
	IRPair<IROp_VFToIScalarInsert> _VFToIScalarInsert(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Vector1, OrderedNodeWrapper Vector2, RoundMode Round, bool ZeroUpperBits) {
		auto _Op = AllocateOp<IROp_VFToIScalarInsert, IROps::OP_VFTOISCALARINSERT>();
		_Op.first->Vector1 = Vector1;
		_Op.first->Vector2 = Vector2;
		_Op.first->Round = Round;
		_Op.first->ZeroUpperBits = ZeroUpperBits;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	/// Does a scalar round float to integral on Vector2, inserting in to Vector1 and storing in to the destination.
	///
	/// Rounding mode determined by argument
	///
	/// Inserting the result in to the lower element of Vector1 and returning the results.
	///
	/// If ZeroUpperBits is set then in a 256-bit wide operation it will zero the upper 128-bits of the destination.
	///
	/// For 128-bit operation this matches SSE insert semantics.
	///
	/// For 256-bit operation with ZeroUpperBits, this matches AVX insert semantics.
	IRPair<IROp_VFToIScalarInsert> _VFToIScalarInsert(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Vector1, OrderedNode *Vector2, RoundMode Round, bool ZeroUpperBits) {
		auto ListDataBegin = DualListData.ListBegin();
		Vector1->AddUse();
		Vector2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR, "GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR, "GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR");
#endif
		return _VFToIScalarInsert(RegisterSize, ElementSize, Vector1->Wrapped(ListDataBegin), Vector2->Wrapped(ListDataBegin), Round, ZeroUpperBits);
	}

	/// Does a scalar 'cmp' between Vector1 and Vecto2, inserting in to Vector1 and storing in to the destination.
	///
	/// Compare op determined by argument
	///
	/// Inserting the result in to the lower element of Vector1 and returning the results.
	///
	/// If ZeroUpperBits is set then in a 256-bit wide operation it will zero the upper 128-bits of the destination.
	///
	/// For 128-bit operation this matches SSE insert semantics.
	///
	/// For 256-bit operation with ZeroUpperBits, this matches AVX insert semantics.
	IRPair<IROp_VFCMPScalarInsert> _VFCMPScalarInsert(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Vector1, OrderedNodeWrapper Vector2, FloatCompareOp Op, bool ZeroUpperBits) {
		auto _Op = AllocateOp<IROp_VFCMPScalarInsert, IROps::OP_VFCMPSCALARINSERT>();
		_Op.first->Vector1 = Vector1;
		_Op.first->Vector2 = Vector2;
		_Op.first->Op = Op;
		_Op.first->ZeroUpperBits = ZeroUpperBits;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	/// Does a scalar 'cmp' between Vector1 and Vecto2, inserting in to Vector1 and storing in to the destination.
	///
	/// Compare op determined by argument
	///
	/// Inserting the result in to the lower element of Vector1 and returning the results.
	///
	/// If ZeroUpperBits is set then in a 256-bit wide operation it will zero the upper 128-bits of the destination.
	///
	/// For 128-bit operation this matches SSE insert semantics.
	///
	/// For 256-bit operation with ZeroUpperBits, this matches AVX insert semantics.
	IRPair<IROp_VFCMPScalarInsert> _VFCMPScalarInsert(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Vector1, OrderedNode *Vector2, FloatCompareOp Op, bool ZeroUpperBits) {
		auto ListDataBegin = DualListData.ListBegin();
		Vector1->AddUse();
		Vector2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR, "GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR, "GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR");
#endif
		return _VFCMPScalarInsert(RegisterSize, ElementSize, Vector1->Wrapped(ListDataBegin), Vector2->Wrapped(ListDataBegin), Op, ZeroUpperBits);
	}

	/// Dest = (Vector1 * Vector2) + Addend
	///
	/// This explicitly matches x86 FMA semantics because ARM semantics are mind-bending.
	///
	/// Upper elements copied from Upper
	IRPair<IROp_VFMLAScalarInsert> _VFMLAScalarInsert(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Upper, OrderedNodeWrapper Vector1, OrderedNodeWrapper Vector2, OrderedNodeWrapper Addend) {
		auto _Op = AllocateOp<IROp_VFMLAScalarInsert, IROps::OP_VFMLASCALARINSERT>();
		_Op.first->Upper = Upper;
		_Op.first->Vector1 = Vector1;
		_Op.first->Vector2 = Vector2;
		_Op.first->Addend = Addend;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	/// Dest = (Vector1 * Vector2) + Addend
	///
	/// This explicitly matches x86 FMA semantics because ARM semantics are mind-bending.
	///
	/// Upper elements copied from Upper
	IRPair<IROp_VFMLAScalarInsert> _VFMLAScalarInsert(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Upper, OrderedNode *Vector1, OrderedNode *Vector2, OrderedNode *Addend) {
		auto ListDataBegin = DualListData.ListBegin();
		Upper->AddUse();
		Vector1->AddUse();
		Vector2->AddUse();
		Addend->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Upper) == RegClass::Invalid || WalkFindRegClass(Upper) == RegClass::FPR, "GetOpRegClass(Upper) == RegClass::Invalid || WalkFindRegClass(Upper) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR, "GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR, "GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Addend) == RegClass::Invalid || WalkFindRegClass(Addend) == RegClass::FPR, "GetOpRegClass(Addend) == RegClass::Invalid || WalkFindRegClass(Addend) == RegClass::FPR");
#endif
		return _VFMLAScalarInsert(RegisterSize, ElementSize, Upper->Wrapped(ListDataBegin), Vector1->Wrapped(ListDataBegin), Vector2->Wrapped(ListDataBegin), Addend->Wrapped(ListDataBegin));
	}

	/// Dest = (Vector1 * Vector2) - Addend
	///
	/// This explicitly matches x86 FMA semantics because ARM semantics are mind-bending.
	///
	/// Upper elements copied from Upper
	IRPair<IROp_VFMLSScalarInsert> _VFMLSScalarInsert(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Upper, OrderedNodeWrapper Vector1, OrderedNodeWrapper Vector2, OrderedNodeWrapper Addend) {
		auto _Op = AllocateOp<IROp_VFMLSScalarInsert, IROps::OP_VFMLSSCALARINSERT>();
		_Op.first->Upper = Upper;
		_Op.first->Vector1 = Vector1;
		_Op.first->Vector2 = Vector2;
		_Op.first->Addend = Addend;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	/// Dest = (Vector1 * Vector2) - Addend
	///
	/// This explicitly matches x86 FMA semantics because ARM semantics are mind-bending.
	///
	/// Upper elements copied from Upper
	IRPair<IROp_VFMLSScalarInsert> _VFMLSScalarInsert(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Upper, OrderedNode *Vector1, OrderedNode *Vector2, OrderedNode *Addend) {
		auto ListDataBegin = DualListData.ListBegin();
		Upper->AddUse();
		Vector1->AddUse();
		Vector2->AddUse();
		Addend->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Upper) == RegClass::Invalid || WalkFindRegClass(Upper) == RegClass::FPR, "GetOpRegClass(Upper) == RegClass::Invalid || WalkFindRegClass(Upper) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR, "GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR, "GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Addend) == RegClass::Invalid || WalkFindRegClass(Addend) == RegClass::FPR, "GetOpRegClass(Addend) == RegClass::Invalid || WalkFindRegClass(Addend) == RegClass::FPR");
#endif
		return _VFMLSScalarInsert(RegisterSize, ElementSize, Upper->Wrapped(ListDataBegin), Vector1->Wrapped(ListDataBegin), Vector2->Wrapped(ListDataBegin), Addend->Wrapped(ListDataBegin));
	}

	/// Dest = (-Vector1 * Vector2) + Addend
	///
	/// This explicitly matches x86 FMA semantics because ARM semantics are mind-bending.
	///
	/// Upper elements copied from Upper
	IRPair<IROp_VFNMLAScalarInsert> _VFNMLAScalarInsert(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Upper, OrderedNodeWrapper Vector1, OrderedNodeWrapper Vector2, OrderedNodeWrapper Addend) {
		auto _Op = AllocateOp<IROp_VFNMLAScalarInsert, IROps::OP_VFNMLASCALARINSERT>();
		_Op.first->Upper = Upper;
		_Op.first->Vector1 = Vector1;
		_Op.first->Vector2 = Vector2;
		_Op.first->Addend = Addend;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	/// Dest = (-Vector1 * Vector2) + Addend
	///
	/// This explicitly matches x86 FMA semantics because ARM semantics are mind-bending.
	///
	/// Upper elements copied from Upper
	IRPair<IROp_VFNMLAScalarInsert> _VFNMLAScalarInsert(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Upper, OrderedNode *Vector1, OrderedNode *Vector2, OrderedNode *Addend) {
		auto ListDataBegin = DualListData.ListBegin();
		Upper->AddUse();
		Vector1->AddUse();
		Vector2->AddUse();
		Addend->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Upper) == RegClass::Invalid || WalkFindRegClass(Upper) == RegClass::FPR, "GetOpRegClass(Upper) == RegClass::Invalid || WalkFindRegClass(Upper) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR, "GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR, "GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Addend) == RegClass::Invalid || WalkFindRegClass(Addend) == RegClass::FPR, "GetOpRegClass(Addend) == RegClass::Invalid || WalkFindRegClass(Addend) == RegClass::FPR");
#endif
		return _VFNMLAScalarInsert(RegisterSize, ElementSize, Upper->Wrapped(ListDataBegin), Vector1->Wrapped(ListDataBegin), Vector2->Wrapped(ListDataBegin), Addend->Wrapped(ListDataBegin));
	}

	/// Dest = (-Vector1 * Vector2) - Addend
	///
	/// This explicitly matches x86 FMA semantics because ARM semantics are mind-bending.
	///
	/// Upper elements copied from Upper
	IRPair<IROp_VFNMLSScalarInsert> _VFNMLSScalarInsert(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Upper, OrderedNodeWrapper Vector1, OrderedNodeWrapper Vector2, OrderedNodeWrapper Addend) {
		auto _Op = AllocateOp<IROp_VFNMLSScalarInsert, IROps::OP_VFNMLSSCALARINSERT>();
		_Op.first->Upper = Upper;
		_Op.first->Vector1 = Vector1;
		_Op.first->Vector2 = Vector2;
		_Op.first->Addend = Addend;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	/// Dest = (-Vector1 * Vector2) - Addend
	///
	/// This explicitly matches x86 FMA semantics because ARM semantics are mind-bending.
	///
	/// Upper elements copied from Upper
	IRPair<IROp_VFNMLSScalarInsert> _VFNMLSScalarInsert(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Upper, OrderedNode *Vector1, OrderedNode *Vector2, OrderedNode *Addend) {
		auto ListDataBegin = DualListData.ListBegin();
		Upper->AddUse();
		Vector1->AddUse();
		Vector2->AddUse();
		Addend->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Upper) == RegClass::Invalid || WalkFindRegClass(Upper) == RegClass::FPR, "GetOpRegClass(Upper) == RegClass::Invalid || WalkFindRegClass(Upper) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR, "GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR, "GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Addend) == RegClass::Invalid || WalkFindRegClass(Addend) == RegClass::FPR, "GetOpRegClass(Addend) == RegClass::Invalid || WalkFindRegClass(Addend) == RegClass::FPR");
#endif
		return _VFNMLSScalarInsert(RegisterSize, ElementSize, Upper->Wrapped(ListDataBegin), Vector1->Wrapped(ListDataBegin), Vector2->Wrapped(ListDataBegin), Addend->Wrapped(ListDataBegin));
	}

	/// Returns a vector where each element has has the magniture of each corresponding element in vector1 and the sign of vector 2.
	IRPair<IROp_VFCopySign> _VFCopySign(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Vector1, OrderedNodeWrapper Vector2) {
		auto _Op = AllocateOp<IROp_VFCopySign, IROps::OP_VFCOPYSIGN>();
		_Op.first->Vector1 = Vector1;
		_Op.first->Vector2 = Vector2;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	/// Returns a vector where each element has has the magniture of each corresponding element in vector1 and the sign of vector 2.
	IRPair<IROp_VFCopySign> _VFCopySign(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Vector1, OrderedNode *Vector2) {
		auto ListDataBegin = DualListData.ListBegin();
		Vector1->AddUse();
		Vector2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR, "GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR, "GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR");
#endif
		return _VFCopySign(RegisterSize, ElementSize, Vector1->Wrapped(ListDataBegin), Vector2->Wrapped(ListDataBegin));
	}

	/// Copy vector register
	///
	/// When Register size is smaller than Source register size,
	///
	/// this op is defined to truncate and zero extend
	IRPair<IROp_VMov> _VMov(FEXCore::IR::OpSize RegisterSize, OrderedNodeWrapper Source) {
		auto _Op = AllocateOp<IROp_VMov, IROps::OP_VMOV>();
		_Op.first->Source = Source;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Copy vector register
	///
	/// When Register size is smaller than Source register size,
	///
	/// this op is defined to truncate and zero extend
	IRPair<IROp_VMov> _VMov(FEXCore::IR::OpSize RegisterSize, OrderedNode *Source) {
		auto ListDataBegin = DualListData.ListBegin();
		Source->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Source) == RegClass::Invalid || WalkFindRegClass(Source) == RegClass::FPR, "GetOpRegClass(Source) == RegClass::Invalid || WalkFindRegClass(Source) == RegClass::FPR");
#endif
		return _VMov(RegisterSize, Source->Wrapped(ListDataBegin));
	}

	/// Generates a vector with each element containg the immediate zexted
	IRPair<IROp_VectorImm> _VectorImm(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, uint8_t Immediate, uint8_t ShiftAmount = 0) {
		auto _Op = AllocateOp<IROp_VectorImm, IROps::OP_VECTORIMM>();
		_Op.first->Immediate = Immediate;
		_Op.first->ShiftAmount = ShiftAmount;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	/// Load a named vector constant.
	///
	/// The list of vector constants can be found in <FEXCore/IR/IR.h>
	IRPair<IROp_LoadNamedVectorConstant> _LoadNamedVectorConstant(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::NamedVectorConstant Constant) {
		auto _Op = AllocateOp<IROp_LoadNamedVectorConstant, IROps::OP_LOADNAMEDVECTORCONSTANT>();
		_Op.first->Constant = Constant;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Load a named vector constant from Indexable table.
	///
	/// Index needs to be aligned register size.
	///
	/// The list of indexable vector constants can be found in <FEXCore/IR/IR.h>
	IRPair<IROp_LoadNamedVectorIndexedConstant> _LoadNamedVectorIndexedConstant(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::IndexNamedVectorConstant Constant, uint32_t Index) {
		auto _Op = AllocateOp<IROp_LoadNamedVectorIndexedConstant, IROps::OP_LOADNAMEDVECTORINDEXEDCONSTANT>();
		_Op.first->Constant = Constant;
		_Op.first->Index = Index;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	IRPair<IROp_VNeg> _VNeg(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Vector) {
		auto _Op = AllocateOp<IROp_VNeg, IROps::OP_VNEG>();
		_Op.first->Vector = Vector;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	IRPair<IROp_VNeg> _VNeg(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Vector) {
		auto ListDataBegin = DualListData.ListBegin();
		Vector->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector) == RegClass::Invalid || WalkFindRegClass(Vector) == RegClass::FPR, "GetOpRegClass(Vector) == RegClass::Invalid || WalkFindRegClass(Vector) == RegClass::FPR");
#endif
		return _VNeg(RegisterSize, ElementSize, Vector->Wrapped(ListDataBegin));
	}

	IRPair<IROp_VNot> _VNot(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Vector) {
		auto _Op = AllocateOp<IROp_VNot, IROps::OP_VNOT>();
		_Op.first->Vector = Vector;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	IRPair<IROp_VNot> _VNot(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Vector) {
		auto ListDataBegin = DualListData.ListBegin();
		Vector->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector) == RegClass::Invalid || WalkFindRegClass(Vector) == RegClass::FPR, "GetOpRegClass(Vector) == RegClass::Invalid || WalkFindRegClass(Vector) == RegClass::FPR");
#endif
		return _VNot(RegisterSize, ElementSize, Vector->Wrapped(ListDataBegin));
	}

	/// Does an signed integer absolute
	IRPair<IROp_VAbs> _VAbs(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Vector) {
		auto _Op = AllocateOp<IROp_VAbs, IROps::OP_VABS>();
		_Op.first->Vector = Vector;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	/// Does an signed integer absolute
	IRPair<IROp_VAbs> _VAbs(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Vector) {
		auto ListDataBegin = DualListData.ListBegin();
		Vector->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector) == RegClass::Invalid || WalkFindRegClass(Vector) == RegClass::FPR, "GetOpRegClass(Vector) == RegClass::Invalid || WalkFindRegClass(Vector) == RegClass::FPR");
#endif
		return _VAbs(RegisterSize, ElementSize, Vector->Wrapped(ListDataBegin));
	}

	/// Does a popcount for each element of the register
	IRPair<IROp_VPopcount> _VPopcount(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Vector) {
		auto _Op = AllocateOp<IROp_VPopcount, IROps::OP_VPOPCOUNT>();
		_Op.first->Vector = Vector;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	/// Does a popcount for each element of the register
	IRPair<IROp_VPopcount> _VPopcount(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Vector) {
		auto ListDataBegin = DualListData.ListBegin();
		Vector->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector) == RegClass::Invalid || WalkFindRegClass(Vector) == RegClass::FPR, "GetOpRegClass(Vector) == RegClass::Invalid || WalkFindRegClass(Vector) == RegClass::FPR");
#endif
		return _VPopcount(RegisterSize, ElementSize, Vector->Wrapped(ListDataBegin));
	}

	/// Does a horizontal vector add of elements across the source vector
	///
	/// Result is a zero extended scalar
	IRPair<IROp_VAddV> _VAddV(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Vector) {
		auto _Op = AllocateOp<IROp_VAddV, IROps::OP_VADDV>();
		_Op.first->Vector = Vector;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	/// Does a horizontal vector add of elements across the source vector
	///
	/// Result is a zero extended scalar
	IRPair<IROp_VAddV> _VAddV(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Vector) {
		auto ListDataBegin = DualListData.ListBegin();
		Vector->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector) == RegClass::Invalid || WalkFindRegClass(Vector) == RegClass::FPR, "GetOpRegClass(Vector) == RegClass::Invalid || WalkFindRegClass(Vector) == RegClass::FPR");
#endif
		return _VAddV(RegisterSize, ElementSize, Vector->Wrapped(ListDataBegin));
	}

	/// Does a horizontal vector unsigned minimum of elements across the source vector
	///
	/// Result is a zero extended scalar
	IRPair<IROp_VUMinV> _VUMinV(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Vector) {
		auto _Op = AllocateOp<IROp_VUMinV, IROps::OP_VUMINV>();
		_Op.first->Vector = Vector;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	/// Does a horizontal vector unsigned minimum of elements across the source vector
	///
	/// Result is a zero extended scalar
	IRPair<IROp_VUMinV> _VUMinV(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Vector) {
		auto ListDataBegin = DualListData.ListBegin();
		Vector->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector) == RegClass::Invalid || WalkFindRegClass(Vector) == RegClass::FPR, "GetOpRegClass(Vector) == RegClass::Invalid || WalkFindRegClass(Vector) == RegClass::FPR");
#endif
		return _VUMinV(RegisterSize, ElementSize, Vector->Wrapped(ListDataBegin));
	}

	/// Does a horizontal vector unsigned maximum of elements across the source vector
	///
	/// Result is a zero extended scalar
	IRPair<IROp_VUMaxV> _VUMaxV(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Vector) {
		auto _Op = AllocateOp<IROp_VUMaxV, IROps::OP_VUMAXV>();
		_Op.first->Vector = Vector;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	/// Does a horizontal vector unsigned maximum of elements across the source vector
	///
	/// Result is a zero extended scalar
	IRPair<IROp_VUMaxV> _VUMaxV(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Vector) {
		auto ListDataBegin = DualListData.ListBegin();
		Vector->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector) == RegClass::Invalid || WalkFindRegClass(Vector) == RegClass::FPR, "GetOpRegClass(Vector) == RegClass::Invalid || WalkFindRegClass(Vector) == RegClass::FPR");
#endif
		return _VUMaxV(RegisterSize, ElementSize, Vector->Wrapped(ListDataBegin));
	}

	IRPair<IROp_VFAbs> _VFAbs(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Vector) {
		auto _Op = AllocateOp<IROp_VFAbs, IROps::OP_VFABS>();
		_Op.first->Vector = Vector;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	IRPair<IROp_VFAbs> _VFAbs(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Vector) {
		auto ListDataBegin = DualListData.ListBegin();
		Vector->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector) == RegClass::Invalid || WalkFindRegClass(Vector) == RegClass::FPR, "GetOpRegClass(Vector) == RegClass::Invalid || WalkFindRegClass(Vector) == RegClass::FPR");
#endif
		return _VFAbs(RegisterSize, ElementSize, Vector->Wrapped(ListDataBegin));
	}

	IRPair<IROp_VFNeg> _VFNeg(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Vector) {
		auto _Op = AllocateOp<IROp_VFNeg, IROps::OP_VFNEG>();
		_Op.first->Vector = Vector;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	IRPair<IROp_VFNeg> _VFNeg(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Vector) {
		auto ListDataBegin = DualListData.ListBegin();
		Vector->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector) == RegClass::Invalid || WalkFindRegClass(Vector) == RegClass::FPR, "GetOpRegClass(Vector) == RegClass::Invalid || WalkFindRegClass(Vector) == RegClass::FPR");
#endif
		return _VFNeg(RegisterSize, ElementSize, Vector->Wrapped(ListDataBegin));
	}

	/// Reciprocal value - matches the precision required by the x86 spec.
	///
	/// It has a relative error of at most 1.5 * 2^-12
	IRPair<IROp_VFRecp> _VFRecp(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Vector) {
		auto _Op = AllocateOp<IROp_VFRecp, IROps::OP_VFRECP>();
		_Op.first->Vector = Vector;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	/// Reciprocal value - matches the precision required by the x86 spec.
	///
	/// It has a relative error of at most 1.5 * 2^-12
	IRPair<IROp_VFRecp> _VFRecp(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Vector) {
		auto ListDataBegin = DualListData.ListBegin();
		Vector->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector) == RegClass::Invalid || WalkFindRegClass(Vector) == RegClass::FPR, "GetOpRegClass(Vector) == RegClass::Invalid || WalkFindRegClass(Vector) == RegClass::FPR");
#endif
		return _VFRecp(RegisterSize, ElementSize, Vector->Wrapped(ListDataBegin));
	}

	/// Similar to VFRecp but carrying more precision for 3DNow!
	///
	/// It provides at least 14 bits precision, with a relative error of at most 2^-14
	IRPair<IROp_VFRecpPrecision> _VFRecpPrecision(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Vector) {
		auto _Op = AllocateOp<IROp_VFRecpPrecision, IROps::OP_VFRECPPRECISION>();
		_Op.first->Vector = Vector;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	/// Similar to VFRecp but carrying more precision for 3DNow!
	///
	/// It provides at least 14 bits precision, with a relative error of at most 2^-14
	IRPair<IROp_VFRecpPrecision> _VFRecpPrecision(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Vector) {
		auto ListDataBegin = DualListData.ListBegin();
		Vector->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector) == RegClass::Invalid || WalkFindRegClass(Vector) == RegClass::FPR, "GetOpRegClass(Vector) == RegClass::Invalid || WalkFindRegClass(Vector) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(RegisterSize == FEXCore::IR::OpSize::i64Bit || RegisterSize == FEXCore::IR::OpSize::i32Bit, "RegisterSize == FEXCore::IR::OpSize::i64Bit || RegisterSize == FEXCore::IR::OpSize::i32Bit");
		LOGMAN_THROW_A_FMT(ElementSize == FEXCore::IR::OpSize::i32Bit, "ElementSize == FEXCore::IR::OpSize::i32Bit");
#endif
		return _VFRecpPrecision(RegisterSize, ElementSize, Vector->Wrapped(ListDataBegin));
	}

	IRPair<IROp_VFSqrt> _VFSqrt(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Vector) {
		auto _Op = AllocateOp<IROp_VFSqrt, IROps::OP_VFSQRT>();
		_Op.first->Vector = Vector;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	IRPair<IROp_VFSqrt> _VFSqrt(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Vector) {
		auto ListDataBegin = DualListData.ListBegin();
		Vector->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector) == RegClass::Invalid || WalkFindRegClass(Vector) == RegClass::FPR, "GetOpRegClass(Vector) == RegClass::Invalid || WalkFindRegClass(Vector) == RegClass::FPR");
#endif
		return _VFSqrt(RegisterSize, ElementSize, Vector->Wrapped(ListDataBegin));
	}

	/// Reciprocal Square Root - matches the precision required by the x86 spec.
	///
	/// It has a relative error of at most 1.5 * 2^-12
	IRPair<IROp_VFRSqrt> _VFRSqrt(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Vector) {
		auto _Op = AllocateOp<IROp_VFRSqrt, IROps::OP_VFRSQRT>();
		_Op.first->Vector = Vector;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	/// Reciprocal Square Root - matches the precision required by the x86 spec.
	///
	/// It has a relative error of at most 1.5 * 2^-12
	IRPair<IROp_VFRSqrt> _VFRSqrt(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Vector) {
		auto ListDataBegin = DualListData.ListBegin();
		Vector->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector) == RegClass::Invalid || WalkFindRegClass(Vector) == RegClass::FPR, "GetOpRegClass(Vector) == RegClass::Invalid || WalkFindRegClass(Vector) == RegClass::FPR");
#endif
		return _VFRSqrt(RegisterSize, ElementSize, Vector->Wrapped(ListDataBegin));
	}

	/// Similar to VFRSqrt but carrying more precision for 3DNow!
	///
	/// It provides at least 15 bits precision, with a relative error of at most 2^-15
	IRPair<IROp_VFRSqrtPrecision> _VFRSqrtPrecision(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Vector) {
		auto _Op = AllocateOp<IROp_VFRSqrtPrecision, IROps::OP_VFRSQRTPRECISION>();
		_Op.first->Vector = Vector;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	/// Similar to VFRSqrt but carrying more precision for 3DNow!
	///
	/// It provides at least 15 bits precision, with a relative error of at most 2^-15
	IRPair<IROp_VFRSqrtPrecision> _VFRSqrtPrecision(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Vector) {
		auto ListDataBegin = DualListData.ListBegin();
		Vector->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector) == RegClass::Invalid || WalkFindRegClass(Vector) == RegClass::FPR, "GetOpRegClass(Vector) == RegClass::Invalid || WalkFindRegClass(Vector) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(RegisterSize == FEXCore::IR::OpSize::i64Bit || RegisterSize == FEXCore::IR::OpSize::i32Bit, "RegisterSize == FEXCore::IR::OpSize::i64Bit || RegisterSize == FEXCore::IR::OpSize::i32Bit");
		LOGMAN_THROW_A_FMT(ElementSize == FEXCore::IR::OpSize::i32Bit, "ElementSize == FEXCore::IR::OpSize::i32Bit");
#endif
		return _VFRSqrtPrecision(RegisterSize, ElementSize, Vector->Wrapped(ListDataBegin));
	}

	IRPair<IROp_VCMPEQZ> _VCMPEQZ(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Vector) {
		auto _Op = AllocateOp<IROp_VCMPEQZ, IROps::OP_VCMPEQZ>();
		_Op.first->Vector = Vector;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	IRPair<IROp_VCMPEQZ> _VCMPEQZ(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Vector) {
		auto ListDataBegin = DualListData.ListBegin();
		Vector->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector) == RegClass::Invalid || WalkFindRegClass(Vector) == RegClass::FPR, "GetOpRegClass(Vector) == RegClass::Invalid || WalkFindRegClass(Vector) == RegClass::FPR");
#endif
		return _VCMPEQZ(RegisterSize, ElementSize, Vector->Wrapped(ListDataBegin));
	}

	/// Vector compare signed greater than
	///
	/// Each element is compared, if the result is true then the resulting element is ~0, else zero
	///
	/// Compares the vector against zero
	IRPair<IROp_VCMPGTZ> _VCMPGTZ(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Vector) {
		auto _Op = AllocateOp<IROp_VCMPGTZ, IROps::OP_VCMPGTZ>();
		_Op.first->Vector = Vector;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	/// Vector compare signed greater than
	///
	/// Each element is compared, if the result is true then the resulting element is ~0, else zero
	///
	/// Compares the vector against zero
	IRPair<IROp_VCMPGTZ> _VCMPGTZ(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Vector) {
		auto ListDataBegin = DualListData.ListBegin();
		Vector->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector) == RegClass::Invalid || WalkFindRegClass(Vector) == RegClass::FPR, "GetOpRegClass(Vector) == RegClass::Invalid || WalkFindRegClass(Vector) == RegClass::FPR");
#endif
		return _VCMPGTZ(RegisterSize, ElementSize, Vector->Wrapped(ListDataBegin));
	}

	/// Vector compare signed less than
	///
	/// Each element is compared, if the result is true then the resulting element is ~0, else zero
	///
	/// Compares the vector against zero
	IRPair<IROp_VCMPLTZ> _VCMPLTZ(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Vector) {
		auto _Op = AllocateOp<IROp_VCMPLTZ, IROps::OP_VCMPLTZ>();
		_Op.first->Vector = Vector;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	/// Vector compare signed less than
	///
	/// Each element is compared, if the result is true then the resulting element is ~0, else zero
	///
	/// Compares the vector against zero
	IRPair<IROp_VCMPLTZ> _VCMPLTZ(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Vector) {
		auto ListDataBegin = DualListData.ListBegin();
		Vector->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector) == RegClass::Invalid || WalkFindRegClass(Vector) == RegClass::FPR, "GetOpRegClass(Vector) == RegClass::Invalid || WalkFindRegClass(Vector) == RegClass::FPR");
#endif
		return _VCMPLTZ(RegisterSize, ElementSize, Vector->Wrapped(ListDataBegin));
	}

	/// Duplicates one element from the source register across the whole register
	IRPair<IROp_VDupElement> _VDupElement(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Vector, uint8_t Index) {
		auto _Op = AllocateOp<IROp_VDupElement, IROps::OP_VDUPELEMENT>();
		_Op.first->Vector = Vector;
		_Op.first->Index = Index;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	/// Duplicates one element from the source register across the whole register
	IRPair<IROp_VDupElement> _VDupElement(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Vector, uint8_t Index) {
		auto ListDataBegin = DualListData.ListBegin();
		Vector->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector) == RegClass::Invalid || WalkFindRegClass(Vector) == RegClass::FPR, "GetOpRegClass(Vector) == RegClass::Invalid || WalkFindRegClass(Vector) == RegClass::FPR");
#endif
		return _VDupElement(RegisterSize, ElementSize, Vector->Wrapped(ListDataBegin), Index);
	}

	IRPair<IROp_VShlI> _VShlI(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Vector, uint8_t BitShift) {
		auto _Op = AllocateOp<IROp_VShlI, IROps::OP_VSHLI>();
		_Op.first->Vector = Vector;
		_Op.first->BitShift = BitShift;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	IRPair<IROp_VShlI> _VShlI(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Vector, uint8_t BitShift) {
		auto ListDataBegin = DualListData.ListBegin();
		Vector->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector) == RegClass::Invalid || WalkFindRegClass(Vector) == RegClass::FPR, "GetOpRegClass(Vector) == RegClass::Invalid || WalkFindRegClass(Vector) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(ElementSize >= FEXCore::IR::OpSize::i8Bit && ElementSize <= FEXCore::IR::OpSize::i64Bit, "ElementSize >= FEXCore::IR::OpSize::i8Bit && ElementSize <= FEXCore::IR::OpSize::i64Bit");
		LOGMAN_THROW_A_FMT(BitShift > 0, "BitShift > 0");
#endif
		return _VShlI(RegisterSize, ElementSize, Vector->Wrapped(ListDataBegin), BitShift);
	}

	IRPair<IROp_VUShrI> _VUShrI(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Vector, uint8_t BitShift) {
		auto _Op = AllocateOp<IROp_VUShrI, IROps::OP_VUSHRI>();
		_Op.first->Vector = Vector;
		_Op.first->BitShift = BitShift;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	IRPair<IROp_VUShrI> _VUShrI(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Vector, uint8_t BitShift) {
		auto ListDataBegin = DualListData.ListBegin();
		Vector->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector) == RegClass::Invalid || WalkFindRegClass(Vector) == RegClass::FPR, "GetOpRegClass(Vector) == RegClass::Invalid || WalkFindRegClass(Vector) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(ElementSize >= FEXCore::IR::OpSize::i8Bit && ElementSize <= FEXCore::IR::OpSize::i64Bit, "ElementSize >= FEXCore::IR::OpSize::i8Bit && ElementSize <= FEXCore::IR::OpSize::i64Bit");
		LOGMAN_THROW_A_FMT(BitShift > 0, "BitShift > 0");
#endif
		return _VUShrI(RegisterSize, ElementSize, Vector->Wrapped(ListDataBegin), BitShift);
	}

	IRPair<IROp_VUShraI> _VUShraI(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper DestVector, OrderedNodeWrapper Vector, uint8_t BitShift) {
		auto _Op = AllocateOp<IROp_VUShraI, IROps::OP_VUSHRAI>();
		_Op.first->DestVector = DestVector;
		_Op.first->Vector = Vector;
		_Op.first->BitShift = BitShift;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	IRPair<IROp_VUShraI> _VUShraI(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *DestVector, OrderedNode *Vector, uint8_t BitShift) {
		auto ListDataBegin = DualListData.ListBegin();
		DestVector->AddUse();
		Vector->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(DestVector) == RegClass::Invalid || WalkFindRegClass(DestVector) == RegClass::FPR, "GetOpRegClass(DestVector) == RegClass::Invalid || WalkFindRegClass(DestVector) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector) == RegClass::Invalid || WalkFindRegClass(Vector) == RegClass::FPR, "GetOpRegClass(Vector) == RegClass::Invalid || WalkFindRegClass(Vector) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(ElementSize >= FEXCore::IR::OpSize::i8Bit && ElementSize <= FEXCore::IR::OpSize::i64Bit, "ElementSize >= FEXCore::IR::OpSize::i8Bit && ElementSize <= FEXCore::IR::OpSize::i64Bit");
		LOGMAN_THROW_A_FMT(BitShift > 0 && BitShift <= IR::OpSizeAsBits(ElementSize), "BitShift > 0 && BitShift <= IR::OpSizeAsBits(ElementSize)");
#endif
		return _VUShraI(RegisterSize, ElementSize, DestVector->Wrapped(ListDataBegin), Vector->Wrapped(ListDataBegin), BitShift);
	}

	IRPair<IROp_VSShrI> _VSShrI(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Vector, uint8_t BitShift) {
		auto _Op = AllocateOp<IROp_VSShrI, IROps::OP_VSSHRI>();
		_Op.first->Vector = Vector;
		_Op.first->BitShift = BitShift;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	IRPair<IROp_VSShrI> _VSShrI(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Vector, uint8_t BitShift) {
		auto ListDataBegin = DualListData.ListBegin();
		Vector->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector) == RegClass::Invalid || WalkFindRegClass(Vector) == RegClass::FPR, "GetOpRegClass(Vector) == RegClass::Invalid || WalkFindRegClass(Vector) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(ElementSize >= FEXCore::IR::OpSize::i8Bit && ElementSize <= FEXCore::IR::OpSize::i64Bit, "ElementSize >= FEXCore::IR::OpSize::i8Bit && ElementSize <= FEXCore::IR::OpSize::i64Bit");
		LOGMAN_THROW_A_FMT(BitShift > 0, "BitShift > 0");
#endif
		return _VSShrI(RegisterSize, ElementSize, Vector->Wrapped(ListDataBegin), BitShift);
	}

	/// U
	///
	/// n
	///
	/// s
	///
	/// i
	///
	/// g
	///
	/// n
	///
	/// e
	///
	/// d
	///
	///  
	///
	/// s
	///
	/// h
	///
	/// i
	///
	/// f
	///
	/// t
	///
	/// s
	///
	///  
	///
	/// r
	///
	/// i
	///
	/// g
	///
	/// h
	///
	/// t
	///
	///  
	///
	/// e
	///
	/// a
	///
	/// c
	///
	/// h
	///
	///  
	///
	/// e
	///
	/// l
	///
	/// e
	///
	/// m
	///
	/// e
	///
	/// n
	///
	/// t
	///
	///  
	///
	/// a
	///
	/// n
	///
	/// d
	///
	///  
	///
	/// t
	///
	/// h
	///
	/// e
	///
	/// n
	///
	///  
	///
	/// n
	///
	/// a
	///
	/// r
	///
	/// r
	///
	/// o
	///
	/// w
	///
	/// s
	///
	///  
	///
	/// t
	///
	/// o
	///
	///  
	///
	/// t
	///
	/// h
	///
	/// e
	///
	///  
	///
	/// n
	///
	/// e
	///
	/// x
	///
	/// t
	///
	///  
	///
	/// l
	///
	/// o
	///
	/// w
	///
	/// e
	///
	/// r
	///
	///  
	///
	/// e
	///
	/// l
	///
	/// e
	///
	/// m
	///
	/// e
	///
	/// n
	///
	/// t
	///
	///  
	///
	/// s
	///
	/// i
	///
	/// z
	///
	/// e
	IRPair<IROp_VUShrNI> _VUShrNI(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Vector, uint8_t BitShift) {
		auto _Op = AllocateOp<IROp_VUShrNI, IROps::OP_VUSHRNI>();
		_Op.first->Vector = Vector;
		_Op.first->BitShift = BitShift;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize >> 1;
		return _Op;
	}

	/// U
	///
	/// n
	///
	/// s
	///
	/// i
	///
	/// g
	///
	/// n
	///
	/// e
	///
	/// d
	///
	///  
	///
	/// s
	///
	/// h
	///
	/// i
	///
	/// f
	///
	/// t
	///
	/// s
	///
	///  
	///
	/// r
	///
	/// i
	///
	/// g
	///
	/// h
	///
	/// t
	///
	///  
	///
	/// e
	///
	/// a
	///
	/// c
	///
	/// h
	///
	///  
	///
	/// e
	///
	/// l
	///
	/// e
	///
	/// m
	///
	/// e
	///
	/// n
	///
	/// t
	///
	///  
	///
	/// a
	///
	/// n
	///
	/// d
	///
	///  
	///
	/// t
	///
	/// h
	///
	/// e
	///
	/// n
	///
	///  
	///
	/// n
	///
	/// a
	///
	/// r
	///
	/// r
	///
	/// o
	///
	/// w
	///
	/// s
	///
	///  
	///
	/// t
	///
	/// o
	///
	///  
	///
	/// t
	///
	/// h
	///
	/// e
	///
	///  
	///
	/// n
	///
	/// e
	///
	/// x
	///
	/// t
	///
	///  
	///
	/// l
	///
	/// o
	///
	/// w
	///
	/// e
	///
	/// r
	///
	///  
	///
	/// e
	///
	/// l
	///
	/// e
	///
	/// m
	///
	/// e
	///
	/// n
	///
	/// t
	///
	///  
	///
	/// s
	///
	/// i
	///
	/// z
	///
	/// e
	IRPair<IROp_VUShrNI> _VUShrNI(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Vector, uint8_t BitShift) {
		auto ListDataBegin = DualListData.ListBegin();
		Vector->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector) == RegClass::Invalid || WalkFindRegClass(Vector) == RegClass::FPR, "GetOpRegClass(Vector) == RegClass::Invalid || WalkFindRegClass(Vector) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(ElementSize >= FEXCore::IR::OpSize::i16Bit && ElementSize <= FEXCore::IR::OpSize::i64Bit, "ElementSize >= FEXCore::IR::OpSize::i16Bit && ElementSize <= FEXCore::IR::OpSize::i64Bit");
		LOGMAN_THROW_A_FMT(BitShift <= IR::OpSizeAsBits(ElementSize), "BitShift <= IR::OpSizeAsBits(ElementSize)");
#endif
		return _VUShrNI(RegisterSize, ElementSize, Vector->Wrapped(ListDataBegin), BitShift);
	}

	/// Unsigned shifts right each element and then narrows to the next lower element size
	///
	/// Inserts results in to the high elements of the first argument
	IRPair<IROp_VUShrNI2> _VUShrNI2(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper VectorLower, OrderedNodeWrapper VectorUpper, uint8_t BitShift) {
		auto _Op = AllocateOp<IROp_VUShrNI2, IROps::OP_VUSHRNI2>();
		_Op.first->VectorLower = VectorLower;
		_Op.first->VectorUpper = VectorUpper;
		_Op.first->BitShift = BitShift;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize >> 1;
		return _Op;
	}

	/// Unsigned shifts right each element and then narrows to the next lower element size
	///
	/// Inserts results in to the high elements of the first argument
	IRPair<IROp_VUShrNI2> _VUShrNI2(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *VectorLower, OrderedNode *VectorUpper, uint8_t BitShift) {
		auto ListDataBegin = DualListData.ListBegin();
		VectorLower->AddUse();
		VectorUpper->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(VectorLower) == RegClass::Invalid || WalkFindRegClass(VectorLower) == RegClass::FPR, "GetOpRegClass(VectorLower) == RegClass::Invalid || WalkFindRegClass(VectorLower) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(VectorUpper) == RegClass::Invalid || WalkFindRegClass(VectorUpper) == RegClass::FPR, "GetOpRegClass(VectorUpper) == RegClass::Invalid || WalkFindRegClass(VectorUpper) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(ElementSize >= FEXCore::IR::OpSize::i16Bit && ElementSize <= FEXCore::IR::OpSize::i64Bit, "ElementSize >= FEXCore::IR::OpSize::i16Bit && ElementSize <= FEXCore::IR::OpSize::i64Bit");
		LOGMAN_THROW_A_FMT(BitShift > 0 && BitShift <= IR::OpSizeAsBits(ElementSize), "BitShift > 0 && BitShift <= IR::OpSizeAsBits(ElementSize)");
#endif
		return _VUShrNI2(RegisterSize, ElementSize, VectorLower->Wrapped(ListDataBegin), VectorUpper->Wrapped(ListDataBegin), BitShift);
	}

	/// S
	///
	/// i
	///
	/// g
	///
	/// n
	///
	///  
	///
	/// e
	///
	/// x
	///
	/// t
	///
	/// e
	///
	/// n
	///
	/// d
	///
	/// s
	///
	///  
	///
	/// e
	///
	/// l
	///
	/// e
	///
	/// m
	///
	/// e
	///
	/// n
	///
	/// t
	///
	/// s
	///
	///  
	///
	/// f
	///
	/// r
	///
	/// o
	///
	/// m
	///
	///  
	///
	/// t
	///
	/// h
	///
	/// e
	///
	///  
	///
	/// s
	///
	/// o
	///
	/// u
	///
	/// r
	///
	/// c
	///
	/// e
	///
	///  
	///
	/// e
	///
	/// l
	///
	/// e
	///
	/// m
	///
	/// e
	///
	/// n
	///
	/// t
	///
	///  
	///
	/// s
	///
	/// i
	///
	/// z
	///
	/// e
	///
	///  
	///
	/// t
	///
	/// o
	///
	///  
	///
	/// t
	///
	/// h
	///
	/// e
	///
	///  
	///
	/// n
	///
	/// e
	///
	/// x
	///
	/// t
	///
	///  
	///
	/// s
	///
	/// i
	///
	/// z
	///
	/// e
	///
	///  
	///
	/// u
	///
	/// p
	IRPair<IROp_VSXTL> _VSXTL(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Vector) {
		auto _Op = AllocateOp<IROp_VSXTL, IROps::OP_VSXTL>();
		_Op.first->Vector = Vector;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize << 1;
		return _Op;
	}

	/// S
	///
	/// i
	///
	/// g
	///
	/// n
	///
	///  
	///
	/// e
	///
	/// x
	///
	/// t
	///
	/// e
	///
	/// n
	///
	/// d
	///
	/// s
	///
	///  
	///
	/// e
	///
	/// l
	///
	/// e
	///
	/// m
	///
	/// e
	///
	/// n
	///
	/// t
	///
	/// s
	///
	///  
	///
	/// f
	///
	/// r
	///
	/// o
	///
	/// m
	///
	///  
	///
	/// t
	///
	/// h
	///
	/// e
	///
	///  
	///
	/// s
	///
	/// o
	///
	/// u
	///
	/// r
	///
	/// c
	///
	/// e
	///
	///  
	///
	/// e
	///
	/// l
	///
	/// e
	///
	/// m
	///
	/// e
	///
	/// n
	///
	/// t
	///
	///  
	///
	/// s
	///
	/// i
	///
	/// z
	///
	/// e
	///
	///  
	///
	/// t
	///
	/// o
	///
	///  
	///
	/// t
	///
	/// h
	///
	/// e
	///
	///  
	///
	/// n
	///
	/// e
	///
	/// x
	///
	/// t
	///
	///  
	///
	/// s
	///
	/// i
	///
	/// z
	///
	/// e
	///
	///  
	///
	/// u
	///
	/// p
	IRPair<IROp_VSXTL> _VSXTL(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Vector) {
		auto ListDataBegin = DualListData.ListBegin();
		Vector->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector) == RegClass::Invalid || WalkFindRegClass(Vector) == RegClass::FPR, "GetOpRegClass(Vector) == RegClass::Invalid || WalkFindRegClass(Vector) == RegClass::FPR");
#endif
		return _VSXTL(RegisterSize, ElementSize, Vector->Wrapped(ListDataBegin));
	}

	/// Sign extends elements from the source element size to the next size up
	///
	/// Source elements come from the upper half of the register
	IRPair<IROp_VSXTL2> _VSXTL2(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Vector) {
		auto _Op = AllocateOp<IROp_VSXTL2, IROps::OP_VSXTL2>();
		_Op.first->Vector = Vector;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize << 1;
		return _Op;
	}

	/// Sign extends elements from the source element size to the next size up
	///
	/// Source elements come from the upper half of the register
	IRPair<IROp_VSXTL2> _VSXTL2(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Vector) {
		auto ListDataBegin = DualListData.ListBegin();
		Vector->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector) == RegClass::Invalid || WalkFindRegClass(Vector) == RegClass::FPR, "GetOpRegClass(Vector) == RegClass::Invalid || WalkFindRegClass(Vector) == RegClass::FPR");
#endif
		return _VSXTL2(RegisterSize, ElementSize, Vector->Wrapped(ListDataBegin));
	}

	/// S
	///
	/// i
	///
	/// g
	///
	/// n
	///
	///  
	///
	/// e
	///
	/// x
	///
	/// t
	///
	/// e
	///
	/// n
	///
	/// d
	///
	/// s
	///
	///  
	///
	/// e
	///
	/// l
	///
	/// e
	///
	/// m
	///
	/// e
	///
	/// n
	///
	/// t
	///
	/// s
	///
	///  
	///
	/// f
	///
	/// r
	///
	/// o
	///
	/// m
	///
	///  
	///
	/// t
	///
	/// h
	///
	/// e
	///
	///  
	///
	/// s
	///
	/// o
	///
	/// u
	///
	/// r
	///
	/// c
	///
	/// e
	///
	///  
	///
	/// e
	///
	/// l
	///
	/// e
	///
	/// m
	///
	/// e
	///
	/// n
	///
	/// t
	///
	///  
	///
	/// s
	///
	/// i
	///
	/// z
	///
	/// e
	///
	///  
	///
	/// t
	///
	/// o
	///
	///  
	///
	/// t
	///
	/// h
	///
	/// e
	///
	///  
	///
	/// n
	///
	/// e
	///
	/// x
	///
	/// t
	///
	///  
	///
	/// s
	///
	/// i
	///
	/// z
	///
	/// e
	///
	///  
	///
	/// u
	///
	/// p
	IRPair<IROp_VSSHLL> _VSSHLL(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Vector, uint8_t BitShift = 0) {
		auto _Op = AllocateOp<IROp_VSSHLL, IROps::OP_VSSHLL>();
		_Op.first->Vector = Vector;
		_Op.first->BitShift = BitShift;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize << 1;
		return _Op;
	}

	/// S
	///
	/// i
	///
	/// g
	///
	/// n
	///
	///  
	///
	/// e
	///
	/// x
	///
	/// t
	///
	/// e
	///
	/// n
	///
	/// d
	///
	/// s
	///
	///  
	///
	/// e
	///
	/// l
	///
	/// e
	///
	/// m
	///
	/// e
	///
	/// n
	///
	/// t
	///
	/// s
	///
	///  
	///
	/// f
	///
	/// r
	///
	/// o
	///
	/// m
	///
	///  
	///
	/// t
	///
	/// h
	///
	/// e
	///
	///  
	///
	/// s
	///
	/// o
	///
	/// u
	///
	/// r
	///
	/// c
	///
	/// e
	///
	///  
	///
	/// e
	///
	/// l
	///
	/// e
	///
	/// m
	///
	/// e
	///
	/// n
	///
	/// t
	///
	///  
	///
	/// s
	///
	/// i
	///
	/// z
	///
	/// e
	///
	///  
	///
	/// t
	///
	/// o
	///
	///  
	///
	/// t
	///
	/// h
	///
	/// e
	///
	///  
	///
	/// n
	///
	/// e
	///
	/// x
	///
	/// t
	///
	///  
	///
	/// s
	///
	/// i
	///
	/// z
	///
	/// e
	///
	///  
	///
	/// u
	///
	/// p
	IRPair<IROp_VSSHLL> _VSSHLL(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Vector, uint8_t BitShift = 0) {
		auto ListDataBegin = DualListData.ListBegin();
		Vector->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector) == RegClass::Invalid || WalkFindRegClass(Vector) == RegClass::FPR, "GetOpRegClass(Vector) == RegClass::Invalid || WalkFindRegClass(Vector) == RegClass::FPR");
#endif
		return _VSSHLL(RegisterSize, ElementSize, Vector->Wrapped(ListDataBegin), BitShift);
	}

	/// Sign extends elements from the source element size to the next size up
	///
	/// Source elements come from the upper half of the register
	IRPair<IROp_VSSHLL2> _VSSHLL2(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Vector, uint8_t BitShift = 0) {
		auto _Op = AllocateOp<IROp_VSSHLL2, IROps::OP_VSSHLL2>();
		_Op.first->Vector = Vector;
		_Op.first->BitShift = BitShift;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize << 1;
		return _Op;
	}

	/// Sign extends elements from the source element size to the next size up
	///
	/// Source elements come from the upper half of the register
	IRPair<IROp_VSSHLL2> _VSSHLL2(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Vector, uint8_t BitShift = 0) {
		auto ListDataBegin = DualListData.ListBegin();
		Vector->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector) == RegClass::Invalid || WalkFindRegClass(Vector) == RegClass::FPR, "GetOpRegClass(Vector) == RegClass::Invalid || WalkFindRegClass(Vector) == RegClass::FPR");
#endif
		return _VSSHLL2(RegisterSize, ElementSize, Vector->Wrapped(ListDataBegin), BitShift);
	}

	/// Z
	///
	/// e
	///
	/// r
	///
	/// o
	///
	///  
	///
	/// e
	///
	/// x
	///
	/// t
	///
	/// e
	///
	/// n
	///
	/// d
	///
	/// s
	///
	///  
	///
	/// e
	///
	/// l
	///
	/// e
	///
	/// m
	///
	/// e
	///
	/// n
	///
	/// t
	///
	/// s
	///
	///  
	///
	/// f
	///
	/// r
	///
	/// o
	///
	/// m
	///
	///  
	///
	/// t
	///
	/// h
	///
	/// e
	///
	///  
	///
	/// s
	///
	/// o
	///
	/// u
	///
	/// r
	///
	/// c
	///
	/// e
	///
	///  
	///
	/// e
	///
	/// l
	///
	/// e
	///
	/// m
	///
	/// e
	///
	/// n
	///
	/// t
	///
	///  
	///
	/// s
	///
	/// i
	///
	/// z
	///
	/// e
	///
	///  
	///
	/// t
	///
	/// o
	///
	///  
	///
	/// t
	///
	/// h
	///
	/// e
	///
	///  
	///
	/// n
	///
	/// e
	///
	/// x
	///
	/// t
	///
	///  
	///
	/// s
	///
	/// i
	///
	/// z
	///
	/// e
	///
	///  
	///
	/// u
	///
	/// p
	IRPair<IROp_VUXTL> _VUXTL(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Vector) {
		auto _Op = AllocateOp<IROp_VUXTL, IROps::OP_VUXTL>();
		_Op.first->Vector = Vector;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize << 1;
		return _Op;
	}

	/// Z
	///
	/// e
	///
	/// r
	///
	/// o
	///
	///  
	///
	/// e
	///
	/// x
	///
	/// t
	///
	/// e
	///
	/// n
	///
	/// d
	///
	/// s
	///
	///  
	///
	/// e
	///
	/// l
	///
	/// e
	///
	/// m
	///
	/// e
	///
	/// n
	///
	/// t
	///
	/// s
	///
	///  
	///
	/// f
	///
	/// r
	///
	/// o
	///
	/// m
	///
	///  
	///
	/// t
	///
	/// h
	///
	/// e
	///
	///  
	///
	/// s
	///
	/// o
	///
	/// u
	///
	/// r
	///
	/// c
	///
	/// e
	///
	///  
	///
	/// e
	///
	/// l
	///
	/// e
	///
	/// m
	///
	/// e
	///
	/// n
	///
	/// t
	///
	///  
	///
	/// s
	///
	/// i
	///
	/// z
	///
	/// e
	///
	///  
	///
	/// t
	///
	/// o
	///
	///  
	///
	/// t
	///
	/// h
	///
	/// e
	///
	///  
	///
	/// n
	///
	/// e
	///
	/// x
	///
	/// t
	///
	///  
	///
	/// s
	///
	/// i
	///
	/// z
	///
	/// e
	///
	///  
	///
	/// u
	///
	/// p
	IRPair<IROp_VUXTL> _VUXTL(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Vector) {
		auto ListDataBegin = DualListData.ListBegin();
		Vector->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector) == RegClass::Invalid || WalkFindRegClass(Vector) == RegClass::FPR, "GetOpRegClass(Vector) == RegClass::Invalid || WalkFindRegClass(Vector) == RegClass::FPR");
#endif
		return _VUXTL(RegisterSize, ElementSize, Vector->Wrapped(ListDataBegin));
	}

	/// Zero extends elements from the source element size to the next size up
	///
	/// Source elements come from the upper half of the register
	IRPair<IROp_VUXTL2> _VUXTL2(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Vector) {
		auto _Op = AllocateOp<IROp_VUXTL2, IROps::OP_VUXTL2>();
		_Op.first->Vector = Vector;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize << 1;
		return _Op;
	}

	/// Zero extends elements from the source element size to the next size up
	///
	/// Source elements come from the upper half of the register
	IRPair<IROp_VUXTL2> _VUXTL2(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Vector) {
		auto ListDataBegin = DualListData.ListBegin();
		Vector->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector) == RegClass::Invalid || WalkFindRegClass(Vector) == RegClass::FPR, "GetOpRegClass(Vector) == RegClass::Invalid || WalkFindRegClass(Vector) == RegClass::FPR");
#endif
		return _VUXTL2(RegisterSize, ElementSize, Vector->Wrapped(ListDataBegin));
	}

	IRPair<IROp_VSQXTN> _VSQXTN(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Vector) {
		auto _Op = AllocateOp<IROp_VSQXTN, IROps::OP_VSQXTN>();
		_Op.first->Vector = Vector;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize >> 1;
		return _Op;
	}

	IRPair<IROp_VSQXTN> _VSQXTN(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Vector) {
		auto ListDataBegin = DualListData.ListBegin();
		Vector->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector) == RegClass::Invalid || WalkFindRegClass(Vector) == RegClass::FPR, "GetOpRegClass(Vector) == RegClass::Invalid || WalkFindRegClass(Vector) == RegClass::FPR");
#endif
		return _VSQXTN(RegisterSize, ElementSize, Vector->Wrapped(ListDataBegin));
	}

	IRPair<IROp_VSQXTN2> _VSQXTN2(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper VectorLower, OrderedNodeWrapper VectorUpper) {
		auto _Op = AllocateOp<IROp_VSQXTN2, IROps::OP_VSQXTN2>();
		_Op.first->VectorLower = VectorLower;
		_Op.first->VectorUpper = VectorUpper;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize >> 1;
		return _Op;
	}

	IRPair<IROp_VSQXTN2> _VSQXTN2(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *VectorLower, OrderedNode *VectorUpper) {
		auto ListDataBegin = DualListData.ListBegin();
		VectorLower->AddUse();
		VectorUpper->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(VectorLower) == RegClass::Invalid || WalkFindRegClass(VectorLower) == RegClass::FPR, "GetOpRegClass(VectorLower) == RegClass::Invalid || WalkFindRegClass(VectorLower) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(VectorUpper) == RegClass::Invalid || WalkFindRegClass(VectorUpper) == RegClass::FPR, "GetOpRegClass(VectorUpper) == RegClass::Invalid || WalkFindRegClass(VectorUpper) == RegClass::FPR");
#endif
		return _VSQXTN2(RegisterSize, ElementSize, VectorLower->Wrapped(ListDataBegin), VectorUpper->Wrapped(ListDataBegin));
	}

	/// Does both VSQXTN and VSQXTN2 in a combined operation.
	IRPair<IROp_VSQXTNPair> _VSQXTNPair(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper VectorLower, OrderedNodeWrapper VectorUpper) {
		auto _Op = AllocateOp<IROp_VSQXTNPair, IROps::OP_VSQXTNPAIR>();
		_Op.first->VectorLower = VectorLower;
		_Op.first->VectorUpper = VectorUpper;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize >> 1;
		return _Op;
	}

	/// Does both VSQXTN and VSQXTN2 in a combined operation.
	IRPair<IROp_VSQXTNPair> _VSQXTNPair(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *VectorLower, OrderedNode *VectorUpper) {
		auto ListDataBegin = DualListData.ListBegin();
		VectorLower->AddUse();
		VectorUpper->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(VectorLower) == RegClass::Invalid || WalkFindRegClass(VectorLower) == RegClass::FPR, "GetOpRegClass(VectorLower) == RegClass::Invalid || WalkFindRegClass(VectorLower) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(VectorUpper) == RegClass::Invalid || WalkFindRegClass(VectorUpper) == RegClass::FPR, "GetOpRegClass(VectorUpper) == RegClass::Invalid || WalkFindRegClass(VectorUpper) == RegClass::FPR");
#endif
		return _VSQXTNPair(RegisterSize, ElementSize, VectorLower->Wrapped(ListDataBegin), VectorUpper->Wrapped(ListDataBegin));
	}

	IRPair<IROp_VSQXTUN> _VSQXTUN(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Vector) {
		auto _Op = AllocateOp<IROp_VSQXTUN, IROps::OP_VSQXTUN>();
		_Op.first->Vector = Vector;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize >> 1;
		return _Op;
	}

	IRPair<IROp_VSQXTUN> _VSQXTUN(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Vector) {
		auto ListDataBegin = DualListData.ListBegin();
		Vector->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector) == RegClass::Invalid || WalkFindRegClass(Vector) == RegClass::FPR, "GetOpRegClass(Vector) == RegClass::Invalid || WalkFindRegClass(Vector) == RegClass::FPR");
#endif
		return _VSQXTUN(RegisterSize, ElementSize, Vector->Wrapped(ListDataBegin));
	}

	IRPair<IROp_VSQXTUN2> _VSQXTUN2(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper VectorLower, OrderedNodeWrapper VectorUpper) {
		auto _Op = AllocateOp<IROp_VSQXTUN2, IROps::OP_VSQXTUN2>();
		_Op.first->VectorLower = VectorLower;
		_Op.first->VectorUpper = VectorUpper;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize >> 1;
		return _Op;
	}

	IRPair<IROp_VSQXTUN2> _VSQXTUN2(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *VectorLower, OrderedNode *VectorUpper) {
		auto ListDataBegin = DualListData.ListBegin();
		VectorLower->AddUse();
		VectorUpper->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(VectorLower) == RegClass::Invalid || WalkFindRegClass(VectorLower) == RegClass::FPR, "GetOpRegClass(VectorLower) == RegClass::Invalid || WalkFindRegClass(VectorLower) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(VectorUpper) == RegClass::Invalid || WalkFindRegClass(VectorUpper) == RegClass::FPR, "GetOpRegClass(VectorUpper) == RegClass::Invalid || WalkFindRegClass(VectorUpper) == RegClass::FPR");
#endif
		return _VSQXTUN2(RegisterSize, ElementSize, VectorLower->Wrapped(ListDataBegin), VectorUpper->Wrapped(ListDataBegin));
	}

	/// Does both VSQXTUN and VSQXTUN2 in a combined operation.
	IRPair<IROp_VSQXTUNPair> _VSQXTUNPair(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper VectorLower, OrderedNodeWrapper VectorUpper) {
		auto _Op = AllocateOp<IROp_VSQXTUNPair, IROps::OP_VSQXTUNPAIR>();
		_Op.first->VectorLower = VectorLower;
		_Op.first->VectorUpper = VectorUpper;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize >> 1;
		return _Op;
	}

	/// Does both VSQXTUN and VSQXTUN2 in a combined operation.
	IRPair<IROp_VSQXTUNPair> _VSQXTUNPair(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *VectorLower, OrderedNode *VectorUpper) {
		auto ListDataBegin = DualListData.ListBegin();
		VectorLower->AddUse();
		VectorUpper->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(VectorLower) == RegClass::Invalid || WalkFindRegClass(VectorLower) == RegClass::FPR, "GetOpRegClass(VectorLower) == RegClass::Invalid || WalkFindRegClass(VectorLower) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(VectorUpper) == RegClass::Invalid || WalkFindRegClass(VectorUpper) == RegClass::FPR, "GetOpRegClass(VectorUpper) == RegClass::Invalid || WalkFindRegClass(VectorUpper) == RegClass::FPR");
#endif
		return _VSQXTUNPair(RegisterSize, ElementSize, VectorLower->Wrapped(ListDataBegin), VectorUpper->Wrapped(ListDataBegin));
	}

	/// Signed rounding shift right by immediate
	///
	/// Exactly matching Arm64 srshr semantics
	IRPair<IROp_VSRSHR> _VSRSHR(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Vector, uint8_t BitShift) {
		auto _Op = AllocateOp<IROp_VSRSHR, IROps::OP_VSRSHR>();
		_Op.first->Vector = Vector;
		_Op.first->BitShift = BitShift;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	/// Signed rounding shift right by immediate
	///
	/// Exactly matching Arm64 srshr semantics
	IRPair<IROp_VSRSHR> _VSRSHR(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Vector, uint8_t BitShift) {
		auto ListDataBegin = DualListData.ListBegin();
		Vector->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector) == RegClass::Invalid || WalkFindRegClass(Vector) == RegClass::FPR, "GetOpRegClass(Vector) == RegClass::Invalid || WalkFindRegClass(Vector) == RegClass::FPR");
#endif
		return _VSRSHR(RegisterSize, ElementSize, Vector->Wrapped(ListDataBegin), BitShift);
	}

	/// Signed satuating shift left by immediate
	///
	/// Exactly matching Arm64 sqshl semantics
	IRPair<IROp_VSQSHL> _VSQSHL(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Vector, uint8_t BitShift) {
		auto _Op = AllocateOp<IROp_VSQSHL, IROps::OP_VSQSHL>();
		_Op.first->Vector = Vector;
		_Op.first->BitShift = BitShift;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	/// Signed satuating shift left by immediate
	///
	/// Exactly matching Arm64 sqshl semantics
	IRPair<IROp_VSQSHL> _VSQSHL(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Vector, uint8_t BitShift) {
		auto ListDataBegin = DualListData.ListBegin();
		Vector->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector) == RegClass::Invalid || WalkFindRegClass(Vector) == RegClass::FPR, "GetOpRegClass(Vector) == RegClass::Invalid || WalkFindRegClass(Vector) == RegClass::FPR");
#endif
		return _VSQSHL(RegisterSize, ElementSize, Vector->Wrapped(ListDataBegin), BitShift);
	}

	/// Reverses elements in 32-bit halfwords
	///
	/// Available element size: 1byte, 2 byte
	IRPair<IROp_VRev32> _VRev32(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Vector) {
		auto _Op = AllocateOp<IROp_VRev32, IROps::OP_VREV32>();
		_Op.first->Vector = Vector;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	/// Reverses elements in 32-bit halfwords
	///
	/// Available element size: 1byte, 2 byte
	IRPair<IROp_VRev32> _VRev32(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Vector) {
		auto ListDataBegin = DualListData.ListBegin();
		Vector->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector) == RegClass::Invalid || WalkFindRegClass(Vector) == RegClass::FPR, "GetOpRegClass(Vector) == RegClass::Invalid || WalkFindRegClass(Vector) == RegClass::FPR");
#endif
		return _VRev32(RegisterSize, ElementSize, Vector->Wrapped(ListDataBegin));
	}

	/// Reverses elements in 64-bit halfwords
	///
	/// Available element size: 1byte, 2 byte, 4 byte
	IRPair<IROp_VRev64> _VRev64(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Vector) {
		auto _Op = AllocateOp<IROp_VRev64, IROps::OP_VREV64>();
		_Op.first->Vector = Vector;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	/// Reverses elements in 64-bit halfwords
	///
	/// Available element size: 1byte, 2 byte, 4 byte
	IRPair<IROp_VRev64> _VRev64(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Vector) {
		auto ListDataBegin = DualListData.ListBegin();
		Vector->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector) == RegClass::Invalid || WalkFindRegClass(Vector) == RegClass::FPR, "GetOpRegClass(Vector) == RegClass::Invalid || WalkFindRegClass(Vector) == RegClass::FPR");
#endif
		return _VRev64(RegisterSize, ElementSize, Vector->Wrapped(ListDataBegin));
	}

	IRPair<IROp_VAdd> _VAdd(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Vector1, OrderedNodeWrapper Vector2) {
		auto _Op = AllocateOp<IROp_VAdd, IROps::OP_VADD>();
		_Op.first->Vector1 = Vector1;
		_Op.first->Vector2 = Vector2;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	IRPair<IROp_VAdd> _VAdd(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Vector1, OrderedNode *Vector2) {
		auto ListDataBegin = DualListData.ListBegin();
		Vector1->AddUse();
		Vector2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR, "GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR, "GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR");
#endif
		return _VAdd(RegisterSize, ElementSize, Vector1->Wrapped(ListDataBegin), Vector2->Wrapped(ListDataBegin));
	}

	IRPair<IROp_VSub> _VSub(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Vector1, OrderedNodeWrapper Vector2) {
		auto _Op = AllocateOp<IROp_VSub, IROps::OP_VSUB>();
		_Op.first->Vector1 = Vector1;
		_Op.first->Vector2 = Vector2;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	IRPair<IROp_VSub> _VSub(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Vector1, OrderedNode *Vector2) {
		auto ListDataBegin = DualListData.ListBegin();
		Vector1->AddUse();
		Vector2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR, "GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR, "GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR");
#endif
		return _VSub(RegisterSize, ElementSize, Vector1->Wrapped(ListDataBegin), Vector2->Wrapped(ListDataBegin));
	}

	IRPair<IROp_VAnd> _VAnd(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Vector1, OrderedNodeWrapper Vector2) {
		auto _Op = AllocateOp<IROp_VAnd, IROps::OP_VAND>();
		_Op.first->Vector1 = Vector1;
		_Op.first->Vector2 = Vector2;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	IRPair<IROp_VAnd> _VAnd(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Vector1, OrderedNode *Vector2) {
		auto ListDataBegin = DualListData.ListBegin();
		Vector1->AddUse();
		Vector2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR, "GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR, "GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(RegisterSize == FEXCore::IR::OpSize::i256Bit || RegisterSize == FEXCore::IR::OpSize::i128Bit || RegisterSize == FEXCore::IR::OpSize::i64Bit, "RegisterSize == FEXCore::IR::OpSize::i256Bit || RegisterSize == FEXCore::IR::OpSize::i128Bit || RegisterSize == FEXCore::IR::OpSize::i64Bit");
#endif
		return _VAnd(RegisterSize, ElementSize, Vector1->Wrapped(ListDataBegin), Vector2->Wrapped(ListDataBegin));
	}

	IRPair<IROp_VAndn> _VAndn(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Vector1, OrderedNodeWrapper Vector2) {
		auto _Op = AllocateOp<IROp_VAndn, IROps::OP_VANDN>();
		_Op.first->Vector1 = Vector1;
		_Op.first->Vector2 = Vector2;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	IRPair<IROp_VAndn> _VAndn(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Vector1, OrderedNode *Vector2) {
		auto ListDataBegin = DualListData.ListBegin();
		Vector1->AddUse();
		Vector2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR, "GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR, "GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(RegisterSize == FEXCore::IR::OpSize::i256Bit || RegisterSize == FEXCore::IR::OpSize::i128Bit || RegisterSize == FEXCore::IR::OpSize::i64Bit, "RegisterSize == FEXCore::IR::OpSize::i256Bit || RegisterSize == FEXCore::IR::OpSize::i128Bit || RegisterSize == FEXCore::IR::OpSize::i64Bit");
#endif
		return _VAndn(RegisterSize, ElementSize, Vector1->Wrapped(ListDataBegin), Vector2->Wrapped(ListDataBegin));
	}

	IRPair<IROp_VOrn> _VOrn(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Vector1, OrderedNodeWrapper Vector2) {
		auto _Op = AllocateOp<IROp_VOrn, IROps::OP_VORN>();
		_Op.first->Vector1 = Vector1;
		_Op.first->Vector2 = Vector2;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	IRPair<IROp_VOrn> _VOrn(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Vector1, OrderedNode *Vector2) {
		auto ListDataBegin = DualListData.ListBegin();
		Vector1->AddUse();
		Vector2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR, "GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR, "GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(RegisterSize == FEXCore::IR::OpSize::i256Bit || RegisterSize == FEXCore::IR::OpSize::i128Bit || RegisterSize == FEXCore::IR::OpSize::i64Bit, "RegisterSize == FEXCore::IR::OpSize::i256Bit || RegisterSize == FEXCore::IR::OpSize::i128Bit || RegisterSize == FEXCore::IR::OpSize::i64Bit");
#endif
		return _VOrn(RegisterSize, ElementSize, Vector1->Wrapped(ListDataBegin), Vector2->Wrapped(ListDataBegin));
	}

	IRPair<IROp_VOr> _VOr(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Vector1, OrderedNodeWrapper Vector2) {
		auto _Op = AllocateOp<IROp_VOr, IROps::OP_VOR>();
		_Op.first->Vector1 = Vector1;
		_Op.first->Vector2 = Vector2;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	IRPair<IROp_VOr> _VOr(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Vector1, OrderedNode *Vector2) {
		auto ListDataBegin = DualListData.ListBegin();
		Vector1->AddUse();
		Vector2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR, "GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR, "GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(RegisterSize == FEXCore::IR::OpSize::i256Bit || RegisterSize == FEXCore::IR::OpSize::i128Bit || RegisterSize == FEXCore::IR::OpSize::i64Bit, "RegisterSize == FEXCore::IR::OpSize::i256Bit || RegisterSize == FEXCore::IR::OpSize::i128Bit || RegisterSize == FEXCore::IR::OpSize::i64Bit");
#endif
		return _VOr(RegisterSize, ElementSize, Vector1->Wrapped(ListDataBegin), Vector2->Wrapped(ListDataBegin));
	}

	IRPair<IROp_VXor> _VXor(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Vector1, OrderedNodeWrapper Vector2) {
		auto _Op = AllocateOp<IROp_VXor, IROps::OP_VXOR>();
		_Op.first->Vector1 = Vector1;
		_Op.first->Vector2 = Vector2;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	IRPair<IROp_VXor> _VXor(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Vector1, OrderedNode *Vector2) {
		auto ListDataBegin = DualListData.ListBegin();
		Vector1->AddUse();
		Vector2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR, "GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR, "GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(RegisterSize == FEXCore::IR::OpSize::i256Bit || RegisterSize == FEXCore::IR::OpSize::i128Bit || RegisterSize == FEXCore::IR::OpSize::i64Bit, "RegisterSize == FEXCore::IR::OpSize::i256Bit || RegisterSize == FEXCore::IR::OpSize::i128Bit || RegisterSize == FEXCore::IR::OpSize::i64Bit");
#endif
		return _VXor(RegisterSize, ElementSize, Vector1->Wrapped(ListDataBegin), Vector2->Wrapped(ListDataBegin));
	}

	IRPair<IROp_VUQAdd> _VUQAdd(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Vector1, OrderedNodeWrapper Vector2) {
		auto _Op = AllocateOp<IROp_VUQAdd, IROps::OP_VUQADD>();
		_Op.first->Vector1 = Vector1;
		_Op.first->Vector2 = Vector2;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	IRPair<IROp_VUQAdd> _VUQAdd(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Vector1, OrderedNode *Vector2) {
		auto ListDataBegin = DualListData.ListBegin();
		Vector1->AddUse();
		Vector2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR, "GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR, "GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR");
#endif
		return _VUQAdd(RegisterSize, ElementSize, Vector1->Wrapped(ListDataBegin), Vector2->Wrapped(ListDataBegin));
	}

	IRPair<IROp_VUQSub> _VUQSub(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Vector1, OrderedNodeWrapper Vector2) {
		auto _Op = AllocateOp<IROp_VUQSub, IROps::OP_VUQSUB>();
		_Op.first->Vector1 = Vector1;
		_Op.first->Vector2 = Vector2;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	IRPair<IROp_VUQSub> _VUQSub(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Vector1, OrderedNode *Vector2) {
		auto ListDataBegin = DualListData.ListBegin();
		Vector1->AddUse();
		Vector2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR, "GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR, "GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR");
#endif
		return _VUQSub(RegisterSize, ElementSize, Vector1->Wrapped(ListDataBegin), Vector2->Wrapped(ListDataBegin));
	}

	IRPair<IROp_VSQAdd> _VSQAdd(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Vector1, OrderedNodeWrapper Vector2) {
		auto _Op = AllocateOp<IROp_VSQAdd, IROps::OP_VSQADD>();
		_Op.first->Vector1 = Vector1;
		_Op.first->Vector2 = Vector2;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	IRPair<IROp_VSQAdd> _VSQAdd(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Vector1, OrderedNode *Vector2) {
		auto ListDataBegin = DualListData.ListBegin();
		Vector1->AddUse();
		Vector2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR, "GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR, "GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR");
#endif
		return _VSQAdd(RegisterSize, ElementSize, Vector1->Wrapped(ListDataBegin), Vector2->Wrapped(ListDataBegin));
	}

	IRPair<IROp_VSQSub> _VSQSub(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Vector1, OrderedNodeWrapper Vector2) {
		auto _Op = AllocateOp<IROp_VSQSub, IROps::OP_VSQSUB>();
		_Op.first->Vector1 = Vector1;
		_Op.first->Vector2 = Vector2;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	IRPair<IROp_VSQSub> _VSQSub(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Vector1, OrderedNode *Vector2) {
		auto ListDataBegin = DualListData.ListBegin();
		Vector1->AddUse();
		Vector2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR, "GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR, "GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR");
#endif
		return _VSQSub(RegisterSize, ElementSize, Vector1->Wrapped(ListDataBegin), Vector2->Wrapped(ListDataBegin));
	}

	/// D
	///
	/// o
	///
	/// e
	///
	/// s
	///
	///  
	///
	/// a
	///
	///  
	///
	/// h
	///
	/// o
	///
	/// r
	///
	/// i
	///
	/// z
	///
	/// o
	///
	/// n
	///
	/// t
	///
	/// a
	///
	/// l
	///
	///  
	///
	/// p
	///
	/// a
	///
	/// i
	///
	/// r
	///
	/// w
	///
	/// i
	///
	/// s
	///
	/// e
	///
	///  
	///
	/// a
	///
	/// d
	///
	/// d
	///
	///  
	///
	/// o
	///
	/// f
	///
	///  
	///
	/// e
	///
	/// l
	///
	/// e
	///
	/// m
	///
	/// e
	///
	/// n
	///
	/// t
	///
	/// s
	///
	///  
	///
	/// a
	///
	/// c
	///
	/// r
	///
	/// o
	///
	/// s
	///
	/// s
	///
	///  
	///
	/// t
	///
	/// h
	///
	/// e
	///
	///  
	///
	/// t
	///
	/// w
	///
	/// o
	///
	///  
	///
	/// s
	///
	/// o
	///
	/// u
	///
	/// r
	///
	/// c
	///
	/// e
	///
	///  
	///
	/// v
	///
	/// e
	///
	/// c
	///
	/// t
	///
	/// o
	///
	/// r
	///
	/// s
	IRPair<IROp_VAddP> _VAddP(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper VectorLower, OrderedNodeWrapper VectorUpper) {
		auto _Op = AllocateOp<IROp_VAddP, IROps::OP_VADDP>();
		_Op.first->VectorLower = VectorLower;
		_Op.first->VectorUpper = VectorUpper;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	/// D
	///
	/// o
	///
	/// e
	///
	/// s
	///
	///  
	///
	/// a
	///
	///  
	///
	/// h
	///
	/// o
	///
	/// r
	///
	/// i
	///
	/// z
	///
	/// o
	///
	/// n
	///
	/// t
	///
	/// a
	///
	/// l
	///
	///  
	///
	/// p
	///
	/// a
	///
	/// i
	///
	/// r
	///
	/// w
	///
	/// i
	///
	/// s
	///
	/// e
	///
	///  
	///
	/// a
	///
	/// d
	///
	/// d
	///
	///  
	///
	/// o
	///
	/// f
	///
	///  
	///
	/// e
	///
	/// l
	///
	/// e
	///
	/// m
	///
	/// e
	///
	/// n
	///
	/// t
	///
	/// s
	///
	///  
	///
	/// a
	///
	/// c
	///
	/// r
	///
	/// o
	///
	/// s
	///
	/// s
	///
	///  
	///
	/// t
	///
	/// h
	///
	/// e
	///
	///  
	///
	/// t
	///
	/// w
	///
	/// o
	///
	///  
	///
	/// s
	///
	/// o
	///
	/// u
	///
	/// r
	///
	/// c
	///
	/// e
	///
	///  
	///
	/// v
	///
	/// e
	///
	/// c
	///
	/// t
	///
	/// o
	///
	/// r
	///
	/// s
	IRPair<IROp_VAddP> _VAddP(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *VectorLower, OrderedNode *VectorUpper) {
		auto ListDataBegin = DualListData.ListBegin();
		VectorLower->AddUse();
		VectorUpper->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(VectorLower) == RegClass::Invalid || WalkFindRegClass(VectorLower) == RegClass::FPR, "GetOpRegClass(VectorLower) == RegClass::Invalid || WalkFindRegClass(VectorLower) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(VectorUpper) == RegClass::Invalid || WalkFindRegClass(VectorUpper) == RegClass::FPR, "GetOpRegClass(VectorUpper) == RegClass::Invalid || WalkFindRegClass(VectorUpper) == RegClass::FPR");
#endif
		return _VAddP(RegisterSize, ElementSize, VectorLower->Wrapped(ListDataBegin), VectorUpper->Wrapped(ListDataBegin));
	}

	/// Does an unsigned rounded average
	///
	/// dst_elem = (src1_elem + src2_elem + 1) >> 1
	IRPair<IROp_VURAvg> _VURAvg(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Vector1, OrderedNodeWrapper Vector2) {
		auto _Op = AllocateOp<IROp_VURAvg, IROps::OP_VURAVG>();
		_Op.first->Vector1 = Vector1;
		_Op.first->Vector2 = Vector2;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	/// Does an unsigned rounded average
	///
	/// dst_elem = (src1_elem + src2_elem + 1) >> 1
	IRPair<IROp_VURAvg> _VURAvg(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Vector1, OrderedNode *Vector2) {
		auto ListDataBegin = DualListData.ListBegin();
		Vector1->AddUse();
		Vector2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR, "GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR, "GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR");
#endif
		return _VURAvg(RegisterSize, ElementSize, Vector1->Wrapped(ListDataBegin), Vector2->Wrapped(ListDataBegin));
	}

	IRPair<IROp_VUMin> _VUMin(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Vector1, OrderedNodeWrapper Vector2) {
		auto _Op = AllocateOp<IROp_VUMin, IROps::OP_VUMIN>();
		_Op.first->Vector1 = Vector1;
		_Op.first->Vector2 = Vector2;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	IRPair<IROp_VUMin> _VUMin(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Vector1, OrderedNode *Vector2) {
		auto ListDataBegin = DualListData.ListBegin();
		Vector1->AddUse();
		Vector2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR, "GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR, "GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR");
#endif
		return _VUMin(RegisterSize, ElementSize, Vector1->Wrapped(ListDataBegin), Vector2->Wrapped(ListDataBegin));
	}

	IRPair<IROp_VUMax> _VUMax(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Vector1, OrderedNodeWrapper Vector2) {
		auto _Op = AllocateOp<IROp_VUMax, IROps::OP_VUMAX>();
		_Op.first->Vector1 = Vector1;
		_Op.first->Vector2 = Vector2;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	IRPair<IROp_VUMax> _VUMax(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Vector1, OrderedNode *Vector2) {
		auto ListDataBegin = DualListData.ListBegin();
		Vector1->AddUse();
		Vector2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR, "GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR, "GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR");
#endif
		return _VUMax(RegisterSize, ElementSize, Vector1->Wrapped(ListDataBegin), Vector2->Wrapped(ListDataBegin));
	}

	IRPair<IROp_VSMin> _VSMin(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Vector1, OrderedNodeWrapper Vector2) {
		auto _Op = AllocateOp<IROp_VSMin, IROps::OP_VSMIN>();
		_Op.first->Vector1 = Vector1;
		_Op.first->Vector2 = Vector2;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	IRPair<IROp_VSMin> _VSMin(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Vector1, OrderedNode *Vector2) {
		auto ListDataBegin = DualListData.ListBegin();
		Vector1->AddUse();
		Vector2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR, "GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR, "GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR");
#endif
		return _VSMin(RegisterSize, ElementSize, Vector1->Wrapped(ListDataBegin), Vector2->Wrapped(ListDataBegin));
	}

	IRPair<IROp_VSMax> _VSMax(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Vector1, OrderedNodeWrapper Vector2) {
		auto _Op = AllocateOp<IROp_VSMax, IROps::OP_VSMAX>();
		_Op.first->Vector1 = Vector1;
		_Op.first->Vector2 = Vector2;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	IRPair<IROp_VSMax> _VSMax(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Vector1, OrderedNode *Vector2) {
		auto ListDataBegin = DualListData.ListBegin();
		Vector1->AddUse();
		Vector2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR, "GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR, "GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR");
#endif
		return _VSMax(RegisterSize, ElementSize, Vector1->Wrapped(ListDataBegin), Vector2->Wrapped(ListDataBegin));
	}

	IRPair<IROp_VZip> _VZip(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper VectorLower, OrderedNodeWrapper VectorUpper) {
		auto _Op = AllocateOp<IROp_VZip, IROps::OP_VZIP>();
		_Op.first->VectorLower = VectorLower;
		_Op.first->VectorUpper = VectorUpper;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	IRPair<IROp_VZip> _VZip(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *VectorLower, OrderedNode *VectorUpper) {
		auto ListDataBegin = DualListData.ListBegin();
		VectorLower->AddUse();
		VectorUpper->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(VectorLower) == RegClass::Invalid || WalkFindRegClass(VectorLower) == RegClass::FPR, "GetOpRegClass(VectorLower) == RegClass::Invalid || WalkFindRegClass(VectorLower) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(VectorUpper) == RegClass::Invalid || WalkFindRegClass(VectorUpper) == RegClass::FPR, "GetOpRegClass(VectorUpper) == RegClass::Invalid || WalkFindRegClass(VectorUpper) == RegClass::FPR");
#endif
		return _VZip(RegisterSize, ElementSize, VectorLower->Wrapped(ListDataBegin), VectorUpper->Wrapped(ListDataBegin));
	}

	IRPair<IROp_VZip2> _VZip2(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper VectorLower, OrderedNodeWrapper VectorUpper) {
		auto _Op = AllocateOp<IROp_VZip2, IROps::OP_VZIP2>();
		_Op.first->VectorLower = VectorLower;
		_Op.first->VectorUpper = VectorUpper;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	IRPair<IROp_VZip2> _VZip2(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *VectorLower, OrderedNode *VectorUpper) {
		auto ListDataBegin = DualListData.ListBegin();
		VectorLower->AddUse();
		VectorUpper->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(VectorLower) == RegClass::Invalid || WalkFindRegClass(VectorLower) == RegClass::FPR, "GetOpRegClass(VectorLower) == RegClass::Invalid || WalkFindRegClass(VectorLower) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(VectorUpper) == RegClass::Invalid || WalkFindRegClass(VectorUpper) == RegClass::FPR, "GetOpRegClass(VectorUpper) == RegClass::Invalid || WalkFindRegClass(VectorUpper) == RegClass::FPR");
#endif
		return _VZip2(RegisterSize, ElementSize, VectorLower->Wrapped(ListDataBegin), VectorUpper->Wrapped(ListDataBegin));
	}

	IRPair<IROp_VUnZip> _VUnZip(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper VectorLower, OrderedNodeWrapper VectorUpper) {
		auto _Op = AllocateOp<IROp_VUnZip, IROps::OP_VUNZIP>();
		_Op.first->VectorLower = VectorLower;
		_Op.first->VectorUpper = VectorUpper;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	IRPair<IROp_VUnZip> _VUnZip(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *VectorLower, OrderedNode *VectorUpper) {
		auto ListDataBegin = DualListData.ListBegin();
		VectorLower->AddUse();
		VectorUpper->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(VectorLower) == RegClass::Invalid || WalkFindRegClass(VectorLower) == RegClass::FPR, "GetOpRegClass(VectorLower) == RegClass::Invalid || WalkFindRegClass(VectorLower) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(VectorUpper) == RegClass::Invalid || WalkFindRegClass(VectorUpper) == RegClass::FPR, "GetOpRegClass(VectorUpper) == RegClass::Invalid || WalkFindRegClass(VectorUpper) == RegClass::FPR");
#endif
		return _VUnZip(RegisterSize, ElementSize, VectorLower->Wrapped(ListDataBegin), VectorUpper->Wrapped(ListDataBegin));
	}

	IRPair<IROp_VUnZip2> _VUnZip2(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper VectorLower, OrderedNodeWrapper VectorUpper) {
		auto _Op = AllocateOp<IROp_VUnZip2, IROps::OP_VUNZIP2>();
		_Op.first->VectorLower = VectorLower;
		_Op.first->VectorUpper = VectorUpper;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	IRPair<IROp_VUnZip2> _VUnZip2(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *VectorLower, OrderedNode *VectorUpper) {
		auto ListDataBegin = DualListData.ListBegin();
		VectorLower->AddUse();
		VectorUpper->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(VectorLower) == RegClass::Invalid || WalkFindRegClass(VectorLower) == RegClass::FPR, "GetOpRegClass(VectorLower) == RegClass::Invalid || WalkFindRegClass(VectorLower) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(VectorUpper) == RegClass::Invalid || WalkFindRegClass(VectorUpper) == RegClass::FPR, "GetOpRegClass(VectorUpper) == RegClass::Invalid || WalkFindRegClass(VectorUpper) == RegClass::FPR");
#endif
		return _VUnZip2(RegisterSize, ElementSize, VectorLower->Wrapped(ListDataBegin), VectorUpper->Wrapped(ListDataBegin));
	}

	IRPair<IROp_VTrn> _VTrn(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper VectorLower, OrderedNodeWrapper VectorUpper) {
		auto _Op = AllocateOp<IROp_VTrn, IROps::OP_VTRN>();
		_Op.first->VectorLower = VectorLower;
		_Op.first->VectorUpper = VectorUpper;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	IRPair<IROp_VTrn> _VTrn(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *VectorLower, OrderedNode *VectorUpper) {
		auto ListDataBegin = DualListData.ListBegin();
		VectorLower->AddUse();
		VectorUpper->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(VectorLower) == RegClass::Invalid || WalkFindRegClass(VectorLower) == RegClass::FPR, "GetOpRegClass(VectorLower) == RegClass::Invalid || WalkFindRegClass(VectorLower) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(VectorUpper) == RegClass::Invalid || WalkFindRegClass(VectorUpper) == RegClass::FPR, "GetOpRegClass(VectorUpper) == RegClass::Invalid || WalkFindRegClass(VectorUpper) == RegClass::FPR");
#endif
		return _VTrn(RegisterSize, ElementSize, VectorLower->Wrapped(ListDataBegin), VectorUpper->Wrapped(ListDataBegin));
	}

	IRPair<IROp_VTrn2> _VTrn2(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper VectorLower, OrderedNodeWrapper VectorUpper) {
		auto _Op = AllocateOp<IROp_VTrn2, IROps::OP_VTRN2>();
		_Op.first->VectorLower = VectorLower;
		_Op.first->VectorUpper = VectorUpper;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	IRPair<IROp_VTrn2> _VTrn2(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *VectorLower, OrderedNode *VectorUpper) {
		auto ListDataBegin = DualListData.ListBegin();
		VectorLower->AddUse();
		VectorUpper->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(VectorLower) == RegClass::Invalid || WalkFindRegClass(VectorLower) == RegClass::FPR, "GetOpRegClass(VectorLower) == RegClass::Invalid || WalkFindRegClass(VectorLower) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(VectorUpper) == RegClass::Invalid || WalkFindRegClass(VectorUpper) == RegClass::FPR, "GetOpRegClass(VectorUpper) == RegClass::Invalid || WalkFindRegClass(VectorUpper) == RegClass::FPR");
#endif
		return _VTrn2(RegisterSize, ElementSize, VectorLower->Wrapped(ListDataBegin), VectorUpper->Wrapped(ListDataBegin));
	}

	IRPair<IROp_VFAdd> _VFAdd(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Vector1, OrderedNodeWrapper Vector2) {
		auto _Op = AllocateOp<IROp_VFAdd, IROps::OP_VFADD>();
		_Op.first->Vector1 = Vector1;
		_Op.first->Vector2 = Vector2;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	IRPair<IROp_VFAdd> _VFAdd(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Vector1, OrderedNode *Vector2) {
		auto ListDataBegin = DualListData.ListBegin();
		Vector1->AddUse();
		Vector2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR, "GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR, "GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR");
#endif
		return _VFAdd(RegisterSize, ElementSize, Vector1->Wrapped(ListDataBegin), Vector2->Wrapped(ListDataBegin));
	}

	/// D
	///
	/// o
	///
	/// e
	///
	/// s
	///
	///  
	///
	/// a
	///
	///  
	///
	/// h
	///
	/// o
	///
	/// r
	///
	/// i
	///
	/// z
	///
	/// o
	///
	/// n
	///
	/// t
	///
	/// a
	///
	/// l
	///
	///  
	///
	/// p
	///
	/// a
	///
	/// i
	///
	/// r
	///
	/// w
	///
	/// i
	///
	/// s
	///
	/// e
	///
	///  
	///
	/// a
	///
	/// d
	///
	/// d
	///
	///  
	///
	/// o
	///
	/// f
	///
	///  
	///
	/// e
	///
	/// l
	///
	/// e
	///
	/// m
	///
	/// e
	///
	/// n
	///
	/// t
	///
	/// s
	///
	///  
	///
	/// a
	///
	/// c
	///
	/// r
	///
	/// o
	///
	/// s
	///
	/// s
	///
	///  
	///
	/// t
	///
	/// h
	///
	/// e
	///
	///  
	///
	/// t
	///
	/// w
	///
	/// o
	///
	///  
	///
	/// s
	///
	/// o
	///
	/// u
	///
	/// r
	///
	/// c
	///
	/// e
	///
	///  
	///
	/// v
	///
	/// e
	///
	/// c
	///
	/// t
	///
	/// o
	///
	/// r
	///
	/// s
	///
	///  
	///
	/// w
	///
	/// i
	///
	/// t
	///
	/// h
	///
	///  
	///
	/// f
	///
	/// l
	///
	/// o
	///
	/// a
	///
	/// t
	///
	///  
	///
	/// e
	///
	/// l
	///
	/// e
	///
	/// m
	///
	/// e
	///
	/// n
	///
	/// t
	///
	///  
	///
	/// t
	///
	/// y
	///
	/// p
	///
	/// e
	///
	/// s
	IRPair<IROp_VFAddP> _VFAddP(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper VectorLower, OrderedNodeWrapper VectorUpper) {
		auto _Op = AllocateOp<IROp_VFAddP, IROps::OP_VFADDP>();
		_Op.first->VectorLower = VectorLower;
		_Op.first->VectorUpper = VectorUpper;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	/// D
	///
	/// o
	///
	/// e
	///
	/// s
	///
	///  
	///
	/// a
	///
	///  
	///
	/// h
	///
	/// o
	///
	/// r
	///
	/// i
	///
	/// z
	///
	/// o
	///
	/// n
	///
	/// t
	///
	/// a
	///
	/// l
	///
	///  
	///
	/// p
	///
	/// a
	///
	/// i
	///
	/// r
	///
	/// w
	///
	/// i
	///
	/// s
	///
	/// e
	///
	///  
	///
	/// a
	///
	/// d
	///
	/// d
	///
	///  
	///
	/// o
	///
	/// f
	///
	///  
	///
	/// e
	///
	/// l
	///
	/// e
	///
	/// m
	///
	/// e
	///
	/// n
	///
	/// t
	///
	/// s
	///
	///  
	///
	/// a
	///
	/// c
	///
	/// r
	///
	/// o
	///
	/// s
	///
	/// s
	///
	///  
	///
	/// t
	///
	/// h
	///
	/// e
	///
	///  
	///
	/// t
	///
	/// w
	///
	/// o
	///
	///  
	///
	/// s
	///
	/// o
	///
	/// u
	///
	/// r
	///
	/// c
	///
	/// e
	///
	///  
	///
	/// v
	///
	/// e
	///
	/// c
	///
	/// t
	///
	/// o
	///
	/// r
	///
	/// s
	///
	///  
	///
	/// w
	///
	/// i
	///
	/// t
	///
	/// h
	///
	///  
	///
	/// f
	///
	/// l
	///
	/// o
	///
	/// a
	///
	/// t
	///
	///  
	///
	/// e
	///
	/// l
	///
	/// e
	///
	/// m
	///
	/// e
	///
	/// n
	///
	/// t
	///
	///  
	///
	/// t
	///
	/// y
	///
	/// p
	///
	/// e
	///
	/// s
	IRPair<IROp_VFAddP> _VFAddP(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *VectorLower, OrderedNode *VectorUpper) {
		auto ListDataBegin = DualListData.ListBegin();
		VectorLower->AddUse();
		VectorUpper->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(VectorLower) == RegClass::Invalid || WalkFindRegClass(VectorLower) == RegClass::FPR, "GetOpRegClass(VectorLower) == RegClass::Invalid || WalkFindRegClass(VectorLower) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(VectorUpper) == RegClass::Invalid || WalkFindRegClass(VectorUpper) == RegClass::FPR, "GetOpRegClass(VectorUpper) == RegClass::Invalid || WalkFindRegClass(VectorUpper) == RegClass::FPR");
#endif
		return _VFAddP(RegisterSize, ElementSize, VectorLower->Wrapped(ListDataBegin), VectorUpper->Wrapped(ListDataBegin));
	}

	/// Does a horizontal float vector add of elements across the source vector
	///
	/// Result is a zero extended scalar
	IRPair<IROp_VFAddV> _VFAddV(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Vector) {
		auto _Op = AllocateOp<IROp_VFAddV, IROps::OP_VFADDV>();
		_Op.first->Vector = Vector;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	/// Does a horizontal float vector add of elements across the source vector
	///
	/// Result is a zero extended scalar
	IRPair<IROp_VFAddV> _VFAddV(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Vector) {
		auto ListDataBegin = DualListData.ListBegin();
		Vector->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector) == RegClass::Invalid || WalkFindRegClass(Vector) == RegClass::FPR, "GetOpRegClass(Vector) == RegClass::Invalid || WalkFindRegClass(Vector) == RegClass::FPR");
#endif
		return _VFAddV(RegisterSize, ElementSize, Vector->Wrapped(ListDataBegin));
	}

	IRPair<IROp_VFSub> _VFSub(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Vector1, OrderedNodeWrapper Vector2) {
		auto _Op = AllocateOp<IROp_VFSub, IROps::OP_VFSUB>();
		_Op.first->Vector1 = Vector1;
		_Op.first->Vector2 = Vector2;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	IRPair<IROp_VFSub> _VFSub(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Vector1, OrderedNode *Vector2) {
		auto ListDataBegin = DualListData.ListBegin();
		Vector1->AddUse();
		Vector2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR, "GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR, "GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR");
#endif
		return _VFSub(RegisterSize, ElementSize, Vector1->Wrapped(ListDataBegin), Vector2->Wrapped(ListDataBegin));
	}

	IRPair<IROp_VFMul> _VFMul(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Vector1, OrderedNodeWrapper Vector2) {
		auto _Op = AllocateOp<IROp_VFMul, IROps::OP_VFMUL>();
		_Op.first->Vector1 = Vector1;
		_Op.first->Vector2 = Vector2;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	IRPair<IROp_VFMul> _VFMul(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Vector1, OrderedNode *Vector2) {
		auto ListDataBegin = DualListData.ListBegin();
		Vector1->AddUse();
		Vector2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR, "GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR, "GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR");
#endif
		return _VFMul(RegisterSize, ElementSize, Vector1->Wrapped(ListDataBegin), Vector2->Wrapped(ListDataBegin));
	}

	IRPair<IROp_VFDiv> _VFDiv(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Vector1, OrderedNodeWrapper Vector2) {
		auto _Op = AllocateOp<IROp_VFDiv, IROps::OP_VFDIV>();
		_Op.first->Vector1 = Vector1;
		_Op.first->Vector2 = Vector2;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	IRPair<IROp_VFDiv> _VFDiv(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Vector1, OrderedNode *Vector2) {
		auto ListDataBegin = DualListData.ListBegin();
		Vector1->AddUse();
		Vector2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR, "GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR, "GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR");
#endif
		return _VFDiv(RegisterSize, ElementSize, Vector1->Wrapped(ListDataBegin), Vector2->Wrapped(ListDataBegin));
	}

	IRPair<IROp_VFMin> _VFMin(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Vector1, OrderedNodeWrapper Vector2) {
		auto _Op = AllocateOp<IROp_VFMin, IROps::OP_VFMIN>();
		_Op.first->Vector1 = Vector1;
		_Op.first->Vector2 = Vector2;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	IRPair<IROp_VFMin> _VFMin(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Vector1, OrderedNode *Vector2) {
		auto ListDataBegin = DualListData.ListBegin();
		Vector1->AddUse();
		Vector2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR, "GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR, "GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR");
#endif
		return _VFMin(RegisterSize, ElementSize, Vector1->Wrapped(ListDataBegin), Vector2->Wrapped(ListDataBegin));
	}

	IRPair<IROp_VFMax> _VFMax(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Vector1, OrderedNodeWrapper Vector2) {
		auto _Op = AllocateOp<IROp_VFMax, IROps::OP_VFMAX>();
		_Op.first->Vector1 = Vector1;
		_Op.first->Vector2 = Vector2;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	IRPair<IROp_VFMax> _VFMax(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Vector1, OrderedNode *Vector2) {
		auto ListDataBegin = DualListData.ListBegin();
		Vector1->AddUse();
		Vector2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR, "GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR, "GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR");
#endif
		return _VFMax(RegisterSize, ElementSize, Vector1->Wrapped(ListDataBegin), Vector2->Wrapped(ListDataBegin));
	}

	IRPair<IROp_VMul> _VMul(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Vector1, OrderedNodeWrapper Vector2) {
		auto _Op = AllocateOp<IROp_VMul, IROps::OP_VMUL>();
		_Op.first->Vector1 = Vector1;
		_Op.first->Vector2 = Vector2;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	IRPair<IROp_VMul> _VMul(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Vector1, OrderedNode *Vector2) {
		auto ListDataBegin = DualListData.ListBegin();
		Vector1->AddUse();
		Vector2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR, "GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR, "GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR");
#endif
		return _VMul(RegisterSize, ElementSize, Vector1->Wrapped(ListDataBegin), Vector2->Wrapped(ListDataBegin));
	}

	IRPair<IROp_VUMull> _VUMull(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Vector1, OrderedNodeWrapper Vector2) {
		auto _Op = AllocateOp<IROp_VUMull, IROps::OP_VUMULL>();
		_Op.first->Vector1 = Vector1;
		_Op.first->Vector2 = Vector2;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize << 1;
		return _Op;
	}

	IRPair<IROp_VUMull> _VUMull(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Vector1, OrderedNode *Vector2) {
		auto ListDataBegin = DualListData.ListBegin();
		Vector1->AddUse();
		Vector2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR, "GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR, "GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR");
#endif
		return _VUMull(RegisterSize, ElementSize, Vector1->Wrapped(ListDataBegin), Vector2->Wrapped(ListDataBegin));
	}

	/// Does a signed integer multiply with extend.
	///
	/// ElementSize is the source size
	IRPair<IROp_VSMull> _VSMull(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Vector1, OrderedNodeWrapper Vector2) {
		auto _Op = AllocateOp<IROp_VSMull, IROps::OP_VSMULL>();
		_Op.first->Vector1 = Vector1;
		_Op.first->Vector2 = Vector2;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize << 1;
		return _Op;
	}

	/// Does a signed integer multiply with extend.
	///
	/// ElementSize is the source size
	IRPair<IROp_VSMull> _VSMull(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Vector1, OrderedNode *Vector2) {
		auto ListDataBegin = DualListData.ListBegin();
		Vector1->AddUse();
		Vector2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR, "GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR, "GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR");
#endif
		return _VSMull(RegisterSize, ElementSize, Vector1->Wrapped(ListDataBegin), Vector2->Wrapped(ListDataBegin));
	}

	/// M
	///
	/// u
	///
	/// l
	///
	/// t
	///
	/// i
	///
	/// p
	///
	/// l
	///
	/// i
	///
	/// e
	///
	/// s
	///
	///  
	///
	/// t
	///
	/// h
	///
	/// e
	///
	///  
	///
	/// h
	///
	/// i
	///
	/// g
	///
	/// h
	///
	///  
	///
	/// e
	///
	/// l
	///
	/// e
	///
	/// m
	///
	/// e
	///
	/// n
	///
	/// t
	///
	/// s
	///
	///  
	///
	/// w
	///
	/// i
	///
	/// t
	///
	/// h
	///
	///  
	///
	/// s
	///
	/// i
	///
	/// z
	///
	/// e
	///
	///  
	///
	/// e
	///
	/// x
	///
	/// t
	///
	/// e
	///
	/// n
	///
	/// s
	///
	/// i
	///
	/// o
	///
	/// n
	IRPair<IROp_VUMull2> _VUMull2(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Vector1, OrderedNodeWrapper Vector2) {
		auto _Op = AllocateOp<IROp_VUMull2, IROps::OP_VUMULL2>();
		_Op.first->Vector1 = Vector1;
		_Op.first->Vector2 = Vector2;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize << 1;
		return _Op;
	}

	/// M
	///
	/// u
	///
	/// l
	///
	/// t
	///
	/// i
	///
	/// p
	///
	/// l
	///
	/// i
	///
	/// e
	///
	/// s
	///
	///  
	///
	/// t
	///
	/// h
	///
	/// e
	///
	///  
	///
	/// h
	///
	/// i
	///
	/// g
	///
	/// h
	///
	///  
	///
	/// e
	///
	/// l
	///
	/// e
	///
	/// m
	///
	/// e
	///
	/// n
	///
	/// t
	///
	/// s
	///
	///  
	///
	/// w
	///
	/// i
	///
	/// t
	///
	/// h
	///
	///  
	///
	/// s
	///
	/// i
	///
	/// z
	///
	/// e
	///
	///  
	///
	/// e
	///
	/// x
	///
	/// t
	///
	/// e
	///
	/// n
	///
	/// s
	///
	/// i
	///
	/// o
	///
	/// n
	IRPair<IROp_VUMull2> _VUMull2(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Vector1, OrderedNode *Vector2) {
		auto ListDataBegin = DualListData.ListBegin();
		Vector1->AddUse();
		Vector2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR, "GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR, "GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR");
#endif
		return _VUMull2(RegisterSize, ElementSize, Vector1->Wrapped(ListDataBegin), Vector2->Wrapped(ListDataBegin));
	}

	/// M
	///
	/// u
	///
	/// l
	///
	/// t
	///
	/// i
	///
	/// p
	///
	/// l
	///
	/// i
	///
	/// e
	///
	/// s
	///
	///  
	///
	/// t
	///
	/// h
	///
	/// e
	///
	///  
	///
	/// h
	///
	/// i
	///
	/// g
	///
	/// h
	///
	///  
	///
	/// e
	///
	/// l
	///
	/// e
	///
	/// m
	///
	/// e
	///
	/// n
	///
	/// t
	///
	/// s
	///
	///  
	///
	/// w
	///
	/// i
	///
	/// t
	///
	/// h
	///
	///  
	///
	/// s
	///
	/// i
	///
	/// z
	///
	/// e
	///
	///  
	///
	/// e
	///
	/// x
	///
	/// t
	///
	/// e
	///
	/// n
	///
	/// s
	///
	/// i
	///
	/// o
	///
	/// n
	IRPair<IROp_VSMull2> _VSMull2(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Vector1, OrderedNodeWrapper Vector2) {
		auto _Op = AllocateOp<IROp_VSMull2, IROps::OP_VSMULL2>();
		_Op.first->Vector1 = Vector1;
		_Op.first->Vector2 = Vector2;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize << 1;
		return _Op;
	}

	/// M
	///
	/// u
	///
	/// l
	///
	/// t
	///
	/// i
	///
	/// p
	///
	/// l
	///
	/// i
	///
	/// e
	///
	/// s
	///
	///  
	///
	/// t
	///
	/// h
	///
	/// e
	///
	///  
	///
	/// h
	///
	/// i
	///
	/// g
	///
	/// h
	///
	///  
	///
	/// e
	///
	/// l
	///
	/// e
	///
	/// m
	///
	/// e
	///
	/// n
	///
	/// t
	///
	/// s
	///
	///  
	///
	/// w
	///
	/// i
	///
	/// t
	///
	/// h
	///
	///  
	///
	/// s
	///
	/// i
	///
	/// z
	///
	/// e
	///
	///  
	///
	/// e
	///
	/// x
	///
	/// t
	///
	/// e
	///
	/// n
	///
	/// s
	///
	/// i
	///
	/// o
	///
	/// n
	IRPair<IROp_VSMull2> _VSMull2(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Vector1, OrderedNode *Vector2) {
		auto ListDataBegin = DualListData.ListBegin();
		Vector1->AddUse();
		Vector2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR, "GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR, "GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR");
#endif
		return _VSMull2(RegisterSize, ElementSize, Vector1->Wrapped(ListDataBegin), Vector2->Wrapped(ListDataBegin));
	}

	/// W
	///
	/// i
	///
	/// d
	///
	/// e
	///
	///  
	///
	/// u
	///
	/// n
	///
	/// s
	///
	/// i
	///
	/// g
	///
	/// n
	///
	/// e
	///
	/// d
	///
	///  
	///
	/// m
	///
	/// u
	///
	/// l
	///
	/// t
	///
	/// i
	///
	/// p
	///
	/// l
	///
	/// y
	///
	///  
	///
	/// r
	///
	/// e
	///
	/// t
	///
	/// u
	///
	/// r
	///
	/// n
	///
	/// i
	///
	/// n
	///
	/// g
	///
	///  
	///
	/// t
	///
	/// h
	///
	/// e
	///
	///  
	///
	/// h
	///
	/// i
	///
	/// g
	///
	/// h
	///
	///  
	///
	/// r
	///
	/// e
	///
	/// s
	///
	/// u
	///
	/// l
	///
	/// t
	///
	/// s
	IRPair<IROp_VUMulH> _VUMulH(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Vector1, OrderedNodeWrapper Vector2) {
		auto _Op = AllocateOp<IROp_VUMulH, IROps::OP_VUMULH>();
		_Op.first->Vector1 = Vector1;
		_Op.first->Vector2 = Vector2;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	/// W
	///
	/// i
	///
	/// d
	///
	/// e
	///
	///  
	///
	/// u
	///
	/// n
	///
	/// s
	///
	/// i
	///
	/// g
	///
	/// n
	///
	/// e
	///
	/// d
	///
	///  
	///
	/// m
	///
	/// u
	///
	/// l
	///
	/// t
	///
	/// i
	///
	/// p
	///
	/// l
	///
	/// y
	///
	///  
	///
	/// r
	///
	/// e
	///
	/// t
	///
	/// u
	///
	/// r
	///
	/// n
	///
	/// i
	///
	/// n
	///
	/// g
	///
	///  
	///
	/// t
	///
	/// h
	///
	/// e
	///
	///  
	///
	/// h
	///
	/// i
	///
	/// g
	///
	/// h
	///
	///  
	///
	/// r
	///
	/// e
	///
	/// s
	///
	/// u
	///
	/// l
	///
	/// t
	///
	/// s
	IRPair<IROp_VUMulH> _VUMulH(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Vector1, OrderedNode *Vector2) {
		auto ListDataBegin = DualListData.ListBegin();
		Vector1->AddUse();
		Vector2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR, "GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR, "GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR");
#endif
		return _VUMulH(RegisterSize, ElementSize, Vector1->Wrapped(ListDataBegin), Vector2->Wrapped(ListDataBegin));
	}

	/// W
	///
	/// i
	///
	/// d
	///
	/// e
	///
	///  
	///
	/// s
	///
	/// i
	///
	/// g
	///
	/// n
	///
	/// e
	///
	/// d
	///
	///  
	///
	/// m
	///
	/// u
	///
	/// l
	///
	/// t
	///
	/// i
	///
	/// p
	///
	/// l
	///
	/// y
	///
	///  
	///
	/// r
	///
	/// e
	///
	/// t
	///
	/// u
	///
	/// r
	///
	/// n
	///
	/// i
	///
	/// n
	///
	/// g
	///
	///  
	///
	/// t
	///
	/// h
	///
	/// e
	///
	///  
	///
	/// h
	///
	/// i
	///
	/// g
	///
	/// h
	///
	///  
	///
	/// r
	///
	/// e
	///
	/// s
	///
	/// u
	///
	/// l
	///
	/// t
	///
	/// s
	IRPair<IROp_VSMulH> _VSMulH(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Vector1, OrderedNodeWrapper Vector2) {
		auto _Op = AllocateOp<IROp_VSMulH, IROps::OP_VSMULH>();
		_Op.first->Vector1 = Vector1;
		_Op.first->Vector2 = Vector2;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	/// W
	///
	/// i
	///
	/// d
	///
	/// e
	///
	///  
	///
	/// s
	///
	/// i
	///
	/// g
	///
	/// n
	///
	/// e
	///
	/// d
	///
	///  
	///
	/// m
	///
	/// u
	///
	/// l
	///
	/// t
	///
	/// i
	///
	/// p
	///
	/// l
	///
	/// y
	///
	///  
	///
	/// r
	///
	/// e
	///
	/// t
	///
	/// u
	///
	/// r
	///
	/// n
	///
	/// i
	///
	/// n
	///
	/// g
	///
	///  
	///
	/// t
	///
	/// h
	///
	/// e
	///
	///  
	///
	/// h
	///
	/// i
	///
	/// g
	///
	/// h
	///
	///  
	///
	/// r
	///
	/// e
	///
	/// s
	///
	/// u
	///
	/// l
	///
	/// t
	///
	/// s
	IRPair<IROp_VSMulH> _VSMulH(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Vector1, OrderedNode *Vector2) {
		auto ListDataBegin = DualListData.ListBegin();
		Vector1->AddUse();
		Vector2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR, "GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR, "GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR");
#endif
		return _VSMulH(RegisterSize, ElementSize, Vector1->Wrapped(ListDataBegin), Vector2->Wrapped(ListDataBegin));
	}

	/// Unsigned Absolute Difference Long
	IRPair<IROp_VUABDL> _VUABDL(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Vector1, OrderedNodeWrapper Vector2) {
		auto _Op = AllocateOp<IROp_VUABDL, IROps::OP_VUABDL>();
		_Op.first->Vector1 = Vector1;
		_Op.first->Vector2 = Vector2;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize << 1;
		return _Op;
	}

	/// Unsigned Absolute Difference Long
	IRPair<IROp_VUABDL> _VUABDL(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Vector1, OrderedNode *Vector2) {
		auto ListDataBegin = DualListData.ListBegin();
		Vector1->AddUse();
		Vector2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR, "GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR, "GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR");
#endif
		return _VUABDL(RegisterSize, ElementSize, Vector1->Wrapped(ListDataBegin), Vector2->Wrapped(ListDataBegin));
	}

	/// Unsigned Absolute Difference Long
	///
	/// Using the high elements of the source vectors
	IRPair<IROp_VUABDL2> _VUABDL2(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Vector1, OrderedNodeWrapper Vector2) {
		auto _Op = AllocateOp<IROp_VUABDL2, IROps::OP_VUABDL2>();
		_Op.first->Vector1 = Vector1;
		_Op.first->Vector2 = Vector2;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize << 1;
		return _Op;
	}

	/// Unsigned Absolute Difference Long
	///
	/// Using the high elements of the source vectors
	IRPair<IROp_VUABDL2> _VUABDL2(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Vector1, OrderedNode *Vector2) {
		auto ListDataBegin = DualListData.ListBegin();
		Vector1->AddUse();
		Vector2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR, "GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR, "GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR");
#endif
		return _VUABDL2(RegisterSize, ElementSize, Vector1->Wrapped(ListDataBegin), Vector2->Wrapped(ListDataBegin));
	}

	IRPair<IROp_VUShl> _VUShl(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Vector, OrderedNodeWrapper ShiftVector, bool RangeCheck) {
		auto _Op = AllocateOp<IROp_VUShl, IROps::OP_VUSHL>();
		_Op.first->Vector = Vector;
		_Op.first->ShiftVector = ShiftVector;
		_Op.first->RangeCheck = RangeCheck;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	IRPair<IROp_VUShl> _VUShl(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Vector, OrderedNode *ShiftVector, bool RangeCheck) {
		auto ListDataBegin = DualListData.ListBegin();
		Vector->AddUse();
		ShiftVector->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector) == RegClass::Invalid || WalkFindRegClass(Vector) == RegClass::FPR, "GetOpRegClass(Vector) == RegClass::Invalid || WalkFindRegClass(Vector) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(ShiftVector) == RegClass::Invalid || WalkFindRegClass(ShiftVector) == RegClass::FPR, "GetOpRegClass(ShiftVector) == RegClass::Invalid || WalkFindRegClass(ShiftVector) == RegClass::FPR");
#endif
		return _VUShl(RegisterSize, ElementSize, Vector->Wrapped(ListDataBegin), ShiftVector->Wrapped(ListDataBegin), RangeCheck);
	}

	IRPair<IROp_VUShr> _VUShr(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Vector, OrderedNodeWrapper ShiftVector, bool RangeCheck) {
		auto _Op = AllocateOp<IROp_VUShr, IROps::OP_VUSHR>();
		_Op.first->Vector = Vector;
		_Op.first->ShiftVector = ShiftVector;
		_Op.first->RangeCheck = RangeCheck;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	IRPair<IROp_VUShr> _VUShr(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Vector, OrderedNode *ShiftVector, bool RangeCheck) {
		auto ListDataBegin = DualListData.ListBegin();
		Vector->AddUse();
		ShiftVector->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector) == RegClass::Invalid || WalkFindRegClass(Vector) == RegClass::FPR, "GetOpRegClass(Vector) == RegClass::Invalid || WalkFindRegClass(Vector) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(ShiftVector) == RegClass::Invalid || WalkFindRegClass(ShiftVector) == RegClass::FPR, "GetOpRegClass(ShiftVector) == RegClass::Invalid || WalkFindRegClass(ShiftVector) == RegClass::FPR");
#endif
		return _VUShr(RegisterSize, ElementSize, Vector->Wrapped(ListDataBegin), ShiftVector->Wrapped(ListDataBegin), RangeCheck);
	}

	IRPair<IROp_VSShr> _VSShr(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Vector, OrderedNodeWrapper ShiftVector, bool RangeCheck) {
		auto _Op = AllocateOp<IROp_VSShr, IROps::OP_VSSHR>();
		_Op.first->Vector = Vector;
		_Op.first->ShiftVector = ShiftVector;
		_Op.first->RangeCheck = RangeCheck;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	IRPair<IROp_VSShr> _VSShr(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Vector, OrderedNode *ShiftVector, bool RangeCheck) {
		auto ListDataBegin = DualListData.ListBegin();
		Vector->AddUse();
		ShiftVector->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector) == RegClass::Invalid || WalkFindRegClass(Vector) == RegClass::FPR, "GetOpRegClass(Vector) == RegClass::Invalid || WalkFindRegClass(Vector) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(ShiftVector) == RegClass::Invalid || WalkFindRegClass(ShiftVector) == RegClass::FPR, "GetOpRegClass(ShiftVector) == RegClass::Invalid || WalkFindRegClass(ShiftVector) == RegClass::FPR");
#endif
		return _VSShr(RegisterSize, ElementSize, Vector->Wrapped(ListDataBegin), ShiftVector->Wrapped(ListDataBegin), RangeCheck);
	}

	IRPair<IROp_VUShlS> _VUShlS(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Vector, OrderedNodeWrapper ShiftScalar) {
		auto _Op = AllocateOp<IROp_VUShlS, IROps::OP_VUSHLS>();
		_Op.first->Vector = Vector;
		_Op.first->ShiftScalar = ShiftScalar;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	IRPair<IROp_VUShlS> _VUShlS(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Vector, OrderedNode *ShiftScalar) {
		auto ListDataBegin = DualListData.ListBegin();
		Vector->AddUse();
		ShiftScalar->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector) == RegClass::Invalid || WalkFindRegClass(Vector) == RegClass::FPR, "GetOpRegClass(Vector) == RegClass::Invalid || WalkFindRegClass(Vector) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(ShiftScalar) == RegClass::Invalid || WalkFindRegClass(ShiftScalar) == RegClass::FPR, "GetOpRegClass(ShiftScalar) == RegClass::Invalid || WalkFindRegClass(ShiftScalar) == RegClass::FPR");
#endif
		return _VUShlS(RegisterSize, ElementSize, Vector->Wrapped(ListDataBegin), ShiftScalar->Wrapped(ListDataBegin));
	}

	IRPair<IROp_VUShrS> _VUShrS(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Vector, OrderedNodeWrapper ShiftScalar) {
		auto _Op = AllocateOp<IROp_VUShrS, IROps::OP_VUSHRS>();
		_Op.first->Vector = Vector;
		_Op.first->ShiftScalar = ShiftScalar;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	IRPair<IROp_VUShrS> _VUShrS(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Vector, OrderedNode *ShiftScalar) {
		auto ListDataBegin = DualListData.ListBegin();
		Vector->AddUse();
		ShiftScalar->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector) == RegClass::Invalid || WalkFindRegClass(Vector) == RegClass::FPR, "GetOpRegClass(Vector) == RegClass::Invalid || WalkFindRegClass(Vector) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(ShiftScalar) == RegClass::Invalid || WalkFindRegClass(ShiftScalar) == RegClass::FPR, "GetOpRegClass(ShiftScalar) == RegClass::Invalid || WalkFindRegClass(ShiftScalar) == RegClass::FPR");
#endif
		return _VUShrS(RegisterSize, ElementSize, Vector->Wrapped(ListDataBegin), ShiftScalar->Wrapped(ListDataBegin));
	}

	IRPair<IROp_VSShrS> _VSShrS(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Vector, OrderedNodeWrapper ShiftScalar) {
		auto _Op = AllocateOp<IROp_VSShrS, IROps::OP_VSSHRS>();
		_Op.first->Vector = Vector;
		_Op.first->ShiftScalar = ShiftScalar;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	IRPair<IROp_VSShrS> _VSShrS(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Vector, OrderedNode *ShiftScalar) {
		auto ListDataBegin = DualListData.ListBegin();
		Vector->AddUse();
		ShiftScalar->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector) == RegClass::Invalid || WalkFindRegClass(Vector) == RegClass::FPR, "GetOpRegClass(Vector) == RegClass::Invalid || WalkFindRegClass(Vector) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(ShiftScalar) == RegClass::Invalid || WalkFindRegClass(ShiftScalar) == RegClass::FPR, "GetOpRegClass(ShiftScalar) == RegClass::Invalid || WalkFindRegClass(ShiftScalar) == RegClass::FPR");
#endif
		return _VSShrS(RegisterSize, ElementSize, Vector->Wrapped(ListDataBegin), ShiftScalar->Wrapped(ListDataBegin));
	}

	IRPair<IROp_VUShrSWide> _VUShrSWide(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Vector, OrderedNodeWrapper ShiftScalar) {
		auto _Op = AllocateOp<IROp_VUShrSWide, IROps::OP_VUSHRSWIDE>();
		_Op.first->Vector = Vector;
		_Op.first->ShiftScalar = ShiftScalar;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	IRPair<IROp_VUShrSWide> _VUShrSWide(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Vector, OrderedNode *ShiftScalar) {
		auto ListDataBegin = DualListData.ListBegin();
		Vector->AddUse();
		ShiftScalar->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector) == RegClass::Invalid || WalkFindRegClass(Vector) == RegClass::FPR, "GetOpRegClass(Vector) == RegClass::Invalid || WalkFindRegClass(Vector) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(ShiftScalar) == RegClass::Invalid || WalkFindRegClass(ShiftScalar) == RegClass::FPR, "GetOpRegClass(ShiftScalar) == RegClass::Invalid || WalkFindRegClass(ShiftScalar) == RegClass::FPR");
#endif
		return _VUShrSWide(RegisterSize, ElementSize, Vector->Wrapped(ListDataBegin), ShiftScalar->Wrapped(ListDataBegin));
	}

	IRPair<IROp_VSShrSWide> _VSShrSWide(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Vector, OrderedNodeWrapper ShiftScalar) {
		auto _Op = AllocateOp<IROp_VSShrSWide, IROps::OP_VSSHRSWIDE>();
		_Op.first->Vector = Vector;
		_Op.first->ShiftScalar = ShiftScalar;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	IRPair<IROp_VSShrSWide> _VSShrSWide(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Vector, OrderedNode *ShiftScalar) {
		auto ListDataBegin = DualListData.ListBegin();
		Vector->AddUse();
		ShiftScalar->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector) == RegClass::Invalid || WalkFindRegClass(Vector) == RegClass::FPR, "GetOpRegClass(Vector) == RegClass::Invalid || WalkFindRegClass(Vector) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(ShiftScalar) == RegClass::Invalid || WalkFindRegClass(ShiftScalar) == RegClass::FPR, "GetOpRegClass(ShiftScalar) == RegClass::Invalid || WalkFindRegClass(ShiftScalar) == RegClass::FPR");
#endif
		return _VSShrSWide(RegisterSize, ElementSize, Vector->Wrapped(ListDataBegin), ShiftScalar->Wrapped(ListDataBegin));
	}

	IRPair<IROp_VUShlSWide> _VUShlSWide(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Vector, OrderedNodeWrapper ShiftScalar) {
		auto _Op = AllocateOp<IROp_VUShlSWide, IROps::OP_VUSHLSWIDE>();
		_Op.first->Vector = Vector;
		_Op.first->ShiftScalar = ShiftScalar;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	IRPair<IROp_VUShlSWide> _VUShlSWide(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Vector, OrderedNode *ShiftScalar) {
		auto ListDataBegin = DualListData.ListBegin();
		Vector->AddUse();
		ShiftScalar->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector) == RegClass::Invalid || WalkFindRegClass(Vector) == RegClass::FPR, "GetOpRegClass(Vector) == RegClass::Invalid || WalkFindRegClass(Vector) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(ShiftScalar) == RegClass::Invalid || WalkFindRegClass(ShiftScalar) == RegClass::FPR, "GetOpRegClass(ShiftScalar) == RegClass::Invalid || WalkFindRegClass(ShiftScalar) == RegClass::FPR");
#endif
		return _VUShlSWide(RegisterSize, ElementSize, Vector->Wrapped(ListDataBegin), ShiftScalar->Wrapped(ListDataBegin));
	}

	IRPair<IROp_VInsElement> _VInsElement(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, uint8_t DestIdx, uint8_t SrcIdx, OrderedNodeWrapper DestVector, OrderedNodeWrapper SrcVector) {
		auto _Op = AllocateOp<IROp_VInsElement, IROps::OP_VINSELEMENT>();
		_Op.first->DestVector = DestVector;
		_Op.first->SrcVector = SrcVector;
		_Op.first->DestIdx = DestIdx;
		_Op.first->SrcIdx = SrcIdx;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	IRPair<IROp_VInsElement> _VInsElement(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, uint8_t DestIdx, uint8_t SrcIdx, OrderedNode *DestVector, OrderedNode *SrcVector) {
		auto ListDataBegin = DualListData.ListBegin();
		DestVector->AddUse();
		SrcVector->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(DestVector) == RegClass::Invalid || WalkFindRegClass(DestVector) == RegClass::FPR, "GetOpRegClass(DestVector) == RegClass::Invalid || WalkFindRegClass(DestVector) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(SrcVector) == RegClass::Invalid || WalkFindRegClass(SrcVector) == RegClass::FPR, "GetOpRegClass(SrcVector) == RegClass::Invalid || WalkFindRegClass(SrcVector) == RegClass::FPR");
#endif
		return _VInsElement(RegisterSize, ElementSize, DestIdx, SrcIdx, DestVector->Wrapped(ListDataBegin), SrcVector->Wrapped(ListDataBegin));
	}

	IRPair<IROp_VInsGPR> _VInsGPR(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, uint8_t DestIdx, OrderedNodeWrapper DestVector, OrderedNodeWrapper Src) {
		auto _Op = AllocateOp<IROp_VInsGPR, IROps::OP_VINSGPR>();
		_Op.first->DestVector = DestVector;
		_Op.first->Src = Src;
		_Op.first->DestIdx = DestIdx;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	IRPair<IROp_VInsGPR> _VInsGPR(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, uint8_t DestIdx, OrderedNode *DestVector, OrderedNode *Src) {
		auto ListDataBegin = DualListData.ListBegin();
		DestVector->AddUse();
		Src->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(DestVector) == RegClass::Invalid || WalkFindRegClass(DestVector) == RegClass::FPR, "GetOpRegClass(DestVector) == RegClass::Invalid || WalkFindRegClass(DestVector) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src) == RegClass::Invalid || WalkFindRegClass(Src) == RegClass::GPR, "GetOpRegClass(Src) == RegClass::Invalid || WalkFindRegClass(Src) == RegClass::GPR");
#endif
		return _VInsGPR(RegisterSize, ElementSize, DestIdx, DestVector->Wrapped(ListDataBegin), Src->Wrapped(ListDataBegin));
	}

	/// Concats two vector registers together and extracts a full width register from the element index
	///
	/// Index is an element index. So it is offset by ElementSize argument
	///
	/// op:
	///
	/// TmpVector <RegisterSize *2> = concat(Upper:Lower)
	///
	/// Dest = TmpVector >> (ElementSize * Index * 8); // Or can be thought of `concat(&TmpVector[Index], i128)`
	IRPair<IROp_VExtr> _VExtr(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper VectorLower, OrderedNodeWrapper VectorUpper, uint8_t Index) {
		auto _Op = AllocateOp<IROp_VExtr, IROps::OP_VEXTR>();
		_Op.first->VectorLower = VectorLower;
		_Op.first->VectorUpper = VectorUpper;
		_Op.first->Index = Index;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	/// Concats two vector registers together and extracts a full width register from the element index
	///
	/// Index is an element index. So it is offset by ElementSize argument
	///
	/// op:
	///
	/// TmpVector <RegisterSize *2> = concat(Upper:Lower)
	///
	/// Dest = TmpVector >> (ElementSize * Index * 8); // Or can be thought of `concat(&TmpVector[Index], i128)`
	IRPair<IROp_VExtr> _VExtr(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *VectorLower, OrderedNode *VectorUpper, uint8_t Index) {
		auto ListDataBegin = DualListData.ListBegin();
		VectorLower->AddUse();
		VectorUpper->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(VectorLower) == RegClass::Invalid || WalkFindRegClass(VectorLower) == RegClass::FPR, "GetOpRegClass(VectorLower) == RegClass::Invalid || WalkFindRegClass(VectorLower) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(VectorUpper) == RegClass::Invalid || WalkFindRegClass(VectorUpper) == RegClass::FPR, "GetOpRegClass(VectorUpper) == RegClass::Invalid || WalkFindRegClass(VectorUpper) == RegClass::FPR");
#endif
		return _VExtr(RegisterSize, ElementSize, VectorLower->Wrapped(ListDataBegin), VectorUpper->Wrapped(ListDataBegin), Index);
	}

	IRPair<IROp_VCMPEQ> _VCMPEQ(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Vector1, OrderedNodeWrapper Vector2) {
		auto _Op = AllocateOp<IROp_VCMPEQ, IROps::OP_VCMPEQ>();
		_Op.first->Vector1 = Vector1;
		_Op.first->Vector2 = Vector2;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	IRPair<IROp_VCMPEQ> _VCMPEQ(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Vector1, OrderedNode *Vector2) {
		auto ListDataBegin = DualListData.ListBegin();
		Vector1->AddUse();
		Vector2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR, "GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR, "GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR");
#endif
		return _VCMPEQ(RegisterSize, ElementSize, Vector1->Wrapped(ListDataBegin), Vector2->Wrapped(ListDataBegin));
	}

	/// Vector compare signed greater than
	///
	/// Each element is compared, if the result is true then the resulting element is ~0, else zero
	IRPair<IROp_VCMPGT> _VCMPGT(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Vector1, OrderedNodeWrapper Vector2) {
		auto _Op = AllocateOp<IROp_VCMPGT, IROps::OP_VCMPGT>();
		_Op.first->Vector1 = Vector1;
		_Op.first->Vector2 = Vector2;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	/// Vector compare signed greater than
	///
	/// Each element is compared, if the result is true then the resulting element is ~0, else zero
	IRPair<IROp_VCMPGT> _VCMPGT(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Vector1, OrderedNode *Vector2) {
		auto ListDataBegin = DualListData.ListBegin();
		Vector1->AddUse();
		Vector2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR, "GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR, "GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR");
#endif
		return _VCMPGT(RegisterSize, ElementSize, Vector1->Wrapped(ListDataBegin), Vector2->Wrapped(ListDataBegin));
	}

	IRPair<IROp_VFCMPEQ> _VFCMPEQ(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Vector1, OrderedNodeWrapper Vector2) {
		auto _Op = AllocateOp<IROp_VFCMPEQ, IROps::OP_VFCMPEQ>();
		_Op.first->Vector1 = Vector1;
		_Op.first->Vector2 = Vector2;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	IRPair<IROp_VFCMPEQ> _VFCMPEQ(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Vector1, OrderedNode *Vector2) {
		auto ListDataBegin = DualListData.ListBegin();
		Vector1->AddUse();
		Vector2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR, "GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR, "GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR");
#endif
		return _VFCMPEQ(RegisterSize, ElementSize, Vector1->Wrapped(ListDataBegin), Vector2->Wrapped(ListDataBegin));
	}

	IRPair<IROp_VFCMPNEQ> _VFCMPNEQ(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Vector1, OrderedNodeWrapper Vector2) {
		auto _Op = AllocateOp<IROp_VFCMPNEQ, IROps::OP_VFCMPNEQ>();
		_Op.first->Vector1 = Vector1;
		_Op.first->Vector2 = Vector2;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	IRPair<IROp_VFCMPNEQ> _VFCMPNEQ(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Vector1, OrderedNode *Vector2) {
		auto ListDataBegin = DualListData.ListBegin();
		Vector1->AddUse();
		Vector2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR, "GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR, "GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR");
#endif
		return _VFCMPNEQ(RegisterSize, ElementSize, Vector1->Wrapped(ListDataBegin), Vector2->Wrapped(ListDataBegin));
	}

	IRPair<IROp_VFCMPLT> _VFCMPLT(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Vector1, OrderedNodeWrapper Vector2) {
		auto _Op = AllocateOp<IROp_VFCMPLT, IROps::OP_VFCMPLT>();
		_Op.first->Vector1 = Vector1;
		_Op.first->Vector2 = Vector2;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	IRPair<IROp_VFCMPLT> _VFCMPLT(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Vector1, OrderedNode *Vector2) {
		auto ListDataBegin = DualListData.ListBegin();
		Vector1->AddUse();
		Vector2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR, "GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR, "GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR");
#endif
		return _VFCMPLT(RegisterSize, ElementSize, Vector1->Wrapped(ListDataBegin), Vector2->Wrapped(ListDataBegin));
	}

	IRPair<IROp_VFCMPGT> _VFCMPGT(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Vector1, OrderedNodeWrapper Vector2) {
		auto _Op = AllocateOp<IROp_VFCMPGT, IROps::OP_VFCMPGT>();
		_Op.first->Vector1 = Vector1;
		_Op.first->Vector2 = Vector2;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	IRPair<IROp_VFCMPGT> _VFCMPGT(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Vector1, OrderedNode *Vector2) {
		auto ListDataBegin = DualListData.ListBegin();
		Vector1->AddUse();
		Vector2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR, "GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR, "GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR");
#endif
		return _VFCMPGT(RegisterSize, ElementSize, Vector1->Wrapped(ListDataBegin), Vector2->Wrapped(ListDataBegin));
	}

	IRPair<IROp_VFCMPLE> _VFCMPLE(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Vector1, OrderedNodeWrapper Vector2) {
		auto _Op = AllocateOp<IROp_VFCMPLE, IROps::OP_VFCMPLE>();
		_Op.first->Vector1 = Vector1;
		_Op.first->Vector2 = Vector2;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	IRPair<IROp_VFCMPLE> _VFCMPLE(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Vector1, OrderedNode *Vector2) {
		auto ListDataBegin = DualListData.ListBegin();
		Vector1->AddUse();
		Vector2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR, "GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR, "GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR");
#endif
		return _VFCMPLE(RegisterSize, ElementSize, Vector1->Wrapped(ListDataBegin), Vector2->Wrapped(ListDataBegin));
	}

	IRPair<IROp_VFCMPORD> _VFCMPORD(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Vector1, OrderedNodeWrapper Vector2) {
		auto _Op = AllocateOp<IROp_VFCMPORD, IROps::OP_VFCMPORD>();
		_Op.first->Vector1 = Vector1;
		_Op.first->Vector2 = Vector2;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	IRPair<IROp_VFCMPORD> _VFCMPORD(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Vector1, OrderedNode *Vector2) {
		auto ListDataBegin = DualListData.ListBegin();
		Vector1->AddUse();
		Vector2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR, "GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR, "GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR");
#endif
		return _VFCMPORD(RegisterSize, ElementSize, Vector1->Wrapped(ListDataBegin), Vector2->Wrapped(ListDataBegin));
	}

	IRPair<IROp_VFCMPUNO> _VFCMPUNO(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Vector1, OrderedNodeWrapper Vector2) {
		auto _Op = AllocateOp<IROp_VFCMPUNO, IROps::OP_VFCMPUNO>();
		_Op.first->Vector1 = Vector1;
		_Op.first->Vector2 = Vector2;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	IRPair<IROp_VFCMPUNO> _VFCMPUNO(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Vector1, OrderedNode *Vector2) {
		auto ListDataBegin = DualListData.ListBegin();
		Vector1->AddUse();
		Vector2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR, "GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR, "GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR");
#endif
		return _VFCMPUNO(RegisterSize, ElementSize, Vector1->Wrapped(ListDataBegin), Vector2->Wrapped(ListDataBegin));
	}

	/// Does a vector table lookup from one register in to the destination
	///
	/// Lookup is byte sized per byte element.
	///
	/// Any index larger than what the registers provide will result in zero for that element
	///
	/// Table is always treated as a 128bit register
	///
	/// Indices matches destination size. Either 64bit or 128bit
	IRPair<IROp_VTBL1> _VTBL1(FEXCore::IR::OpSize RegisterSize, OrderedNodeWrapper VectorTable, OrderedNodeWrapper VectorIndices) {
		auto _Op = AllocateOp<IROp_VTBL1, IROps::OP_VTBL1>();
		_Op.first->VectorTable = VectorTable;
		_Op.first->VectorIndices = VectorIndices;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Does a vector table lookup from one register in to the destination
	///
	/// Lookup is byte sized per byte element.
	///
	/// Any index larger than what the registers provide will result in zero for that element
	///
	/// Table is always treated as a 128bit register
	///
	/// Indices matches destination size. Either 64bit or 128bit
	IRPair<IROp_VTBL1> _VTBL1(FEXCore::IR::OpSize RegisterSize, OrderedNode *VectorTable, OrderedNode *VectorIndices) {
		auto ListDataBegin = DualListData.ListBegin();
		VectorTable->AddUse();
		VectorIndices->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(VectorTable) == RegClass::Invalid || WalkFindRegClass(VectorTable) == RegClass::FPR, "GetOpRegClass(VectorTable) == RegClass::Invalid || WalkFindRegClass(VectorTable) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(VectorIndices) == RegClass::Invalid || WalkFindRegClass(VectorIndices) == RegClass::FPR, "GetOpRegClass(VectorIndices) == RegClass::Invalid || WalkFindRegClass(VectorIndices) == RegClass::FPR");
#endif
		return _VTBL1(RegisterSize, VectorTable->Wrapped(ListDataBegin), VectorIndices->Wrapped(ListDataBegin));
	}

	/// Does a vector table lookup from two registers in to the destination
	///
	/// Lookup is byte sized per byte element.
	///
	/// Any index larger than what the registers provide will result in zero for that element
	///
	/// Table is always treated as a two 128bit registers
	///
	/// Indices matches destination size. Either 64bit or 128bit
	///
	/// Careful about not using sequential table registers, will result in some moves if they aren't sequential.
	IRPair<IROp_VTBL2> _VTBL2(FEXCore::IR::OpSize RegisterSize, OrderedNodeWrapper VectorTable1, OrderedNodeWrapper VectorTable2, OrderedNodeWrapper VectorIndices) {
		auto _Op = AllocateOp<IROp_VTBL2, IROps::OP_VTBL2>();
		_Op.first->VectorTable1 = VectorTable1;
		_Op.first->VectorTable2 = VectorTable2;
		_Op.first->VectorIndices = VectorIndices;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Does a vector table lookup from two registers in to the destination
	///
	/// Lookup is byte sized per byte element.
	///
	/// Any index larger than what the registers provide will result in zero for that element
	///
	/// Table is always treated as a two 128bit registers
	///
	/// Indices matches destination size. Either 64bit or 128bit
	///
	/// Careful about not using sequential table registers, will result in some moves if they aren't sequential.
	IRPair<IROp_VTBL2> _VTBL2(FEXCore::IR::OpSize RegisterSize, OrderedNode *VectorTable1, OrderedNode *VectorTable2, OrderedNode *VectorIndices) {
		auto ListDataBegin = DualListData.ListBegin();
		VectorTable1->AddUse();
		VectorTable2->AddUse();
		VectorIndices->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(VectorTable1) == RegClass::Invalid || WalkFindRegClass(VectorTable1) == RegClass::FPR, "GetOpRegClass(VectorTable1) == RegClass::Invalid || WalkFindRegClass(VectorTable1) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(VectorTable2) == RegClass::Invalid || WalkFindRegClass(VectorTable2) == RegClass::FPR, "GetOpRegClass(VectorTable2) == RegClass::Invalid || WalkFindRegClass(VectorTable2) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(VectorIndices) == RegClass::Invalid || WalkFindRegClass(VectorIndices) == RegClass::FPR, "GetOpRegClass(VectorIndices) == RegClass::Invalid || WalkFindRegClass(VectorIndices) == RegClass::FPR");
#endif
		return _VTBL2(RegisterSize, VectorTable1->Wrapped(ListDataBegin), VectorTable2->Wrapped(ListDataBegin), VectorIndices->Wrapped(ListDataBegin));
	}

	/// Does a vector table lookup from one register in to the destination
	///
	/// Lookup is byte sized per byte element.
	///
	/// Any index larger than what the registers provide will result in not modifying that element
	///
	/// Table is always treated as a 128bit register
	///
	/// Indices matches destination size. Either 64bit or 128bit
	IRPair<IROp_VTBX1> _VTBX1(FEXCore::IR::OpSize RegisterSize, OrderedNodeWrapper VectorSrcDst, OrderedNodeWrapper VectorTable, OrderedNodeWrapper VectorIndices) {
		auto _Op = AllocateOp<IROp_VTBX1, IROps::OP_VTBX1>();
		_Op.first->VectorSrcDst = VectorSrcDst;
		_Op.first->VectorTable = VectorTable;
		_Op.first->VectorIndices = VectorIndices;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Does a vector table lookup from one register in to the destination
	///
	/// Lookup is byte sized per byte element.
	///
	/// Any index larger than what the registers provide will result in not modifying that element
	///
	/// Table is always treated as a 128bit register
	///
	/// Indices matches destination size. Either 64bit or 128bit
	IRPair<IROp_VTBX1> _VTBX1(FEXCore::IR::OpSize RegisterSize, OrderedNode *VectorSrcDst, OrderedNode *VectorTable, OrderedNode *VectorIndices) {
		auto ListDataBegin = DualListData.ListBegin();
		VectorSrcDst->AddUse();
		VectorTable->AddUse();
		VectorIndices->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(VectorSrcDst) == RegClass::Invalid || WalkFindRegClass(VectorSrcDst) == RegClass::FPR, "GetOpRegClass(VectorSrcDst) == RegClass::Invalid || WalkFindRegClass(VectorSrcDst) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(VectorTable) == RegClass::Invalid || WalkFindRegClass(VectorTable) == RegClass::FPR, "GetOpRegClass(VectorTable) == RegClass::Invalid || WalkFindRegClass(VectorTable) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(VectorIndices) == RegClass::Invalid || WalkFindRegClass(VectorIndices) == RegClass::FPR, "GetOpRegClass(VectorIndices) == RegClass::Invalid || WalkFindRegClass(VectorIndices) == RegClass::FPR");
#endif
		return _VTBX1(RegisterSize, VectorSrcDst->Wrapped(ListDataBegin), VectorTable->Wrapped(ListDataBegin), VectorIndices->Wrapped(ListDataBegin));
	}

	/// Does a vector bitwise select.
	///
	/// If the bit in the field is 1 then the corresponding bit is pulled from VectorTrue
	///
	/// If the bit in the field is 0 then the corresponding bit is pulled from VectorFalse
	IRPair<IROp_VBSL> _VBSL(FEXCore::IR::OpSize RegisterSize, OrderedNodeWrapper VectorMask, OrderedNodeWrapper VectorTrue, OrderedNodeWrapper VectorFalse) {
		auto _Op = AllocateOp<IROp_VBSL, IROps::OP_VBSL>();
		_Op.first->VectorMask = VectorMask;
		_Op.first->VectorTrue = VectorTrue;
		_Op.first->VectorFalse = VectorFalse;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Does a vector bitwise select.
	///
	/// If the bit in the field is 1 then the corresponding bit is pulled from VectorTrue
	///
	/// If the bit in the field is 0 then the corresponding bit is pulled from VectorFalse
	IRPair<IROp_VBSL> _VBSL(FEXCore::IR::OpSize RegisterSize, OrderedNode *VectorMask, OrderedNode *VectorTrue, OrderedNode *VectorFalse) {
		auto ListDataBegin = DualListData.ListBegin();
		VectorMask->AddUse();
		VectorTrue->AddUse();
		VectorFalse->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(VectorMask) == RegClass::Invalid || WalkFindRegClass(VectorMask) == RegClass::FPR, "GetOpRegClass(VectorMask) == RegClass::Invalid || WalkFindRegClass(VectorMask) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(VectorTrue) == RegClass::Invalid || WalkFindRegClass(VectorTrue) == RegClass::FPR, "GetOpRegClass(VectorTrue) == RegClass::Invalid || WalkFindRegClass(VectorTrue) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(VectorFalse) == RegClass::Invalid || WalkFindRegClass(VectorFalse) == RegClass::FPR, "GetOpRegClass(VectorFalse) == RegClass::Invalid || WalkFindRegClass(VectorFalse) == RegClass::FPR");
#endif
		return _VBSL(RegisterSize, VectorMask->Wrapped(ListDataBegin), VectorTrue->Wrapped(ListDataBegin), VectorFalse->Wrapped(ListDataBegin));
	}

	/// Performs intermediate behavior analogous to the x86 PCMPESTRI/PCMPESTRM instruction
	///
	/// This will return the intermediate result of a PCMPESTR-type operation, but NOT the final
	///
	/// result. This must be derived from the intermediate result
	///
	/// NOTE: On top of returning the intermediate result, the returned value also combines the status
	///
	/// flags into the upper 16-bits of the 32-bit result, as these can also be derived over the
	///
	/// course of creating the intermediate result
	IRPair<IROp_VPCMPESTRX> _VPCMPESTRX(OrderedNodeWrapper LHS, OrderedNodeWrapper RHS, OrderedNodeWrapper RAX, OrderedNodeWrapper RDX, uint16_t Control) {
		auto _Op = AllocateOp<IROp_VPCMPESTRX, IROps::OP_VPCMPESTRX>();
		_Op.first->LHS = LHS;
		_Op.first->RHS = RHS;
		_Op.first->RAX = RAX;
		_Op.first->RDX = RDX;
		_Op.first->Control = Control;
		_Op.first->Header.Size = OpSize::i32Bit;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Performs intermediate behavior analogous to the x86 PCMPESTRI/PCMPESTRM instruction
	///
	/// This will return the intermediate result of a PCMPESTR-type operation, but NOT the final
	///
	/// result. This must be derived from the intermediate result
	///
	/// NOTE: On top of returning the intermediate result, the returned value also combines the status
	///
	/// flags into the upper 16-bits of the 32-bit result, as these can also be derived over the
	///
	/// course of creating the intermediate result
	IRPair<IROp_VPCMPESTRX> _VPCMPESTRX(OrderedNode *LHS, OrderedNode *RHS, OrderedNode *RAX, OrderedNode *RDX, uint16_t Control) {
		auto ListDataBegin = DualListData.ListBegin();
		LHS->AddUse();
		RHS->AddUse();
		RAX->AddUse();
		RDX->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(LHS) == RegClass::Invalid || WalkFindRegClass(LHS) == RegClass::FPR, "GetOpRegClass(LHS) == RegClass::Invalid || WalkFindRegClass(LHS) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(RHS) == RegClass::Invalid || WalkFindRegClass(RHS) == RegClass::FPR, "GetOpRegClass(RHS) == RegClass::Invalid || WalkFindRegClass(RHS) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(RAX) == RegClass::Invalid || WalkFindRegClass(RAX) == RegClass::GPR, "GetOpRegClass(RAX) == RegClass::Invalid || WalkFindRegClass(RAX) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(RDX) == RegClass::Invalid || WalkFindRegClass(RDX) == RegClass::GPR, "GetOpRegClass(RDX) == RegClass::Invalid || WalkFindRegClass(RDX) == RegClass::GPR");
#endif
		return _VPCMPESTRX(LHS->Wrapped(ListDataBegin), RHS->Wrapped(ListDataBegin), RAX->Wrapped(ListDataBegin), RDX->Wrapped(ListDataBegin), Control);
	}

	/// Performs intermediate behavior analogous to the x86 PCMPISTRI/PCMPISTRM instruction
	///
	/// This will return the intermediate result of a PCMPISTR-type operation, but NOT the final
	///
	/// result. This must be derived from the intermediate result
	///
	/// NOTE: On top of returning the intermediate result, the returned value also combines the status
	///
	/// flags into the upper 16-bits of the 32-bit result, as these can also be derived over the
	///
	/// course of creating the intermediate result
	IRPair<IROp_VPCMPISTRX> _VPCMPISTRX(OrderedNodeWrapper LHS, OrderedNodeWrapper RHS, uint8_t Control) {
		auto _Op = AllocateOp<IROp_VPCMPISTRX, IROps::OP_VPCMPISTRX>();
		_Op.first->LHS = LHS;
		_Op.first->RHS = RHS;
		_Op.first->Control = Control;
		_Op.first->Header.Size = OpSize::i32Bit;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Performs intermediate behavior analogous to the x86 PCMPISTRI/PCMPISTRM instruction
	///
	/// This will return the intermediate result of a PCMPISTR-type operation, but NOT the final
	///
	/// result. This must be derived from the intermediate result
	///
	/// NOTE: On top of returning the intermediate result, the returned value also combines the status
	///
	/// flags into the upper 16-bits of the 32-bit result, as these can also be derived over the
	///
	/// course of creating the intermediate result
	IRPair<IROp_VPCMPISTRX> _VPCMPISTRX(OrderedNode *LHS, OrderedNode *RHS, uint8_t Control) {
		auto ListDataBegin = DualListData.ListBegin();
		LHS->AddUse();
		RHS->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(LHS) == RegClass::Invalid || WalkFindRegClass(LHS) == RegClass::FPR, "GetOpRegClass(LHS) == RegClass::Invalid || WalkFindRegClass(LHS) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(RHS) == RegClass::Invalid || WalkFindRegClass(RHS) == RegClass::FPR, "GetOpRegClass(RHS) == RegClass::Invalid || WalkFindRegClass(RHS) == RegClass::FPR");
#endif
		return _VPCMPISTRX(LHS->Wrapped(ListDataBegin), RHS->Wrapped(ListDataBegin), Control);
	}

	IRPair<IROp_VFCADD> _VFCADD(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Vector1, OrderedNodeWrapper Vector2, uint16_t Rotate) {
		auto _Op = AllocateOp<IROp_VFCADD, IROps::OP_VFCADD>();
		_Op.first->Vector1 = Vector1;
		_Op.first->Vector2 = Vector2;
		_Op.first->Rotate = Rotate;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	IRPair<IROp_VFCADD> _VFCADD(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Vector1, OrderedNode *Vector2, uint16_t Rotate) {
		auto ListDataBegin = DualListData.ListBegin();
		Vector1->AddUse();
		Vector2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR, "GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR, "GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR");
#endif
		return _VFCADD(RegisterSize, ElementSize, Vector1->Wrapped(ListDataBegin), Vector2->Wrapped(ListDataBegin), Rotate);
	}

	/// Dest = (Vector1 * Vector2) + Addend
	///
	/// This explicitly matches x86 FMA semantics because ARM semantics are mind-bending.
	IRPair<IROp_VFMLA> _VFMLA(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Vector1, OrderedNodeWrapper Vector2, OrderedNodeWrapper Addend) {
		auto _Op = AllocateOp<IROp_VFMLA, IROps::OP_VFMLA>();
		_Op.first->Vector1 = Vector1;
		_Op.first->Vector2 = Vector2;
		_Op.first->Addend = Addend;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	/// Dest = (Vector1 * Vector2) + Addend
	///
	/// This explicitly matches x86 FMA semantics because ARM semantics are mind-bending.
	IRPair<IROp_VFMLA> _VFMLA(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Vector1, OrderedNode *Vector2, OrderedNode *Addend) {
		auto ListDataBegin = DualListData.ListBegin();
		Vector1->AddUse();
		Vector2->AddUse();
		Addend->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR, "GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR, "GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Addend) == RegClass::Invalid || WalkFindRegClass(Addend) == RegClass::FPR, "GetOpRegClass(Addend) == RegClass::Invalid || WalkFindRegClass(Addend) == RegClass::FPR");
#endif
		return _VFMLA(RegisterSize, ElementSize, Vector1->Wrapped(ListDataBegin), Vector2->Wrapped(ListDataBegin), Addend->Wrapped(ListDataBegin));
	}

	/// Dest = (Vector1 * Vector2) - Addend
	///
	/// This explicitly matches x86 FMA semantics because ARM semantics are mind-bending.
	IRPair<IROp_VFMLS> _VFMLS(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Vector1, OrderedNodeWrapper Vector2, OrderedNodeWrapper Addend) {
		auto _Op = AllocateOp<IROp_VFMLS, IROps::OP_VFMLS>();
		_Op.first->Vector1 = Vector1;
		_Op.first->Vector2 = Vector2;
		_Op.first->Addend = Addend;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	/// Dest = (Vector1 * Vector2) - Addend
	///
	/// This explicitly matches x86 FMA semantics because ARM semantics are mind-bending.
	IRPair<IROp_VFMLS> _VFMLS(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Vector1, OrderedNode *Vector2, OrderedNode *Addend) {
		auto ListDataBegin = DualListData.ListBegin();
		Vector1->AddUse();
		Vector2->AddUse();
		Addend->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR, "GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR, "GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Addend) == RegClass::Invalid || WalkFindRegClass(Addend) == RegClass::FPR, "GetOpRegClass(Addend) == RegClass::Invalid || WalkFindRegClass(Addend) == RegClass::FPR");
#endif
		return _VFMLS(RegisterSize, ElementSize, Vector1->Wrapped(ListDataBegin), Vector2->Wrapped(ListDataBegin), Addend->Wrapped(ListDataBegin));
	}

	/// Dest = (-Vector1 * Vector2) + Addend
	///
	/// This explicitly matches x86 FMA semantics because ARM semantics are mind-bending.
	IRPair<IROp_VFNMLA> _VFNMLA(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Vector1, OrderedNodeWrapper Vector2, OrderedNodeWrapper Addend) {
		auto _Op = AllocateOp<IROp_VFNMLA, IROps::OP_VFNMLA>();
		_Op.first->Vector1 = Vector1;
		_Op.first->Vector2 = Vector2;
		_Op.first->Addend = Addend;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	/// Dest = (-Vector1 * Vector2) + Addend
	///
	/// This explicitly matches x86 FMA semantics because ARM semantics are mind-bending.
	IRPair<IROp_VFNMLA> _VFNMLA(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Vector1, OrderedNode *Vector2, OrderedNode *Addend) {
		auto ListDataBegin = DualListData.ListBegin();
		Vector1->AddUse();
		Vector2->AddUse();
		Addend->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR, "GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR, "GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Addend) == RegClass::Invalid || WalkFindRegClass(Addend) == RegClass::FPR, "GetOpRegClass(Addend) == RegClass::Invalid || WalkFindRegClass(Addend) == RegClass::FPR");
#endif
		return _VFNMLA(RegisterSize, ElementSize, Vector1->Wrapped(ListDataBegin), Vector2->Wrapped(ListDataBegin), Addend->Wrapped(ListDataBegin));
	}

	/// Dest = (-Vector1 * Vector2) - Addend
	///
	/// This explicitly matches x86 FMA semantics because ARM semantics are mind-bending.
	IRPair<IROp_VFNMLS> _VFNMLS(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Vector1, OrderedNodeWrapper Vector2, OrderedNodeWrapper Addend) {
		auto _Op = AllocateOp<IROp_VFNMLS, IROps::OP_VFNMLS>();
		_Op.first->Vector1 = Vector1;
		_Op.first->Vector2 = Vector2;
		_Op.first->Addend = Addend;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	/// Dest = (-Vector1 * Vector2) - Addend
	///
	/// This explicitly matches x86 FMA semantics because ARM semantics are mind-bending.
	IRPair<IROp_VFNMLS> _VFNMLS(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Vector1, OrderedNode *Vector2, OrderedNode *Addend) {
		auto ListDataBegin = DualListData.ListBegin();
		Vector1->AddUse();
		Vector2->AddUse();
		Addend->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR, "GetOpRegClass(Vector1) == RegClass::Invalid || WalkFindRegClass(Vector1) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR, "GetOpRegClass(Vector2) == RegClass::Invalid || WalkFindRegClass(Vector2) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Addend) == RegClass::Invalid || WalkFindRegClass(Addend) == RegClass::FPR, "GetOpRegClass(Addend) == RegClass::Invalid || WalkFindRegClass(Addend) == RegClass::FPR");
#endif
		return _VFNMLS(RegisterSize, ElementSize, Vector1->Wrapped(ListDataBegin), Vector2->Wrapped(ListDataBegin), Addend->Wrapped(ListDataBegin));
	}

	/// Moves a GPR to a Vector register with zero extension to full length of the register.
	///
	/// No conversion is done on the data as it moves register files
	IRPair<IROp_VCastFromGPR> _VCastFromGPR(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Src) {
		auto _Op = AllocateOp<IROp_VCastFromGPR, IROps::OP_VCASTFROMGPR>();
		_Op.first->Src = Src;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	/// Moves a GPR to a Vector register with zero extension to full length of the register.
	///
	/// No conversion is done on the data as it moves register files
	IRPair<IROp_VCastFromGPR> _VCastFromGPR(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Src) {
		auto ListDataBegin = DualListData.ListBegin();
		Src->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src) == RegClass::Invalid || WalkFindRegClass(Src) == RegClass::GPR, "GetOpRegClass(Src) == RegClass::Invalid || WalkFindRegClass(Src) == RegClass::GPR");
#endif
		return _VCastFromGPR(RegisterSize, ElementSize, Src->Wrapped(ListDataBegin));
	}

	/// Broadcasts a value in a GPR into each ElementSize-sized element in a vector
	IRPair<IROp_VDupFromGPR> _VDupFromGPR(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Src) {
		auto _Op = AllocateOp<IROp_VDupFromGPR, IROps::OP_VDUPFROMGPR>();
		_Op.first->Src = Src;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	/// Broadcasts a value in a GPR into each ElementSize-sized element in a vector
	IRPair<IROp_VDupFromGPR> _VDupFromGPR(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Src) {
		auto ListDataBegin = DualListData.ListBegin();
		Src->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src) == RegClass::Invalid || WalkFindRegClass(Src) == RegClass::GPR, "GetOpRegClass(Src) == RegClass::Invalid || WalkFindRegClass(Src) == RegClass::GPR");
#endif
		return _VDupFromGPR(RegisterSize, ElementSize, Src->Wrapped(ListDataBegin));
	}

	/// Moves two 64-bit registers to a vector register optimally
	IRPair<IROp_VLoadTwoGPRs> _VLoadTwoGPRs(OrderedNodeWrapper Lower, OrderedNodeWrapper Upper) {
		auto _Op = AllocateOp<IROp_VLoadTwoGPRs, IROps::OP_VLOADTWOGPRS>();
		_Op.first->Lower = Lower;
		_Op.first->Upper = Upper;
		_Op.first->Header.Size = OpSize::i128Bit;
		_Op.first->Header.ElementSize = OpSize::i64Bit;
		return _Op;
	}

	/// Moves two 64-bit registers to a vector register optimally
	IRPair<IROp_VLoadTwoGPRs> _VLoadTwoGPRs(OrderedNode *Lower, OrderedNode *Upper) {
		auto ListDataBegin = DualListData.ListBegin();
		Lower->AddUse();
		Upper->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Lower) == RegClass::Invalid || WalkFindRegClass(Lower) == RegClass::GPR, "GetOpRegClass(Lower) == RegClass::Invalid || WalkFindRegClass(Lower) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Upper) == RegClass::Invalid || WalkFindRegClass(Upper) == RegClass::GPR, "GetOpRegClass(Upper) == RegClass::Invalid || WalkFindRegClass(Upper) == RegClass::GPR");
#endif
		return _VLoadTwoGPRs(Lower->Wrapped(ListDataBegin), Upper->Wrapped(ListDataBegin));
	}

	/// Scalar op: Converts signed GPR to Scalar float
	///
	/// Zeroes the upper bits of the vector register
	IRPair<IROp_Float_FromGPR_S> _Float_FromGPR_S(FEXCore::IR::OpSize DstElementSize, FEXCore::IR::OpSize SrcElementSize, OrderedNodeWrapper Src) {
		auto _Op = AllocateOp<IROp_Float_FromGPR_S, IROps::OP_FLOAT_FROMGPR_S>();
		_Op.first->Src = Src;
		_Op.first->SrcElementSize = SrcElementSize;
		_Op.first->Header.Size = DstElementSize;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Scalar op: Converts signed GPR to Scalar float
	///
	/// Zeroes the upper bits of the vector register
	IRPair<IROp_Float_FromGPR_S> _Float_FromGPR_S(FEXCore::IR::OpSize DstElementSize, FEXCore::IR::OpSize SrcElementSize, OrderedNode *Src) {
		auto ListDataBegin = DualListData.ListBegin();
		Src->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src) == RegClass::Invalid || WalkFindRegClass(Src) == RegClass::GPR, "GetOpRegClass(Src) == RegClass::Invalid || WalkFindRegClass(Src) == RegClass::GPR");
#endif
		return _Float_FromGPR_S(DstElementSize, SrcElementSize, Src->Wrapped(ListDataBegin));
	}

	/// Scalar op: Converts float from one size to another
	///
	/// Zeroes the upper bits of the vector register
	IRPair<IROp_Float_FToF> _Float_FToF(FEXCore::IR::OpSize DstElementSize, FEXCore::IR::OpSize SrcElementSize, OrderedNodeWrapper Scalar) {
		auto _Op = AllocateOp<IROp_Float_FToF, IROps::OP_FLOAT_FTOF>();
		_Op.first->Scalar = Scalar;
		_Op.first->SrcElementSize = SrcElementSize;
		_Op.first->Header.Size = DstElementSize;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Scalar op: Converts float from one size to another
	///
	/// Zeroes the upper bits of the vector register
	IRPair<IROp_Float_FToF> _Float_FToF(FEXCore::IR::OpSize DstElementSize, FEXCore::IR::OpSize SrcElementSize, OrderedNode *Scalar) {
		auto ListDataBegin = DualListData.ListBegin();
		Scalar->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Scalar) == RegClass::Invalid || WalkFindRegClass(Scalar) == RegClass::FPR, "GetOpRegClass(Scalar) == RegClass::Invalid || WalkFindRegClass(Scalar) == RegClass::FPR");
#endif
		return _Float_FToF(DstElementSize, SrcElementSize, Scalar->Wrapped(ListDataBegin));
	}

	/// V
	///
	/// e
	///
	/// c
	///
	/// t
	///
	/// o
	///
	/// r
	///
	///  
	///
	/// o
	///
	/// p
	///
	/// :
	///
	///  
	///
	/// C
	///
	/// o
	///
	/// n
	///
	/// v
	///
	/// e
	///
	/// r
	///
	/// t
	///
	/// s
	///
	///  
	///
	/// s
	///
	/// i
	///
	/// g
	///
	/// n
	///
	/// e
	///
	/// d
	///
	///  
	///
	/// i
	///
	/// n
	///
	/// t
	///
	/// e
	///
	/// g
	///
	/// e
	///
	/// r
	///
	///  
	///
	/// t
	///
	/// o
	///
	///  
	///
	/// s
	///
	/// a
	///
	/// m
	///
	/// e
	///
	///  
	///
	/// s
	///
	/// i
	///
	/// z
	///
	/// e
	///
	///  
	///
	/// f
	///
	/// l
	///
	/// o
	///
	/// a
	///
	/// t
	IRPair<IROp_Vector_SToF> _Vector_SToF(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Vector) {
		auto _Op = AllocateOp<IROp_Vector_SToF, IROps::OP_VECTOR_STOF>();
		_Op.first->Vector = Vector;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	/// V
	///
	/// e
	///
	/// c
	///
	/// t
	///
	/// o
	///
	/// r
	///
	///  
	///
	/// o
	///
	/// p
	///
	/// :
	///
	///  
	///
	/// C
	///
	/// o
	///
	/// n
	///
	/// v
	///
	/// e
	///
	/// r
	///
	/// t
	///
	/// s
	///
	///  
	///
	/// s
	///
	/// i
	///
	/// g
	///
	/// n
	///
	/// e
	///
	/// d
	///
	///  
	///
	/// i
	///
	/// n
	///
	/// t
	///
	/// e
	///
	/// g
	///
	/// e
	///
	/// r
	///
	///  
	///
	/// t
	///
	/// o
	///
	///  
	///
	/// s
	///
	/// a
	///
	/// m
	///
	/// e
	///
	///  
	///
	/// s
	///
	/// i
	///
	/// z
	///
	/// e
	///
	///  
	///
	/// f
	///
	/// l
	///
	/// o
	///
	/// a
	///
	/// t
	IRPair<IROp_Vector_SToF> _Vector_SToF(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Vector) {
		auto ListDataBegin = DualListData.ListBegin();
		Vector->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector) == RegClass::Invalid || WalkFindRegClass(Vector) == RegClass::FPR, "GetOpRegClass(Vector) == RegClass::Invalid || WalkFindRegClass(Vector) == RegClass::FPR");
#endif
		return _Vector_SToF(RegisterSize, ElementSize, Vector->Wrapped(ListDataBegin));
	}

	/// Vector op: Converts float to signed integer, rounding towards zero
	///
	/// Rounding mode determined by host rounding mode
	IRPair<IROp_Vector_FToS> _Vector_FToS(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Vector) {
		auto _Op = AllocateOp<IROp_Vector_FToS, IROps::OP_VECTOR_FTOS>();
		_Op.first->Vector = Vector;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	/// Vector op: Converts float to signed integer, rounding towards zero
	///
	/// Rounding mode determined by host rounding mode
	IRPair<IROp_Vector_FToS> _Vector_FToS(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Vector) {
		auto ListDataBegin = DualListData.ListBegin();
		Vector->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector) == RegClass::Invalid || WalkFindRegClass(Vector) == RegClass::FPR, "GetOpRegClass(Vector) == RegClass::Invalid || WalkFindRegClass(Vector) == RegClass::FPR");
#endif
		return _Vector_FToS(RegisterSize, ElementSize, Vector->Wrapped(ListDataBegin));
	}

	/// V
	///
	/// e
	///
	/// c
	///
	/// t
	///
	/// o
	///
	/// r
	///
	///  
	///
	/// o
	///
	/// p
	///
	/// :
	///
	///  
	///
	/// C
	///
	/// o
	///
	/// n
	///
	/// v
	///
	/// e
	///
	/// r
	///
	/// t
	///
	/// s
	///
	///  
	///
	/// f
	///
	/// l
	///
	/// o
	///
	/// a
	///
	/// t
	///
	///  
	///
	/// t
	///
	/// o
	///
	///  
	///
	/// s
	///
	/// i
	///
	/// g
	///
	/// n
	///
	/// e
	///
	/// d
	///
	///  
	///
	/// i
	///
	/// n
	///
	/// t
	///
	/// e
	///
	/// g
	///
	/// e
	///
	/// r
	///
	/// ,
	///
	///  
	///
	/// r
	///
	/// o
	///
	/// u
	///
	/// n
	///
	/// d
	///
	/// i
	///
	/// n
	///
	/// g
	///
	///  
	///
	/// t
	///
	/// o
	///
	/// w
	///
	/// a
	///
	/// r
	///
	/// d
	///
	/// s
	///
	///  
	///
	/// z
	///
	/// e
	///
	/// r
	///
	/// o
	IRPair<IROp_Vector_FToZS> _Vector_FToZS(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Vector) {
		auto _Op = AllocateOp<IROp_Vector_FToZS, IROps::OP_VECTOR_FTOZS>();
		_Op.first->Vector = Vector;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	/// V
	///
	/// e
	///
	/// c
	///
	/// t
	///
	/// o
	///
	/// r
	///
	///  
	///
	/// o
	///
	/// p
	///
	/// :
	///
	///  
	///
	/// C
	///
	/// o
	///
	/// n
	///
	/// v
	///
	/// e
	///
	/// r
	///
	/// t
	///
	/// s
	///
	///  
	///
	/// f
	///
	/// l
	///
	/// o
	///
	/// a
	///
	/// t
	///
	///  
	///
	/// t
	///
	/// o
	///
	///  
	///
	/// s
	///
	/// i
	///
	/// g
	///
	/// n
	///
	/// e
	///
	/// d
	///
	///  
	///
	/// i
	///
	/// n
	///
	/// t
	///
	/// e
	///
	/// g
	///
	/// e
	///
	/// r
	///
	/// ,
	///
	///  
	///
	/// r
	///
	/// o
	///
	/// u
	///
	/// n
	///
	/// d
	///
	/// i
	///
	/// n
	///
	/// g
	///
	///  
	///
	/// t
	///
	/// o
	///
	/// w
	///
	/// a
	///
	/// r
	///
	/// d
	///
	/// s
	///
	///  
	///
	/// z
	///
	/// e
	///
	/// r
	///
	/// o
	IRPair<IROp_Vector_FToZS> _Vector_FToZS(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Vector) {
		auto ListDataBegin = DualListData.ListBegin();
		Vector->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector) == RegClass::Invalid || WalkFindRegClass(Vector) == RegClass::FPR, "GetOpRegClass(Vector) == RegClass::Invalid || WalkFindRegClass(Vector) == RegClass::FPR");
#endif
		return _Vector_FToZS(RegisterSize, ElementSize, Vector->Wrapped(ListDataBegin));
	}

	/// V
	///
	/// e
	///
	/// c
	///
	/// t
	///
	/// o
	///
	/// r
	///
	///  
	///
	/// o
	///
	/// p
	///
	/// :
	///
	///  
	///
	/// C
	///
	/// o
	///
	/// n
	///
	/// v
	///
	/// e
	///
	/// r
	///
	/// t
	///
	/// s
	///
	///  
	///
	/// f
	///
	/// l
	///
	/// o
	///
	/// a
	///
	/// t
	///
	///  
	///
	/// f
	///
	/// r
	///
	/// o
	///
	/// m
	///
	///  
	///
	/// s
	///
	/// o
	///
	/// u
	///
	/// r
	///
	/// c
	///
	/// e
	///
	///  
	///
	/// e
	///
	/// l
	///
	/// e
	///
	/// m
	///
	/// e
	///
	/// n
	///
	/// t
	///
	///  
	///
	/// s
	///
	/// i
	///
	/// z
	///
	/// e
	///
	///  
	///
	/// t
	///
	/// o
	///
	///  
	///
	/// d
	///
	/// e
	///
	/// s
	///
	/// t
	///
	/// i
	///
	/// n
	///
	/// a
	///
	/// t
	///
	/// i
	///
	/// o
	///
	/// n
	///
	///  
	///
	/// s
	///
	/// i
	///
	/// z
	///
	/// e
	///
	///  
	///
	/// (
	///
	/// f
	///
	/// p
	///
	/// 3
	///
	/// 2
	///
	/// <
	///
	/// -
	///
	/// >
	///
	/// f
	///
	/// p
	///
	/// 6
	///
	/// 4
	///
	/// )
	IRPair<IROp_Vector_FToF> _Vector_FToF(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize DestElementSize, OrderedNodeWrapper Vector, FEXCore::IR::OpSize SrcElementSize) {
		auto _Op = AllocateOp<IROp_Vector_FToF, IROps::OP_VECTOR_FTOF>();
		_Op.first->Vector = Vector;
		_Op.first->SrcElementSize = SrcElementSize;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = DestElementSize;
		return _Op;
	}

	/// V
	///
	/// e
	///
	/// c
	///
	/// t
	///
	/// o
	///
	/// r
	///
	///  
	///
	/// o
	///
	/// p
	///
	/// :
	///
	///  
	///
	/// C
	///
	/// o
	///
	/// n
	///
	/// v
	///
	/// e
	///
	/// r
	///
	/// t
	///
	/// s
	///
	///  
	///
	/// f
	///
	/// l
	///
	/// o
	///
	/// a
	///
	/// t
	///
	///  
	///
	/// f
	///
	/// r
	///
	/// o
	///
	/// m
	///
	///  
	///
	/// s
	///
	/// o
	///
	/// u
	///
	/// r
	///
	/// c
	///
	/// e
	///
	///  
	///
	/// e
	///
	/// l
	///
	/// e
	///
	/// m
	///
	/// e
	///
	/// n
	///
	/// t
	///
	///  
	///
	/// s
	///
	/// i
	///
	/// z
	///
	/// e
	///
	///  
	///
	/// t
	///
	/// o
	///
	///  
	///
	/// d
	///
	/// e
	///
	/// s
	///
	/// t
	///
	/// i
	///
	/// n
	///
	/// a
	///
	/// t
	///
	/// i
	///
	/// o
	///
	/// n
	///
	///  
	///
	/// s
	///
	/// i
	///
	/// z
	///
	/// e
	///
	///  
	///
	/// (
	///
	/// f
	///
	/// p
	///
	/// 3
	///
	/// 2
	///
	/// <
	///
	/// -
	///
	/// >
	///
	/// f
	///
	/// p
	///
	/// 6
	///
	/// 4
	///
	/// )
	IRPair<IROp_Vector_FToF> _Vector_FToF(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize DestElementSize, OrderedNode *Vector, FEXCore::IR::OpSize SrcElementSize) {
		auto ListDataBegin = DualListData.ListBegin();
		Vector->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector) == RegClass::Invalid || WalkFindRegClass(Vector) == RegClass::FPR, "GetOpRegClass(Vector) == RegClass::Invalid || WalkFindRegClass(Vector) == RegClass::FPR");
#endif
		return _Vector_FToF(RegisterSize, DestElementSize, Vector->Wrapped(ListDataBegin), SrcElementSize);
	}

	/// Vector op: Converts float from source element size to destination size (fp32->fp64)
	///
	/// Selecting from the high half of the register.
	IRPair<IROp_VFCVTL2> _VFCVTL2(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Vector) {
		auto _Op = AllocateOp<IROp_VFCVTL2, IROps::OP_VFCVTL2>();
		_Op.first->Vector = Vector;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize << 1;
		return _Op;
	}

	/// Vector op: Converts float from source element size to destination size (fp32->fp64)
	///
	/// Selecting from the high half of the register.
	IRPair<IROp_VFCVTL2> _VFCVTL2(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Vector) {
		auto ListDataBegin = DualListData.ListBegin();
		Vector->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector) == RegClass::Invalid || WalkFindRegClass(Vector) == RegClass::FPR, "GetOpRegClass(Vector) == RegClass::Invalid || WalkFindRegClass(Vector) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(RegisterSize != FEXCore::IR::OpSize::i256Bit && "What does 256-bit mean in this context?", "RegisterSize != FEXCore::IR::OpSize::i256Bit && \"What does 256-bit mean in this context?\"");
#endif
		return _VFCVTL2(RegisterSize, ElementSize, Vector->Wrapped(ListDataBegin));
	}

	/// Vector op: Converts float from source element size and inserting in to the high bits.
	///
	/// Bottom half is untouched
	///
	/// Narrowing to the element size below what is passed in.
	///
	/// F64->F32, F32->F16
	IRPair<IROp_VFCVTN2> _VFCVTN2(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper VectorLower, OrderedNodeWrapper VectorUpper) {
		auto _Op = AllocateOp<IROp_VFCVTN2, IROps::OP_VFCVTN2>();
		_Op.first->VectorLower = VectorLower;
		_Op.first->VectorUpper = VectorUpper;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize >> 1;
		return _Op;
	}

	/// Vector op: Converts float from source element size and inserting in to the high bits.
	///
	/// Bottom half is untouched
	///
	/// Narrowing to the element size below what is passed in.
	///
	/// F64->F32, F32->F16
	IRPair<IROp_VFCVTN2> _VFCVTN2(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *VectorLower, OrderedNode *VectorUpper) {
		auto ListDataBegin = DualListData.ListBegin();
		VectorLower->AddUse();
		VectorUpper->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(VectorLower) == RegClass::Invalid || WalkFindRegClass(VectorLower) == RegClass::FPR, "GetOpRegClass(VectorLower) == RegClass::Invalid || WalkFindRegClass(VectorLower) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(VectorUpper) == RegClass::Invalid || WalkFindRegClass(VectorUpper) == RegClass::FPR, "GetOpRegClass(VectorUpper) == RegClass::Invalid || WalkFindRegClass(VectorUpper) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(RegisterSize != FEXCore::IR::OpSize::i256Bit && "What does 256-bit mean in this context?", "RegisterSize != FEXCore::IR::OpSize::i256Bit && \"What does 256-bit mean in this context?\"");
#endif
		return _VFCVTN2(RegisterSize, ElementSize, VectorLower->Wrapped(ListDataBegin), VectorUpper->Wrapped(ListDataBegin));
	}

	/// Vector op: Rounds float to integral
	///
	/// Rounding mode determined by argument
	IRPair<IROp_Vector_FToI> _Vector_FToI(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Vector, RoundMode Round) {
		auto _Op = AllocateOp<IROp_Vector_FToI, IROps::OP_VECTOR_FTOI>();
		_Op.first->Vector = Vector;
		_Op.first->Round = Round;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	/// Vector op: Rounds float to integral
	///
	/// Rounding mode determined by argument
	IRPair<IROp_Vector_FToI> _Vector_FToI(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Vector, RoundMode Round) {
		auto ListDataBegin = DualListData.ListBegin();
		Vector->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector) == RegClass::Invalid || WalkFindRegClass(Vector) == RegClass::FPR, "GetOpRegClass(Vector) == RegClass::Invalid || WalkFindRegClass(Vector) == RegClass::FPR");
#endif
		return _Vector_FToI(RegisterSize, ElementSize, Vector->Wrapped(ListDataBegin), Round);
	}

	/// Vector op: Rounds float to sized integral
	///
	/// Either host rounding or round-to-zero
	///
	/// Rounding mode determined by argument
	IRPair<IROp_Vector_FToISized> _Vector_FToISized(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNodeWrapper Vector, bool HostRound, FEXCore::IR::OpSize IntSize) {
		auto _Op = AllocateOp<IROp_Vector_FToISized, IROps::OP_VECTOR_FTOISIZED>();
		_Op.first->Vector = Vector;
		_Op.first->HostRound = HostRound;
		_Op.first->IntSize = IntSize;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = ElementSize;
		return _Op;
	}

	/// Vector op: Rounds float to sized integral
	///
	/// Either host rounding or round-to-zero
	///
	/// Rounding mode determined by argument
	IRPair<IROp_Vector_FToISized> _Vector_FToISized(FEXCore::IR::OpSize RegisterSize, FEXCore::IR::OpSize ElementSize, OrderedNode *Vector, bool HostRound, FEXCore::IR::OpSize IntSize) {
		auto ListDataBegin = DualListData.ListBegin();
		Vector->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector) == RegClass::Invalid || WalkFindRegClass(Vector) == RegClass::FPR, "GetOpRegClass(Vector) == RegClass::Invalid || WalkFindRegClass(Vector) == RegClass::FPR");
#endif
		return _Vector_FToISized(RegisterSize, ElementSize, Vector->Wrapped(ListDataBegin), HostRound, IntSize);
	}

	/// Vector op: Rounds 64-bit float to 32-bit integral with round mode
	///
	/// Matches CVTPD2DQ/CVTTPD2DQ behaviour
	IRPair<IROp_Vector_F64ToI32> _Vector_F64ToI32(FEXCore::IR::OpSize RegisterSize, OrderedNodeWrapper Vector, RoundMode Round, bool EnsureZeroUpperHalf) {
		auto _Op = AllocateOp<IROp_Vector_F64ToI32, IROps::OP_VECTOR_F64TOI32>();
		_Op.first->Vector = Vector;
		_Op.first->Round = Round;
		_Op.first->EnsureZeroUpperHalf = EnsureZeroUpperHalf;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = FEXCore::IR::OpSize::i32Bit;
		return _Op;
	}

	/// Vector op: Rounds 64-bit float to 32-bit integral with round mode
	///
	/// Matches CVTPD2DQ/CVTTPD2DQ behaviour
	IRPair<IROp_Vector_F64ToI32> _Vector_F64ToI32(FEXCore::IR::OpSize RegisterSize, OrderedNode *Vector, RoundMode Round, bool EnsureZeroUpperHalf) {
		auto ListDataBegin = DualListData.ListBegin();
		Vector->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector) == RegClass::Invalid || WalkFindRegClass(Vector) == RegClass::FPR, "GetOpRegClass(Vector) == RegClass::Invalid || WalkFindRegClass(Vector) == RegClass::FPR");
#endif
		return _Vector_F64ToI32(RegisterSize, Vector->Wrapped(ListDataBegin), Round, EnsureZeroUpperHalf);
	}

	/// D
	///
	/// o
	///
	/// e
	///
	/// s
	///
	///  
	///
	/// a
	///
	///  
	///
	/// s
	///
	/// t
	///
	/// a
	///
	/// g
	///
	/// e
	///
	///  
	///
	/// o
	///
	/// f
	///
	///  
	///
	/// t
	///
	/// h
	///
	/// e
	///
	///  
	///
	/// i
	///
	/// n
	///
	/// v
	///
	/// e
	///
	/// r
	///
	/// s
	///
	/// e
	///
	///  
	///
	/// m
	///
	/// i
	///
	/// x
	///
	///  
	///
	/// c
	///
	/// o
	///
	/// l
	///
	/// u
	///
	/// m
	///
	/// n
	///
	///  
	///
	/// t
	///
	/// r
	///
	/// a
	///
	/// n
	///
	/// s
	///
	/// f
	///
	/// o
	///
	/// r
	///
	/// m
	///
	/// a
	///
	/// t
	///
	/// i
	///
	/// o
	///
	/// n
	IRPair<IROp_VAESImc> _VAESImc(OrderedNodeWrapper Vector) {
		auto _Op = AllocateOp<IROp_VAESImc, IROps::OP_VAESIMC>();
		_Op.first->Vector = Vector;
		_Op.first->Header.Size = OpSize::i128Bit;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// D
	///
	/// o
	///
	/// e
	///
	/// s
	///
	///  
	///
	/// a
	///
	///  
	///
	/// s
	///
	/// t
	///
	/// a
	///
	/// g
	///
	/// e
	///
	///  
	///
	/// o
	///
	/// f
	///
	///  
	///
	/// t
	///
	/// h
	///
	/// e
	///
	///  
	///
	/// i
	///
	/// n
	///
	/// v
	///
	/// e
	///
	/// r
	///
	/// s
	///
	/// e
	///
	///  
	///
	/// m
	///
	/// i
	///
	/// x
	///
	///  
	///
	/// c
	///
	/// o
	///
	/// l
	///
	/// u
	///
	/// m
	///
	/// n
	///
	///  
	///
	/// t
	///
	/// r
	///
	/// a
	///
	/// n
	///
	/// s
	///
	/// f
	///
	/// o
	///
	/// r
	///
	/// m
	///
	/// a
	///
	/// t
	///
	/// i
	///
	/// o
	///
	/// n
	IRPair<IROp_VAESImc> _VAESImc(OrderedNode *Vector) {
		auto ListDataBegin = DualListData.ListBegin();
		Vector->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Vector) == RegClass::Invalid || WalkFindRegClass(Vector) == RegClass::FPR, "GetOpRegClass(Vector) == RegClass::Invalid || WalkFindRegClass(Vector) == RegClass::FPR");
#endif
		return _VAESImc(Vector->Wrapped(ListDataBegin));
	}

	/// D
	///
	/// o
	///
	/// e
	///
	/// s
	///
	///  
	///
	/// a
	///
	///  
	///
	/// s
	///
	/// t
	///
	/// e
	///
	/// p
	///
	///  
	///
	/// o
	///
	/// f
	///
	///  
	///
	/// A
	///
	/// E
	///
	/// S
	///
	///  
	///
	/// e
	///
	/// n
	///
	/// c
	///
	/// r
	///
	/// y
	///
	/// p
	///
	/// t
	///
	/// i
	///
	/// o
	///
	/// n
	IRPair<IROp_VAESEnc> _VAESEnc(FEXCore::IR::OpSize RegisterSize, OrderedNodeWrapper State, OrderedNodeWrapper Key, OrderedNodeWrapper ZeroReg) {
		auto _Op = AllocateOp<IROp_VAESEnc, IROps::OP_VAESENC>();
		_Op.first->State = State;
		_Op.first->Key = Key;
		_Op.first->ZeroReg = ZeroReg;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// D
	///
	/// o
	///
	/// e
	///
	/// s
	///
	///  
	///
	/// a
	///
	///  
	///
	/// s
	///
	/// t
	///
	/// e
	///
	/// p
	///
	///  
	///
	/// o
	///
	/// f
	///
	///  
	///
	/// A
	///
	/// E
	///
	/// S
	///
	///  
	///
	/// e
	///
	/// n
	///
	/// c
	///
	/// r
	///
	/// y
	///
	/// p
	///
	/// t
	///
	/// i
	///
	/// o
	///
	/// n
	IRPair<IROp_VAESEnc> _VAESEnc(FEXCore::IR::OpSize RegisterSize, OrderedNode *State, OrderedNode *Key, OrderedNode *ZeroReg) {
		auto ListDataBegin = DualListData.ListBegin();
		State->AddUse();
		Key->AddUse();
		ZeroReg->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(State) == RegClass::Invalid || WalkFindRegClass(State) == RegClass::FPR, "GetOpRegClass(State) == RegClass::Invalid || WalkFindRegClass(State) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Key) == RegClass::Invalid || WalkFindRegClass(Key) == RegClass::FPR, "GetOpRegClass(Key) == RegClass::Invalid || WalkFindRegClass(Key) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(ZeroReg) == RegClass::Invalid || WalkFindRegClass(ZeroReg) == RegClass::FPR, "GetOpRegClass(ZeroReg) == RegClass::Invalid || WalkFindRegClass(ZeroReg) == RegClass::FPR");
#endif
		return _VAESEnc(RegisterSize, State->Wrapped(ListDataBegin), Key->Wrapped(ListDataBegin), ZeroReg->Wrapped(ListDataBegin));
	}

	/// D
	///
	/// o
	///
	/// e
	///
	/// s
	///
	///  
	///
	/// t
	///
	/// h
	///
	/// e
	///
	///  
	///
	/// l
	///
	/// a
	///
	/// s
	///
	/// t
	///
	///  
	///
	/// s
	///
	/// t
	///
	/// e
	///
	/// p
	///
	///  
	///
	/// o
	///
	/// f
	///
	///  
	///
	/// A
	///
	/// E
	///
	/// S
	///
	///  
	///
	/// e
	///
	/// n
	///
	/// c
	///
	/// r
	///
	/// y
	///
	/// p
	///
	/// t
	///
	/// i
	///
	/// o
	///
	/// n
	IRPair<IROp_VAESEncLast> _VAESEncLast(FEXCore::IR::OpSize RegisterSize, OrderedNodeWrapper State, OrderedNodeWrapper Key, OrderedNodeWrapper ZeroReg) {
		auto _Op = AllocateOp<IROp_VAESEncLast, IROps::OP_VAESENCLAST>();
		_Op.first->State = State;
		_Op.first->Key = Key;
		_Op.first->ZeroReg = ZeroReg;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// D
	///
	/// o
	///
	/// e
	///
	/// s
	///
	///  
	///
	/// t
	///
	/// h
	///
	/// e
	///
	///  
	///
	/// l
	///
	/// a
	///
	/// s
	///
	/// t
	///
	///  
	///
	/// s
	///
	/// t
	///
	/// e
	///
	/// p
	///
	///  
	///
	/// o
	///
	/// f
	///
	///  
	///
	/// A
	///
	/// E
	///
	/// S
	///
	///  
	///
	/// e
	///
	/// n
	///
	/// c
	///
	/// r
	///
	/// y
	///
	/// p
	///
	/// t
	///
	/// i
	///
	/// o
	///
	/// n
	IRPair<IROp_VAESEncLast> _VAESEncLast(FEXCore::IR::OpSize RegisterSize, OrderedNode *State, OrderedNode *Key, OrderedNode *ZeroReg) {
		auto ListDataBegin = DualListData.ListBegin();
		State->AddUse();
		Key->AddUse();
		ZeroReg->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(State) == RegClass::Invalid || WalkFindRegClass(State) == RegClass::FPR, "GetOpRegClass(State) == RegClass::Invalid || WalkFindRegClass(State) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Key) == RegClass::Invalid || WalkFindRegClass(Key) == RegClass::FPR, "GetOpRegClass(Key) == RegClass::Invalid || WalkFindRegClass(Key) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(ZeroReg) == RegClass::Invalid || WalkFindRegClass(ZeroReg) == RegClass::FPR, "GetOpRegClass(ZeroReg) == RegClass::Invalid || WalkFindRegClass(ZeroReg) == RegClass::FPR");
#endif
		return _VAESEncLast(RegisterSize, State->Wrapped(ListDataBegin), Key->Wrapped(ListDataBegin), ZeroReg->Wrapped(ListDataBegin));
	}

	/// D
	///
	/// o
	///
	/// e
	///
	/// s
	///
	///  
	///
	/// a
	///
	///  
	///
	/// s
	///
	/// t
	///
	/// e
	///
	/// p
	///
	///  
	///
	/// o
	///
	/// f
	///
	///  
	///
	/// A
	///
	/// E
	///
	/// S
	///
	///  
	///
	/// d
	///
	/// e
	///
	/// c
	///
	/// r
	///
	/// y
	///
	/// p
	///
	/// t
	///
	/// i
	///
	/// o
	///
	/// n
	IRPair<IROp_VAESDec> _VAESDec(FEXCore::IR::OpSize RegisterSize, OrderedNodeWrapper State, OrderedNodeWrapper Key, OrderedNodeWrapper ZeroReg) {
		auto _Op = AllocateOp<IROp_VAESDec, IROps::OP_VAESDEC>();
		_Op.first->State = State;
		_Op.first->Key = Key;
		_Op.first->ZeroReg = ZeroReg;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// D
	///
	/// o
	///
	/// e
	///
	/// s
	///
	///  
	///
	/// a
	///
	///  
	///
	/// s
	///
	/// t
	///
	/// e
	///
	/// p
	///
	///  
	///
	/// o
	///
	/// f
	///
	///  
	///
	/// A
	///
	/// E
	///
	/// S
	///
	///  
	///
	/// d
	///
	/// e
	///
	/// c
	///
	/// r
	///
	/// y
	///
	/// p
	///
	/// t
	///
	/// i
	///
	/// o
	///
	/// n
	IRPair<IROp_VAESDec> _VAESDec(FEXCore::IR::OpSize RegisterSize, OrderedNode *State, OrderedNode *Key, OrderedNode *ZeroReg) {
		auto ListDataBegin = DualListData.ListBegin();
		State->AddUse();
		Key->AddUse();
		ZeroReg->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(State) == RegClass::Invalid || WalkFindRegClass(State) == RegClass::FPR, "GetOpRegClass(State) == RegClass::Invalid || WalkFindRegClass(State) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Key) == RegClass::Invalid || WalkFindRegClass(Key) == RegClass::FPR, "GetOpRegClass(Key) == RegClass::Invalid || WalkFindRegClass(Key) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(ZeroReg) == RegClass::Invalid || WalkFindRegClass(ZeroReg) == RegClass::FPR, "GetOpRegClass(ZeroReg) == RegClass::Invalid || WalkFindRegClass(ZeroReg) == RegClass::FPR");
#endif
		return _VAESDec(RegisterSize, State->Wrapped(ListDataBegin), Key->Wrapped(ListDataBegin), ZeroReg->Wrapped(ListDataBegin));
	}

	/// D
	///
	/// o
	///
	/// e
	///
	/// s
	///
	///  
	///
	/// t
	///
	/// h
	///
	/// e
	///
	///  
	///
	/// l
	///
	/// a
	///
	/// s
	///
	/// t
	///
	///  
	///
	/// s
	///
	/// t
	///
	/// e
	///
	/// p
	///
	///  
	///
	/// o
	///
	/// f
	///
	///  
	///
	/// A
	///
	/// E
	///
	/// S
	///
	///  
	///
	/// d
	///
	/// e
	///
	/// c
	///
	/// r
	///
	/// y
	///
	/// p
	///
	/// t
	///
	/// i
	///
	/// o
	///
	/// n
	IRPair<IROp_VAESDecLast> _VAESDecLast(FEXCore::IR::OpSize RegisterSize, OrderedNodeWrapper State, OrderedNodeWrapper Key, OrderedNodeWrapper ZeroReg) {
		auto _Op = AllocateOp<IROp_VAESDecLast, IROps::OP_VAESDECLAST>();
		_Op.first->State = State;
		_Op.first->Key = Key;
		_Op.first->ZeroReg = ZeroReg;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// D
	///
	/// o
	///
	/// e
	///
	/// s
	///
	///  
	///
	/// t
	///
	/// h
	///
	/// e
	///
	///  
	///
	/// l
	///
	/// a
	///
	/// s
	///
	/// t
	///
	///  
	///
	/// s
	///
	/// t
	///
	/// e
	///
	/// p
	///
	///  
	///
	/// o
	///
	/// f
	///
	///  
	///
	/// A
	///
	/// E
	///
	/// S
	///
	///  
	///
	/// d
	///
	/// e
	///
	/// c
	///
	/// r
	///
	/// y
	///
	/// p
	///
	/// t
	///
	/// i
	///
	/// o
	///
	/// n
	IRPair<IROp_VAESDecLast> _VAESDecLast(FEXCore::IR::OpSize RegisterSize, OrderedNode *State, OrderedNode *Key, OrderedNode *ZeroReg) {
		auto ListDataBegin = DualListData.ListBegin();
		State->AddUse();
		Key->AddUse();
		ZeroReg->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(State) == RegClass::Invalid || WalkFindRegClass(State) == RegClass::FPR, "GetOpRegClass(State) == RegClass::Invalid || WalkFindRegClass(State) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Key) == RegClass::Invalid || WalkFindRegClass(Key) == RegClass::FPR, "GetOpRegClass(Key) == RegClass::Invalid || WalkFindRegClass(Key) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(ZeroReg) == RegClass::Invalid || WalkFindRegClass(ZeroReg) == RegClass::FPR, "GetOpRegClass(ZeroReg) == RegClass::Invalid || WalkFindRegClass(ZeroReg) == RegClass::FPR");
#endif
		return _VAESDecLast(RegisterSize, State->Wrapped(ListDataBegin), Key->Wrapped(ListDataBegin), ZeroReg->Wrapped(ListDataBegin));
	}

	/// A
	///
	/// s
	///
	/// s
	///
	/// i
	///
	/// s
	///
	/// t
	///
	/// s
	///
	///  
	///
	/// i
	///
	/// n
	///
	///  
	///
	/// k
	///
	/// e
	///
	/// y
	///
	///  
	///
	/// g
	///
	/// e
	///
	/// n
	///
	/// e
	///
	/// r
	///
	/// a
	///
	/// t
	///
	/// i
	///
	/// o
	///
	/// n
	IRPair<IROp_VAESKeyGenAssist> _VAESKeyGenAssist(OrderedNodeWrapper Src, OrderedNodeWrapper KeyGenTBLSwizzle, OrderedNodeWrapper ZeroReg, uint8_t RCON) {
		auto _Op = AllocateOp<IROp_VAESKeyGenAssist, IROps::OP_VAESKEYGENASSIST>();
		_Op.first->Src = Src;
		_Op.first->KeyGenTBLSwizzle = KeyGenTBLSwizzle;
		_Op.first->ZeroReg = ZeroReg;
		_Op.first->RCON = RCON;
		_Op.first->Header.Size = OpSize::i128Bit;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// A
	///
	/// s
	///
	/// s
	///
	/// i
	///
	/// s
	///
	/// t
	///
	/// s
	///
	///  
	///
	/// i
	///
	/// n
	///
	///  
	///
	/// k
	///
	/// e
	///
	/// y
	///
	///  
	///
	/// g
	///
	/// e
	///
	/// n
	///
	/// e
	///
	/// r
	///
	/// a
	///
	/// t
	///
	/// i
	///
	/// o
	///
	/// n
	IRPair<IROp_VAESKeyGenAssist> _VAESKeyGenAssist(OrderedNode *Src, OrderedNode *KeyGenTBLSwizzle, OrderedNode *ZeroReg, uint8_t RCON) {
		auto ListDataBegin = DualListData.ListBegin();
		Src->AddUse();
		KeyGenTBLSwizzle->AddUse();
		ZeroReg->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src) == RegClass::Invalid || WalkFindRegClass(Src) == RegClass::FPR, "GetOpRegClass(Src) == RegClass::Invalid || WalkFindRegClass(Src) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(KeyGenTBLSwizzle) == RegClass::Invalid || WalkFindRegClass(KeyGenTBLSwizzle) == RegClass::FPR, "GetOpRegClass(KeyGenTBLSwizzle) == RegClass::Invalid || WalkFindRegClass(KeyGenTBLSwizzle) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(ZeroReg) == RegClass::Invalid || WalkFindRegClass(ZeroReg) == RegClass::FPR, "GetOpRegClass(ZeroReg) == RegClass::Invalid || WalkFindRegClass(ZeroReg) == RegClass::FPR");
#endif
		return _VAESKeyGenAssist(Src->Wrapped(ListDataBegin), KeyGenTBLSwizzle->Wrapped(ListDataBegin), ZeroReg->Wrapped(ListDataBegin), RCON);
	}

	/// D
	///
	/// o
	///
	/// e
	///
	/// s
	///
	///  
	///
	/// v
	///
	/// e
	///
	/// c
	///
	/// t
	///
	/// o
	///
	/// r
	///
	///  
	///
	/// s
	///
	/// c
	///
	/// a
	///
	/// l
	///
	/// a
	///
	/// r
	///
	///  
	///
	/// S
	///
	/// H
	///
	/// A
	///
	/// 1
	///
	/// H
	///
	///  
	///
	/// i
	///
	/// n
	///
	/// s
	///
	/// t
	///
	/// r
	///
	/// u
	///
	/// c
	///
	/// t
	///
	/// i
	///
	/// o
	///
	/// n
	IRPair<IROp_VSha1H> _VSha1H(OrderedNodeWrapper Src) {
		auto _Op = AllocateOp<IROp_VSha1H, IROps::OP_VSHA1H>();
		_Op.first->Src = Src;
		_Op.first->Header.Size = FEXCore::IR::OpSize::i32Bit;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// D
	///
	/// o
	///
	/// e
	///
	/// s
	///
	///  
	///
	/// v
	///
	/// e
	///
	/// c
	///
	/// t
	///
	/// o
	///
	/// r
	///
	///  
	///
	/// s
	///
	/// c
	///
	/// a
	///
	/// l
	///
	/// a
	///
	/// r
	///
	///  
	///
	/// S
	///
	/// H
	///
	/// A
	///
	/// 1
	///
	/// H
	///
	///  
	///
	/// i
	///
	/// n
	///
	/// s
	///
	/// t
	///
	/// r
	///
	/// u
	///
	/// c
	///
	/// t
	///
	/// i
	///
	/// o
	///
	/// n
	IRPair<IROp_VSha1H> _VSha1H(OrderedNode *Src) {
		auto ListDataBegin = DualListData.ListBegin();
		Src->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src) == RegClass::Invalid || WalkFindRegClass(Src) == RegClass::FPR, "GetOpRegClass(Src) == RegClass::Invalid || WalkFindRegClass(Src) == RegClass::FPR");
#endif
		return _VSha1H(Src->Wrapped(ListDataBegin));
	}

	/// D
	///
	/// o
	///
	/// e
	///
	/// s
	///
	///  
	///
	/// v
	///
	/// e
	///
	/// c
	///
	/// t
	///
	/// o
	///
	/// r
	///
	///  
	///
	/// S
	///
	/// H
	///
	/// A
	///
	/// 1
	///
	/// C
	///
	///  
	///
	/// i
	///
	/// n
	///
	/// s
	///
	/// t
	///
	/// r
	///
	/// u
	///
	/// c
	///
	/// t
	///
	/// i
	///
	/// o
	///
	/// n
	IRPair<IROp_VSha1C> _VSha1C(OrderedNodeWrapper Src1, OrderedNodeWrapper Src2, OrderedNodeWrapper Src3) {
		auto _Op = AllocateOp<IROp_VSha1C, IROps::OP_VSHA1C>();
		_Op.first->Src1 = Src1;
		_Op.first->Src2 = Src2;
		_Op.first->Src3 = Src3;
		_Op.first->Header.Size = FEXCore::IR::OpSize::i128Bit;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// D
	///
	/// o
	///
	/// e
	///
	/// s
	///
	///  
	///
	/// v
	///
	/// e
	///
	/// c
	///
	/// t
	///
	/// o
	///
	/// r
	///
	///  
	///
	/// S
	///
	/// H
	///
	/// A
	///
	/// 1
	///
	/// C
	///
	///  
	///
	/// i
	///
	/// n
	///
	/// s
	///
	/// t
	///
	/// r
	///
	/// u
	///
	/// c
	///
	/// t
	///
	/// i
	///
	/// o
	///
	/// n
	IRPair<IROp_VSha1C> _VSha1C(OrderedNode *Src1, OrderedNode *Src2, OrderedNode *Src3) {
		auto ListDataBegin = DualListData.ListBegin();
		Src1->AddUse();
		Src2->AddUse();
		Src3->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src1) == RegClass::Invalid || WalkFindRegClass(Src1) == RegClass::FPR, "GetOpRegClass(Src1) == RegClass::Invalid || WalkFindRegClass(Src1) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src2) == RegClass::Invalid || WalkFindRegClass(Src2) == RegClass::FPR, "GetOpRegClass(Src2) == RegClass::Invalid || WalkFindRegClass(Src2) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src3) == RegClass::Invalid || WalkFindRegClass(Src3) == RegClass::FPR, "GetOpRegClass(Src3) == RegClass::Invalid || WalkFindRegClass(Src3) == RegClass::FPR");
#endif
		return _VSha1C(Src1->Wrapped(ListDataBegin), Src2->Wrapped(ListDataBegin), Src3->Wrapped(ListDataBegin));
	}

	/// D
	///
	/// o
	///
	/// e
	///
	/// s
	///
	///  
	///
	/// v
	///
	/// e
	///
	/// c
	///
	/// t
	///
	/// o
	///
	/// r
	///
	///  
	///
	/// S
	///
	/// H
	///
	/// A
	///
	/// 1
	///
	/// M
	///
	///  
	///
	/// i
	///
	/// n
	///
	/// s
	///
	/// t
	///
	/// r
	///
	/// u
	///
	/// c
	///
	/// t
	///
	/// i
	///
	/// o
	///
	/// n
	IRPair<IROp_VSha1M> _VSha1M(OrderedNodeWrapper Src1, OrderedNodeWrapper Src2, OrderedNodeWrapper Src3) {
		auto _Op = AllocateOp<IROp_VSha1M, IROps::OP_VSHA1M>();
		_Op.first->Src1 = Src1;
		_Op.first->Src2 = Src2;
		_Op.first->Src3 = Src3;
		_Op.first->Header.Size = FEXCore::IR::OpSize::i128Bit;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// D
	///
	/// o
	///
	/// e
	///
	/// s
	///
	///  
	///
	/// v
	///
	/// e
	///
	/// c
	///
	/// t
	///
	/// o
	///
	/// r
	///
	///  
	///
	/// S
	///
	/// H
	///
	/// A
	///
	/// 1
	///
	/// M
	///
	///  
	///
	/// i
	///
	/// n
	///
	/// s
	///
	/// t
	///
	/// r
	///
	/// u
	///
	/// c
	///
	/// t
	///
	/// i
	///
	/// o
	///
	/// n
	IRPair<IROp_VSha1M> _VSha1M(OrderedNode *Src1, OrderedNode *Src2, OrderedNode *Src3) {
		auto ListDataBegin = DualListData.ListBegin();
		Src1->AddUse();
		Src2->AddUse();
		Src3->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src1) == RegClass::Invalid || WalkFindRegClass(Src1) == RegClass::FPR, "GetOpRegClass(Src1) == RegClass::Invalid || WalkFindRegClass(Src1) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src2) == RegClass::Invalid || WalkFindRegClass(Src2) == RegClass::FPR, "GetOpRegClass(Src2) == RegClass::Invalid || WalkFindRegClass(Src2) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src3) == RegClass::Invalid || WalkFindRegClass(Src3) == RegClass::FPR, "GetOpRegClass(Src3) == RegClass::Invalid || WalkFindRegClass(Src3) == RegClass::FPR");
#endif
		return _VSha1M(Src1->Wrapped(ListDataBegin), Src2->Wrapped(ListDataBegin), Src3->Wrapped(ListDataBegin));
	}

	/// D
	///
	/// o
	///
	/// e
	///
	/// s
	///
	///  
	///
	/// v
	///
	/// e
	///
	/// c
	///
	/// t
	///
	/// o
	///
	/// r
	///
	///  
	///
	/// S
	///
	/// H
	///
	/// A
	///
	/// 1
	///
	/// P
	///
	///  
	///
	/// i
	///
	/// n
	///
	/// s
	///
	/// t
	///
	/// r
	///
	/// u
	///
	/// c
	///
	/// t
	///
	/// i
	///
	/// o
	///
	/// n
	IRPair<IROp_VSha1P> _VSha1P(OrderedNodeWrapper Src1, OrderedNodeWrapper Src2, OrderedNodeWrapper Src3) {
		auto _Op = AllocateOp<IROp_VSha1P, IROps::OP_VSHA1P>();
		_Op.first->Src1 = Src1;
		_Op.first->Src2 = Src2;
		_Op.first->Src3 = Src3;
		_Op.first->Header.Size = FEXCore::IR::OpSize::i128Bit;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// D
	///
	/// o
	///
	/// e
	///
	/// s
	///
	///  
	///
	/// v
	///
	/// e
	///
	/// c
	///
	/// t
	///
	/// o
	///
	/// r
	///
	///  
	///
	/// S
	///
	/// H
	///
	/// A
	///
	/// 1
	///
	/// P
	///
	///  
	///
	/// i
	///
	/// n
	///
	/// s
	///
	/// t
	///
	/// r
	///
	/// u
	///
	/// c
	///
	/// t
	///
	/// i
	///
	/// o
	///
	/// n
	IRPair<IROp_VSha1P> _VSha1P(OrderedNode *Src1, OrderedNode *Src2, OrderedNode *Src3) {
		auto ListDataBegin = DualListData.ListBegin();
		Src1->AddUse();
		Src2->AddUse();
		Src3->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src1) == RegClass::Invalid || WalkFindRegClass(Src1) == RegClass::FPR, "GetOpRegClass(Src1) == RegClass::Invalid || WalkFindRegClass(Src1) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src2) == RegClass::Invalid || WalkFindRegClass(Src2) == RegClass::FPR, "GetOpRegClass(Src2) == RegClass::Invalid || WalkFindRegClass(Src2) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src3) == RegClass::Invalid || WalkFindRegClass(Src3) == RegClass::FPR, "GetOpRegClass(Src3) == RegClass::Invalid || WalkFindRegClass(Src3) == RegClass::FPR");
#endif
		return _VSha1P(Src1->Wrapped(ListDataBegin), Src2->Wrapped(ListDataBegin), Src3->Wrapped(ListDataBegin));
	}

	/// D
	///
	/// o
	///
	/// e
	///
	/// s
	///
	///  
	///
	/// v
	///
	/// e
	///
	/// c
	///
	/// t
	///
	/// o
	///
	/// r
	///
	///  
	///
	/// s
	///
	/// c
	///
	/// a
	///
	/// l
	///
	/// a
	///
	/// r
	///
	///  
	///
	/// S
	///
	/// H
	///
	/// A
	///
	/// 1
	///
	/// H
	///
	///  
	///
	/// i
	///
	/// n
	///
	/// s
	///
	/// t
	///
	/// r
	///
	/// u
	///
	/// c
	///
	/// t
	///
	/// i
	///
	/// o
	///
	/// n
	IRPair<IROp_VSha1SU1> _VSha1SU1(OrderedNodeWrapper Src1, OrderedNodeWrapper Src2) {
		auto _Op = AllocateOp<IROp_VSha1SU1, IROps::OP_VSHA1SU1>();
		_Op.first->Src1 = Src1;
		_Op.first->Src2 = Src2;
		_Op.first->Header.Size = FEXCore::IR::OpSize::i128Bit;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// D
	///
	/// o
	///
	/// e
	///
	/// s
	///
	///  
	///
	/// v
	///
	/// e
	///
	/// c
	///
	/// t
	///
	/// o
	///
	/// r
	///
	///  
	///
	/// s
	///
	/// c
	///
	/// a
	///
	/// l
	///
	/// a
	///
	/// r
	///
	///  
	///
	/// S
	///
	/// H
	///
	/// A
	///
	/// 1
	///
	/// H
	///
	///  
	///
	/// i
	///
	/// n
	///
	/// s
	///
	/// t
	///
	/// r
	///
	/// u
	///
	/// c
	///
	/// t
	///
	/// i
	///
	/// o
	///
	/// n
	IRPair<IROp_VSha1SU1> _VSha1SU1(OrderedNode *Src1, OrderedNode *Src2) {
		auto ListDataBegin = DualListData.ListBegin();
		Src1->AddUse();
		Src2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src1) == RegClass::Invalid || WalkFindRegClass(Src1) == RegClass::FPR, "GetOpRegClass(Src1) == RegClass::Invalid || WalkFindRegClass(Src1) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src2) == RegClass::Invalid || WalkFindRegClass(Src2) == RegClass::FPR, "GetOpRegClass(Src2) == RegClass::Invalid || WalkFindRegClass(Src2) == RegClass::FPR");
#endif
		return _VSha1SU1(Src1->Wrapped(ListDataBegin), Src2->Wrapped(ListDataBegin));
	}

	/// D
	///
	/// o
	///
	/// e
	///
	/// s
	///
	///  
	///
	/// v
	///
	/// e
	///
	/// c
	///
	/// t
	///
	/// o
	///
	/// r
	///
	///  
	///
	/// s
	///
	/// c
	///
	/// a
	///
	/// l
	///
	/// a
	///
	/// r
	///
	///  
	///
	/// V
	///
	/// S
	///
	/// h
	///
	/// a
	///
	/// 2
	///
	/// 5
	///
	/// 6
	///
	/// U
	///
	/// 0
	///
	///  
	///
	/// i
	///
	/// n
	///
	/// s
	///
	/// t
	///
	/// r
	///
	/// u
	///
	/// c
	///
	/// t
	///
	/// i
	///
	/// o
	///
	/// n
	IRPair<IROp_VSha256U0> _VSha256U0(OrderedNodeWrapper Src1, OrderedNodeWrapper Src2) {
		auto _Op = AllocateOp<IROp_VSha256U0, IROps::OP_VSHA256U0>();
		_Op.first->Src1 = Src1;
		_Op.first->Src2 = Src2;
		_Op.first->Header.Size = FEXCore::IR::OpSize::i128Bit;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// D
	///
	/// o
	///
	/// e
	///
	/// s
	///
	///  
	///
	/// v
	///
	/// e
	///
	/// c
	///
	/// t
	///
	/// o
	///
	/// r
	///
	///  
	///
	/// s
	///
	/// c
	///
	/// a
	///
	/// l
	///
	/// a
	///
	/// r
	///
	///  
	///
	/// V
	///
	/// S
	///
	/// h
	///
	/// a
	///
	/// 2
	///
	/// 5
	///
	/// 6
	///
	/// U
	///
	/// 0
	///
	///  
	///
	/// i
	///
	/// n
	///
	/// s
	///
	/// t
	///
	/// r
	///
	/// u
	///
	/// c
	///
	/// t
	///
	/// i
	///
	/// o
	///
	/// n
	IRPair<IROp_VSha256U0> _VSha256U0(OrderedNode *Src1, OrderedNode *Src2) {
		auto ListDataBegin = DualListData.ListBegin();
		Src1->AddUse();
		Src2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src1) == RegClass::Invalid || WalkFindRegClass(Src1) == RegClass::FPR, "GetOpRegClass(Src1) == RegClass::Invalid || WalkFindRegClass(Src1) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src2) == RegClass::Invalid || WalkFindRegClass(Src2) == RegClass::FPR, "GetOpRegClass(Src2) == RegClass::Invalid || WalkFindRegClass(Src2) == RegClass::FPR");
#endif
		return _VSha256U0(Src1->Wrapped(ListDataBegin), Src2->Wrapped(ListDataBegin));
	}

	/// D
	///
	/// o
	///
	/// e
	///
	/// s
	///
	///  
	///
	/// v
	///
	/// e
	///
	/// c
	///
	/// t
	///
	/// o
	///
	/// r
	///
	///  
	///
	/// s
	///
	/// c
	///
	/// a
	///
	/// l
	///
	/// a
	///
	/// r
	///
	///  
	///
	/// V
	///
	/// S
	///
	/// h
	///
	/// a
	///
	/// 2
	///
	/// 5
	///
	/// 6
	///
	/// U
	///
	/// 1
	///
	///  
	///
	/// i
	///
	/// n
	///
	/// s
	///
	/// t
	///
	/// r
	///
	/// u
	///
	/// c
	///
	/// t
	///
	/// i
	///
	/// o
	///
	/// n
	IRPair<IROp_VSha256U1> _VSha256U1(OrderedNodeWrapper Src1, OrderedNodeWrapper Src2) {
		auto _Op = AllocateOp<IROp_VSha256U1, IROps::OP_VSHA256U1>();
		_Op.first->Src1 = Src1;
		_Op.first->Src2 = Src2;
		_Op.first->Header.Size = FEXCore::IR::OpSize::i128Bit;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// D
	///
	/// o
	///
	/// e
	///
	/// s
	///
	///  
	///
	/// v
	///
	/// e
	///
	/// c
	///
	/// t
	///
	/// o
	///
	/// r
	///
	///  
	///
	/// s
	///
	/// c
	///
	/// a
	///
	/// l
	///
	/// a
	///
	/// r
	///
	///  
	///
	/// V
	///
	/// S
	///
	/// h
	///
	/// a
	///
	/// 2
	///
	/// 5
	///
	/// 6
	///
	/// U
	///
	/// 1
	///
	///  
	///
	/// i
	///
	/// n
	///
	/// s
	///
	/// t
	///
	/// r
	///
	/// u
	///
	/// c
	///
	/// t
	///
	/// i
	///
	/// o
	///
	/// n
	IRPair<IROp_VSha256U1> _VSha256U1(OrderedNode *Src1, OrderedNode *Src2) {
		auto ListDataBegin = DualListData.ListBegin();
		Src1->AddUse();
		Src2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src1) == RegClass::Invalid || WalkFindRegClass(Src1) == RegClass::FPR, "GetOpRegClass(Src1) == RegClass::Invalid || WalkFindRegClass(Src1) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src2) == RegClass::Invalid || WalkFindRegClass(Src2) == RegClass::FPR, "GetOpRegClass(Src2) == RegClass::Invalid || WalkFindRegClass(Src2) == RegClass::FPR");
#endif
		return _VSha256U1(Src1->Wrapped(ListDataBegin), Src2->Wrapped(ListDataBegin));
	}

	/// D
	///
	/// o
	///
	/// e
	///
	/// s
	///
	///  
	///
	/// v
	///
	/// e
	///
	/// c
	///
	/// t
	///
	/// o
	///
	/// r
	///
	///  
	///
	/// s
	///
	/// c
	///
	/// a
	///
	/// l
	///
	/// a
	///
	/// r
	///
	///  
	///
	/// V
	///
	/// S
	///
	/// h
	///
	/// a
	///
	/// 2
	///
	/// 5
	///
	/// 6
	///
	/// H
	///
	///  
	///
	/// i
	///
	/// n
	///
	/// s
	///
	/// t
	///
	/// r
	///
	/// u
	///
	/// c
	///
	/// t
	///
	/// i
	///
	/// o
	///
	/// n
	IRPair<IROp_VSha256H> _VSha256H(OrderedNodeWrapper Src1, OrderedNodeWrapper Src2, OrderedNodeWrapper Src3) {
		auto _Op = AllocateOp<IROp_VSha256H, IROps::OP_VSHA256H>();
		_Op.first->Src1 = Src1;
		_Op.first->Src2 = Src2;
		_Op.first->Src3 = Src3;
		_Op.first->Header.Size = FEXCore::IR::OpSize::i128Bit;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// D
	///
	/// o
	///
	/// e
	///
	/// s
	///
	///  
	///
	/// v
	///
	/// e
	///
	/// c
	///
	/// t
	///
	/// o
	///
	/// r
	///
	///  
	///
	/// s
	///
	/// c
	///
	/// a
	///
	/// l
	///
	/// a
	///
	/// r
	///
	///  
	///
	/// V
	///
	/// S
	///
	/// h
	///
	/// a
	///
	/// 2
	///
	/// 5
	///
	/// 6
	///
	/// H
	///
	///  
	///
	/// i
	///
	/// n
	///
	/// s
	///
	/// t
	///
	/// r
	///
	/// u
	///
	/// c
	///
	/// t
	///
	/// i
	///
	/// o
	///
	/// n
	IRPair<IROp_VSha256H> _VSha256H(OrderedNode *Src1, OrderedNode *Src2, OrderedNode *Src3) {
		auto ListDataBegin = DualListData.ListBegin();
		Src1->AddUse();
		Src2->AddUse();
		Src3->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src1) == RegClass::Invalid || WalkFindRegClass(Src1) == RegClass::FPR, "GetOpRegClass(Src1) == RegClass::Invalid || WalkFindRegClass(Src1) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src2) == RegClass::Invalid || WalkFindRegClass(Src2) == RegClass::FPR, "GetOpRegClass(Src2) == RegClass::Invalid || WalkFindRegClass(Src2) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src3) == RegClass::Invalid || WalkFindRegClass(Src3) == RegClass::FPR, "GetOpRegClass(Src3) == RegClass::Invalid || WalkFindRegClass(Src3) == RegClass::FPR");
#endif
		return _VSha256H(Src1->Wrapped(ListDataBegin), Src2->Wrapped(ListDataBegin), Src3->Wrapped(ListDataBegin));
	}

	/// D
	///
	/// o
	///
	/// e
	///
	/// s
	///
	///  
	///
	/// v
	///
	/// e
	///
	/// c
	///
	/// t
	///
	/// o
	///
	/// r
	///
	///  
	///
	/// s
	///
	/// c
	///
	/// a
	///
	/// l
	///
	/// a
	///
	/// r
	///
	///  
	///
	/// V
	///
	/// S
	///
	/// h
	///
	/// a
	///
	/// 2
	///
	/// 5
	///
	/// 6
	///
	/// H
	///
	/// 2
	///
	///  
	///
	/// i
	///
	/// n
	///
	/// s
	///
	/// t
	///
	/// r
	///
	/// u
	///
	/// c
	///
	/// t
	///
	/// i
	///
	/// o
	///
	/// n
	IRPair<IROp_VSha256H2> _VSha256H2(OrderedNodeWrapper Src1, OrderedNodeWrapper Src2, OrderedNodeWrapper Src3) {
		auto _Op = AllocateOp<IROp_VSha256H2, IROps::OP_VSHA256H2>();
		_Op.first->Src1 = Src1;
		_Op.first->Src2 = Src2;
		_Op.first->Src3 = Src3;
		_Op.first->Header.Size = FEXCore::IR::OpSize::i128Bit;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// D
	///
	/// o
	///
	/// e
	///
	/// s
	///
	///  
	///
	/// v
	///
	/// e
	///
	/// c
	///
	/// t
	///
	/// o
	///
	/// r
	///
	///  
	///
	/// s
	///
	/// c
	///
	/// a
	///
	/// l
	///
	/// a
	///
	/// r
	///
	///  
	///
	/// V
	///
	/// S
	///
	/// h
	///
	/// a
	///
	/// 2
	///
	/// 5
	///
	/// 6
	///
	/// H
	///
	/// 2
	///
	///  
	///
	/// i
	///
	/// n
	///
	/// s
	///
	/// t
	///
	/// r
	///
	/// u
	///
	/// c
	///
	/// t
	///
	/// i
	///
	/// o
	///
	/// n
	IRPair<IROp_VSha256H2> _VSha256H2(OrderedNode *Src1, OrderedNode *Src2, OrderedNode *Src3) {
		auto ListDataBegin = DualListData.ListBegin();
		Src1->AddUse();
		Src2->AddUse();
		Src3->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src1) == RegClass::Invalid || WalkFindRegClass(Src1) == RegClass::FPR, "GetOpRegClass(Src1) == RegClass::Invalid || WalkFindRegClass(Src1) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src2) == RegClass::Invalid || WalkFindRegClass(Src2) == RegClass::FPR, "GetOpRegClass(Src2) == RegClass::Invalid || WalkFindRegClass(Src2) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src3) == RegClass::Invalid || WalkFindRegClass(Src3) == RegClass::FPR, "GetOpRegClass(Src3) == RegClass::Invalid || WalkFindRegClass(Src3) == RegClass::FPR");
#endif
		return _VSha256H2(Src1->Wrapped(ListDataBegin), Src2->Wrapped(ListDataBegin), Src3->Wrapped(ListDataBegin));
	}

	/// CRC32 using polynomial 0x1EDC6F41
	IRPair<IROp_CRC32> _CRC32(OrderedNodeWrapper Src1, OrderedNodeWrapper Src2, FEXCore::IR::OpSize SrcSize) {
		auto _Op = AllocateOp<IROp_CRC32, IROps::OP_CRC32>();
		_Op.first->Src1 = Src1;
		_Op.first->Src2 = Src2;
		_Op.first->SrcSize = SrcSize;
		_Op.first->Header.Size = OpSize::i32Bit;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// CRC32 using polynomial 0x1EDC6F41
	IRPair<IROp_CRC32> _CRC32(OrderedNode *Src1, OrderedNode *Src2, FEXCore::IR::OpSize SrcSize) {
		auto ListDataBegin = DualListData.ListBegin();
		Src1->AddUse();
		Src2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src1) == RegClass::Invalid || WalkFindRegClass(Src1) == RegClass::GPR, "GetOpRegClass(Src1) == RegClass::Invalid || WalkFindRegClass(Src1) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src2) == RegClass::Invalid || WalkFindRegClass(Src2) == RegClass::GPR, "GetOpRegClass(Src2) == RegClass::Invalid || WalkFindRegClass(Src2) == RegClass::GPR");
#endif
		return _CRC32(Src1->Wrapped(ListDataBegin), Src2->Wrapped(ListDataBegin), SrcSize);
	}

	/// Performs carryless multiplication of 64-bit elements depending on the selector.
	///
	/// Selector = 0b00000000: Uses low 64-bit elements from both input vectors
	///
	/// Selector = 0b00000001: Uses high 64-bit element from Src1 and low 64-bit element from Src2
	///
	/// Selector = 0b00010000: Uses low 64-bit element from Src1 and high 64-bit element from Src2
	///
	/// Selector = 0b00010001: Uses high 64-bit elements from both input vectors
	IRPair<IROp_PCLMUL> _PCLMUL(FEXCore::IR::OpSize RegisterSize, OrderedNodeWrapper Src1, OrderedNodeWrapper Src2, uint8_t Selector) {
		auto _Op = AllocateOp<IROp_PCLMUL, IROps::OP_PCLMUL>();
		_Op.first->Src1 = Src1;
		_Op.first->Src2 = Src2;
		_Op.first->Selector = Selector;
		_Op.first->Header.Size = RegisterSize;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Performs carryless multiplication of 64-bit elements depending on the selector.
	///
	/// Selector = 0b00000000: Uses low 64-bit elements from both input vectors
	///
	/// Selector = 0b00000001: Uses high 64-bit element from Src1 and low 64-bit element from Src2
	///
	/// Selector = 0b00010000: Uses low 64-bit element from Src1 and high 64-bit element from Src2
	///
	/// Selector = 0b00010001: Uses high 64-bit elements from both input vectors
	IRPair<IROp_PCLMUL> _PCLMUL(FEXCore::IR::OpSize RegisterSize, OrderedNode *Src1, OrderedNode *Src2, uint8_t Selector) {
		auto ListDataBegin = DualListData.ListBegin();
		Src1->AddUse();
		Src2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src1) == RegClass::Invalid || WalkFindRegClass(Src1) == RegClass::FPR, "GetOpRegClass(Src1) == RegClass::Invalid || WalkFindRegClass(Src1) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src2) == RegClass::Invalid || WalkFindRegClass(Src2) == RegClass::FPR, "GetOpRegClass(Src2) == RegClass::Invalid || WalkFindRegClass(Src2) == RegClass::FPR");
#endif
		return _PCLMUL(RegisterSize, Src1->Wrapped(ListDataBegin), Src2->Wrapped(ListDataBegin), Selector);
	}

	IRPair<IROp_F64ATAN> _F64ATAN(OrderedNodeWrapper Src1, OrderedNodeWrapper Src2) {
		auto _Op = AllocateOp<IROp_F64ATAN, IROps::OP_F64ATAN>();
		_Op.first->Src1 = Src1;
		_Op.first->Src2 = Src2;
		_Op.first->Header.Size = OpSize::i64Bit;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	IRPair<IROp_F64ATAN> _F64ATAN(OrderedNode *Src1, OrderedNode *Src2) {
		auto ListDataBegin = DualListData.ListBegin();
		Src1->AddUse();
		Src2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src1) == RegClass::Invalid || WalkFindRegClass(Src1) == RegClass::FPR, "GetOpRegClass(Src1) == RegClass::Invalid || WalkFindRegClass(Src1) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src2) == RegClass::Invalid || WalkFindRegClass(Src2) == RegClass::FPR, "GetOpRegClass(Src2) == RegClass::Invalid || WalkFindRegClass(Src2) == RegClass::FPR");
#endif
		return _F64ATAN(Src1->Wrapped(ListDataBegin), Src2->Wrapped(ListDataBegin));
	}

	IRPair<IROp_F64FPREM> _F64FPREM(OrderedNodeWrapper Src1, OrderedNodeWrapper Src2) {
		auto _Op = AllocateOp<IROp_F64FPREM, IROps::OP_F64FPREM>();
		_Op.first->Src1 = Src1;
		_Op.first->Src2 = Src2;
		_Op.first->Header.Size = OpSize::i64Bit;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	IRPair<IROp_F64FPREM> _F64FPREM(OrderedNode *Src1, OrderedNode *Src2) {
		auto ListDataBegin = DualListData.ListBegin();
		Src1->AddUse();
		Src2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src1) == RegClass::Invalid || WalkFindRegClass(Src1) == RegClass::FPR, "GetOpRegClass(Src1) == RegClass::Invalid || WalkFindRegClass(Src1) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src2) == RegClass::Invalid || WalkFindRegClass(Src2) == RegClass::FPR, "GetOpRegClass(Src2) == RegClass::Invalid || WalkFindRegClass(Src2) == RegClass::FPR");
#endif
		return _F64FPREM(Src1->Wrapped(ListDataBegin), Src2->Wrapped(ListDataBegin));
	}

	IRPair<IROp_F64FPREM1> _F64FPREM1(OrderedNodeWrapper Src1, OrderedNodeWrapper Src2) {
		auto _Op = AllocateOp<IROp_F64FPREM1, IROps::OP_F64FPREM1>();
		_Op.first->Src1 = Src1;
		_Op.first->Src2 = Src2;
		_Op.first->Header.Size = OpSize::i64Bit;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	IRPair<IROp_F64FPREM1> _F64FPREM1(OrderedNode *Src1, OrderedNode *Src2) {
		auto ListDataBegin = DualListData.ListBegin();
		Src1->AddUse();
		Src2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src1) == RegClass::Invalid || WalkFindRegClass(Src1) == RegClass::FPR, "GetOpRegClass(Src1) == RegClass::Invalid || WalkFindRegClass(Src1) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src2) == RegClass::Invalid || WalkFindRegClass(Src2) == RegClass::FPR, "GetOpRegClass(Src2) == RegClass::Invalid || WalkFindRegClass(Src2) == RegClass::FPR");
#endif
		return _F64FPREM1(Src1->Wrapped(ListDataBegin), Src2->Wrapped(ListDataBegin));
	}

	IRPair<IROp_F64SCALE> _F64SCALE(OrderedNodeWrapper Src1, OrderedNodeWrapper Src2) {
		auto _Op = AllocateOp<IROp_F64SCALE, IROps::OP_F64SCALE>();
		_Op.first->Src1 = Src1;
		_Op.first->Src2 = Src2;
		_Op.first->Header.Size = OpSize::i64Bit;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	IRPair<IROp_F64SCALE> _F64SCALE(OrderedNode *Src1, OrderedNode *Src2) {
		auto ListDataBegin = DualListData.ListBegin();
		Src1->AddUse();
		Src2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src1) == RegClass::Invalid || WalkFindRegClass(Src1) == RegClass::FPR, "GetOpRegClass(Src1) == RegClass::Invalid || WalkFindRegClass(Src1) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src2) == RegClass::Invalid || WalkFindRegClass(Src2) == RegClass::FPR, "GetOpRegClass(Src2) == RegClass::Invalid || WalkFindRegClass(Src2) == RegClass::FPR");
#endif
		return _F64SCALE(Src1->Wrapped(ListDataBegin), Src2->Wrapped(ListDataBegin));
	}

	IRPair<IROp_F64F2XM1> _F64F2XM1(OrderedNodeWrapper Src) {
		auto _Op = AllocateOp<IROp_F64F2XM1, IROps::OP_F64F2XM1>();
		_Op.first->Src = Src;
		_Op.first->Header.Size = OpSize::i64Bit;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	IRPair<IROp_F64F2XM1> _F64F2XM1(OrderedNode *Src) {
		auto ListDataBegin = DualListData.ListBegin();
		Src->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src) == RegClass::Invalid || WalkFindRegClass(Src) == RegClass::FPR, "GetOpRegClass(Src) == RegClass::Invalid || WalkFindRegClass(Src) == RegClass::FPR");
#endif
		return _F64F2XM1(Src->Wrapped(ListDataBegin));
	}

	IRPair<IROp_F64FYL2X> _F64FYL2X(OrderedNodeWrapper Src, OrderedNodeWrapper Src2) {
		auto _Op = AllocateOp<IROp_F64FYL2X, IROps::OP_F64FYL2X>();
		_Op.first->Src = Src;
		_Op.first->Src2 = Src2;
		_Op.first->Header.Size = OpSize::i64Bit;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	IRPair<IROp_F64FYL2X> _F64FYL2X(OrderedNode *Src, OrderedNode *Src2) {
		auto ListDataBegin = DualListData.ListBegin();
		Src->AddUse();
		Src2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src) == RegClass::Invalid || WalkFindRegClass(Src) == RegClass::FPR, "GetOpRegClass(Src) == RegClass::Invalid || WalkFindRegClass(Src) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src2) == RegClass::Invalid || WalkFindRegClass(Src2) == RegClass::FPR, "GetOpRegClass(Src2) == RegClass::Invalid || WalkFindRegClass(Src2) == RegClass::FPR");
#endif
		return _F64FYL2X(Src->Wrapped(ListDataBegin), Src2->Wrapped(ListDataBegin));
	}

	IRPair<IROp_F64FYL2XP1> _F64FYL2XP1(OrderedNodeWrapper Src, OrderedNodeWrapper Src2) {
		auto _Op = AllocateOp<IROp_F64FYL2XP1, IROps::OP_F64FYL2XP1>();
		_Op.first->Src = Src;
		_Op.first->Src2 = Src2;
		_Op.first->Header.Size = OpSize::i64Bit;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	IRPair<IROp_F64FYL2XP1> _F64FYL2XP1(OrderedNode *Src, OrderedNode *Src2) {
		auto ListDataBegin = DualListData.ListBegin();
		Src->AddUse();
		Src2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src) == RegClass::Invalid || WalkFindRegClass(Src) == RegClass::FPR, "GetOpRegClass(Src) == RegClass::Invalid || WalkFindRegClass(Src) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src2) == RegClass::Invalid || WalkFindRegClass(Src2) == RegClass::FPR, "GetOpRegClass(Src2) == RegClass::Invalid || WalkFindRegClass(Src2) == RegClass::FPR");
#endif
		return _F64FYL2XP1(Src->Wrapped(ListDataBegin), Src2->Wrapped(ListDataBegin));
	}

	IRPair<IROp_F64TAN> _F64TAN(OrderedNodeWrapper Src) {
		auto _Op = AllocateOp<IROp_F64TAN, IROps::OP_F64TAN>();
		_Op.first->Src = Src;
		_Op.first->Header.Size = OpSize::i64Bit;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	IRPair<IROp_F64TAN> _F64TAN(OrderedNode *Src) {
		auto ListDataBegin = DualListData.ListBegin();
		Src->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src) == RegClass::Invalid || WalkFindRegClass(Src) == RegClass::FPR, "GetOpRegClass(Src) == RegClass::Invalid || WalkFindRegClass(Src) == RegClass::FPR");
#endif
		return _F64TAN(Src->Wrapped(ListDataBegin));
	}

	IRPair<IROp_F64SIN> _F64SIN(OrderedNodeWrapper Src) {
		auto _Op = AllocateOp<IROp_F64SIN, IROps::OP_F64SIN>();
		_Op.first->Src = Src;
		_Op.first->Header.Size = OpSize::i64Bit;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	IRPair<IROp_F64SIN> _F64SIN(OrderedNode *Src) {
		auto ListDataBegin = DualListData.ListBegin();
		Src->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src) == RegClass::Invalid || WalkFindRegClass(Src) == RegClass::FPR, "GetOpRegClass(Src) == RegClass::Invalid || WalkFindRegClass(Src) == RegClass::FPR");
#endif
		return _F64SIN(Src->Wrapped(ListDataBegin));
	}

	IRPair<IROp_F64COS> _F64COS(OrderedNodeWrapper Src) {
		auto _Op = AllocateOp<IROp_F64COS, IROps::OP_F64COS>();
		_Op.first->Src = Src;
		_Op.first->Header.Size = OpSize::i64Bit;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	IRPair<IROp_F64COS> _F64COS(OrderedNode *Src) {
		auto ListDataBegin = DualListData.ListBegin();
		Src->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src) == RegClass::Invalid || WalkFindRegClass(Src) == RegClass::FPR, "GetOpRegClass(Src) == RegClass::Invalid || WalkFindRegClass(Src) == RegClass::FPR");
#endif
		return _F64COS(Src->Wrapped(ListDataBegin));
	}

	IRPair<IROp_F64SINCOS> _F64SINCOS(OrderedNodeWrapper Src, OrderedNodeWrapper OutSin, OrderedNodeWrapper OutCos) {
		auto _Op = AllocateOp<IROp_F64SINCOS, IROps::OP_F64SINCOS>();
		_Op.first->Src = Src;
		_Op.first->OutSin = OutSin;
		_Op.first->OutCos = OutCos;
		_Op.first->Header.Size = OpSize::i64Bit;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	IRPair<IROp_F64SINCOS> _F64SINCOS(OrderedNode *Src, OrderedNode *OutSin, OrderedNode *OutCos) {
		auto ListDataBegin = DualListData.ListBegin();
		Src->AddUse();
		OutSin->AddUse();
		OutCos->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src) == RegClass::Invalid || WalkFindRegClass(Src) == RegClass::FPR, "GetOpRegClass(Src) == RegClass::Invalid || WalkFindRegClass(Src) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(OutSin) == RegClass::Invalid || WalkFindRegClass(OutSin) == RegClass::FPR, "GetOpRegClass(OutSin) == RegClass::Invalid || WalkFindRegClass(OutSin) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(OutCos) == RegClass::Invalid || WalkFindRegClass(OutCos) == RegClass::FPR, "GetOpRegClass(OutCos) == RegClass::Invalid || WalkFindRegClass(OutCos) == RegClass::FPR");
#endif
		return _F64SINCOS(Src->Wrapped(ListDataBegin), OutSin->Wrapped(ListDataBegin), OutCos->Wrapped(ListDataBegin));
	}

	/// Synchronizes the virtual stack environment to the physical registers.
	///
	/// Returns the current stack top.
	IRPair<IROp_SyncStackToSlow> _SyncStackToSlow() {
		RecordX87Use();
		if(MMXState == MMXState_MMX) ChgStateMMX_X87();
		auto _Op = AllocateOp<IROp_SyncStackToSlow, IROps::OP_SYNCSTACKTOSLOW>();
		_Op.first->Header.Size = OpSize::i64Bit;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Forces the slow path.
	IRPair<IROp_StackForceSlow> _StackForceSlow() {
		RecordX87Use();
		if(MMXState == MMXState_MMX) ChgStateMMX_X87();
		auto _Op = AllocateOp<IROp_StackForceSlow, IROps::OP_STACKFORCESLOW>();
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Initializes the stack by marking all tags as invalid and setting top to zero.
	IRPair<IROp_InitStack> _InitStack() {
		RecordX87Use();
		if(MMXState == MMXState_MMX) ChgStateMMX_X87();
		auto _Op = AllocateOp<IROp_InitStack, IROps::OP_INITSTACK>();
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Increase stack top-pointer.
	IRPair<IROp_IncStackTop> _IncStackTop() {
		RecordX87Use();
		if(MMXState == MMXState_MMX) ChgStateMMX_X87();
		auto _Op = AllocateOp<IROp_IncStackTop, IROps::OP_INCSTACKTOP>();
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Decrease stack top-pointer.
	IRPair<IROp_DecStackTop> _DecStackTop() {
		RecordX87Use();
		if(MMXState == MMXState_MMX) ChgStateMMX_X87();
		auto _Op = AllocateOp<IROp_DecStackTop, IROps::OP_DECSTACKTOP>();
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Marks the value in TOP+$StackLocation as empty / invalid 0b11.
	///
	/// If the StackLocation is 0xff, we invalidate all locations.
	IRPair<IROp_InvalidateStack> _InvalidateStack(uint8_t StackLocation) {
		RecordX87Use();
		if(MMXState == MMXState_MMX) ChgStateMMX_X87();
		auto _Op = AllocateOp<IROp_InvalidateStack, IROps::OP_INVALIDATESTACK>();
		_Op.first->StackLocation = StackLocation;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Pushes the provided X80Src source on to the x87 stack.
	///
	/// Tracks OriginalValue as the original value of X80Src. OriginalValue can be Invalid() in which case no tracking is done.
	///
	/// Opsize is 128bit for F80 values, 64-bit for low precision.
	///
	/// LoadSize the original load size, i.e. of size of OriginalValue.
	///
	/// Float: 80-bit, 64-bit, 32-bit
	IRPair<IROp_PushStack> _PushStack(OrderedNodeWrapper X80Src, OrderedNodeWrapper OriginalValue, FEXCore::IR::OpSize LoadSize) {
		RecordX87Use();
		if(MMXState == MMXState_MMX) ChgStateMMX_X87();
		auto _Op = AllocateOp<IROp_PushStack, IROps::OP_PUSHSTACK>();
		_Op.first->X80Src = X80Src;
		_Op.first->OriginalValue = OriginalValue;
		_Op.first->LoadSize = LoadSize;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Pushes the provided X80Src source on to the x87 stack.
	///
	/// Tracks OriginalValue as the original value of X80Src. OriginalValue can be Invalid() in which case no tracking is done.
	///
	/// Opsize is 128bit for F80 values, 64-bit for low precision.
	///
	/// LoadSize the original load size, i.e. of size of OriginalValue.
	///
	/// Float: 80-bit, 64-bit, 32-bit
	IRPair<IROp_PushStack> _PushStack(OrderedNode *X80Src, OrderedNode *OriginalValue, FEXCore::IR::OpSize LoadSize) {
		auto ListDataBegin = DualListData.ListBegin();
		X80Src->AddUse();
		OriginalValue->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(X80Src) == RegClass::Invalid || WalkFindRegClass(X80Src) == RegClass::FPR, "GetOpRegClass(X80Src) == RegClass::Invalid || WalkFindRegClass(X80Src) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(OriginalValue) == RegClass::Invalid || WalkFindRegClass(OriginalValue) == RegClass::FPR, "GetOpRegClass(OriginalValue) == RegClass::Invalid || WalkFindRegClass(OriginalValue) == RegClass::FPR");
#endif
		return _PushStack(X80Src->Wrapped(ListDataBegin), OriginalValue->Wrapped(ListDataBegin), LoadSize);
	}

	/// Pushes an element already on the stack onto the top.
	IRPair<IROp_CopyPushStack> _CopyPushStack(uint8_t StackLocation) {
		RecordX87Use();
		if(MMXState == MMXState_MMX) ChgStateMMX_X87();
		auto _Op = AllocateOp<IROp_CopyPushStack, IROps::OP_COPYPUSHSTACK>();
		_Op.first->StackLocation = StackLocation;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Takes the top value off the x87 stack and stores it to memory.
	///
	/// SourceSize is 128bit for F80 values, 64-bit for low precision.
	///
	/// StoreSize is the store size for conversion:
	///
	/// Float: 80-bit, 64-bit, or 32-bit
	IRPair<IROp_StoreStackMem> _StoreStackMem(FEXCore::IR::OpSize SourceSize, FEXCore::IR::OpSize StoreSize, OrderedNodeWrapper Addr, OrderedNodeWrapper Offset, FEXCore::IR::OpSize Align, MemOffsetType OffsetType, uint8_t OffsetScale) {
		RecordX87Use();
		if(MMXState == MMXState_MMX) ChgStateMMX_X87();
		auto _Op = AllocateOp<IROp_StoreStackMem, IROps::OP_STORESTACKMEM>();
		_Op.first->Addr = Addr;
		_Op.first->Offset = Offset;
		_Op.first->SourceSize = SourceSize;
		_Op.first->StoreSize = StoreSize;
		_Op.first->Align = Align;
		_Op.first->OffsetType = OffsetType;
		_Op.first->OffsetScale = OffsetScale;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Takes the top value off the x87 stack and stores it to memory.
	///
	/// SourceSize is 128bit for F80 values, 64-bit for low precision.
	///
	/// StoreSize is the store size for conversion:
	///
	/// Float: 80-bit, 64-bit, or 32-bit
	IRPair<IROp_StoreStackMem> _StoreStackMem(FEXCore::IR::OpSize SourceSize, FEXCore::IR::OpSize StoreSize, OrderedNode *Addr, OrderedNode *Offset, FEXCore::IR::OpSize Align, MemOffsetType OffsetType, uint8_t OffsetScale) {
		auto ListDataBegin = DualListData.ListBegin();
		Addr->AddUse();
		Offset->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Addr) == RegClass::Invalid || WalkFindRegClass(Addr) == RegClass::GPR, "GetOpRegClass(Addr) == RegClass::Invalid || WalkFindRegClass(Addr) == RegClass::GPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(Offset) == RegClass::Invalid || WalkFindRegClass(Offset) == RegClass::GPR, "GetOpRegClass(Offset) == RegClass::Invalid || WalkFindRegClass(Offset) == RegClass::GPR");
#endif
		return _StoreStackMem(SourceSize, StoreSize, Addr->Wrapped(ListDataBegin), Offset->Wrapped(ListDataBegin), Align, OffsetType, OffsetScale);
	}

	/// Takes the top value off the x87 stack and stores it to stack location TOP+StackLocation
	///
	/// Float: 80-bit, 64-bit, or 32-bit
	///
	/// Int: 64-bit, 32-bit, 16-bit
	IRPair<IROp_StoreStackToStack> _StoreStackToStack(uint8_t StackLocation) {
		RecordX87Use();
		if(MMXState == MMXState_MMX) ChgStateMMX_X87();
		auto _Op = AllocateOp<IROp_StoreStackToStack, IROps::OP_STORESTACKTOSTACK>();
		_Op.first->StackLocation = StackLocation;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Pops the top value off the stack but doesn't save it anywhere.
	IRPair<IROp_PopStackDestroy> _PopStackDestroy() {
		RecordX87Use();
		if(MMXState == MMXState_MMX) ChgStateMMX_X87();
		auto _Op = AllocateOp<IROp_PopStackDestroy, IROps::OP_POPSTACKDESTROY>();
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Reads a value off the stack at the offset
	IRPair<IROp_ReadStackValue> _ReadStackValue(uint8_t StackLocation) {
		RecordX87Use();
		if(MMXState == MMXState_MMX) ChgStateMMX_X87();
		auto _Op = AllocateOp<IROp_ReadStackValue, IROps::OP_READSTACKVALUE>();
		_Op.first->StackLocation = StackLocation;
		_Op.first->Header.Size = OpSize::i128Bit;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Returns 1 if the value in location TOP+$StackLocation is valid, 0 otherwise.
	IRPair<IROp_StackValidTag> _StackValidTag(uint8_t StackLocation) {
		RecordX87Use();
		if(MMXState == MMXState_MMX) ChgStateMMX_X87();
		auto _Op = AllocateOp<IROp_StackValidTag, IROps::OP_STACKVALIDTAG>();
		_Op.first->StackLocation = StackLocation;
		_Op.first->Header.Size = OpSize::i32Bit;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Adds two stack locations together, storing the result in to the first stack location
	IRPair<IROp_F80AddStack> _F80AddStack(uint8_t SrcStack1, uint8_t SrcStack2) {
		RecordX87Use();
		if(MMXState == MMXState_MMX) ChgStateMMX_X87();
		auto _Op = AllocateOp<IROp_F80AddStack, IROps::OP_F80ADDSTACK>();
		_Op.first->SrcStack1 = SrcStack1;
		_Op.first->SrcStack2 = SrcStack2;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Adds a operand value to a stack location. The result stored in to the stack location provided.
	IRPair<IROp_F80AddValue> _F80AddValue(uint8_t SrcStack, OrderedNodeWrapper X80Src) {
		RecordX87Use();
		if(MMXState == MMXState_MMX) ChgStateMMX_X87();
		auto _Op = AllocateOp<IROp_F80AddValue, IROps::OP_F80ADDVALUE>();
		_Op.first->X80Src = X80Src;
		_Op.first->SrcStack = SrcStack;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Adds a operand value to a stack location. The result stored in to the stack location provided.
	IRPair<IROp_F80AddValue> _F80AddValue(uint8_t SrcStack, OrderedNode *X80Src) {
		auto ListDataBegin = DualListData.ListBegin();
		X80Src->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(X80Src) == RegClass::Invalid || WalkFindRegClass(X80Src) == RegClass::FPR, "GetOpRegClass(X80Src) == RegClass::Invalid || WalkFindRegClass(X80Src) == RegClass::FPR");
#endif
		return _F80AddValue(SrcStack, X80Src->Wrapped(ListDataBegin));
	}

	IRPair<IROp_F80Add> _F80Add(OrderedNodeWrapper X80Src1, OrderedNodeWrapper X80Src2) {
		auto _Op = AllocateOp<IROp_F80Add, IROps::OP_F80ADD>();
		_Op.first->X80Src1 = X80Src1;
		_Op.first->X80Src2 = X80Src2;
		_Op.first->Header.Size = OpSize::i128Bit;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	IRPair<IROp_F80Add> _F80Add(OrderedNode *X80Src1, OrderedNode *X80Src2) {
		auto ListDataBegin = DualListData.ListBegin();
		X80Src1->AddUse();
		X80Src2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(X80Src1) == RegClass::Invalid || WalkFindRegClass(X80Src1) == RegClass::FPR, "GetOpRegClass(X80Src1) == RegClass::Invalid || WalkFindRegClass(X80Src1) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(X80Src2) == RegClass::Invalid || WalkFindRegClass(X80Src2) == RegClass::FPR, "GetOpRegClass(X80Src2) == RegClass::Invalid || WalkFindRegClass(X80Src2) == RegClass::FPR");
#endif
		return _F80Add(X80Src1->Wrapped(ListDataBegin), X80Src2->Wrapped(ListDataBegin));
	}

	/// Subtracts the value in stack location TOP+$SrcStack2 from the value in stack location TOP+$SrcStack1.
	///
	/// The result is stored in stack location TOP+$DstStack.
	IRPair<IROp_F80SubStack> _F80SubStack(uint8_t DstStack, uint8_t SrcStack1, uint8_t SrcStack2) {
		RecordX87Use();
		if(MMXState == MMXState_MMX) ChgStateMMX_X87();
		auto _Op = AllocateOp<IROp_F80SubStack, IROps::OP_F80SUBSTACK>();
		_Op.first->DstStack = DstStack;
		_Op.first->SrcStack1 = SrcStack1;
		_Op.first->SrcStack2 = SrcStack2;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Subtracts the value $X80Src from the value in stack location TOP+$SrcStack.
	///
	/// The result is stored in stack location TOP.
	IRPair<IROp_F80SubValue> _F80SubValue(uint8_t SrcStack, OrderedNodeWrapper X80Src) {
		RecordX87Use();
		if(MMXState == MMXState_MMX) ChgStateMMX_X87();
		auto _Op = AllocateOp<IROp_F80SubValue, IROps::OP_F80SUBVALUE>();
		_Op.first->X80Src = X80Src;
		_Op.first->SrcStack = SrcStack;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Subtracts the value $X80Src from the value in stack location TOP+$SrcStack.
	///
	/// The result is stored in stack location TOP.
	IRPair<IROp_F80SubValue> _F80SubValue(uint8_t SrcStack, OrderedNode *X80Src) {
		auto ListDataBegin = DualListData.ListBegin();
		X80Src->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(X80Src) == RegClass::Invalid || WalkFindRegClass(X80Src) == RegClass::FPR, "GetOpRegClass(X80Src) == RegClass::Invalid || WalkFindRegClass(X80Src) == RegClass::FPR");
#endif
		return _F80SubValue(SrcStack, X80Src->Wrapped(ListDataBegin));
	}

	/// Subtracts the value in stack location TOP+$SrcStack from the value $X80Src.
	///
	/// The result is stored in stack location TOP.
	IRPair<IROp_F80SubRValue> _F80SubRValue(OrderedNodeWrapper X80Src, uint8_t SrcStack) {
		RecordX87Use();
		if(MMXState == MMXState_MMX) ChgStateMMX_X87();
		auto _Op = AllocateOp<IROp_F80SubRValue, IROps::OP_F80SUBRVALUE>();
		_Op.first->X80Src = X80Src;
		_Op.first->SrcStack = SrcStack;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Subtracts the value in stack location TOP+$SrcStack from the value $X80Src.
	///
	/// The result is stored in stack location TOP.
	IRPair<IROp_F80SubRValue> _F80SubRValue(OrderedNode *X80Src, uint8_t SrcStack) {
		auto ListDataBegin = DualListData.ListBegin();
		X80Src->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(X80Src) == RegClass::Invalid || WalkFindRegClass(X80Src) == RegClass::FPR, "GetOpRegClass(X80Src) == RegClass::Invalid || WalkFindRegClass(X80Src) == RegClass::FPR");
#endif
		return _F80SubRValue(X80Src->Wrapped(ListDataBegin), SrcStack);
	}

	/// Subtracts the value in $X80Src1 from the value in $X80Src2.
	///
	/// The result is returned.
	///
	/// `FPR = X80Src2 - X80Src1`
	IRPair<IROp_F80Sub> _F80Sub(OrderedNodeWrapper X80Src1, OrderedNodeWrapper X80Src2) {
		auto _Op = AllocateOp<IROp_F80Sub, IROps::OP_F80SUB>();
		_Op.first->X80Src1 = X80Src1;
		_Op.first->X80Src2 = X80Src2;
		_Op.first->Header.Size = OpSize::i128Bit;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Subtracts the value in $X80Src1 from the value in $X80Src2.
	///
	/// The result is returned.
	///
	/// `FPR = X80Src2 - X80Src1`
	IRPair<IROp_F80Sub> _F80Sub(OrderedNode *X80Src1, OrderedNode *X80Src2) {
		auto ListDataBegin = DualListData.ListBegin();
		X80Src1->AddUse();
		X80Src2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(X80Src1) == RegClass::Invalid || WalkFindRegClass(X80Src1) == RegClass::FPR, "GetOpRegClass(X80Src1) == RegClass::Invalid || WalkFindRegClass(X80Src1) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(X80Src2) == RegClass::Invalid || WalkFindRegClass(X80Src2) == RegClass::FPR, "GetOpRegClass(X80Src2) == RegClass::Invalid || WalkFindRegClass(X80Src2) == RegClass::FPR");
#endif
		return _F80Sub(X80Src1->Wrapped(ListDataBegin), X80Src2->Wrapped(ListDataBegin));
	}

	/// Multiplies two stack locations together, storing the result in to the first stack location
	IRPair<IROp_F80MulStack> _F80MulStack(uint8_t SrcStack1, uint8_t SrcStack2) {
		RecordX87Use();
		if(MMXState == MMXState_MMX) ChgStateMMX_X87();
		auto _Op = AllocateOp<IROp_F80MulStack, IROps::OP_F80MULSTACK>();
		_Op.first->SrcStack1 = SrcStack1;
		_Op.first->SrcStack2 = SrcStack2;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Multiplies a operand value to a stack location. The result stored in to the stack location provided.
	IRPair<IROp_F80MulValue> _F80MulValue(uint8_t SrcStack, OrderedNodeWrapper X80Src) {
		RecordX87Use();
		if(MMXState == MMXState_MMX) ChgStateMMX_X87();
		auto _Op = AllocateOp<IROp_F80MulValue, IROps::OP_F80MULVALUE>();
		_Op.first->X80Src = X80Src;
		_Op.first->SrcStack = SrcStack;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Multiplies a operand value to a stack location. The result stored in to the stack location provided.
	IRPair<IROp_F80MulValue> _F80MulValue(uint8_t SrcStack, OrderedNode *X80Src) {
		auto ListDataBegin = DualListData.ListBegin();
		X80Src->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(X80Src) == RegClass::Invalid || WalkFindRegClass(X80Src) == RegClass::FPR, "GetOpRegClass(X80Src) == RegClass::Invalid || WalkFindRegClass(X80Src) == RegClass::FPR");
#endif
		return _F80MulValue(SrcStack, X80Src->Wrapped(ListDataBegin));
	}

	IRPair<IROp_F80Mul> _F80Mul(OrderedNodeWrapper X80Src1, OrderedNodeWrapper X80Src2) {
		auto _Op = AllocateOp<IROp_F80Mul, IROps::OP_F80MUL>();
		_Op.first->X80Src1 = X80Src1;
		_Op.first->X80Src2 = X80Src2;
		_Op.first->Header.Size = OpSize::i128Bit;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	IRPair<IROp_F80Mul> _F80Mul(OrderedNode *X80Src1, OrderedNode *X80Src2) {
		auto ListDataBegin = DualListData.ListBegin();
		X80Src1->AddUse();
		X80Src2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(X80Src1) == RegClass::Invalid || WalkFindRegClass(X80Src1) == RegClass::FPR, "GetOpRegClass(X80Src1) == RegClass::Invalid || WalkFindRegClass(X80Src1) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(X80Src2) == RegClass::Invalid || WalkFindRegClass(X80Src2) == RegClass::FPR, "GetOpRegClass(X80Src2) == RegClass::Invalid || WalkFindRegClass(X80Src2) == RegClass::FPR");
#endif
		return _F80Mul(X80Src1->Wrapped(ListDataBegin), X80Src2->Wrapped(ListDataBegin));
	}

	/// Divides the value in stack location TOP+$SrcStack1 by the value in stack location TOP+$SrcStack2.
	///
	/// The result is stored in stack location TOP+$DstStack.
	///
	/// `FPR|Stack[TOP+DstStack] = Stack[TOP+SrcStack1] / Stack[TOP+SrcStack2]`
	IRPair<IROp_F80DivStack> _F80DivStack(uint8_t DstStack, uint8_t SrcStack1, uint8_t SrcStack2) {
		RecordX87Use();
		if(MMXState == MMXState_MMX) ChgStateMMX_X87();
		auto _Op = AllocateOp<IROp_F80DivStack, IROps::OP_F80DIVSTACK>();
		_Op.first->DstStack = DstStack;
		_Op.first->SrcStack1 = SrcStack1;
		_Op.first->SrcStack2 = SrcStack2;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Divides the value in stack location TOP+$SrcStack by the value $X80Src.
	///
	/// The result is stored in stack location TOP and returned.
	///
	/// `FPR|Stack[TOP] = Stack[TOP+SrcStack] / X80Src`
	IRPair<IROp_F80DivValue> _F80DivValue(uint8_t SrcStack, OrderedNodeWrapper X80Src) {
		RecordX87Use();
		if(MMXState == MMXState_MMX) ChgStateMMX_X87();
		auto _Op = AllocateOp<IROp_F80DivValue, IROps::OP_F80DIVVALUE>();
		_Op.first->X80Src = X80Src;
		_Op.first->SrcStack = SrcStack;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Divides the value in stack location TOP+$SrcStack by the value $X80Src.
	///
	/// The result is stored in stack location TOP and returned.
	///
	/// `FPR|Stack[TOP] = Stack[TOP+SrcStack] / X80Src`
	IRPair<IROp_F80DivValue> _F80DivValue(uint8_t SrcStack, OrderedNode *X80Src) {
		auto ListDataBegin = DualListData.ListBegin();
		X80Src->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(X80Src) == RegClass::Invalid || WalkFindRegClass(X80Src) == RegClass::FPR, "GetOpRegClass(X80Src) == RegClass::Invalid || WalkFindRegClass(X80Src) == RegClass::FPR");
#endif
		return _F80DivValue(SrcStack, X80Src->Wrapped(ListDataBegin));
	}

	/// Divides the value X80Src by the value in stack location TOP+$SrcStack.
	///
	/// The result is stored in stack location TOP.
	///
	/// `FPR|Stack[TOP] = X80Src / Stack[TOP+SrcStack]`
	IRPair<IROp_F80DivRValue> _F80DivRValue(OrderedNodeWrapper X80Src, uint8_t SrcStack) {
		RecordX87Use();
		if(MMXState == MMXState_MMX) ChgStateMMX_X87();
		auto _Op = AllocateOp<IROp_F80DivRValue, IROps::OP_F80DIVRVALUE>();
		_Op.first->X80Src = X80Src;
		_Op.first->SrcStack = SrcStack;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Divides the value X80Src by the value in stack location TOP+$SrcStack.
	///
	/// The result is stored in stack location TOP.
	///
	/// `FPR|Stack[TOP] = X80Src / Stack[TOP+SrcStack]`
	IRPair<IROp_F80DivRValue> _F80DivRValue(OrderedNode *X80Src, uint8_t SrcStack) {
		auto ListDataBegin = DualListData.ListBegin();
		X80Src->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(X80Src) == RegClass::Invalid || WalkFindRegClass(X80Src) == RegClass::FPR, "GetOpRegClass(X80Src) == RegClass::Invalid || WalkFindRegClass(X80Src) == RegClass::FPR");
#endif
		return _F80DivRValue(X80Src->Wrapped(ListDataBegin), SrcStack);
	}

	/// Divides the value in $X80Src1 by the value in $X80Src2.
	///
	/// The result is returned.
	///
	/// `FPR = X80Src1 / X80Src2`
	IRPair<IROp_F80Div> _F80Div(OrderedNodeWrapper X80Src1, OrderedNodeWrapper X80Src2) {
		auto _Op = AllocateOp<IROp_F80Div, IROps::OP_F80DIV>();
		_Op.first->X80Src1 = X80Src1;
		_Op.first->X80Src2 = X80Src2;
		_Op.first->Header.Size = OpSize::i128Bit;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Divides the value in $X80Src1 by the value in $X80Src2.
	///
	/// The result is returned.
	///
	/// `FPR = X80Src1 / X80Src2`
	IRPair<IROp_F80Div> _F80Div(OrderedNode *X80Src1, OrderedNode *X80Src2) {
		auto ListDataBegin = DualListData.ListBegin();
		X80Src1->AddUse();
		X80Src2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(X80Src1) == RegClass::Invalid || WalkFindRegClass(X80Src1) == RegClass::FPR, "GetOpRegClass(X80Src1) == RegClass::Invalid || WalkFindRegClass(X80Src1) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(X80Src2) == RegClass::Invalid || WalkFindRegClass(X80Src2) == RegClass::FPR, "GetOpRegClass(X80Src2) == RegClass::Invalid || WalkFindRegClass(X80Src2) == RegClass::FPR");
#endif
		return _F80Div(X80Src1->Wrapped(ListDataBegin), X80Src2->Wrapped(ListDataBegin));
	}

	/// Exchanges the value at the top of the stack with the value at TOP+$SrcStack.
	IRPair<IROp_F80StackXchange> _F80StackXchange(uint8_t SrcStack) {
		RecordX87Use();
		if(MMXState == MMXState_MMX) ChgStateMMX_X87();
		auto _Op = AllocateOp<IROp_F80StackXchange, IROps::OP_F80STACKXCHANGE>();
		_Op.first->SrcStack = SrcStack;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Complements the sign bit of the value at the top of the stack.
	///
	/// Returns the new value at the top of the stack.
	IRPair<IROp_F80StackChangeSign> _F80StackChangeSign() {
		RecordX87Use();
		if(MMXState == MMXState_MMX) ChgStateMMX_X87();
		auto _Op = AllocateOp<IROp_F80StackChangeSign, IROps::OP_F80STACKCHANGESIGN>();
		_Op.first->Header.Size = OpSize::i128Bit;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Clears the sign bit of the value at the top of the stack.
	///
	/// Returns the new value at the top of the stack.
	IRPair<IROp_F80StackAbs> _F80StackAbs() {
		RecordX87Use();
		if(MMXState == MMXState_MMX) ChgStateMMX_X87();
		auto _Op = AllocateOp<IROp_F80StackAbs, IROps::OP_F80STACKABS>();
		_Op.first->Header.Size = OpSize::i128Bit;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Computes the approximate tangent of the source operand in register ST(0), stores the result in ST(0), and pushes a 1.0 onto the FPU register stack.
	IRPair<IROp_F80PTANStack> _F80PTANStack() {
		RecordX87Use();
		if(MMXState == MMXState_MMX) ChgStateMMX_X87();
		auto _Op = AllocateOp<IROp_F80PTANStack, IROps::OP_F80PTANSTACK>();
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Computes arctan(st1/st0) and stores it in st0. Then pops the stack.
	IRPair<IROp_F80ATANStack> _F80ATANStack() {
		RecordX87Use();
		if(MMXState == MMXState_MMX) ChgStateMMX_X87();
		auto _Op = AllocateOp<IROp_F80ATANStack, IROps::OP_F80ATANSTACK>();
		_Op.first->Header.Size = OpSize::i128Bit;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	IRPair<IROp_F80ATAN> _F80ATAN(OrderedNodeWrapper X80Src1, OrderedNodeWrapper X80Src2) {
		auto _Op = AllocateOp<IROp_F80ATAN, IROps::OP_F80ATAN>();
		_Op.first->X80Src1 = X80Src1;
		_Op.first->X80Src2 = X80Src2;
		_Op.first->Header.Size = OpSize::i128Bit;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	IRPair<IROp_F80ATAN> _F80ATAN(OrderedNode *X80Src1, OrderedNode *X80Src2) {
		auto ListDataBegin = DualListData.ListBegin();
		X80Src1->AddUse();
		X80Src2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(X80Src1) == RegClass::Invalid || WalkFindRegClass(X80Src1) == RegClass::FPR, "GetOpRegClass(X80Src1) == RegClass::Invalid || WalkFindRegClass(X80Src1) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(X80Src2) == RegClass::Invalid || WalkFindRegClass(X80Src2) == RegClass::FPR, "GetOpRegClass(X80Src2) == RegClass::Invalid || WalkFindRegClass(X80Src2) == RegClass::FPR");
#endif
		return _F80ATAN(X80Src1->Wrapped(ListDataBegin), X80Src2->Wrapped(ListDataBegin));
	}

	IRPair<IROp_F80FPREMStack> _F80FPREMStack() {
		RecordX87Use();
		if(MMXState == MMXState_MMX) ChgStateMMX_X87();
		auto _Op = AllocateOp<IROp_F80FPREMStack, IROps::OP_F80FPREMSTACK>();
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	IRPair<IROp_F80FPREM> _F80FPREM(OrderedNodeWrapper X80Src1, OrderedNodeWrapper X80Src2) {
		auto _Op = AllocateOp<IROp_F80FPREM, IROps::OP_F80FPREM>();
		_Op.first->X80Src1 = X80Src1;
		_Op.first->X80Src2 = X80Src2;
		_Op.first->Header.Size = OpSize::i128Bit;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	IRPair<IROp_F80FPREM> _F80FPREM(OrderedNode *X80Src1, OrderedNode *X80Src2) {
		auto ListDataBegin = DualListData.ListBegin();
		X80Src1->AddUse();
		X80Src2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(X80Src1) == RegClass::Invalid || WalkFindRegClass(X80Src1) == RegClass::FPR, "GetOpRegClass(X80Src1) == RegClass::Invalid || WalkFindRegClass(X80Src1) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(X80Src2) == RegClass::Invalid || WalkFindRegClass(X80Src2) == RegClass::FPR, "GetOpRegClass(X80Src2) == RegClass::Invalid || WalkFindRegClass(X80Src2) == RegClass::FPR");
#endif
		return _F80FPREM(X80Src1->Wrapped(ListDataBegin), X80Src2->Wrapped(ListDataBegin));
	}

	IRPair<IROp_F80FPREM1Stack> _F80FPREM1Stack() {
		RecordX87Use();
		if(MMXState == MMXState_MMX) ChgStateMMX_X87();
		auto _Op = AllocateOp<IROp_F80FPREM1Stack, IROps::OP_F80FPREM1STACK>();
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	IRPair<IROp_F80FPREM1> _F80FPREM1(OrderedNodeWrapper X80Src1, OrderedNodeWrapper X80Src2) {
		auto _Op = AllocateOp<IROp_F80FPREM1, IROps::OP_F80FPREM1>();
		_Op.first->X80Src1 = X80Src1;
		_Op.first->X80Src2 = X80Src2;
		_Op.first->Header.Size = OpSize::i128Bit;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	IRPair<IROp_F80FPREM1> _F80FPREM1(OrderedNode *X80Src1, OrderedNode *X80Src2) {
		auto ListDataBegin = DualListData.ListBegin();
		X80Src1->AddUse();
		X80Src2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(X80Src1) == RegClass::Invalid || WalkFindRegClass(X80Src1) == RegClass::FPR, "GetOpRegClass(X80Src1) == RegClass::Invalid || WalkFindRegClass(X80Src1) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(X80Src2) == RegClass::Invalid || WalkFindRegClass(X80Src2) == RegClass::FPR, "GetOpRegClass(X80Src2) == RegClass::Invalid || WalkFindRegClass(X80Src2) == RegClass::FPR");
#endif
		return _F80FPREM1(X80Src1->Wrapped(ListDataBegin), X80Src2->Wrapped(ListDataBegin));
	}

	IRPair<IROp_F80SCALEStack> _F80SCALEStack() {
		RecordX87Use();
		if(MMXState == MMXState_MMX) ChgStateMMX_X87();
		auto _Op = AllocateOp<IROp_F80SCALEStack, IROps::OP_F80SCALESTACK>();
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	IRPair<IROp_F80SCALE> _F80SCALE(OrderedNodeWrapper X80Src1, OrderedNodeWrapper X80Src2) {
		auto _Op = AllocateOp<IROp_F80SCALE, IROps::OP_F80SCALE>();
		_Op.first->X80Src1 = X80Src1;
		_Op.first->X80Src2 = X80Src2;
		_Op.first->Header.Size = OpSize::i128Bit;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	IRPair<IROp_F80SCALE> _F80SCALE(OrderedNode *X80Src1, OrderedNode *X80Src2) {
		auto ListDataBegin = DualListData.ListBegin();
		X80Src1->AddUse();
		X80Src2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(X80Src1) == RegClass::Invalid || WalkFindRegClass(X80Src1) == RegClass::FPR, "GetOpRegClass(X80Src1) == RegClass::Invalid || WalkFindRegClass(X80Src1) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(X80Src2) == RegClass::Invalid || WalkFindRegClass(X80Src2) == RegClass::FPR, "GetOpRegClass(X80Src2) == RegClass::Invalid || WalkFindRegClass(X80Src2) == RegClass::FPR");
#endif
		return _F80SCALE(X80Src1->Wrapped(ListDataBegin), X80Src2->Wrapped(ListDataBegin));
	}

	IRPair<IROp_F80CVT> _F80CVT(FEXCore::IR::OpSize Size, OrderedNodeWrapper X80Src) {
		auto _Op = AllocateOp<IROp_F80CVT, IROps::OP_F80CVT>();
		_Op.first->X80Src = X80Src;
		_Op.first->Header.Size = Size;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	IRPair<IROp_F80CVT> _F80CVT(FEXCore::IR::OpSize Size, OrderedNode *X80Src) {
		auto ListDataBegin = DualListData.ListBegin();
		X80Src->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(X80Src) == RegClass::Invalid || WalkFindRegClass(X80Src) == RegClass::FPR, "GetOpRegClass(X80Src) == RegClass::Invalid || WalkFindRegClass(X80Src) == RegClass::FPR");
#endif
		return _F80CVT(Size, X80Src->Wrapped(ListDataBegin));
	}

	IRPair<IROp_F80CVTInt> _F80CVTInt(FEXCore::IR::OpSize Size, OrderedNodeWrapper X80Src, bool Truncate) {
		auto _Op = AllocateOp<IROp_F80CVTInt, IROps::OP_F80CVTINT>();
		_Op.first->X80Src = X80Src;
		_Op.first->Truncate = Truncate;
		_Op.first->Header.Size = Size;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	IRPair<IROp_F80CVTInt> _F80CVTInt(FEXCore::IR::OpSize Size, OrderedNode *X80Src, bool Truncate) {
		auto ListDataBegin = DualListData.ListBegin();
		X80Src->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(X80Src) == RegClass::Invalid || WalkFindRegClass(X80Src) == RegClass::FPR, "GetOpRegClass(X80Src) == RegClass::Invalid || WalkFindRegClass(X80Src) == RegClass::FPR");
#endif
		return _F80CVTInt(Size, X80Src->Wrapped(ListDataBegin), Truncate);
	}

	IRPair<IROp_F80CVTTo> _F80CVTTo(OrderedNodeWrapper X80Src, FEXCore::IR::OpSize SrcSize) {
		auto _Op = AllocateOp<IROp_F80CVTTo, IROps::OP_F80CVTTO>();
		_Op.first->X80Src = X80Src;
		_Op.first->SrcSize = SrcSize;
		_Op.first->Header.Size = OpSize::i128Bit;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	IRPair<IROp_F80CVTTo> _F80CVTTo(OrderedNode *X80Src, FEXCore::IR::OpSize SrcSize) {
		auto ListDataBegin = DualListData.ListBegin();
		X80Src->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(X80Src) == RegClass::Invalid || WalkFindRegClass(X80Src) == RegClass::FPR, "GetOpRegClass(X80Src) == RegClass::Invalid || WalkFindRegClass(X80Src) == RegClass::FPR");
#endif
		return _F80CVTTo(X80Src->Wrapped(ListDataBegin), SrcSize);
	}

	IRPair<IROp_F80CVTToInt> _F80CVTToInt(OrderedNodeWrapper Src, FEXCore::IR::OpSize SrcSize) {
		auto _Op = AllocateOp<IROp_F80CVTToInt, IROps::OP_F80CVTTOINT>();
		_Op.first->Src = Src;
		_Op.first->SrcSize = SrcSize;
		_Op.first->Header.Size = OpSize::i128Bit;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	IRPair<IROp_F80CVTToInt> _F80CVTToInt(OrderedNode *Src, FEXCore::IR::OpSize SrcSize) {
		auto ListDataBegin = DualListData.ListBegin();
		Src->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(Src) == RegClass::Invalid || WalkFindRegClass(Src) == RegClass::GPR, "GetOpRegClass(Src) == RegClass::Invalid || WalkFindRegClass(Src) == RegClass::GPR");
#endif
		return _F80CVTToInt(Src->Wrapped(ListDataBegin), SrcSize);
	}

	/// Replaces the value at the top of the stack with its nearest integral value.
	IRPair<IROp_F80RoundStack> _F80RoundStack() {
		RecordX87Use();
		if(MMXState == MMXState_MMX) ChgStateMMX_X87();
		auto _Op = AllocateOp<IROp_F80RoundStack, IROps::OP_F80ROUNDSTACK>();
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	IRPair<IROp_F80Round> _F80Round(OrderedNodeWrapper X80Src) {
		auto _Op = AllocateOp<IROp_F80Round, IROps::OP_F80ROUND>();
		_Op.first->X80Src = X80Src;
		_Op.first->Header.Size = OpSize::i128Bit;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	IRPair<IROp_F80Round> _F80Round(OrderedNode *X80Src) {
		auto ListDataBegin = DualListData.ListBegin();
		X80Src->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(X80Src) == RegClass::Invalid || WalkFindRegClass(X80Src) == RegClass::FPR, "GetOpRegClass(X80Src) == RegClass::Invalid || WalkFindRegClass(X80Src) == RegClass::FPR");
#endif
		return _F80Round(X80Src->Wrapped(ListDataBegin));
	}

	IRPair<IROp_F80F2XM1Stack> _F80F2XM1Stack() {
		RecordX87Use();
		if(MMXState == MMXState_MMX) ChgStateMMX_X87();
		auto _Op = AllocateOp<IROp_F80F2XM1Stack, IROps::OP_F80F2XM1STACK>();
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	IRPair<IROp_F80F2XM1> _F80F2XM1(OrderedNodeWrapper X80Src) {
		auto _Op = AllocateOp<IROp_F80F2XM1, IROps::OP_F80F2XM1>();
		_Op.first->X80Src = X80Src;
		_Op.first->Header.Size = OpSize::i128Bit;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	IRPair<IROp_F80F2XM1> _F80F2XM1(OrderedNode *X80Src) {
		auto ListDataBegin = DualListData.ListBegin();
		X80Src->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(X80Src) == RegClass::Invalid || WalkFindRegClass(X80Src) == RegClass::FPR, "GetOpRegClass(X80Src) == RegClass::Invalid || WalkFindRegClass(X80Src) == RegClass::FPR");
#endif
		return _F80F2XM1(X80Src->Wrapped(ListDataBegin));
	}

	IRPair<IROp_F80TAN> _F80TAN(OrderedNodeWrapper X80Src) {
		auto _Op = AllocateOp<IROp_F80TAN, IROps::OP_F80TAN>();
		_Op.first->X80Src = X80Src;
		_Op.first->Header.Size = OpSize::i128Bit;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	IRPair<IROp_F80TAN> _F80TAN(OrderedNode *X80Src) {
		auto ListDataBegin = DualListData.ListBegin();
		X80Src->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(X80Src) == RegClass::Invalid || WalkFindRegClass(X80Src) == RegClass::FPR, "GetOpRegClass(X80Src) == RegClass::Invalid || WalkFindRegClass(X80Src) == RegClass::FPR");
#endif
		return _F80TAN(X80Src->Wrapped(ListDataBegin));
	}

	IRPair<IROp_F80SINStack> _F80SINStack() {
		RecordX87Use();
		if(MMXState == MMXState_MMX) ChgStateMMX_X87();
		auto _Op = AllocateOp<IROp_F80SINStack, IROps::OP_F80SINSTACK>();
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	IRPair<IROp_F80SIN> _F80SIN(OrderedNodeWrapper X80Src) {
		auto _Op = AllocateOp<IROp_F80SIN, IROps::OP_F80SIN>();
		_Op.first->X80Src = X80Src;
		_Op.first->Header.Size = OpSize::i128Bit;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	IRPair<IROp_F80SIN> _F80SIN(OrderedNode *X80Src) {
		auto ListDataBegin = DualListData.ListBegin();
		X80Src->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(X80Src) == RegClass::Invalid || WalkFindRegClass(X80Src) == RegClass::FPR, "GetOpRegClass(X80Src) == RegClass::Invalid || WalkFindRegClass(X80Src) == RegClass::FPR");
#endif
		return _F80SIN(X80Src->Wrapped(ListDataBegin));
	}

	IRPair<IROp_F80COSStack> _F80COSStack() {
		RecordX87Use();
		if(MMXState == MMXState_MMX) ChgStateMMX_X87();
		auto _Op = AllocateOp<IROp_F80COSStack, IROps::OP_F80COSSTACK>();
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	IRPair<IROp_F80COS> _F80COS(OrderedNodeWrapper X80Src) {
		auto _Op = AllocateOp<IROp_F80COS, IROps::OP_F80COS>();
		_Op.first->X80Src = X80Src;
		_Op.first->Header.Size = OpSize::i128Bit;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	IRPair<IROp_F80COS> _F80COS(OrderedNode *X80Src) {
		auto ListDataBegin = DualListData.ListBegin();
		X80Src->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(X80Src) == RegClass::Invalid || WalkFindRegClass(X80Src) == RegClass::FPR, "GetOpRegClass(X80Src) == RegClass::Invalid || WalkFindRegClass(X80Src) == RegClass::FPR");
#endif
		return _F80COS(X80Src->Wrapped(ListDataBegin));
	}

	IRPair<IROp_F80SINCOS> _F80SINCOS(OrderedNodeWrapper X80Src, OrderedNodeWrapper OutSin, OrderedNodeWrapper OutCos) {
		auto _Op = AllocateOp<IROp_F80SINCOS, IROps::OP_F80SINCOS>();
		_Op.first->X80Src = X80Src;
		_Op.first->OutSin = OutSin;
		_Op.first->OutCos = OutCos;
		_Op.first->Header.Size = OpSize::i128Bit;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	IRPair<IROp_F80SINCOS> _F80SINCOS(OrderedNode *X80Src, OrderedNode *OutSin, OrderedNode *OutCos) {
		auto ListDataBegin = DualListData.ListBegin();
		X80Src->AddUse();
		OutSin->AddUse();
		OutCos->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(X80Src) == RegClass::Invalid || WalkFindRegClass(X80Src) == RegClass::FPR, "GetOpRegClass(X80Src) == RegClass::Invalid || WalkFindRegClass(X80Src) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(OutSin) == RegClass::Invalid || WalkFindRegClass(OutSin) == RegClass::FPR, "GetOpRegClass(OutSin) == RegClass::Invalid || WalkFindRegClass(OutSin) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(OutCos) == RegClass::Invalid || WalkFindRegClass(OutCos) == RegClass::FPR, "GetOpRegClass(OutCos) == RegClass::Invalid || WalkFindRegClass(OutCos) == RegClass::FPR");
#endif
		return _F80SINCOS(X80Src->Wrapped(ListDataBegin), OutSin->Wrapped(ListDataBegin), OutCos->Wrapped(ListDataBegin));
	}

	IRPair<IROp_F80SINCOSStack> _F80SINCOSStack() {
		RecordX87Use();
		if(MMXState == MMXState_MMX) ChgStateMMX_X87();
		auto _Op = AllocateOp<IROp_F80SINCOSStack, IROps::OP_F80SINCOSSTACK>();
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	IRPair<IROp_F80SQRTStack> _F80SQRTStack() {
		RecordX87Use();
		if(MMXState == MMXState_MMX) ChgStateMMX_X87();
		auto _Op = AllocateOp<IROp_F80SQRTStack, IROps::OP_F80SQRTSTACK>();
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	IRPair<IROp_F80SQRT> _F80SQRT(OrderedNodeWrapper X80Src) {
		auto _Op = AllocateOp<IROp_F80SQRT, IROps::OP_F80SQRT>();
		_Op.first->X80Src = X80Src;
		_Op.first->Header.Size = OpSize::i128Bit;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	IRPair<IROp_F80SQRT> _F80SQRT(OrderedNode *X80Src) {
		auto ListDataBegin = DualListData.ListBegin();
		X80Src->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(X80Src) == RegClass::Invalid || WalkFindRegClass(X80Src) == RegClass::FPR, "GetOpRegClass(X80Src) == RegClass::Invalid || WalkFindRegClass(X80Src) == RegClass::FPR");
#endif
		return _F80SQRT(X80Src->Wrapped(ListDataBegin));
	}

	IRPair<IROp_F80XTRACT_EXP> _F80XTRACT_EXP(OrderedNodeWrapper X80Src) {
		auto _Op = AllocateOp<IROp_F80XTRACT_EXP, IROps::OP_F80XTRACT_EXP>();
		_Op.first->X80Src = X80Src;
		_Op.first->Header.Size = OpSize::i128Bit;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	IRPair<IROp_F80XTRACT_EXP> _F80XTRACT_EXP(OrderedNode *X80Src) {
		auto ListDataBegin = DualListData.ListBegin();
		X80Src->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(X80Src) == RegClass::Invalid || WalkFindRegClass(X80Src) == RegClass::FPR, "GetOpRegClass(X80Src) == RegClass::Invalid || WalkFindRegClass(X80Src) == RegClass::FPR");
#endif
		return _F80XTRACT_EXP(X80Src->Wrapped(ListDataBegin));
	}

	IRPair<IROp_F80XTRACT_SIG> _F80XTRACT_SIG(OrderedNodeWrapper X80Src) {
		auto _Op = AllocateOp<IROp_F80XTRACT_SIG, IROps::OP_F80XTRACT_SIG>();
		_Op.first->X80Src = X80Src;
		_Op.first->Header.Size = OpSize::i128Bit;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	IRPair<IROp_F80XTRACT_SIG> _F80XTRACT_SIG(OrderedNode *X80Src) {
		auto ListDataBegin = DualListData.ListBegin();
		X80Src->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(X80Src) == RegClass::Invalid || WalkFindRegClass(X80Src) == RegClass::FPR, "GetOpRegClass(X80Src) == RegClass::Invalid || WalkFindRegClass(X80Src) == RegClass::FPR");
#endif
		return _F80XTRACT_SIG(X80Src->Wrapped(ListDataBegin));
	}

	/// Does comparison between value in stack at TOP + SrcStack
	IRPair<IROp_F80StackTest> _F80StackTest(uint8_t SrcStack) {
		RecordX87Use();
		if(MMXState == MMXState_MMX) ChgStateMMX_X87();
		auto _Op = AllocateOp<IROp_F80StackTest, IROps::OP_F80STACKTEST>();
		_Op.first->SrcStack = SrcStack;
		_Op.first->Header.Size = OpSize::i32Bit;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Does a scalar unordered compare between the value at the top of the stack and the value in stack position TOP+$SrcStack and stores the flags in to a GPR
	///
	/// Ordering flag result is true if either float input is NaN
	IRPair<IROp_F80CmpStack> _F80CmpStack(uint8_t SrcStack) {
		RecordX87Use();
		if(MMXState == MMXState_MMX) ChgStateMMX_X87();
		auto _Op = AllocateOp<IROp_F80CmpStack, IROps::OP_F80CMPSTACK>();
		_Op.first->SrcStack = SrcStack;
		_Op.first->Header.Size = OpSize::i32Bit;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Does a scalar unordered compare between the value at the top of the stack and $X80Src and stores the asked for flags in to a GPR
	///
	/// Ordering flag result is true if either float input is NaN
	IRPair<IROp_F80CmpValue> _F80CmpValue(OrderedNodeWrapper X80Src) {
		RecordX87Use();
		if(MMXState == MMXState_MMX) ChgStateMMX_X87();
		auto _Op = AllocateOp<IROp_F80CmpValue, IROps::OP_F80CMPVALUE>();
		_Op.first->X80Src = X80Src;
		_Op.first->Header.Size = OpSize::i32Bit;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Does a scalar unordered compare between the value at the top of the stack and $X80Src and stores the asked for flags in to a GPR
	///
	/// Ordering flag result is true if either float input is NaN
	IRPair<IROp_F80CmpValue> _F80CmpValue(OrderedNode *X80Src) {
		auto ListDataBegin = DualListData.ListBegin();
		X80Src->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(X80Src) == RegClass::Invalid || WalkFindRegClass(X80Src) == RegClass::FPR, "GetOpRegClass(X80Src) == RegClass::Invalid || WalkFindRegClass(X80Src) == RegClass::FPR");
#endif
		return _F80CmpValue(X80Src->Wrapped(ListDataBegin));
	}

	/// Does a scalar unordered compare and stores the flags in to a GPR
	///
	/// Ordering flag result is true if either float input is NaN
	IRPair<IROp_F80Cmp> _F80Cmp(OrderedNodeWrapper X80Src1, OrderedNodeWrapper X80Src2) {
		auto _Op = AllocateOp<IROp_F80Cmp, IROps::OP_F80CMP>();
		_Op.first->X80Src1 = X80Src1;
		_Op.first->X80Src2 = X80Src2;
		_Op.first->Header.Size = OpSize::i32Bit;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Does a scalar unordered compare and stores the flags in to a GPR
	///
	/// Ordering flag result is true if either float input is NaN
	IRPair<IROp_F80Cmp> _F80Cmp(OrderedNode *X80Src1, OrderedNode *X80Src2) {
		auto ListDataBegin = DualListData.ListBegin();
		X80Src1->AddUse();
		X80Src2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(X80Src1) == RegClass::Invalid || WalkFindRegClass(X80Src1) == RegClass::FPR, "GetOpRegClass(X80Src1) == RegClass::Invalid || WalkFindRegClass(X80Src1) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(X80Src2) == RegClass::Invalid || WalkFindRegClass(X80Src2) == RegClass::FPR, "GetOpRegClass(X80Src2) == RegClass::Invalid || WalkFindRegClass(X80Src2) == RegClass::FPR");
#endif
		return _F80Cmp(X80Src1->Wrapped(ListDataBegin), X80Src2->Wrapped(ListDataBegin));
	}

	IRPair<IROp_F80BCDLoad> _F80BCDLoad(OrderedNodeWrapper X80Src) {
		auto _Op = AllocateOp<IROp_F80BCDLoad, IROps::OP_F80BCDLOAD>();
		_Op.first->X80Src = X80Src;
		_Op.first->Header.Size = OpSize::i128Bit;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	IRPair<IROp_F80BCDLoad> _F80BCDLoad(OrderedNode *X80Src) {
		auto ListDataBegin = DualListData.ListBegin();
		X80Src->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(X80Src) == RegClass::Invalid || WalkFindRegClass(X80Src) == RegClass::FPR, "GetOpRegClass(X80Src) == RegClass::Invalid || WalkFindRegClass(X80Src) == RegClass::FPR");
#endif
		return _F80BCDLoad(X80Src->Wrapped(ListDataBegin));
	}

	IRPair<IROp_F80BCDStore> _F80BCDStore(OrderedNodeWrapper X80Src) {
		auto _Op = AllocateOp<IROp_F80BCDStore, IROps::OP_F80BCDSTORE>();
		_Op.first->X80Src = X80Src;
		_Op.first->Header.Size = OpSize::i128Bit;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	IRPair<IROp_F80BCDStore> _F80BCDStore(OrderedNode *X80Src) {
		auto ListDataBegin = DualListData.ListBegin();
		X80Src->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(X80Src) == RegClass::Invalid || WalkFindRegClass(X80Src) == RegClass::FPR, "GetOpRegClass(X80Src) == RegClass::Invalid || WalkFindRegClass(X80Src) == RegClass::FPR");
#endif
		return _F80BCDStore(X80Src->Wrapped(ListDataBegin));
	}

	/// Computes ST1 * log2(ST0)
	///
	/// Stores the result in ST1, and pops the top of the stack.
	///
	/// Returns the new value at the top of the stack, i.e. the result of the operation.
	IRPair<IROp_F80FYL2XStack> _F80FYL2XStack() {
		RecordX87Use();
		if(MMXState == MMXState_MMX) ChgStateMMX_X87();
		auto _Op = AllocateOp<IROp_F80FYL2XStack, IROps::OP_F80FYL2XSTACK>();
		_Op.first->Header.Size = OpSize::i128Bit;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	IRPair<IROp_F80FYL2X> _F80FYL2X(OrderedNodeWrapper X80Src1, OrderedNodeWrapper X80Src2) {
		auto _Op = AllocateOp<IROp_F80FYL2X, IROps::OP_F80FYL2X>();
		_Op.first->X80Src1 = X80Src1;
		_Op.first->X80Src2 = X80Src2;
		_Op.first->Header.Size = OpSize::i128Bit;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	IRPair<IROp_F80FYL2X> _F80FYL2X(OrderedNode *X80Src1, OrderedNode *X80Src2) {
		auto ListDataBegin = DualListData.ListBegin();
		X80Src1->AddUse();
		X80Src2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(X80Src1) == RegClass::Invalid || WalkFindRegClass(X80Src1) == RegClass::FPR, "GetOpRegClass(X80Src1) == RegClass::Invalid || WalkFindRegClass(X80Src1) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(X80Src2) == RegClass::Invalid || WalkFindRegClass(X80Src2) == RegClass::FPR, "GetOpRegClass(X80Src2) == RegClass::Invalid || WalkFindRegClass(X80Src2) == RegClass::FPR");
#endif
		return _F80FYL2X(X80Src1->Wrapped(ListDataBegin), X80Src2->Wrapped(ListDataBegin));
	}

	/// Computes ST1 * log2(1 + ST0)
	///
	/// Stores the result in ST1, and pops the top of the stack.
	///
	/// Returns the new value at the top of the stack, i.e. the result of the operation.
	IRPair<IROp_F80FYL2XP1Stack> _F80FYL2XP1Stack() {
		RecordX87Use();
		if(MMXState == MMXState_MMX) ChgStateMMX_X87();
		auto _Op = AllocateOp<IROp_F80FYL2XP1Stack, IROps::OP_F80FYL2XP1STACK>();
		_Op.first->Header.Size = OpSize::i128Bit;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	IRPair<IROp_F80FYL2XP1> _F80FYL2XP1(OrderedNodeWrapper X80Src1, OrderedNodeWrapper X80Src2) {
		auto _Op = AllocateOp<IROp_F80FYL2XP1, IROps::OP_F80FYL2XP1>();
		_Op.first->X80Src1 = X80Src1;
		_Op.first->X80Src2 = X80Src2;
		_Op.first->Header.Size = OpSize::i128Bit;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	IRPair<IROp_F80FYL2XP1> _F80FYL2XP1(OrderedNode *X80Src1, OrderedNode *X80Src2) {
		auto ListDataBegin = DualListData.ListBegin();
		X80Src1->AddUse();
		X80Src2->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(X80Src1) == RegClass::Invalid || WalkFindRegClass(X80Src1) == RegClass::FPR, "GetOpRegClass(X80Src1) == RegClass::Invalid || WalkFindRegClass(X80Src1) == RegClass::FPR");
		LOGMAN_THROW_A_FMT(GetOpRegClass(X80Src2) == RegClass::Invalid || WalkFindRegClass(X80Src2) == RegClass::FPR, "GetOpRegClass(X80Src2) == RegClass::Invalid || WalkFindRegClass(X80Src2) == RegClass::FPR");
#endif
		return _F80FYL2XP1(X80Src1->Wrapped(ListDataBegin), X80Src2->Wrapped(ListDataBegin));
	}

	/// Does a vector bitwise select.
	///
	/// If the bit in the field is 1 then the corresponding bit is pulled from VectorTrue
	///
	/// If the bit in the field is 0 then the corresponding bit is pulled from VectorFalse
	///
	/// Writes the result to the top of the stack.
	IRPair<IROp_F80VBSLStack> _F80VBSLStack(FEXCore::IR::OpSize RegisterSize, OrderedNodeWrapper VectorMask, uint8_t SrcStack1, uint8_t SrcStack2) {
		RecordX87Use();
		if(MMXState == MMXState_MMX) ChgStateMMX_X87();
		auto _Op = AllocateOp<IROp_F80VBSLStack, IROps::OP_F80VBSLSTACK>();
		_Op.first->VectorMask = VectorMask;
		_Op.first->SrcStack1 = SrcStack1;
		_Op.first->SrcStack2 = SrcStack2;
		_Op.first->Header.ElementSize = _Op.first->Header.Size;
		return _Op;
	}

	/// Does a vector bitwise select.
	///
	/// If the bit in the field is 1 then the corresponding bit is pulled from VectorTrue
	///
	/// If the bit in the field is 0 then the corresponding bit is pulled from VectorFalse
	///
	/// Writes the result to the top of the stack.
	IRPair<IROp_F80VBSLStack> _F80VBSLStack(FEXCore::IR::OpSize RegisterSize, OrderedNode *VectorMask, uint8_t SrcStack1, uint8_t SrcStack2) {
		auto ListDataBegin = DualListData.ListBegin();
		VectorMask->AddUse();
#if defined(ASSERTIONS_ENABLED) && ASSERTIONS_ENABLED
		LOGMAN_THROW_A_FMT(GetOpRegClass(VectorMask) == RegClass::Invalid || WalkFindRegClass(VectorMask) == RegClass::FPR, "GetOpRegClass(VectorMask) == RegClass::Invalid || WalkFindRegClass(VectorMask) == RegClass::FPR");
#endif
		return _F80VBSLStack(RegisterSize, VectorMask->Wrapped(ListDataBegin), SrcStack1, SrcStack2);
	}

#undef IROP_ALLOCATE_HELPERS
#endif
