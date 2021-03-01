/******************************************************************************
 *
 * Name: acpredef - Information table for ACPI predefined methods and objects
 *
 *****************************************************************************/

/******************************************************************************
 *
 * 1. Copyright Notice
 *
 * Some or all of this work - Copyright (c) 1999 - 2021, Intel Corp.
 * All rights reserved.
 *
 * 2. License
 *
 * 2.1. This is your license from Intel Corp. under its intellectual property
 * rights. You may have additional license terms from the party that provided
 * you this software, covering your right to use that party's intellectual
 * property rights.
 *
 * 2.2. Intel grants, free of charge, to any person ("Licensee") obtaining a
 * copy of the source code appearing in this file ("Covered Code") an
 * irrevocable, perpetual, worldwide license under Intel's copyrights in the
 * base code distributed originally by Intel ("Original Intel Code") to copy,
 * make derivatives, distribute, use and display any portion of the Covered
 * Code in any form, with the right to sublicense such rights; and
 *
 * 2.3. Intel grants Licensee a non-exclusive and non-transferable patent
 * license (with the right to sublicense), under only those claims of Intel
 * patents that are infringed by the Original Intel Code, to make, use, sell,
 * offer to sell, and import the Covered Code and derivative works thereof
 * solely to the minimum extent necessary to exercise the above copyright
 * license, and in no event shall the patent license extend to any additions
 * to or modifications of the Original Intel Code. No other license or right
 * is granted directly or by implication, estoppel or otherwise;
 *
 * The above copyright and patent license is granted only if the following
 * conditions are met:
 *
 * 3. Conditions
 *
 * 3.1. Redistribution of Source with Rights to Further Distribute Source.
 * Redistribution of source code of any substantial portion of the Covered
 * Code or modification with rights to further distribute source must include
 * the above Copyright Notice, the above License, this list of Conditions,
 * and the following Disclaimer and Export Compliance provision. In addition,
 * Licensee must cause all Covered Code to which Licensee contributes to
 * contain a file documenting the changes Licensee made to create that Covered
 * Code and the date of any change. Licensee must include in that file the
 * documentation of any changes made by any predecessor Licensee. Licensee
 * must include a prominent statement that the modification is derived,
 * directly or indirectly, from Original Intel Code.
 *
 * 3.2. Redistribution of Source with no Rights to Further Distribute Source.
 * Redistribution of source code of any substantial portion of the Covered
 * Code or modification without rights to further distribute source must
 * include the following Disclaimer and Export Compliance provision in the
 * documentation and/or other materials provided with distribution. In
 * addition, Licensee may not authorize further sublicense of source of any
 * portion of the Covered Code, and must include terms to the effect that the
 * license from Licensee to its licensee is limited to the intellectual
 * property embodied in the software Licensee provides to its licensee, and
 * not to intellectual property embodied in modifications its licensee may
 * make.
 *
 * 3.3. Redistribution of Executable. Redistribution in executable form of any
 * substantial portion of the Covered Code or modification must reproduce the
 * above Copyright Notice, and the following Disclaimer and Export Compliance
 * provision in the documentation and/or other materials provided with the
 * distribution.
 *
 * 3.4. Intel retains all right, title, and interest in and to the Original
 * Intel Code.
 *
 * 3.5. Neither the name Intel nor any other trademark owned or controlled by
 * Intel shall be used in advertising or otherwise to promote the sale, use or
 * other dealings in products derived from or relating to the Covered Code
 * without prior written authorization from Intel.
 *
 * 4. Disclaimer and Export Compliance
 *
 * 4.1. INTEL MAKES NO WARRANTY OF ANY KIND REGARDING ANY SOFTWARE PROVIDED
 * HERE. ANY SOFTWARE ORIGINATING FROM INTEL OR DERIVED FROM INTEL SOFTWARE
 * IS PROVIDED "AS IS," AND INTEL WILL NOT PROVIDE ANY SUPPORT, ASSISTANCE,
 * INSTALLATION, TRAINING OR OTHER SERVICES. INTEL WILL NOT PROVIDE ANY
 * UPDATES, ENHANCEMENTS OR EXTENSIONS. INTEL SPECIFICALLY DISCLAIMS ANY
 * IMPLIED WARRANTIES OF MERCHANTABILITY, NONINFRINGEMENT AND FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 * 4.2. IN NO EVENT SHALL INTEL HAVE ANY LIABILITY TO LICENSEE, ITS LICENSEES
 * OR ANY OTHER THIRD PARTY, FOR ANY LOST PROFITS, LOST DATA, LOSS OF USE OR
 * COSTS OF PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES, OR FOR ANY INDIRECT,
 * SPECIAL OR CONSEQUENTIAL DAMAGES ARISING OUT OF THIS AGREEMENT, UNDER ANY
 * CAUSE OF ACTION OR THEORY OF LIABILITY, AND IRRESPECTIVE OF WHETHER INTEL
 * HAS ADVANCE NOTICE OF THE POSSIBILITY OF SUCH DAMAGES. THESE LIMITATIONS
 * SHALL APPLY NOTWITHSTANDING THE FAILURE OF THE ESSENTIAL PURPOSE OF ANY
 * LIMITED REMEDY.
 *
 * 4.3. Licensee shall not export, either directly or indirectly, any of this
 * software or system incorporating such software without first obtaining any
 * required license or other approval from the U. S. Department of Commerce or
 * any other agency or department of the United States Government. In the
 * event Licensee exports any such software from the United States or
 * re-exports any such software from a foreign destination, Licensee shall
 * ensure that the distribution and export/re-export of the software is in
 * compliance with all laws, regulations, orders, or other restrictions of the
 * U.S. Export Administration Regulations. Licensee agrees that neither it nor
 * any of its subsidiaries will export/re-export any technical data, process,
 * software, or service, directly or indirectly, to any country for which the
 * United States government or any agency thereof requires an export license,
 * other governmental approval, or letter of assurance, without first obtaining
 * such license, approval or letter.
 *
 *****************************************************************************
 *
 * Alternatively, you may choose to be licensed under the terms of the
 * following license:
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions, and the following disclaimer,
 *    without modification.
 * 2. Redistributions in binary form must reproduce at minimum a disclaimer
 *    substantially similar to the "NO WARRANTY" disclaimer below
 *    ("Disclaimer") and any redistribution must be conditioned upon
 *    including a substantially similar Disclaimer requirement for further
 *    binary redistribution.
 * 3. Neither the names of the above-listed copyright holders nor the names
 *    of any contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Alternatively, you may choose to be licensed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation.
 *
 *****************************************************************************/

#ifndef __ACPREDEF_H__
#define __ACPREDEF_H__

/******************************************************************************
 *
 * Return Package types
 *
 * 1) PTYPE1 packages do not contain subpackages.
 *
 * ACPI_PTYPE1_FIXED: Fixed-length length, 1 or 2 object types:
 *      object type
 *      count
 *      object type
 *      count
 *
 * ACPI_PTYPE1_VAR: Variable-length length. Zero-length package is allowed:
 *      object type (Int/Buf/Ref)
 *
 * ACPI_PTYPE1_OPTION: Package has some required and some optional elements
 *      (Used for _PRW)
 *
 *
 * 2) PTYPE2 packages contain a Variable-length number of subpackages. Each
 *    of the different types describe the contents of each of the subpackages.
 *
 * ACPI_PTYPE2: Each subpackage contains 1 or 2 object types. Zero-length
 *      parent package is allowed:
 *      object type
 *      count
 *      object type
 *      count
 *      (Used for _ALR,_MLS,_PSS,_TRT,_TSS)
 *
 * ACPI_PTYPE2_COUNT: Each subpackage has a count as first element.
 *      Zero-length parent package is allowed:
 *      object type
 *      (Used for _CSD,_PSD,_TSD)
 *
 * ACPI_PTYPE2_PKG_COUNT: Count of subpackages at start, 1 or 2 object types:
 *      object type
 *      count
 *      object type
 *      count
 *      (Used for _CST)
 *
 * ACPI_PTYPE2_FIXED: Each subpackage is of Fixed-length. Zero-length
 *      parent package is allowed.
 *      (Used for _PRT)
 *
 * ACPI_PTYPE2_MIN: Each subpackage has a Variable-length but minimum length.
 *      Zero-length parent package is allowed:
 *      (Used for _HPX)
 *
 * ACPI_PTYPE2_REV_FIXED: Revision at start, each subpackage is Fixed-length
 *      (Used for _ART, _FPS)
 *
 * ACPI_PTYPE2_FIX_VAR: Each subpackage consists of some fixed-length elements
 *      followed by an optional element. Zero-length parent package is allowed.
 *      object type
 *      count
 *      object type
 *      count = 0 (optional)
 *      (Used for _DLM)
 *
 * ACPI_PTYPE2_VAR_VAR: Variable number of subpackages, each of either a
 *      constant or variable length. The subpackages are preceded by a
 *      constant number of objects.
 *      (Used for _LPI, _RDI)
 *
 * ACPI_PTYPE2_UUID_PAIR: Each subpackage is preceded by a UUID Buffer. The UUID
 *      defines the format of the package. Zero-length parent package is
 *      allowed.
 *      (Used for _DSD)
 *
 *****************************************************************************/

