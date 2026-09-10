#pragma once
// Keeps the stock 239A:0001 identity from info.json. See rules.mk for why this
// board must NOT wear the 0554:1001 PowerMic identity.
// #undef first: info.json's generated defines land earlier in the TU.
#undef PRODUCT
#define PRODUCT "RadPad"
