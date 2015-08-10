extern bool preciseF32,
            receiveJSON,
            emitJSON,
            minifyWhitespace,
            last;

extern Ref extraInfo;

void eliminateDeadFuncs(Ref ast);
void eliminate(Ref ast, bool memSafe=false);
void eliminateMemSafe(Ref ast);
void simplifyExpressions(Ref ast);
void optimizeFrounds(Ref ast);
void simplifyIfs(Ref ast);
void registerize(Ref ast);
void registerizeHarder(Ref ast);
void minifyLocals(Ref ast);
void asmLastOpts(Ref ast);