enum AcpiReturnPackageTypes { ACPI_PTYPE1_FIXED = 1, ACPI_PTYPE1_VAR = 2, ACPI_PTYPE1_OPTION = 3, ACPI_PTYPE2 = 4, ACPI_PTYPE2_COUNT = 5, ACPI_PTYPE2_PKG_COUNT = 6, ACPI_PTYPE2_FIXED = 7, ACPI_PTYPE2_MIN = 8, ACPI_PTYPE2_REV_FIXED = 9, ACPI_PTYPE2_FIX_VAR = 10, ACPI_PTYPE2_VAR_VAR = 11, ACPI_PTYPE2_UUID_PAIR = 12, ACPI_PTYPE_CUSTOM = 13 };

/* Support macros for users of the predefined info table */

#define METHOD_PREDEF_ARGS_MAX 5
#define METHOD_ARG_BIT_WIDTH 3
#define METHOD_ARG_MASK 0x0007
#define ARG_COUNT_IS_MINIMUM 0x8000
#define METHOD_MAX_ARG_TYPE ACPI_TYPE_PACKAGE

#define METHOD_GET_ARG_COUNT(ArgList) ((ArgList)&METHOD_ARG_MASK)
#define METHOD_GET_NEXT_TYPE(ArgList) (((ArgList) >>= METHOD_ARG_BIT_WIDTH) & METHOD_ARG_MASK)

/* Macros used to build the predefined info table */

#define METHOD_0ARGS 0
#define METHOD_1ARGS(a1) (1 | (a1 << 3))
#define METHOD_2ARGS(a1, a2) (2 | (a1 << 3) | (a2 << 6))
#define METHOD_3ARGS(a1, a2, a3) (3 | (a1 << 3) | (a2 << 6) | (a3 << 9))
#define METHOD_4ARGS(a1, a2, a3, a4) (4 | (a1 << 3) | (a2 << 6) | (a3 << 9) | (a4 << 12))
#define METHOD_5ARGS(a1, a2, a3, a4, a5) (5 | (a1 << 3) | (a2 << 6) | (a3 << 9) | (a4 << 12) | (a5 << 15))

#define METHOD_RETURNS(type) (type)
#define METHOD_NO_RETURN_VALUE 0

#define PACKAGE_INFO(a, b, c, d, e, f)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 \
    {                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  \
        { {(a), (b), (c), (d)}, ((((UINT16)(f)) << 8) | (e)), 0 }                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                      \
    }

/* Support macros for the resource descriptor info table */

#define WIDTH_1 0x0001
#define WIDTH_2 0x0002
#define WIDTH_3 0x0004
#define WIDTH_8 0x0008
#define WIDTH_16 0x0010
#define WIDTH_32 0x0020
#define WIDTH_64 0x0040
#define VARIABLE_DATA 0x0080
#define NUM_RESOURCE_WIDTHS 8

#define WIDTH_ADDRESS WIDTH_16 | WIDTH_32 | WIDTH_64

#ifdef ACPI_CREATE_PREDEFINED_TABLE
/******************************************************************************
 *
 * Predefined method/object information table.
 *
 * These are the names that can actually be evaluated via AcpiEvaluateObject.
 * Not present in this table are the following:
 *
 *      1) Predefined/Reserved names that are not usually evaluated via
 *         AcpiEvaluateObject:
 *              _Lxx and _Exx GPE methods
 *              _Qxx EC methods
 *              _T_x compiler temporary variables
 *              _Wxx wake events
 *
 *      2) Predefined names that never actually exist within the AML code:
 *              Predefined resource descriptor field names
 *
 *      3) Predefined names that are implemented within ACPICA:
 *              _OSI
 *
 * The main entries in the table each contain the following items:
 *
 * Name                 - The ACPI reserved name
 * ArgumentList         - Contains (in 16 bits), the number of required
 *                        arguments to the method (3 bits), and a 3-bit type
 *                        field for each argument (up to 4 arguments). The
 *                        METHOD_?ARGS macros generate the correct packed data.
 * ExpectedBtypes       - Allowed type(s) for the return value.
 *                        0 means that no return value is expected.
 *
 * For methods that return packages, the next entry in the table contains
 * information about the expected structure of the package. This information
 * is saved here (rather than in a separate table) in order to minimize the
 * overall size of the stored data.
 *
 * Note: The additional braces are intended to promote portability.
 *
 * Note2: Table is used by the kernel-resident subsystem, the iASL compiler,
 * and the AcpiHelp utility.
 *
 * TBD: _PRT - currently ignore reversed entries. Attempt to fix in nsrepair.
 * Possibly fixing package elements like _BIF, etc.
 *
 *****************************************************************************/

