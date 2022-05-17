/*
 * THE - The Hessling Editor. A text editor similar to VM/CMS xedit.
 * Copyright (C) 1991-2001 Mark Hessling
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to:
 *
 *    The Free Software Foundation, Inc.
 *    675 Mass Ave,
 *    Cambridge, MA 02139 USA.
 *
 *
 * If you make modifications to this software that you feel increases
 * it usefulness for the rest of the community, please email the
 * changes, enhancements, bug fixes as well as any and all ideas to me.
 * This software is going to be maintained and enhanced as deemed
 * necessary by the community.
 *
 * Mark Hessling,  M.Hessling@qut.edu.au  http://www.lightlink.com/hessling/
 */

/*
$Id: therexx.h,v 1.9 2013/04/01 01:27:34 mark Exp $
*/


#  include <rexxsaa.h>





/*
 * The following abbreviations are used as prefixes for the following
 * #defines:
 *
 * RFH   - RexxFunctionHandler
 * RRFE  - RexxRegisterFuntionExe
 * RDF   - RexxDeregisterFuntion
 *
 * REH   - RexxExitHandler
 * RREE  - RexxRegisterExitExe
 * RDE   - RexxDeregisterExit
 *
 * RSH   - RexxSubcomHandler
 * RRSE  - RexxRegisterSubcomExe
 * RDS   - RexxDeregisterSubcom
 *
 * RS    - RexxStart
 */
# define RXSTRING_STRPTR_TYPE char *
# define RFH_RETURN_TYPE      ULONG
# define RFH_ARG0_TYPE        PSZ
# define RFH_ARG1_TYPE        ULONG
# define RFH_ARG2_TYPE        PRXSTRING
# define RFH_ARG3_TYPE        PSZ
# define RFH_ARG4_TYPE        PRXSTRING
# define RRFE_ARG0_TYPE       PSZ
# define RRFE_ARG1_TYPE       RexxFunctionHandler*
# define RDF_ARG0_TYPE        PSZ
# define REH_RETURN_TYPE      LONG
# define REH_ARG0_TYPE        LONG
# define REH_ARG1_TYPE        LONG
# define REH_ARG2_TYPE        PEXIT
# define RREE_ARG0_TYPE       PSZ
# define RREE_ARG1_TYPE       RexxExitHandler*
# define RREE_ARG2_TYPE       PUCHAR
# define RSH_RETURN_TYPE      ULONG
# define RSH_ARG0_TYPE        PRXSTRING
# define RSH_ARG1_TYPE        PUSHORT
# define RSH_ARG2_TYPE        PRXSTRING
# define RRSE_ARG0_TYPE       PSZ
# define RRSE_ARG1_TYPE       RexxSubcomHandler *
# define RRSE_ARG2_TYPE       PUCHAR
# define RDE_ARG0_TYPE        PSZ
# define RDE_ARG1_TYPE        PSZ
# define RDS_ARG0_TYPE        PSZ
# define RDS_ARG1_TYPE        PSZ
# define RS_ARG0_TYPE         long
# define RS_ARG1_TYPE         PRXSTRING
# define RS_ARG2_TYPE         char *
# define RS_ARG3_TYPE         PRXSTRING
# define RS_ARG4_TYPE         PSZ
# define RS_ARG5_TYPE         long
# define RS_ARG6_TYPE         PRXSYSEXIT
# define RS_ARG7_TYPE         short *
# define RS_ARG8_TYPE         PRXSTRING


