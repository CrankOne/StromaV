%{

# include <stdio.h>
# include <stdlib.h>
# include <string.h>

extern int yylex();
extern int yyparse();
extern FILE* yyin;

void yyerror(const char* s);

%}

%code requires {
# include "detector_ids.h"
}

%union {
    AFR_DetSignature exactDetID;
    AFR_DetFamID familyID;
	uint32_t ival;
    const char * strID;

    enum DSuL_BinCompOperatorCode {
        DSuL_Operators_none = 0,
        /* --- */
        DSuL_Operators_lt   = 1,
        DSuL_Operators_lte  = 2,
        DSuL_Operators_gt   = 3,
        DSuL_Operators_gte  = 4,
        /* --- */
        DSuL_Operators_and      = 11,
        DSuL_Operators_andNot   = 12,
    } cmpBinOp;

    struct sV_DSuL_MVarIndex {
        uint32_t nDim;
        union {
            uint32_t x[7];  /* <- TODO: make configurable? */
            char * strID;
        } components;
    } mVarIndex;

    struct MVarIndexRange {
        struct sV_DSuL_MVarIndex first;
        union {
            struct sV_DSuL_MVarIndex border;
            enum DSuL_BinCompOperatorCode binop;
        } second;
    } mVarIndexRange;
    struct MVarIndexRangeList {
        struct MVarIndexRange self;
        enum DSuL_BinCompOperatorCode binop;
        struct MVarIndexRangeList * next;
    } mVarIdxRangeExpr;

    struct MajorSelector {
        /* Setting this to major number means exact match. Otherwise has to be
         * set to family number. Apply family bitmask to get whether this is a
         * exact match or a family specifier: */
        AFR_DetMjNo majorNumber;
        struct MVarIndexRangeList * range;
    } majorSelector;

    struct Selector {
        struct MajorSelector mjSelector;
        struct MVarIndexRangeList * range;
    } selector;

    struct Expression {
        struct Selector left;
        enum DSuL_BinCompOperatorCode binop;
        struct Expression * next;
    } expressions;
}

%token<exactDetID> T_DETECTOR_CODE;
%token<familyID> T_FAMILY_CODE;
%token<ival> T_INT
%token<strID> L_STR
%token T_DASH T_COMMA T_PLUS
%token T_GT T_GTE T_LT T_LTE
%token T_EXCLMM T_AMP T_CROSS
%token T_LBRCT_SQR T_RBRCT_SQR T_LBRCT_CRL T_RBRCT_CRL

%type<expressions> expr;
%type<cmpBinOp> cmpOp;
%type<mVarIndex> mvarIdx;
%type<mVarIdxRangeExpr> mvarIdxExpr;
%type<mVarIndexRange> mvarIdxRng;
%type<majorSelector> mjSelector;
%type<selector> mnSelector selector;

%start expr