const ACPI_PREDEFINED_INFO AcpiGbl_PredefinedMethods[] = {
    {{{'_', 'A', 'C', '0'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_INTEGER)}},

    {{{'_', 'A', 'C', '1'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_INTEGER)}},

    {{{'_', 'A', 'C', '2'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_INTEGER)}},

    {{{'_', 'A', 'C', '3'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_INTEGER)}},

    {{{'_', 'A', 'C', '4'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_INTEGER)}},

    {{{'_', 'A', 'C', '5'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_INTEGER)}},

    {{{'_', 'A', 'C', '6'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_INTEGER)}},

    {{{'_', 'A', 'C', '7'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_INTEGER)}},

    {{{'_', 'A', 'C', '8'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_INTEGER)}},

    {{{'_', 'A', 'C', '9'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_INTEGER)}},

    {{{'_', 'A', 'D', 'R'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_INTEGER)}},

    {{{'_', 'A', 'E', 'I'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_BUFFER)}},

    {{{'_', 'A', 'L', '0'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_PACKAGE)}}, /* Variable-length (Refs) */
    PACKAGE_INFO(ACPI_PTYPE1_VAR, ACPI_RTYPE_REFERENCE, 0, 0, 0, 0),

    {{{'_', 'A', 'L', '1'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_PACKAGE)}}, /* Variable-length (Refs) */
    PACKAGE_INFO(ACPI_PTYPE1_VAR, ACPI_RTYPE_REFERENCE, 0, 0, 0, 0),

    {{{'_', 'A', 'L', '2'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_PACKAGE)}}, /* Variable-length (Refs) */
    PACKAGE_INFO(ACPI_PTYPE1_VAR, ACPI_RTYPE_REFERENCE, 0, 0, 0, 0),

    {{{'_', 'A', 'L', '3'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_PACKAGE)}}, /* Variable-length (Refs) */
    PACKAGE_INFO(ACPI_PTYPE1_VAR, ACPI_RTYPE_REFERENCE, 0, 0, 0, 0),

    {{{'_', 'A', 'L', '4'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_PACKAGE)}}, /* Variable-length (Refs) */
    PACKAGE_INFO(ACPI_PTYPE1_VAR, ACPI_RTYPE_REFERENCE, 0, 0, 0, 0),

    {{{'_', 'A', 'L', '5'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_PACKAGE)}}, /* Variable-length (Refs) */
    PACKAGE_INFO(ACPI_PTYPE1_VAR, ACPI_RTYPE_REFERENCE, 0, 0, 0, 0),

    {{{'_', 'A', 'L', '6'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_PACKAGE)}}, /* Variable-length (Refs) */
    PACKAGE_INFO(ACPI_PTYPE1_VAR, ACPI_RTYPE_REFERENCE, 0, 0, 0, 0),

    {{{'_', 'A', 'L', '7'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_PACKAGE)}}, /* Variable-length (Refs) */
    PACKAGE_INFO(ACPI_PTYPE1_VAR, ACPI_RTYPE_REFERENCE, 0, 0, 0, 0),

    {{{'_', 'A', 'L', '8'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_PACKAGE)}}, /* Variable-length (Refs) */
    PACKAGE_INFO(ACPI_PTYPE1_VAR, ACPI_RTYPE_REFERENCE, 0, 0, 0, 0),

    {{{'_', 'A', 'L', '9'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_PACKAGE)}}, /* Variable-length (Refs) */
    PACKAGE_INFO(ACPI_PTYPE1_VAR, ACPI_RTYPE_REFERENCE, 0, 0, 0, 0),

    {{{'_', 'A', 'L', 'C'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_INTEGER)}},

    {{{'_', 'A', 'L', 'I'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_INTEGER)}},

    {{{'_', 'A', 'L', 'P'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_INTEGER)}},

    {{{'_', 'A', 'L', 'R'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_PACKAGE)}}, /* Variable-length (Pkgs) each 2 (Ints) */
    PACKAGE_INFO(ACPI_PTYPE2, ACPI_RTYPE_INTEGER, 2, 0, 0, 0),

    {{{'_', 'A', 'L', 'T'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_INTEGER)}},

    {{{'_', 'A', 'R', 'T'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_PACKAGE)}}, /* Variable-length (1 Int(rev), n Pkg (2 Ref/11 Int) */
    PACKAGE_INFO(ACPI_PTYPE2_REV_FIXED, ACPI_RTYPE_REFERENCE, 2, ACPI_RTYPE_INTEGER, 11, 0),

    {{{'_', 'B', 'B', 'N'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_INTEGER)}},

    {{{'_', 'B', 'C', 'L'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_PACKAGE)}}, /* Variable-length (Ints) */
    PACKAGE_INFO(ACPI_PTYPE1_VAR, ACPI_RTYPE_INTEGER, 0, 0, 0, 0),

    {{{'_', 'B', 'C', 'M'}, METHOD_1ARGS(ACPI_TYPE_INTEGER), METHOD_NO_RETURN_VALUE}},

    {{{'_', 'B', 'C', 'T'}, METHOD_1ARGS(ACPI_TYPE_INTEGER), METHOD_RETURNS(ACPI_RTYPE_INTEGER)}},

    {{{'_', 'B', 'D', 'N'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_INTEGER)}},

    {{{'_', 'B', 'F', 'S'}, METHOD_1ARGS(ACPI_TYPE_INTEGER), METHOD_NO_RETURN_VALUE}},

    {{{'_', 'B', 'I', 'F'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_PACKAGE)}}, /* Fixed-length (9 Int),(4 Str) */
    PACKAGE_INFO(ACPI_PTYPE1_FIXED, ACPI_RTYPE_INTEGER, 9, ACPI_RTYPE_STRING, 4, 0),

    {{{'_', 'B', 'I', 'X'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_PACKAGE)}}, /* Fixed-length (16 Int),(4 Str) */
    PACKAGE_INFO(ACPI_PTYPE_CUSTOM, ACPI_RTYPE_INTEGER, 16, ACPI_RTYPE_STRING, 4, 0),

    {{{'_', 'B', 'L', 'T'}, METHOD_3ARGS(ACPI_TYPE_INTEGER, ACPI_TYPE_INTEGER, ACPI_TYPE_INTEGER), METHOD_NO_RETURN_VALUE}},

    {{{'_', 'B', 'M', 'A'}, METHOD_1ARGS(ACPI_TYPE_INTEGER), METHOD_RETURNS(ACPI_RTYPE_INTEGER)}},

    {{{'_', 'B', 'M', 'C'}, METHOD_1ARGS(ACPI_TYPE_INTEGER), METHOD_NO_RETURN_VALUE}},

    {{{'_', 'B', 'M', 'D'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_PACKAGE)}}, /* Fixed-length (5 Int) */
    PACKAGE_INFO(ACPI_PTYPE1_FIXED, ACPI_RTYPE_INTEGER, 5, 0, 0, 0),

    {{{'_', 'B', 'M', 'S'}, METHOD_1ARGS(ACPI_TYPE_INTEGER), METHOD_RETURNS(ACPI_RTYPE_INTEGER)}},

    {{{'_', 'B', 'Q', 'C'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_INTEGER)}},

    {{{'_', 'B', 'S', 'T'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_PACKAGE)}}, /* Fixed-length (4 Int) */
    PACKAGE_INFO(ACPI_PTYPE1_FIXED, ACPI_RTYPE_INTEGER, 4, 0, 0, 0),

    {{{'_', 'B', 'T', 'H'},
      METHOD_1ARGS(ACPI_TYPE_INTEGER), /* ACPI 6.0 */
      METHOD_NO_RETURN_VALUE}},

    {{{'_', 'B', 'T', 'M'}, METHOD_1ARGS(ACPI_TYPE_INTEGER), METHOD_RETURNS(ACPI_RTYPE_INTEGER)}},

    {{{'_', 'B', 'T', 'P'}, METHOD_1ARGS(ACPI_TYPE_INTEGER), METHOD_NO_RETURN_VALUE}},

    {{{'_', 'C', 'B', 'A'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_INTEGER)}}, /* See PCI firmware spec 3.0 */

    {{{'_', 'C', 'C', 'A'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_INTEGER)}}, /* ACPI 5.1 */

    {{{'_', 'C', 'D', 'M'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_INTEGER)}},

    {{{'_', 'C', 'I', 'D'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_INTEGER | ACPI_RTYPE_STRING | ACPI_RTYPE_PACKAGE)}}, /* Variable-length (Ints/Strs) */
    PACKAGE_INFO(ACPI_PTYPE1_VAR, ACPI_RTYPE_INTEGER | ACPI_RTYPE_STRING, 0, 0, 0, 0),

    {{{'_', 'C', 'L', 'S'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_PACKAGE)}}, /* Fixed-length (3 Int) */
    PACKAGE_INFO(ACPI_PTYPE1_FIXED, ACPI_RTYPE_INTEGER, 3, 0, 0, 0),

    {{{'_', 'C', 'P', 'C'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_PACKAGE)}}, /* Variable-length (Ints/Bufs) */
    PACKAGE_INFO(ACPI_PTYPE1_VAR, ACPI_RTYPE_INTEGER | ACPI_RTYPE_BUFFER, 0, 0, 0, 0),

    {{{'_', 'C', 'R', '3'},
      METHOD_0ARGS, /* ACPI 6.0 */
      METHOD_RETURNS(ACPI_RTYPE_INTEGER)}},

    {{{'_', 'C', 'R', 'S'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_BUFFER)}},

    {{{'_', 'C', 'R', 'T'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_INTEGER)}},

    {{{'_', 'C', 'S', 'D'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_PACKAGE)}}, /* Variable-length (1 Int(n), n-1 Int) */
    PACKAGE_INFO(ACPI_PTYPE2_COUNT, ACPI_RTYPE_INTEGER, 0, 0, 0, 0),

    {{{'_', 'C', 'S', 'T'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_PACKAGE)}}, /* Variable-length (1 Int(n), n Pkg (1 Buf/3 Int) */
    PACKAGE_INFO(ACPI_PTYPE2_PKG_COUNT, ACPI_RTYPE_BUFFER, 1, ACPI_RTYPE_INTEGER, 3, 0),

    {{{'_', 'C', 'W', 'S'}, METHOD_1ARGS(ACPI_TYPE_INTEGER), METHOD_RETURNS(ACPI_RTYPE_INTEGER)}},

    {{{'_', 'D', 'C', 'K'}, METHOD_1ARGS(ACPI_TYPE_INTEGER), METHOD_RETURNS(ACPI_RTYPE_INTEGER)}},

    {{{'_', 'D', 'C', 'S'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_INTEGER)}},

    {{{'_', 'D', 'D', 'C'}, METHOD_1ARGS(ACPI_TYPE_INTEGER), METHOD_RETURNS(ACPI_RTYPE_INTEGER | ACPI_RTYPE_BUFFER)}},

    {{{'_', 'D', 'D', 'N'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_STRING)}},

    {{{'_', 'D', 'E', 'P'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_PACKAGE)}}, /* Variable-length (Refs) */
    PACKAGE_INFO(ACPI_PTYPE1_VAR, ACPI_RTYPE_REFERENCE, 0, 0, 0, 0),

    {{{'_', 'D', 'G', 'S'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_INTEGER)}},

    {{{'_', 'D', 'I', 'S'}, METHOD_0ARGS, METHOD_NO_RETURN_VALUE}},

    {{{'_', 'D', 'L', 'M'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_PACKAGE)}}, /* Variable-length (Pkgs) each (1 Ref, 0/1 Optional Buf/Ref) */
    PACKAGE_INFO(ACPI_PTYPE2_FIX_VAR, ACPI_RTYPE_REFERENCE, 1, ACPI_RTYPE_REFERENCE | ACPI_RTYPE_BUFFER, 0, 0),

    {{{'_', 'D', 'M', 'A'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_BUFFER)}},

    {{{'_', 'D', 'O', 'D'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_PACKAGE)}}, /* Variable-length (Ints) */
    PACKAGE_INFO(ACPI_PTYPE1_VAR, ACPI_RTYPE_INTEGER, 0, 0, 0, 0),

    {{{'_', 'D', 'O', 'S'}, METHOD_1ARGS(ACPI_TYPE_INTEGER), METHOD_NO_RETURN_VALUE}},

    {{{'_', 'D', 'S', 'D'},
      METHOD_0ARGS,                         /* ACPI 6.0 */
      METHOD_RETURNS(ACPI_RTYPE_PACKAGE)}}, /* Variable-length (Pkgs) each: 1 Buf, 1 Pkg */
    PACKAGE_INFO(ACPI_PTYPE2_UUID_PAIR, ACPI_RTYPE_BUFFER, 1, ACPI_RTYPE_PACKAGE, 1, 0),

    {{{'_', 'D', 'S', 'M'}, METHOD_4ARGS(ACPI_TYPE_BUFFER, ACPI_TYPE_INTEGER, ACPI_TYPE_INTEGER, ACPI_TYPE_PACKAGE), METHOD_RETURNS(ACPI_RTYPE_ALL)}}, /* Must return a value, but it can be of any type */

    {{{'_', 'D', 'S', 'S'}, METHOD_1ARGS(ACPI_TYPE_INTEGER), METHOD_NO_RETURN_VALUE}},

    {{{'_', 'D', 'S', 'W'}, METHOD_3ARGS(ACPI_TYPE_INTEGER, ACPI_TYPE_INTEGER, ACPI_TYPE_INTEGER), METHOD_NO_RETURN_VALUE}},

    {{{'_', 'D', 'T', 'I'}, METHOD_1ARGS(ACPI_TYPE_INTEGER), METHOD_NO_RETURN_VALUE}},

    {{{'_', 'E', 'C', '_'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_INTEGER)}},

    {{{'_', 'E', 'D', 'L'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_PACKAGE)}}, /* Variable-length (Refs)*/
    PACKAGE_INFO(ACPI_PTYPE1_VAR, ACPI_RTYPE_REFERENCE, 0, 0, 0, 0),

    {{{'_', 'E', 'J', '0'}, METHOD_1ARGS(ACPI_TYPE_INTEGER), METHOD_NO_RETURN_VALUE}},

    {{{'_', 'E', 'J', '1'}, METHOD_1ARGS(ACPI_TYPE_INTEGER), METHOD_NO_RETURN_VALUE}},

    {{{'_', 'E', 'J', '2'}, METHOD_1ARGS(ACPI_TYPE_INTEGER), METHOD_NO_RETURN_VALUE}},

    {{{'_', 'E', 'J', '3'}, METHOD_1ARGS(ACPI_TYPE_INTEGER), METHOD_NO_RETURN_VALUE}},

    {{{'_', 'E', 'J', '4'}, METHOD_1ARGS(ACPI_TYPE_INTEGER), METHOD_NO_RETURN_VALUE}},

    {{{'_', 'E', 'J', 'D'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_STRING)}},

    {{{'_', 'E', 'R', 'R'}, METHOD_3ARGS(ACPI_TYPE_INTEGER, ACPI_TYPE_STRING, ACPI_TYPE_INTEGER), METHOD_RETURNS(ACPI_RTYPE_INTEGER)}}, /* Internal use only, used by ACPICA test suites */

    {{{'_', 'E', 'V', 'T'}, METHOD_1ARGS(ACPI_TYPE_INTEGER), METHOD_NO_RETURN_VALUE}},

    {{{'_', 'F', 'D', 'E'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_BUFFER)}},

    {{{'_', 'F', 'D', 'I'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_PACKAGE)}}, /* Fixed-length (16 Int) */
    PACKAGE_INFO(ACPI_PTYPE1_FIXED, ACPI_RTYPE_INTEGER, 16, 0, 0, 0),

    {{{'_', 'F', 'D', 'M'}, METHOD_1ARGS(ACPI_TYPE_INTEGER), METHOD_NO_RETURN_VALUE}},

    {{{'_', 'F', 'I', 'F'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_PACKAGE)}}, /* Fixed-length (4 Int) */
    PACKAGE_INFO(ACPI_PTYPE1_FIXED, ACPI_RTYPE_INTEGER, 4, 0, 0, 0),

    {{{'_', 'F', 'I', 'T'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_BUFFER)}}, /* ACPI 6.0 */

    {{{'_', 'F', 'I', 'X'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_PACKAGE)}}, /* Variable-length (Ints) */
    PACKAGE_INFO(ACPI_PTYPE1_VAR, ACPI_RTYPE_INTEGER, 0, 0, 0, 0),

    {{{'_', 'F', 'P', 'S'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_PACKAGE)}}, /* Variable-length (1 Int(rev), n Pkg (5 Int) */
    PACKAGE_INFO(ACPI_PTYPE2_REV_FIXED, ACPI_RTYPE_INTEGER, 5, 0, 0, 0),

    {{{'_', 'F', 'S', 'L'}, METHOD_1ARGS(ACPI_TYPE_INTEGER), METHOD_NO_RETURN_VALUE}},

    {{{'_', 'F', 'S', 'T'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_PACKAGE)}}, /* Fixed-length (3 Int) */
    PACKAGE_INFO(ACPI_PTYPE1_FIXED, ACPI_RTYPE_INTEGER, 3, 0, 0, 0),

    {{{'_', 'G', 'A', 'I'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_INTEGER)}},

    {{{'_', 'G', 'C', 'P'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_INTEGER)}},

    {{{'_', 'G', 'H', 'L'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_INTEGER)}},

    {{{'_', 'G', 'L', 'K'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_INTEGER)}},

    {{{'_', 'G', 'P', 'D'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_INTEGER)}},

    {{{'_', 'G', 'P', 'E'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_INTEGER)}}, /* _GPE method, not _GPE scope */

    {{{'_', 'G', 'R', 'T'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_BUFFER)}},

    {{{'_', 'G', 'S', 'B'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_INTEGER)}},

    {{{'_', 'G', 'T', 'F'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_BUFFER)}},

    {{{'_', 'G', 'T', 'M'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_BUFFER)}},

    {{{'_', 'G', 'T', 'S'}, METHOD_1ARGS(ACPI_TYPE_INTEGER), METHOD_NO_RETURN_VALUE}},

    {{{'_', 'G', 'W', 'S'}, METHOD_1ARGS(ACPI_TYPE_INTEGER), METHOD_RETURNS(ACPI_RTYPE_INTEGER)}},

    {{{'_', 'H', 'I', 'D'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_INTEGER | ACPI_RTYPE_STRING)}},

    {{{'_', 'H', 'M', 'A'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_BUFFER)}},

    {{{'_', 'H', 'O', 'T'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_INTEGER)}},

    {{{'_', 'H', 'P', 'P'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_PACKAGE)}}, /* Fixed-length (4 Int) */
    PACKAGE_INFO(ACPI_PTYPE1_FIXED, ACPI_RTYPE_INTEGER, 4, 0, 0, 0),

    /*
     * For _HPX, a single package is returned, containing a variable-length number
     * of subpackages. Each subpackage contains a PCI record setting.
     * There are several different type of record settings, of different
     * lengths, but all elements of all settings are Integers.
     */
    {{{'_', 'H', 'P', 'X'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_PACKAGE)}}, /* Variable-length (Pkgs) each (var Ints) */
    PACKAGE_INFO(ACPI_PTYPE2_MIN, ACPI_RTYPE_INTEGER, 5, 0, 0, 0),

    {{{'_', 'H', 'R', 'V'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_INTEGER)}},

    {{{'_', 'I', 'F', 'T'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_INTEGER)}}, /* See IPMI spec */

    {{{'_', 'I', 'N', 'I'}, METHOD_0ARGS, METHOD_NO_RETURN_VALUE}},

    {{{'_', 'I', 'R', 'C'}, METHOD_0ARGS, METHOD_NO_RETURN_VALUE}},

    {{{'_', 'L', 'C', 'K'}, METHOD_1ARGS(ACPI_TYPE_INTEGER), METHOD_NO_RETURN_VALUE}},

    {{{'_', 'L', 'I', 'D'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_INTEGER)}},

    {{{'_', 'L', 'P', 'D'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_PACKAGE)}}, /* Variable-length (1 Int(rev), n Pkg (2 Int) */
    PACKAGE_INFO(ACPI_PTYPE2_REV_FIXED, ACPI_RTYPE_INTEGER, 2, 0, 0, 0),

    {{{'_', 'L', 'P', 'I'},
      METHOD_0ARGS,                         /* ACPI 6.0 */
      METHOD_RETURNS(ACPI_RTYPE_PACKAGE)}}, /* Variable-length (3 Int, n Pkg (10 Int/Buf) */
    PACKAGE_INFO(ACPI_PTYPE2_VAR_VAR, ACPI_RTYPE_INTEGER, 3, ACPI_RTYPE_INTEGER | ACPI_RTYPE_BUFFER | ACPI_RTYPE_STRING, 10, 0),

    {{{'_', 'L', 'S', 'I'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_PACKAGE)}},
    PACKAGE_INFO(ACPI_PTYPE1_FIXED, ACPI_RTYPE_INTEGER, 3, 0, 0, 0),

    {{{'_', 'L', 'S', 'R'}, METHOD_2ARGS(ACPI_TYPE_INTEGER, ACPI_TYPE_INTEGER), METHOD_RETURNS(ACPI_RTYPE_PACKAGE)}},
    PACKAGE_INFO(ACPI_PTYPE1_FIXED, ACPI_RTYPE_INTEGER, 1, ACPI_RTYPE_BUFFER, 1, 0),

    {{{'_', 'L', 'S', 'W'}, METHOD_3ARGS(ACPI_TYPE_INTEGER, ACPI_TYPE_INTEGER, ACPI_TYPE_BUFFER), METHOD_RETURNS(ACPI_RTYPE_INTEGER)}},

    {{{'_', 'M', 'A', 'T'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_BUFFER)}},

    {{{'_', 'M', 'B', 'M'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_PACKAGE)}}, /* Fixed-length (8 Int) */
    PACKAGE_INFO(ACPI_PTYPE1_FIXED, ACPI_RTYPE_INTEGER, 8, 0, 0, 0),

    {{{'_', 'M', 'L', 'S'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_PACKAGE)}}, /* Variable-length (Pkgs) each (1 Str/1 Buf) */
    PACKAGE_INFO(ACPI_PTYPE2, ACPI_RTYPE_STRING, 1, ACPI_RTYPE_BUFFER, 1, 0),

    {{{'_', 'M', 'S', 'G'}, METHOD_1ARGS(ACPI_TYPE_INTEGER), METHOD_NO_RETURN_VALUE}},

    {{{'_', 'M', 'S', 'M'}, METHOD_4ARGS(ACPI_TYPE_INTEGER, ACPI_TYPE_INTEGER, ACPI_TYPE_INTEGER, ACPI_TYPE_INTEGER), METHOD_RETURNS(ACPI_RTYPE_INTEGER)}},

    {{{'_', 'M', 'T', 'L'},
      METHOD_0ARGS, /* ACPI 6.0 */
      METHOD_RETURNS(ACPI_RTYPE_INTEGER)}},

    {{{'_', 'N', 'B', 'S'},
      METHOD_0ARGS, /* ACPI 6.3 */
      METHOD_RETURNS(ACPI_RTYPE_BUFFER)}},

    {{{'_', 'N', 'C', 'H'},
      METHOD_0ARGS, /* ACPI 6.3 */
      METHOD_RETURNS(ACPI_RTYPE_BUFFER)}},

    {{{'_', 'N', 'I', 'C'},
      METHOD_0ARGS, /* ACPI 6.3 */
      METHOD_RETURNS(ACPI_RTYPE_BUFFER)}},

    {{{'_', 'N', 'I', 'G'},
      METHOD_0ARGS, /* ACPI 6.3 */
      METHOD_RETURNS(ACPI_RTYPE_BUFFER)}},

    {{{'_', 'N', 'I', 'H'},
      METHOD_1ARGS(ACPI_TYPE_BUFFER), /* ACPI 6.3 */
      METHOD_RETURNS(ACPI_RTYPE_BUFFER)}},

    {{{'_', 'N', 'T', 'T'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_INTEGER)}},

    {{{'_', 'O', 'F', 'F'}, METHOD_0ARGS, METHOD_NO_RETURN_VALUE}},

    {{{'_', 'O', 'N', '_'}, METHOD_0ARGS, METHOD_NO_RETURN_VALUE}},

    {{{'_', 'O', 'S', '_'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_STRING)}},

    {{{'_', 'O', 'S', 'C'}, METHOD_4ARGS(ACPI_TYPE_BUFFER, ACPI_TYPE_INTEGER, ACPI_TYPE_INTEGER, ACPI_TYPE_BUFFER), METHOD_RETURNS(ACPI_RTYPE_BUFFER)}},

    {{{'_', 'O', 'S', 'T'}, METHOD_3ARGS(ACPI_TYPE_INTEGER, ACPI_TYPE_INTEGER, ACPI_TYPE_BUFFER), METHOD_NO_RETURN_VALUE}},

    {{{'_', 'P', 'A', 'I'}, METHOD_1ARGS(ACPI_TYPE_INTEGER), METHOD_RETURNS(ACPI_RTYPE_INTEGER)}},

    {{{'_', 'P', 'C', 'L'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_PACKAGE)}}, /* Variable-length (Refs) */
    PACKAGE_INFO(ACPI_PTYPE1_VAR, ACPI_RTYPE_REFERENCE, 0, 0, 0, 0),

    {{{'_', 'P', 'C', 'T'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_PACKAGE)}}, /* Fixed-length (2 Buf) */
    PACKAGE_INFO(ACPI_PTYPE1_FIXED, ACPI_RTYPE_BUFFER, 2, 0, 0, 0),

    {{{'_', 'P', 'D', 'C'}, METHOD_1ARGS(ACPI_TYPE_BUFFER), METHOD_NO_RETURN_VALUE}},

    {{{'_', 'P', 'D', 'L'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_INTEGER)}},

    {{{'_', 'P', 'I', 'C'}, METHOD_1ARGS(ACPI_TYPE_INTEGER), METHOD_NO_RETURN_VALUE}},

    {{{'_', 'P', 'I', 'F'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_PACKAGE)}}, /* Fixed-length (3 Int),(3 Str) */
    PACKAGE_INFO(ACPI_PTYPE1_FIXED, ACPI_RTYPE_INTEGER, 3, ACPI_RTYPE_STRING, 3, 0),

    {{{'_', 'P', 'L', 'D'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_PACKAGE)}}, /* Variable-length (Bufs) */
    PACKAGE_INFO(ACPI_PTYPE1_VAR, ACPI_RTYPE_BUFFER, 0, 0, 0, 0),

    {{{'_', 'P', 'M', 'C'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_PACKAGE)}}, /* Fixed-length (11 Int),(3 Str) */
    PACKAGE_INFO(ACPI_PTYPE1_FIXED, ACPI_RTYPE_INTEGER, 11, ACPI_RTYPE_STRING, 3, 0),

    {{{'_', 'P', 'M', 'D'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_PACKAGE)}}, /* Variable-length (Refs) */
    PACKAGE_INFO(ACPI_PTYPE1_VAR, ACPI_RTYPE_REFERENCE, 0, 0, 0, 0),

    {{{'_', 'P', 'M', 'M'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_INTEGER)}},

    {{{'_', 'P', 'P', 'C'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_INTEGER)}},

    {{{'_', 'P', 'P', 'E'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_INTEGER)}}, /* See dig64 spec */

    {{{'_', 'P', 'R', '0'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_PACKAGE)}}, /* Variable-length (Refs) */
    PACKAGE_INFO(ACPI_PTYPE1_VAR, ACPI_RTYPE_REFERENCE, 0, 0, 0, 0),

    {{{'_', 'P', 'R', '1'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_PACKAGE)}}, /* Variable-length (Refs) */
    PACKAGE_INFO(ACPI_PTYPE1_VAR, ACPI_RTYPE_REFERENCE, 0, 0, 0, 0),

    {{{'_', 'P', 'R', '2'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_PACKAGE)}}, /* Variable-length (Refs) */
    PACKAGE_INFO(ACPI_PTYPE1_VAR, ACPI_RTYPE_REFERENCE, 0, 0, 0, 0),

    {{{'_', 'P', 'R', '3'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_PACKAGE)}}, /* Variable-length (Refs) */
    PACKAGE_INFO(ACPI_PTYPE1_VAR, ACPI_RTYPE_REFERENCE, 0, 0, 0, 0),

    {{{'_', 'P', 'R', 'E'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_PACKAGE)}}, /* Variable-length (Refs) */
    PACKAGE_INFO(ACPI_PTYPE1_VAR, ACPI_RTYPE_REFERENCE, 0, 0, 0, 0),

    {{{'_', 'P', 'R', 'L'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_PACKAGE)}}, /* Variable-length (Refs) */
    PACKAGE_INFO(ACPI_PTYPE1_VAR, ACPI_RTYPE_REFERENCE, 0, 0, 0, 0),

    {{{'_', 'P', 'R', 'R'},
      METHOD_0ARGS,                         /* ACPI 6.0 */
      METHOD_RETURNS(ACPI_RTYPE_PACKAGE)}}, /* Fixed-length (1 Ref) */
    PACKAGE_INFO(ACPI_PTYPE1_FIXED, ACPI_RTYPE_REFERENCE, 1, 0, 0, 0),

    {{{'_', 'P', 'R', 'S'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_BUFFER)}},

    /*
     * For _PRT, many BIOSs reverse the 3rd and 4th Package elements (Source
     * and SourceIndex). This bug is so prevalent that there is code in the
     * ACPICA Resource Manager to detect this and switch them back. For now,
     * do not allow and issue a warning. To allow this and eliminate the
     * warning, add the ACPI_RTYPE_REFERENCE type to the 4th element (index 3)
     * in the statement below.
     */
    {{{'_', 'P', 'R', 'T'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_PACKAGE)}}, /* Variable-length (Pkgs) each (4): Int,Int,Int/Ref,Int */
    PACKAGE_INFO(ACPI_PTYPE2_FIXED, 4, ACPI_RTYPE_INTEGER, ACPI_RTYPE_INTEGER, ACPI_RTYPE_INTEGER | ACPI_RTYPE_REFERENCE, ACPI_RTYPE_INTEGER),

    {{{'_', 'P', 'R', 'W'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_PACKAGE)}}, /* Variable-length (Pkgs) each: Pkg/Int,Int,[Variable-length Refs] (Pkg is Ref/Int) */
    PACKAGE_INFO(ACPI_PTYPE1_OPTION, 2, ACPI_RTYPE_INTEGER | ACPI_RTYPE_PACKAGE, ACPI_RTYPE_INTEGER, ACPI_RTYPE_REFERENCE, 0),

    {{{'_', 'P', 'S', '0'}, METHOD_0ARGS, METHOD_NO_RETURN_VALUE}},

    {{{'_', 'P', 'S', '1'}, METHOD_0ARGS, METHOD_NO_RETURN_VALUE}},

    {{{'_', 'P', 'S', '2'}, METHOD_0ARGS, METHOD_NO_RETURN_VALUE}},

    {{{'_', 'P', 'S', '3'}, METHOD_0ARGS, METHOD_NO_RETURN_VALUE}},

    {{{'_', 'P', 'S', 'C'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_INTEGER)}},

    {{{'_', 'P', 'S', 'D'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_PACKAGE)}}, /* Variable-length (Pkgs) each (5 Int) with count */
    PACKAGE_INFO(ACPI_PTYPE2_COUNT, ACPI_RTYPE_INTEGER, 0, 0, 0, 0),

    {{{'_', 'P', 'S', 'E'}, METHOD_1ARGS(ACPI_TYPE_INTEGER), METHOD_NO_RETURN_VALUE}},

    {{{'_', 'P', 'S', 'L'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_PACKAGE)}}, /* Variable-length (Refs) */
    PACKAGE_INFO(ACPI_PTYPE1_VAR, ACPI_RTYPE_REFERENCE, 0, 0, 0, 0),

    {{{'_', 'P', 'S', 'R'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_INTEGER)}},

    {{{'_', 'P', 'S', 'S'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_PACKAGE)}}, /* Variable-length (Pkgs) each (6 Int) */
    PACKAGE_INFO(ACPI_PTYPE2, ACPI_RTYPE_INTEGER, 6, 0, 0, 0),

    {{{'_', 'P', 'S', 'V'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_INTEGER)}},

    {{{'_', 'P', 'S', 'W'}, METHOD_1ARGS(ACPI_TYPE_INTEGER), METHOD_NO_RETURN_VALUE}},

    {{{'_', 'P', 'T', 'C'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_PACKAGE)}}, /* Fixed-length (2 Buf) */
    PACKAGE_INFO(ACPI_PTYPE1_FIXED, ACPI_RTYPE_BUFFER, 2, 0, 0, 0),

    {{{'_', 'P', 'T', 'P'}, METHOD_2ARGS(ACPI_TYPE_INTEGER, ACPI_TYPE_INTEGER), METHOD_RETURNS(ACPI_RTYPE_INTEGER)}},

    {{{'_', 'P', 'T', 'S'}, METHOD_1ARGS(ACPI_TYPE_INTEGER), METHOD_NO_RETURN_VALUE}},

    {{{'_', 'P', 'U', 'R'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_PACKAGE)}}, /* Fixed-length (2 Int) */
    PACKAGE_INFO(ACPI_PTYPE1_FIXED, ACPI_RTYPE_INTEGER, 2, 0, 0, 0),

    {{{'_', 'P', 'X', 'M'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_INTEGER)}},

    {{{'_', 'R', 'D', 'I'},
      METHOD_0ARGS,                         /* ACPI 6.0 */
      METHOD_RETURNS(ACPI_RTYPE_PACKAGE)}}, /* Variable-length (1 Int, n Pkg (m Ref)) */
    PACKAGE_INFO(ACPI_PTYPE2_VAR_VAR, ACPI_RTYPE_INTEGER, 1, ACPI_RTYPE_REFERENCE, 0, 0),

    {{{'_', 'R', 'E', 'G'}, METHOD_2ARGS(ACPI_TYPE_INTEGER, ACPI_TYPE_INTEGER), METHOD_NO_RETURN_VALUE}},

    {{{'_', 'R', 'E', 'V'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_INTEGER)}},

    {{{'_', 'R', 'M', 'V'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_INTEGER)}},

    {{{'_', 'R', 'O', 'M'}, METHOD_2ARGS(ACPI_TYPE_INTEGER, ACPI_TYPE_INTEGER), METHOD_RETURNS(ACPI_RTYPE_BUFFER)}},

    {{{'_', 'R', 'S', 'T'},
      METHOD_0ARGS, /* ACPI 6.0 */
      METHOD_NO_RETURN_VALUE}},

    {{{'_', 'R', 'T', 'V'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_INTEGER)}},

    /*
     * For _S0_ through _S5_, the ACPI spec defines a return Package
     * containing 1 Integer, but most DSDTs have it wrong - 2,3, or 4 integers.
     * Allow this by making the objects "Variable-length length", but all elements
     * must be Integers.
     */
    {{{'_', 'S', '0', '_'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_PACKAGE)}}, /* Fixed-length (1 Int) */
    PACKAGE_INFO(ACPI_PTYPE1_VAR, ACPI_RTYPE_INTEGER, 1, 0, 0, 0),

    {{{'_', 'S', '1', '_'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_PACKAGE)}}, /* Fixed-length (1 Int) */
    PACKAGE_INFO(ACPI_PTYPE1_VAR, ACPI_RTYPE_INTEGER, 1, 0, 0, 0),

    {{{'_', 'S', '2', '_'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_PACKAGE)}}, /* Fixed-length (1 Int) */
    PACKAGE_INFO(ACPI_PTYPE1_VAR, ACPI_RTYPE_INTEGER, 1, 0, 0, 0),

    {{{'_', 'S', '3', '_'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_PACKAGE)}}, /* Fixed-length (1 Int) */
    PACKAGE_INFO(ACPI_PTYPE1_VAR, ACPI_RTYPE_INTEGER, 1, 0, 0, 0),

    {{{'_', 'S', '4', '_'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_PACKAGE)}}, /* Fixed-length (1 Int) */
    PACKAGE_INFO(ACPI_PTYPE1_VAR, ACPI_RTYPE_INTEGER, 1, 0, 0, 0),

    {{{'_', 'S', '5', '_'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_PACKAGE)}}, /* Fixed-length (1 Int) */
    PACKAGE_INFO(ACPI_PTYPE1_VAR, ACPI_RTYPE_INTEGER, 1, 0, 0, 0),

    {{{'_', 'S', '1', 'D'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_INTEGER)}},

    {{{'_', 'S', '2', 'D'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_INTEGER)}},

    {{{'_', 'S', '3', 'D'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_INTEGER)}},

    {{{'_', 'S', '4', 'D'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_INTEGER)}},

    {{{'_', 'S', '0', 'W'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_INTEGER)}},

    {{{'_', 'S', '1', 'W'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_INTEGER)}},

    {{{'_', 'S', '2', 'W'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_INTEGER)}},

    {{{'_', 'S', '3', 'W'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_INTEGER)}},

    {{{'_', 'S', '4', 'W'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_INTEGER)}},

    {{{'_', 'S', 'B', 'A'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_PACKAGE)}}, /* Fixed-length (4 Int) */
    PACKAGE_INFO(ACPI_PTYPE1_FIXED, ACPI_RTYPE_INTEGER, 4, 0, 0, 0),

    {{{'_', 'S', 'B', 'I'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_PACKAGE)}}, /* Fixed-length (1 Int, 1 Buf) */
    PACKAGE_INFO(ACPI_PTYPE1_FIXED, ACPI_RTYPE_INTEGER, 1, ACPI_RTYPE_BUFFER, 1, 0),

    {{{'_', 'S', 'B', 'R'}, METHOD_3ARGS(ACPI_TYPE_INTEGER, ACPI_TYPE_INTEGER, ACPI_TYPE_INTEGER), METHOD_RETURNS(ACPI_RTYPE_PACKAGE)}}, /* Fixed-length (2 Int) */
    PACKAGE_INFO(ACPI_PTYPE1_FIXED, ACPI_RTYPE_INTEGER, 2, ACPI_RTYPE_BUFFER | ACPI_RTYPE_INTEGER, 1, 0),

    {{{'_', 'S', 'B', 'S'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_INTEGER)}},

    {{{'_', 'S', 'B', 'T'}, METHOD_4ARGS(ACPI_TYPE_INTEGER, ACPI_TYPE_INTEGER, ACPI_TYPE_INTEGER, ACPI_TYPE_ANY), METHOD_RETURNS(ACPI_RTYPE_PACKAGE)}}, /* Fixed-length (2 Int, 1 Buf | Int) */
    PACKAGE_INFO(ACPI_PTYPE1_FIXED, ACPI_RTYPE_INTEGER, 2, ACPI_RTYPE_BUFFER | ACPI_RTYPE_INTEGER, 1, 0),

    {{{'_', 'S', 'B', 'W'}, METHOD_5ARGS(ACPI_TYPE_INTEGER, ACPI_TYPE_INTEGER, ACPI_TYPE_INTEGER, ACPI_TYPE_INTEGER, ACPI_TYPE_ANY), METHOD_RETURNS(ACPI_RTYPE_PACKAGE)}},
    PACKAGE_INFO(ACPI_PTYPE1_FIXED, ACPI_RTYPE_BUFFER | ACPI_RTYPE_INTEGER, 1, 0, 0, 0),

    {{{'_', 'S', 'C', 'P'}, METHOD_1ARGS(ACPI_TYPE_INTEGER) | ARG_COUNT_IS_MINIMUM, METHOD_NO_RETURN_VALUE}}, /* Acpi 1.0 allowed 1 integer arg. Acpi 3.0 expanded to 3 args. Allow both. */

    {{{'_', 'S', 'D', 'D'}, METHOD_1ARGS(ACPI_TYPE_BUFFER), METHOD_NO_RETURN_VALUE}},

    {{{'_', 'S', 'E', 'G'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_INTEGER)}},

    {{{'_', 'S', 'H', 'L'}, METHOD_1ARGS(ACPI_TYPE_INTEGER), METHOD_RETURNS(ACPI_RTYPE_INTEGER)}},

    {{{'_', 'S', 'L', 'I'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_BUFFER)}},

    {{{'_', 'S', 'P', 'D'}, METHOD_1ARGS(ACPI_TYPE_INTEGER), METHOD_RETURNS(ACPI_RTYPE_INTEGER)}},

    {{{'_', 'S', 'R', 'S'}, METHOD_1ARGS(ACPI_TYPE_BUFFER), METHOD_NO_RETURN_VALUE}},

    {{{'_', 'S', 'R', 'T'}, METHOD_1ARGS(ACPI_TYPE_BUFFER), METHOD_RETURNS(ACPI_RTYPE_INTEGER)}},

    {{{'_', 'S', 'R', 'V'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_INTEGER)}}, /* See IPMI spec */

    {{{'_', 'S', 'S', 'T'}, METHOD_1ARGS(ACPI_TYPE_INTEGER), METHOD_NO_RETURN_VALUE}},

    {{{'_', 'S', 'T', 'A'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_INTEGER)}},

    {{{'_', 'S', 'T', 'M'}, METHOD_3ARGS(ACPI_TYPE_BUFFER, ACPI_TYPE_BUFFER, ACPI_TYPE_BUFFER), METHOD_NO_RETURN_VALUE}},

    {{{'_', 'S', 'T', 'P'}, METHOD_2ARGS(ACPI_TYPE_INTEGER, ACPI_TYPE_INTEGER), METHOD_RETURNS(ACPI_RTYPE_INTEGER)}},

    {{{'_', 'S', 'T', 'R'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_BUFFER)}},

    {{{'_', 'S', 'T', 'V'}, METHOD_2ARGS(ACPI_TYPE_INTEGER, ACPI_TYPE_INTEGER), METHOD_RETURNS(ACPI_RTYPE_INTEGER)}},

    {{{'_', 'S', 'U', 'B'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_STRING)}},

    {{{'_', 'S', 'U', 'N'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_INTEGER)}},

    {{{'_', 'S', 'W', 'S'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_INTEGER)}},

    {{{'_', 'T', 'C', '1'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_INTEGER)}},

    {{{'_', 'T', 'C', '2'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_INTEGER)}},

    {{{'_', 'T', 'D', 'L'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_INTEGER)}},

    {{{'_', 'T', 'F', 'P'},
      METHOD_0ARGS, /* ACPI 6.0 */
      METHOD_RETURNS(ACPI_RTYPE_INTEGER)}},

    {{{'_', 'T', 'I', 'P'}, METHOD_1ARGS(ACPI_TYPE_INTEGER), METHOD_RETURNS(ACPI_RTYPE_INTEGER)}},

    {{{'_', 'T', 'I', 'V'}, METHOD_1ARGS(ACPI_TYPE_INTEGER), METHOD_RETURNS(ACPI_RTYPE_INTEGER)}},

    {{{'_', 'T', 'M', 'P'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_INTEGER)}},

    {{{'_', 'T', 'P', 'C'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_INTEGER)}},

    {{{'_', 'T', 'P', 'T'}, METHOD_1ARGS(ACPI_TYPE_INTEGER), METHOD_NO_RETURN_VALUE}},

    {{{'_', 'T', 'R', 'T'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_PACKAGE)}}, /* Variable-length (Pkgs) each 2 Ref/6 Int */
    PACKAGE_INFO(ACPI_PTYPE2, ACPI_RTYPE_REFERENCE, 2, ACPI_RTYPE_INTEGER, 6, 0),

    {{{'_', 'T', 'S', 'D'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_PACKAGE)}}, /* Variable-length (Pkgs) each 5 Int with count */
    PACKAGE_INFO(ACPI_PTYPE2_COUNT, ACPI_RTYPE_INTEGER, 5, 0, 0, 0),

    {{{'_', 'T', 'S', 'N'},
      METHOD_0ARGS, /* ACPI 6.0 */
      METHOD_RETURNS(ACPI_RTYPE_REFERENCE)}},

    {{{'_', 'T', 'S', 'P'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_INTEGER)}},

    {{{'_', 'T', 'S', 'S'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_PACKAGE)}}, /* Variable-length (Pkgs) each 5 Int */
    PACKAGE_INFO(ACPI_PTYPE2, ACPI_RTYPE_INTEGER, 5, 0, 0, 0),

    {{{'_', 'T', 'S', 'T'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_INTEGER)}},

    {{{'_', 'T', 'T', 'S'}, METHOD_1ARGS(ACPI_TYPE_INTEGER), METHOD_NO_RETURN_VALUE}},

    {{{'_', 'T', 'Z', 'D'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_PACKAGE)}}, /* Variable-length (Refs) */
    PACKAGE_INFO(ACPI_PTYPE1_VAR, ACPI_RTYPE_REFERENCE, 0, 0, 0, 0),

    {{{'_', 'T', 'Z', 'M'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_REFERENCE)}},

    {{{'_', 'T', 'Z', 'P'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_INTEGER)}},

    {{{'_', 'U', 'I', 'D'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_INTEGER | ACPI_RTYPE_STRING)}},

    {{{'_', 'U', 'P', 'C'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_PACKAGE)}}, /* Fixed-length (4 Int) */
    PACKAGE_INFO(ACPI_PTYPE1_FIXED, ACPI_RTYPE_INTEGER, 4, 0, 0, 0),

    {{{'_', 'U', 'P', 'D'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_INTEGER)}},

    {{{'_', 'U', 'P', 'P'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_INTEGER)}},

    {{{'_', 'V', 'P', 'O'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_INTEGER)}},

    /* Acpi 1.0 defined _WAK with no return value. Later, it was changed to return a package */

    {{{'_', 'W', 'A', 'K'}, METHOD_1ARGS(ACPI_TYPE_INTEGER), METHOD_RETURNS(ACPI_RTYPE_NONE | ACPI_RTYPE_INTEGER | ACPI_RTYPE_PACKAGE)}},
    PACKAGE_INFO(ACPI_PTYPE1_FIXED, ACPI_RTYPE_INTEGER, 2, 0, 0, 0), /* Fixed-length (2 Int), but is optional */

    /* _WDG/_WED are MS extensions defined by "Windows Instrumentation" */

    {{{'_', 'W', 'D', 'G'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_BUFFER)}},

    {{{'_', 'W', 'E', 'D'}, METHOD_1ARGS(ACPI_TYPE_INTEGER), METHOD_RETURNS(ACPI_RTYPE_INTEGER | ACPI_RTYPE_STRING | ACPI_RTYPE_BUFFER)}},

    {{{'_', 'W', 'P', 'C'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_INTEGER)}}, /* ACPI 6.1 */

    {{{'_', 'W', 'P', 'P'}, METHOD_0ARGS, METHOD_RETURNS(ACPI_RTYPE_INTEGER)}}, /* ACPI 6.1 */

    PACKAGE_INFO(0, 0, 0, 0, 0, 0) /* Table terminator */
};
#else
extern const ACPI_PREDEFINED_INFO AcpiGbl_PredefinedMethods[];
#endif

