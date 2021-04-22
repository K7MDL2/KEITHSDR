#ifndef _SD_CARD_H_
#define _SD_CARD_H_
//
//  SD_CARD.h 
//
//  SD card related support
//
//
#include "SDR_RA8875.h"
#include "RadioConfig.h"

bool Open_SD_cfgfile(void);
void SD_CardInfo(void);
void write_db_tables(void);
void write_cfg(void);
void read_db_tables(void);
bool write_radiocfg_h(void);

#endif  // _SD_CARD_H_
