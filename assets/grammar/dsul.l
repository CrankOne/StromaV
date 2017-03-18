%option noyywrap

D			[0-9]
L           [a-zA-Z_\\]
UL          [A-Z_]
DTNAME      {UL}


%{
# include <stdio.h>
# include "detector_ids.h"
# include "dsul_parser_generated.h"

#define YY_DECL int yylex()

%}

%%

[ \t\n]	        ; // ignore all whitespace
[0-9]+		    {yylval.ival = atoi(yytext); return T_INT;}
"-"		        {return T_DASH;}
","             {return T_COMMA;}
">"             {return T_GT;}
"<"             {return T_LT;}
">="            {return T_GTE;}
"<="            {return T_LTE;}
"!"             {return T_EXCLMM;}
"{"             {return T_LBRCT_CRL;}
"}"             {return T_RBRCT_CRL;}
"["             {return T_LBRCT_SQR;}
"]"             {return T_RBRCT_SQR;}
"x"             {return T_CROSS;}
"&"             {return T_AMP;}
"+"             {return T_PLUS;}
\"{L}*\"        {return L_STR;}
{L}({L}|{D})    {
                    yylval.exactDetID = AFR_detector_by_name(yytext);
                    if( yylval.exactDetID ) {  /* This is an exact detector name */
                        return T_DETECTOR_CODE;
                    }
                    yylval.familyID = AFR_family_id_by_name(yytext);
                    if( yylval.familyID ) {
                        return T_FAMILY_CODE;
                    }
                    /* TODO: error */
                    ECHO;
                }

%%
