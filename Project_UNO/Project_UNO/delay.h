/***************************************************************************************************
                                   ExploreEmbedded	
****************************************************************************************************
 * File:   delay.h
 * Version: 15.0
 * Author: ExploreEmbedded
 * Website: http://www.exploreembedded.com/wiki
 * Description: File contains the function prototypes for the delay routines
 
The libraries have been tested on ExploreEmbedded development boards. We strongly believe that the 
library works on any of development boards for respective controllers. However, ExploreEmbedded 
disclaims any kind of hardware failure resulting out of usage of libraries, directly or indirectly.
Files may be subject to change without prior notice. The revision history contains the information 
related to updates. 
 
 
GNU GENERAL PUBLIC LICENSE: 
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>. 
 
Errors and omissions should be reported to codelibraries@exploreembedded.com
 **************************************************************************************************/
 
 
 /***************************************************************************************************
                             Revision History
****************************************************************************************************				   
15.0: Initial version 
***************************************************************************************************/
 
#ifndef _DELAY_H
#define _DELAY_H

#include <util/delay.h>
#include"stdutils.h"



/***************************************************************************************************
                             Function prototypes
***************************************************************************************************/

/* DELAY_us and DELAY_ms are mapped to the library functions provided by WinAvr compiler */

#define  DELAY_us(x)  _delay_us(x)
#define  DELAY_ms(x)  _delay_ms(x)
void DELAY_sec(uint16_t var_delaySecCount_u16);
/**************************************************************************************************/

#endif
