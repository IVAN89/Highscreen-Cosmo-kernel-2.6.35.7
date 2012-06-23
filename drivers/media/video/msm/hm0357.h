/* Copyright (c) 2009, Code Aurora Forum. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 *
 */
#ifndef CAMSENSOR_HM0357
#define CAMSENSOR_HM0357

#include <mach/board.h>

/*Register Define*/
#define HM0357_REG_ISPCTRL1_FLICKER       0x0120
#define HM0357_REG_FSTEP60H               0x0540
#define HM0357_REG_FSTEP60L               0x0541
#define HM0357_REG_FSTEP50H               0x0542
#define HM0357_REG_FSTEP50L               0x0543
#define HM0357_REG_ISPCTRL6_BRIGHT       0x0125    /*brightness enable*/
#define HM0357_REG_BRIGHT_STRENGTH       0x04C0    /*y brihtness strength*/
#define HM0357_REG_ISPCTRL6_CONTRAST     0x0125   /*contrast enable*/
#define HM0357_REG_CONTM                 0x04B0
#define HM0357_REG_ACONTM                0x04B1
#define HM0357_REG_CONTQ                 0x04B3
#define HM0357_REG_ACONTQ                0x04B4
#define HM0357_REG_CONTN                 0x04B6
#define HM0357_REG_CONTP                 0x04B9
#define HM0357_REG_CONTGAIN              0x04BC
#define HM0357_REG_YMEAN                 0x04BD

#define HM0357_REG_SAT_COMMON_MODE_DIS   0x0120
#define HM0357_REG_SATCB                 0x0480

typedef  struct
{
   uint32_t	addr;
   uint16_t	val;
}CAM_REG_ADDR_VAL_TYPE;

#endif /* CAMSENSOR_S5K5CAG */

