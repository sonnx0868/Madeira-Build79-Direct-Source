# Index
## Op Classes
- [ALU](#ALU)

- [Atomic](#Atomic)

- [Backend](#Backend)

- [Branch](#Branch)

- [Conv](#Conv)

- [Crypto](#Crypto)

- [F64](#F64)

- [F80](#F80)

- [Memory](#Memory)

- [Misc](#Misc)

- [Moves](#Moves)

- [StaticRA](#StaticRA)

- [Vector](#Vector)

- [VectorScalar](#VectorScalar)

## Definitions
- [Defines](#Defines)

# IR documentation

# ALU

## GPR = EntrypointOffset OpSize:#Size, i64:$Offset
>GPR = EntrypointOffset OpSize:#Size, i64:$Offset

Returns the <entrypoint> + Offset address

When the size is 4 bytes then 32-bit overflow and underflow needs to work

## InlineEntrypointOffset OpSize:#Size, i64:$Offset
>InlineEntrypointOffset OpSize:#Size, i64:$Offset

Returns the <entrypoint> + Offset address

When the size is 4 bytes then 32-bit overflow and underflow needs to work

## GPR = Constant i64:$Constant, ConstPad:$Pad{IR::ConstPad::NoPad}, i32:$MaxBytes{0}
>GPR = Constant i64:$Constant, ConstPad:$Pad{IR::ConstPad::NoPad}, i32:$MaxBytes{0}

Generates a 64bit constant inside of a GPR

Unsupported to create a constant in FPR

## InlineConstant i64:$Constant
>InlineConstant i64:$Constant

Generates a 64bit constant to be used directly, non-FPR

## GPR = CycleCounter i1:$SelfSynchronizingLoads
>GPR = CycleCounter i1:$SelfSynchronizingLoads

Returns the host 64bit cycle counter

Useful when emulating rdtsc

Be careful, the frequency of this counter changes based on host

On AArch64 make sure to query the CNTFRQ_EL0 system register to get the frequency

On x86-64 make sure to query CPUID fn8000_0008[EDX_8] for constant TSC

x86-64 constant frequency lives in MSR_PLATFORM_INFO. Which is only available to kernel

Part of the ART frequency equation can be pulled from CPUID fn0000_0015[EBX & EAX]

But it's missing the ART multiplier still?

If the self-synchronizing flag is toggled then all instructions and loads must be completed before the cycle counter read

## GPR = Neg OpSize:#Size, GPR:$Src, CondClass:$Cond{CondClass::AL}
>GPR = Neg OpSize:#Size, GPR:$Src, CondClass:$Cond{CondClass::AL}

Integer negation, with optional predication

Dest = Cond ? -Src : Src

Will truncate to 64 or 32bits

## GPR = Not OpSize:#Size, GPR:$Src
>GPR = Not OpSize:#Size, GPR:$Src

Integer binary not

op:

Dest = ~Src

## GPR = Popcount OpSize:#Size, GPR:$Src
>GPR = Popcount OpSize:#Size, GPR:$Src

Population count of source register

Returns the number of bits set

## GPR = FindLSB OpSize:#Size, GPR:$Src
>GPR = FindLSB OpSize:#Size, GPR:$Src

Find least-significant-bit set

Returns the index of the least significant bit set

Undefined result if Src is zero.

## GPR = FindMSB OpSize:#Size, GPR:$Src
>GPR = FindMSB OpSize:#Size, GPR:$Src

Find most-significant-bit set

Returns the index of the most significant bit set

Undefined result if Src is zero.

## GPR = FindTrailingZeroes OpSize:#Size, GPR:$Src
>GPR = FindTrailingZeroes OpSize:#Size, GPR:$Src

Counts the number of trailing zero bits in a GPR

Returns the number of bits that are zero trailing

In the case of zero returns the size in bits of the input

## GPR = CountLeadingZeroes OpSize:#Size, GPR:$Src
>GPR = CountLeadingZeroes OpSize:#Size, GPR:$Src

Counts the number of leading zero bits in a GPR

Returns the number of bits that are zero leading

In the case of zero returns the size in bits of the input

## GPR = Rev OpSize:#Size, GPR:$Src
>GPR = Rev OpSize:#Size, GPR:$Src

Reverses the byte order of the register

Specifically 8bit byte swap size. (Not 16bit or 32bit word swapping)

## GPR = Rbit OpSize:#Size, GPR:$Src
>GPR = Rbit OpSize:#Size, GPR:$Src

Reverses the bit order of the register

## GPR = Add OpSize:#Size, GPR:$Src1, GPR:$Src2
>GPR = Add OpSize:#Size, GPR:$Src1, GPR:$Src2

Integer Add

Will truncate to 64 or 32bits

## GPR = Adc OpSize:#Size, GPR:$Src1, GPR:$Src2
>GPR = Adc OpSize:#Size, GPR:$Src1, GPR:$Src2

Integer Add with carry

Will truncate to 64 or 32bits

## GPR = Sbb OpSize:#Size, GPR:$Src1, GPR:$Src2
>GPR = Sbb OpSize:#Size, GPR:$Src1, GPR:$Src2

Integer Subtract with carry/borrow

Will truncate to 64 or 32bits

## GPR = AddShift OpSize:#Size, GPR:$Src1, GPR:$Src2, ShiftType:$Shift{ShiftType::LSL}, u8:$ShiftAmount{0}
>GPR = AddShift OpSize:#Size, GPR:$Src1, GPR:$Src2, ShiftType:$Shift{ShiftType::LSL}, u8:$ShiftAmount{0}

Integer Add with shifted register

Will truncate to 64 or 32bits

Dest = Src1 + (Src2 << ShiftAmount)

## GPR = AddWithFlags OpSize:#Size, GPR:$Src1, GPR:$Src2
>GPR = AddWithFlags OpSize:#Size, GPR:$Src1, GPR:$Src2

Integer add. Truncates and sets NZCV per AddNZCV

## AddNZCV OpSize:#Size, GPR:$Src1, GPR:$Src2
>AddNZCV OpSize:#Size, GPR:$Src1, GPR:$Src2

Set NZCV for the sum of two GPRs

## SetSmallNZV OpSize:#Size, GPR:$Src
>SetSmallNZV OpSize:#Size, GPR:$Src

Set NZV with a SETF instruction. Preserves CF.

## CarryInvert
>CarryInvert

Invert carry flag in NZCV

## AXFlag GPR:$V_inv
>AXFlag GPR:$V_inv

After an FCmp, converts NZCV flags from the Arm format to a mysterious eXternal format

On FlagM2-less platforms, takes the inverted 1/0 overflow flag

## GPR = Parity GPR:$Raw, i1:$Mask, i1:$Invert
>GPR = Parity GPR:$Raw, i1:$Mask, i1:$Invert

Calculates PF

## RmifNZCV GPR:$Src, u8:$Rotate, u8:$Mask
>RmifNZCV GPR:$Src, u8:$Rotate, u8:$Mask

Rotate, mask, and insert into NZCV on FlagM platforms

## CondAddNZCV OpSize:#Size, GPR:$Src1, GPR:$Src2, CondClass:$Cond, u8:$FalseNZCV
>CondAddNZCV OpSize:#Size, GPR:$Src1, GPR:$Src2, CondClass:$Cond, u8:$FalseNZCV

If condition is true, set NZCV per sum of GPRs, else force NZCV to a constant.

## CondSubNZCV OpSize:#Size, GPR:$Src1, GPR:$Src2, CondClass:$Cond, u8:$FalseNZCV
>CondSubNZCV OpSize:#Size, GPR:$Src1, GPR:$Src2, CondClass:$Cond, u8:$FalseNZCV

If condition is true, set NZCV per difference of GPRs, else force NZCV to a constant.

## GPR = AdcWithFlags OpSize:#Size, GPR:$Src1, GPR:$Src2
>GPR = AdcWithFlags OpSize:#Size, GPR:$Src1, GPR:$Src2

Adds and set NZCV for the sum of two GPRs and carry-in given as NZCV

## GPR = AdcZero OpSize:#Size, GPR:$Src1
>GPR = AdcZero OpSize:#Size, GPR:$Src1

Adds GPR with inverted carry-in

## GPR = AdcZeroWithFlags OpSize:#Size, GPR:$Src1
>GPR = AdcZeroWithFlags OpSize:#Size, GPR:$Src1

Adds and set NZCV for the sum of GPR and inverted carry-in given as NZCV

## GPR = SbbWithFlags OpSize:#Size, GPR:$Src1, GPR:$Src2
>GPR = SbbWithFlags OpSize:#Size, GPR:$Src1, GPR:$Src2

Subtracts and set NZCV for the difference of two GPRs and carry-in given as NZCV

## AdcNZCV OpSize:#Size, GPR:$Src1, GPR:$Src2
>AdcNZCV OpSize:#Size, GPR:$Src1, GPR:$Src2

Set NZCV for the sum of two GPRs and carry-in given as NZCV

## SbbNZCV OpSize:#Size, GPR:$Src1, GPR:$Src2
>SbbNZCV OpSize:#Size, GPR:$Src1, GPR:$Src2

Set NZCV for the difference of two GPRs and carry-in given as NZCV

## GPR = Sub OpSize:#Size, GPR:$Src1, GPR:$Src2
>GPR = Sub OpSize:#Size, GPR:$Src1, GPR:$Src2

Integer Sub

Will truncate to 64 or 32bits

## GPR = SubShift OpSize:#Size, GPR:$Src1, GPR:$Src2, ShiftType:$Shift{ShiftType::LSL}, u8:$ShiftAmount{0}
>GPR = SubShift OpSize:#Size, GPR:$Src1, GPR:$Src2, ShiftType:$Shift{ShiftType::LSL}, u8:$ShiftAmount{0}

Integer Sub with shifted register

Will truncate to 64 or 32bits

## GPR = SubWithFlags OpSize:#Size, GPR:$Src1, GPR:$Src2
>GPR = SubWithFlags OpSize:#Size, GPR:$Src1, GPR:$Src2

Integer Sub. Truncates and sets NZCV per SubNZCV

## CmpPairZ OpSize:#Size, GPR:$Src1Lo, GPR:$Src1Hi, GPR:$Src2Lo, GPR:$Src2Hi
>CmpPairZ OpSize:#Size, GPR:$Src1Lo, GPR:$Src1Hi, GPR:$Src2Lo, GPR:$Src2Hi

Compares register pairs and sets Z accordingly, preserving N/Z/V.

This accelerates cmpxchg.

## SubNZCV OpSize:#Size, GPR:$Src1, GPR:$Src2
>SubNZCV OpSize:#Size, GPR:$Src1, GPR:$Src2

Set NZCV for the difference of two GPRs. 

Carry flag uses arm64 definition, inverted x86.



## GPR = Or OpSize:#Size, GPR:$Src1, GPR:$Src2
>GPR = Or OpSize:#Size, GPR:$Src1, GPR:$Src2

Integer binary or

## GPR = Orlshl OpSize:#Size, GPR:$Src1, GPR:$Src2, u8:$BitShift
>GPR = Orlshl OpSize:#Size, GPR:$Src1, GPR:$Src2, u8:$BitShift

Integer binary or with logical shift left

## GPR = Orlshr OpSize:#Size, GPR:$Src1, GPR:$Src2, u8:$BitShift
>GPR = Orlshr OpSize:#Size, GPR:$Src1, GPR:$Src2, u8:$BitShift

Integer binary or with logical shift right

## GPR = Ornror OpSize:#Size, GPR:$Src1, GPR:$Src2, u8:$BitShift
>GPR = Ornror OpSize:#Size, GPR:$Src1, GPR:$Src2, u8:$BitShift

Integer binary or with NOT on second source and rotation right

## GPR = Xor OpSize:#Size, GPR:$Src1, GPR:$Src2
>GPR = Xor OpSize:#Size, GPR:$Src1, GPR:$Src2

Integer binary exclusive or

## GPR = XorShift OpSize:#Size, GPR:$Src1, GPR:$Src2, ShiftType:$Shift{ShiftType::LSL}, u8:$ShiftAmount{0}
>GPR = XorShift OpSize:#Size, GPR:$Src1, GPR:$Src2, ShiftType:$Shift{ShiftType::LSL}, u8:$ShiftAmount{0}

Integer binary exclusive or with shifted register

## GPR = XornShift OpSize:#Size, GPR:$Src1, GPR:$Src2, ShiftType:$Shift{ShiftType::LSL}, u8:$ShiftAmount{0}
>GPR = XornShift OpSize:#Size, GPR:$Src1, GPR:$Src2, ShiftType:$Shift{ShiftType::LSL}, u8:$ShiftAmount{0}

Integer binary exclusive or not with shifted register

## GPR = And OpSize:#Size, GPR:$Src1, GPR:$Src2
>GPR = And OpSize:#Size, GPR:$Src1, GPR:$Src2

Integer binary and

## GPR = AndShift OpSize:#Size, GPR:$Src1, GPR:$Src2, ShiftType:$Shift{ShiftType::LSL}, u8:$ShiftAmount{0}
>GPR = AndShift OpSize:#Size, GPR:$Src1, GPR:$Src2, ShiftType:$Shift{ShiftType::LSL}, u8:$ShiftAmount{0}

Integer binary and with shifted register

## GPR = AndWithFlags OpSize:#Size, GPR:$Src1, GPR:$Src2
>GPR = AndWithFlags OpSize:#Size, GPR:$Src1, GPR:$Src2

Integer binary and

## GPR = Andn OpSize:#Size, GPR:$Src1, GPR:$Src2
>GPR = Andn OpSize:#Size, GPR:$Src1, GPR:$Src2

Integer binary AND NOT. Performs the equivalent of Src1 & ~Src2

## TestNZ OpSize:#Size, GPR:$Src1, GPR:$Src2
>TestNZ OpSize:#Size, GPR:$Src1, GPR:$Src2

Set NZCV for the binary AND of two GPRs, setting N and Z accordingly and zeroing C and V

## TestZ OpSize:#Size, GPR:$Src1, GPR:$Src2
>TestZ OpSize:#Size, GPR:$Src1, GPR:$Src2

Set NZCV for the binary AND of two GPRs, setting Z accordingly and zeroing C and V. N is undefined.

## GPR = Lshl OpSize:#Size, GPR:$Src1, GPR:$Src2
>GPR = Lshl OpSize:#Size, GPR:$Src1, GPR:$Src2

Integer logical shift left

## GPR = Lshr OpSize:#Size, GPR:$Src1, GPR:$Src2
>GPR = Lshr OpSize:#Size, GPR:$Src1, GPR:$Src2

Integer logical shift right

## GPR = Ashr OpSize:#Size, GPR:$Src1, GPR:$Src2
>GPR = Ashr OpSize:#Size, GPR:$Src1, GPR:$Src2

Integer arithmetic shift right

## GPR = ShiftFlags OpSize:$Size, GPR:$Result, GPR:$Src1, ShiftType:$Shift, GPR:$Src2, GPR:$PFInput, i1:$InvertCF
>GPR = ShiftFlags OpSize:$Size, GPR:$Result, GPR:$Src1, ShiftType:$Shift, GPR:$Src2, GPR:$PFInput, i1:$InvertCF

Set NZCV flags for specified variable integer shift with given result.

Returns updated raw PF.

## RotateFlags OpSize:$Size, GPR:$Result, GPR:$Shift, i1:$Left
>RotateFlags OpSize:$Size, GPR:$Result, GPR:$Shift, i1:$Left

Set NZCV flags for specified variable integer rotate with given result.

## GPR = Ror OpSize:#Size, GPR:$Src1, GPR:$Src2
>GPR = Ror OpSize:#Size, GPR:$Src1, GPR:$Src2

Integer rotate right

## GPR = Mul OpSize:#Size, GPR:$Src1, GPR:$Src2
>GPR = Mul OpSize:#Size, GPR:$Src1, GPR:$Src2

Integer signed multiplication

## GPR = UMul OpSize:#Size, GPR:$Src1, GPR:$Src2
>GPR = UMul OpSize:#Size, GPR:$Src1, GPR:$Src2

Integer unsigned multiplication

## GPR = UMull GPR:$Src1, GPR:$Src2
>GPR = UMull GPR:$Src1, GPR:$Src2

Integer unsigned multiplication long

Multiplies two 32-bit numbers, returning a 64-bit destination register.

## GPR = SMull GPR:$Src1, GPR:$Src2
>GPR = SMull GPR:$Src1, GPR:$Src2

Integer signed multiplication long

Multiplies two 32-bit numbers, returning a 64-bit destination register.

## GPR = MulH OpSize:#Size, GPR:$Src1, GPR:$Src2
>GPR = MulH OpSize:#Size, GPR:$Src1, GPR:$Src2

Integer signed multiply returning high results

op:

Tmp <size * 2> = Src1 * Src2;

Dest = Tmp >> (size * 8);

## GPR = UMulH OpSize:#Size, GPR:$Src1, GPR:$Src2
>GPR = UMulH OpSize:#Size, GPR:$Src1, GPR:$Src2

Integer unsigned multiply returning high results

op:

Tmp <size * 2> = Src1 * Src2;

Dest = Tmp >> (size * 8);

## GPR = Bfi OpSize:#Size, u8:$Width, u8:$lsb, GPR:$Dest, GPR:$Src
>GPR = Bfi OpSize:#Size, u8:$Width, u8:$lsb, GPR:$Dest, GPR:$Src

Copies a bitfield from one GPR to another

The source bitfield is from Src[Width:0]

The bitfield is copied in to Dest[(Width + lsb):lsb]

## GPR = Bfxil OpSize:#Size, u8:$Width, u8:$lsb, GPR:$Dest, GPR:$Src
>GPR = Bfxil OpSize:#Size, u8:$Width, u8:$lsb, GPR:$Dest, GPR:$Src

Copies a bitfield from one GPR to another

Inserting in to the low bits of the destination

The source bitfield is from Src[(Width + lsb):lsb]

The bitfield is copied in to Dest[Width:0]

## GPR = Bfe OpSize:#Size, u8:$Width, u8:$lsb, GPR:$Src
>GPR = Bfe OpSize:#Size, u8:$Width, u8:$lsb, GPR:$Src

Extracts a bitfield from one GPR with zext

The source bitfield is from Src[Width:0]

The bitfield is then zero extended

## GPR = Sbfe OpSize:#Size, u8:$Width, u8:$lsb, GPR:$Src
>GPR = Sbfe OpSize:#Size, u8:$Width, u8:$lsb, GPR:$Src

Extracts a bitfield from one GPR with sext

The source bitfield is from Src[Width:0]

The bitfield is then sign extended

## GPR = NZCVSelect OpSize:#ResultSize, CondClass:$Cond, GPR:$TrueVal, GPR:$FalseVal
>GPR = NZCVSelect OpSize:#ResultSize, CondClass:$Cond, GPR:$TrueVal, GPR:$FalseVal

Select based on value in NZCV flags

op:

Dest = Cond ? TrueVal : FalseVal

## FPR = NZCVSelectV OpSize:#ResultSize, CondClass:$Cond, FPR:$TrueVal, FPR:$FalseVal
>FPR = NZCVSelectV OpSize:#ResultSize, CondClass:$Cond, FPR:$TrueVal, FPR:$FalseVal

Select based on value in NZCV flags, where TrueVal and FalseVal are both FPRs.

op:

Dest = Cond ? TrueVal : FalseVal

## GPR = NZCVSelectIncrement OpSize:#ResultSize, CondClass:$Cond, GPR:$TrueVal, GPR:$FalseVal
>GPR = NZCVSelectIncrement OpSize:#ResultSize, CondClass:$Cond, GPR:$TrueVal, GPR:$FalseVal

Select and increment based on value in NZCV flags

op:

Dest = Cond ? TrueVal : (FalseVal + 1)

## GPR = Select OpSize:#ResultSize, OpSize:$CompareSize, CondClass:$Cond, SSA:$Cmp1, SSA:$Cmp2, GPR:$TrueVal, GPR:$FalseVal
>GPR = Select OpSize:#ResultSize, OpSize:$CompareSize, CondClass:$Cond, SSA:$Cmp1, SSA:$Cmp2, GPR:$TrueVal, GPR:$FalseVal

Ternary selection of GPRs

op:

Dest = Cmp1 <Cond> Cmp2 ? TrueVal : FalseVal

## GPR = MaskGenerateFromBitWidth GPR:$BitWidth
>GPR = MaskGenerateFromBitWidth GPR:$BitWidth

Generates a bit mask from with a value from [0, 63]

0 is special cased to full-mask

Special operation for SSE4a bitmask generation.

## GPR = Extr OpSize:#Size, GPR:$Upper, GPR:$Lower, u8:$LSB
>GPR = Extr OpSize:#Size, GPR:$Upper, GPR:$Lower, u8:$LSB

Concats the two GPRs to create a value that is the size of the full two GPRs

It then extracts a bitfield width that size of a GPR from the LSB

Valid LSB range is 0-31 for 32bit and 0-63 for 64bit

<Size * 2> ConcatValue = $Upper:$Lower

Result = ConcatValue<LSB+Size - 1: LSB>

## GPR = PDep OpSize:#Size, GPR:$Input, GPR:$Mask
>GPR = PDep OpSize:#Size, GPR:$Input, GPR:$Mask

Performs a parallel bit deposit.

Takes the contiguous low-order bits and deposits them into

the destination at the locations specified by the Mask.

## GPR = PExt OpSize:#Size, GPR:$Input, GPR:$Mask
>GPR = PExt OpSize:#Size, GPR:$Input, GPR:$Mask

Performs a parallel bit extract.

Each bit set in the mask will select the corresponding bit in the Input

and transfers them to the lower contiguous bits in the destination.

## GPR:$Quotient, GPR:$Remainder = Div OpSize:#Size, GPR:$Lower, GPR:$Upper, GPR:$Divisor
>GPR:$Quotient, GPR:$Remainder = Div OpSize:#Size, GPR:$Lower, GPR:$Upper, GPR:$Divisor

Integer long signed division returning lower bits

The Lower and Upper registers will be concated together to generate a dividend twice the size

Then the divisor divides the temporary dividend and returns the results in the original sized register

If Upper is invalid, this is a non-long division.

## GPR:$Quotient, GPR:$Remainder = UDiv OpSize:#Size, GPR:$Lower, GPR:$Upper, GPR:$Divisor
>GPR:$Quotient, GPR:$Remainder = UDiv OpSize:#Size, GPR:$Lower, GPR:$Upper, GPR:$Divisor

Integer long unsigned division returning lower bits

The Lower and Upper registers will be concated together to generate a dividend twice the size

Then the divisor divides the temporary dividend and returns the results in the original sized register

If Upper is invalid, this is a non-long division.

## Float to GPR
>Float to GPR

XXX: Missing op desc!
## GPR = VExtractToGPR OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector, u8:$Index
>GPR = VExtractToGPR OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector, u8:$Index

Extracts an element from a vector and places it in a GPR

The element that is extracted from the vector is zero extended to the GPR size

## GPR = Float_ToGPR_S OpSize:#DestElementSize, OpSize:$SrcElementSize, FPR:$Scalar
>GPR = Float_ToGPR_S OpSize:#DestElementSize, OpSize:$SrcElementSize, FPR:$Scalar

Moves the scalar element to a GPR with conversion

Converts the 32bit or 64bit float to an signed integer

Rounding mode determined by host flag's rounding mode

## GPR = Float_ToGPR_ZS OpSize:#DestElementSize, OpSize:$SrcElementSize, FPR:$Scalar
>GPR = Float_ToGPR_ZS OpSize:#DestElementSize, OpSize:$SrcElementSize, FPR:$Scalar

Moves the scalar element to a GPR with conversion

Converts the 32bit or 64bit float to an signed integer rounding towards zero (Truncating)

## FCmp OpSize:$ElementSize, FPR:$Scalar1, FPR:$Scalar2
>FCmp OpSize:$ElementSize, FPR:$Scalar1, FPR:$Scalar2

Does a scalar unordered compare and sets NZCV accordingly.

NZCV follows Arm conventions, a separate AXFLAG instruction is required for x86

Ordering flag result is true if either float input is NaN

# Atomic

## GPR = CAS OpSize:#Size, GPR:$Expected, GPR:$Desired, GPR:$Addr
>GPR = CAS OpSize:#Size, GPR:$Expected, GPR:$Desired, GPR:$Addr

Does a compare and swap of values to a memory location

This mostly matches the C++ atomic_compare_exchange_strong function

Dest = atomic_compare_exchange_strong(%Addr, %Expected, %Desired)

Depending on if the value in %Addr is Expected the results destination will be different

Behaves like the following but atomically

Dest = %Expected

if (deref(%Addr) != %Expected) Dest = deref(%Addr)

## GPR:$Lo, GPR:$Hi = CASPair OpSize:#Size, GPR:$ExpectedLo, GPR:$ExpectedHi, GPR:$DesiredLo, GPR:$DesiredHi, GPR:$Addr
>GPR:$Lo, GPR:$Hi = CASPair OpSize:#Size, GPR:$ExpectedLo, GPR:$ExpectedHi, GPR:$DesiredLo, GPR:$DesiredHi, GPR:$Addr

Does a compare and exchange with two pairs of values

ssa0 is the comparison value

ssa1 is the new value

ssa2 is the memory location

Returns the lower & upper halves of the value in memory.

## GPR = AtomicSwap OpSize:#Size, GPR:$Value, GPR:$Addr
>GPR = AtomicSwap OpSize:#Size, GPR:$Value, GPR:$Addr

Atomic integer swap

## GPR = AtomicFetchAdd OpSize:#Size, GPR:$Value, GPR:$Addr
>GPR = AtomicFetchAdd OpSize:#Size, GPR:$Value, GPR:$Addr

Atomic integer fetch and add

Atomically fetches %Addr and adds %value to the memory location

Dest is the value prior to operating on the value in memory

IR layout must match NonFetch-variant, otherwise DCE IR optimization breaks!

## GPR = AtomicFetchSub OpSize:#Size, GPR:$Value, GPR:$Addr
>GPR = AtomicFetchSub OpSize:#Size, GPR:$Value, GPR:$Addr

Atomic integer fetch and sub

Atomically fetches %Addr and subtracts %value to the memory location

Dest is the value prior to operating on the value in memory

IR layout must match NonFetch-variant, otherwise DCE IR optimization breaks!

## GPR = AtomicFetchAnd OpSize:#Size, GPR:$Value, GPR:$Addr
>GPR = AtomicFetchAnd OpSize:#Size, GPR:$Value, GPR:$Addr

Atomic integer fetch and binary and

Atomically fetches %Addr and binary ands %value to the memory location

Dest is the value prior to operating on the value in memory

IR layout must match NonFetch-variant, otherwise DCE IR optimization breaks!

## GPR = AtomicFetchCLR OpSize:#Size, GPR:$Value, GPR:$Addr
>GPR = AtomicFetchCLR OpSize:#Size, GPR:$Value, GPR:$Addr

Atomic integer fetch and binary clear

Atomically fetches %Addr and binary clears %value to the memory location

Dest is the value prior to operating on the value in memory

Matches ARM ldclral semantics

eg: Dest[Addr] &= ~Value

IR layout must match NonFetch-variant, otherwise DCE IR optimization breaks!

## GPR = AtomicFetchOr OpSize:#Size, GPR:$Value, GPR:$Addr
>GPR = AtomicFetchOr OpSize:#Size, GPR:$Value, GPR:$Addr

Atomic integer fetch and binary or

Atomically fetches %Addr and binary ors %value to the memory location

Dest is the value prior to operating on the value in memory

IR layout must match NonFetch-variant, otherwise DCE IR optimization breaks!

## GPR = AtomicFetchXor OpSize:#Size, GPR:$Value, GPR:$Addr
>GPR = AtomicFetchXor OpSize:#Size, GPR:$Value, GPR:$Addr

Atomic integer fetch and binary exclusive or

Atomically fetches %Addr and binary exclusive ors %value to the memory location

Dest is the value prior to operating on the value in memory

IR layout must match NonFetch-variant, otherwise DCE IR optimization breaks!

## GPR = AtomicFetchNeg OpSize:#Size, GPR:$Addr
>GPR = AtomicFetchNeg OpSize:#Size, GPR:$Addr

Atomic integer fetch and two's complement negate

Dest is the value prior to operating on the value in memory

IR layout must match NonFetch-variant, otherwise DCE IR optimization breaks!

## TelemetrySetValue GPR:$Value, u8:$TelemetryValueIndex
>TelemetrySetValue GPR:$Value, u8:$TelemetryValueIndex

Set Telemetry value if the passed in 32-bit value isn't zero.

Only useful for 32-bit applications.

# Backend

## Last
>Last

XXX: Missing op desc!
# Branch

## Jump SSA:$TargetBlock
>Jump SSA:$TargetBlock

XXX: Missing op desc!
## CondJump SSA:$Cmp1, SSA:$Cmp2, SSA:$TrueBlock, SSA:$FalseBlock, CondClass:$Cond{CondClass::NEQ}, OpSize:$CompareSize{OpSize::iInvalid}, i1:$FromNZCV{false}
>CondJump SSA:$Cmp1, SSA:$Cmp2, SSA:$TrueBlock, SSA:$FalseBlock, CondClass:$Cond{CondClass::NEQ}, OpSize:$CompareSize{OpSize::iInvalid}, i1:$FromNZCV{false}

XXX: Missing op desc!
## ExitFunction OpSize:#Size, GPR:$NewRIP, BranchHint:$Hint, GPR:$CallReturnAddress, SSA:$CallReturnBlock
>ExitFunction OpSize:#Size, GPR:$NewRIP, BranchHint:$Hint, GPR:$CallReturnAddress, SSA:$CallReturnBlock

Exits the current JIT function with a target RIP

## Break BreakDefinition:$Reason
>Break BreakDefinition:$Reason

XXX: Missing op desc!
## CallbackReturn
>CallbackReturn

XXX: Missing op desc!
## GPR = Syscall GPR:$SyscallID, GPR:$Arg0, GPR:$Arg1, GPR:$Arg2, GPR:$Arg3, GPR:$Arg4, GPR:$Arg5
>GPR = Syscall GPR:$SyscallID, GPR:$Arg0, GPR:$Arg1, GPR:$Arg2, GPR:$Arg3, GPR:$Arg4, GPR:$Arg5

Dispatches a guest syscall through to the SyscallHandler class

## Thunk GPR:$ArgPtr, SHA256Sum:$ThunkNameHash
>Thunk GPR:$ArgPtr, SHA256Sum:$ThunkNameHash

XXX: Missing op desc!
## GPR:$EAX, GPR:$EBX, GPR:$ECX, GPR:$EDX = CPUID GPR:$Function, GPR:$Leaf
>GPR:$EAX, GPR:$EBX, GPR:$ECX, GPR:$EDX = CPUID GPR:$Function, GPR:$Leaf

Calls in to the CPUID handler function to return emulated CPUID

## GPR:$EAX, GPR:$EDX = XGetBV GPR:$Function
>GPR:$EAX, GPR:$EDX = XGetBV GPR:$Function

Calls in to the XCR handler function to return emulated XCR

# Conv

## FPR = VCastFromGPR OpSize:#RegisterSize, OpSize:#ElementSize, GPR:$Src
>FPR = VCastFromGPR OpSize:#RegisterSize, OpSize:#ElementSize, GPR:$Src

Moves a GPR to a Vector register with zero extension to full length of the register.

No conversion is done on the data as it moves register files

## FPR = VDupFromGPR OpSize:#RegisterSize, OpSize:#ElementSize, GPR:$Src
>FPR = VDupFromGPR OpSize:#RegisterSize, OpSize:#ElementSize, GPR:$Src

Broadcasts a value in a GPR into each ElementSize-sized element in a vector

## FPR = VLoadTwoGPRs GPR:$Lower, GPR:$Upper
>FPR = VLoadTwoGPRs GPR:$Lower, GPR:$Upper

Moves two 64-bit registers to a vector register optimally

## FPR = Float_FromGPR_S OpSize:#DstElementSize, OpSize:$SrcElementSize, GPR:$Src
>FPR = Float_FromGPR_S OpSize:#DstElementSize, OpSize:$SrcElementSize, GPR:$Src

Scalar op: Converts signed GPR to Scalar float

Zeroes the upper bits of the vector register

## FPR = Float_FToF OpSize:#DstElementSize, OpSize:$SrcElementSize, FPR:$Scalar
>FPR = Float_FToF OpSize:#DstElementSize, OpSize:$SrcElementSize, FPR:$Scalar

Scalar op: Converts float from one size to another

Zeroes the upper bits of the vector register

## FPR = Vector_SToF OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector
>FPR = Vector_SToF OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector

Vector op: Converts signed integer to same size float
## FPR = Vector_FToS OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector
>FPR = Vector_FToS OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector

Vector op: Converts float to signed integer, rounding towards zero

Rounding mode determined by host rounding mode

## FPR = Vector_FToZS OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector
>FPR = Vector_FToZS OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector

Vector op: Converts float to signed integer, rounding towards zero
## FPR = Vector_FToF OpSize:#RegisterSize, OpSize:#DestElementSize, FPR:$Vector, OpSize:$SrcElementSize
>FPR = Vector_FToF OpSize:#RegisterSize, OpSize:#DestElementSize, FPR:$Vector, OpSize:$SrcElementSize

Vector op: Converts float from source element size to destination size (fp32<->fp64)
## FPR = VFCVTL2 OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector
>FPR = VFCVTL2 OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector

Vector op: Converts float from source element size to destination size (fp32->fp64)

Selecting from the high half of the register.

## FPR = VFCVTN2 OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$VectorLower, FPR:$VectorUpper
>FPR = VFCVTN2 OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$VectorLower, FPR:$VectorUpper

Vector op: Converts float from source element size and inserting in to the high bits.

Bottom half is untouched

Narrowing to the element size below what is passed in.

F64->F32, F32->F16

## FPR = Vector_FToI OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector, RoundType:$Round
>FPR = Vector_FToI OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector, RoundType:$Round

Vector op: Rounds float to integral

Rounding mode determined by argument

## FPR = Vector_FToISized OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector, i1:$HostRound, OpSize:$IntSize
>FPR = Vector_FToISized OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector, i1:$HostRound, OpSize:$IntSize

Vector op: Rounds float to sized integral

Either host rounding or round-to-zero

Rounding mode determined by argument

## FPR = Vector_F64ToI32 OpSize:#RegisterSize, FPR:$Vector, RoundType:$Round, i1:$EnsureZeroUpperHalf
>FPR = Vector_F64ToI32 OpSize:#RegisterSize, FPR:$Vector, RoundType:$Round, i1:$EnsureZeroUpperHalf

Vector op: Rounds 64-bit float to 32-bit integral with round mode

Matches CVTPD2DQ/CVTTPD2DQ behaviour

# Crypto

## FPR = VAESImc FPR:$Vector
>FPR = VAESImc FPR:$Vector

Does a stage of the inverse mix column transformation
## FPR = VAESEnc OpSize:#RegisterSize, FPR:$State, FPR:$Key, FPR:$ZeroReg
>FPR = VAESEnc OpSize:#RegisterSize, FPR:$State, FPR:$Key, FPR:$ZeroReg

Does a step of AES encryption
## FPR = VAESEncLast OpSize:#RegisterSize, FPR:$State, FPR:$Key, FPR:$ZeroReg
>FPR = VAESEncLast OpSize:#RegisterSize, FPR:$State, FPR:$Key, FPR:$ZeroReg

Does the last step of AES encryption
## FPR = VAESDec OpSize:#RegisterSize, FPR:$State, FPR:$Key, FPR:$ZeroReg
>FPR = VAESDec OpSize:#RegisterSize, FPR:$State, FPR:$Key, FPR:$ZeroReg

Does a step of AES decryption
## FPR = VAESDecLast OpSize:#RegisterSize, FPR:$State, FPR:$Key, FPR:$ZeroReg
>FPR = VAESDecLast OpSize:#RegisterSize, FPR:$State, FPR:$Key, FPR:$ZeroReg

Does the last step of AES decryption
## FPR = VAESKeyGenAssist FPR:$Src, FPR:$KeyGenTBLSwizzle, FPR:$ZeroReg, u8:$RCON
>FPR = VAESKeyGenAssist FPR:$Src, FPR:$KeyGenTBLSwizzle, FPR:$ZeroReg, u8:$RCON

Assists in key generation
## FPR = VSha1H FPR:$Src
>FPR = VSha1H FPR:$Src

Does vector scalar SHA1H instruction
## FPR = VSha1C FPR:$Src1, FPR:$Src2, FPR:$Src3
>FPR = VSha1C FPR:$Src1, FPR:$Src2, FPR:$Src3

Does vector SHA1C instruction
## FPR = VSha1M FPR:$Src1, FPR:$Src2, FPR:$Src3
>FPR = VSha1M FPR:$Src1, FPR:$Src2, FPR:$Src3

Does vector SHA1M instruction
## FPR = VSha1P FPR:$Src1, FPR:$Src2, FPR:$Src3
>FPR = VSha1P FPR:$Src1, FPR:$Src2, FPR:$Src3

Does vector SHA1P instruction
## FPR = VSha1SU1 FPR:$Src1, FPR:$Src2
>FPR = VSha1SU1 FPR:$Src1, FPR:$Src2

Does vector scalar SHA1H instruction
## FPR = VSha256U0 FPR:$Src1, FPR:$Src2
>FPR = VSha256U0 FPR:$Src1, FPR:$Src2

Does vector scalar VSha256U0 instruction
## FPR = VSha256U1 FPR:$Src1, FPR:$Src2
>FPR = VSha256U1 FPR:$Src1, FPR:$Src2

Does vector scalar VSha256U1 instruction
## FPR = VSha256H FPR:$Src1, FPR:$Src2, FPR:$Src3
>FPR = VSha256H FPR:$Src1, FPR:$Src2, FPR:$Src3

Does vector scalar VSha256H instruction
## FPR = VSha256H2 FPR:$Src1, FPR:$Src2, FPR:$Src3
>FPR = VSha256H2 FPR:$Src1, FPR:$Src2, FPR:$Src3

Does vector scalar VSha256H2 instruction
## GPR = CRC32 GPR:$Src1, GPR:$Src2, OpSize:$SrcSize
>GPR = CRC32 GPR:$Src1, GPR:$Src2, OpSize:$SrcSize

CRC32 using polynomial 0x1EDC6F41

## FPR = PCLMUL OpSize:#RegisterSize, FPR:$Src1, FPR:$Src2, u8:$Selector
>FPR = PCLMUL OpSize:#RegisterSize, FPR:$Src1, FPR:$Src2, u8:$Selector

Performs carryless multiplication of 64-bit elements depending on the selector.

Selector = 0b00000000: Uses low 64-bit elements from both input vectors

Selector = 0b00000001: Uses high 64-bit element from Src1 and low 64-bit element from Src2

Selector = 0b00010000: Uses low 64-bit element from Src1 and high 64-bit element from Src2

Selector = 0b00010001: Uses high 64-bit elements from both input vectors

# F64

## FPR = F64ATAN FPR:$Src1, FPR:$Src2
>FPR = F64ATAN FPR:$Src1, FPR:$Src2

XXX: Missing op desc!
## FPR = F64FPREM FPR:$Src1, FPR:$Src2
>FPR = F64FPREM FPR:$Src1, FPR:$Src2

XXX: Missing op desc!
## FPR = F64FPREM1 FPR:$Src1, FPR:$Src2
>FPR = F64FPREM1 FPR:$Src1, FPR:$Src2

XXX: Missing op desc!
## FPR = F64SCALE FPR:$Src1, FPR:$Src2
>FPR = F64SCALE FPR:$Src1, FPR:$Src2

XXX: Missing op desc!
## FPR = F64F2XM1 FPR:$Src
>FPR = F64F2XM1 FPR:$Src

XXX: Missing op desc!
## FPR = F64FYL2X FPR:$Src, FPR:$Src2
>FPR = F64FYL2X FPR:$Src, FPR:$Src2

XXX: Missing op desc!
## FPR = F64FYL2XP1 FPR:$Src, FPR:$Src2
>FPR = F64FYL2XP1 FPR:$Src, FPR:$Src2

XXX: Missing op desc!
## FPR = F64TAN FPR:$Src
>FPR = F64TAN FPR:$Src

XXX: Missing op desc!
## FPR = F64SIN FPR:$Src
>FPR = F64SIN FPR:$Src

XXX: Missing op desc!
## FPR = F64COS FPR:$Src
>FPR = F64COS FPR:$Src

XXX: Missing op desc!
## FPR:$Sin, FPR:$Cos = F64SINCOS FPR:$Src
>FPR:$Sin, FPR:$Cos = F64SINCOS FPR:$Src

XXX: Missing op desc!
# F80

## GPR = SyncStackToSlow
>GPR = SyncStackToSlow

Synchronizes the virtual stack environment to the physical registers.

Returns the current stack top.

## StackForceSlow
>StackForceSlow

Forces the slow path.

## InitStack
>InitStack

Initializes the stack by marking all tags as invalid and setting top to zero.

## IncStackTop
>IncStackTop

Increase stack top-pointer.

## DecStackTop
>DecStackTop

Decrease stack top-pointer.

## InvalidateStack u8:$StackLocation
>InvalidateStack u8:$StackLocation

Marks the value in TOP+$StackLocation as empty / invalid 0b11.

If the StackLocation is 0xff, we invalidate all locations.

## PushStack FPR:$X80Src, FPR:$OriginalValue, OpSize:$LoadSize
>PushStack FPR:$X80Src, FPR:$OriginalValue, OpSize:$LoadSize

Pushes the provided X80Src source on to the x87 stack.

Tracks OriginalValue as the original value of X80Src. OriginalValue can be Invalid() in which case no tracking is done.

Opsize is 128bit for F80 values, 64-bit for low precision.

LoadSize the original load size, i.e. of size of OriginalValue.

Float: 80-bit, 64-bit, 32-bit

## CopyPushStack u8:$StackLocation
>CopyPushStack u8:$StackLocation

Pushes an element already on the stack onto the top.

## StoreStackMem OpSize:$SourceSize, OpSize:$StoreSize, GPR:$Addr, GPR:$Offset, OpSize:$Align, MemOffsetType:$OffsetType, u8:$OffsetScale
>StoreStackMem OpSize:$SourceSize, OpSize:$StoreSize, GPR:$Addr, GPR:$Offset, OpSize:$Align, MemOffsetType:$OffsetType, u8:$OffsetScale

Takes the top value off the x87 stack and stores it to memory.

SourceSize is 128bit for F80 values, 64-bit for low precision.

StoreSize is the store size for conversion:

Float: 80-bit, 64-bit, or 32-bit

## StoreStackToStack u8:$StackLocation
>StoreStackToStack u8:$StackLocation

Takes the top value off the x87 stack and stores it to stack location TOP+StackLocation

Float: 80-bit, 64-bit, or 32-bit

Int: 64-bit, 32-bit, 16-bit

## PopStackDestroy
>PopStackDestroy

Pops the top value off the stack but doesn't save it anywhere.

## FPR = ReadStackValue u8:$StackLocation
>FPR = ReadStackValue u8:$StackLocation

Reads a value off the stack at the offset

## GPR = StackValidTag u8:$StackLocation
>GPR = StackValidTag u8:$StackLocation

Returns 1 if the value in location TOP+$StackLocation is valid, 0 otherwise.

## F80AddStack u8:$SrcStack1, u8:$SrcStack2
>F80AddStack u8:$SrcStack1, u8:$SrcStack2

Adds two stack locations together, storing the result in to the first stack location

## F80AddValue u8:$SrcStack, FPR:$X80Src
>F80AddValue u8:$SrcStack, FPR:$X80Src

Adds a operand value to a stack location. The result stored in to the stack location provided.

## FPR = F80Add FPR:$X80Src1, FPR:$X80Src2
>FPR = F80Add FPR:$X80Src1, FPR:$X80Src2

XXX: Missing op desc!
## F80SubStack u8:$DstStack, u8:$SrcStack1, u8:$SrcStack2
>F80SubStack u8:$DstStack, u8:$SrcStack1, u8:$SrcStack2

Subtracts the value in stack location TOP+$SrcStack2 from the value in stack location TOP+$SrcStack1.

The result is stored in stack location TOP+$DstStack.

## F80SubValue u8:$SrcStack, FPR:$X80Src
>F80SubValue u8:$SrcStack, FPR:$X80Src

Subtracts the value $X80Src from the value in stack location TOP+$SrcStack.

The result is stored in stack location TOP.

## F80SubRValue FPR:$X80Src, u8:$SrcStack
>F80SubRValue FPR:$X80Src, u8:$SrcStack

Subtracts the value in stack location TOP+$SrcStack from the value $X80Src.

The result is stored in stack location TOP.

## FPR = F80Sub FPR:$X80Src1, FPR:$X80Src2
>FPR = F80Sub FPR:$X80Src1, FPR:$X80Src2

Subtracts the value in $X80Src1 from the value in $X80Src2.

The result is returned.

`FPR = X80Src2 - X80Src1`

## F80MulStack u8:$SrcStack1, u8:$SrcStack2
>F80MulStack u8:$SrcStack1, u8:$SrcStack2

Multiplies two stack locations together, storing the result in to the first stack location

## F80MulValue u8:$SrcStack, FPR:$X80Src
>F80MulValue u8:$SrcStack, FPR:$X80Src

Multiplies a operand value to a stack location. The result stored in to the stack location provided.

## FPR = F80Mul FPR:$X80Src1, FPR:$X80Src2
>FPR = F80Mul FPR:$X80Src1, FPR:$X80Src2

XXX: Missing op desc!
## F80DivStack u8:$DstStack, u8:$SrcStack1, u8:$SrcStack2
>F80DivStack u8:$DstStack, u8:$SrcStack1, u8:$SrcStack2

Divides the value in stack location TOP+$SrcStack1 by the value in stack location TOP+$SrcStack2.

The result is stored in stack location TOP+$DstStack.

`FPR|Stack[TOP+DstStack] = Stack[TOP+SrcStack1] / Stack[TOP+SrcStack2]`

## F80DivValue u8:$SrcStack, FPR:$X80Src
>F80DivValue u8:$SrcStack, FPR:$X80Src

Divides the value in stack location TOP+$SrcStack by the value $X80Src.

The result is stored in stack location TOP and returned.

`FPR|Stack[TOP] = Stack[TOP+SrcStack] / X80Src`

## F80DivRValue FPR:$X80Src, u8:$SrcStack
>F80DivRValue FPR:$X80Src, u8:$SrcStack

Divides the value X80Src by the value in stack location TOP+$SrcStack.

The result is stored in stack location TOP.

`FPR|Stack[TOP] = X80Src / Stack[TOP+SrcStack]`

## FPR = F80Div FPR:$X80Src1, FPR:$X80Src2
>FPR = F80Div FPR:$X80Src1, FPR:$X80Src2

Divides the value in $X80Src1 by the value in $X80Src2.

The result is returned.

`FPR = X80Src1 / X80Src2`

## F80StackXchange u8:$SrcStack
>F80StackXchange u8:$SrcStack

Exchanges the value at the top of the stack with the value at TOP+$SrcStack.

## FPR = F80StackChangeSign
>FPR = F80StackChangeSign

Complements the sign bit of the value at the top of the stack.

Returns the new value at the top of the stack.

## FPR = F80StackAbs
>FPR = F80StackAbs

Clears the sign bit of the value at the top of the stack.

Returns the new value at the top of the stack.

## F80PTANStack
>F80PTANStack

Computes the approximate tangent of the source operand in register ST(0), stores the result in ST(0), and pushes a 1.0 onto the FPU register stack.

## FPR = F80ATANStack
>FPR = F80ATANStack

Computes arctan(st1/st0) and stores it in st0. Then pops the stack.

## FPR = F80ATAN FPR:$X80Src1, FPR:$X80Src2
>FPR = F80ATAN FPR:$X80Src1, FPR:$X80Src2

XXX: Missing op desc!
## F80FPREMStack
>F80FPREMStack

XXX: Missing op desc!
## FPR = F80FPREM FPR:$X80Src1, FPR:$X80Src2
>FPR = F80FPREM FPR:$X80Src1, FPR:$X80Src2

XXX: Missing op desc!
## F80FPREM1Stack
>F80FPREM1Stack

XXX: Missing op desc!
## FPR = F80FPREM1 FPR:$X80Src1, FPR:$X80Src2
>FPR = F80FPREM1 FPR:$X80Src1, FPR:$X80Src2

XXX: Missing op desc!
## F80SCALEStack
>F80SCALEStack

XXX: Missing op desc!
## FPR = F80SCALE FPR:$X80Src1, FPR:$X80Src2
>FPR = F80SCALE FPR:$X80Src1, FPR:$X80Src2

XXX: Missing op desc!
## FPR = F80CVT OpSize:#Size, FPR:$X80Src
>FPR = F80CVT OpSize:#Size, FPR:$X80Src

XXX: Missing op desc!
## GPR = F80CVTInt OpSize:#Size, FPR:$X80Src, i1:$Truncate
>GPR = F80CVTInt OpSize:#Size, FPR:$X80Src, i1:$Truncate

XXX: Missing op desc!
## FPR = F80CVTTo FPR:$X80Src, OpSize:$SrcSize
>FPR = F80CVTTo FPR:$X80Src, OpSize:$SrcSize

XXX: Missing op desc!
## FPR = F80CVTToInt GPR:$Src, OpSize:$SrcSize
>FPR = F80CVTToInt GPR:$Src, OpSize:$SrcSize

XXX: Missing op desc!
## F80RoundStack
>F80RoundStack

Replaces the value at the top of the stack with its nearest integral value.

## FPR = F80Round FPR:$X80Src
>FPR = F80Round FPR:$X80Src

XXX: Missing op desc!
## F80F2XM1Stack
>F80F2XM1Stack

XXX: Missing op desc!
## FPR = F80F2XM1 FPR:$X80Src
>FPR = F80F2XM1 FPR:$X80Src

XXX: Missing op desc!
## FPR = F80TAN FPR:$X80Src
>FPR = F80TAN FPR:$X80Src

XXX: Missing op desc!
## F80SINStack
>F80SINStack

XXX: Missing op desc!
## FPR = F80SIN FPR:$X80Src
>FPR = F80SIN FPR:$X80Src

XXX: Missing op desc!
## F80COSStack
>F80COSStack

XXX: Missing op desc!
## FPR = F80COS FPR:$X80Src
>FPR = F80COS FPR:$X80Src

XXX: Missing op desc!
## FPR:$Sin, FPR:$Cos = F80SINCOS FPR:$X80Src
>FPR:$Sin, FPR:$Cos = F80SINCOS FPR:$X80Src

XXX: Missing op desc!
## F80SINCOSStack
>F80SINCOSStack

XXX: Missing op desc!
## F80SQRTStack
>F80SQRTStack

XXX: Missing op desc!
## FPR = F80SQRT FPR:$X80Src
>FPR = F80SQRT FPR:$X80Src

XXX: Missing op desc!
## FPR = F80XTRACT_EXP FPR:$X80Src
>FPR = F80XTRACT_EXP FPR:$X80Src

XXX: Missing op desc!
## FPR = F80XTRACT_SIG FPR:$X80Src
>FPR = F80XTRACT_SIG FPR:$X80Src

XXX: Missing op desc!
## GPR = F80StackTest u8:$SrcStack
>GPR = F80StackTest u8:$SrcStack

Does comparison between value in stack at TOP + SrcStack

## GPR = F80CmpStack u8:$SrcStack
>GPR = F80CmpStack u8:$SrcStack

Does a scalar unordered compare between the value at the top of the stack and the value in stack position TOP+$SrcStack and stores the flags in to a GPR

Ordering flag result is true if either float input is NaN

## GPR = F80CmpValue FPR:$X80Src
>GPR = F80CmpValue FPR:$X80Src

Does a scalar unordered compare between the value at the top of the stack and $X80Src and stores the asked for flags in to a GPR

Ordering flag result is true if either float input is NaN

## GPR = F80Cmp FPR:$X80Src1, FPR:$X80Src2
>GPR = F80Cmp FPR:$X80Src1, FPR:$X80Src2

Does a scalar unordered compare and stores the flags in to a GPR

Ordering flag result is true if either float input is NaN

## FPR = F80BCDLoad FPR:$X80Src
>FPR = F80BCDLoad FPR:$X80Src

XXX: Missing op desc!
## FPR = F80BCDStore FPR:$X80Src
>FPR = F80BCDStore FPR:$X80Src

XXX: Missing op desc!
## FPR = F80FYL2XStack
>FPR = F80FYL2XStack

Computes ST1 * log2(ST0)

Stores the result in ST1, and pops the top of the stack.

Returns the new value at the top of the stack, i.e. the result of the operation.

## FPR = F80FYL2X FPR:$X80Src1, FPR:$X80Src2
>FPR = F80FYL2X FPR:$X80Src1, FPR:$X80Src2

XXX: Missing op desc!
## FPR = F80FYL2XP1Stack
>FPR = F80FYL2XP1Stack

Computes ST1 * log2(1 + ST0)

Stores the result in ST1, and pops the top of the stack.

Returns the new value at the top of the stack, i.e. the result of the operation.

## FPR = F80FYL2XP1 FPR:$X80Src1, FPR:$X80Src2
>FPR = F80FYL2XP1 FPR:$X80Src1, FPR:$X80Src2

XXX: Missing op desc!
## F80VBSLStack OpSize:#RegisterSize, FPR:$VectorMask, u8:$SrcStack1, u8:$SrcStack2
>F80VBSLStack OpSize:#RegisterSize, FPR:$VectorMask, u8:$SrcStack1, u8:$SrcStack2

Does a vector bitwise select.

If the bit in the field is 1 then the corresponding bit is pulled from VectorTrue

If the bit in the field is 0 then the corresponding bit is pulled from VectorFalse

Writes the result to the top of the stack.

# Memory

## SSA = LoadContext OpSize:#ByteSize, RegisterClass:$Class, u32:$Offset
>SSA = LoadContext OpSize:#ByteSize, RegisterClass:$Class, u32:$Offset

Loads a value from the context with offset

Dest = Ctx[Offset]

## SSA:$Value1, SSA:$Value2 = LoadContextPair OpSize:#ByteSize, RegisterClass:$Class, u32:$Offset
>SSA:$Value1, SSA:$Value2 = LoadContextPair OpSize:#ByteSize, RegisterClass:$Class, u32:$Offset

Loads a pair of values from the context with offset

Value0 = Ctx[Offset], Value1 = Ctx[Offset + ByteSize]

## StoreContext OpSize:#ByteSize, RegisterClass:$Class, SSA:$Value, u32:$Offset
>StoreContext OpSize:#ByteSize, RegisterClass:$Class, SSA:$Value, u32:$Offset

Stores a value to the context with offset

Ctx[Offset] = Value

Zero Extends if value's type is too small

Truncates if value's type is too large

## StoreContextPair OpSize:#ByteSize, RegisterClass:$Class, SSA:$Value1, SSA:$Value2, u32:$Offset
>StoreContextPair OpSize:#ByteSize, RegisterClass:$Class, SSA:$Value1, SSA:$Value2, u32:$Offset

Stores a pair of values to the context with offset

Ctx[Offset] = Value1, Ctx[Offset + ByteSize] = Value2

Zero Extends if value's type is too small

Truncates if value's type is too large

## SSA = LoadContextIndexed GPR:$Index, OpSize:#ByteSize, u32:$BaseOffset, u32:$Stride, RegisterClass:$Class
>SSA = LoadContextIndexed GPR:$Index, OpSize:#ByteSize, u32:$BaseOffset, u32:$Stride, RegisterClass:$Class

Loads a value from the context with offset and indexed by SSA value

Dest = Ctx[BaseOffset + Index * Stride]

## StoreContextIndexed SSA:$Value, GPR:$Index, OpSize:#ByteSize, u32:$BaseOffset, u32:$Stride, RegisterClass:$Class
>StoreContextIndexed SSA:$Value, GPR:$Index, OpSize:#ByteSize, u32:$BaseOffset, u32:$Stride, RegisterClass:$Class

Stores a value to the context with offset and indexed by SSA value

Ctx[BaseOffset + Index * Stride] = Value

## GPR = FormContextAddress OpSize:#Size, GPR:$Index, u32:$Stride
>GPR = FormContextAddress OpSize:#Size, GPR:$Index, u32:$Stride

Forms an address into the context structure indexed by SSA value

Dest = Ctx + Index * Stride

This allows backends to compute the address once and reuse it for multiple memory operations

Stride must be a power of 2

## SpillRegister SSA:$Value, u32:$Slot, RegisterClass:$Class
>SpillRegister SSA:$Value, u32:$Slot, RegisterClass:$Class

Spills an SSA value to memory

Spill slots are register allocated and has live ranges calculated to handle slot calculation

!Don't use this op. It is for RA to handle spilling and filling!

## SSA = FillRegister OpSize:#Size, OpSize:#ElementSize, u32:$Slot, RegisterClass:$Class
>SSA = FillRegister OpSize:#Size, OpSize:#ElementSize, u32:$Slot, RegisterClass:$Class

Fills a register from a spill slot

Spill slots are register allocated and has live ranges calculated to handle slot calculation

!Don't use this op. It is for RA to handle spilling and filling!

## GPR = LoadNZCV
>GPR = LoadNZCV

Loads value of NZCV register

## StoreNZCV GPR:$Value
>StoreNZCV GPR:$Value

Stores value to NZCV register

## GPR = LoadDF
>GPR = LoadDF

Loads the decimal flag from the context object in -1/1

representation for easy consumption

## SSA = LoadMem RegisterClass:$Class, OpSize:#Size, GPR:$Addr, GPR:$Offset, OpSize:$Align, MemOffsetType:$OffsetType, u8:$OffsetScale
>SSA = LoadMem RegisterClass:$Class, OpSize:#Size, GPR:$Addr, GPR:$Offset, OpSize:$Align, MemOffsetType:$OffsetType, u8:$OffsetScale

XXX: Missing op desc!
## SSA:$Value1, SSA:$Value2 = LoadMemPair RegisterClass:$Class, OpSize:#Size, GPR:$Addr, u32:$Offset
>SSA:$Value1, SSA:$Value2 = LoadMemPair RegisterClass:$Class, OpSize:#Size, GPR:$Addr, u32:$Offset

Load a pair of values from memory.

## StoreMem RegisterClass:$Class, OpSize:#Size, SSA:$Value, GPR:$Addr, GPR:$Offset, OpSize:$Align, MemOffsetType:$OffsetType, u8:$OffsetScale
>StoreMem RegisterClass:$Class, OpSize:#Size, SSA:$Value, GPR:$Addr, GPR:$Offset, OpSize:$Align, MemOffsetType:$OffsetType, u8:$OffsetScale

Stores a value to memory.

Zero Extends if value's type is too small

Truncates if value's type is too large

## StoreMemPair RegisterClass:$Class, OpSize:#Size, SSA:$Value1, SSA:$Value2, GPR:$Addr, u32:$Offset
>StoreMemPair RegisterClass:$Class, OpSize:#Size, SSA:$Value1, SSA:$Value2, GPR:$Addr, u32:$Offset

Stores a pair of values to memory.

Zero Extends if value's type is too small

Truncates if value's type is too large

## StoreMemX87SVEOptPredicate OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Value, GPR:$Addr
>StoreMemX87SVEOptPredicate OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Value, GPR:$Addr

Stores a value to memory using SVE predicate mask that's designed

specifically for use in the X87 SVE Ldst optimization.

## FPR = LoadMemX87SVEOptPredicate OpSize:#RegisterSize, OpSize:#ElementSize, GPR:$Addr
>FPR = LoadMemX87SVEOptPredicate OpSize:#RegisterSize, OpSize:#ElementSize, GPR:$Addr

Loads a value to memory using SVE predicate mask that's designed

specifically for use in the X87 SVE Ldst optimization.

## SSA = LoadMemTSO RegisterClass:$Class, OpSize:#Size, GPR:$Addr, GPR:$Offset, OpSize:$Align, MemOffsetType:$OffsetType, u8:$OffsetScale
>SSA = LoadMemTSO RegisterClass:$Class, OpSize:#Size, GPR:$Addr, GPR:$Offset, OpSize:$Align, MemOffsetType:$OffsetType, u8:$OffsetScale

Does a x86 TSO compatible load from memory. Offset must be Invalid().

## StoreMemTSO RegisterClass:$Class, OpSize:#Size, SSA:$Value, GPR:$Addr, GPR:$Offset, OpSize:$Align, MemOffsetType:$OffsetType, u8:$OffsetScale
>StoreMemTSO RegisterClass:$Class, OpSize:#Size, SSA:$Value, GPR:$Addr, GPR:$Offset, OpSize:$Align, MemOffsetType:$OffsetType, u8:$OffsetScale

Does a x86 TSO compatible store to memory. Offset must be Invalid().

## FPR = VLoadVectorMasked OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Mask, GPR:$Addr, GPR:$Offset, MemOffsetType:$OffsetType, u8:$OffsetScale
>FPR = VLoadVectorMasked OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Mask, GPR:$Addr, GPR:$Offset, MemOffsetType:$OffsetType, u8:$OffsetScale

Does a masked load similar to VPMASKMOV/VMASKMOV where the upper bit of each element

determines whether or not that element will be loaded from memory

## VStoreVectorMasked OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Mask, FPR:$Data, GPR:$Addr, GPR:$Offset, MemOffsetType:$OffsetType, u8:$OffsetScale
>VStoreVectorMasked OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Mask, FPR:$Data, GPR:$Addr, GPR:$Offset, MemOffsetType:$OffsetType, u8:$OffsetScale

Does a masked store similar to VPMASKMOV/VMASKMOV where the upper bit of each element

determines whether or not that element will be stored to memory

## FPR = VLoadVectorGatherMasked OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Incoming, FPR:$Mask, GPR:$AddrBase, FPR:$VectorIndexLow, FPR:$VectorIndexHigh, OpSize:$VectorIndexElementSize, u8:$OffsetScale, u8:$DataElementOffsetStart, u8:$IndexElementOffsetStart, OpSize:$AddrSize
>FPR = VLoadVectorGatherMasked OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Incoming, FPR:$Mask, GPR:$AddrBase, FPR:$VectorIndexLow, FPR:$VectorIndexHigh, OpSize:$VectorIndexElementSize, u8:$OffsetScale, u8:$DataElementOffsetStart, u8:$IndexElementOffsetStart, OpSize:$AddrSize

Does a masked load similar to VPGATHERD* where the upper bit of each element

determines whether or not that element will be loaded from memory.

Most of VSIB encoding is passed directly through to the IR operation.

## FPR = VLoadVectorGatherMaskedQPS OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Incoming, FPR:$MaskReg, GPR:$AddrBase, FPR:$VectorIndexLow, FPR:$VectorIndexHigh, u8:$OffsetScale, OpSize:$AddrSize
>FPR = VLoadVectorGatherMaskedQPS OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Incoming, FPR:$MaskReg, GPR:$AddrBase, FPR:$VectorIndexLow, FPR:$VectorIndexHigh, u8:$OffsetScale, OpSize:$AddrSize

Does a masked load similar to VPGATHERQPS where the upper bit of each element

determines whether or not that element will be loaded from memory.

Most of VSIB encoding is passed directly through to the IR operation.

Only supports the case of 32-bit data element sizes from 64-bit addresses

## FPR = VLoadVectorElement OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$DstSrc, u8:$Index, GPR:$Addr
>FPR = VLoadVectorElement OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$DstSrc, u8:$Index, GPR:$Addr

Does a memory load to a single element of a vector.

Leaves the rest of the vector's data intact.

Matches arm64 ld1 semantics

## VStoreVectorElement OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Value, u8:$Index, GPR:$Addr
>VStoreVectorElement OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Value, u8:$Index, GPR:$Addr

Does a memory store of a single element of a vector.

Matches arm64 st1 semantics

## FPR = VBroadcastFromMem OpSize:#RegisterSize, OpSize:#ElementSize, GPR:$Address
>FPR = VBroadcastFromMem OpSize:#RegisterSize, OpSize:#ElementSize, GPR:$Address

Broadcasts an ElementSize value from memory into each element of a vector.

## GPR = Push OpSize:#Size, OpSize:$ValueSize, GPR:$Value, GPR:$Addr
>GPR = Push OpSize:#Size, OpSize:$ValueSize, GPR:$Value, GPR:$Addr

Pushes a value to the address, returning the new pointer after incrementing.

The address is decremented by the value size while.

The return value size is the size of the current operating mode

## PushTwo OpSize:#Size, OpSize:$ValueSize, GPR:$Value1, GPR:$Value2, GPR:$Addr
>PushTwo OpSize:#Size, OpSize:$ValueSize, GPR:$Value1, GPR:$Value2, GPR:$Addr

Push two values to the address, incrementing the pointer in the place.

Fused post-RA so doesn't have a destination.

## GPR = RMWHandle GPR:$Value
>GPR = RMWHandle GPR:$Value

This is a special move that indicates the result will be poisoned by a non-SSA instruction writing to its result.

In effect, it serves to prevent invalid optimizations with non-SSA instructions.

## GPR:$Addr, GPR:$Value = Pop OpSize:$Size, GPR:$Addr
>GPR:$Addr, GPR:$Value = Pop OpSize:$Size, GPR:$Addr

Pops a value from the address, updating the new pointer after incrementing.

The address is incremented by the size via an RMW source/destintaion.

## GPR:$Addr, GPR:$Value1, GPR:$Value2 = PopTwo OpSize:$Size, GPR:$Addr
>GPR:$Addr, GPR:$Value1, GPR:$Value2 = PopTwo OpSize:$Size, GPR:$Addr

Pop two values from the address. Fused post-RA.

## GPR = MemSet i1:$IsAtomic, OpSize:$Size, GPR:$Prefix, GPR:$Addr, GPR:$Value, GPR:$Length, GPR:$Direction
>GPR = MemSet i1:$IsAtomic, OpSize:$Size, GPR:$Prefix, GPR:$Addr, GPR:$Value, GPR:$Length, GPR:$Direction

Duplicates behaviour of x86 STOS repeat

Returns the final address that gets generated without the prefix appended.

## GPR:$DstAddress, GPR:$SrcAddress = MemCpy i1:$IsAtomic, OpSize:$Size, GPR:$Dest, GPR:$Src, GPR:$Length, GPR:$Direction
>GPR:$DstAddress, GPR:$SrcAddress = MemCpy i1:$IsAtomic, OpSize:$Size, GPR:$Dest, GPR:$Src, GPR:$Length, GPR:$Direction

Duplicates behaviour of x86 MOVS repeat

Returns the final addresses after they have been incremented or decremented

## CacheLineClear GPR:$Addr, i1:$Serialize
>CacheLineClear GPR:$Addr, i1:$Serialize

Does a 64 byte cacheline clear at the address specified

Only clears the data cachelines. Doesn't do any zeroing

Can skip serialization if requested.

## CacheLineClean GPR:$Addr
>CacheLineClean GPR:$Addr

Does a 64 byte cacheline clean at the address specified

Only cleans the data cachelines. Doesn't do any zeroing

Skips the invalidation step of the CacheLineClear operation

## CacheLineZero GPR:$Addr
>CacheLineZero GPR:$Addr

Does a 64 byte zero at the address specified

Writing zeroes to memory

It is specifically non-temporal and weakly ordered

This matches CLZero behaviour

## Fence FenceType:$Fence
>Fence FenceType:$Fence

Does a memory fence operation of the desired type

FenceType::Load: Ensures load memory operations are serialized

FenceType::Store: Ensures store memory operations are serialized

FenceType::LoadStore: Ensures loads and store memory operations are serialized

FenceType::Inst: Instruction barrier. Ensures all instructions after this point will be explicitly fetched

Ensures the memory operations are globally visible

## Prefetch i1:$ForStore, i1:$Stream, i8:$CacheLevel, GPR:$Addr, GPR:$Offset, MemOffsetType:$OffsetType, u8:$OffsetScale
>Prefetch i1:$ForStore, i1:$Stream, i8:$CacheLevel, GPR:$Addr, GPR:$Offset, MemOffsetType:$OffsetType, u8:$OffsetScale

Does a cacheline prefetch operation

## VStoreNonTemporal OpSize:#RegisterSize, FPR:$Value, GPR:$Addr, i8:$Offset
>VStoreNonTemporal OpSize:#RegisterSize, FPR:$Value, GPR:$Addr, i8:$Offset

Does a non-temporal memory store of a vector.

Matches arm64 SVE stnt1b semantics.

Specifically weak-memory model ordered to match x86 non-temporal stores.

## VStoreNonTemporalPair OpSize:#RegisterSize, FPR:$ValueLow, FPR:$ValueHigh, GPR:$Addr, i8:$Offset
>VStoreNonTemporalPair OpSize:#RegisterSize, FPR:$ValueLow, FPR:$ValueHigh, GPR:$Addr, i8:$Offset

Does a non-temporal memory store of two vector registers.

Matches arm64 stnp semantics.

Specifically weak-memory model ordered to match x86 non-temporal stores.

## FPR = VLoadNonTemporal OpSize:#RegisterSize, GPR:$Addr, i8:$Offset
>FPR = VLoadNonTemporal OpSize:#RegisterSize, GPR:$Addr, i8:$Offset

Does a non-temporal memory load of a vector.

Matches arm64 SVE ldnt1b semantics.

Specifically weak-memory model ordered to match x86 non-temporal stores.

## ContextClear u32:$Offset, u32:$Size
>ContextClear u32:$Offset, u32:$Size

Clears a region of the context by CLZero size

Both the offset and size alignment need to be by CLZero size

# Misc

## Dummy
>Dummy

XXX: Missing op desc!
## IRHeader SSA:$Blocks, u64:$OriginalRIP, u32:$BlockCount, u32:$NumHostInstructions, u32:$SpillSlots, i1:$PostRA{false}, i1:$HasX87{false}, i1:$ReadsParity{false}
>IRHeader SSA:$Blocks, u64:$OriginalRIP, u32:$BlockCount, u32:$NumHostInstructions, u32:$SpillSlots, i1:$PostRA{false}, i1:$HasX87{false}, i1:$ReadsParity{false}

XXX: Missing op desc!
## CodeBlock SSA:$Begin, SSA:$Last, u32:$ID, i1:$EntryPoint{false}, u32:$GuestEntryOffset{0}
>CodeBlock SSA:$Begin, SSA:$Last, u32:$ID, i1:$EntryPoint{false}, u32:$GuestEntryOffset{0}

XXX: Missing op desc!
## BeginBlock SSA:$BlockHeader
>BeginBlock SSA:$BlockHeader

XXX: Missing op desc!
## InvalidateFlags u64:$Flags
>InvalidateFlags u64:$Flags

XXX: Missing op desc!
## EndBlock SSA:$BlockHeader
>EndBlock SSA:$BlockHeader

XXX: Missing op desc!
## GuestOpcode u32:$GuestEntryOffset
>GuestOpcode u32:$GuestEntryOffset

Marks the beginning of a guest opcode

## GPR = ValidateCode Array16:$CodeOriginal, GPR:$Address, u8:$CodeLength
>GPR = ValidateCode Array16:$CodeOriginal, GPR:$Address, u8:$CodeLength

XXX: Missing op desc!
## ThreadRemoveCodeEntry
>ThreadRemoveCodeEntry

XXX: Missing op desc!
## GPR = ProcessorID
>GPR = ProcessorID

Returns the processor ID correlating to the current running CPU

This may be out of date by time this instruction is executed so care must be taken

This same information can be gotten from syscall getcpu(&cpu, &node)

uint32_t Res = (node << 12) | cpu;

This means it has a limitation of 4096 CPU cores. Which is fine and matches x86 behaviour

## GPR = GetRoundingMode
>GPR = GetRoundingMode

Gets the current rounding mode options

## SetRoundingMode GPR:$RoundMode, i1:$SetDAZ, GPR:$MXCSR
>SetRoundingMode GPR:$RoundMode, i1:$SetDAZ, GPR:$MXCSR

Sets the current rounding mode options for the thread

## GPR = PushRoundingMode u8:$RoundMode
>GPR = PushRoundingMode u8:$RoundMode

Override the current rounding mode options for the thread, returning old FPCR

## PopRoundingMode GPR:$FPCR
>PopRoundingMode GPR:$FPCR

Resets rounding mode after PushRoundingMode operation

## Print SSA:$Value
>Print SSA:$Value

Debug operation that prints an SSA value to the console

May only print 64bits of the value

## PrintMsg c_str:$Value
>PrintMsg c_str:$Value

Debug operation that prints an string to the console.

This is for debug only! Will break code caching!

## GPR = AllocateGPR i1:$ForPair
>GPR = AllocateGPR i1:$ForPair

Silly pseudo-instruction to allocate a register for a future destination

Note: if an instruction uses allocated destinations-as-sources,

it cannot use a regular destination too. This ensures RA correctness.

This is a kludge to deal with the IR's lack of multiple destinations

If ForPair is set, RA will try to allocate the base of a register pair

## FPR = AllocateFPR OpSize:#RegisterSize, OpSize:#ElementSize
>FPR = AllocateFPR OpSize:#RegisterSize, OpSize:#ElementSize

Like AllocateGPR, but for FPR

## GPR = AllocateGPRAfter GPR:$After
>GPR = AllocateGPRAfter GPR:$After

Silly pseudo-instruction to allocate a register for a future destination

This is a kludge to deal with the IR's lack of multiple destinations

RA will attempt to allocate to the register after $After.

It may not succeed.

## GPR = RDRAND i1:$GetReseeded
>GPR = RDRAND i1:$GetReseeded

Uses the hardware random number generator to generate a 64bit number

The boolean argument asks if we should be reading the reseeded number or not

Reseeded RNG calculation is more expensive and will be heavier to use

Returns the 64-bit number

Sets the Z flag if the number is valid.

RNG hardware is allowed to fail early and return. Software must always check this

## Yield
>Yield

This is a hint instruction that the CPU is likely to do a spin so it might want to pause to help out SMP

Can be implemented as a NOP if necessary

## WFET GPR:$Upper, GPR:$Lower
>WFET GPR:$Upper, GPR:$Lower

Implement a low power wait attempting to sleep until RDTSC >= Upper:Lower.

Will spuriously wake up.

## MonoBackpatcherWrite OpSize:$Size, GPR:$Value, GPR:$Addr
>MonoBackpatcherWrite OpSize:$Size, GPR:$Value, GPR:$Addr

Writes and invalidates the target address with the invalidation mutex locked. This is a fault-avoiding

replacement for the atomic SMC writes used in the mono callsite backpatcher.

# Moves

## GPR = Copy GPR:$Source
>GPR = Copy GPR:$Source

GPR copy, generated by RA to split live ranges

# StaticRA

## SSA = LoadRegister u32:$Reg, RegisterClass:$Class, OpSize:#Size
>SSA = LoadRegister u32:$Reg, RegisterClass:$Class, OpSize:#Size

Loads a value from the given register

Size must match the execution mode.

## GPR = LoadPF OpSize:#Size
>GPR = LoadPF OpSize:#Size

Loads raw PF

## GPR = LoadAF OpSize:#Size
>GPR = LoadAF OpSize:#Size

Loads raw PF

## SSA = StoreRegister SSA:$Value, OpSize:#Size
>SSA = StoreRegister SSA:$Value, OpSize:#Size

Stores a value to a given register.

Size must match the execution mode.

## StorePF GPR:$Value, OpSize:#Size
>StorePF GPR:$Value, OpSize:#Size

Stores raw PF

## StoreAF GPR:$Value, OpSize:#Size
>StoreAF GPR:$Value, OpSize:#Size

Stores raw AF

# Vector

## FPR = VMov OpSize:#RegisterSize, FPR:$Source
>FPR = VMov OpSize:#RegisterSize, FPR:$Source

Copy vector register

When Register size is smaller than Source register size,

this op is defined to truncate and zero extend

## FPR = VectorImm OpSize:#RegisterSize, OpSize:#ElementSize, u8:$Immediate, u8:$ShiftAmount{0}
>FPR = VectorImm OpSize:#RegisterSize, OpSize:#ElementSize, u8:$Immediate, u8:$ShiftAmount{0}

Generates a vector with each element containg the immediate zexted

## FPR = LoadNamedVectorConstant OpSize:#RegisterSize, NamedVectorConstant:$Constant
>FPR = LoadNamedVectorConstant OpSize:#RegisterSize, NamedVectorConstant:$Constant

Load a named vector constant.

The list of vector constants can be found in <FEXCore/IR/IR.h>

## FPR = LoadNamedVectorIndexedConstant OpSize:#RegisterSize, IndexNamedVectorConstant:$Constant, u32:$Index
>FPR = LoadNamedVectorIndexedConstant OpSize:#RegisterSize, IndexNamedVectorConstant:$Constant, u32:$Index

Load a named vector constant from Indexable table.

Index needs to be aligned register size.

The list of indexable vector constants can be found in <FEXCore/IR/IR.h>

## FPR = VNeg OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector
>FPR = VNeg OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector

XXX: Missing op desc!
## FPR = VNot OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector
>FPR = VNot OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector

XXX: Missing op desc!
## FPR = VAbs OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector
>FPR = VAbs OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector

Does an signed integer absolute

## FPR = VPopcount OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector
>FPR = VPopcount OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector

Does a popcount for each element of the register

## FPR = VAddV OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector
>FPR = VAddV OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector

Does a horizontal vector add of elements across the source vector

Result is a zero extended scalar

## FPR = VUMinV OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector
>FPR = VUMinV OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector

Does a horizontal vector unsigned minimum of elements across the source vector

Result is a zero extended scalar

## FPR = VUMaxV OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector
>FPR = VUMaxV OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector

Does a horizontal vector unsigned maximum of elements across the source vector

Result is a zero extended scalar

## FPR = VFAbs OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector
>FPR = VFAbs OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector

XXX: Missing op desc!
## FPR = VFNeg OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector
>FPR = VFNeg OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector

XXX: Missing op desc!
## FPR = VFRecp OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector
>FPR = VFRecp OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector

Reciprocal value - matches the precision required by the x86 spec.

It has a relative error of at most 1.5 * 2^-12

## FPR = VFRecpPrecision OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector
>FPR = VFRecpPrecision OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector

Similar to VFRecp but carrying more precision for 3DNow!

It provides at least 14 bits precision, with a relative error of at most 2^-14

## FPR = VFSqrt OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector
>FPR = VFSqrt OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector

XXX: Missing op desc!
## FPR = VFRSqrt OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector
>FPR = VFRSqrt OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector

Reciprocal Square Root - matches the precision required by the x86 spec.

It has a relative error of at most 1.5 * 2^-12

## FPR = VFRSqrtPrecision OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector
>FPR = VFRSqrtPrecision OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector

Similar to VFRSqrt but carrying more precision for 3DNow!

It provides at least 15 bits precision, with a relative error of at most 2^-15

## FPR = VCMPEQZ OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector
>FPR = VCMPEQZ OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector

XXX: Missing op desc!
## FPR = VCMPGTZ OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector
>FPR = VCMPGTZ OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector

Vector compare signed greater than

Each element is compared, if the result is true then the resulting element is ~0, else zero

Compares the vector against zero

## FPR = VCMPLTZ OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector
>FPR = VCMPLTZ OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector

Vector compare signed less than

Each element is compared, if the result is true then the resulting element is ~0, else zero

Compares the vector against zero

## FPR = VDupElement OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector, u8:$Index
>FPR = VDupElement OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector, u8:$Index

Duplicates one element from the source register across the whole register

## FPR = VShlI OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector, u8:$BitShift
>FPR = VShlI OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector, u8:$BitShift

XXX: Missing op desc!
## FPR = VUShrI OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector, u8:$BitShift
>FPR = VUShrI OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector, u8:$BitShift

XXX: Missing op desc!
## FPR = VUShraI OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$DestVector, FPR:$Vector, u8:$BitShift
>FPR = VUShraI OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$DestVector, FPR:$Vector, u8:$BitShift

XXX: Missing op desc!
## FPR = VSShrI OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector, u8:$BitShift
>FPR = VSShrI OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector, u8:$BitShift

XXX: Missing op desc!
## FPR = VUShrNI OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector, u8:$BitShift
>FPR = VUShrNI OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector, u8:$BitShift

Unsigned shifts right each element and then narrows to the next lower element size
## FPR = VUShrNI2 OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$VectorLower, FPR:$VectorUpper, u8:$BitShift
>FPR = VUShrNI2 OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$VectorLower, FPR:$VectorUpper, u8:$BitShift

Unsigned shifts right each element and then narrows to the next lower element size

Inserts results in to the high elements of the first argument

## FPR = VSXTL OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector
>FPR = VSXTL OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector

Sign extends elements from the source element size to the next size up
## FPR = VSXTL2 OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector
>FPR = VSXTL2 OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector

Sign extends elements from the source element size to the next size up

Source elements come from the upper half of the register

## FPR = VSSHLL OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector, u8:$BitShift{0}
>FPR = VSSHLL OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector, u8:$BitShift{0}

Sign extends elements from the source element size to the next size up
## FPR = VSSHLL2 OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector, u8:$BitShift{0}
>FPR = VSSHLL2 OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector, u8:$BitShift{0}

Sign extends elements from the source element size to the next size up

Source elements come from the upper half of the register

## FPR = VUXTL OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector
>FPR = VUXTL OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector

Zero extends elements from the source element size to the next size up
## FPR = VUXTL2 OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector
>FPR = VUXTL2 OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector

Zero extends elements from the source element size to the next size up

Source elements come from the upper half of the register

## FPR = VSQXTN OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector
>FPR = VSQXTN OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector

XXX: Missing op desc!
## FPR = VSQXTN2 OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$VectorLower, FPR:$VectorUpper
>FPR = VSQXTN2 OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$VectorLower, FPR:$VectorUpper

XXX: Missing op desc!
## FPR = VSQXTNPair OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$VectorLower, FPR:$VectorUpper
>FPR = VSQXTNPair OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$VectorLower, FPR:$VectorUpper

Does both VSQXTN and VSQXTN2 in a combined operation.

## FPR = VSQXTUN OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector
>FPR = VSQXTUN OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector

XXX: Missing op desc!
## FPR = VSQXTUN2 OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$VectorLower, FPR:$VectorUpper
>FPR = VSQXTUN2 OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$VectorLower, FPR:$VectorUpper

XXX: Missing op desc!
## FPR = VSQXTUNPair OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$VectorLower, FPR:$VectorUpper
>FPR = VSQXTUNPair OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$VectorLower, FPR:$VectorUpper

Does both VSQXTUN and VSQXTUN2 in a combined operation.

## FPR = VSRSHR OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector, u8:$BitShift
>FPR = VSRSHR OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector, u8:$BitShift

Signed rounding shift right by immediate

Exactly matching Arm64 srshr semantics

## FPR = VSQSHL OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector, u8:$BitShift
>FPR = VSQSHL OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector, u8:$BitShift

Signed satuating shift left by immediate

Exactly matching Arm64 sqshl semantics

## FPR = VRev32 OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector
>FPR = VRev32 OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector

Reverses elements in 32-bit halfwords

Available element size: 1byte, 2 byte

## FPR = VRev64 OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector
>FPR = VRev64 OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector

Reverses elements in 64-bit halfwords

Available element size: 1byte, 2 byte, 4 byte

## FPR = VAdd OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector1, FPR:$Vector2
>FPR = VAdd OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector1, FPR:$Vector2

XXX: Missing op desc!
## FPR = VSub OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector1, FPR:$Vector2
>FPR = VSub OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector1, FPR:$Vector2

XXX: Missing op desc!
## FPR = VAnd OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector1, FPR:$Vector2
>FPR = VAnd OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector1, FPR:$Vector2

XXX: Missing op desc!
## FPR = VAndn OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector1, FPR:$Vector2
>FPR = VAndn OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector1, FPR:$Vector2

XXX: Missing op desc!
## FPR = VOrn OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector1, FPR:$Vector2
>FPR = VOrn OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector1, FPR:$Vector2

XXX: Missing op desc!
## FPR = VOr OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector1, FPR:$Vector2
>FPR = VOr OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector1, FPR:$Vector2

XXX: Missing op desc!
## FPR = VXor OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector1, FPR:$Vector2
>FPR = VXor OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector1, FPR:$Vector2

XXX: Missing op desc!
## FPR = VUQAdd OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector1, FPR:$Vector2
>FPR = VUQAdd OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector1, FPR:$Vector2

XXX: Missing op desc!
## FPR = VUQSub OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector1, FPR:$Vector2
>FPR = VUQSub OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector1, FPR:$Vector2

XXX: Missing op desc!
## FPR = VSQAdd OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector1, FPR:$Vector2
>FPR = VSQAdd OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector1, FPR:$Vector2

XXX: Missing op desc!
## FPR = VSQSub OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector1, FPR:$Vector2
>FPR = VSQSub OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector1, FPR:$Vector2

XXX: Missing op desc!
## FPR = VAddP OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$VectorLower, FPR:$VectorUpper
>FPR = VAddP OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$VectorLower, FPR:$VectorUpper

Does a horizontal pairwise add of elements across the two source vectors
## FPR = VURAvg OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector1, FPR:$Vector2
>FPR = VURAvg OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector1, FPR:$Vector2

Does an unsigned rounded average

dst_elem = (src1_elem + src2_elem + 1) >> 1

## FPR = VUMin OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector1, FPR:$Vector2
>FPR = VUMin OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector1, FPR:$Vector2

XXX: Missing op desc!
## FPR = VUMax OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector1, FPR:$Vector2
>FPR = VUMax OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector1, FPR:$Vector2

XXX: Missing op desc!
## FPR = VSMin OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector1, FPR:$Vector2
>FPR = VSMin OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector1, FPR:$Vector2

XXX: Missing op desc!
## FPR = VSMax OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector1, FPR:$Vector2
>FPR = VSMax OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector1, FPR:$Vector2

XXX: Missing op desc!
## FPR = VZip OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$VectorLower, FPR:$VectorUpper
>FPR = VZip OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$VectorLower, FPR:$VectorUpper

XXX: Missing op desc!
## FPR = VZip2 OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$VectorLower, FPR:$VectorUpper
>FPR = VZip2 OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$VectorLower, FPR:$VectorUpper

XXX: Missing op desc!
## FPR = VUnZip OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$VectorLower, FPR:$VectorUpper
>FPR = VUnZip OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$VectorLower, FPR:$VectorUpper

XXX: Missing op desc!
## FPR = VUnZip2 OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$VectorLower, FPR:$VectorUpper
>FPR = VUnZip2 OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$VectorLower, FPR:$VectorUpper

XXX: Missing op desc!
## FPR = VTrn OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$VectorLower, FPR:$VectorUpper
>FPR = VTrn OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$VectorLower, FPR:$VectorUpper

XXX: Missing op desc!
## FPR = VTrn2 OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$VectorLower, FPR:$VectorUpper
>FPR = VTrn2 OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$VectorLower, FPR:$VectorUpper

XXX: Missing op desc!
## FPR = VFAdd OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector1, FPR:$Vector2
>FPR = VFAdd OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector1, FPR:$Vector2

XXX: Missing op desc!
## FPR = VFAddP OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$VectorLower, FPR:$VectorUpper
>FPR = VFAddP OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$VectorLower, FPR:$VectorUpper

Does a horizontal pairwise add of elements across the two source vectors with float element types
## FPR = VFAddV OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector
>FPR = VFAddV OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector

Does a horizontal float vector add of elements across the source vector

Result is a zero extended scalar

## FPR = VFSub OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector1, FPR:$Vector2
>FPR = VFSub OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector1, FPR:$Vector2

XXX: Missing op desc!
## FPR = VFMul OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector1, FPR:$Vector2
>FPR = VFMul OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector1, FPR:$Vector2

XXX: Missing op desc!
## FPR = VFDiv OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector1, FPR:$Vector2
>FPR = VFDiv OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector1, FPR:$Vector2

XXX: Missing op desc!
## FPR = VFMin OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector1, FPR:$Vector2
>FPR = VFMin OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector1, FPR:$Vector2

XXX: Missing op desc!
## FPR = VFMax OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector1, FPR:$Vector2
>FPR = VFMax OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector1, FPR:$Vector2

XXX: Missing op desc!
## FPR = VMul OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector1, FPR:$Vector2
>FPR = VMul OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector1, FPR:$Vector2

XXX: Missing op desc!
## FPR = VUMull OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector1, FPR:$Vector2
>FPR = VUMull OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector1, FPR:$Vector2

XXX: Missing op desc!
## FPR = VSMull OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector1, FPR:$Vector2
>FPR = VSMull OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector1, FPR:$Vector2

Does a signed integer multiply with extend.

ElementSize is the source size

## FPR = VUMull2 OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector1, FPR:$Vector2
>FPR = VUMull2 OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector1, FPR:$Vector2

Multiplies the high elements with size extension
## FPR = VSMull2 OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector1, FPR:$Vector2
>FPR = VSMull2 OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector1, FPR:$Vector2

Multiplies the high elements with size extension
## FPR = VUMulH OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector1, FPR:$Vector2
>FPR = VUMulH OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector1, FPR:$Vector2

Wide unsigned multiply returning the high results
## FPR = VSMulH OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector1, FPR:$Vector2
>FPR = VSMulH OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector1, FPR:$Vector2

Wide signed multiply returning the high results
## FPR = VUABDL OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector1, FPR:$Vector2
>FPR = VUABDL OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector1, FPR:$Vector2

Unsigned Absolute Difference Long

## FPR = VUABDL2 OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector1, FPR:$Vector2
>FPR = VUABDL2 OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector1, FPR:$Vector2

Unsigned Absolute Difference Long

Using the high elements of the source vectors

## FPR = VUShl OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector, FPR:$ShiftVector, i1:$RangeCheck
>FPR = VUShl OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector, FPR:$ShiftVector, i1:$RangeCheck

XXX: Missing op desc!
## FPR = VUShr OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector, FPR:$ShiftVector, i1:$RangeCheck
>FPR = VUShr OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector, FPR:$ShiftVector, i1:$RangeCheck

XXX: Missing op desc!
## FPR = VSShr OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector, FPR:$ShiftVector, i1:$RangeCheck
>FPR = VSShr OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector, FPR:$ShiftVector, i1:$RangeCheck

XXX: Missing op desc!
## FPR = VUShlS OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector, FPR:$ShiftScalar
>FPR = VUShlS OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector, FPR:$ShiftScalar

XXX: Missing op desc!
## FPR = VUShrS OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector, FPR:$ShiftScalar
>FPR = VUShrS OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector, FPR:$ShiftScalar

XXX: Missing op desc!
## FPR = VSShrS OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector, FPR:$ShiftScalar
>FPR = VSShrS OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector, FPR:$ShiftScalar

XXX: Missing op desc!
## FPR = VUShrSWide OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector, FPR:$ShiftScalar
>FPR = VUShrSWide OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector, FPR:$ShiftScalar

XXX: Missing op desc!
## FPR = VSShrSWide OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector, FPR:$ShiftScalar
>FPR = VSShrSWide OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector, FPR:$ShiftScalar

XXX: Missing op desc!
## FPR = VUShlSWide OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector, FPR:$ShiftScalar
>FPR = VUShlSWide OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector, FPR:$ShiftScalar

XXX: Missing op desc!
## FPR = VInsElement OpSize:#RegisterSize, OpSize:#ElementSize, u8:$DestIdx, u8:$SrcIdx, FPR:$DestVector, FPR:$SrcVector
>FPR = VInsElement OpSize:#RegisterSize, OpSize:#ElementSize, u8:$DestIdx, u8:$SrcIdx, FPR:$DestVector, FPR:$SrcVector

XXX: Missing op desc!
## FPR = VInsGPR OpSize:#RegisterSize, OpSize:#ElementSize, u8:$DestIdx, FPR:$DestVector, GPR:$Src
>FPR = VInsGPR OpSize:#RegisterSize, OpSize:#ElementSize, u8:$DestIdx, FPR:$DestVector, GPR:$Src

XXX: Missing op desc!
## FPR = VExtr OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$VectorLower, FPR:$VectorUpper, u8:$Index
>FPR = VExtr OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$VectorLower, FPR:$VectorUpper, u8:$Index

Concats two vector registers together and extracts a full width register from the element index

Index is an element index. So it is offset by ElementSize argument

op:

TmpVector <RegisterSize *2> = concat(Upper:Lower)

Dest = TmpVector >> (ElementSize * Index * 8); // Or can be thought of `concat(&TmpVector[Index], i128)`

## FPR = VCMPEQ OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector1, FPR:$Vector2
>FPR = VCMPEQ OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector1, FPR:$Vector2

XXX: Missing op desc!
## FPR = VCMPGT OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector1, FPR:$Vector2
>FPR = VCMPGT OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector1, FPR:$Vector2

Vector compare signed greater than

Each element is compared, if the result is true then the resulting element is ~0, else zero

## FPR = VFCMPEQ OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector1, FPR:$Vector2
>FPR = VFCMPEQ OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector1, FPR:$Vector2

XXX: Missing op desc!
## FPR = VFCMPNEQ OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector1, FPR:$Vector2
>FPR = VFCMPNEQ OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector1, FPR:$Vector2

XXX: Missing op desc!
## FPR = VFCMPLT OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector1, FPR:$Vector2
>FPR = VFCMPLT OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector1, FPR:$Vector2

XXX: Missing op desc!
## FPR = VFCMPGT OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector1, FPR:$Vector2
>FPR = VFCMPGT OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector1, FPR:$Vector2

XXX: Missing op desc!
## FPR = VFCMPLE OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector1, FPR:$Vector2
>FPR = VFCMPLE OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector1, FPR:$Vector2

XXX: Missing op desc!
## FPR = VFCMPORD OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector1, FPR:$Vector2
>FPR = VFCMPORD OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector1, FPR:$Vector2

XXX: Missing op desc!
## FPR = VFCMPUNO OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector1, FPR:$Vector2
>FPR = VFCMPUNO OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector1, FPR:$Vector2

XXX: Missing op desc!
## FPR = VTBL1 OpSize:#RegisterSize, FPR:$VectorTable, FPR:$VectorIndices
>FPR = VTBL1 OpSize:#RegisterSize, FPR:$VectorTable, FPR:$VectorIndices

Does a vector table lookup from one register in to the destination

Lookup is byte sized per byte element.

Any index larger than what the registers provide will result in zero for that element

Table is always treated as a 128bit register

Indices matches destination size. Either 64bit or 128bit

## FPR = VTBL2 OpSize:#RegisterSize, FPR:$VectorTable1, FPR:$VectorTable2, FPR:$VectorIndices
>FPR = VTBL2 OpSize:#RegisterSize, FPR:$VectorTable1, FPR:$VectorTable2, FPR:$VectorIndices

Does a vector table lookup from two registers in to the destination

Lookup is byte sized per byte element.

Any index larger than what the registers provide will result in zero for that element

Table is always treated as a two 128bit registers

Indices matches destination size. Either 64bit or 128bit

Careful about not using sequential table registers, will result in some moves if they aren't sequential.

## FPR = VTBX1 OpSize:#RegisterSize, FPR:$VectorSrcDst, FPR:$VectorTable, FPR:$VectorIndices
>FPR = VTBX1 OpSize:#RegisterSize, FPR:$VectorSrcDst, FPR:$VectorTable, FPR:$VectorIndices

Does a vector table lookup from one register in to the destination

Lookup is byte sized per byte element.

Any index larger than what the registers provide will result in not modifying that element

Table is always treated as a 128bit register

Indices matches destination size. Either 64bit or 128bit

## FPR = VBSL OpSize:#RegisterSize, FPR:$VectorMask, FPR:$VectorTrue, FPR:$VectorFalse
>FPR = VBSL OpSize:#RegisterSize, FPR:$VectorMask, FPR:$VectorTrue, FPR:$VectorFalse

Does a vector bitwise select.

If the bit in the field is 1 then the corresponding bit is pulled from VectorTrue

If the bit in the field is 0 then the corresponding bit is pulled from VectorFalse

## GPR = VPCMPESTRX FPR:$LHS, FPR:$RHS, GPR:$RAX, GPR:$RDX, u16:$Control
>GPR = VPCMPESTRX FPR:$LHS, FPR:$RHS, GPR:$RAX, GPR:$RDX, u16:$Control

Performs intermediate behavior analogous to the x86 PCMPESTRI/PCMPESTRM instruction

This will return the intermediate result of a PCMPESTR-type operation, but NOT the final

result. This must be derived from the intermediate result

NOTE: On top of returning the intermediate result, the returned value also combines the status

flags into the upper 16-bits of the 32-bit result, as these can also be derived over the

course of creating the intermediate result

## GPR = VPCMPISTRX FPR:$LHS, FPR:$RHS, u8:$Control
>GPR = VPCMPISTRX FPR:$LHS, FPR:$RHS, u8:$Control

Performs intermediate behavior analogous to the x86 PCMPISTRI/PCMPISTRM instruction

This will return the intermediate result of a PCMPISTR-type operation, but NOT the final

result. This must be derived from the intermediate result

NOTE: On top of returning the intermediate result, the returned value also combines the status

flags into the upper 16-bits of the 32-bit result, as these can also be derived over the

course of creating the intermediate result

## FPR = VFCADD OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector1, FPR:$Vector2, u16:$Rotate
>FPR = VFCADD OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector1, FPR:$Vector2, u16:$Rotate

XXX: Missing op desc!
## FPR = VFMLA OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector1, FPR:$Vector2, FPR:$Addend
>FPR = VFMLA OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector1, FPR:$Vector2, FPR:$Addend

Dest = (Vector1 * Vector2) + Addend

This explicitly matches x86 FMA semantics because ARM semantics are mind-bending.

## FPR = VFMLS OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector1, FPR:$Vector2, FPR:$Addend
>FPR = VFMLS OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector1, FPR:$Vector2, FPR:$Addend

Dest = (Vector1 * Vector2) - Addend

This explicitly matches x86 FMA semantics because ARM semantics are mind-bending.

## FPR = VFNMLA OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector1, FPR:$Vector2, FPR:$Addend
>FPR = VFNMLA OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector1, FPR:$Vector2, FPR:$Addend

Dest = (-Vector1 * Vector2) + Addend

This explicitly matches x86 FMA semantics because ARM semantics are mind-bending.

## FPR = VFNMLS OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector1, FPR:$Vector2, FPR:$Addend
>FPR = VFNMLS OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector1, FPR:$Vector2, FPR:$Addend

Dest = (-Vector1 * Vector2) - Addend

This explicitly matches x86 FMA semantics because ARM semantics are mind-bending.

# VectorScalar

## FPR = VFAddScalarInsert OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector1, FPR:$Vector2, i1:$ZeroUpperBits
>FPR = VFAddScalarInsert OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector1, FPR:$Vector2, i1:$ZeroUpperBits

Does a scalar 'add' between Vector1 and Vector2.

Inserting the result in to the lower element of Vector1 and returning the results.

If ZeroUpperBits is set then in a 256-bit wide operation it will zero the upper 128-bits of the destination.

For 128-bit operation this matches SSE insert semantics.

For 256-bit operation with ZeroUpperBits, this matches AVX insert semantics.

## FPR = VFSubScalarInsert OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector1, FPR:$Vector2, i1:$ZeroUpperBits
>FPR = VFSubScalarInsert OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector1, FPR:$Vector2, i1:$ZeroUpperBits

Does a scalar 'sub' between Vector1 and Vector2.

Inserting the result in to the lower element of Vector1 and returning the results.

If ZeroUpperBits is set then in a 256-bit wide operation it will zero the upper 128-bits of the destination.

For 128-bit operation this matches SSE insert semantics.

For 256-bit operation with ZeroUpperBits, this matches AVX insert semantics.

## FPR = VFMulScalarInsert OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector1, FPR:$Vector2, i1:$ZeroUpperBits
>FPR = VFMulScalarInsert OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector1, FPR:$Vector2, i1:$ZeroUpperBits

Does a scalar 'mul' between Vector1 and Vector2.

Inserting the result in to the lower element of Vector1 and returning the results.

If ZeroUpperBits is set then in a 256-bit wide operation it will zero the upper 128-bits of the destination.

For 128-bit operation this matches SSE insert semantics.

For 256-bit operation with ZeroUpperBits, this matches AVX insert semantics.

## FPR = VFDivScalarInsert OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector1, FPR:$Vector2, i1:$ZeroUpperBits
>FPR = VFDivScalarInsert OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector1, FPR:$Vector2, i1:$ZeroUpperBits

Does a scalar 'div' between Vector1 and Vector2.

Inserting the result in to the lower element of Vector1 and returning the results.

If ZeroUpperBits is set then in a 256-bit wide operation it will zero the upper 128-bits of the destination.

For 128-bit operation this matches SSE insert semantics.

For 256-bit operation with ZeroUpperBits, this matches AVX insert semantics.

## FPR = VFMinScalarInsert OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector1, FPR:$Vector2, i1:$ZeroUpperBits
>FPR = VFMinScalarInsert OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector1, FPR:$Vector2, i1:$ZeroUpperBits

Does a scalar 'min' between Vector1 and Vector2.

Inserting the result in to the lower element of Vector1 and returning the results.

If ZeroUpperBits is set then in a 256-bit wide operation it will zero the upper 128-bits of the destination.

For 128-bit operation this matches SSE insert semantics.

For 256-bit operation with ZeroUpperBits, this matches AVX insert semantics.

Additionally matches x86 zero and NaN semantics

If both source operands are zero, return the second operand (in the case of negative and positive zero)

If either source operand is NaN then return the second operand.

## FPR = VFMaxScalarInsert OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector1, FPR:$Vector2, i1:$ZeroUpperBits
>FPR = VFMaxScalarInsert OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector1, FPR:$Vector2, i1:$ZeroUpperBits

Does a scalar 'max' between Vector1 and Vector2.

Inserting the result in to the lower element of Vector1 and returning the results.

If ZeroUpperBits is set then in a 256-bit wide operation it will zero the upper 128-bits of the destination.

For 128-bit operation this matches SSE insert semantics.

For 256-bit operation with ZeroUpperBits, this matches AVX insert semantics.

Additionally matches x86 zero and NaN semantics

If both source operands are zero, return the second operand (in the case of negative and positive zero)

If either source operand is NaN then return the second operand.

## FPR = VFSqrtScalarInsert OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector1, FPR:$Vector2, i1:$ZeroUpperBits
>FPR = VFSqrtScalarInsert OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector1, FPR:$Vector2, i1:$ZeroUpperBits

Does a scalar 'sqrt' on Vector2, inserting in to Vector1 and storing in to the destination.

Inserting the result in to the lower element of Vector1 and returning the results.

If ZeroUpperBits is set then in a 256-bit wide operation it will zero the upper 128-bits of the destination.

For 128-bit operation this matches SSE insert semantics.

For 256-bit operation with ZeroUpperBits, this matches AVX insert semantics.

## FPR = VFRSqrtScalarInsert OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector1, FPR:$Vector2, i1:$ZeroUpperBits
>FPR = VFRSqrtScalarInsert OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector1, FPR:$Vector2, i1:$ZeroUpperBits

Does a scalar 'rsqrt' on Vector2, inserting in to Vector1 and storing in to the destination.

Inserting the result in to the lower element of Vector1 and returning the results.

If ZeroUpperBits is set then in a 256-bit wide operation it will zero the upper 128-bits of the destination.

For 128-bit operation this matches SSE insert semantics.

For 256-bit operation with ZeroUpperBits, this matches AVX insert semantics.

## FPR = VFRecpScalarInsert OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector1, FPR:$Vector2, i1:$ZeroUpperBits
>FPR = VFRecpScalarInsert OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector1, FPR:$Vector2, i1:$ZeroUpperBits

Does a scalar 'recip' on Vector2, inserting in to Vector1 and storing in to the destination.

Inserting the result in to the lower element of Vector1 and returning the results.

If ZeroUpperBits is set then in a 256-bit wide operation it will zero the upper 128-bits of the destination.

For 128-bit operation this matches SSE insert semantics.

For 256-bit operation with ZeroUpperBits, this matches AVX insert semantics.

## FPR = VFToFScalarInsert OpSize:#RegisterSize, OpSize:#DstElementSize, OpSize:$SrcElementSize, FPR:$Vector1, FPR:$Vector2, i1:$ZeroUpperBits
>FPR = VFToFScalarInsert OpSize:#RegisterSize, OpSize:#DstElementSize, OpSize:$SrcElementSize, FPR:$Vector1, FPR:$Vector2, i1:$ZeroUpperBits

Does a scalar 'cvt' between Vector1 and Vector2.

Inserting the result in to the lower element of Vector1 and returning the results.

If ZeroUpperBits is set then in a 256-bit wide operation it will zero the upper 128-bits of the destination.

For 128-bit operation this matches SSE insert semantics.

For 256-bit operation with ZeroUpperBits, this matches AVX insert semantics.

## FPR = VSToFVectorInsert OpSize:#RegisterSize, OpSize:#DstElementSize, OpSize:$SrcElementSize, FPR:$Vector1, FPR:$Vector2, i8:$HasTwoElements, i1:$ZeroUpperBits
>FPR = VSToFVectorInsert OpSize:#RegisterSize, OpSize:#DstElementSize, OpSize:$SrcElementSize, FPR:$Vector1, FPR:$Vector2, i8:$HasTwoElements, i1:$ZeroUpperBits

Does a Vector 'scvt' between Vector1 and Vector2.

Inserting the result in to the lower element of Vector1 and returning the results.

If ZeroUpperBits is set then in a 256-bit wide operation it will zero the upper 128-bits of the destination.

For 128-bit operation this matches SSE insert semantics.

For 256-bit operation with ZeroUpperBits, this matches AVX insert semantics.

HasTwoElements is slightly different than most of these scalar operations.

Handles the edge case of cvtpi2ps xmm0, mm0 which is two elements in the lower 64-bits

## FPR = VSToFGPRInsert OpSize:#RegisterSize, OpSize:#DstElementSize, OpSize:$SrcElementSize, FPR:$Vector, GPR:$Src, i1:$ZeroUpperBits
>FPR = VSToFGPRInsert OpSize:#RegisterSize, OpSize:#DstElementSize, OpSize:$SrcElementSize, FPR:$Vector, GPR:$Src, i1:$ZeroUpperBits

Does a scalar 'cvt' between Vector1 and GPR.

Inserting the result in to the lower element of Vector1 and returning the results.

If ZeroUpperBits is set then in a 256-bit wide operation it will zero the upper 128-bits of the destination.

For 128-bit operation this matches SSE insert semantics.

For 256-bit operation with ZeroUpperBits, this matches AVX insert semantics.

## FPR = VFToIScalarInsert OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector1, FPR:$Vector2, RoundType:$Round, i1:$ZeroUpperBits
>FPR = VFToIScalarInsert OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector1, FPR:$Vector2, RoundType:$Round, i1:$ZeroUpperBits

Does a scalar round float to integral on Vector2, inserting in to Vector1 and storing in to the destination.

Rounding mode determined by argument

Inserting the result in to the lower element of Vector1 and returning the results.

If ZeroUpperBits is set then in a 256-bit wide operation it will zero the upper 128-bits of the destination.

For 128-bit operation this matches SSE insert semantics.

For 256-bit operation with ZeroUpperBits, this matches AVX insert semantics.

## FPR = VFCMPScalarInsert OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector1, FPR:$Vector2, FloatCompareOp:$Op, i1:$ZeroUpperBits
>FPR = VFCMPScalarInsert OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector1, FPR:$Vector2, FloatCompareOp:$Op, i1:$ZeroUpperBits

Does a scalar 'cmp' between Vector1 and Vecto2, inserting in to Vector1 and storing in to the destination.

Compare op determined by argument

Inserting the result in to the lower element of Vector1 and returning the results.

If ZeroUpperBits is set then in a 256-bit wide operation it will zero the upper 128-bits of the destination.

For 128-bit operation this matches SSE insert semantics.

For 256-bit operation with ZeroUpperBits, this matches AVX insert semantics.

## FPR = VFMLAScalarInsert OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Upper, FPR:$Vector1, FPR:$Vector2, FPR:$Addend
>FPR = VFMLAScalarInsert OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Upper, FPR:$Vector1, FPR:$Vector2, FPR:$Addend

Dest = (Vector1 * Vector2) + Addend

This explicitly matches x86 FMA semantics because ARM semantics are mind-bending.

Upper elements copied from Upper

## FPR = VFMLSScalarInsert OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Upper, FPR:$Vector1, FPR:$Vector2, FPR:$Addend
>FPR = VFMLSScalarInsert OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Upper, FPR:$Vector1, FPR:$Vector2, FPR:$Addend

Dest = (Vector1 * Vector2) - Addend

This explicitly matches x86 FMA semantics because ARM semantics are mind-bending.

Upper elements copied from Upper

## FPR = VFNMLAScalarInsert OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Upper, FPR:$Vector1, FPR:$Vector2, FPR:$Addend
>FPR = VFNMLAScalarInsert OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Upper, FPR:$Vector1, FPR:$Vector2, FPR:$Addend

Dest = (-Vector1 * Vector2) + Addend

This explicitly matches x86 FMA semantics because ARM semantics are mind-bending.

Upper elements copied from Upper

## FPR = VFNMLSScalarInsert OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Upper, FPR:$Vector1, FPR:$Vector2, FPR:$Addend
>FPR = VFNMLSScalarInsert OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Upper, FPR:$Vector1, FPR:$Vector2, FPR:$Addend

Dest = (-Vector1 * Vector2) - Addend

This explicitly matches x86 FMA semantics because ARM semantics are mind-bending.

Upper elements copied from Upper

## FPR = VFCopySign OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector1,  FPR:$Vector2
>FPR = VFCopySign OpSize:#RegisterSize, OpSize:#ElementSize, FPR:$Vector1,  FPR:$Vector2

Returns a vector where each element has has the magniture of each corresponding element in vector1 and the sign of vector 2.

## Defines
```cpp
constexpr uint8_t NumClasses {6}
constexpr uint8_t FCMP_FLAG_EQ        = 0
constexpr uint8_t FCMP_FLAG_LT        = 1
constexpr uint8_t FCMP_FLAG_UNORDERED = 2
struct BreakDefinition {
  uint16_t ErrorRegister;
  uint8_t Signal;
  uint8_t TrapNumber;
  uint8_t si_code;
};
```
