#pragma once
// Keeps the stock 239A:0001 identity from info.json. See rules.mk for why this
// board must NOT wear the 0554:1001 PowerMic identity.
// #undef first: info.json's generated defines land earlier in the TU.
#undef PRODUCT
#define PRODUCT "RadPad"

// Force attached HID devices into REPORT protocol rather than TinyUSB's BOOT
// default. The Kensington Orbit declares report IDs and 12-bit axes in its
// report descriptor; in boot mode it sends 3-byte [buttons][X][Y] instead, and
// the parser misreads the buttons byte as a report ID. See matrix.c.
#define HOST_FORCE_REPORT_PROTOCOL