%%

       expr : selector {
                    $$.left = $1;
                    $$.binop = DSuL_Operators_none;
                    $$.next = NULL;
                }
            | selector T_PLUS expr {
                    $$.left = $1;
                    $$.binop = DSuL_Operators_and;
                    $$.next = malloc( sizeof(struct Expression) );
                    memcpy( $$.next, &($3), sizeof(struct Expression) );
                }
            | selector T_EXCLMM expr {
                    $$.left = $1;
                    $$.binop = DSuL_Operators_andNot;
                    $$.next = malloc( sizeof(struct Expression) );
                    memcpy( $$.next, &($3), sizeof(struct Expression) );
                }
            ;

   selector : T_FAMILY_CODE {
                    $$.mjSelector.majorNumber = AFR_compose_detector_major($1, NULL);
                    $$.mjSelector.range = NULL;
                    $$.range = NULL;
                }
            | T_DETECTOR_CODE {
                    union AFR_UniqueDetectorID uid;
                    uid.wholenum = $1;
                    $$.mjSelector.majorNumber = uid.byNumber.major;
                    $$.mjSelector.range = NULL;
                    $$.range = malloc( sizeof(struct MVarIndexRangeList) );
                    bzero( $$.range, sizeof(struct MVarIndexRangeList) );
                    AFR_decode_minor_to_indexes( uid.byNumber.minor,
                                                 &($$.range->self.first) );
                }
            | mjSelector {
                    $$.mjSelector = $1;
                    $$.range = NULL;
                }
            | mnSelector {
                    $$ = $1;
                }
            ;

 mnSelector : mjSelector T_LBRCT_SQR mvarIdx T_RBRCT_SQR {
                    $$.mjSelector = $1;
                    const size_t ssz = sizeof(struct MVarIndexRangeList);
                    $$.range = malloc(ssz);
                    bzero( $$.range, ssz );
                    memcpy( &($$.range->self), &($3), sizeof(struct MVarIndexRange) );
                }
            | mjSelector T_LBRCT_SQR mvarIdxExpr T_RBRCT_SQR {
                    $$.mjSelector = $1;
                    $$.range = malloc(sizeof(struct MVarIndexRangeList));
                    memcpy( $$.range, &($3), sizeof(struct MVarIndexRangeList) );
                }
            ;

 mjSelector : T_FAMILY_CODE mvarIdx {
                    $$.majorNumber = AFR_compose_detector_major( $1, &($2) );
                    $$.range = NULL;
                }
            | T_FAMILY_CODE T_LBRCT_CRL mvarIdx T_RBRCT_CRL {
                    $$.majorNumber = AFR_compose_detector_major( $1, &($3) );
                    $$.range = NULL;
                }
            | T_FAMILY_CODE T_LBRCT_CRL mvarIdxExpr T_RBRCT_CRL {
                    $$.majorNumber = AFR_compose_detector_major( $1, NULL );
                    $$.range = malloc(sizeof(struct MVarIndexRangeList));
                    memcpy( $$.range, &($3), sizeof(struct MVarIndexRangeList) );
                }
            ;

mvarIdxExpr : mvarIdxRng {
                    $$.binop = DSuL_Operators_none;
                    $$.self = $1;
                    $$.next = NULL;
                }
            | mvarIdxRng T_AMP mvarIdxExpr {
                    $$.binop = DSuL_Operators_and;
                    $$.self = $1;
                    $$.next = malloc(sizeof(struct MVarIndexRangeList));
                    *($$.next) = $3;
                }
            | mvarIdxRng T_EXCLMM mvarIdxExpr  {
                    $$.binop = DSuL_Operators_andNot;
                    $$.self = $1;
                    $$.next = malloc(sizeof(struct MVarIndexRangeList));
                    *($$.next) = $3;
                }
            ;

 mvarIdxRng : mvarIdx T_DASH mvarIdx {
                    $$.first = $1;
                    $$.second.border = $3;
                }
            | cmpOp mvarIdx {
                    $$.first = $2;
                    $$.second.binop = $1;
                }
            ;

    mvarIdx : T_INT {
                    $$.nDim = 1;
                    $$.components.x[0] = $1;
                }
            | L_STR {
                    $$.nDim = 0;
                    $$.components.strID = strdup($1);
                }
            | T_INT T_CROSS mvarIdx {
                    if( !$3.nDim ) {
                        yyerror( "Numerical and string indexes mixed." );
                    }
                    $$.components.x[$$.nDim] = $1;
                    $$.nDim++;
                }
            ;

      cmpOp : T_GT      { $$ = DSuL_Operators_gt; }
            | T_GTE     { $$ = DSuL_Operators_gte; }
            | T_LT      { $$ = DSuL_Operators_lt; }
            | T_LTE     { $$ = DSuL_Operators_lte; }
            ;

%%

# include "detector_ids.h"

# if 0
int main() {
	yyin = stdin;

	do { 
		yyparse();
	} while(!feof(yyin));

	return 0;
}
# endif

void yyerror(const char* s) {
	fprintf(stderr, "Parse error: %s\n", s);
	exit(1);
}
