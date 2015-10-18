#ifndef OW_SLAVE_C_INCLUDED
#define OW_SLAVE_C_INCLUDED


/* one wire slave */
/* bitanged implementation */
/* regular speed (ie. not overdrive) */
/* https://www.maximintegrated.com/en/app-notes/index.mvp/id/126 */
/* https://en.wikipedia.org/wiki/1-Wire */


#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>


/* check required macros */

#ifndef OW_SLAVE_DATA_MASK
#error "OW_SLAVE_DATA_MASK" not defined
#endif /* OW_SLAVE_DATA_MASK */

#ifndef OW_SLAVE_DATA_DDR
#error "OW_SLAVE_DATA_DDR" not defined
#endif /* OW_SLAVE_DATA_DDR */

#ifndef OW_SLAVE_DATA_PORT
#error "OW_SLAVE_DATA_PORT" not defined
#endif /* OW_SLAVE_DATA_PORT */

#ifndef OW_SLAVE_DATA_PIN
#error "OW_SLAVE_DATA_PIN" not defined
#endif /* OW_SLAVE_DATA_PIN */


#endif /* OW_SLAVE_C_INCLUDED */