#if (defined ACPI_CREATE_RESOURCE_TABLE && defined ACPI_APPLICATION)
/******************************************************************************
 *
 * Predefined names for use in Resource Descriptors. These names do not
 * appear in the global Predefined Name table (since these names never
 * appear in actual AML byte code, only in the original ASL)
 *
 * Note: Used by iASL compiler and AcpiHelp utility only.
 *
 *****************************************************************************/

const ACPI_PREDEFINED_INFO AcpiGbl_ResourceNames[] = {
    {{{'_', 'A', 'D', 'R'}, WIDTH_16 | WIDTH_64, 0}},
    {{{'_', 'A', 'L', 'N'}, WIDTH_8 | WIDTH_16 | WIDTH_32, 0}},
    {{{'_', 'A', 'S', 'I'}, WIDTH_8, 0}},
    {{{'_', 'A', 'S', 'Z'}, WIDTH_8, 0}},
    {{{'_', 'A', 'T', 'T'}, WIDTH_64, 0}},
    {{{'_', 'B', 'A', 'S'}, WIDTH_16 | WIDTH_32, 0}},
    {{{'_', 'B', 'M', '_'}, WIDTH_1, 0}},
    {{{'_', 'D', 'B', 'T'}, WIDTH_16, 0}}, /* Acpi 5.0 */
    {{{'_', 'D', 'E', 'C'}, WIDTH_1, 0}},
    {{{'_', 'D', 'M', 'A'}, WIDTH_8, 0}},
    {{{'_', 'D', 'P', 'L'}, WIDTH_1, 0}},  /* Acpi 5.0 */
    {{{'_', 'D', 'R', 'S'}, WIDTH_16, 0}}, /* Acpi 5.0 */
    {{{'_', 'E', 'N', 'D'}, WIDTH_1, 0}},  /* Acpi 5.0 */
    {{{'_', 'F', 'L', 'C'}, WIDTH_2, 0}},  /* Acpi 5.0 */
    {{{'_', 'G', 'R', 'A'}, WIDTH_ADDRESS, 0}},
    {{{'_', 'H', 'E', '_'}, WIDTH_1, 0}},
    {{{'_', 'I', 'N', 'T'}, WIDTH_16 | WIDTH_32, 0}},
    {{{'_', 'I', 'O', 'R'}, WIDTH_2, 0}}, /* Acpi 5.0 */
    {{{'_', 'L', 'E', 'N'}, WIDTH_8 | WIDTH_ADDRESS, 0}},
    {{{'_', 'L', 'I', 'N'}, WIDTH_8, 0}}, /* Acpi 5.0 */
    {{{'_', 'L', 'L', '_'}, WIDTH_1, 0}},
    {{{'_', 'M', 'A', 'F'}, WIDTH_1, 0}},
    {{{'_', 'M', 'A', 'X'}, WIDTH_ADDRESS, 0}},
    {{{'_', 'M', 'E', 'M'}, WIDTH_2, 0}},
    {{{'_', 'M', 'I', 'F'}, WIDTH_1, 0}},
    {{{'_', 'M', 'I', 'N'}, WIDTH_ADDRESS, 0}},
    {{{'_', 'M', 'O', 'D'}, WIDTH_1, 0}}, /* Acpi 5.0 */
    {{{'_', 'M', 'T', 'P'}, WIDTH_2, 0}},
    {{{'_', 'P', 'A', 'R'}, WIDTH_8, 0}},           /* Acpi 5.0 */
    {{{'_', 'P', 'H', 'A'}, WIDTH_1, 0}},           /* Acpi 5.0 */
    {{{'_', 'P', 'I', 'N'}, WIDTH_16, 0}},          /* Acpi 5.0 */
    {{{'_', 'P', 'P', 'I'}, WIDTH_8, 0}},           /* Acpi 5.0 */
    {{{'_', 'P', 'O', 'L'}, WIDTH_1 | WIDTH_2, 0}}, /* Acpi 5.0 */
    {{{'_', 'R', 'B', 'O'}, WIDTH_8, 0}},
    {{{'_', 'R', 'B', 'W'}, WIDTH_8, 0}},
    {{{'_', 'R', 'N', 'G'}, WIDTH_1, 0}},
    {{{'_', 'R', 'T', '_'}, WIDTH_8, 0}}, /* Acpi 3.0 */
    {{{'_', 'R', 'W', '_'}, WIDTH_1, 0}},
    {{{'_', 'R', 'X', 'L'}, WIDTH_16, 0}}, /* Acpi 5.0 */
    {{{'_', 'S', 'H', 'R'}, WIDTH_2, 0}},
    {{{'_', 'S', 'I', 'Z'}, WIDTH_2, 0}},
    {{{'_', 'S', 'L', 'V'}, WIDTH_1, 0}},  /* Acpi 5.0 */
    {{{'_', 'S', 'P', 'E'}, WIDTH_32, 0}}, /* Acpi 5.0 */
    {{{'_', 'S', 'T', 'B'}, WIDTH_2, 0}},  /* Acpi 5.0 */
    {{{'_', 'T', 'R', 'A'}, WIDTH_ADDRESS, 0}},
    {{{'_', 'T', 'R', 'S'}, WIDTH_1, 0}},
    {{{'_', 'T', 'S', 'F'}, WIDTH_8, 0}}, /* Acpi 3.0 */
    {{{'_', 'T', 'T', 'P'}, WIDTH_1, 0}},
    {{{'_', 'T', 'X', 'L'}, WIDTH_16, 0}}, /* Acpi 5.0 */
    {{{'_', 'T', 'Y', 'P'}, WIDTH_2 | WIDTH_16, 0}},
    {{{'_', 'V', 'E', 'N'}, VARIABLE_DATA, 0}}, /* Acpi 5.0 */
    PACKAGE_INFO(0, 0, 0, 0, 0, 0)              /* Table terminator */
};

const ACPI_PREDEFINED_INFO AcpiGbl_ScopeNames[] = {
    {{{'_', 'G', 'P', 'E'}, 0, 0}}, {{{'_', 'P', 'R', '_'}, 0, 0}}, {{{'_', 'S', 'B', '_'}, 0, 0}}, {{{'_', 'S', 'I', '_'}, 0, 0}}, {{{'_', 'T', 'Z', '_'}, 0, 0}}, PACKAGE_INFO(0, 0, 0, 0, 0, 0) /* Table terminator */
};
#else
extern const ACPI_PREDEFINED_INFO AcpiGbl_ResourceNames[];
#endif

#endif
