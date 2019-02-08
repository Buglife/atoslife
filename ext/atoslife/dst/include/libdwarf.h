/*

  Copyright (C) 2000-2010 Silicon Graphics, Inc.  All Rights Reserved.
  Portions Copyright 2007-2010 Sun Microsystems, Inc. All rights reserved.
  Portions Copyright 2008-2018 David Anderson. All rights reserved.
  Portions Copyright 2008-2010 Arxan Technologies, Inc. All rights reserved.
  Portions Copyright 2010-2012 SN Systems Ltd. All rights reserved.

  This program is free software; you can redistribute it and/or modify it
  under the terms of version 2.1 of the GNU Lesser General Public License
  as published by the Free Software Foundation.

  This program is distributed in the hope that it would be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

  Further, this software is distributed without any warranty that it is
  free of the rightful claim of any third person regarding infringement
  or the like.  Any license provided herein, whether implied or
  otherwise, applies only to this software file.  Patent licenses, if
  any, provided herein do not apply to combinations of this program with
  other software, or any other product whatsoever.

  You should have received a copy of the GNU Lesser General Public
  License along with this program; if not, write the Free Software
  Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston MA 02110-1301,
  USA.

*/


#ifndef _LIBDWARF_H
#define _LIBDWARF_H
#ifdef __cplusplus
extern "C" {
#endif
/*
    libdwarf.h
    $Revision: #9 $ $Date: 2008/01/17 $

    For libdwarf producers and consumers

    The interface is defined as having 8-byte signed and unsigned
    values so it can handle 64-or-32bit target on 64-or-32bit host.
    Dwarf_Ptr is the native size: it represents pointers on
    the host machine (not the target!).

    This contains declarations for types and all producer
    and consumer functions.

    Function declarations are written on a single line each here
    so one can use grep  to each declaration in its entirety.
    The declarations are a little harder to read this way, but...

    The seeming duplication of the Elf typedef allows
    both verification we have the right struct name (when
    libelf.h included before this) and
    creation of a local handle so we have the struct pointer
    here (if libelf.h is not included before this file).

*/

typedef struct Elf Elf;
typedef struct Elf* dwarf_elf_handle;

/* To enable printing with printf regardless of the
   actual underlying data type, we define the DW_PR_xxx macros.
   To ensure uses of DW_PR_DUx or DW_PR_DSx look the way you want
   ensure the right DW_PR_XZEROS define is uncommented.
*/
/*#define DW_PR_XZEROS "" */
#define DW_PR_XZEROS "08"

typedef unsigned long long Dwarf_Unsigned;
typedef signed   long long Dwarf_Signed;
typedef unsigned long long Dwarf_Off;
typedef unsigned long long Dwarf_Addr;
typedef int                Dwarf_Bool;   /* boolean type */
typedef unsigned short     Dwarf_Half;   /* 2 byte unsigned value */
typedef unsigned char      Dwarf_Small;  /* 1 byte unsigned value */
/*  If sizeof(Dwarf_Half) is greater than  2
    we intend libdwarf should work anyway, but
    as of June 14, 2018 it only mostly works. */

#ifdef _WIN32
#define DW_PR_DUx "I64x"
#define DW_PR_DSx "I64x"
#define DW_PR_DUu "I64u"
#define DW_PR_DSd "I64d"
#else
#define DW_PR_DUx "llx"
#define DW_PR_DSx "llx"
#define DW_PR_DUu "llu"
#define DW_PR_DSd "lld"
#endif /* DW_PR defines */

typedef void*        Dwarf_Ptr;          /* host machine pointer */

/*  DWARF5: a container for a DW_FORM_data16 data item.
    We have no integer types suitable so this special
    struct is used instead.  It is up to consumers/producers
    to deal with the contents.
    New October 18, 2017 .  */
typedef struct Dwarf_Form_Data16_s {
    unsigned char fd_data[16];
} Dwarf_Form_Data16;



/* Used for signatures where ever they appear.
   It is not a string, it
   is 8 bytes of a signature one would use to find
   a type unit. See dwarf_formsig8()

   Sometimes it is used in calculations as
   Dwarf_Unsigned, but that is done inside libdwarf
   and the endianness question makes it a bit sketchy.
*/
struct Dwarf_Sig8_s  {
    char signature[8];
};
typedef struct Dwarf_Sig8_s Dwarf_Sig8;

/* Contains info on an uninterpreted block of data
   Used with certain frame information functions.
*/
typedef struct {
    Dwarf_Unsigned  bl_len;         /* length of block bl_data points at */
    Dwarf_Ptr       bl_data;        /* uninterpreted data */

    /*  0 if location description,
        1 if .debug_info loclist,
        2 if .debug_info.dwo split dwarf loclist. */
    Dwarf_Small     bl_from_loclist;

    /* Section (not CU) offset which 'data' comes from. */
    Dwarf_Unsigned  bl_section_offset;
} Dwarf_Block;


/*  NEW October 2015.  */
/*  Dwarf_Loc_c_s,Dwarf_Locdesc_c_s, and Dwarf_Loc_Head_c_s
    are not defined publically. */
struct Dwarf_Loc_c_s;
typedef struct Dwarf_Loc_c_s * Dwarf_Loc_c;

/*  NEW October 2015.  */
/*  This provides access to Dwarf_Loc_c, a single location operator */
struct Dwarf_Locdesc_c_s;
typedef struct Dwarf_Locdesc_c_s * Dwarf_Locdesc_c;

/*  NEW October 2015.  */
/*  This provides access to Dwarf_Locdesc_c, a single
    location list entry (or for a locexpr, the fake
    Loc_Head for the locexpr) */
struct Dwarf_Loc_Head_c_s;
typedef struct Dwarf_Loc_Head_c_s * Dwarf_Loc_Head_c;

/* NEW November 2015. For DWARF5 .debug_macro section */
struct Dwarf_Macro_Context_s;
typedef struct Dwarf_Macro_Context_s * Dwarf_Loc_Macro_Context;

/*  NEW September 2016. Allows easy access to DW_AT_discr_list
    array of discriminant values. Input in blockpointer
    is a block with a list of uleb or sleb numbers
    (all one or the other, lebunsignedflag instructs
    how to read the leb values properly) */
typedef struct Dwarf_Dsc_Head_s * Dwarf_Dsc_Head;

/*  Location record. Records up to 2 operand values.
    Not usable with DWARF5 or DWARF4 with location
    operator  extensions. */
typedef struct {
    Dwarf_Small     lr_atom;        /* location operation */
    Dwarf_Unsigned  lr_number;      /* operand */
    Dwarf_Unsigned  lr_number2;     /* for OP_BREGx  and DW_OP_GNU_const_type*/
    Dwarf_Unsigned  lr_offset;      /* offset in locexpr for OP_BRA etc */
} Dwarf_Loc;


/* Location description. DWARF 2,3,4.
   When this is from a split-dwarf loclist (.debug_loc.dwo)
   and no tied object is present
   then ld_lowpc and ld_highpc are actually indices in
   the .debug_addr section of the tied object).
   If there is a tied object then these fields are actuall
   addresses and DW_AT_addr_base in the skeleton CU DIE applies to
   that .debug_addr.

   Location record. Records up to 2 operand values.
   Not usable with DWARF5 or DWARF4 with extensions.

   If from DWARF2,3,4 non-split dwarf then things operate as
   in DWARF2.
   See dwarf_get_loclist_b() and the other related
   new functions that
   avoid using public structures Dwarf_Loc and Dwarf_Locdesc.
*/
typedef struct {
    /*  Beginning of active range. This is actually an offset
        of an applicable base address, not a pc value.  */
    Dwarf_Addr      ld_lopc;

    /*  End of active range. This is actually an offset
        of an applicable base address, not a pc value.  */
    Dwarf_Addr      ld_hipc;

    Dwarf_Half      ld_cents;       /* count of location records */
    Dwarf_Loc*      ld_s;           /* pointer to list of same */

    /*  non-0 if loclist, 1 if non-split (dwarf 2,3,4) */
    Dwarf_Small     ld_from_loclist;

    Dwarf_Unsigned  ld_section_offset; /* Section (not CU) offset
        where loc-expr begins*/
} Dwarf_Locdesc;

/*  First appears in DWARF3.
    The dwr_addr1/addr2 data is either an offset (DW_RANGES_ENTRY)
    or an address (dwr_addr2 in DW_RANGES_ADDRESS_SELECTION) or
    both are zero (DW_RANGES_END).
*/
enum Dwarf_Ranges_Entry_Type { DW_RANGES_ENTRY,
    DW_RANGES_ADDRESS_SELECTION,
    DW_RANGES_END
};
typedef struct {
    Dwarf_Addr dwr_addr1;
    Dwarf_Addr dwr_addr2;
    enum Dwarf_Ranges_Entry_Type  dwr_type;
} Dwarf_Ranges;

/* Frame description instructions expanded.
*/
typedef struct {
    Dwarf_Small     fp_base_op;
    Dwarf_Small     fp_extended_op;
    Dwarf_Half      fp_register;

    /*  Value may be signed, depends on op.
        Any applicable data_alignment_factor has
        not been applied, this is the  raw offset. */
    Dwarf_Unsigned  fp_offset;
    Dwarf_Off       fp_instr_offset;
} Dwarf_Frame_Op; /* DWARF2 */

/*  ***IMPORTANT NOTE, TARGET DEPENDENCY ****
    DW_REG_TABLE_SIZE must be at least as large as
    the number of registers
    (DW_FRAME_LAST_REG_NUM) as defined in dwarf.h
    Preferably identical to DW_FRAME_LAST_REG_NUM.
    Ensure [0-DW_REG_TABLE_SIZE] does not overlap
    DW_FRAME_UNDEFINED_VAL or DW_FRAME_SAME_VAL.
    Also ensure DW_FRAME_REG_INITIAL_VALUE is set to what
    is appropriate to your cpu.
    For various CPUs  DW_FRAME_UNDEFINED_VAL is correct
    as the value for DW_FRAME_REG_INITIAL_VALUE.

    For consumer apps, this can be set dynamically: see
    dwarf_set_frame_rule_table_size(); */
#ifndef DW_REG_TABLE_SIZE
#define DW_REG_TABLE_SIZE  66
#endif

/* For MIPS, DW_FRAME_SAME_VAL is the correct default value
   for a frame register value. For other CPUS another value
   may be better, such as DW_FRAME_UNDEFINED_VAL.
   See dwarf_set_frame_rule_table_size
*/
#ifndef DW_FRAME_REG_INITIAL_VALUE
#define DW_FRAME_REG_INITIAL_VALUE DW_FRAME_SAME_VAL
#endif

/* Taken as meaning 'undefined value', this is not
   a column or register number.
   Only present at libdwarf runtime in the consumer
   interfaces. Never on disk.
   DW_FRAME_* Values present on disk are in dwarf.h
   Ensure this is > DW_REG_TABLE_SIZE (the reg table
   size is changeable at runtime with the *reg3() interfaces,
   and this value must be greater than the reg table size).
*/
#define DW_FRAME_UNDEFINED_VAL          1034

/* Taken as meaning 'same value' as caller had, not a column
   or register number.
   Only present at libdwarf runtime in the consumer
   interfaces. Never on disk.
   DW_FRAME_* Values present on disk are in dwarf.h
   Ensure this is > DW_REG_TABLE_SIZE (the reg table
   size is changeable at runtime with the *reg3() interfaces,
   and this value must be greater than the reg table size).
*/
#define DW_FRAME_SAME_VAL               1035

/* For DWARF3 consumer interfaces, make the CFA a column with no
   real table number.  This is what should have been done
   for the DWARF2 interfaces.  This actually works for
   both DWARF2 and DWARF3, but see the libdwarf documentation
   on Dwarf_Regtable3 and  dwarf_get_fde_info_for_reg3()
   and  dwarf_get_fde_info_for_all_regs3()
   Do NOT use this with the older dwarf_get_fde_info_for_reg()
   or dwarf_get_fde_info_for_all_regs() consumer interfaces.
   Must be higher than any register count for *any* ABI
   (ensures maximum applicability with minimum effort).
   Ensure this is > DW_REG_TABLE_SIZE (the reg table
   size is changeable at runtime with the *reg3() interfaces,
   and this value must be greater than the reg table size).
   Only present at libdwarf runtime in the consumer
   interfaces. Never on disk.
*/
#define DW_FRAME_CFA_COL3               1436

/* The following are all needed to evaluate DWARF3 register rules.
*/
#define DW_EXPR_OFFSET 0  /* DWARF2 only sees this. */
#define DW_EXPR_VAL_OFFSET 1
#define DW_EXPR_EXPRESSION 2
#define DW_EXPR_VAL_EXPRESSION 3

typedef struct Dwarf_Regtable_Entry_s {
    /*  For each index i (naming a hardware register with dwarf number
        i) the following is true and defines the value of that register:

        If dw_regnum is Register DW_FRAME_UNDEFINED_VAL
            it is not DWARF register number but
            a place holder indicating the register has no defined value.
        If dw_regnum is Register DW_FRAME_SAME_VAL
            it  is not DWARF register number but
            a place holder indicating the register has the same
            value in the previous frame.

            DW_FRAME_UNDEFINED_VAL, DW_FRAME_SAME_VAL are
            only present at libdwarf runtime. Never on disk.
            DW_FRAME_* Values present on disk are in dwarf.h

        Otherwise: the register number is a DWARF register number
            (see ABI documents for how this translates to hardware/
            software register numbers in the machine hardware)
            and the following applies:

            if dw_value_type == DW_EXPR_OFFSET (the only case for dwarf2):
                If dw_offset_relevant is non-zero, then
                    the value is stored at at the address CFA+N where
                    N is a signed offset.
                    Rule: Offset(N)
                If dw_offset_relevant is zero, then the value of the register
                    is the value of (DWARF) register number dw_regnum.
                    Rule: register(F)
            Other values of dw_value_type are an error.
    */
    Dwarf_Small         dw_offset_relevant;

    /* For DWARF2, always 0 */
    Dwarf_Small         dw_value_type;

    Dwarf_Half          dw_regnum;

    /*  The data type here should  the larger of Dwarf_Addr
        and Dwarf_Unsigned and Dwarf_Signed. */
    Dwarf_Addr          dw_offset;
} Dwarf_Regtable_Entry;

typedef struct Dwarf_Regtable_s {
    struct Dwarf_Regtable_Entry_s rules[DW_REG_TABLE_SIZE];
} Dwarf_Regtable;

/* opaque type. Functional interface shown later. */
struct Dwarf_Reg_value3_s;
typedef struct Dwarf_Reg_value3_s Dwarf_Reg_Value3;

typedef struct Dwarf_Regtable_Entry3_s {
/*  For each index i (naming a hardware register with dwarf number
    i) the following is true and defines the value of that register:

        If dw_regnum is Register DW_FRAME_UNDEFINED_VAL
            it is not DWARF register number but
            a place holder indicating the register has no defined value.
        If dw_regnum is Register DW_FRAME_SAME_VAL
            it  is not DWARF register number but
            a place holder indicating the register has the same
            value in the previous frame.

            DW_FRAME_UNDEFINED_VAL, DW_FRAME_SAME_VAL and
            DW_FRAME_CFA_COL3 are only present at libdwarf runtime.
            Never on disk.
            DW_FRAME_* Values present on disk are in dwarf.h
            Because DW_FRAME_SAME_VAL and DW_FRAME_UNDEFINED_VAL
            and DW_FRAME_CFA_COL3 are definable at runtime
            consider the names symbolic in this comment, not absolute.

        Otherwise: the register number is a DWARF register number
            (see ABI documents for how this translates to hardware/
            software register numbers in the machine hardware)
            and the following applies:

        In a cfa-defining entry (rt3_cfa_rule) the regnum is the
        CFA 'register number'. Which is some 'normal' register,
        not DW_FRAME_CFA_COL3, nor DW_FRAME_SAME_VAL, nor
        DW_FRAME_UNDEFINED_VAL.

        If dw_value_type == DW_EXPR_OFFSET (the only  possible case for
        dwarf2):
            If dw_offset_relevant is non-zero, then
                the value is stored at at the address
                CFA+N where N is a signed offset.
                dw_regnum is the cfa register rule which means
                one ignores dw_regnum and uses the CFA appropriately.
                So dw_offset_or_block_len is a signed value, really,
                and must be printed/evaluated as such.
                Rule: Offset(N)
            If dw_offset_relevant is zero, then the value of the register
                is the value of (DWARF) register number dw_regnum.
                Rule: register(R)
        If dw_value_type  == DW_EXPR_VAL_OFFSET
            the  value of this register is CFA +N where N is a signed offset.
            dw_regnum is the cfa register rule which means
            one ignores dw_regnum and uses the CFA appropriately.
            Rule: val_offset(N)
        If dw_value_type  == DW_EXPR_EXPRESSION
            The value of the register is the value at the address
            computed by evaluating the DWARF expression E.
            Rule: expression(E)
            The expression E byte stream is pointed to by dw_block_ptr.
            The expression length in bytes is given by
            dw_offset_or_block_len.
        If dw_value_type  == DW_EXPR_VAL_EXPRESSION
            The value of the register is the value
            computed by evaluating the DWARF expression E.
            Rule: val_expression(E)
            The expression E byte stream is pointed to by dw_block_ptr.
            The expression length in bytes is given by
            dw_offset_or_block_len.
        Other values of dw_value_type are an error.
*/
    Dwarf_Small         dw_offset_relevant;
    Dwarf_Small         dw_value_type;
    Dwarf_Half          dw_regnum;
    Dwarf_Unsigned      dw_offset_or_block_len;
    Dwarf_Ptr           dw_block_ptr;

}Dwarf_Regtable_Entry3;

/*  For the DWARF3 version, moved the DW_FRAME_CFA_COL
    out of the array and into its own struct.
    Having it part of the array is not very easy to work
    with from a portability point of view: changing
    the number for every architecture is a pain (if one fails
    to set it correctly a register rule gets clobbered when
    setting CFA).  With MIPS it just happened to be easy to use
    DW_FRAME_CFA_COL (it was wrong conceptually but it was easy...).

    rt3_rules and rt3_reg_table_size must be filled in before
    calling libdwarf.  Filled in with a pointer to an array
    (pointer and array  set up by the calling application)
    of rt3_reg_table_size Dwarf_Regtable_Entry3_s structs.
    libdwarf does not allocate or deallocate space for the
    rules, you must do so.   libdwarf will initialize the
    contents rules array, you do not need to do so (though
    if you choose to initialize the array somehow that is ok:
    libdwarf will overwrite your initializations with its own).

*/
typedef struct Dwarf_Regtable3_s {
    struct Dwarf_Regtable_Entry3_s   rt3_cfa_rule;

    Dwarf_Half                       rt3_reg_table_size;
    struct Dwarf_Regtable_Entry3_s * rt3_rules;
} Dwarf_Regtable3;


/*  Use for DW_EPXR_STANDARD., DW_EXPR_VAL_OFFSET.
    Returns DW_DLV_OK if the value is available.
    If DW_DLV_OK returns the regnum and offset thru the pointers
    (which the consumer must use appropriately).
*/
int dwarf_frame_get_reg_register(struct Dwarf_Regtable_Entry3_s *reg_in,
    Dwarf_Small *offset_relevant,
    Dwarf_Half *regnum_out,
    Dwarf_Signed *offset_out);

/*  Use for DW_EXPR_EXPRESSION, DW_EXPR_VAL_EXPRESSION.
    Returns DW_DLV_OK if the value is available.
    The caller must pass in the address of a valid
    Dwarf_Block (the caller need not initialize it).
*/
int dwarf_frame_get_reg_expression(struct Dwarf_Regtable_Entry3_s *reg_in,
    Dwarf_Block *block_out);


/*  For DW_DLC_SYMBOLIC_RELOCATIONS output to caller
    v2, adding drd_length: some relocations are 4 and
    some 8 bytes (pointers are 8, section offsets 4) in
    some dwarf environments. (MIPS relocations are all one
    size in any given ABI.) Changing drd_type to an unsigned char
    to keep struct size down.
*/
enum Dwarf_Rel_Type {
    dwarf_drt_none,        /* Should not get to caller */
    dwarf_drt_data_reloc,  /* Simple normal relocation. */
    dwarf_drt_segment_rel, /* Special reloc, exceptions. */
    /* dwarf_drt_first_of_length_pair  and drt_second
        are for for the  .word end - begin case. */
    dwarf_drt_first_of_length_pair,
    dwarf_drt_second_of_length_pair
};

typedef struct Dwarf_P_Marker_s * Dwarf_P_Marker;
struct Dwarf_P_Marker_s {
    Dwarf_Unsigned ma_marker;
    Dwarf_Unsigned ma_offset;
};

typedef struct Dwarf_Relocation_Data_s  * Dwarf_Relocation_Data;
struct Dwarf_Relocation_Data_s {
    unsigned char drd_type;   /* Cast to/from Dwarf_Rel_Type
        to keep size small in struct. */
    unsigned char drd_length; /* Length in bytes of data being
        relocated. 4 for 32bit data,
        8 for 64bit data. */
    Dwarf_Unsigned       drd_offset; /* Where the data to reloc is. */
    Dwarf_Unsigned       drd_symbol_index;
};

typedef struct Dwarf_P_String_Attr_s  * Dwarf_P_String_Attr;
struct Dwarf_P_String_Attr_s {
    Dwarf_Unsigned        sa_offset;  /* Offset of string attribute data */
    Dwarf_Unsigned        sa_nbytes;
};


/* Opaque types for Consumer Library. */
typedef struct Dwarf_Debug_s*      Dwarf_Debug;
typedef struct Dwarf_Die_s*        Dwarf_Die;
typedef struct Dwarf_Line_s*       Dwarf_Line;
typedef struct Dwarf_Global_s*     Dwarf_Global;
typedef struct Dwarf_Func_s*       Dwarf_Func;
typedef struct Dwarf_Type_s*       Dwarf_Type;
typedef struct Dwarf_Var_s*        Dwarf_Var;
typedef struct Dwarf_Weak_s*       Dwarf_Weak;
typedef struct Dwarf_Error_s*      Dwarf_Error;
typedef struct Dwarf_Attribute_s*  Dwarf_Attribute;
typedef struct Dwarf_Abbrev_s*     Dwarf_Abbrev;
typedef struct Dwarf_Fde_s*        Dwarf_Fde;
typedef struct Dwarf_Cie_s*        Dwarf_Cie;
typedef struct Dwarf_Arange_s*     Dwarf_Arange;
typedef struct Dwarf_Gdbindex_s*   Dwarf_Gdbindex;
struct Dwarf_Xu_Index_Header_s;
typedef struct Dwarf_Xu_Index_Header_s* Dwarf_Xu_Index_Header;
struct Dwarf_Line_Context_s;
typedef struct Dwarf_Line_Context_s *Dwarf_Line_Context;
struct Dwarf_Macro_Context_s;
typedef struct Dwarf_Macro_Context_s *Dwarf_Macro_Context;
struct Dwarf_Dnames_Head_s;
typedef struct Dwarf_Dnames_Head_s* Dwarf_Dnames_Head;



/* Opaque types for Producer Library. */
typedef struct Dwarf_P_Debug_s*       Dwarf_P_Debug;
typedef struct Dwarf_P_Die_s*         Dwarf_P_Die;
typedef struct Dwarf_P_Attribute_s*   Dwarf_P_Attribute;
typedef struct Dwarf_P_Fde_s*         Dwarf_P_Fde;
typedef struct Dwarf_P_Expr_s*        Dwarf_P_Expr;
typedef Dwarf_Unsigned                Dwarf_Tag;


/* error handler function
*/
typedef void  (*Dwarf_Handler)(Dwarf_Error /*error*/, Dwarf_Ptr /*errarg*/);


/* Begin libdwarf Object File Interface declarations.

As of February 2008 there are multiple dwarf_reader object access
initialization methods available:
The traditional dwarf_elf_init() and dwarf_init()  and dwarf_finish()
    which assume libelf and POSIX file access.
An object-file and library agnostic dwarf_object_init() and dwarf_object_finish()
    which allow the coder to provide object access routines
    abstracting away the elf interface.  So there is no dependence in the
    reader code on the object format and no dependence on libelf.
    See the code in dwarf_elf_access.c  and dwarf_original_elf_init.c
    to see an example of initializing the structures mentioned below.

Projects using dwarf_elf_init() or dwarf_init() can ignore
the Dwarf_Obj_Access* structures entirely as all these details
are completed for you.

As of March 2017 additional functions dwarf_elf_init_b() and dwarf_init_b()
and dwarf_object_init_b() add a groupnumber argument so DWARF5
split-dwarf sections can be accessed.

*/

typedef struct Dwarf_Obj_Access_Interface_s   Dwarf_Obj_Access_Interface;
typedef struct Dwarf_Obj_Access_Methods_s     Dwarf_Obj_Access_Methods;
typedef struct Dwarf_Obj_Access_Section_s     Dwarf_Obj_Access_Section;


/*  Used in the get_section interface function
    in Dwarf_Obj_Access_Section_s.  Since libdwarf
    depends on standard DWARF section names an object
    format that has no such names (but has some
    method of setting up 'sections equivalents')
    must arrange to return standard DWARF section
    names in the 'name' field.  libdwarf does
    not free the strings in 'name'. */
struct Dwarf_Obj_Access_Section_s {
    /*  addr is the virtual address of the first byte of
        the section data.  Usually zero when the address
        makes no sense for a given section. */
    Dwarf_Addr     addr;

    /* Section type. */
    Dwarf_Unsigned type;

    /* Size in bytes of the section. */
    Dwarf_Unsigned size;

    /*  Having an accurate section name makes debugging of libdwarf easier.
        and is essential to find the .debug_ sections.  */
    const char*    name;
    /*  Set link to zero if it is meaningless.  If non-zero
        it should be a link to a rela section or from symtab
        to strtab.  In Elf it is sh_link. */
    Dwarf_Unsigned link;

    /*  The section header index of the section to which the
        relocation applies. In Elf it is sh_info. */
    Dwarf_Unsigned info;

    /*  Elf sections that are tables have a non-zero entrysize so
        the count of entries can be calculated even without
        the right structure definition. If your object format
        does not have this data leave this zero. */
    Dwarf_Unsigned entrysize;
};

/*  Returned by the get_endianness function in
    Dwarf_Obj_Access_Methods_s. */
typedef enum {
    DW_OBJECT_MSB,
    DW_OBJECT_LSB
} Dwarf_Endianness;

/*  The functions we need to access object data from libdwarf are declared here.

    In these function pointer declarations
    'void *obj' is intended to be a pointer (the object field in
    Dwarf_Obj_Access_Interface_s)
    that hides the library-specific and object-specific data that makes
    it possible to handle multiple object formats and multiple libraries.
    It's not required that one handles multiple such in a single libdwarf
    archive/shared-library (but not ruled out either).
    See  dwarf_elf_object_access_internals_t and dwarf_elf_access.c
    for an example.

*/
struct Dwarf_Obj_Access_Methods_s {
    /*
        get_section_info

        Get address, size, and name info about a section.

        Parameters
        section_index - Zero-based index.
        return_section - Pointer to a structure in which section info
            will be placed.   Caller must provide a valid pointer to a
            structure area.  The structure's contents will be overwritten
            by the call to get_section_info.
        error - A pointer to an integer in which an error code may be stored.

        Return
        DW_DLV_OK - Everything ok.
        DW_DLV_ERROR - Error occurred. Use 'error' to determine the
            libdwarf defined error.
        DW_DLV_NO_ENTRY - No such section.  */
    int    (*get_section_info)(void* obj, Dwarf_Half section_index,
        Dwarf_Obj_Access_Section* return_section, int* error);
    /*
        get_byte_order

        Get whether the object file represented by this interface is big-endian
        (DW_OBJECT_MSB) or little endian (DW_OBJECT_LSB).

        Parameters
        obj - Equivalent to 'this' in OO languages.

        Return
        Endianness of object. Cannot fail.  */
    Dwarf_Endianness  (*get_byte_order)(void* obj);
    /*
        get_length_size

        Get the size of a length field in the underlying object file.
        libdwarf currently supports * 4 and 8 byte sizes, but may
        support larger in the future.
        Perhaps the return type should be an enumeration?

        Parameters
        obj - Equivalent to 'this' in OO languages.

        Return
        Size of length. Cannot fail.  */
    Dwarf_Small   (*get_length_size)(void* obj);
    /*
        get_pointer_size

        Get the size of a pointer field in the underlying object file.
        libdwarf currently supports  4 and 8 byte sizes.
        Perhaps the return type should be an enumeration?

        Return
        Size of pointer. Cannot fail.  */
    Dwarf_Small   (*get_pointer_size)(void* obj);
    /*
        get_section_count

        Get the number of sections in the object file.

        Parameters

        Return
        Number of sections */
    Dwarf_Unsigned  (*get_section_count)(void* obj);
    /*
        load_section

        Get a pointer to an array of bytes that represent the section.

        Parameters
        section_index - Zero-based index.
        return_data - The address of a pointer to which the section data block
            will be assigned.
        error - Pointer to an integer for returning libdwarf-defined
            error numbers.

        Return
        DW_DLV_OK - No error.
        DW_DLV_ERROR - Error. Use 'error' to indicate a libdwarf-defined
            error number.
        DW_DLV_NO_ENTRY - No such section.  */
    int    (*load_section)(void* obj, Dwarf_Half section_index,
        Dwarf_Small** return_data, int* error);

    /**
        relocate_a_section
        If relocations are not supported leave this pointer NULL.

        Get a pointer to an array of bytes that represent the section.

        Parameters
        section_index - Zero-based index of the section to be relocated.
        error - Pointer to an integer for returning libdwarf-defined
            error numbers.

        Return
        DW_DLV_OK - No error.
        DW_DLV_ERROR - Error. Use 'error' to indicate a libdwarf-defined
            error number.
        DW_DLV_NO_ENTRY - No such section.  */
    int    (*relocate_a_section)(void* obj, Dwarf_Half section_index,
        Dwarf_Debug dbg,
        int* error);

};



/*  These structures are allocated and deallocated by your code
    when you are using the libdwarf Object File Interface
    [dwarf_object_init() and dwarf_object_finish()] directly.
    dwarf_object_finish() does not free
    struct Dwarf_Obj_Access_Interface_s or its content.
    (libdwarf does record a pointer to this struct: you must
    ensure that pointer remains valid for as long as
    a libdwarf instance is open (meaning
    after dwarf_init() and before dwarf_finish()).

    If you are reading Elf objects and libelf use dwarf_init()
    or dwarf_elf_init() which take care of these details.
*/
struct Dwarf_Obj_Access_Interface_s {
    /*  object is a void* as it hides the data the object access routines
        need (which varies by library in use and object format).
    */
    void* object;
    const Dwarf_Obj_Access_Methods * methods;
};

/* End libdwarf Object File Interface */

/*
    Dwarf_dealloc() alloc_type arguments.
    Argument points to:
*/
#define DW_DLA_STRING          0x01     /* char* */
#define DW_DLA_LOC             0x02     /* Dwarf_Loc */
#define DW_DLA_LOCDESC         0x03     /* Dwarf_Locdesc */
#define DW_DLA_ELLIST          0x04     /* Dwarf_Ellist (not used)*/
#define DW_DLA_BOUNDS          0x05     /* Dwarf_Bounds (not used) */
#define DW_DLA_BLOCK           0x06     /* Dwarf_Block */
#define DW_DLA_DEBUG           0x07     /* Dwarf_Debug */
#define DW_DLA_DIE             0x08     /* Dwarf_Die */
#define DW_DLA_LINE            0x09     /* Dwarf_Line */
#define DW_DLA_ATTR            0x0a     /* Dwarf_Attribute */
#define DW_DLA_TYPE            0x0b     /* Dwarf_Type  (not used) */
#define DW_DLA_SUBSCR          0x0c     /* Dwarf_Subscr (not used) */
#define DW_DLA_GLOBAL          0x0d     /* Dwarf_Global */
#define DW_DLA_ERROR           0x0e     /* Dwarf_Error */
#define DW_DLA_LIST            0x0f     /* a list */
#define DW_DLA_LINEBUF         0x10     /* Dwarf_Line* (not used) */
#define DW_DLA_ARANGE          0x11     /* Dwarf_Arange */
#define DW_DLA_ABBREV          0x12     /* Dwarf_Abbrev */
#define DW_DLA_FRAME_OP        0x13     /* Dwarf_Frame_Op */
#define DW_DLA_CIE             0x14     /* Dwarf_Cie */
#define DW_DLA_FDE             0x15     /* Dwarf_Fde */
#define DW_DLA_LOC_BLOCK       0x16     /* Dwarf_Loc */
#define DW_DLA_FRAME_BLOCK     0x17     /* Dwarf_Frame Block (not used) */
#define DW_DLA_FUNC            0x18     /* Dwarf_Func */
#define DW_DLA_TYPENAME        0x19     /* Dwarf_Type */
#define DW_DLA_VAR             0x1a     /* Dwarf_Var */
#define DW_DLA_WEAK            0x1b     /* Dwarf_Weak */
#define DW_DLA_ADDR            0x1c     /* Dwarf_Addr sized entries */
#define DW_DLA_RANGES          0x1d     /* Dwarf_Ranges */

/* 0x1e (30) to 0x36 (54) reserved for internal to libdwarf types. */

#define DW_DLA_GDBINDEX        0x37     /* Dwarf_Gdbindex */
#define DW_DLA_XU_INDEX        0x38     /* Dwarf_Xu_Index_Header */
#define DW_DLA_LOC_BLOCK_C     0x39     /* Dwarf_Loc_c*/
#define DW_DLA_LOCDESC_C       0x3a     /* Dwarf_Locdesc_c */
#define DW_DLA_LOC_HEAD_C      0x3b     /* Dwarf_Loc_Head_c */
#define DW_DLA_MACRO_CONTEXT   0x3c     /* Dwarf_Macro_Context */
/*  0x3d (61) is for libdwarf internal use.               */
#define DW_DLA_DSC_HEAD        0x3e     /* Dwarf_Dsc_Head */
#define DW_DLA_DNAMES_HEAD     0x3f     /* Dwarf_Dnames_Head */
#define DW_DLA_STR_OFFSETS     0x40     /* struct Dwarf_Str_Offsets_Table_s */

/* The augmenter string for CIE */
#define DW_CIE_AUGMENTER_STRING_V0              "z"

/* dwarf_init() access arguments
*/
#define DW_DLC_READ        0        /* read only access */
#define DW_DLC_WRITE       1        /* write only access */
#define DW_DLC_RDWR        2        /* read/write access NOT SUPPORTED*/

/* dwarf_producer_init*() access flag modifiers
   No longer depends on compile-time settings for
   how to produce 64bit offset. See DW_DLC_IRIX_OFFSET64.
   Historic  versions. One of
   If DW_DLC_POINTER64 is not set DW_DLC_POINTER32 is assumed.
   If DW_DLC_OFFSET64 or DW_DLC_IRIX_OFFSET64 is not
   set 32bit offset DWARF is assumed.
   Non-MIPS Non IA64 should use DW_DLC_SYMBOLIC_RELOCATIONS
   and handle the relocation creation for the target
   itself using the symbolic relocations to do so, those
   use the Dwarf_Rel_Type enum relocation indicators.

*/
/*  The first three are traditional dwarf producer names.
    These names still work.
    Newer names below.
*/
#define DW_DLC_SIZE_64              0x40000000 /* 64-bit address-size target */
#define DW_DLC_SIZE_32              0x20000000 /* 32-bit address-size target */
#define DW_DLC_OFFSET_SIZE_64       0x10000000 /* 64-bit offset-size DWARF */

/* dwarf_producer_init*() access flag modifiers
   Some new April 2014.
   If DW_DLC_STREAM_RELOCATIONS is set the
   DW_DLC_ISA_* flags are ignored. See the Dwarf_Rel_Type enum.
*/

/* Old style Elf binary relocation (.rel) records. The default. */
#define DW_DLC_STREAM_RELOCATIONS   0x02000000

#define DW_DLC_OFFSET32             0x00010000 /* use 32-bit sec offsets */
/* The following 3 are new sensible names. Old names above with same values. */
#define DW_DLC_OFFSET64             0x10000000 /* use 64-bit sec offsets */
#define DW_DLC_POINTER32            0x20000000 /* use 4 for address_size */
#define DW_DLC_POINTER64            0x40000000 /* use 8 for address_size */
/* Special for IRIX only */
#define DW_DLC_IRIX_OFFSET64        0x00200000 /* use non-std IRIX 64bitoffset headers  */

/*  Usable with assembly output because it is up to the producer to
    deal with locations in whatever manner the calling producer
    code wishes.  For example, when the libdwarf caller wishes
    to produce relocations differently than the binary
    relocation bits that libdwarf Stream Relocations generate.
    */
#define DW_DLC_SYMBOLIC_RELOCATIONS 0x04000000


#define DW_DLC_TARGET_BIGENDIAN     0x08000000 /* Big    endian target */
#define DW_DLC_TARGET_LITTLEENDIAN  0x00100000 /* Little endian target */


/* dwarf_pcline() slide arguments
*/
#define DW_DLS_BACKWARD   -1       /* slide backward to find line */
#define DW_DLS_NOSLIDE     0       /* match exactly without sliding */
#define DW_DLS_FORWARD     1       /* slide forward to find line */

/* libdwarf error numbers
*/
#define DW_DLE_NE          0     /* no error */
#define DW_DLE_VMM         1     /* dwarf format/library version mismatch */
#define DW_DLE_MAP         2     /* memory map failure */
#define DW_DLE_LEE         3     /* libelf error */
#define DW_DLE_NDS         4     /* no debug section */
#define DW_DLE_NLS         5     /* no line section */
#define DW_DLE_ID          6     /* invalid descriptor for query */
#define DW_DLE_IOF         7     /* I/O failure */
#define DW_DLE_MAF         8     /* memory allocation failure */
#define DW_DLE_IA          9     /* invalid argument */
#define DW_DLE_MDE         10     /* mangled debugging entry */
#define DW_DLE_MLE         11     /* mangled line number entry */
#define DW_DLE_FNO         12     /* file not open */
#define DW_DLE_FNR         13     /* file not a regular file */
#define DW_DLE_FWA         14     /* file open with wrong access */
#define DW_DLE_NOB         15     /* not an object file */
#define DW_DLE_MOF         16     /* mangled object file header */
#define DW_DLE_EOLL        17     /* end of location list entries */
#define DW_DLE_NOLL        18     /* no location list section */
#define DW_DLE_BADOFF      19     /* Invalid offset */
#define DW_DLE_EOS         20     /* end of section  */
#define DW_DLE_ATRUNC      21     /* abbreviations section appears truncated*/
#define DW_DLE_BADBITC     22     /* Address size passed to dwarf bad*/
                    /* It is not an allowed size (64 or 32) */
    /* Error codes defined by the current Libdwarf Implementation. */
#define DW_DLE_DBG_ALLOC                        23
#define DW_DLE_FSTAT_ERROR                      24
#define DW_DLE_FSTAT_MODE_ERROR                 25
#define DW_DLE_INIT_ACCESS_WRONG                26
#define DW_DLE_ELF_BEGIN_ERROR                  27
#define DW_DLE_ELF_GETEHDR_ERROR                28
#define DW_DLE_ELF_GETSHDR_ERROR                29
#define DW_DLE_ELF_STRPTR_ERROR                 30
#define DW_DLE_DEBUG_INFO_DUPLICATE             31
#define DW_DLE_DEBUG_INFO_NULL                  32
#define DW_DLE_DEBUG_ABBREV_DUPLICATE           33
#define DW_DLE_DEBUG_ABBREV_NULL                34
#define DW_DLE_DEBUG_ARANGES_DUPLICATE          35
#define DW_DLE_DEBUG_ARANGES_NULL               36
#define DW_DLE_DEBUG_LINE_DUPLICATE             37
#define DW_DLE_DEBUG_LINE_NULL                  38
#define DW_DLE_DEBUG_LOC_DUPLICATE              39
#define DW_DLE_DEBUG_LOC_NULL                   40
#define DW_DLE_DEBUG_MACINFO_DUPLICATE          41
#define DW_DLE_DEBUG_MACINFO_NULL               42
#define DW_DLE_DEBUG_PUBNAMES_DUPLICATE         43
#define DW_DLE_DEBUG_PUBNAMES_NULL              44
#define DW_DLE_DEBUG_STR_DUPLICATE              45
#define DW_DLE_DEBUG_STR_NULL                   46
#define DW_DLE_CU_LENGTH_ERROR                  47
#define DW_DLE_VERSION_STAMP_ERROR              48
#define DW_DLE_ABBREV_OFFSET_ERROR              49
#define DW_DLE_ADDRESS_SIZE_ERROR               50
#define DW_DLE_DEBUG_INFO_PTR_NULL              51
#define DW_DLE_DIE_NULL                         52
#define DW_DLE_STRING_OFFSET_BAD                53
#define DW_DLE_DEBUG_LINE_LENGTH_BAD            54
#define DW_DLE_LINE_PROLOG_LENGTH_BAD           55
#define DW_DLE_LINE_NUM_OPERANDS_BAD            56
#define DW_DLE_LINE_SET_ADDR_ERROR              57 /* No longer used. */
#define DW_DLE_LINE_EXT_OPCODE_BAD              58
#define DW_DLE_DWARF_LINE_NULL                  59
#define DW_DLE_INCL_DIR_NUM_BAD                 60
#define DW_DLE_LINE_FILE_NUM_BAD                61
#define DW_DLE_ALLOC_FAIL                       62
#define DW_DLE_NO_CALLBACK_FUNC                 63
#define DW_DLE_SECT_ALLOC                       64
#define DW_DLE_FILE_ENTRY_ALLOC                 65
#define DW_DLE_LINE_ALLOC                       66
#define DW_DLE_FPGM_ALLOC                       67
#define DW_DLE_INCDIR_ALLOC                     68
#define DW_DLE_STRING_ALLOC                     69
#define DW_DLE_CHUNK_ALLOC                      70
#define DW_DLE_BYTEOFF_ERR                      71
#define DW_DLE_CIE_ALLOC                        72
#define DW_DLE_FDE_ALLOC                        73
#define DW_DLE_REGNO_OVFL                       74
#define DW_DLE_CIE_OFFS_ALLOC                   75
#define DW_DLE_WRONG_ADDRESS                    76
#define DW_DLE_EXTRA_NEIGHBORS                  77
#define DW_DLE_WRONG_TAG                        78
#define DW_DLE_DIE_ALLOC                        79
#define DW_DLE_PARENT_EXISTS                    80
#define DW_DLE_DBG_NULL                         81
#define DW_DLE_DEBUGLINE_ERROR                  82
#define DW_DLE_DEBUGFRAME_ERROR                 83
#define DW_DLE_DEBUGINFO_ERROR                  84
#define DW_DLE_ATTR_ALLOC                       85
#define DW_DLE_ABBREV_ALLOC                     86
#define DW_DLE_OFFSET_UFLW                      87
#define DW_DLE_ELF_SECT_ERR                     88
#define DW_DLE_DEBUG_FRAME_LENGTH_BAD           89
#define DW_DLE_FRAME_VERSION_BAD                90
#define DW_DLE_CIE_RET_ADDR_REG_ERROR           91
#define DW_DLE_FDE_NULL                         92
#define DW_DLE_FDE_DBG_NULL                     93
#define DW_DLE_CIE_NULL                         94
#define DW_DLE_CIE_DBG_NULL                     95
#define DW_DLE_FRAME_TABLE_COL_BAD              96
#define DW_DLE_PC_NOT_IN_FDE_RANGE              97
#define DW_DLE_CIE_INSTR_EXEC_ERROR             98
#define DW_DLE_FRAME_INSTR_EXEC_ERROR           99
#define DW_DLE_FDE_PTR_NULL                    100
#define DW_DLE_RET_OP_LIST_NULL                101
#define DW_DLE_LINE_CONTEXT_NULL               102
#define DW_DLE_DBG_NO_CU_CONTEXT               103
#define DW_DLE_DIE_NO_CU_CONTEXT               104
#define DW_DLE_FIRST_DIE_NOT_CU                105
#define DW_DLE_NEXT_DIE_PTR_NULL               106
#define DW_DLE_DEBUG_FRAME_DUPLICATE           107
#define DW_DLE_DEBUG_FRAME_NULL                108
#define DW_DLE_ABBREV_DECODE_ERROR             109
#define DW_DLE_DWARF_ABBREV_NULL               110
#define DW_DLE_ATTR_NULL                       111
#define DW_DLE_DIE_BAD                         112
#define DW_DLE_DIE_ABBREV_BAD                  113
#define DW_DLE_ATTR_FORM_BAD                   114
#define DW_DLE_ATTR_NO_CU_CONTEXT              115
#define DW_DLE_ATTR_FORM_SIZE_BAD              116
#define DW_DLE_ATTR_DBG_NULL                   117
#define DW_DLE_BAD_REF_FORM                    118
#define DW_DLE_ATTR_FORM_OFFSET_BAD            119
#define DW_DLE_LINE_OFFSET_BAD                 120
#define DW_DLE_DEBUG_STR_OFFSET_BAD            121
#define DW_DLE_STRING_PTR_NULL                 122
#define DW_DLE_PUBNAMES_VERSION_ERROR          123
#define DW_DLE_PUBNAMES_LENGTH_BAD             124
#define DW_DLE_GLOBAL_NULL                     125
#define DW_DLE_GLOBAL_CONTEXT_NULL             126
#define DW_DLE_DIR_INDEX_BAD                   127
#define DW_DLE_LOC_EXPR_BAD                    128
#define DW_DLE_DIE_LOC_EXPR_BAD                129
#define DW_DLE_ADDR_ALLOC                      130
#define DW_DLE_OFFSET_BAD                      131
#define DW_DLE_MAKE_CU_CONTEXT_FAIL            132
#define DW_DLE_REL_ALLOC                       133
#define DW_DLE_ARANGE_OFFSET_BAD               134
#define DW_DLE_SEGMENT_SIZE_BAD                135
#define DW_DLE_ARANGE_LENGTH_BAD               136
#define DW_DLE_ARANGE_DECODE_ERROR             137
#define DW_DLE_ARANGES_NULL                    138
#define DW_DLE_ARANGE_NULL                     139
#define DW_DLE_NO_FILE_NAME                    140
#define DW_DLE_NO_COMP_DIR                     141
#define DW_DLE_CU_ADDRESS_SIZE_BAD             142
#define DW_DLE_INPUT_ATTR_BAD                  143
#define DW_DLE_EXPR_NULL                       144
#define DW_DLE_BAD_EXPR_OPCODE                 145
#define DW_DLE_EXPR_LENGTH_BAD                 146
#define DW_DLE_MULTIPLE_RELOC_IN_EXPR          147
#define DW_DLE_ELF_GETIDENT_ERROR              148
#define DW_DLE_NO_AT_MIPS_FDE                  149
#define DW_DLE_NO_CIE_FOR_FDE                  150
#define DW_DLE_DIE_ABBREV_LIST_NULL            151
#define DW_DLE_DEBUG_FUNCNAMES_DUPLICATE       152
#define DW_DLE_DEBUG_FUNCNAMES_NULL            153
#define DW_DLE_DEBUG_FUNCNAMES_VERSION_ERROR   154
#define DW_DLE_DEBUG_FUNCNAMES_LENGTH_BAD      155
#define DW_DLE_FUNC_NULL                       156
#define DW_DLE_FUNC_CONTEXT_NULL               157
#define DW_DLE_DEBUG_TYPENAMES_DUPLICATE       158
#define DW_DLE_DEBUG_TYPENAMES_NULL            159
#define DW_DLE_DEBUG_TYPENAMES_VERSION_ERROR   160
#define DW_DLE_DEBUG_TYPENAMES_LENGTH_BAD      161
#define DW_DLE_TYPE_NULL                       162
#define DW_DLE_TYPE_CONTEXT_NULL               163
#define DW_DLE_DEBUG_VARNAMES_DUPLICATE        164
#define DW_DLE_DEBUG_VARNAMES_NULL             165
#define DW_DLE_DEBUG_VARNAMES_VERSION_ERROR    166
#define DW_DLE_DEBUG_VARNAMES_LENGTH_BAD       167
#define DW_DLE_VAR_NULL                        168
#define DW_DLE_VAR_CONTEXT_NULL                169
#define DW_DLE_DEBUG_WEAKNAMES_DUPLICATE       170
#define DW_DLE_DEBUG_WEAKNAMES_NULL            171
#define DW_DLE_DEBUG_WEAKNAMES_VERSION_ERROR   172
#define DW_DLE_DEBUG_WEAKNAMES_LENGTH_BAD      173
#define DW_DLE_WEAK_NULL                       174
#define DW_DLE_WEAK_CONTEXT_NULL               175
#define DW_DLE_LOCDESC_COUNT_WRONG             176
#define DW_DLE_MACINFO_STRING_NULL             177
#define DW_DLE_MACINFO_STRING_EMPTY            178
#define DW_DLE_MACINFO_INTERNAL_ERROR_SPACE    179
#define DW_DLE_MACINFO_MALLOC_FAIL             180
#define DW_DLE_DEBUGMACINFO_ERROR              181
#define DW_DLE_DEBUG_MACRO_LENGTH_BAD          182
#define DW_DLE_DEBUG_MACRO_MAX_BAD             183
#define DW_DLE_DEBUG_MACRO_INTERNAL_ERR        184
#define DW_DLE_DEBUG_MACRO_MALLOC_SPACE        185
#define DW_DLE_DEBUG_MACRO_INCONSISTENT        186
#define DW_DLE_DF_NO_CIE_AUGMENTATION          187
#define DW_DLE_DF_REG_NUM_TOO_HIGH             188
#define DW_DLE_DF_MAKE_INSTR_NO_INIT           189
#define DW_DLE_DF_NEW_LOC_LESS_OLD_LOC         190
#define DW_DLE_DF_POP_EMPTY_STACK              191
#define DW_DLE_DF_ALLOC_FAIL                   192
#define DW_DLE_DF_FRAME_DECODING_ERROR         193
#define DW_DLE_DEBUG_LOC_SECTION_SHORT         194
#define DW_DLE_FRAME_AUGMENTATION_UNKNOWN      195
#define DW_DLE_PUBTYPE_CONTEXT                 196 /* Unused. */
#define DW_DLE_DEBUG_PUBTYPES_LENGTH_BAD       197
#define DW_DLE_DEBUG_PUBTYPES_VERSION_ERROR    198
#define DW_DLE_DEBUG_PUBTYPES_DUPLICATE        199
#define DW_DLE_FRAME_CIE_DECODE_ERROR          200
#define DW_DLE_FRAME_REGISTER_UNREPRESENTABLE  201
#define DW_DLE_FRAME_REGISTER_COUNT_MISMATCH   202
#define DW_DLE_LINK_LOOP                       203
#define DW_DLE_STRP_OFFSET_BAD                 204
#define DW_DLE_DEBUG_RANGES_DUPLICATE          205
#define DW_DLE_DEBUG_RANGES_OFFSET_BAD         206
#define DW_DLE_DEBUG_RANGES_MISSING_END        207
#define DW_DLE_DEBUG_RANGES_OUT_OF_MEM         208
#define DW_DLE_DEBUG_SYMTAB_ERR                209
#define DW_DLE_DEBUG_STRTAB_ERR                210
#define DW_DLE_RELOC_MISMATCH_INDEX            211
#define DW_DLE_RELOC_MISMATCH_RELOC_INDEX      212
#define DW_DLE_RELOC_MISMATCH_STRTAB_INDEX     213
#define DW_DLE_RELOC_SECTION_MISMATCH          214
#define DW_DLE_RELOC_SECTION_MISSING_INDEX     215
#define DW_DLE_RELOC_SECTION_LENGTH_ODD        216
#define DW_DLE_RELOC_SECTION_PTR_NULL          217
#define DW_DLE_RELOC_SECTION_MALLOC_FAIL       218
#define DW_DLE_NO_ELF64_SUPPORT                219
#define DW_DLE_MISSING_ELF64_SUPPORT           220
#define DW_DLE_ORPHAN_FDE                      221
#define DW_DLE_DUPLICATE_INST_BLOCK            222
#define DW_DLE_BAD_REF_SIG8_FORM               223
#define DW_DLE_ATTR_EXPRLOC_FORM_BAD           224
#define DW_DLE_FORM_SEC_OFFSET_LENGTH_BAD      225
#define DW_DLE_NOT_REF_FORM                    226
#define DW_DLE_DEBUG_FRAME_LENGTH_NOT_MULTIPLE 227
#define DW_DLE_REF_SIG8_NOT_HANDLED            228
#define DW_DLE_DEBUG_FRAME_POSSIBLE_ADDRESS_BOTCH 229
#define DW_DLE_LOC_BAD_TERMINATION             230
#define DW_DLE_SYMTAB_SECTION_LENGTH_ODD       231
#define DW_DLE_RELOC_SECTION_SYMBOL_INDEX_BAD  232
#define DW_DLE_RELOC_SECTION_RELOC_TARGET_SIZE_UNKNOWN  233
#define DW_DLE_SYMTAB_SECTION_ENTRYSIZE_ZERO   234
#define DW_DLE_LINE_NUMBER_HEADER_ERROR        235
#define DW_DLE_DEBUG_TYPES_NULL                236
#define DW_DLE_DEBUG_TYPES_DUPLICATE           237
#define DW_DLE_DEBUG_TYPES_ONLY_DWARF4         238
#define DW_DLE_DEBUG_TYPEOFFSET_BAD            239
#define DW_DLE_GNU_OPCODE_ERROR                240
#define DW_DLE_DEBUGPUBTYPES_ERROR             241
#define DW_DLE_AT_FIXUP_NULL                   242
#define DW_DLE_AT_FIXUP_DUP                    243
#define DW_DLE_BAD_ABINAME                     244
#define DW_DLE_TOO_MANY_DEBUG                  245
#define DW_DLE_DEBUG_STR_OFFSETS_DUPLICATE     246
#define DW_DLE_SECTION_DUPLICATION             247
#define DW_DLE_SECTION_ERROR                   248
#define DW_DLE_DEBUG_ADDR_DUPLICATE            249
#define DW_DLE_DEBUG_CU_UNAVAILABLE_FOR_FORM   250
#define DW_DLE_DEBUG_FORM_HANDLING_INCOMPLETE  251
#define DW_DLE_NEXT_DIE_PAST_END               252
#define DW_DLE_NEXT_DIE_WRONG_FORM             253
#define DW_DLE_NEXT_DIE_NO_ABBREV_LIST         254
#define DW_DLE_NESTED_FORM_INDIRECT_ERROR      255
#define DW_DLE_CU_DIE_NO_ABBREV_LIST           256
#define DW_DLE_MISSING_NEEDED_DEBUG_ADDR_SECTION 257
#define DW_DLE_ATTR_FORM_NOT_ADDR_INDEX        258
#define DW_DLE_ATTR_FORM_NOT_STR_INDEX         259
#define DW_DLE_DUPLICATE_GDB_INDEX             260
#define DW_DLE_ERRONEOUS_GDB_INDEX_SECTION     261
#define DW_DLE_GDB_INDEX_COUNT_ERROR           262
#define DW_DLE_GDB_INDEX_COUNT_ADDR_ERROR      263
#define DW_DLE_GDB_INDEX_INDEX_ERROR           264
#define DW_DLE_GDB_INDEX_CUVEC_ERROR           265
#define DW_DLE_DUPLICATE_CU_INDEX              266
#define DW_DLE_DUPLICATE_TU_INDEX              267
#define DW_DLE_XU_TYPE_ARG_ERROR               268
#define DW_DLE_XU_IMPOSSIBLE_ERROR             269
#define DW_DLE_XU_NAME_COL_ERROR               270
#define DW_DLE_XU_HASH_ROW_ERROR               271
#define DW_DLE_XU_HASH_INDEX_ERROR             272
/* ..._FAILSAFE_ERRVAL is an aid when out of memory. */
#define DW_DLE_FAILSAFE_ERRVAL                 273
#define DW_DLE_ARANGE_ERROR                    274
#define DW_DLE_PUBNAMES_ERROR                  275
#define DW_DLE_FUNCNAMES_ERROR                 276
#define DW_DLE_TYPENAMES_ERROR                 277
#define DW_DLE_VARNAMES_ERROR                  278
#define DW_DLE_WEAKNAMES_ERROR                 279
#define DW_DLE_RELOCS_ERROR                    280
#define DW_DLE_ATTR_OUTSIDE_SECTION            281
#define DW_DLE_FISSION_INDEX_WRONG             282
#define DW_DLE_FISSION_VERSION_ERROR           283
#define DW_DLE_NEXT_DIE_LOW_ERROR              284
#define DW_DLE_CU_UT_TYPE_ERROR                285
#define DW_DLE_NO_SUCH_SIGNATURE_FOUND         286
#define DW_DLE_SIGNATURE_SECTION_NUMBER_WRONG  287
#define DW_DLE_ATTR_FORM_NOT_DATA8             288
#define DW_DLE_SIG_TYPE_WRONG_STRING           289
#define DW_DLE_MISSING_REQUIRED_TU_OFFSET_HASH 290
#define DW_DLE_MISSING_REQUIRED_CU_OFFSET_HASH 291
#define DW_DLE_DWP_MISSING_DWO_ID              292
#define DW_DLE_DWP_SIBLING_ERROR               293
#define DW_DLE_DEBUG_FISSION_INCOMPLETE        294
#define DW_DLE_FISSION_SECNUM_ERR              295
#define DW_DLE_DEBUG_MACRO_DUPLICATE           296
#define DW_DLE_DEBUG_NAMES_DUPLICATE           297
#define DW_DLE_DEBUG_LINE_STR_DUPLICATE        298
#define DW_DLE_DEBUG_SUP_DUPLICATE             299
#define DW_DLE_NO_SIGNATURE_TO_LOOKUP          300
#define DW_DLE_NO_TIED_ADDR_AVAILABLE          301
#define DW_DLE_NO_TIED_SIG_AVAILABLE           302
#define DW_DLE_STRING_NOT_TERMINATED           303
#define DW_DLE_BAD_LINE_TABLE_OPERATION        304
#define DW_DLE_LINE_CONTEXT_BOTCH              305
#define DW_DLE_LINE_CONTEXT_INDEX_WRONG        306
#define DW_DLE_NO_TIED_STRING_AVAILABLE        307
#define DW_DLE_NO_TIED_FILE_AVAILABLE          308
#define DW_DLE_CU_TYPE_MISSING                 309
#define DW_DLE_LLE_CODE_UNKNOWN                310
#define DW_DLE_LOCLIST_INTERFACE_ERROR         311
#define DW_DLE_LOCLIST_INDEX_ERROR             312
#define DW_DLE_INTERFACE_NOT_SUPPORTED         313
#define DW_DLE_ZDEBUG_REQUIRES_ZLIB            314
#define DW_DLE_ZDEBUG_INPUT_FORMAT_ODD         315
#define DW_DLE_ZLIB_BUF_ERROR                  316
#define DW_DLE_ZLIB_DATA_ERROR                 317
#define DW_DLE_MACRO_OFFSET_BAD                318
#define DW_DLE_MACRO_OPCODE_BAD                319
#define DW_DLE_MACRO_OPCODE_FORM_BAD           320
#define DW_DLE_UNKNOWN_FORM                    321
#define DW_DLE_BAD_MACRO_HEADER_POINTER        322
#define DW_DLE_BAD_MACRO_INDEX                 323
#define DW_DLE_MACRO_OP_UNHANDLED              324
#define DW_DLE_MACRO_PAST_END                  325
#define DW_DLE_LINE_STRP_OFFSET_BAD            326
#define DW_DLE_STRING_FORM_IMPROPER            327
#define DW_DLE_ELF_FLAGS_NOT_AVAILABLE         328
#define DW_DLE_LEB_IMPROPER                    329
#define DW_DLE_DEBUG_LINE_RANGE_ZERO           330
#define DW_DLE_READ_LITTLEENDIAN_ERROR         331
#define DW_DLE_READ_BIGENDIAN_ERROR            332
#define DW_DLE_RELOC_INVALID                   333
#define DW_DLE_INFO_HEADER_ERROR               334
#define DW_DLE_ARANGES_HEADER_ERROR            335
#define DW_DLE_LINE_OFFSET_WRONG_FORM          336
#define DW_DLE_FORM_BLOCK_LENGTH_ERROR         337
#define DW_DLE_ZLIB_SECTION_SHORT              338
#define DW_DLE_CIE_INSTR_PTR_ERROR             339
#define DW_DLE_FDE_INSTR_PTR_ERROR             340
#define DW_DLE_FISSION_ADDITION_ERROR          341
#define DW_DLE_HEADER_LEN_BIGGER_THAN_SECSIZE  342
#define DW_DLE_LOCEXPR_OFF_SECTION_END         343
#define DW_DLE_POINTER_SECTION_UNKNOWN         344
#define DW_DLE_ERRONEOUS_XU_INDEX_SECTION      345
#define DW_DLE_DIRECTORY_FORMAT_COUNT_VS_DIRECTORIES_MISMATCH 346
#define DW_DLE_COMPRESSED_EMPTY_SECTION        347
#define DW_DLE_SIZE_WRAPAROUND                 348
#define DW_DLE_ILLOGICAL_TSEARCH               349
#define DW_DLE_BAD_STRING_FORM                 350
#define DW_DLE_DEBUGSTR_ERROR                  351
#define DW_DLE_DEBUGSTR_UNEXPECTED_REL         352
#define DW_DLE_DISCR_ARRAY_ERROR               353
#define DW_DLE_LEB_OUT_ERROR                   354
#define DW_DLE_SIBLING_LIST_IMPROPER           355
#define DW_DLE_LOCLIST_OFFSET_BAD              356
#define DW_DLE_LINE_TABLE_BAD                  357
#define DW_DLE_DEBUG_LOClISTS_DUPLICATE        358
#define DW_DLE_DEBUG_RNGLISTS_DUPLICATE        359
#define DW_DLE_ABBREV_OFF_END                  360
#define DW_DLE_FORM_STRING_BAD_STRING          361
#define DW_DLE_AUGMENTATION_STRING_OFF_END     362
#define DW_DLE_STRING_OFF_END_PUBNAMES_LIKE    363
#define DW_DLE_LINE_STRING_BAD                 364
#define DW_DLE_DEFINE_FILE_STRING_BAD          365
#define DW_DLE_MACRO_STRING_BAD                366
#define DW_DLE_MACINFO_STRING_BAD              367
#define DW_DLE_ZLIB_UNCOMPRESS_ERROR           368
#define DW_DLE_IMPROPER_DWO_ID                 369
#define DW_DLE_GROUPNUMBER_ERROR               370
#define DW_DLE_ADDRESS_SIZE_ZERO               371
#define DW_DLE_DEBUG_NAMES_HEADER_ERROR        372
#define DW_DLE_DEBUG_NAMES_AUG_STRING_ERROR    373
#define DW_DLE_DEBUG_NAMES_PAD_NON_ZERO        374
#define DW_DLE_DEBUG_NAMES_OFF_END             375
#define DW_DLE_DEBUG_NAMES_ABBREV_OVERFLOW     376
#define DW_DLE_DEBUG_NAMES_ABBREV_CORRUPTION   377
#define DW_DLE_DEBUG_NAMES_NULL_POINTER        378
#define DW_DLE_DEBUG_NAMES_BAD_INDEX_ARG       379
#define DW_DLE_DEBUG_NAMES_ENTRYPOOL_OFFSET    380
#define DW_DLE_DEBUG_NAMES_UNHANDLED_FORM      381
#define DW_DLE_LNCT_CODE_UNKNOWN               382
#define DW_DLE_LNCT_FORM_CODE_NOT_HANDLED      383
#define DW_DLE_LINE_HEADER_LENGTH_BOTCH        384
#define DW_DLE_STRING_HASHTAB_IDENTITY_ERROR   385
#define DW_DLE_UNIT_TYPE_NOT_HANDLED           386
#define DW_DLE_GROUP_MAP_ALLOC                 387
#define DW_DLE_GROUP_MAP_DUPLICATE             388
#define DW_DLE_GROUP_COUNT_ERROR               389
#define DW_DLE_GROUP_INTERNAL_ERROR            390
#define DW_DLE_GROUP_LOAD_ERROR                391
#define DW_DLE_GROUP_LOAD_READ_ERROR           392
#define DW_DLE_AUG_DATA_LENGTH_BAD             393
#define DW_DLE_ABBREV_MISSING                  394
#define DW_DLE_NO_TAG_FOR_DIE                  395
#define DW_DLE_LOWPC_WRONG_CLASS               396
#define DW_DLE_HIGHPC_WRONG_FORM               397
#define DW_DLE_STR_OFFSETS_BASE_WRONG_FORM     398
#define DW_DLE_DATA16_OUTSIDE_SECTION          399
#define DW_DLE_LNCT_MD5_WRONG_FORM             400
#define DW_DLE_LINE_HEADER_CORRUPT             401
#define DW_DLE_STR_OFFSETS_NULLARGUMENT        402
#define DW_DLE_STR_OFFSETS_NULL_DBG            403
#define DW_DLE_STR_OFFSETS_NO_MAGIC            404
#define DW_DLE_STR_OFFSETS_ARRAY_SIZE          405
#define DW_DLE_STR_OFFSETS_VERSION_WRONG       406
#define DW_DLE_STR_OFFSETS_ARRAY_INDEX_WRONG   407
#define DW_DLE_STR_OFFSETS_EXTRA_BYTES         408
#define DW_DLE_DUP_ATTR_ON_DIE                 409
#define DW_DLE_SECTION_NAME_BIG                410

    /* LAST MUST EQUAL LAST ERROR NUMBER */
#define DW_DLE_LAST        410

#define DW_DLE_LO_USER     0x10000

    /*  Taken as meaning 'undefined value', this is not
        a column or register number.
        Only present at libdwarf runtime. Never on disk.
        DW_FRAME_* Values present on disk are in dwarf.h
    */
#define DW_FRAME_UNDEFINED_VAL          1034

    /*  Taken as meaning 'same value' as caller had, not a column
        or register number
        Only present at libdwarf runtime. Never on disk.
        DW_FRAME_* Values present on disk are in dwarf.h
    */
#define DW_FRAME_SAME_VAL               1035



/* error return values
*/
#define DW_DLV_BADADDR     (~(Dwarf_Addr)0)
    /* for functions returning target address */

#define DW_DLV_NOCOUNT     ((Dwarf_Signed)-1)
    /* for functions returning count */

#define DW_DLV_BADOFFSET   (~(Dwarf_Off)0)
    /* for functions returning offset */

/* standard return values for functions */
#define DW_DLV_NO_ENTRY -1
#define DW_DLV_OK        0
#define DW_DLV_ERROR     1

/* Special values for offset_into_exception_table field of dwarf fde's. */
/* The following value indicates that there is no Exception table offset
   associated with a dwarf frame. */
#define DW_DLX_NO_EH_OFFSET         (-1LL)
/* The following value indicates that the producer was unable to analyse the
   source file to generate Exception tables for this function. */
#define DW_DLX_EH_OFFSET_UNAVAILABLE  (-2LL)

/* The dwarf specification separates FORMs into
different classes.  To do the seperation properly
requires 4 pieces of data as of DWARF4 (thus the
function arguments listed here).
The DWARF4 specification class definition suffices to
describe all DWARF versions.
See section 7.5.4, Attribute Encodings.
A return of DW_FORM_CLASS_UNKNOWN means we could not properly figure
out what form-class it is.

    DW_FORM_CLASS_FRAMEPTR is MIPS/IRIX only, and refers
    to the DW_AT_MIPS_fde attribute (a reference to the
    .debug_frame section).

    DWARF5:
    DW_FORM_CLASS_LOCLISTSPTR  is like DW_FORM_CLASS_LOCLIST
    except that LOCLISTSPTR is aways a section offset,
    never an index, and LOCLISTSPTR is only referenced
    by DW_AT_loclists_base.
    Note DW_FORM_CLASS_LOCLISTSPTR spelling to distinguish
    from DW_FORM_CLASS_LOCLISTPTR.

    DWARF5:
    DW_FORM_CLASS_RNGLISTSPTR  is like DW_FORM_CLASS_RNGLIST
    except that RNGLISTSPTR is aways a section offset,
    never an index. DW_FORM_CLASS_RNGLISTSPTR is only
    referenced by DW_AT_rnglists_base.
*/
enum Dwarf_Form_Class {
    DW_FORM_CLASS_UNKNOWN,   DW_FORM_CLASS_ADDRESS,
    DW_FORM_CLASS_BLOCK,     DW_FORM_CLASS_CONSTANT,
    DW_FORM_CLASS_EXPRLOC,   DW_FORM_CLASS_FLAG,
    DW_FORM_CLASS_LINEPTR,
    DW_FORM_CLASS_LOCLISTPTR,   /* DWARF2,3,4 only */
    DW_FORM_CLASS_MACPTR,       /* DWARF2,3,4 only */
    DW_FORM_CLASS_RANGELISTPTR, /* DWARF2,3,4 only */
    DW_FORM_CLASS_REFERENCE,
    DW_FORM_CLASS_STRING,
    DW_FORM_CLASS_FRAMEPTR,      /* MIPS/IRIX DWARF2 only */
    DW_FORM_CLASS_MACROPTR,      /* DWARF5 */
    DW_FORM_CLASS_ADDRPTR,       /* DWARF5 */
    DW_FORM_CLASS_LOCLIST,       /* DWARF5 */
    DW_FORM_CLASS_LOCLISTSPTR,   /* DWARF5 */
    DW_FORM_CLASS_RNGLIST,       /* DWARF5 */
    DW_FORM_CLASS_RNGLISTSPTR,   /* DWARF5 */
    DW_FORM_CLASS_STROFFSETSPTR  /* DWARF5 */
};

/*  These support opening DWARF5 split dwarf objects. */
#define DW_GROUPNUMBER_ANY  0
#define DW_GROUPNUMBER_BASE 1
#define DW_GROUPNUMBER_DWO  2

/*===========================================================================*/
/*  Dwarf consumer interface initialization and termination operations */

/*  Initialization based on Unix open fd (using libelf internally). */
/*  New March 2017 */
int dwarf_init_b(int    /*fd*/,
    Dwarf_Unsigned    /*access*/,
    unsigned          /*groupnumber*/,
    Dwarf_Handler     /*errhand*/,
    Dwarf_Ptr         /*errarg*/,
    Dwarf_Debug*      /*dbg*/,
    Dwarf_Error*      /*error*/);

int dwarf_init(int    /*fd*/,
    Dwarf_Unsigned    /*access*/,
    Dwarf_Handler     /*errhand*/,
    Dwarf_Ptr         /*errarg*/,
    Dwarf_Debug*      /*dbg*/,
    Dwarf_Error*      /*error*/);

/*  Initialization based on libelf/sgi-fastlibelf open pointer. */
/*  New March 2017 */
int dwarf_elf_init_b(dwarf_elf_handle /*elf*/,
    Dwarf_Unsigned    /*access*/,
    unsigned          /*group_number*/,
    Dwarf_Handler     /*errhand*/,
    Dwarf_Ptr         /*errarg*/,
    Dwarf_Debug*      /*dbg*/,
    Dwarf_Error*      /*error*/);
int dwarf_elf_init(dwarf_elf_handle /*elf*/,
    Dwarf_Unsigned    /*access*/,
    Dwarf_Handler     /*errhand*/,
    Dwarf_Ptr         /*errarg*/,
    Dwarf_Debug*      /*dbg*/,
    Dwarf_Error*      /*error*/);

/* Undocumented function for memory allocator. */
void dwarf_print_memory_stats(Dwarf_Debug  /*dbg*/);

int dwarf_get_elf(Dwarf_Debug /*dbg*/,
    dwarf_elf_handle* /*return_elfptr*/,
    Dwarf_Error*      /*error*/);

int dwarf_finish(Dwarf_Debug /*dbg*/, Dwarf_Error* /*error*/);


/*  NEW March 2017. */
int dwarf_object_init_b(Dwarf_Obj_Access_Interface* /*obj*/,
    Dwarf_Handler /*errhand*/,
    Dwarf_Ptr     /*errarg*/,
    unsigned      /*groupnumber*/,
    Dwarf_Debug*  /*dbg*/,
    Dwarf_Error*  /*error*/);

int dwarf_object_init(Dwarf_Obj_Access_Interface* /*obj*/,
    Dwarf_Handler /*errhand*/,
    Dwarf_Ptr     /*errarg*/,
    Dwarf_Debug*  /*dbg*/,
    Dwarf_Error*  /*error*/);

int dwarf_set_tied_dbg(Dwarf_Debug /*basedbg*/,
    Dwarf_Debug /*tied_dbg*/,
    Dwarf_Error*  /*error*/);

/*  Likely not very useful.? */
int dwarf_get_tied_dbg(Dwarf_Debug /*dbg*/,
    Dwarf_Debug * /*tieddbg_out*/,
    Dwarf_Error * /*error*/);

int dwarf_object_finish(Dwarf_Debug /*dbg*/,
    Dwarf_Error* /*error*/);

/*  Section name access.  Because sections might
    now end with .dwo or be .zdebug  or might not.
*/
int
dwarf_get_die_section_name(Dwarf_Debug /*dbg*/,
    Dwarf_Bool    /*is_info*/,
    const char ** /*sec_name*/,
    Dwarf_Error * /*error*/);

int
dwarf_get_die_section_name_b(Dwarf_Die /*die*/,
    const char ** /*sec_name*/,
    Dwarf_Error * /*error*/);

int
dwarf_get_real_section_name(Dwarf_Debug /*dbg*/,
    const char * /*std_section_name*/,
    const char ** /*actual_sec_name_out*/,
    Dwarf_Small * /*marked_compressed*/,  /* .zdebug... */
    Dwarf_Small * /*marked_zlib_compressed */, /* ZLIB string */
    Dwarf_Small * /*marked_shf_compressed*/, /* SHF_COMPRESSED */
    Dwarf_Unsigned * /*compressed_length*/,
    Dwarf_Unsigned * /*uncompressed_length*/,
    Dwarf_Error * /*error*/);



/*  dwarf_next_cu_header_d() traverses debug_types CU headers.
    New in May, 2015.
    */
int dwarf_next_cu_header_d(Dwarf_Debug /*dbg*/,
    Dwarf_Bool      /*is_info*/,
    Dwarf_Unsigned* /*cu_header_length*/,
    Dwarf_Half*     /*version_stamp*/,
    Dwarf_Off*      /*abbrev_offset*/,
    Dwarf_Half*     /*address_size*/,
    Dwarf_Half*     /*length_size*/,
    Dwarf_Half*     /*extension_size*/,
    Dwarf_Sig8*     /*type signature*/,
    Dwarf_Unsigned* /*typeoffset*/,
    Dwarf_Unsigned* /*next_cu_header_offset*/,
    Dwarf_Half    * /*header_cu_type*/,
    Dwarf_Error*    /*error*/);

/*  Die traversal operations.
    dwarf_next_cu_header_b() traverses debug_info CU headers.
    Obsolete but supported.
    */
int dwarf_next_cu_header_b(Dwarf_Debug /*dbg*/,
    Dwarf_Unsigned* /*cu_header_length*/,
    Dwarf_Half*     /*version_stamp*/,
    Dwarf_Off*      /*abbrev_offset*/,
    Dwarf_Half*     /*address_size*/,
    Dwarf_Half*     /*length_size*/,
    Dwarf_Half*     /*extension_size*/,
    Dwarf_Unsigned* /*next_cu_header_offset*/,
    Dwarf_Error*    /*error*/);

/*  dwarf_next_cu_header_types() traverses debug_types CU headers.
    New in October, 2011. Obsolete but supported May 2015.
    */
int dwarf_next_cu_header_c(Dwarf_Debug /*dbg*/,
    Dwarf_Bool      /*is_info*/,
    Dwarf_Unsigned* /*cu_header_length*/,
    Dwarf_Half*     /*version_stamp*/,
    Dwarf_Off*      /*abbrev_offset*/,
    Dwarf_Half*     /*address_size*/,
    Dwarf_Half*     /*length_size*/,
    Dwarf_Half*     /*extension_size*/,
    Dwarf_Sig8*     /*type signature*/,
    Dwarf_Unsigned* /*typeoffset*/,
    Dwarf_Unsigned* /*next_cu_header_offset*/,
    Dwarf_Error*    /*error*/);
/* The following is obsolete, though supported. November 2009. */
int dwarf_next_cu_header(Dwarf_Debug /*dbg*/,
    Dwarf_Unsigned* /*cu_header_length*/,
    Dwarf_Half*     /*version_stamp*/,
    Dwarf_Off*      /*abbrev_offset*/,
    Dwarf_Half*     /*address_size*/,
    Dwarf_Unsigned* /*next_cu_header_offset*/,
    Dwarf_Error*    /*error*/);

int dwarf_siblingof(Dwarf_Debug /*dbg*/,
    Dwarf_Die        /*die*/,
    Dwarf_Die*       /*return_siblingdie*/,
    Dwarf_Error*     /*error*/);
/* dwarf_siblingof_b new October 2011. */
int dwarf_siblingof_b(Dwarf_Debug /*dbg*/,
    Dwarf_Die        /*die*/,
    Dwarf_Bool       /*is_info*/,
    Dwarf_Die*       /*return_siblingdie*/,
    Dwarf_Error*     /*error*/);

/* New 27 April 2015. */
int dwarf_die_from_hash_signature(Dwarf_Debug /*dbg*/,
    Dwarf_Sig8 *     /*hash_sig*/,
    const char *     /*sig_type: "tu" or "cu"*/,
    Dwarf_Die*       /*returned_CU_die */,
    Dwarf_Error*     /*error*/);

int dwarf_child(Dwarf_Die /*die*/,
    Dwarf_Die*       /*return_childdie*/,
    Dwarf_Error*     /*error*/);

/*  Finding die given global (not CU-relative) offset.
    Applies only to debug_info. */
int dwarf_offdie(Dwarf_Debug /*dbg*/,
    Dwarf_Off        /*offset*/,
    Dwarf_Die*       /*return_die*/,
    Dwarf_Error*     /*error*/);

/*  dwarf_offdie_b() new October 2011 */
/*  Finding die given global (not CU-relative) offset.
    Applies to debug_info (is_info true) or debug_types (is_info false). */
int dwarf_offdie_b(Dwarf_Debug /*dbg*/,
    Dwarf_Off        /*offset*/,
    Dwarf_Bool       /*is_info*/,
    Dwarf_Die*       /*return_die*/,
    Dwarf_Error*     /*error*/);

/*  Returns the is_info flag through the pointer if the function returns
    DW_DLV_OK. Needed so client software knows if a DIE is in debug_info
    or debug_types.
    New October 2011. */
Dwarf_Bool dwarf_get_die_infotypes_flag(Dwarf_Die /*die*/);

/*  New March 2016.
    So we can associate a DIE's abbreviations with the contents
    the abbreviations section. */
int dwarf_die_abbrev_global_offset(Dwarf_Die /*die*/,
    Dwarf_Off       * /*abbrev_offset*/,
    Dwarf_Unsigned  * /*abbrev_count*/,
    Dwarf_Error*      /*error*/);

/*  operations on DIEs */
int dwarf_tag(Dwarf_Die /*die*/,
    Dwarf_Half*      /*return_tag*/,
    Dwarf_Error*     /*error*/);

/*  dwarf_dieoffset returns the global debug_info
    section offset, not the CU relative offset. */
int dwarf_dieoffset(Dwarf_Die /*die*/,
    Dwarf_Off*       /*return_offset*/,
    Dwarf_Error*     /*error*/);

/*  NEW October 2015. DWARF5.
    The DIE here can be any DIE in the relevant CU.
    index is an index into .debug_addr.
    This will look first for .debug_addr
    in the dbg object DIE
    and if not there (because the dbg object is
    a dwo or dwp split dwarf object)
    will look in the tied object if tied is available. */
int
dwarf_debug_addr_index_to_addr(Dwarf_Die /*die*/,
    Dwarf_Unsigned  /*index*/,
    Dwarf_Addr    * /*return_addr*/,
    Dwarf_Error   * /*error*/);


/*  dwarf_CU_dieoffset_given_die returns
    the global debug_info section offset of the CU die
    that is the CU containing the given_die
    (the passed in DIE can be any DIE).
    This information makes it possible for a consumer to
    find and print CU context information for any die.
    See also dwarf_get_cu_die_offset_given_cu_header_offset(). */
int dwarf_CU_dieoffset_given_die(Dwarf_Die /*given_die*/,
    Dwarf_Off*       /*return_offset*/,
    Dwarf_Error*     /*error*/);

/*  dwarf_die_CU_offset returns the CU relative offset
    not the global debug_info section offset, given
    any DIE in the CU.  See also dwarf_CU_dieoffset_given_die().
    */
int dwarf_die_CU_offset(Dwarf_Die /*die*/,
    Dwarf_Off*       /*return_offset*/,
    Dwarf_Error*     /*error*/);

int dwarf_die_CU_offset_range(Dwarf_Die /*die*/,
    Dwarf_Off*       /*return_CU_header_offset*/,
    Dwarf_Off*       /*return_CU_length_bytes*/,
    Dwarf_Error*     /*error*/);

int dwarf_attr (Dwarf_Die /*die*/,
    Dwarf_Half        /*attr*/,
    Dwarf_Attribute * /*returned_attr*/,
    Dwarf_Error*      /*error*/);

int dwarf_die_text(Dwarf_Die /*die*/,
    Dwarf_Half    /*attr*/,
    char       ** /*ret_name*/,
    Dwarf_Error * /*error*/);

int dwarf_diename(Dwarf_Die /*die*/,
    char   **        /*diename*/,
    Dwarf_Error*     /*error*/);

/* Returns the  abbrev code of the die. Cannot fail. */
int dwarf_die_abbrev_code(Dwarf_Die /*die */);

/*  Returns a flag through ab_has_child. Non-zero if
    the DIE has children, zero if it does not.   */
int dwarf_die_abbrev_children_flag(Dwarf_Die /*die*/,
    Dwarf_Half * /*ab_has_child*/);

/* Validate the sibling DIE. This only makes sense to call
   if the sibling's DIEs have been travsersed and
   dwarf_child() called on each,
   so that the last DIE dwarf_child saw was the last.
   Essentially ensuring that (after such traversal) that we
   are in the same place a sibling attribute would identify.
   In case we return DW_DLV_ERROR, the global offset of the last
   DIE traversed by dwarf_child is returned through *offset */
int dwarf_validate_die_sibling(Dwarf_Die /*sibling*/,Dwarf_Off* /*offset*/);

/* convenience functions, alternative to using dwarf_attrlist() */
int dwarf_hasattr(Dwarf_Die /*die*/,
    Dwarf_Half   /*attr*/,
    Dwarf_Bool * /*returned_bool*/,
    Dwarf_Error* /*error*/);

/* Returns the children offsets for the given offset */
int dwarf_offset_list(Dwarf_Debug /*dbg*/,
    Dwarf_Off         /*offset*/,
    Dwarf_Bool        /*is_info*/,
    Dwarf_Off      ** /*offbuf*/,
    Dwarf_Unsigned *  /*offcnt*/,
    Dwarf_Error    *  /*error*/);

/*  BEGIN: loclist_c interfaces
    NEW October 2015.
    This works for any attribute that identifies
    a loclist or a locexpr. When the attribute is a locexpr
    a single loclist (created by libdwarf)
    is attached to loclist_head. */
int dwarf_get_loclist_c (Dwarf_Attribute /*attr*/,
   Dwarf_Loc_Head_c * /*loclist_head*/,
   Dwarf_Unsigned   * /*locCount*/,
   Dwarf_Error      * /*error*/);

int dwarf_get_locdesc_entry_c(Dwarf_Loc_Head_c /*loclist_head*/,
   Dwarf_Unsigned    /*index*/,

   /* identifies type of locdesc entry*/
   Dwarf_Small    *  /*lle_value_out*/,
   Dwarf_Addr     *  /*lowpc_out*/,
   Dwarf_Addr     *  /*hipc_out*/,
   Dwarf_Unsigned *  /*loclist_count_out*/,
   Dwarf_Locdesc_c * /*locentry_out*/,
   Dwarf_Small    *  /*loclist_source_out*/, /* 0,1, or 2 */
   Dwarf_Unsigned *  /*expression_offset_out*/,
   Dwarf_Unsigned *  /*locdesc_offset_out*/,
   Dwarf_Error    *  /*error*/);

int dwarf_get_location_op_value_c(Dwarf_Locdesc_c /*locdesc*/,
   Dwarf_Unsigned   /*index*/,
   Dwarf_Small    * /*atom_out*/,
   Dwarf_Unsigned * /*operand1*/,
   Dwarf_Unsigned * /*operand2*/,
   Dwarf_Unsigned * /*operand3*/,
   Dwarf_Unsigned * /*offset_for_branch*/,
   Dwarf_Error*     /*error*/);

int dwarf_loclist_from_expr_c(Dwarf_Debug /*dbg*/,
    Dwarf_Ptr      /*expression_in*/,
    Dwarf_Unsigned /*expression_length*/,
    Dwarf_Half     /*address_size*/,
    Dwarf_Half     /*offset_size*/,
    Dwarf_Small    /*dwarf_version*/,
    Dwarf_Loc_Head_c* /*loc_head*/,
    Dwarf_Unsigned  * /*listlen*/,
    Dwarf_Error     * /*error*/);

/* This frees all memory allocated by the applicable
    dwarf_get_loclist_c() */
void dwarf_loc_head_c_dealloc(Dwarf_Loc_Head_c /*loclist_head*/);

/* END: loclist_c interfaces */


/*  As of 2015 the preferred interface
    is dwarf_get_loclist_c()
    and only dwarf_get_loclist_c() will work
    for DWARF5 (and also all earlier versions).  */
int dwarf_loclist_n(Dwarf_Attribute /*attr*/,
    Dwarf_Locdesc*** /*llbuf*/,
    Dwarf_Signed *   /*locCount*/,
    Dwarf_Error*     /*error*/);

/*  The original interfaces.  Please do not use this. */
int dwarf_loclist(Dwarf_Attribute /*attr*/,  /* inflexible! */
    Dwarf_Locdesc**  /*llbuf*/,
    Dwarf_Signed *   /*locCount*/,
    Dwarf_Error*     /*error*/);

/* Extracts a dwarf expression from an expression byte stream.
   Useful to get expressions from DW_CFA_def_cfa_expression
   DW_CFA_expression DW_CFA_val_expression expression bytes.
   27 April 2009: dwarf_loclist_from_expr() interface with
   no addr_size is obsolete but supported,
   use dwarf_loclist_from_expr_a() instead.
*/
int dwarf_loclist_from_expr(Dwarf_Debug /*dbg*/,
    Dwarf_Ptr      /* expression_in*/,
    Dwarf_Unsigned /* expression_length*/,
    Dwarf_Locdesc ** /* llbuf*/,
    Dwarf_Signed * /*listlen*/,
    Dwarf_Error *  /* error*/ );

/*  dwarf_loclist_from_expr_a() new 27 Apr 2009:
    added addr_size argument. */
int dwarf_loclist_from_expr_a(Dwarf_Debug /*dbg*/,
    Dwarf_Ptr      /*expression_in*/,
    Dwarf_Unsigned /*expression_length*/,
    Dwarf_Half     /*addr_size*/,
    Dwarf_Locdesc ** /*llbuf*/,
    Dwarf_Signed * /*listlen*/,
    Dwarf_Error *  /*error*/);

/*  dwarf_loclist_from_expr_b() new 13 Nov 2012:
    added dwarf_version (DWARF version number
    of the applicable compilation unit)
    and offset_size arguments. Added for
    DW_OP_GNU_implicit_pointer. */
int dwarf_loclist_from_expr_b(Dwarf_Debug /*dbg*/,
    Dwarf_Ptr      /*expression_in*/ ,
    Dwarf_Unsigned /*expression_length*/ ,
    Dwarf_Half     /*addr_size*/ ,
    Dwarf_Half     /*offset_size*/ ,
    Dwarf_Small    /*dwarf_version*/ ,
    Dwarf_Locdesc ** /*llbuf*/ ,
    Dwarf_Signed * /*listlen*/ ,
    Dwarf_Error *  /*error*/ );

int dwarf_lowpc(Dwarf_Die /*die*/,
    Dwarf_Addr  *    /*returned_addr*/,
    Dwarf_Error*     /*error*/);

/*  When the highpc attribute is of class  'constant'
    it is not an address, it is an offset from the
    base address (such as lowpc) of the function.
    This is therefore a required interface for DWARF4
    style DW_AT_highpc.  */
int dwarf_highpc_b(Dwarf_Die /*die*/,
    Dwarf_Addr  *           /*return_value*/,
    Dwarf_Half  *           /*return_form*/,
    enum Dwarf_Form_Class * /*return_class*/,
    Dwarf_Error *           /*error*/);

/*  This works for DWARF2 and DWARF3 styles of DW_AT_highpc,
    but not for the DWARF4 class constant forms.
    If the FORM is of class constant this returns an error */
int dwarf_highpc(Dwarf_Die /*die*/,
    Dwarf_Addr  *    /*returned_addr*/,
    Dwarf_Error*     /*error*/);

/*  New January 2016. */
int dwarf_dietype_offset(Dwarf_Die /*die*/,
    Dwarf_Off   * /*return_off*/,
    Dwarf_Error * /*error*/);

int dwarf_bytesize(Dwarf_Die /*die*/,
    Dwarf_Unsigned * /*returned_size*/,
    Dwarf_Error*     /*error*/);

int dwarf_bitsize(Dwarf_Die /*die*/,
    Dwarf_Unsigned * /*returned_size*/,
    Dwarf_Error*     /*error*/);

int dwarf_bitoffset(Dwarf_Die /*die*/,
    Dwarf_Unsigned * /*returned_offset*/,
    Dwarf_Error*     /*error*/);

int dwarf_srclang(Dwarf_Die /*die*/,
    Dwarf_Unsigned * /*returned_lang*/,
    Dwarf_Error*     /*error*/);

int dwarf_arrayorder(Dwarf_Die /*die*/,
    Dwarf_Unsigned * /*returned_order*/,
    Dwarf_Error*     /*error*/);

/* end of convenience function list */

/* this is the main interface to attributes of a DIE */
int dwarf_attrlist(Dwarf_Die /*die*/,
    Dwarf_Attribute** /*attrbuf*/,
    Dwarf_Signed   * /*attrcount*/,
    Dwarf_Error*     /*error*/);

/* query operations for attributes */
int dwarf_hasform(Dwarf_Attribute /*attr*/,
    Dwarf_Half       /*form*/,
    Dwarf_Bool *     /*returned_bool*/,
    Dwarf_Error*     /*error*/);

int dwarf_whatform(Dwarf_Attribute /*attr*/,
    Dwarf_Half *     /*returned_final_form*/,
    Dwarf_Error*     /*error*/);

int dwarf_whatform_direct(Dwarf_Attribute /*attr*/,
    Dwarf_Half *     /*returned_initial_form*/,
    Dwarf_Error*     /*error*/);

int dwarf_whatattr(Dwarf_Attribute /*attr*/,
    Dwarf_Half *     /*returned_attr_num*/,
    Dwarf_Error*     /*error*/);

/*
    The following are concerned with the Primary Interface: getting
    the actual data values. One function per 'kind' of FORM.
*/
/*  dwarf_formref returns, thru return_offset, a CU-relative offset
    and does not allow DW_FORM_ref_addr*/
int dwarf_formref(Dwarf_Attribute /*attr*/,
    Dwarf_Off*       /*return_offset*/,
    Dwarf_Error*     /*error*/);
/*  dwarf_global_formref returns, thru return_offset,
    a debug_info-relative offset and does allow all reference forms*/
int dwarf_global_formref(Dwarf_Attribute /*attr*/,
    Dwarf_Off*       /*return_offset*/,
    Dwarf_Error*     /*error*/);

/*  dwarf_formsig8 returns in the caller-provided 8 byte area
    the 8 bytes of a DW_FORM_ref_sig8.  Not a string.  */
int dwarf_formsig8(Dwarf_Attribute /*attr*/,
    Dwarf_Sig8 * /*returned sig bytes*/,
    Dwarf_Error*     /*error*/);
/*  dwarf_formsig8_const returns in the caller-provided 8 byte area
    the 8 bytes of a form const (DW_FORM_data8).  Not a string.  */
int dwarf_formsig8_const(Dwarf_Attribute /*attr*/,
    Dwarf_Sig8 * /*returned sig bytes*/,
    Dwarf_Error*     /*error*/);

int dwarf_formaddr(Dwarf_Attribute /*attr*/,
    Dwarf_Addr   *   /*returned_addr*/,
    Dwarf_Error*     /*error*/);

/*  Part of DebugFission.  So a consumer can get the index when
    the object with the actual .debug_addr section is
    elsewhere. And so a print application can
    print the index.  New May 2014*/
int dwarf_get_debug_addr_index(Dwarf_Attribute /*attr*/,
    Dwarf_Unsigned * /*return_index*/,
    Dwarf_Error * /*error*/);

int dwarf_formflag(Dwarf_Attribute /*attr*/,
    Dwarf_Bool *     /*returned_bool*/,
    Dwarf_Error*     /*error*/);

int dwarf_formdata16(Dwarf_Attribute /*attr*/,
    Dwarf_Form_Data16  * /*returned_val*/,
    Dwarf_Error*     /*error*/);

int dwarf_formudata(Dwarf_Attribute /*attr*/,
    Dwarf_Unsigned  * /*returned_val*/,
    Dwarf_Error*     /*error*/);

int dwarf_formsdata(Dwarf_Attribute     /*attr*/,
    Dwarf_Signed  *  /*returned_val*/,
    Dwarf_Error*     /*error*/);

int dwarf_formblock(Dwarf_Attribute /*attr*/,
    Dwarf_Block    ** /*returned_block*/,
    Dwarf_Error*     /*error*/);

int dwarf_formstring(Dwarf_Attribute /*attr*/,
    char   **        /*returned_string*/,
    Dwarf_Error*     /*error*/);

/* DebugFission.  So a DWARF print application can
   get the string index (DW_FORM_strx) and print it.
   A convenience function.
   New May 2014. */
int
dwarf_get_debug_str_index(Dwarf_Attribute /*attr*/,
    Dwarf_Unsigned * /*return_index*/,
    Dwarf_Error * /*error*/);


int dwarf_formexprloc(Dwarf_Attribute /*attr*/,
    Dwarf_Unsigned * /*return_exprlen*/,
    Dwarf_Ptr  * /*block_ptr*/,
    Dwarf_Error * /*error*/);


/* end attribute query operations. */

/* Start line number operations */
/* dwarf_srclines  is the original interface from 1993. */
int dwarf_srclines(Dwarf_Die /*die*/,
    Dwarf_Line**     /*linebuf*/,
    Dwarf_Signed *   /*linecount*/,
    Dwarf_Error*     /*error*/);


/* If we have two-level line tables, this will return the
   logicals table in linebuf and the actuals table in
   linebuf_actuals. For old-style (one-level) tables, it
   will return the single table through linebuf, and the
   value returned through linecount_actuals will be 0.
   The actual version number is returned through version.
   For two-level line tables, the version returned will
   be 0xf006. This interface can return data from two-level
   line tables, which are experimental.
   Most users will not wish to use dwarf_srclines_two_level() */
int dwarf_srclines_two_level(Dwarf_Die /*die*/,
    Dwarf_Unsigned * /*version*/,
    Dwarf_Line**     /*linebuf*/,
    Dwarf_Signed *   /*linecount*/,
    Dwarf_Line**     /*linebuf_actuals*/,
    Dwarf_Signed *   /*linecount_actuals*/,
    Dwarf_Error*     /*error*/);

/* dwarf_srclines_dealloc, created July 2005, is the
   appropriate method for deallocating what dwarf_srclines()
   and dwarf_srclines_two_level() return.
   More complete free than using dwarf_dealloc directly.

   When dwarf_srclines_two_level returns two line tables
   user code should call dwarf_srclines_dealloc()
   once on each linebuf returned by dwarf_srclines_two_level()
   first on linebuf_actuals and then on linebuf{_logicals}.   */

void dwarf_srclines_dealloc(Dwarf_Debug /*dbg*/,
    Dwarf_Line*       /*linebuf*/,
    Dwarf_Signed      /*count */);


/*  New October 2015, must be used to deallocating
    what is allocated by dwarf_srclines_b() and
    dwarf_srclines_from_linecontext()  use.
    Works for DWARF2,3,4,5 and for experimental line tables.
    New work should use the new Dwarf_Line_Context
    interface.
    This interface only reads the line table header, so
    it takes relatively little time.
    *is_single_table will be set non-zero for all standard dwarf line sections.
    *is_single_table will be set zero for line sections with the
    two_level line table extension (which will have *version_out 0xf006).  */
int dwarf_srclines_b(Dwarf_Die /*die*/,
    Dwarf_Unsigned     * /* version_out*/,
    Dwarf_Small        * /* table_count */,
    Dwarf_Line_Context * /* linecontext*/,
    Dwarf_Error        * /* error*/);

/*  Functions passing in a Dwarf_Line_Context  are only
    available if dwarf_srclines_b() was used to access
    line table information.  */
/*  New October 2015.  Returns line details.
    Works for DWARF2,3,4,5.  If linecount
    returned is zero this is a line table with no lines.*/
int dwarf_srclines_from_linecontext(
    Dwarf_Line_Context /*line_context*/,
    Dwarf_Line  **   /*linebuf*/,
    Dwarf_Signed *   /*linecount*/,
    Dwarf_Error  *   /* error*/);

/*  New October 2015.  Returns line details.
    Works for DWARF2,3,4,5 and for experimental
    two-level line tables. A single level table will
    have *linebuf_actuals and *linecount_actuals set
    to 0. */
int dwarf_srclines_two_level_from_linecontext(
    Dwarf_Line_Context /*line_context*/,
    Dwarf_Line  **   /*linebuf */,
    Dwarf_Signed *   /*linecount*/,
    Dwarf_Line  **   /*linebuf_actuals*/,
    Dwarf_Signed *   /*linecount_actuals*/,
    Dwarf_Error  *   /* error*/);

/* dwarf_srclines_dealloc_b(), created October 2015, is the
   appropriate method for deallocating everything
   and dwarf_srclines_from_linecontext(),
   dwarf_srclines_twolevel_from_linecontext(),
   and dwarf_srclines_b()  allocate.  */
void dwarf_srclines_dealloc_b(Dwarf_Line_Context /*line_context*/);

/*  New October 2015. */
/*    The offset is in the relevent .debug_line or .debug_line.dwo
    section (and in a split dwarf package file includes)
    the base line table offset). */
int dwarf_srclines_table_offset(Dwarf_Line_Context /*line_context*/,
    Dwarf_Unsigned * /*offset*/,
    Dwarf_Error  * /* error*/);

/*  New October 2015. */
/*  Compilation Directory name for the current CU.
    section (and in a split dwarf package file includes)
    the base line table offset).  Do not free() the string,
    it is in a dwarf section. */
int dwarf_srclines_comp_dir(Dwarf_Line_Context /*line_context*/,
    const char ** /*compilation_directory*/,
    Dwarf_Error  *  /*error*/);

/*  New October 2015.  Part of the two-level line table extension. */
/*  Count is the real count of suprogram array entries. */
int dwarf_srclines_subprog_count(Dwarf_Line_Context /*line_context*/,
    Dwarf_Signed * /*count*/,
    Dwarf_Error  * /*error*/);

/*  New October 2015. */
/*  Index starts with 1, last is 'count' */
int dwarf_srclines_subprog_data(Dwarf_Line_Context /*line_context*/,
    Dwarf_Signed     /*index*/,
    const char **    /*name*/,
    Dwarf_Unsigned * /*decl_file*/,
    Dwarf_Unsigned * /*decl_line*/,
    Dwarf_Error   *  /*error*/);

/*  New October 2015. */
/*  Count is the real count of files array entries.
    This remains supported though it is pretty useless for
    DWARF5.  To process DWARF5 as well
    as DWARF 2,3,4 (in a uniform fashion)
    use dwarf_srclines_files_indexes() instead.
*/
int dwarf_srclines_files_count(Dwarf_Line_Context /*line_context*/,
    Dwarf_Signed  *  /*count*/,
    Dwarf_Error   *  /*error*/);

/*  New March 2018. */
/*  Count is the real count of files array entries.
    Since DWARF 2,3,4 are zero origin indexes and
    DWARF5 and later are one origin, this function
    replaces dwarf_srclines_files_count(). */
int dwarf_srclines_files_indexes(Dwarf_Line_Context /*line_context*/,
    Dwarf_Signed  *  /*baseindex*/,
    Dwarf_Signed  *  /*count*/,
    Dwarf_Signed  *  /*endindex*/,
    Dwarf_Error   *  /*error*/);


/*  New March 2018. Same as dwarf_srclines_files_data,
    but adds the md5ptr field so cases where DW_LNCT_MD5
    is present can return pointer to the MD5 value.
    With DWARF 5 index starts with 0.
    See dwarf_srclines_files_indexes() which makes
    indexing through the files easy. */
int
dwarf_srclines_files_data_b(Dwarf_Line_Context line_context,
    Dwarf_Signed     index_in,
    const char **    name,
    Dwarf_Unsigned * directory_index,
    Dwarf_Unsigned * last_mod_time,
    Dwarf_Unsigned * file_length,
    Dwarf_Form_Data16 ** md5ptr,
    Dwarf_Error    * error);

/*  New October 2015. */
/*  Unlike dwarf_srcfiles() this returns the raw file table
    strings without the directory being prefixed.
    Index starts with 1, last is 'count' */
int dwarf_srclines_files_data(Dwarf_Line_Context /*line_context*/,
    Dwarf_Signed     /*index*/,
    const char **    /*name*/,
    Dwarf_Unsigned * /*directory_index*/,
    Dwarf_Unsigned * /*last_mod_time*/,
    Dwarf_Unsigned * /*file_length*/,
    Dwarf_Error    * /* error*/);

/*  New October 2015. */
/*  Count is the real count of include array entries. */
int dwarf_srclines_include_dir_count(Dwarf_Line_Context /*line_context*/,
    Dwarf_Signed *  /*count*/,
    Dwarf_Error  * /* error*/);

/*  New October 2015. */
/*  Index starts with 1, last is 'count' */
int dwarf_srclines_include_dir_data(Dwarf_Line_Context /*line_context*/,
    Dwarf_Signed    /*index*/,
    const char **   /*name*/,
    Dwarf_Error   * /* error*/);

/*  New October 2015. */
/*  The DWARF version number of this compile-unit
    in the .debug_lines section and the number of
    actual tables:0 (header with no lines),
    1 (standard table), or 2 (experimental). */
int dwarf_srclines_version(Dwarf_Line_Context /*line_context*/,
    Dwarf_Unsigned * /*version*/,
    Dwarf_Small    * /*table_count*/,
    Dwarf_Error    * /*error*/);


int dwarf_get_line_section_name_from_die(Dwarf_Die /*die*/,
    const char ** /*section_name_out*/,
    Dwarf_Error * /*error*/);


/*  While 'filecount' is signed, the value
    returned through the pointer is never negative.
    Original libdwarf from 199x.  */
int dwarf_srcfiles(Dwarf_Die /*die*/,
    char***          /*srcfiles*/,
    Dwarf_Signed *   /*filecount*/,
    Dwarf_Error*     /*error*/);

int dwarf_linebeginstatement(Dwarf_Line /*line*/,
    Dwarf_Bool  *    /*returned_bool*/,
    Dwarf_Error*     /*error*/);

int dwarf_lineendsequence(Dwarf_Line /*line*/,
    Dwarf_Bool  *    /*returned_bool*/,
    Dwarf_Error*     /*error*/);

int dwarf_lineno(Dwarf_Line /*line*/,
    Dwarf_Unsigned * /*returned_lineno*/,
    Dwarf_Error*     /*error*/);

int dwarf_line_srcfileno(Dwarf_Line /*line*/,
    Dwarf_Unsigned * /*ret_fileno*/,
    Dwarf_Error *    /*error*/);

/* Is the line address from DW_LNS_set_address? */
int dwarf_line_is_addr_set(Dwarf_Line /*line*/,
    Dwarf_Bool *     /*is_addr_set*/,
    Dwarf_Error *    /*error*/);

int dwarf_lineaddr(Dwarf_Line /*line*/,
    Dwarf_Addr *     /*returned_addr*/,
    Dwarf_Error*     /*error*/);

/* dwarf_lineoff() is OBSOLETE as of December 2011. Do not use. */
int dwarf_lineoff(Dwarf_Line /*line*/,
    Dwarf_Signed  *  /*returned_lineoffset*/,
    Dwarf_Error*     /*error*/);

/*  dwarf_lineoff_b() correctly returns an unsigned column number
    through the pointer returned_lineoffset.
    dwarf_lineoff_b() is new in December 2011.  */
int dwarf_lineoff_b(Dwarf_Line /*line*/,
    Dwarf_Unsigned * /*returned_lineoffset*/,
    Dwarf_Error*     /*error*/);

int dwarf_linesrc(Dwarf_Line /*line*/,
    char   **        /*returned_name*/,
    Dwarf_Error*     /*error*/);

int dwarf_lineblock(Dwarf_Line /*line*/,
    Dwarf_Bool  *    /*returned_bool*/,
    Dwarf_Error*     /*error*/);

/* We gather these into one call as it's likely one
   will want all or none of them.  */
int dwarf_prologue_end_etc(Dwarf_Line /* line */,
    Dwarf_Bool  *    /*prologue_end*/,
    Dwarf_Bool  *    /*eplogue_begin*/,
    Dwarf_Unsigned * /* isa */,
    Dwarf_Unsigned * /* discriminator */,
    Dwarf_Error *    /*error*/);
/* End line table operations */

/* Two-level line tables:
   When reading from an actuals table, dwarf_line_logical()
   returns the logical row number for the line. */
int dwarf_linelogical(Dwarf_Line /*line*/,
    Dwarf_Unsigned * /*returned_logical*/,
    Dwarf_Error*     /*error*/);

/* Two-level line tables:
   When reading from a logicals table, dwarf_linecontext()
   returns the logical row number corresponding the the
   calling context for an inlined call. */
int dwarf_linecontext(Dwarf_Line /*line*/,
    Dwarf_Unsigned * /*returned_context*/,
    Dwarf_Error*     /*error*/);

/* Two-level line tables:
   When reading from a logicals table, dwarf_linesubprogno()
   returns the index in the subprograms table of the inlined
   subprogram. */
int dwarf_line_subprogno(Dwarf_Line /*line*/,
    Dwarf_Unsigned * /*ret_subprogno*/,
    Dwarf_Error *    /*error*/);

/* Two-level line tables:
   When reading from a logicals table, dwarf_linesubprog()
   returns the name of the inlined subprogram, its declaration
   filename, and its declaration line number, if available. */
int dwarf_line_subprog(Dwarf_Line /*line*/,
    char   **        /*returned_subprog_name*/,
    char   **        /*returned_filename*/,
    Dwarf_Unsigned * /*returned_lineno*/,
    Dwarf_Error *    /*error*/);
/* End of line table interfaces. */

/* .debug_names names table interfaces. DWARF5 */
/*  New April 2017 */
int dwarf_debugnames_header(Dwarf_Debug /*dbg*/,
    Dwarf_Dnames_Head * /*dn_out*/,

    /* *dn_count_out returns the number of name indexes
        in the .debug_names section */
    Dwarf_Unsigned    * /*dn_index_count_out*/,
    Dwarf_Error *       /*error*/);

/*  Since there may be multiple name indexes in a .debug_names
    section we use index_number starting at 0 through
    dn_index_count_out-1. */
int dwarf_debugnames_sizes(Dwarf_Dnames_Head /*dn*/,
    Dwarf_Unsigned      /*index_number*/,

    Dwarf_Unsigned * /*section_offset*/,
    Dwarf_Unsigned * /*version*/,
    Dwarf_Unsigned * /*offset_size*/, /* 4 or 8 */

    /* The counts are entry counts, not byte sizes. */
    Dwarf_Unsigned * /*comp_unit_count*/,
    Dwarf_Unsigned * /*local_type_unit_count*/,
    Dwarf_Unsigned * /*foreign_type_unit_count*/,
    Dwarf_Unsigned * /*bucket_count*/,
    Dwarf_Unsigned * /*name_count*/,

    /* The following are counted in bytes */
    Dwarf_Unsigned * /*indextable_overall_length*/,
    Dwarf_Unsigned * /*abbrev_table_size*/,
    Dwarf_Unsigned * /*entry_pool_size*/,
    Dwarf_Unsigned * /*augmentation_string_size*/,

    Dwarf_Error *       /*error*/);

int dwarf_debugnames_cu_entry(Dwarf_Dnames_Head /*dn*/,
    Dwarf_Unsigned      /*index_number*/,
    Dwarf_Unsigned      /*offset_number*/,
    Dwarf_Unsigned    * /*offset_count*/,
    Dwarf_Unsigned    * /*offset*/,
    Dwarf_Error *       /*error*/);
int dwarf_debugnames_local_tu_entry(Dwarf_Dnames_Head /*dn*/,
    Dwarf_Unsigned      /*index_number*/,
    Dwarf_Unsigned      /*offset_number*/,
    Dwarf_Unsigned    * /*offset_count*/,
    Dwarf_Unsigned    * /*offset*/,
    Dwarf_Error *       /*error*/);
int dwarf_debugnames_foreign_tu_entry(Dwarf_Dnames_Head /*dn*/,
    Dwarf_Unsigned      /*index_number*/,
    Dwarf_Unsigned      /*sig_number*/,
    Dwarf_Unsigned    * /*sig_mininum*/,
    Dwarf_Unsigned    * /*sig_count*/,
    Dwarf_Sig8        * /*signature*/,
    Dwarf_Error *       /*error*/);
int dwarf_debugnames_bucket(Dwarf_Dnames_Head /*dn*/,
    Dwarf_Unsigned      /*index_number*/,
    Dwarf_Unsigned      /*bucket_number*/,
    Dwarf_Unsigned    * /*bucket_count*/,
    Dwarf_Unsigned    * /*index_of_name_entry*/,
    Dwarf_Error *       /*error*/);

int dwarf_debugnames_name(Dwarf_Dnames_Head /*dn*/,
    Dwarf_Unsigned      /*index_number*/,
    Dwarf_Unsigned      /*name_entry*/,
    Dwarf_Unsigned    * /*names_count*/,
    Dwarf_Sig8        * /*signature*/,
    Dwarf_Unsigned    * /*offset_to_debug_str*/,
    Dwarf_Unsigned    * /*offset_in_entrypool*/,
    Dwarf_Error *       /*error*/);

int
dwarf_debugnames_abbrev_by_index(Dwarf_Dnames_Head /*dn*/,
    Dwarf_Unsigned    /*index_number*/,
    Dwarf_Unsigned    /*abbrev_entry*/,
    Dwarf_Unsigned *  /*abbrev_code*/,
    Dwarf_Unsigned *  /*tag*/,

    /*  The number of valid abbrev_entry values: 0 to number_of_abbrev-1
        */
    Dwarf_Unsigned *  /*number_of_abbrev*/,

    /*  The number of attr/form pairs, not counting the trailing
        0,0 pair. */
    Dwarf_Unsigned * /*number_of_attr_form_entries*/,
    Dwarf_Error *    /*error*/);

int
dwarf_debugnames_abbrev_by_code(Dwarf_Dnames_Head /*dn*/,
    Dwarf_Unsigned    /*index_number*/,
    Dwarf_Unsigned    /*abbrev_code*/,
    Dwarf_Unsigned *  /*tag*/,

    /*  The number of this code/tag as an array index. */
    Dwarf_Unsigned *  /*index_of_abbrev*/,

    /*  The number of attr/form pairs, not counting the trailing
        0,0 pair. */
    Dwarf_Unsigned * /*number_of_attr_form_entries*/,
    Dwarf_Error *    /*error*/);

int
dwarf_debugnames_abbrev_form_by_index(Dwarf_Dnames_Head /*dn*/,
    Dwarf_Unsigned   /*index_number*/,
    Dwarf_Unsigned   /*abbrev_entry_index*/,
    Dwarf_Unsigned   /*abbrev_form_index*/,
    Dwarf_Unsigned * /*name_index_attr*/,
    Dwarf_Unsigned * /*form*/,
    Dwarf_Unsigned * /*number_of_attr_form_entries*/,
    Dwarf_Error    * /*error*/);


/*  This, combined with dwarf_debugnames_entrypool_values(),
    lets one examine as much or as little of an entrypool
    as one wants to by alternately calling these two
    functions. */
int dwarf_debugnames_entrypool(Dwarf_Dnames_Head /*dn*/,
    Dwarf_Unsigned      /*index_number*/,
    Dwarf_Unsigned      /*offset_in_entrypool*/,
    Dwarf_Unsigned *    /*abbrev_code*/,
    Dwarf_Unsigned *    /*tag*/,
    Dwarf_Unsigned *    /*value_count*/,
    Dwarf_Unsigned *    /*index_of_abbrev*/,
    Dwarf_Unsigned *    /*offset_of_initial_value*/,
    Dwarf_Error *       /*error*/);

/*  Caller, knowing array size needed, passes in arrays
    it allocates of for idx, form, offset-size-values,
    and signature values.  Caller must examine idx-number
    and form to decide, for each array element, whether
    the offset or the signature contains the value.
    So this returns all the values for the abbrev code.
    And points via offset_of_next to the next abbrev code.
    */
int dwarf_debugnames_entrypool_values(Dwarf_Dnames_Head /*dn*/,
    Dwarf_Unsigned      /*index_number*/,
    Dwarf_Unsigned      /*index_of_abbrev*/,
    Dwarf_Unsigned      /*offset_in_entrypool_of_values*/,
    Dwarf_Unsigned *    /*array_dw_idx_number*/,
    Dwarf_Unsigned *    /*array_form*/,
    Dwarf_Unsigned *    /*array_of_offsets*/,
    Dwarf_Sig8     *    /*array_of_signatures*/,

    /*  offset of the next entrypool entry. */
    Dwarf_Unsigned *    /*offset_of_next_entrypool*/,
    Dwarf_Error *       /*error*/);

/*  FIXME: add interfaces for string search given hash and
    string */



/* end of .debug_names interfaces. */


/*  global name space operations (.debug_pubnames access) */
/*  The pubnames and similar sections are rarely used. Few compilers
    emit them. They are DWARF 2,3,4 only., not DWARF 5.  */
int dwarf_get_globals(Dwarf_Debug /*dbg*/,
    Dwarf_Global**   /*globals*/,
    Dwarf_Signed *   /*number_of_globals*/,
    Dwarf_Error*     /*error*/);
void dwarf_globals_dealloc(Dwarf_Debug /*dbg*/,
    Dwarf_Global*    /*globals*/,
    Dwarf_Signed     /*number_of_globals*/);

int dwarf_globname(Dwarf_Global /*glob*/,
    char   **        /*returned_name*/,
    Dwarf_Error*     /*error*/);

int dwarf_global_die_offset(Dwarf_Global /*global*/,
    Dwarf_Off*       /*return_offset*/,
    Dwarf_Error *    /*error*/);

/* This returns the CU die global offset if one knows the
   CU header global offset.
   See also dwarf_CU_dieoffset_given_die(). */
int dwarf_get_cu_die_offset_given_cu_header_offset(
    Dwarf_Debug      /*dbg*/,
    Dwarf_Off        /*in_cu_header_offset*/,
    Dwarf_Off *  /*out_cu_die_offset*/,
    Dwarf_Error *    /*err*/);

/*  The _b form is new October 2011. */
int dwarf_get_cu_die_offset_given_cu_header_offset_b(
    Dwarf_Debug      /*dbg*/,
    Dwarf_Off        /*in_cu_header_offset*/,
    Dwarf_Bool       /*is_info. True means look in debug_Info,
        false use debug_types.*/,
    Dwarf_Off *  /*out_cu_die_offset*/,
    Dwarf_Error *    /*err*/);

#ifdef __sgi /* pragma is sgi MIPS only */
#pragma optional dwarf_get_cu_die_offset_given_cu_header_offset
#endif

int dwarf_global_cu_offset(Dwarf_Global /*global*/,
    Dwarf_Off*       /*return_offset*/,
    Dwarf_Error*     /*error*/);

int dwarf_global_name_offsets(Dwarf_Global /*global*/,
    char   **        /*returned_name*/,
    Dwarf_Off*       /*die_offset*/,
    Dwarf_Off*       /*cu_offset*/,
    Dwarf_Error*     /*error*/);

/* Static function name operations.  */
int dwarf_get_funcs(Dwarf_Debug    /*dbg*/,
    Dwarf_Func**     /*funcs*/,
    Dwarf_Signed *   /*number_of_funcs*/,
    Dwarf_Error*     /*error*/);
void dwarf_funcs_dealloc(Dwarf_Debug /*dbg*/,
    Dwarf_Func*      /*funcs*/,
    Dwarf_Signed     /*number_of_funcs*/);

int dwarf_funcname(Dwarf_Func /*func*/,
    char   **        /*returned_name*/,
    Dwarf_Error*     /*error*/);

int dwarf_func_die_offset(Dwarf_Func /*func*/,
    Dwarf_Off*       /*return_offset*/,
    Dwarf_Error*     /*error*/);

int dwarf_func_cu_offset(Dwarf_Func /*func*/,
    Dwarf_Off*       /*return_offset*/,
    Dwarf_Error*     /*error*/);

int dwarf_func_name_offsets(Dwarf_Func /*func*/,
    char   **        /*returned_name*/,
    Dwarf_Off*       /*die_offset*/,
    Dwarf_Off*       /*cu_offset*/,
    Dwarf_Error*     /*error*/);

/* User-defined type name operations, SGI IRIX .debug_typenames section.
   Same content as DWARF3 .debug_pubtypes, but defined years before
   .debug_pubtypes was defined.   SGI IRIX only. */
int dwarf_get_types(Dwarf_Debug    /*dbg*/,
    Dwarf_Type**     /*types*/,
    Dwarf_Signed *   /*number_of_types*/,
    Dwarf_Error*     /*error*/);
void dwarf_types_dealloc(Dwarf_Debug /*dbg*/,
    Dwarf_Type*      /*types*/,
    Dwarf_Signed     /*number_of_types*/);


int dwarf_typename(Dwarf_Type /*type*/,
    char   **        /*returned_name*/,
    Dwarf_Error*     /*error*/);

int dwarf_type_die_offset(Dwarf_Type /*type*/,
    Dwarf_Off*       /*return_offset*/,
    Dwarf_Error*     /*error*/);

int dwarf_type_cu_offset(Dwarf_Type /*type*/,
    Dwarf_Off*       /*return_offset*/,
    Dwarf_Error*     /*error*/);

int dwarf_type_name_offsets(Dwarf_Type    /*type*/,
    char   **        /*returned_name*/,
    Dwarf_Off*       /*die_offset*/,
    Dwarf_Off*       /*cu_offset*/,
    Dwarf_Error*     /*error*/);

/* User-defined type name operations, DWARF3  .debug_pubtypes section.
*/
int dwarf_get_pubtypes(Dwarf_Debug    /*dbg*/,
    Dwarf_Type**     /*types*/,
    Dwarf_Signed *   /*number_of_types*/,
    Dwarf_Error*     /*error*/);
void dwarf_pubtypes_dealloc(Dwarf_Debug /*dbg*/,
    Dwarf_Type*      /*pubtypes*/,
    Dwarf_Signed     /*number_of_pubtypes*/);


int dwarf_pubtypename(Dwarf_Type /*type*/,
    char   **        /*returned_name*/,
    Dwarf_Error*     /*error*/);

int dwarf_pubtype_type_die_offset(Dwarf_Type /*type*/,
    Dwarf_Off*       /*return_offset*/,
    Dwarf_Error*     /*error*/);

int dwarf_pubtype_cu_offset(Dwarf_Type /*type*/,
    Dwarf_Off*       /*return_offset*/,
    Dwarf_Error*     /*error*/);

int dwarf_pubtype_name_offsets(Dwarf_Type    /*type*/,
    char   **        /*returned_name*/,
    Dwarf_Off*       /*die_offset*/,
    Dwarf_Off*       /*cu_offset*/,
    Dwarf_Error*     /*error*/);

/* File-scope static variable name operations.  */
int dwarf_get_vars(Dwarf_Debug    /*dbg*/,
    Dwarf_Var**      /*vars*/,
    Dwarf_Signed *   /*number_of_vars*/,
    Dwarf_Error*     /*error*/);
void dwarf_vars_dealloc(Dwarf_Debug /*dbg*/,
    Dwarf_Var*       /*vars*/,
    Dwarf_Signed     /*number_of_vars*/);


int dwarf_varname(Dwarf_Var /*var*/,
    char   **        /*returned_name*/,
    Dwarf_Error*     /*error*/);

int dwarf_var_die_offset(Dwarf_Var /*var*/,
    Dwarf_Off*       /*return_offset*/,
    Dwarf_Error*     /*error*/);

int dwarf_var_cu_offset(Dwarf_Var /*var*/,
    Dwarf_Off*       /*return_offset*/,
    Dwarf_Error*     /*error*/);

int dwarf_var_name_offsets(Dwarf_Var /*var*/,
    char   **        /*returned_name*/,
    Dwarf_Off*       /*die_offset*/,
    Dwarf_Off*       /*cu_offset*/,
    Dwarf_Error*     /*error*/);

/* weak name operations.  */
int dwarf_get_weaks(Dwarf_Debug    /*dbg*/,
    Dwarf_Weak**     /*weaks*/,
    Dwarf_Signed *   /*number_of_weaks*/,
    Dwarf_Error*     /*error*/);
void dwarf_weaks_dealloc(Dwarf_Debug /*dbg*/,
    Dwarf_Weak*      /*weaks*/,
    Dwarf_Signed     /*number_of_weaks*/);


int dwarf_weakname(Dwarf_Weak /*weak*/,
    char   **        /*returned_name*/,
    Dwarf_Error*     /*error*/);

int dwarf_weak_die_offset(Dwarf_Weak /*weak*/,
    Dwarf_Off*       /*return_offset*/,
    Dwarf_Error*     /*error*/);

int dwarf_weak_cu_offset(Dwarf_Weak /*weak*/,
    Dwarf_Off*       /*return_offset*/,
    Dwarf_Error*     /*error*/);

int dwarf_weak_name_offsets(Dwarf_Weak    /*weak*/,
    char   **        /*returned_name*/,
    Dwarf_Off*       /*die_offset*/,
    Dwarf_Off*       /*cu_offset*/,
    Dwarf_Error*     /*error*/);

/* location list section operation.  (.debug_loc access) */
int dwarf_get_loclist_entry(Dwarf_Debug /*dbg*/,
    Dwarf_Unsigned   /*offset*/,
    Dwarf_Addr*      /*hipc*/,
    Dwarf_Addr*      /*lopc*/,
    Dwarf_Ptr*       /*data*/,
    Dwarf_Unsigned*  /*entry_len*/,
    Dwarf_Unsigned*  /*next_entry*/,
    Dwarf_Error*     /*error*/);

/* abbreviation section operations */
int dwarf_get_abbrev(Dwarf_Debug /*dbg*/,
    Dwarf_Unsigned   /*offset*/,
    Dwarf_Abbrev  *  /*returned_abbrev*/,
    Dwarf_Unsigned*  /*length*/,
    Dwarf_Unsigned*  /*attr_count*/,
    Dwarf_Error*     /*error*/);

int dwarf_get_abbrev_tag(Dwarf_Abbrev /*abbrev*/,
    Dwarf_Half*      /*return_tag_number*/,
    Dwarf_Error*     /*error*/);
int dwarf_get_abbrev_code(Dwarf_Abbrev /*abbrev*/,
    Dwarf_Unsigned*  /*return_code_number*/,
    Dwarf_Error*     /*error*/);
/* See comments in dwarf_abbrev.c. Not an entirely safe function. */
int dwarf_get_abbrev_count(Dwarf_Debug /*dbg*/);

int dwarf_get_abbrev_children_flag(Dwarf_Abbrev /*abbrev*/,
    Dwarf_Signed*    /*return_flag*/,
    Dwarf_Error*     /*error*/);

int dwarf_get_abbrev_entry(Dwarf_Abbrev /*abbrev*/,
    Dwarf_Signed     /*index*/,
    Dwarf_Half  *    /*returned_attr_num*/,
    Dwarf_Signed*    /*form*/,
    Dwarf_Off*       /*offset*/,
    Dwarf_Error*     /*error*/);

int dwarf_get_string_section_name(Dwarf_Debug /*dbg*/,
    const char ** /*section_name_out*/,
    Dwarf_Error * /*error*/);

/* consumer string section operation */
int dwarf_get_str(Dwarf_Debug /*dbg*/,
    Dwarf_Off        /*offset*/,
    char**           /*string*/,
    Dwarf_Signed *   /*strlen_of_string*/,
    Dwarf_Error*     /*error*/);

/* New November 2015 */
int dwarf_get_frame_section_name(Dwarf_Debug /*dbg*/,
   const char ** /*section_name_out*/,
   Dwarf_Error * /*error*/);

/* New November 2015 */
int dwarf_get_frame_section_name_eh_gnu(Dwarf_Debug /*dbg*/,
   const char ** /*section_name_out*/,
   Dwarf_Error * /*error*/);

/* Consumer op on  gnu .eh_frame info */
int dwarf_get_fde_list_eh(
    Dwarf_Debug      /*dbg*/,
    Dwarf_Cie**      /*cie_data*/,
    Dwarf_Signed*    /*cie_element_count*/,
    Dwarf_Fde**      /*fde_data*/,
    Dwarf_Signed*    /*fde_element_count*/,
    Dwarf_Error*     /*error*/);


/* consumer operations on frame info: .debug_frame */
int dwarf_get_fde_list(Dwarf_Debug /*dbg*/,
    Dwarf_Cie**      /*cie_data*/,
    Dwarf_Signed*    /*cie_element_count*/,
    Dwarf_Fde**      /*fde_data*/,
    Dwarf_Signed*    /*fde_element_count*/,
    Dwarf_Error*     /*error*/);

/* Release storage gotten by dwarf_get_fde_list_eh() or
   dwarf_get_fde_list() */
void dwarf_fde_cie_list_dealloc(Dwarf_Debug /*dbg*/,
    Dwarf_Cie *  /*cie_data*/,
    Dwarf_Signed /*cie_element_count*/,
    Dwarf_Fde *  /*fde_data*/,
    Dwarf_Signed /*fde_element_count*/);



int dwarf_get_fde_range(Dwarf_Fde /*fde*/,
    Dwarf_Addr*      /*low_pc*/,
    Dwarf_Unsigned*  /*func_length*/,
    Dwarf_Ptr*       /*fde_bytes*/,
    Dwarf_Unsigned*  /*fde_byte_length*/,
    Dwarf_Off*       /*cie_offset*/,
    Dwarf_Signed*    /*cie_index*/,
    Dwarf_Off*       /*fde_offset*/,
    Dwarf_Error*     /*error*/);

/*  Useful for IRIX only:  see dwarf_get_cie_augmentation_data()
    dwarf_get_fde_augmentation_data() for GNU .eh_frame. */
int dwarf_get_fde_exception_info(Dwarf_Fde /*fde*/,
    Dwarf_Signed*    /* offset_into_exception_tables */,
    Dwarf_Error*     /*error*/);


int dwarf_get_cie_of_fde(Dwarf_Fde /*fde*/,
    Dwarf_Cie *      /*cie_returned*/,
    Dwarf_Error*     /*error*/);

int dwarf_get_cie_info_b(Dwarf_Cie /*cie*/,
    Dwarf_Unsigned * /*bytes_in_cie*/,
    Dwarf_Small*     /*version*/,
    char        **   /*augmenter*/,
    Dwarf_Unsigned*  /*code_alignment_factor*/,
    Dwarf_Signed*    /*data_alignment_factor*/,
    Dwarf_Half*      /*return_address_register_rule*/,
    Dwarf_Ptr*       /*initial_instructions*/,
    Dwarf_Unsigned*  /*initial_instructions_length*/,
    Dwarf_Half*      /*offset_size*/,
    Dwarf_Error*     /*error*/);
int dwarf_get_cie_info(Dwarf_Cie /*cie*/,
    Dwarf_Unsigned * /*bytes_in_cie*/,
    Dwarf_Small*     /*version*/,
    char        **   /*augmenter*/,
    Dwarf_Unsigned*  /*code_alignment_factor*/,
    Dwarf_Signed*    /*data_alignment_factor*/,
    Dwarf_Half*      /*return_address_register_rule*/,
    Dwarf_Ptr*       /*initial_instructions*/,
    Dwarf_Unsigned*  /*initial_instructions_length*/,
    Dwarf_Error*     /*error*/);

/* dwarf_get_cie_index new September 2009. */
int dwarf_get_cie_index(
    Dwarf_Cie /*cie*/,
    Dwarf_Signed* /*index*/,
    Dwarf_Error* /*error*/ );


int dwarf_get_fde_instr_bytes(Dwarf_Fde /*fde*/,
    Dwarf_Ptr *      /*outinstrs*/, Dwarf_Unsigned * /*outlen*/,
    Dwarf_Error *    /*error*/);

int dwarf_get_fde_info_for_all_regs(Dwarf_Fde /*fde*/,
    Dwarf_Addr       /*pc_requested*/,
    Dwarf_Regtable*  /*reg_table*/,
    Dwarf_Addr*      /*row_pc*/,
    Dwarf_Error*     /*error*/);

int dwarf_get_fde_info_for_all_regs3(Dwarf_Fde /*fde*/,
    Dwarf_Addr       /*pc_requested*/,
    Dwarf_Regtable3* /*reg_table*/,
    Dwarf_Addr*      /*row_pc*/,
    Dwarf_Error*     /*error*/);

/* In this older interface DW_FRAME_CFA_COL is a meaningful
    column (which does not work well with DWARF3 or
    non-MIPS architectures). */
int dwarf_get_fde_info_for_reg(Dwarf_Fde /*fde*/,
    Dwarf_Half       /*table_column*/,
    Dwarf_Addr       /*pc_requested*/,
    Dwarf_Signed*    /*offset_relevant*/,
    Dwarf_Signed*    /*register*/,
    Dwarf_Signed*    /*offset*/,
    Dwarf_Addr*      /*row_pc*/,
    Dwarf_Error*     /*error*/);

/* See discussion of dw_value_type, libdwarf.h.
   Use of DW_FRAME_CFA_COL is not meaningful in this interface.
   See dwarf_get_fde_info_for_cfa_reg3().
*/
/* dwarf_get_fde_info_for_reg3 is useful on a single column, but
   it is inefficient to iterate across all table_columns using this
   function.  Instead call dwarf_get_fde_info_for_all_regs3() and index
   into the table it fills in. */
int dwarf_get_fde_info_for_reg3(Dwarf_Fde /*fde*/,
    Dwarf_Half       /*table_column*/,
    Dwarf_Addr       /*pc_requested*/,
    Dwarf_Small  *   /*value_type*/,
    Dwarf_Signed *   /*offset_relevant*/,
    Dwarf_Signed *   /*register*/,
    Dwarf_Signed *   /*offset_or_block_len*/,
    Dwarf_Ptr    *   /*block_ptr */,
    Dwarf_Addr   *   /*row_pc_out*/,
    Dwarf_Error  *    /*error*/);

int dwarf_get_fde_info_for_reg3_b(Dwarf_Fde /*fde*/,
    Dwarf_Half       /*table_column*/,
    Dwarf_Addr       /*pc_requested*/,
    Dwarf_Small  *   /*value_type*/,
    Dwarf_Signed *   /*offset_relevant*/,
    Dwarf_Signed *    /*register*/,
    Dwarf_Signed *    /*offset_or_block_len*/,
    Dwarf_Ptr    *    /*block_ptr */,
    Dwarf_Addr   *    /*row_pc_out*/,
    Dwarf_Bool   *    /* has_more_rows */,
    Dwarf_Addr   *    /* subsequent_pc */,
    Dwarf_Error  *    /*error*/);

/*  Use this or the next function to get the cfa.
    New function, June 11, 2016*/
int dwarf_get_fde_info_for_cfa_reg3_b(Dwarf_Fde /*fde*/,
    Dwarf_Addr       /*pc_requested*/,
    Dwarf_Small  *   /*value_type*/,
    Dwarf_Signed *   /*offset_relevant*/,
    Dwarf_Signed*    /*register*/,
    Dwarf_Signed*    /*offset_or_block_len*/,
    Dwarf_Ptr   *    /*block_ptr */,
    Dwarf_Addr*      /*row_pc_out*/,
    Dwarf_Bool  *    /* has_more_rows */,
    Dwarf_Addr  *    /* subsequent_pc */,
    Dwarf_Error*     /*error*/);
/* Use this to get the cfa. Or the above function. */
int dwarf_get_fde_info_for_cfa_reg3(Dwarf_Fde /*fde*/,
    Dwarf_Addr       /*pc_requested*/,
    Dwarf_Small  *   /*value_type*/,
    Dwarf_Signed *   /*offset_relevant*/,
    Dwarf_Signed*    /*register*/,
    Dwarf_Signed*    /*offset_or_block_len*/,
    Dwarf_Ptr   *    /*block_ptr */,
    Dwarf_Addr*      /*row_pc_out*/,
    Dwarf_Error*     /*error*/);

int dwarf_get_fde_for_die(Dwarf_Debug /*dbg*/,
    Dwarf_Die        /*subr_die */,
    Dwarf_Fde  *     /*returned_fde*/,
    Dwarf_Error*     /*error*/);

int dwarf_get_fde_n(Dwarf_Fde* /*fde_data*/,
    Dwarf_Unsigned   /*fde_index*/,
    Dwarf_Fde  *     /*returned_fde*/,
    Dwarf_Error*     /*error*/);

int dwarf_get_fde_at_pc(Dwarf_Fde* /*fde_data*/,
    Dwarf_Addr       /*pc_of_interest*/,
    Dwarf_Fde  *     /*returned_fde*/,
    Dwarf_Addr*      /*lopc*/,
    Dwarf_Addr*      /*hipc*/,
    Dwarf_Error*     /*error*/);

/* GNU .eh_frame augmentation information, raw form, see
   Linux Standard Base Core Specification version 3.0 . */
int dwarf_get_cie_augmentation_data(Dwarf_Cie /* cie*/,
    Dwarf_Small **   /* augdata */,
    Dwarf_Unsigned * /* augdata_len */,
    Dwarf_Error*     /*error*/);
/* GNU .eh_frame augmentation information, raw form, see
   Linux Standard Base Core Specification version 3.0 . */
int dwarf_get_fde_augmentation_data(Dwarf_Fde /* fde*/,
    Dwarf_Small **   /* augdata */,
    Dwarf_Unsigned * /* augdata_len */,
    Dwarf_Error*     /*error*/);

int dwarf_expand_frame_instructions(Dwarf_Cie /*cie*/,
    Dwarf_Ptr        /*instruction*/,
    Dwarf_Unsigned   /*i_length*/,
    Dwarf_Frame_Op** /*returned_op_list*/,
    Dwarf_Signed*    /*op_count*/,
    Dwarf_Error*     /*error*/);

/* Operations on .debug_aranges. */
int dwarf_get_aranges(Dwarf_Debug /*dbg*/,
    Dwarf_Arange**   /*aranges*/,
    Dwarf_Signed *   /*arange_count*/,
    Dwarf_Error*     /*error*/);

int dwarf_get_ranges_section_name(Dwarf_Debug /*dbg*/,
    const char ** /*section_name_out*/,
    Dwarf_Error * /*error*/);

int dwarf_get_aranges_section_name(Dwarf_Debug /*dbg*/,
    const char ** /*section_name_out*/,
    Dwarf_Error * /*error*/);



int dwarf_get_arange(
    Dwarf_Arange*    /*aranges*/,
    Dwarf_Unsigned   /*arange_count*/,
    Dwarf_Addr       /*address*/,
    Dwarf_Arange *   /*returned_arange*/,
    Dwarf_Error*     /*error*/);

int dwarf_get_cu_die_offset(
    Dwarf_Arange     /*arange*/,
    Dwarf_Off*       /*return_offset*/,
    Dwarf_Error*     /*error*/);

int dwarf_get_arange_cu_header_offset(
    Dwarf_Arange     /*arange*/,
    Dwarf_Off*       /*return_cu_header_offset*/,
    Dwarf_Error*     /*error*/);
#ifdef __sgi /* pragma is sgi MIPS only */
#pragma optional dwarf_get_arange_cu_header_offset
#endif

/* DWARF2,3 interface. No longer really adequate (it was never
   right for segmented address spaces, please switch
   to using dwarf_get_arange_info_b instead.
   There is no effective difference between these
   functions  if the address space
   of the target is not segmented.  */
int dwarf_get_arange_info(
    Dwarf_Arange     /*arange*/,
    Dwarf_Addr*      /*start*/,
    Dwarf_Unsigned*  /*length*/,
    Dwarf_Off*       /*cu_die_offset*/,
    Dwarf_Error*     /*error*/ );

/* New for DWARF4, entries may have segment information.
   *segment is only meaningful if *segment_entry_size is non-zero. */
int dwarf_get_arange_info_b(
    Dwarf_Arange     /*arange*/,
    Dwarf_Unsigned*  /*segment*/,
    Dwarf_Unsigned*  /*segment_entry_size*/,
    Dwarf_Addr    *  /*start*/,
    Dwarf_Unsigned*  /*length*/,
    Dwarf_Off     *  /*cu_die_offset*/,
    Dwarf_Error   *  /*error*/ );

/*  BEGIN: DWARF5 .debug_macro  interfaces
    NEW November 2015.  */
int dwarf_get_macro_context(Dwarf_Die /*die*/,
    Dwarf_Unsigned      * /*version_out*/,
    Dwarf_Macro_Context * /*macro_context*/,
    Dwarf_Unsigned      * /*macro_unit_offset_out*/,
    Dwarf_Unsigned      * /*macro_ops_count_out*/,
    Dwarf_Unsigned      * /*macro_ops_data_length_out*/,
    Dwarf_Error         * /*error*/);

/*  Just like dwarf_get_macro_context, but instead of using
    DW_AT_macros or DW_AT_GNU_macros to get the offset we just
    take the offset given. */
int dwarf_get_macro_context_by_offset(Dwarf_Die /*die*/,
    Dwarf_Unsigned        /*offset*/,
    Dwarf_Unsigned      * /*version_out*/,
    Dwarf_Macro_Context * /*macro_context*/,
    Dwarf_Unsigned      * /*macro_ops_count_out*/,
    Dwarf_Unsigned      * /*macro_ops_data_length*/,
    Dwarf_Error         * /*error*/);

void dwarf_dealloc_macro_context(Dwarf_Macro_Context /*mc*/);
int dwarf_get_macro_section_name(Dwarf_Debug /*dbg*/,
    const char ** /*sec_name_out*/,
    Dwarf_Error * /*err*/);

int dwarf_macro_context_head(Dwarf_Macro_Context /*head*/,
    Dwarf_Half     * /*version*/,
    Dwarf_Unsigned * /*mac_offset*/,
    Dwarf_Unsigned * /*mac_len*/,
    Dwarf_Unsigned * /*mac_header_len*/,
    unsigned       * /*flags*/,
    Dwarf_Bool     * /*has_line_offset*/,
    Dwarf_Unsigned * /*line_offset*/,
    Dwarf_Bool     * /*has_offset_size_64*/,
    Dwarf_Bool     * /*has_operands_table*/,
    Dwarf_Half     * /*opcode_count*/,
    Dwarf_Error    * /*error*/);

/*  Returns data from the operands table
    in the macro unit header. */
int dwarf_macro_operands_table(Dwarf_Macro_Context /*head*/,
    Dwarf_Half    /*index*/, /* 0 to opcode_count -1 */
    Dwarf_Half  * /*opcode_number*/,
    Dwarf_Half  * /*operand_count*/,
    const Dwarf_Small ** /*operand_array*/,
    Dwarf_Error * /*error*/);

/*  Access to the macro operations, 0 to macro_ops_count_out-1
    Where the last of these will have macro_operator 0 (which appears
    in the ops data and means end-of-ops).
    op_start_section_offset is the section offset of
    the macro operator (which is a single unsigned byte,
    and is followed by the macro operand data). */
int dwarf_get_macro_op(Dwarf_Macro_Context /*macro_context*/,
    Dwarf_Unsigned   /*op_number*/,
    Dwarf_Unsigned * /*op_start_section_offset*/,
    Dwarf_Half     * /*macro_operator*/,
    Dwarf_Half     * /*forms_count*/,
    const Dwarf_Small **  /*formcode_array*/,
    Dwarf_Error    * /*error*/);

int dwarf_get_macro_defundef(Dwarf_Macro_Context /*macro_context*/,
    Dwarf_Unsigned   /*op_number*/,
    Dwarf_Unsigned * /*line_number*/,
    Dwarf_Unsigned * /*index*/,
    Dwarf_Unsigned * /*offset*/,
    Dwarf_Half     * /*forms_count*/,
    const char    ** /*macro_string*/,
    Dwarf_Error    * /*error*/);
int dwarf_get_macro_startend_file(Dwarf_Macro_Context /*macro_context*/,
    Dwarf_Unsigned   /*op_number*/,
    Dwarf_Unsigned * /*line_number*/,
    Dwarf_Unsigned * /*name_index_to_line_tab*/,
    const char    ** /*src_file_name*/,
    Dwarf_Error    * /*error*/);
int dwarf_get_macro_import(Dwarf_Macro_Context /*macro_context*/,
    Dwarf_Unsigned   /*op_number*/,
    Dwarf_Unsigned * /*target_offset*/,
    Dwarf_Error    * /*error*/);

/*  END: DWARF5 .debug_macro interfaces. */

/* consumer .debug_macinfo information interface.
*/
struct Dwarf_Macro_Details_s {
    Dwarf_Off    dmd_offset; /* offset, in the section,
        of this macro info */
    Dwarf_Small  dmd_type;   /* the type, DW_MACINFO_define etc*/
    Dwarf_Signed dmd_lineno; /* the source line number where
        applicable and vend_def number if
        vendor_extension op */

    Dwarf_Signed dmd_fileindex;/* the source file index:
        applies to define undef start_file */
    char *       dmd_macro;  /* macro name (with value for defineop)
        string from vendor ext */
};

/*  dwarf_print_lines is for use by dwarfdump: it prints
    line info to stdout.
    The _dwarf name is obsolete. Use dwarf_ instead.
    Added extra argnument 2/2009 for better checking.
*/
int _dwarf_print_lines(Dwarf_Die /*cu_die*/,Dwarf_Error * /*error*/);
int dwarf_print_lines(Dwarf_Die /*cu_die*/,Dwarf_Error * /*error*/,
   int * /*error_count_out */);

/*  As of August 2013, dwarf_print_lines() no longer uses printf.
    Instead it calls back to the application using a function pointer
    once per line-to-print.  The lines passed back already have any needed
    newlines.    The following struct is used to initialize
    the callback mechanism.

    Failing to call the dwarf_register_printf_callback() function will
    prevent the lines from being passed back but such omission
    is not an error.
    See libdwarf2.1.mm for further documentation.

    The return value is the previous set of callback values.
*/

typedef void (* dwarf_printf_callback_function_type)
    (void * /*user_pointer*/, const char * /*linecontent*/);

struct Dwarf_Printf_Callback_Info_s {
    void *                        dp_user_pointer;
    dwarf_printf_callback_function_type dp_fptr;
    char *                        dp_buffer;
    unsigned int                  dp_buffer_len;
    int                           dp_buffer_user_provided;
    void *                        dp_reserved;
};

/*  If called with a NULL newvalues pointer, it simply returns
    the current set of values for this Dwarf_Debug. */
struct  Dwarf_Printf_Callback_Info_s
dwarf_register_printf_callback(Dwarf_Debug /*dbg*/,
    struct  Dwarf_Printf_Callback_Info_s * /*newvalues*/);


/*  dwarf_check_lineheader lets dwarfdump get detailed messages
    about some compiler errors we detect.
    We return the count of detected errors through the
    pointer.
*/
void dwarf_check_lineheader(Dwarf_Die /*cu_die*/,int *errcount_out);

/*  dwarf_ld_sort_lines helps SGI IRIX ld
    rearrange lines in .debug_line in a .o created with a text
    section per function.
        -OPT:procedure_reorder=ON
    where ld-cord (cord(1)ing by ld,
    not by cord(1)) may have changed the function order.
    The _dwarf name is obsolete. Use dwarf_ instead.
*/
int _dwarf_ld_sort_lines(
    void *         /*orig_buffer*/,
    unsigned long  /* buffer_len*/,
    int            /*is_64_bit*/,
    int *          /*any_change*/,
    int *          /*err_code*/);
int dwarf_ld_sort_lines(
    void *         /*orig_buffer*/,
    unsigned long  /*buffer_len*/,
    int            /*is_64_bit*/,
    int *          /*any_change*/,
    int *          /*err_code*/);

/* Used by dwarfdump -v to print fde offsets from debugging
   info.
   The _dwarf name is obsolete. Use dwarf_ instead.
*/
int _dwarf_fde_section_offset(Dwarf_Debug /*dbg*/,
    Dwarf_Fde         /*in_fde*/,
    Dwarf_Off *       /*fde_off*/,
    Dwarf_Off *       /*cie_off*/,
    Dwarf_Error *     /*err*/);

int dwarf_fde_section_offset(Dwarf_Debug /*dbg*/,
    Dwarf_Fde         /*in_fde*/,
    Dwarf_Off *       /*fde_off*/,
    Dwarf_Off *       /*cie_off*/,
    Dwarf_Error *     /*err*/);

/* Used by dwarfdump -v to print cie offsets from debugging
   info.
   The _dwarf name is obsolete. Use dwarf_ instead.
*/
int dwarf_cie_section_offset(Dwarf_Debug /*dbg*/,
    Dwarf_Cie     /*in_cie*/,
    Dwarf_Off *   /*cie_off */,
    Dwarf_Error * /*err*/);
int _dwarf_cie_section_offset(Dwarf_Debug /*dbg*/,
    Dwarf_Cie     /*in_cie*/,
    Dwarf_Off *   /*cie_off*/,
    Dwarf_Error * /*err*/);

typedef struct Dwarf_Macro_Details_s Dwarf_Macro_Details;

char *dwarf_find_macro_value_start(char * /*macro_string*/);

int dwarf_get_macro_details(Dwarf_Debug /*dbg*/,
    Dwarf_Off            /*macro_offset*/,
    Dwarf_Unsigned       /*maximum_count*/,
    Dwarf_Signed         * /*entry_count*/,
    Dwarf_Macro_Details ** /*details*/,
    Dwarf_Error *        /*err*/);


/*  dwarf_get_offset_size() New October 2015 */
int dwarf_get_offset_size(Dwarf_Debug /*dbg*/,
    Dwarf_Half  *    /*offset_size*/,
    Dwarf_Error *    /*error*/);
int dwarf_get_address_size(Dwarf_Debug /*dbg*/,
    Dwarf_Half  *    /*addr_size*/,
    Dwarf_Error *    /*error*/);
int dwarf_get_die_address_size(Dwarf_Die /*die*/,
    Dwarf_Half  *    /*addr_size*/,
    Dwarf_Error *    /*error*/);

enum Dwarf_Form_Class dwarf_get_form_class(
    Dwarf_Half /* dwversion */,
    Dwarf_Half /* attrnum */,
    Dwarf_Half /*offset_size */,
    Dwarf_Half /*form*/);


/*   BEGIN gdbindex operations interfaces. */
/*  .gdb_index section operations.
    A GDB extension.
    The section is in some executables and if present
    is used to quickly map an address or name to
    a skeleton CU or TU.  If present then there are
    .dwo or .dwp files somewhere to make detailed
    debugging possible (up to user code to
    find it/them and deal with them).

    Version 8 built by gdb, so type entries are ok as is.
    Version 7 built by the 'gold' linker and type index
    entries for a CU must be derived othewise, the
    type index is not correct... ? FIXME
    */

/*  Creates a Dwarf_Gdbindex, returning it and
    its values through the pointers. */
int dwarf_gdbindex_header(Dwarf_Debug /*dbg*/,
    Dwarf_Gdbindex * /*gdbindexptr*/,
    Dwarf_Unsigned * /*version*/,
    Dwarf_Unsigned * /*cu_list_offset*/,
    Dwarf_Unsigned * /*types_cu_list_offset*/,
    Dwarf_Unsigned * /*address_area_offset*/,
    Dwarf_Unsigned * /*symbol_table_offset*/,
    Dwarf_Unsigned * /*constant_pool_offset*/,
    Dwarf_Unsigned * /*section_size*/,
    Dwarf_Unsigned * /*unused_reserved*/,
    const char    ** /*section_name*/,
    Dwarf_Error    * /*error*/);

int dwarf_gdbindex_culist_array(Dwarf_Gdbindex /*gdbindexptr*/,
    Dwarf_Unsigned       * /*list_length*/,
    Dwarf_Error          * /*error*/);

/*  entryindex: 0 to list_length-1 */
int dwarf_gdbindex_culist_entry(Dwarf_Gdbindex /*gdbindexptr*/,
    Dwarf_Unsigned   /*entryindex*/,
    Dwarf_Unsigned * /*cu_offset*/,
    Dwarf_Unsigned * /*cu_length*/,
    Dwarf_Error    * /*error*/);

int dwarf_gdbindex_types_culist_array(Dwarf_Gdbindex /*gdbindexptr*/,
    Dwarf_Unsigned            * /*types_list_length*/,
    Dwarf_Error               * /*error*/);

/*  entryindex: 0 to types_list_length -1 */
int dwarf_gdbindex_types_culist_entry(
    Dwarf_Gdbindex   /*gdbindexptr*/,
    Dwarf_Unsigned   /*entryindex*/,
    Dwarf_Unsigned * /*cu_offset*/,
    Dwarf_Unsigned * /*tu_offset*/,
    Dwarf_Unsigned * /*type_signature*/,
    Dwarf_Error    * /*error*/);

int dwarf_gdbindex_addressarea(Dwarf_Gdbindex /*gdbindexptr*/,
    Dwarf_Unsigned            * /*addressarea_list_length*/,
    Dwarf_Error               * /*error*/);

/*    entryindex: 0 to addressarea_list_length-1 */
int dwarf_gdbindex_addressarea_entry(
    Dwarf_Gdbindex   /*gdbindexptr*/,
    Dwarf_Unsigned   /*entryindex*/,
    Dwarf_Unsigned * /*low_adddress*/,
    Dwarf_Unsigned * /*high_address*/,
    Dwarf_Unsigned * /*cu_index*/,
    Dwarf_Error    * /*error*/);

int dwarf_gdbindex_symboltable_array(Dwarf_Gdbindex /*gdbindexptr*/,
    Dwarf_Unsigned            * /*symtab_list_length*/,
    Dwarf_Error               * /*error*/);

/*  entryindex: 0 to symtab_list_length-1 */
int dwarf_gdbindex_symboltable_entry(
    Dwarf_Gdbindex   /*gdbindexptr*/,
    Dwarf_Unsigned   /*entryindex*/,
    Dwarf_Unsigned * /*string_offset*/,
    Dwarf_Unsigned * /*cu_vector_offset*/,
    Dwarf_Error    * /*error*/);

int dwarf_gdbindex_cuvector_length(Dwarf_Gdbindex /*gdbindex*/,
    Dwarf_Unsigned   /*cuvector_offset*/,
    Dwarf_Unsigned * /*innercount*/,
    Dwarf_Error    * /*error*/);


int dwarf_gdbindex_cuvector_inner_attributes(Dwarf_Gdbindex /*gdbindex*/,
    Dwarf_Unsigned   /*cuvector_offset*/,
    Dwarf_Unsigned   /*innerindex*/,
    /* The attr_value is a field of bits. For expanded version
        use  dwarf_gdbindex_cuvector_expand_value() */
    Dwarf_Unsigned * /*attr_value*/,
    Dwarf_Error    * /*error*/);

int dwarf_gdbindex_cuvector_instance_expand_value(Dwarf_Gdbindex /*gdbindex*/,
    Dwarf_Unsigned   /*value*/,
    Dwarf_Unsigned * /*cu_index*/,
    Dwarf_Unsigned * /*reserved1*/,
    Dwarf_Unsigned * /*symbol_kind*/,
    Dwarf_Unsigned * /*is_static*/,
    Dwarf_Error    * /*error*/);


/*  The strings in the pool follow (in memory) the cu index
    set and are NUL terminated. */

int dwarf_gdbindex_string_by_offset(Dwarf_Gdbindex /*gdbindexptr*/,
    Dwarf_Unsigned   /*stringoffset*/,
    const char    ** /*string_ptr*/,
    Dwarf_Error   *  /*error*/);

void dwarf_gdbindex_free(Dwarf_Gdbindex /*gdbindexptr*/);

/*  END gdbindex/debugfission operations. */

/*  START debugfission dwp .debug_cu_index and .debug_tu_index operations. */

int dwarf_get_xu_index_header(Dwarf_Debug /*dbg*/,
    const char *  section_type, /* "tu" or "cu" */
    Dwarf_Xu_Index_Header *     /*xuhdr*/,
    Dwarf_Unsigned *            /*version_number*/,
    Dwarf_Unsigned *            /*offsets_count L*/,
    Dwarf_Unsigned *            /*units_count N*/,
    Dwarf_Unsigned *            /*hash_slots_count M*/,
    const char     **           /*sect_name*/,
    Dwarf_Error *               /*err*/);

int dwarf_get_xu_index_section_type(Dwarf_Xu_Index_Header /*xuhdr*/,
    /*  the function returns a pointer to
        the immutable string "tu" or "cu" via this arg. Do not free.  */
    const char ** /*typename*/,
    /*  the function returns a pointer to
        the immutable section name. Do not free.
        .debug_cu_index or .debug_tu_index */
    const char ** /*sectionname*/,
    Dwarf_Error * /*err*/);

/*  Index values 0 to M-1 are valid. */
int dwarf_get_xu_hash_entry(Dwarf_Xu_Index_Header /*xuhdr*/,
    Dwarf_Unsigned     /*index*/,

    /*  Returns the hash value. 64  bits.  */
    Dwarf_Sig8 *      /*hash_value*/,

    /* returns the index into rows of offset/size tables. */
    Dwarf_Unsigned *  /*index_to_sections*/,
    Dwarf_Error *     /*err*/);

/*  Columns 0 to L-1,  valid. */
int dwarf_get_xu_section_names(Dwarf_Xu_Index_Header /*xuhdr*/,
    /* Row index defined to be row zero. */
    Dwarf_Unsigned  /*column_index*/,
    Dwarf_Unsigned* /*DW_SECT_ number*/,
    const char **   /*DW_SECT_ name*/,
    Dwarf_Error *   /*err*/);

    /* Rows 1 to N col 0 to L-1  are valid */
int dwarf_get_xu_section_offset(Dwarf_Xu_Index_Header /*xuhdr*/,
    Dwarf_Unsigned  /*row_index*/,
    Dwarf_Unsigned  /*column_index*/,
    Dwarf_Unsigned* /*sec_offset*/,
    Dwarf_Unsigned* /*sec_size*/,
    Dwarf_Error *   /*err*/);

void dwarf_xu_header_free(Dwarf_Xu_Index_Header /*xuhdr*/);

/*  Defined larger than necessary. This struct, being visible,
    will be difficult to change: binary compatibility. */
#define DW_FISSION_SECT_COUNT 12

/*  User must allocate this struct, zero it,
    and pass a pointer to it
    into dwarf_get_debugfission_for_cu .  */
struct Dwarf_Debug_Fission_Per_CU_s  {
    /*  Do not free the string. It contains "cu" or "tu". */
    /*  If this is not set (ie, not a CU/TU in  DWP Package File)
        then pcu_type will be NULL.  */
    const char   * pcu_type;
    /*  pcu_index is the index (range 1 to N )
        into the tu/cu table of offsets and the table
        of sizes.  1 to N as the zero index is reserved
        for special purposes.  Not a value one
        actually needs. */
    Dwarf_Unsigned pcu_index;
    Dwarf_Sig8     pcu_hash;  /* 8 byte  */
    /*  [0] has offset and size 0.
        [1]-[8] are DW_SECT_* indexes and the
        values are  the offset and size
        of the respective section contribution
        of a single .dwo object. When pcu_size[n] is
        zero the corresponding section is not present. */
    Dwarf_Unsigned pcu_offset[DW_FISSION_SECT_COUNT];
    Dwarf_Unsigned pcu_size[DW_FISSION_SECT_COUNT];
    Dwarf_Unsigned unused1;
    Dwarf_Unsigned unused2;
};
typedef struct Dwarf_Debug_Fission_Per_CU_s  Dwarf_Debug_Fission_Per_CU ;
/*  For any Dwarf_Die in a compilation unit, return
    the debug fission table data through
    percu_out.   Usually applications
    will pass in the CU die.
    Calling code should zero all of the
    struct Dwarf_Debug_Fission_Per_CU_s before calling this.
    If there is no debugfission data this returns
    DW_DLV_NO_ENTRY (only .dwp objects have debugfission data).  */
int dwarf_get_debugfission_for_die(Dwarf_Die /* die */,
    Dwarf_Debug_Fission_Per_CU * /* percu_out */,
    Dwarf_Error * /* err */);

/* Given a key (hash signature)  from a .o, find the per-cu information
    for the CU with that key. */
int dwarf_get_debugfission_for_key(Dwarf_Debug /*dbg*/,
    Dwarf_Sig8 *                 /*key, hash signature */,
    const char * key_type        /*"cu" or "tu" */,
    Dwarf_Debug_Fission_Per_CU * /*percu_out */,
    Dwarf_Error *                /*err */);

/*  END debugfission dwp .debug_cu_index and .debug_tu_index operations. */


/*  Utility operations */
Dwarf_Unsigned dwarf_errno(Dwarf_Error     /*error*/);

char* dwarf_errmsg(Dwarf_Error    /*error*/);

/*  stringcheck zero is default and means do all
    string length validity checks.
    Call with parameter value 1 to turn off many such checks (and
    increase performance).
    Call with zero for safest running.
    Actual value saved and returned is only 8 bits! Upper bits
    ignored by libdwarf (and zero on return).
    Returns previous value.  */
int dwarf_set_stringcheck(int /*stringcheck*/);

/*  'apply' defaults to 1 and means do all
    'rela' relocations on reading in a dwarf object section with
    such relocations.
    Call with parameter value 0 to turn off application of
    such relocations.
    Since the static linker leaves 'bogus' data in object sections
    with a 'rela' relocation section such data cannot be read
    sensibly without processing the relocations.  Such relocations
    do not exist in executables and shared objects (.so), the
    relocations only exist in plain .o relocatable object files.
    Actual value saved and returned is only 8 bits! Upper bits
    ignored by libdwarf (and zero on return).
    Returns previous value.  */
int dwarf_set_reloc_application(int /*apply*/);

/* Unimplemented */
Dwarf_Handler dwarf_seterrhand(Dwarf_Debug /*dbg*/, Dwarf_Handler /*errhand*/);

/* Unimplemented */
Dwarf_Ptr dwarf_seterrarg(Dwarf_Debug /*dbg*/, Dwarf_Ptr /*errarg*/);

void dwarf_dealloc(Dwarf_Debug /*dbg*/, void* /*space*/,
    Dwarf_Unsigned /*type*/);


/* DWARF Producer Interface */

/*  New form June, 2011. Adds user_data argument. */
typedef int (*Dwarf_Callback_Func)(
    const char*     /*name*/,
    int             /*size*/,
    Dwarf_Unsigned  /*type*/,
    Dwarf_Unsigned  /*flags*/,
    Dwarf_Unsigned  /*link*/,
    Dwarf_Unsigned  /*info*/,
    Dwarf_Unsigned* /*sect_name_index*/,
    void *          /*user_data*/,
    int*            /*error*/);

/*  Returns DW_DLV_OK or DW_DLV_ERROR and
    if DW_DLV_OK returns the Dwarf_P_Debug
    pointer through the dbg_returned argument. */
int dwarf_producer_init(
    Dwarf_Unsigned        /*flags*/,
    Dwarf_Callback_Func   /*func*/,
    Dwarf_Handler         /*errhand*/,
    Dwarf_Ptr             /*errarg*/,
    void *                /*user_data*/,
    const char *isa_name, /* See isa/abi names in pro_init.c */
    const char *dwarf_version, /* V2 V3 V4 or V5. */
    const char *extra,    /* Extra input strings, comma separated. */
    Dwarf_P_Debug *,      /* dbg_returned */
    Dwarf_Error *         /*error*/);

/*  Returns DW_DLV_OK or DW_DLV_ERROR.
    The desired form must be DW_FORM_string (the default)
    or DW_FORM_strp.  */
int dwarf_pro_set_default_string_form(Dwarf_P_Debug /*dbg*/,
    int /*desired_form*/,
    Dwarf_Error*     /*error*/);

/*  the old interface. Still supported. */
Dwarf_Signed dwarf_transform_to_disk_form(Dwarf_P_Debug /*dbg*/,
    Dwarf_Error*     /*error*/);
/*  New September 2016. The preferred interface. */
int dwarf_transform_to_disk_form_a(Dwarf_P_Debug /*dbg*/,
    Dwarf_Signed *   /*nbufs_out*/,
    Dwarf_Error*     /*error*/);

/* New September 2016. Preferred. */
int dwarf_get_section_bytes_a(Dwarf_P_Debug /*dbg*/,
    Dwarf_Signed     /*dwarf_section*/,
    Dwarf_Signed*    /*elf_section_index*/,
    Dwarf_Unsigned*  /*length*/,
    Dwarf_Ptr     *  /*section_bytes*/,
    Dwarf_Error*     /*error*/);

/* Original function.  Checking for error is difficult. */
Dwarf_Ptr dwarf_get_section_bytes(Dwarf_P_Debug /*dbg*/,
    Dwarf_Signed     /*dwarf_section*/,
    Dwarf_Signed*    /*elf_section_index*/,
    Dwarf_Unsigned*  /*length*/,
    Dwarf_Error*     /*error*/);

int  dwarf_get_relocation_info_count(
    Dwarf_P_Debug    /*dbg*/,
    Dwarf_Unsigned * /*count_of_relocation_sections*/,
    int *                /*drd_buffer_version*/,
    Dwarf_Error*     /*error*/);

int dwarf_get_relocation_info(
    Dwarf_P_Debug           /*dbg*/,
    Dwarf_Signed          * /*elf_section_index*/,
    Dwarf_Signed          * /*elf_section_index_link*/,
    Dwarf_Unsigned        * /*relocation_buffer_count*/,
    Dwarf_Relocation_Data * /*reldata_buffer*/,
    Dwarf_Error*            /*error*/);

/* v1:  no drd_length field, enum explicit */
/* v2:  has the drd_length field, enum value in uchar member */
#define DWARF_DRD_BUFFER_VERSION 2

/* Markers are not written  to DWARF2/3/4, they are user
   defined and may be used for any purpose.
*/
Dwarf_Signed dwarf_get_die_markers(
    Dwarf_P_Debug     /*dbg*/,
    Dwarf_P_Marker *  /*marker_list*/,
    Dwarf_Unsigned *  /*marker_count*/,
    Dwarf_Error *     /*error*/);

int dwarf_get_string_attributes_count(Dwarf_P_Debug,
    Dwarf_Unsigned *,
    int *,
    Dwarf_Error *);

int dwarf_get_string_attributes_info(Dwarf_P_Debug,
    Dwarf_Signed *,
    Dwarf_Unsigned *,
    Dwarf_P_String_Attr *,
    Dwarf_Error *);

void dwarf_reset_section_bytes(Dwarf_P_Debug /*dbg*/);

Dwarf_Unsigned dwarf_producer_finish(Dwarf_P_Debug /*dbg*/,
    Dwarf_Error* /*error*/);
int dwarf_producer_finish_a(Dwarf_P_Debug /*dbg*/,
    Dwarf_Error* /*error*/);

/* Producer attribute addition functions. */
Dwarf_P_Attribute dwarf_add_AT_targ_address(Dwarf_P_Debug /*dbg*/,
    Dwarf_P_Die     /*ownerdie*/,
    Dwarf_Half      /*attr*/,
    Dwarf_Unsigned  /*pc_value*/,
    Dwarf_Signed    /*sym_index*/,
    Dwarf_Error*    /*error*/);

Dwarf_P_Attribute dwarf_add_AT_block(Dwarf_P_Debug /*dbg*/,
    Dwarf_P_Die     /*ownerdie*/,
    Dwarf_Half      /*attr*/,
    Dwarf_Small*    /*block_data*/,
    Dwarf_Unsigned  /*block_len*/,
    Dwarf_Error*    /*error*/);

Dwarf_P_Attribute dwarf_add_AT_targ_address_b(Dwarf_P_Debug /*dbg*/,
    Dwarf_P_Die     /*ownerdie*/,
    Dwarf_Half      /*attr*/,
    Dwarf_Unsigned  /*pc_value*/,
    Dwarf_Unsigned  /*sym_index*/,
    Dwarf_Error*    /*error*/);

Dwarf_P_Attribute dwarf_add_AT_ref_address(Dwarf_P_Debug /*dbg*/,
    Dwarf_P_Die     /*ownerdie*/,
    Dwarf_Half      /*attr*/,
    Dwarf_Unsigned  /*pc_value*/,
    Dwarf_Unsigned  /*sym_index*/,
    Dwarf_Error*    /*error*/);

Dwarf_P_Attribute dwarf_add_AT_unsigned_const(Dwarf_P_Debug /*dbg*/,
    Dwarf_P_Die     /*ownerdie*/,
    Dwarf_Half      /*attr*/,
    Dwarf_Unsigned  /*value*/,
    Dwarf_Error*    /*error*/);

Dwarf_P_Attribute dwarf_add_AT_signed_const(Dwarf_P_Debug /*dbg*/,
    Dwarf_P_Die     /*ownerdie*/,
    Dwarf_Half      /*attr*/,
    Dwarf_Signed    /*value*/,
    Dwarf_Error*    /*error*/);

Dwarf_P_Attribute dwarf_add_AT_reference(Dwarf_P_Debug /*dbg*/,
    Dwarf_P_Die     /*ownerdie*/,
    Dwarf_Half      /*attr*/,
    Dwarf_P_Die     /*otherdie*/,
    Dwarf_Error*    /*error*/);

/*  dwarf_add_AT_reference_b allows otherdie to be NULL with
    the assumption the caller will then later call
    dwarf_fixup_AT_reference_die() with a non-null target die.
    New 22 October, 2013
*/
Dwarf_P_Attribute dwarf_add_AT_reference_b(Dwarf_P_Debug /*dbg*/,
    Dwarf_P_Die     /*ownerdie*/,
    Dwarf_Half      /*attr*/,
    Dwarf_P_Die     /*otherdie*/,
    Dwarf_Error*    /*error*/);

/* The following is for out-of-order cu-local
   references.  Allowing nominating the target Dwarf_P_Die
   after calling dwarf_add_AT_reference with a NULL otherdie
   after a single pass thru the DIE generation. Needed
   for forward-references.
   New 22 October, 2013.
*/
int
dwarf_fixup_AT_reference_die(Dwarf_P_Debug /*dbg*/,
    Dwarf_Half    /* attrnum */,
    Dwarf_P_Die   /* sourcedie*/,
    Dwarf_P_Die   /* targetdie*/,
    Dwarf_Error * /*error*/);


Dwarf_P_Attribute dwarf_add_AT_dataref(
    Dwarf_P_Debug   /*dbg*/,
    Dwarf_P_Die     /*ownerdie*/,
    Dwarf_Half      /*attr*/,
    Dwarf_Unsigned  /*pcvalue*/,
    Dwarf_Unsigned  /*sym_index*/,
    Dwarf_Error*    /*error*/);

Dwarf_P_Attribute dwarf_add_AT_const_value_string(Dwarf_P_Die /*ownerdie*/,
    char*           /*string_value*/,
    Dwarf_Error*    /*error*/);

Dwarf_P_Attribute dwarf_add_AT_location_expr(Dwarf_P_Debug /*dbg*/,
    Dwarf_P_Die     /*ownerdie*/,
    Dwarf_Half      /*attr*/,
    Dwarf_P_Expr    /*loc_expr*/,
    Dwarf_Error*    /*error*/);

Dwarf_P_Attribute dwarf_add_AT_string(Dwarf_P_Debug /*dbg*/,
    Dwarf_P_Die     /*ownerdie*/,
    Dwarf_Half      /*attr*/,
    char*           /*string*/,
    Dwarf_Error*     /*error*/);

Dwarf_P_Attribute dwarf_add_AT_flag(Dwarf_P_Debug /*dbg*/,
    Dwarf_P_Die     /*ownerdie*/,
    Dwarf_Half      /*attr*/,
    Dwarf_Small     /*flag*/,
    Dwarf_Error*    /*error*/);

Dwarf_P_Attribute dwarf_add_AT_producer(Dwarf_P_Die /*ownerdie*/,
    char*           /*producer_string*/,
    Dwarf_Error*    /*error*/);

/*  October 2017 for DW_FORM_data16. Usable with any attribute,
    though it should only be in limited use. DWARF5 only.
    Returns DW_DLV_OK on success, DW_DLV_ERROR on failure.
    Returns the new attribute pointer through *return_attr. */
int dwarf_add_AT_data16(Dwarf_P_Die /*ownerdie*/,
    Dwarf_Half    /*attrnum*/,
    Dwarf_Form_Data16 * /* pointstovalue */,
    Dwarf_P_Attribute * /* return_attr */,
    Dwarf_Error * /*error*/);

/* August 2013 sleb creator. For any attribute. */
Dwarf_P_Attribute dwarf_add_AT_any_value_sleb(Dwarf_P_Die /*ownerdie*/,
    Dwarf_Half    /*attrnum*/,
    Dwarf_Signed  /*signed_value*/,
    Dwarf_Error * /*error*/);

/* Original sleb creator. Only for DW_AT_const_value. */
Dwarf_P_Attribute dwarf_add_AT_const_value_signedint(Dwarf_P_Die /*ownerdie*/,
    Dwarf_Signed    /*signed_value*/,
    Dwarf_Error*    /*error*/);

/* August 2013 uleb creator. For any attribute. */
Dwarf_P_Attribute dwarf_add_AT_any_value_uleb(Dwarf_P_Die /*ownerdie*/,
    Dwarf_Half      /*attrnum*/,
    Dwarf_Unsigned  /*signed_value*/,
    Dwarf_Error *   /*error*/);

/* Original uleb creator. Only for DW_AT_const_value. */
Dwarf_P_Attribute dwarf_add_AT_const_value_unsignedint(
    Dwarf_P_Die     /*ownerdie*/,
    Dwarf_Unsigned  /*unsigned_value*/,
    Dwarf_Error*    /*error*/);

Dwarf_P_Attribute dwarf_add_AT_comp_dir(Dwarf_P_Die /*ownerdie*/,
    char*           /*current_working_directory*/,
    Dwarf_Error*    /*error*/);

Dwarf_P_Attribute dwarf_add_AT_name(Dwarf_P_Die    /*die*/,
    char*           /*name*/,
    Dwarf_Error*    /*error*/);

Dwarf_P_Attribute
dwarf_add_AT_with_ref_sig8(
   Dwarf_P_Die   /*ownerdie */,
   Dwarf_Half    /*attrnum */,
   const Dwarf_Sig8 *  /*sig8_in*/,
   Dwarf_Error * /*error*/);


/* Producer line creation functions (.debug_line) */
Dwarf_Unsigned dwarf_add_directory_decl(Dwarf_P_Debug /*dbg*/,
    char*           /*name*/,
    Dwarf_Error*    /*error*/);

Dwarf_Unsigned dwarf_add_file_decl(Dwarf_P_Debug /*dbg*/,
    char*           /*name*/,
    Dwarf_Unsigned  /*dir_index*/,
    Dwarf_Unsigned  /*time_last_modified*/,
    Dwarf_Unsigned  /*length*/,
    Dwarf_Error*    /*error*/);

Dwarf_Unsigned dwarf_add_line_entry_b(Dwarf_P_Debug /*dbg*/,
    Dwarf_Unsigned  /*file_index*/,
    Dwarf_Addr      /*code_address*/,
    Dwarf_Unsigned  /*lineno*/,
    Dwarf_Signed    /*column_number*/,
    Dwarf_Bool      /*is_source_stmt_begin*/,
    Dwarf_Bool      /*is_basic_block_begin*/,
    Dwarf_Bool      /*is_epilogue_begin*/,
    Dwarf_Bool      /*is_prologue_end*/,
    Dwarf_Unsigned  /*isa*/,
    Dwarf_Unsigned  /*discriminator*/,
    Dwarf_Error*    /*error*/);
Dwarf_Unsigned dwarf_add_line_entry(Dwarf_P_Debug /*dbg*/,
    Dwarf_Unsigned  /*file_index*/,
    Dwarf_Addr      /*code_address*/,
    Dwarf_Unsigned  /*lineno*/,
    Dwarf_Signed    /*column_number*/,
    Dwarf_Bool      /*is_source_stmt_begin*/,
    Dwarf_Bool      /*is_basic_block_begin*/,
    Dwarf_Error*    /*error*/);

Dwarf_Unsigned dwarf_lne_set_address(Dwarf_P_Debug /*dbg*/,
    Dwarf_Unsigned  /*offset*/,
    Dwarf_Unsigned  /*symbol_index*/,
    Dwarf_Error*    /*error*/);

Dwarf_Unsigned dwarf_lne_end_sequence(Dwarf_P_Debug /*dbg*/,
    Dwarf_Addr      /*end_address*/,
    Dwarf_Error*    /*error*/);

/* Producer .debug_frame functions */
Dwarf_Unsigned dwarf_add_frame_cie(Dwarf_P_Debug /*dbg*/,
    char*           /*augmenter*/,
    Dwarf_Small     /*code_alignment_factor*/,
    Dwarf_Small     /*data_alignment_factor*/,
    Dwarf_Small     /*return_address_reg*/,
    Dwarf_Ptr       /*initialization_bytes*/,
    Dwarf_Unsigned  /*init_byte_len*/,
    Dwarf_Error*    /*error*/);

Dwarf_Unsigned dwarf_add_frame_fde(
    Dwarf_P_Debug   /*dbg*/,
    Dwarf_P_Fde     /*fde*/,
    Dwarf_P_Die     /*corresponding subprogram die*/,
    Dwarf_Unsigned  /*cie_to_use*/,
    Dwarf_Unsigned  /*virt_addr_of_described_code*/,
    Dwarf_Unsigned  /*length_of_code*/,
    Dwarf_Unsigned  /*symbol_index*/,
    Dwarf_Error*    /*error*/);

Dwarf_Unsigned dwarf_add_frame_fde_b(
    Dwarf_P_Debug  /*dbg*/,
    Dwarf_P_Fde    /*fde*/,
    Dwarf_P_Die    /*die*/,
    Dwarf_Unsigned /*cie*/,
    Dwarf_Addr     /*virt_addr*/,
    Dwarf_Unsigned /*code_len*/,
    Dwarf_Unsigned /*sym_idx*/,
    Dwarf_Unsigned /*sym_idx_of_end*/,
    Dwarf_Addr     /*offset_from_end_sym*/,
    Dwarf_Error*   /*error*/);

Dwarf_Unsigned dwarf_add_frame_info_b(
    Dwarf_P_Debug   /*dbg*/,
    Dwarf_P_Fde     /*fde*/,
    Dwarf_P_Die     /*die*/,
    Dwarf_Unsigned  /*cie*/,
    Dwarf_Addr      /*virt_addr*/,
    Dwarf_Unsigned  /*code_len*/,
    Dwarf_Unsigned  /*symidx*/,
    Dwarf_Unsigned  /*end_symbol */,
    Dwarf_Addr      /*offset_from_end_symbol */,
    Dwarf_Signed    /*offset_into_exception_tables*/,
    Dwarf_Unsigned  /*exception_table_symbol*/,
    Dwarf_Error*    /*error*/);

Dwarf_Unsigned dwarf_add_frame_info(
    Dwarf_P_Debug   /*dbg*/,
    Dwarf_P_Fde     /*fde*/,
    Dwarf_P_Die     /*die*/,
    Dwarf_Unsigned  /*cie*/,
    Dwarf_Addr      /*virt_addr*/,
    Dwarf_Unsigned  /*code_len*/,
    Dwarf_Unsigned  /*symidx*/,
    Dwarf_Signed    /*offset_into_exception_tables*/,
    Dwarf_Unsigned  /*exception_table_symbol*/,
    Dwarf_Error*    /*error*/);

Dwarf_P_Fde dwarf_add_fde_inst(
    Dwarf_P_Fde     /*fde*/,
    Dwarf_Small     /*op*/,
    Dwarf_Unsigned  /*val1*/,
    Dwarf_Unsigned  /*val2*/,
    Dwarf_Error*    /*error*/);

/* New September 17, 2009 */
int dwarf_insert_fde_inst_bytes(
    Dwarf_P_Debug  /*dbg*/,
    Dwarf_P_Fde    /*fde*/,
    Dwarf_Unsigned /*len*/,
    Dwarf_Ptr      /*ibytes*/,
    Dwarf_Error*   /*error*/);


Dwarf_P_Fde dwarf_new_fde(Dwarf_P_Debug    /*dbg*/, Dwarf_Error* /*error*/);

Dwarf_P_Fde dwarf_fde_cfa_offset(
    Dwarf_P_Fde     /*fde*/,
    Dwarf_Unsigned  /*register_number*/,
    Dwarf_Signed    /*offset*/,
    Dwarf_Error*    /*error*/);

/*  die creation & addition routines
    dwarf_new_die_a() new September 2016.
    Preferred over dwarf_new_die(). */
int dwarf_new_die_a(
    Dwarf_P_Debug   /*dbg*/,
    Dwarf_Tag       /*tag*/,
    Dwarf_P_Die     /*parent*/,
    Dwarf_P_Die     /*child*/,
    Dwarf_P_Die     /*left */,
    Dwarf_P_Die     /*right*/,
    Dwarf_P_Die   * /*die_out*/,
    Dwarf_Error*    /*error*/);
Dwarf_P_Die dwarf_new_die(
    Dwarf_P_Debug    /*dbg*/,
    Dwarf_Tag         /*tag*/,
    Dwarf_P_Die     /*parent*/,
    Dwarf_P_Die     /*child*/,
    Dwarf_P_Die     /*left */,
    Dwarf_P_Die     /*right*/,
    Dwarf_Error*    /*error*/);

/* New September 2016. */
int dwarf_add_die_to_debug_a(
    Dwarf_P_Debug   /*dbg*/,
    Dwarf_P_Die     /*die*/,
    Dwarf_Error*    /*error*/);
/*  Original form. Still supported. */
Dwarf_Unsigned dwarf_add_die_to_debug(
    Dwarf_P_Debug   /*dbg*/,
    Dwarf_P_Die     /*die*/,
    Dwarf_Error*    /*error*/);

/* Markers are not written  to DWARF2/3/4, they are user
   defined and may be used for any purpose.
*/
Dwarf_Unsigned dwarf_add_die_marker(
    Dwarf_P_Debug   /*dbg*/,
    Dwarf_P_Die     /*die*/,
    Dwarf_Unsigned  /*marker*/,
    Dwarf_Error *   /*error*/);

Dwarf_Unsigned dwarf_get_die_marker(
    Dwarf_P_Debug   /*dbg*/,
    Dwarf_P_Die     /*die*/,
    Dwarf_Unsigned *  /*marker*/,
    Dwarf_Error *   /*error*/);

/*  New September 2016. Preferred version */
int dwarf_die_link_a(
    Dwarf_P_Die     /*die*/,
    Dwarf_P_Die     /*parent*/,
    Dwarf_P_Die     /*child*/,
    Dwarf_P_Die     /*left*/,
    Dwarf_P_Die     /*right*/,
    Dwarf_Error*    /*error*/);

/*  Original version. Use dwarf_die_link_a() instead. */
Dwarf_P_Die dwarf_die_link(
    Dwarf_P_Die     /*die*/,
    Dwarf_P_Die     /*parent*/,
    Dwarf_P_Die     /*child*/,
    Dwarf_P_Die     /*left*/,
    Dwarf_P_Die     /*right*/,
    Dwarf_Error*    /*error*/);

void dwarf_dealloc_compressed_block(
    Dwarf_P_Debug,
    void *
);

/*  Call this passing in return value from dwarf_uncompress_integer_block()
    to free the space the decompression allocated. */
void dwarf_dealloc_uncompressed_block(
    Dwarf_Debug,
    void *
);

void * dwarf_compress_integer_block(
    Dwarf_P_Debug,    /*dbg*/
    Dwarf_Bool,       /*signed==true (or unsigned)*/
    Dwarf_Small,      /*size of integer units: 8, 16, 32, 64*/
    void*,            /*data*/
    Dwarf_Unsigned,   /*number of elements*/
    Dwarf_Unsigned*,  /*number of bytes in output block*/
    Dwarf_Error*      /*error*/
);

/*  Decode an array of signed leb integers (so of course the
    array is not composed of fixed length values, but is instead
    a sequence of sleb values).
    Returns a DW_DLV_BADADDR on error.
    Otherwise returns a pointer to an array of 32bit integers.
    The signed argument must be non-zero (the decode
    assumes sleb integers in the input data) at this time.
    Size of integer units must be 32 (32 bits each) at this time.
    Number of bytes in block is a byte count (not array count).
    Returns number of units in output block (ie, number of elements
    of the array that the return value points to) thru the argument.  */
void * dwarf_uncompress_integer_block(
    Dwarf_Debug,      /*dbg */
    Dwarf_Bool,       /*signed==true (or unsigned) */
    Dwarf_Small,      /*size of integer units: 8, 16, 32, 64 */
    void*,            /*input data */
    Dwarf_Unsigned,   /*number of bytes in input */
    Dwarf_Unsigned*,  /*number of units in output block */
    Dwarf_Error*      /*error */
);

/* Operations to create location expressions. */
Dwarf_P_Expr dwarf_new_expr(Dwarf_P_Debug /*dbg*/, Dwarf_Error* /*error*/);

void dwarf_expr_reset(
    Dwarf_P_Expr      /*expr*/,
    Dwarf_Error*      /*error*/);

Dwarf_Unsigned dwarf_add_expr_gen(
    Dwarf_P_Expr      /*expr*/,
    Dwarf_Small       /*opcode*/,
    Dwarf_Unsigned    /*val1*/,
    Dwarf_Unsigned    /*val2*/,
    Dwarf_Error*      /*error*/);

Dwarf_Unsigned dwarf_add_expr_addr(
    Dwarf_P_Expr      /*expr*/,
    Dwarf_Unsigned    /*addr*/,
    Dwarf_Signed      /*sym_index*/,
    Dwarf_Error*      /*error*/);

Dwarf_Unsigned dwarf_add_expr_addr_b(
    Dwarf_P_Expr      /*expr*/,
    Dwarf_Unsigned    /*addr*/,
    Dwarf_Unsigned    /*sym_index*/,
    Dwarf_Error*      /*error*/);

Dwarf_Unsigned dwarf_expr_current_offset(
    Dwarf_P_Expr      /*expr*/,
    Dwarf_Error*      /*error*/);

Dwarf_Addr dwarf_expr_into_block(
    Dwarf_P_Expr      /*expr*/,
    Dwarf_Unsigned*   /*length*/,
    Dwarf_Error*      /*error*/);

Dwarf_Unsigned dwarf_add_arange(Dwarf_P_Debug /*dbg*/,
    Dwarf_Addr        /*begin_address*/,
    Dwarf_Unsigned    /*length*/,
    Dwarf_Signed      /*symbol_index*/,
    Dwarf_Error*      /*error*/);

Dwarf_Unsigned dwarf_add_arange_b(
    Dwarf_P_Debug  /*dbg*/,
    Dwarf_Addr     /*begin_address*/,
    Dwarf_Unsigned /*length*/,
    Dwarf_Unsigned /*symbol_index*/,
    Dwarf_Unsigned /*end_symbol_index*/,
    Dwarf_Addr     /*offset_from_end_symbol*/,
    Dwarf_Error *  /*error*/);

Dwarf_Unsigned dwarf_add_pubname(
    Dwarf_P_Debug      /*dbg*/,
    Dwarf_P_Die        /*die*/,
    char*              /*pubname_name*/,
    Dwarf_Error*       /*error*/);

/* Added 17 October 2013.  Introduced in DWARF3. */
Dwarf_Unsigned dwarf_add_pubtype(
    Dwarf_P_Debug      /*dbg*/,
    Dwarf_P_Die        /*die*/,
    char*              /*pubtype_name*/,
    Dwarf_Error*       /*error*/);


Dwarf_Unsigned dwarf_add_funcname(
    Dwarf_P_Debug      /*dbg*/,
    Dwarf_P_Die        /*die*/,
    char*              /*func_name*/,
    Dwarf_Error*       /*error*/);

Dwarf_Unsigned dwarf_add_typename(
    Dwarf_P_Debug     /*dbg*/,
    Dwarf_P_Die       /*die*/,
    char*             /*type_name*/,
    Dwarf_Error*      /*error*/);

Dwarf_Unsigned dwarf_add_varname(
    Dwarf_P_Debug     /*dbg*/,
    Dwarf_P_Die       /*die*/,
    char*             /*var_name*/,
    Dwarf_Error*      /*error*/);

Dwarf_Unsigned dwarf_add_weakname(
    Dwarf_P_Debug    /*dbg*/,
    Dwarf_P_Die      /*die*/,
    char*            /*weak_name*/,
    Dwarf_Error*     /*error*/);

/*  .debug_names producer functions */

/*  dwarf_force_debug_names forces creation
    of .debug_names (if DWARF5 being produced)
    even if empty. Only for testing libdwarf. */
int dwarf_force_debug_names(Dwarf_P_Debug /* dbg */,
    Dwarf_Error*     /*error*/);

/*  Other debug_names functions are needed... FIXME */

/*  end .debug_names producer functions */

/*  .debug_macinfo producer functions
    Functions must be called in right order: the section is output
    In the order these are presented.
*/
int dwarf_def_macro(Dwarf_P_Debug /*dbg*/,
    Dwarf_Unsigned   /*line*/,
    char *           /*macname, with (arglist), no space before (*/,
    char *           /*macvalue*/,
    Dwarf_Error*     /*error*/);

int dwarf_undef_macro(Dwarf_P_Debug /*dbg*/,
    Dwarf_Unsigned   /*line*/,
    char *           /*macname, no arglist, of course*/,
    Dwarf_Error*     /*error*/);

int dwarf_start_macro_file(Dwarf_P_Debug /*dbg*/,
    Dwarf_Unsigned   /*fileindex*/,
    Dwarf_Unsigned   /*linenumber*/,
    Dwarf_Error*     /*error*/);

int dwarf_end_macro_file(Dwarf_P_Debug /*dbg*/,
    Dwarf_Error*     /*error*/);

int dwarf_vendor_ext(Dwarf_P_Debug /*dbg*/,
    Dwarf_Unsigned   /*constant*/,
    char *           /*string*/,
    Dwarf_Error*     /*error*/);

/* end macinfo producer functions */

int dwarf_attr_offset(Dwarf_Die /*die*/,
    Dwarf_Attribute /*attr of above die*/,
    Dwarf_Off     * /*returns offset thru this ptr */,
    Dwarf_Error   * /*error*/);

/*  This is a hack so clients can verify offsets.
    Added April 2005 so that debugger can detect broken offsets
    (which happened in an IRIX executable larger than 2GB
    with MIPSpro 7.3.1.3 toolchain.).
*/
int
dwarf_get_section_max_offsets(Dwarf_Debug /*dbg*/,
    Dwarf_Unsigned * /*debug_info_size*/,
    Dwarf_Unsigned * /*debug_abbrev_size*/,
    Dwarf_Unsigned * /*debug_line_size*/,
    Dwarf_Unsigned * /*debug_loc_size*/,
    Dwarf_Unsigned * /*debug_aranges_size*/,
    Dwarf_Unsigned * /*debug_macinfo_size*/,
    Dwarf_Unsigned * /*debug_pubnames_size*/,
    Dwarf_Unsigned * /*debug_str_size*/,
    Dwarf_Unsigned * /*debug_frame_size*/,
    Dwarf_Unsigned * /*debug_ranges_size*/,
    Dwarf_Unsigned * /*debug_pubtypes_size*/);

/*  New October 2011., adds .debug_types section to the sizes
    returned. */
int
dwarf_get_section_max_offsets_b(Dwarf_Debug /*dbg*/,

    Dwarf_Unsigned * /*debug_info_size*/,
    Dwarf_Unsigned * /*debug_abbrev_size*/,
    Dwarf_Unsigned * /*debug_line_size*/,
    Dwarf_Unsigned * /*debug_loc_size*/,
    Dwarf_Unsigned * /*debug_aranges_size*/,
    Dwarf_Unsigned * /*debug_macinfo_size*/,
    Dwarf_Unsigned * /*debug_pubnames_size*/,
    Dwarf_Unsigned * /*debug_str_size*/,
    Dwarf_Unsigned * /*debug_frame_size*/,
    Dwarf_Unsigned * /*debug_ranges_size*/,
    Dwarf_Unsigned * /*debug_pubtypes_size*/,
    Dwarf_Unsigned * /*debug_types_size*/);

int
dwarf_get_section_max_offsets_c(Dwarf_Debug /*dbg*/,
    Dwarf_Unsigned * /*debug_info_size*/,
    Dwarf_Unsigned * /*debug_abbrev_size*/,
    Dwarf_Unsigned * /*debug_line_size*/,
    Dwarf_Unsigned * /*debug_loc_size*/,
    Dwarf_Unsigned * /*debug_aranges_size*/,
    Dwarf_Unsigned * /*debug_macinfo_size*/,
    Dwarf_Unsigned * /*debug_pubnames_size*/,
    Dwarf_Unsigned * /*debug_str_size*/,
    Dwarf_Unsigned * /*debug_frame_size*/,
    Dwarf_Unsigned * /*debug_ranges_size*/,
    Dwarf_Unsigned * /*debug_pubtypes_size*/,
    Dwarf_Unsigned * /*debug_types_size*/,
    Dwarf_Unsigned * /*debug_macro_size*/,
    Dwarf_Unsigned * /*debug_str_offsets_size*/,
    Dwarf_Unsigned * /*debug_sup_size*/,
    Dwarf_Unsigned * /*debug_cu_index_size*/,
    Dwarf_Unsigned * /*debug_tu_index_size*/);
int
dwarf_get_section_max_offsets_d(Dwarf_Debug /*dbg*/,
    Dwarf_Unsigned * /*debug_info_size*/,
    Dwarf_Unsigned * /*debug_abbrev_size*/,
    Dwarf_Unsigned * /*debug_line_size*/,
    Dwarf_Unsigned * /*debug_loc_size*/,
    Dwarf_Unsigned * /*debug_aranges_size*/,
    Dwarf_Unsigned * /*debug_macinfo_size*/,
    Dwarf_Unsigned * /*debug_pubnames_size*/,
    Dwarf_Unsigned * /*debug_str_size*/,
    Dwarf_Unsigned * /*debug_frame_size*/,
    Dwarf_Unsigned * /*debug_ranges_size*/,
    Dwarf_Unsigned * /*debug_pubtypes_size*/,
    Dwarf_Unsigned * /*debug_types_size*/,
    Dwarf_Unsigned * /*debug_macro_size*/,
    Dwarf_Unsigned * /*debug_str_offsets_size*/,
    Dwarf_Unsigned * /*debug_sup_size*/,
    Dwarf_Unsigned * /*debug_cu_index_size*/,
    Dwarf_Unsigned * /*debug_tu_index_size*/,
    Dwarf_Unsigned * /*debug_names_size*/,
    Dwarf_Unsigned * /*debug_loclists_size*/,
    Dwarf_Unsigned * /*debug_rnglists_size*/);


/*  The 'set' calls here return the original (before any change
    by these set routines) of the respective fields. */
/*  Multiple releases spelled 'initial' as 'inital' .
    The 'inital' spelling should not be used. */
Dwarf_Half dwarf_set_frame_rule_inital_value(Dwarf_Debug /*dbg*/,
    Dwarf_Half /*value*/);
/*  Additional interface with correct 'initial' spelling. */
/*  It is likely you will want to call the following 6 functions
    before accessing any frame information.  All are useful
    to tailor handling of pseudo-registers needed to turn
    frame operation references into simpler forms and to
    reflect ABI specific data.  Of course altering libdwarf.h
    and dwarf.h allow the same capabilities, but such header changes
    do not let one change these values at runtime. */
Dwarf_Half dwarf_set_frame_rule_initial_value(Dwarf_Debug /*dbg*/,
    Dwarf_Half /*value*/);
Dwarf_Half dwarf_set_frame_rule_table_size(Dwarf_Debug /*dbg*/,
    Dwarf_Half /*value*/);
Dwarf_Half dwarf_set_frame_cfa_value(Dwarf_Debug /*dbg*/,
    Dwarf_Half /*value*/);
Dwarf_Half dwarf_set_frame_same_value(Dwarf_Debug /*dbg*/,
    Dwarf_Half /*value*/);
Dwarf_Half dwarf_set_frame_undefined_value(Dwarf_Debug /*dbg*/,
    Dwarf_Half /*value*/);
/*  dwarf_set_default_address_size only sets 'value' if value is
    greater than zero. */
Dwarf_Small dwarf_set_default_address_size(Dwarf_Debug /*dbg*/,
    Dwarf_Small /* value */);

/*  As of April 27, 2009, this version with no diepointer is
    obsolete though supported.  Use dwarf_get_ranges_a() instead. */
int dwarf_get_ranges(Dwarf_Debug /*dbg*/,
    Dwarf_Off /*rangesoffset*/,
    Dwarf_Ranges ** /*rangesbuf*/,
    Dwarf_Signed * /*listlen*/,
    Dwarf_Unsigned * /*bytecount*/,
    Dwarf_Error * /*error*/);

/* This adds the address_size argument. New April 27, 2009 */
int dwarf_get_ranges_a(Dwarf_Debug /*dbg*/,
    Dwarf_Off /*rangesoffset*/,
    Dwarf_Die  /* diepointer */,
    Dwarf_Ranges ** /*rangesbuf*/,
    Dwarf_Signed * /*listlen*/,
    Dwarf_Unsigned * /*bytecount*/,
    Dwarf_Error * /*error*/);

void dwarf_ranges_dealloc(Dwarf_Debug /*dbg*/,
    Dwarf_Ranges * /*rangesbuf*/,
    Dwarf_Signed /*rangecount*/);

/*  New April 2018.
    Allows applications to print the .debug_str_offsets
    section.
    Beginning at starting_offset zero,
    returns data about the first table found.
    The value *next_table_offset is the value
    of the next table (if any), one byte past
    the end of the table whose data is returned..
    Returns DW_DLV_NO_ENTRY if the starting offset
    is past the end of valid data.

    There is no guarantee that there are no non-0 nonsense
    bytes in the section outside of useful tables,
    so this can fail and return nonsense or
    DW_DLV_ERROR  if such garbage exists.
*/

struct Dwarf_Str_Offsets_Table_s;
typedef struct  Dwarf_Str_Offsets_Table_s *  Dwarf_Str_Offsets_Table;

/*  Allocates a struct Dwarf_Str_Offsets_Table_s for the section
    and returns DW_DLV_OK and sets a pointer to the struct through
    the table_data pointer if successful.

    If there is no such section it returns DW_DLV_NO_ENTRY. */
int dwarf_open_str_offsets_table_access(Dwarf_Debug  /*dbg*/,
    Dwarf_Str_Offsets_Table * /*table_data*/,
    Dwarf_Error             * /*error*/);

/*  Close access, free table_data. */
int dwarf_close_str_offsets_table_access(
    Dwarf_Str_Offsets_Table   /*table_data*/,
    Dwarf_Error             * /*error*/);

/*  Call till it returns DW_DLV_NO_ENTRY (normal end)
    or DW_DLV_ERROR (error) and stop.

    On successful call, call dwarf_str_offsets_table_entry()
    to get the individual table values on the now-active table. */
int dwarf_next_str_offsets_table( Dwarf_Str_Offsets_Table /*table_data*/,
    Dwarf_Unsigned * /*unit_length*/,
    Dwarf_Unsigned * /*unit_length_offset*/,
    Dwarf_Unsigned * /*table_start_offset*/,
    Dwarf_Half     * /*entry_size*/,
    Dwarf_Half     * /*version*/,
    Dwarf_Half     * /*padding*/,
    Dwarf_Unsigned * /*table_value_count*/,
    Dwarf_Error    * /*error*/);

/*  Valid index values n:  0 <= n <  table_entry_count
    for the active table */
int dwarf_str_offsets_value_by_index(Dwarf_Str_Offsets_Table /*table_data*/,
    Dwarf_Unsigned   /*index_to_entry*/,
    Dwarf_Unsigned * /*entry_value*/,
    Dwarf_Error    * /*error*/);

/*  After all str_offsets read this reports final
    wasted-bytes count. */
int dwarf_str_offsets_statistics(Dwarf_Str_Offsets_Table /*table_data*/,
    Dwarf_Unsigned * /*wasted_byte_count*/,
    Dwarf_Unsigned * /*table_count*/,
    Dwarf_Error    * /*error*/);



/* The harmless error list is a circular buffer of
   errors we note but which do not stop us from processing
   the object.  Created so dwarfdump or other tools
   can report such inconsequential errors without causing
   anything to stop early. */
#define DW_HARMLESS_ERROR_CIRCULAR_LIST_DEFAULT_SIZE 4
#define DW_HARMLESS_ERROR_MSG_STRING_SIZE   200
/* User code supplies size of array of pointers errmsg_ptrs_array
    in count and the array of pointers (the pointers themselves
    need not be initialized).
    The pointers returned in the array of pointers
    are invalidated by ANY call to libdwarf.
    Use them before making another libdwarf call!
    The array of string pointers passed in always has
    a final null pointer, so if there are N pointers the
    and M actual strings, then MIN(M,N-1) pointers are
    set to point to error strings.  The array of pointers
    to strings always terminates with a NULL pointer.
    If 'count' is passed in zero then errmsg_ptrs_array
    is not touched.

    The function returns DW_DLV_NO_ENTRY if no harmless errors
    were noted so far.  Returns DW_DLV_OK if there are errors.
    Never returns DW_DLV_ERROR.

    Each call empties the error list (discarding all current entries).
    If newerr_count is non-NULL the count of harmless errors
    since the last call is returned through the pointer
    (some may have been discarded or not returned, it is a circular
    list...).
    If DW_DLV_NO_ENTRY is returned none of the arguments
    here are touched or used.
    */
int dwarf_get_harmless_error_list(Dwarf_Debug /*dbg*/,
    unsigned  /*count*/,
    const char ** /*errmsg_ptrs_array*/,
    unsigned * /*newerr_count*/);

/*  Insertion is only for testing the harmless error code, it is not
    necessarily useful otherwise. */
void dwarf_insert_harmless_error(Dwarf_Debug /*dbg*/,
    char * /*newerror*/);

/*  The size of the circular list of strings may be set
    and reset as needed.  If it is shortened excess
    messages are simply dropped.  It returns the previous
    size. If zero passed in the size is unchanged
    and it simply returns the current size  */
unsigned dwarf_set_harmless_error_list_size(Dwarf_Debug /*dbg*/,
    unsigned /*maxcount*/);
/*  The harmless error strings (if any) are freed when the dbg
    is dwarf_finish()ed. */

/*  When the val_in is known these dwarf_get_TAG_name (etc)
    functions return the string corresponding to the val_in passed in
    through the pointer s_out and the value returned is DW_DLV_OK.
    The strings are in static storage
    and must not be freed.
    If DW_DLV_NO_ENTRY is returned the val_in is not known and
    *s_out is not set.  DW_DLV_ERROR is never returned.*/

/* The following copied from a generated dwarf_names.h */

/* BEGIN FILE */
extern int dwarf_get_ACCESS_name(unsigned int /*val_in*/, const char ** /*s_out */);
extern int dwarf_get_ADDR_name(unsigned int /*val_in*/, const char ** /*s_out */);
extern int dwarf_get_ATCF_name(unsigned int /*val_in*/, const char ** /*s_out */);
extern int dwarf_get_ATE_name(unsigned int /*val_in*/, const char ** /*s_out */);
extern int dwarf_get_AT_name(unsigned int /*val_in*/, const char ** /*s_out */);
extern int dwarf_get_CC_name(unsigned int /*val_in*/, const char ** /*s_out */);
extern int dwarf_get_CFA_name(unsigned int /*val_in*/, const char ** /*s_out */);
extern int dwarf_get_children_name(unsigned int /*val_in*/, const char ** /*s_out */);
extern int dwarf_get_CHILDREN_name(unsigned int /*val_in*/, const char ** /*s_out */);
extern int dwarf_get_DEFAULTED_name(unsigned int /*val_in*/, const char ** /*s_out */);
extern int dwarf_get_DSC_name(unsigned int /*val_in*/, const char ** /*s_out */);
extern int dwarf_get_DS_name(unsigned int /*val_in*/, const char ** /*s_out */);
extern int dwarf_get_EH_name(unsigned int /*val_in*/, const char ** /*s_out */);
extern int dwarf_get_END_name(unsigned int /*val_in*/, const char ** /*s_out */);
extern int dwarf_get_FORM_name(unsigned int /*val_in*/, const char ** /*s_out */);
extern int dwarf_get_FRAME_name(unsigned int /*val_in*/, const char ** /*s_out */);
extern int dwarf_get_ID_name(unsigned int /*val_in*/, const char ** /*s_out */);
extern int dwarf_get_IDX_name(unsigned int /*val_in*/, const char ** /*s_out */);
extern int dwarf_get_INL_name(unsigned int /*val_in*/, const char ** /*s_out */);
extern int dwarf_get_ISA_name(unsigned int /*val_in*/, const char ** /*s_out */);
extern int dwarf_get_LANG_name(unsigned int /*val_in*/, const char ** /*s_out */);
extern int dwarf_get_LLE_name(unsigned int /*val_in*/, const char ** /*s_out */);
/*  dwarf_get_LLEX_name is likely just temporary. Not standard. */
extern int dwarf_get_LLEX_name(unsigned int /*val_in*/, const char ** /*s_out */);
extern int dwarf_get_LNCT_name(unsigned int /*val_in*/, const char ** /*s_out */);
extern int dwarf_get_LNE_name(unsigned int /*val_in*/, const char ** /*s_out */);
extern int dwarf_get_LNS_name(unsigned int /*val_in*/, const char ** /*s_out */);
extern int dwarf_get_MACINFO_name(unsigned int /*val_in*/, const char ** /*s_out */);
extern int dwarf_get_MACRO_name(unsigned int /*val_in*/, const char ** /*s_out */);
extern int dwarf_get_OP_name(unsigned int /*val_in*/, const char ** /*s_out */);
extern int dwarf_get_ORD_name(unsigned int /*val_in*/, const char ** /*s_out */);
extern int dwarf_get_RLE_name(unsigned int /*val_in*/, const char ** /*s_out */);
extern int dwarf_get_SECT_name(unsigned int /*val_in*/, const char ** /*s_out */);
extern int dwarf_get_TAG_name(unsigned int /*val_in*/, const char ** /*s_out */);
extern int dwarf_get_UT_name(unsigned int /*val_in*/, const char ** /*s_out */);
extern int dwarf_get_VIRTUALITY_name(unsigned int /*val_in*/, const char ** /*s_out */);
extern int dwarf_get_VIS_name(unsigned int /*val_in*/, const char ** /*s_out */);
/* END FILE */

/* Convert local offset into global offset */
int dwarf_convert_to_global_offset(Dwarf_Attribute  /*attr*/,
    Dwarf_Off        /*offset*/,
    Dwarf_Off*       /*ret_offset*/,
    Dwarf_Error*     /*error*/);

/* Get both offsets (local and global) */
int dwarf_die_offsets(Dwarf_Die     /*die*/,
    Dwarf_Off*    /*global_offset*/,
    Dwarf_Off*    /*local_offset*/,
    Dwarf_Error*  /*error*/);

/* Giving a section name, get its size and address */
int dwarf_get_section_info_by_name(Dwarf_Debug      /*dbg*/,
    const char *     /*section_name*/,
    Dwarf_Addr*      /*section_addr*/,
    Dwarf_Unsigned*  /*section_size*/,
    Dwarf_Error*     /*error*/);

/* Giving a section index, get its size and address */
int dwarf_get_section_info_by_index(Dwarf_Debug      /*dbg*/,
    int              /*section_index*/,
    const char **    /*section_name*/,
    Dwarf_Addr*      /*section_addr*/,
    Dwarf_Unsigned*  /*section_size*/,
    Dwarf_Error*     /*error*/);

/*  Get section count, of object file sections. */
int dwarf_get_section_count(Dwarf_Debug /*dbg*/);



/*  Get the version and offset size of a CU context.
    This is useful as a precursor to
    calling dwarf_get_form_class() at times.  */
int dwarf_get_version_of_die(Dwarf_Die /*die*/,
    Dwarf_Half * /*version*/,
    Dwarf_Half * /*offset_size*/);


int dwarf_discr_list(Dwarf_Debug /*dbg*/,
    Dwarf_Small    * /*blockpointer*/,
    Dwarf_Unsigned   /*blocklen*/,
    Dwarf_Dsc_Head * /*dsc_head_out*/,
    Dwarf_Unsigned * /*dsc_array_length_out*/,
    Dwarf_Error    * /*error*/);

/*  NEW September 2016. Allows easy access to DW_AT_discr_list
    entry. Callers must know which is the appropriate
    one of the following two interfaces, though both
    will work. */
int dwarf_discr_entry_u(Dwarf_Dsc_Head /* dsc */,
    Dwarf_Unsigned   /*entrynum*/,
    Dwarf_Half     * /*out_type*/,
    Dwarf_Unsigned * /*out_discr_low*/,
    Dwarf_Unsigned * /*out_discr_high*/,
    Dwarf_Error    * /*error*/);

/*  NEW September 2016. Allows easy access to DW_AT_discr_list
    entry. */
int dwarf_discr_entry_s(Dwarf_Dsc_Head /* dsc */,
    Dwarf_Unsigned   /*entrynum*/,
    Dwarf_Half     * /*out_type*/,
    Dwarf_Signed   * /*out_discr_low*/,
    Dwarf_Signed   * /*out_discr_high*/,
    Dwarf_Error    * /*error*/);

/*  New May 2017.  So users can find out what groups (dwo or COMDAT)
    are in the object and how much to allocate so one can get the
    group-section map data. */
int dwarf_sec_group_sizes(Dwarf_Debug /*dbg*/,
    Dwarf_Unsigned * /*section_count_out*/,
    Dwarf_Unsigned * /*group_count_out*/,
    Dwarf_Unsigned * /*selected_group_out*/,
    Dwarf_Unsigned * /*map_entry_count_out*/,
    Dwarf_Error    * /*error*/);

/*  New May 2017. Reveals the map between group numbers and section numbers.
    Caller must allocate the arrays with space for 'map_entry_count'
    values and this function fills in the array entries.
    Output ordered by group number and section number.
    */
int dwarf_sec_group_map(Dwarf_Debug /*dbg*/,
    Dwarf_Unsigned   /*map_entry_count*/,
    Dwarf_Unsigned * /*group_numbers_array*/,
    Dwarf_Unsigned * /*sec_numbers_array*/,
    const char    ** /*sec_names_array*/,
    Dwarf_Error    * /*error*/);


/*  These make the  LEB encoding routines visible to libdwarf
    callers. Added November, 2012. */
int dwarf_encode_leb128(Dwarf_Unsigned /*val*/,
    int * /*nbytes*/,
    char * /*space*/,
    int /*splen*/);
int dwarf_encode_signed_leb128(Dwarf_Signed /*val*/,
    int * /*nbytes*/,
    char * /*space*/,
    int /*splen*/);

/*  Record some application command line options in libdwarf.
    This is not arc/argv processing, just precooked setting
    of a flag in libdwarf based on something the application
    wants.  check_verbose_mode of TRUE means do more checking
    and sometimes print errors (from libdwarf).
    Not restricted to a single Dwarf_Debug, it applies
    to the libdwarf the executable is using.
*/
typedef struct {
    Dwarf_Bool check_verbose_mode;
} Dwarf_Cmdline_Options;
extern Dwarf_Cmdline_Options dwarf_cmdline_options;

/* Set libdwarf to reflect some application command line options. */
void dwarf_record_cmdline_options(Dwarf_Cmdline_Options /*options*/);

int dwarf_pro_get_string_stats(Dwarf_P_Debug /*dbg*/,
    Dwarf_Unsigned * /*str_count*/,
    Dwarf_Unsigned * /*str_total_length*/,
    Dwarf_Unsigned * /*count_debug_str*/,
    Dwarf_Unsigned * /*len_debug_str*/,
    Dwarf_Unsigned * /*reused_count*/,
    Dwarf_Unsigned * /*reused_len*/,
    Dwarf_Error    * /*error*/);

#ifdef __cplusplus
}
#endif
#endif /* _LIBDWARF_H */
