//
// Created by natty on 24.06.2026.
//

#ifndef BYTECODE_H
#define BYTECODE_H

#include <map>
#include <string>
#include <unordered_map>


enum class Bytecodes {
	illegal = -1,

	// Java Bytecodes
	_nop = 0, // 0x00
	_aconst_null = 1, // 0x01
	_iconst_m1 = 2, // 0x02
	_iconst_0 = 3, // 0x03
	_iconst_1 = 4, // 0x04
	_iconst_2 = 5, // 0x05
	_iconst_3 = 6, // 0x06
	_iconst_4 = 7, // 0x07
	_iconst_5 = 8, // 0x08
	_lconst_0 = 9, // 0x09
	_lconst_1 = 10, // 0x0a
	_fconst_0 = 11, // 0x0b
	_fconst_1 = 12, // 0x0c
	_fconst_2 = 13, // 0x0d
	_dconst_0 = 14, // 0x0e
	_dconst_1 = 15, // 0x0f
	_bipush = 16, // 0x10
	_sipush = 17, // 0x11
	_ldc = 18, // 0x12
	_ldc_w = 19, // 0x13
	_ldc2_w = 20, // 0x14
	_iload = 21, // 0x15
	_lload = 22, // 0x16
	_fload = 23, // 0x17
	_dload = 24, // 0x18
	_aload = 25, // 0x19
	_iload_0 = 26, // 0x1a
	_iload_1 = 27, // 0x1b
	_iload_2 = 28, // 0x1c
	_iload_3 = 29, // 0x1d
	_lload_0 = 30, // 0x1e
	_lload_1 = 31, // 0x1f
	_lload_2 = 32, // 0x20
	_lload_3 = 33, // 0x21
	_fload_0 = 34, // 0x22
	_fload_1 = 35, // 0x23
	_fload_2 = 36, // 0x24
	_fload_3 = 37, // 0x25
	_dload_0 = 38, // 0x26
	_dload_1 = 39, // 0x27
	_dload_2 = 40, // 0x28
	_dload_3 = 41, // 0x29
	_aload_0 = 42, // 0x2a
	_aload_1 = 43, // 0x2b
	_aload_2 = 44, // 0x2c
	_aload_3 = 45, // 0x2d
	_iaload = 46, // 0x2e
	_laload = 47, // 0x2f
	_faload = 48, // 0x30
	_daload = 49, // 0x31
	_aaload = 50, // 0x32
	_baload = 51, // 0x33
	_caload = 52, // 0x34
	_saload = 53, // 0x35
	_istore = 54, // 0x36
	_lstore = 55, // 0x37
	_fstore = 56, // 0x38
	_dstore = 57, // 0x39
	_astore = 58, // 0x3a
	_istore_0 = 59, // 0x3b
	_istore_1 = 60, // 0x3c
	_istore_2 = 61, // 0x3d
	_istore_3 = 62, // 0x3e
	_lstore_0 = 63, // 0x3f
	_lstore_1 = 64, // 0x40
	_lstore_2 = 65, // 0x41
	_lstore_3 = 66, // 0x42
	_fstore_0 = 67, // 0x43
	_fstore_1 = 68, // 0x44
	_fstore_2 = 69, // 0x45
	_fstore_3 = 70, // 0x46
	_dstore_0 = 71, // 0x47
	_dstore_1 = 72, // 0x48
	_dstore_2 = 73, // 0x49
	_dstore_3 = 74, // 0x4a
	_astore_0 = 75, // 0x4b
	_astore_1 = 76, // 0x4c
	_astore_2 = 77, // 0x4d
	_astore_3 = 78, // 0x4e
	_iastore = 79, // 0x4f
	_lastore = 80, // 0x50
	_fastore = 81, // 0x51
	_dastore = 82, // 0x52
	_aastore = 83, // 0x53
	_bastore = 84, // 0x54
	_castore = 85, // 0x55
	_sastore = 86, // 0x56
	_pop = 87, // 0x57
	_pop2 = 88, // 0x58
	_dup = 89, // 0x59
	_dup_x1 = 90, // 0x5a
	_dup_x2 = 91, // 0x5b
	_dup2 = 92, // 0x5c
	_dup2_x1 = 93, // 0x5d
	_dup2_x2 = 94, // 0x5e
	_swap = 95, // 0x5f
	_iadd = 96, // 0x60
	_ladd = 97, // 0x61
	_fadd = 98, // 0x62
	_dadd = 99, // 0x63
	_isub = 100, // 0x64
	_lsub = 101, // 0x65
	_fsub = 102, // 0x66
	_dsub = 103, // 0x67
	_imul = 104, // 0x68
	_lmul = 105, // 0x69
	_fmul = 106, // 0x6a
	_dmul = 107, // 0x6b
	_idiv = 108, // 0x6c
	_ldiv = 109, // 0x6d
	_fdiv = 110, // 0x6e
	_ddiv = 111, // 0x6f
	_irem = 112, // 0x70
	_lrem = 113, // 0x71
	_frem = 114, // 0x72
	_drem = 115, // 0x73
	_ineg = 116, // 0x74
	_lneg = 117, // 0x75
	_fneg = 118, // 0x76
	_dneg = 119, // 0x77
	_ishl = 120, // 0x78
	_lshl = 121, // 0x79
	_ishr = 122, // 0x7a
	_lshr = 123, // 0x7b
	_iushr = 124, // 0x7c
	_lushr = 125, // 0x7d
	_iand = 126, // 0x7e
	_land = 127, // 0x7f
	_ior = 128, // 0x80
	_lor = 129, // 0x81
	_ixor = 130, // 0x82
	_lxor = 131, // 0x83
	_iinc = 132, // 0x84
	_i2l = 133, // 0x85
	_i2f = 134, // 0x86
	_i2d = 135, // 0x87
	_l2i = 136, // 0x88
	_l2f = 137, // 0x89
	_l2d = 138, // 0x8a
	_f2i = 139, // 0x8b
	_f2l = 140, // 0x8c
	_f2d = 141, // 0x8d
	_d2i = 142, // 0x8e
	_d2l = 143, // 0x8f
	_d2f = 144, // 0x90
	_i2b = 145, // 0x91
	_i2c = 146, // 0x92
	_i2s = 147, // 0x93
	_lcmp = 148, // 0x94
	_fcmpl = 149, // 0x95
	_fcmpg = 150, // 0x96
	_dcmpl = 151, // 0x97
	_dcmpg = 152, // 0x98
	_ifeq = 153, // 0x99
	_ifne = 154, // 0x9a
	_iflt = 155, // 0x9b
	_ifge = 156, // 0x9c
	_ifgt = 157, // 0x9d
	_ifle = 158, // 0x9e
	_if_icmpeq = 159, // 0x9f
	_if_icmpne = 160, // 0xa0
	_if_icmplt = 161, // 0xa1
	_if_icmpge = 162, // 0xa2
	_if_icmpgt = 163, // 0xa3
	_if_icmple = 164, // 0xa4
	_if_acmpeq = 165, // 0xa5
	_if_acmpne = 166, // 0xa6
	_goto = 167, // 0xa7
	_jsr = 168, // 0xa8
	_ret = 169, // 0xa9
	_tableswitch = 170, // 0xaa
	_lookupswitch = 171, // 0xab
	_ireturn = 172, // 0xac
	_lreturn = 173, // 0xad
	_freturn = 174, // 0xae
	_dreturn = 175, // 0xaf
	_areturn = 176, // 0xb0
	_return = 177, // 0xb1
	_getstatic = 178, // 0xb2
	_putstatic = 179, // 0xb3
	_getfield = 180, // 0xb4
	_putfield = 181, // 0xb5
	_invokevirtual = 182, // 0xb6
	_invokespecial = 183, // 0xb7
	_invokestatic = 184, // 0xb8
	_invokeinterface = 185, // 0xb9
	_invokedynamic = 186, // 0xba     // if EnableInvokeDynamic
	_new = 187, // 0xbb
	_newarray = 188, // 0xbc
	_anewarray = 189, // 0xbd
	_arraylength = 190, // 0xbe
	_athrow = 191, // 0xbf
	_checkcast = 192, // 0xc0
	_instanceof = 193, // 0xc1
	_monitorenter = 194, // 0xc2
	_monitorexit = 195, // 0xc3
	_wide = 196, // 0xc4
	_multianewarray = 197, // 0xc5
	_ifnull = 198, // 0xc6
	_ifnonnull = 199, // 0xc7
	_goto_w = 200, // 0xc8
	_jsr_w = 201, // 0xc9
	_breakpoint = 202, // 0xca

