// ----------------------------------------------------------------------------
//
//	dl_fldigi.h
//
// ----------------------------------------------------------------------------

#ifndef DL_FLDIGI_H
#define DL_FLDIGI_H

#include <string>
#include <unistd.h>
#include <sys/types.h>

#include <curl/curl.h>
#include <curl/types.h>
#include <curl/easy.h>

#include <FL/Fl_Choice.H>

#include "confdialog.h"
#include "fl_digi.h"
#include "main.h"

extern bool dl_fldigi_downloaded_once;
extern Fl_Choice *habFlightXML;

void dl_fldigi_init();
void dl_fldigi_post(const char *data, const char *identity);
void dl_fldigi_download();
void dl_fldigi_update_payloads();
void dl_fldigi_select_payload(Fl_Choice* o, void *a);

#endif