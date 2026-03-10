#include "bssAst.h"

readonly BssAstStmt gBssAstStmtEmpty = {0};
readonly BssAstFunc gBssAstFuncEmpty = {0};
readonly BssAstTopLevel gBssAstTopLevelEmpty = {0};

BASE_CREATE_EFFICIENT_LL_DEFS(BssAstStmtList, BssAstStmt)
BASE_CREATE_EFFICIENT_LL_DEFS(BssAstTopLevelList, BssAstTopLevel)