	number_of_java_codes,

	// The length of the following Bytecodes does not exist in the table
	// JVM Bytecodes
	_fast_agetfield = number_of_java_codes,
	_fast_bgetfield,
	_fast_cgetfield,
	_fast_dgetfield,
	_fast_fgetfield,
	_fast_igetfield,
	_fast_lgetfield,
	_fast_sgetfield,

	_fast_aputfield,
	_fast_bputfield,
	_fast_zputfield,
	_fast_cputfield,
	_fast_dputfield,
	_fast_fputfield,
	_fast_iputfield,
	_fast_lputfield,
	_fast_sputfield,

	_fast_aload_0,
	_fast_iaccess_0,
	_fast_aaccess_0,
	_fast_faccess_0,

	_fast_iload,
	_fast_iload2,
	_fast_icaload,

	_fast_invokevfinal,
	_fast_linearswitch,
	_fast_binaryswitch,

	// special handling of oop constants:
	_fast_aldc,
	_fast_aldc_w,
	_return_register_finalizer,
	// special handling of signature-polymorphic methods:
	_invokehandle,
	_shouldnotreachhere, // For debugging
	number_of_codes
};

inline std::unordered_map<Bytecodes, std::string> bytecode_names = {
	{Bytecodes::illegal, "illegal"},
	{Bytecodes::_nop, "nop"},
	{Bytecodes::_aconst_null, "aconst_null"},
	{Bytecodes::_iconst_m1, "iconst_m1"},
	{Bytecodes::_iconst_0, "iconst_0"},
	{Bytecodes::_iconst_1, "iconst_1"},
	{Bytecodes::_iconst_2, "iconst_2"},
	{Bytecodes::_iconst_3, "iconst_3"},
	{Bytecodes::_iconst_4, "iconst_4"},
	{Bytecodes::_iconst_5, "iconst_5"},
	{Bytecodes::_lconst_0, "lconst_0"},
	{Bytecodes::_lconst_1, "lconst_1"},
	{Bytecodes::_fconst_0, "fconst_0"},
	{Bytecodes::_fconst_1, "fconst_1"},
	{Bytecodes::_fconst_2, "fconst_2"},
	{Bytecodes::_dconst_0, "dconst_0"},
	{Bytecodes::_dconst_1, "dconst_1"},
	{Bytecodes::_bipush, "bipush"},
	{Bytecodes::_sipush, "sipush"},
	{Bytecodes::_ldc, "ldc"},
	{Bytecodes::_ldc_w, "ldc_w"},
	{Bytecodes::_ldc2_w, "ldc2_w"},
	{Bytecodes::_iload, "iload"},
	{Bytecodes::_lload, "lload"},
	{Bytecodes::_fload, "fload"},
	{Bytecodes::_dload, "dload"},
	{Bytecodes::_aload, "aload"},
	{Bytecodes::_iload_0, "iload_0"},
	{Bytecodes::_iload_1, "iload_1"},
	{Bytecodes::_iload_2, "iload_2"},
	{Bytecodes::_iload_3, "iload_3"},
	{Bytecodes::_lload_0, "lload_0"},
	{Bytecodes::_lload_1, "lload_1"},
	{Bytecodes::_lload_2, "lload_2"},
	{Bytecodes::_lload_3, "lload_3"},
	{Bytecodes::_fload_0, "fload_0"},
	{Bytecodes::_fload_1, "fload_1"},
	{Bytecodes::_fload_2, "fload_2"},
	{Bytecodes::_fload_3, "fload_3"},
	{Bytecodes::_dload_0, "dload_0"},
	{Bytecodes::_dload_1, "dload_1"},
	{Bytecodes::_dload_2, "dload_2"},
	{Bytecodes::_dload_3, "dload_3"},
	{Bytecodes::_aload_0, "aload_0"},
	{Bytecodes::_aload_1, "aload_1"},
	{Bytecodes::_aload_2, "aload_2"},
	{Bytecodes::_aload_3, "aload_3"},
	{Bytecodes::_iaload, "iaload"},
	{Bytecodes::_laload, "laload"},
	{Bytecodes::_faload, "faload"},
	{Bytecodes::_daload, "daload"},
	{Bytecodes::_aaload, "aaload"},
	{Bytecodes::_baload, "baload"},
	{Bytecodes::_caload, "caload"},
	{Bytecodes::_saload, "saload"},
	{Bytecodes::_istore, "istore"},
	{Bytecodes::_lstore, "lstore"},
	{Bytecodes::_fstore, "fstore"},
	{Bytecodes::_dstore, "dstore"},
	{Bytecodes::_astore, "astore"},
	{Bytecodes::_istore_0, "istore_0"},
	{Bytecodes::_istore_1, "istore_1"},
	{Bytecodes::_istore_2, "istore_2"},
	{Bytecodes::_istore_3, "istore_3"},
	{Bytecodes::_lstore_0, "lstore_0"},
	{Bytecodes::_lstore_1, "lstore_1"},
	{Bytecodes::_lstore_2, "lstore_2"},
	{Bytecodes::_lstore_3, "lstore_3"},
	{Bytecodes::_fstore_0, "fstore_0"},
	{Bytecodes::_fstore_1, "fstore_1"},
	{Bytecodes::_fstore_2, "fstore_2"},
	{Bytecodes::_fstore_3, "fstore_3"},
	{Bytecodes::_dstore_0, "dstore_0"},
	{Bytecodes::_dstore_1, "dstore_1"},
	{Bytecodes::_dstore_2, "dstore_2"},
	{Bytecodes::_dstore_3, "dstore_3"},
	{Bytecodes::_astore_0, "astore_0"},
	{Bytecodes::_astore_1, "astore_1"},
	{Bytecodes::_astore_2, "astore_2"},
	{Bytecodes::_astore_3, "astore_3"},
	{Bytecodes::_iastore, "iastore"},
	{Bytecodes::_lastore, "lastore"},
	{Bytecodes::_fastore, "fastore"},
	{Bytecodes::_dastore, "dastore"},
	{Bytecodes::_aastore, "aastore"},
	{Bytecodes::_bastore, "bastore"},
	{Bytecodes::_castore, "castore"},
	{Bytecodes::_sastore, "sastore"},
	{Bytecodes::_pop, "pop"},
	{Bytecodes::_pop2, "pop2"},
	{Bytecodes::_dup, "dup"},
	{Bytecodes::_dup_x1, "dup_x1"},
	{Bytecodes::_dup_x2, "dup_x2"},
	{Bytecodes::_dup2, "dup2"},
	{Bytecodes::_dup2_x1, "dup2_x1"},
	{Bytecodes::_dup2_x2, "dup2_x2"},
	{Bytecodes::_swap, "swap"},
	{Bytecodes::_iadd, "iadd"},
	{Bytecodes::_ladd, "ladd"},
	{Bytecodes::_fadd, "fadd"},
	{Bytecodes::_dadd, "dadd"},
	{Bytecodes::_isub, "isub"},
	{Bytecodes::_lsub, "lsub"},
	{Bytecodes::_fsub, "fsub"},
	{Bytecodes::_dsub, "dsub"},
	{Bytecodes::_imul, "imul"},
	{Bytecodes::_lmul, "lmul"},
	{Bytecodes::_fmul, "fmul"},
	{Bytecodes::_dmul, "dmul"},
	{Bytecodes::_idiv, "idiv"},
	{Bytecodes::_ldiv, "ldiv"},
	{Bytecodes::_fdiv, "fdiv"},
	{Bytecodes::_ddiv, "ddiv"},
	{Bytecodes::_irem, "irem"},
	{Bytecodes::_lrem, "lrem"},
	{Bytecodes::_frem, "frem"},
	{Bytecodes::_drem, "drem"},
	{Bytecodes::_ineg, "ineg"},
	{Bytecodes::_lneg, "lneg"},
	{Bytecodes::_fneg, "fneg"},
	{Bytecodes::_dneg, "dneg"},
	{Bytecodes::_ishl, "ishl"},
	{Bytecodes::_lshl, "lshl"},
	{Bytecodes::_ishr, "ishr"},
	{Bytecodes::_lshr, "lshr"},
	{Bytecodes::_iushr, "iushr"},
	{Bytecodes::_lushr, "lushr"},
	{Bytecodes::_iand, "iand"},
	{Bytecodes::_land, "land"},
	{Bytecodes::_ior, "ior"},
	{Bytecodes::_lor, "lor"},
	{Bytecodes::_ixor, "ixor"},
	{Bytecodes::_lxor, "lxor"},
	{Bytecodes::_iinc, "iinc"},
	{Bytecodes::_i2l, "i2l"},
	{Bytecodes::_i2f, "i2f"},
	{Bytecodes::_i2d, "i2d"},
	{Bytecodes::_l2i, "l2i"},
	{Bytecodes::_l2f, "l2f"},
	{Bytecodes::_l2d, "l2d"},
	{Bytecodes::_f2i, "f2i"},
	{Bytecodes::_f2l, "f2l"},
	{Bytecodes::_f2d, "f2d"},
	{Bytecodes::_d2i, "d2i"},
	{Bytecodes::_d2l, "d2l"},
	{Bytecodes::_d2f, "d2f"},
	{Bytecodes::_i2b, "i2b"},
	{Bytecodes::_i2c, "i2c"},
	{Bytecodes::_i2s, "i2s"},
	{Bytecodes::_lcmp, "lcmp"},
	{Bytecodes::_fcmpl, "fcmpl"},
	{Bytecodes::_fcmpg, "fcmpg"},
	{Bytecodes::_dcmpl, "dcmpl"},
	{Bytecodes::_dcmpg, "dcmpg"},
	{Bytecodes::_ifeq, "ifeq"},
	{Bytecodes::_ifne, "ifne"},
	{Bytecodes::_iflt, "iflt"},
	{Bytecodes::_ifge, "ifge"},
	{Bytecodes::_ifgt, "ifgt"},
	{Bytecodes::_ifle, "ifle"},
	{Bytecodes::_if_icmpeq, "if_icmpeq"},
	{Bytecodes::_if_icmpne, "if_icmpne"},
	{Bytecodes::_if_icmplt, "if_icmplt"},
	{Bytecodes::_if_icmpge, "if_icmpge"},
	{Bytecodes::_if_icmpgt, "if_icmpgt"},
	{Bytecodes::_if_icmple, "if_icmple"},
	{Bytecodes::_if_acmpeq, "if_acmpeq"},
	{Bytecodes::_if_acmpne, "if_acmpne"},
	{Bytecodes::_goto, "goto"},
	{Bytecodes::_jsr, "jsr"},
	{Bytecodes::_ret, "ret"},
	{Bytecodes::_tableswitch, "tableswitch"},
	{Bytecodes::_lookupswitch, "lookupswitch"},
	{Bytecodes::_ireturn, "ireturn"},
	{Bytecodes::_lreturn, "lreturn"},
	{Bytecodes::_freturn, "freturn"},
	{Bytecodes::_dreturn, "dreturn"},
	{Bytecodes::_areturn, "areturn"},
	{Bytecodes::_return, "return"},
	{Bytecodes::_getstatic, "getstatic"},
	{Bytecodes::_putstatic, "putstatic"},
	{Bytecodes::_getfield, "getfield"},
	{Bytecodes::_putfield, "putfield"},
	{Bytecodes::_invokevirtual, "invokevirtual"},
	{Bytecodes::_invokespecial, "invokespecial"},
	{Bytecodes::_invokestatic, "invokestatic"},
	{Bytecodes::_invokeinterface, "invokeinterface"},
	{Bytecodes::_invokedynamic, "invokedynamic"},
	{Bytecodes::_new, "new"},
	{Bytecodes::_newarray, "newarray"},
	{Bytecodes::_anewarray, "anewarray"},
	{Bytecodes::_arraylength, "arraylength"},
	{Bytecodes::_athrow, "athrow"},
	{Bytecodes::_checkcast, "checkcast"},
	{Bytecodes::_instanceof, "instanceof"},
	{Bytecodes::_monitorenter, "monitorenter"},
	{Bytecodes::_monitorexit, "monitorexit"},
	{Bytecodes::_wide, "wide"},
	{Bytecodes::_multianewarray, "multianewarray"},
	{Bytecodes::_ifnull, "ifnull"},
	{Bytecodes::_ifnonnull, "ifnonnull"},
	{Bytecodes::_goto_w, "goto_w"},
	{Bytecodes::_jsr_w, "jsr_w"},
	{Bytecodes::_breakpoint, "breakpoint"},
	{Bytecodes::_fast_agetfield, "fast_agetfield"},
	{Bytecodes::_fast_bgetfield, "fast_bgetfield"},
	{Bytecodes::_fast_cgetfield, "fast_cgetfield"},
	{Bytecodes::_fast_dgetfield, "fast_dgetfield"},
	{Bytecodes::_fast_fgetfield, "fast_fgetfield"},
	{Bytecodes::_fast_igetfield, "fast_igetfield"},
	{Bytecodes::_fast_lgetfield, "fast_lgetfield"},
	{Bytecodes::_fast_sgetfield, "fast_sgetfield"},
	{Bytecodes::_fast_aputfield, "fast_aputfield"},
	{Bytecodes::_fast_bputfield, "fast_bputfield"},
	{Bytecodes::_fast_zputfield, "fast_zputfield"},
	{Bytecodes::_fast_cputfield, "fast_cputfield"},
	{Bytecodes::_fast_dputfield, "fast_dputfield"},
	{Bytecodes::_fast_fputfield, "fast_fputfield"},
	{Bytecodes::_fast_iputfield, "fast_iputfield"},
	{Bytecodes::_fast_lputfield, "fast_lputfield"},
	{Bytecodes::_fast_sputfield, "fast_sputfield"},
	{Bytecodes::_fast_aload_0, "fast_aload_0"},
	{Bytecodes::_fast_iaccess_0, "fast_iaccess_0"},
	{Bytecodes::_fast_aaccess_0, "fast_aaccess_0"},
	{Bytecodes::_fast_faccess_0, "fast_faccess_0"},
	{Bytecodes::_fast_iload, "fast_iload"},
	{Bytecodes::_fast_iload2, "fast_iload2"},
	{Bytecodes::_fast_icaload, "fast_icaload"},
	{ Bytecodes::_fast_invokevfinal,"fast_invokevfinal" },
	{ Bytecodes::_fast_linearswitch,"fast_linearswitch" },
	{ Bytecodes::_fast_binaryswitch,"fast_binaryswitch" },
	{ Bytecodes::_fast_aldc,"fast_aldc" },
	{ Bytecodes::_fast_aldc_w,"fast_aldc_w" },
	{ Bytecodes::_return_register_finalizer,"return_register_finalizer" },
	{ Bytecodes::_invokehandle,"invokehandle" },
	{ Bytecodes::_shouldnotreachhere,"shouldnotreachhere" },
	{ Bytecodes::number_of_codes,"number_of_codes" }
};


class Bytecode {
private:
	uint8_t* currentOpcode;

public:
	static std::map<uint8_t*, uint16_t> bytecodeOffsets;

	explicit Bytecode(uint8_t* address);
	explicit Bytecode(uint8_t* start, uint8_t* address);

	Bytecodes getOpcode();
	int getLength();
	int getStackConsumption();


};


#endif //BYTECODE_H