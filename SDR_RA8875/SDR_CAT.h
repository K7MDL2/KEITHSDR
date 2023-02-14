#ifndef _SDR_CAT_H_
#define _SDR_CAT_H_
//
//  SDR_CAT.h 
//
//  Serial port control protocol useful for control head and panadapter information exchange.
//
//
#include "SDR_RA8875.h"
#include "RadioConfig.h"

#ifndef USE_CAT_SER
    #ifdef PAN_CAT

    void CAT_handler(void);
    void CAT_setup(void);
    void print_CAT_status(void);
    void init_CAT_comms(void);

    #endif
#endif  // ! USE_CAT_SER

#endif  // _SDR_CAT_H_
