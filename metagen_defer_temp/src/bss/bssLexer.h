#ifndef BSS_LEXER_H
#define BSS_LEXER_H

#include "bssCore.h"

#include "os/core/osCore.h"
#include "bssLexer.h"

bool bssLexerAdvanceChar(BssInterp *interp);
u8 bssLexerPeekCharEx(BssInterp *interp, u64 amount);
u8 bssLexerPeekChar(BssInterp *interp);

BssTok bssLexerLexNextTok(BssInterp *interp);

bool bssLexerFromBuffer(BssInterp *interp, U8Array buf);
bool bssLexerLexFile(BssInterp *interp, str8 file);
BssTok bssLexerGetNextTok(BssInterp *interp);
BssTok bssLexerPeekTok(BssInterp *interp, u64 amount);

#endif