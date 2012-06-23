#ifndef _LINUX_AN32155A_H
#define _LINUX_AN32155A_H

#include <linux/types.h>

/* platform data for the AN32155A LED controller driver */

struct an32155a_platform_data {
	/* number of the enable GPIO */
	unsigned	enable_gpio;
	/* number of the interrupt GPIO */
	unsigned	int_gpio;
};

#endif /* _LINUX_AN32155A_H */
