/* (c) 2010 Anthony Catel */
/* WIP */
/* gcc http.c -fshort-enums -o http_parser_test */

#include "ape_http_parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define __   -1

#define MAX_CL 1048576
#define MAX_RCODE 9999

/* Todo : check for endieness + aligned */
#define BYTES_GET(b) \
    *(uint32_t *) b == ((' ' << 24) | ('T' << 16) | ('E' << 8) | 'G')

#define BYTES_POST(b) \
    *(uint32_t *) b == (('T' << 24) | ('S' << 16) | ('O' << 8) | 'P')


typedef enum classes {
    C_NUL,  /* BAD CHAR */
    C_SPACE,/* space */
    C_WHITE,/* other whitespace */
    C_CR,   /* \r */
    C_LF,   /* \n */
    C_COLON,/* : */
    C_COMMA,/* , */
    C_QUOTE,/* " */
    C_BACKS,/* \ */
    C_SLASH,/* / */
    C_PLUS, /* + */
    C_MINUS,/* - */
    C_POINT,/* . */
    C_MARK, /* ? */
    C_PCT,  /* % */
    C_DIGIT,/* 0-9 */
    C_ETC,  /* other */
    C_STAR, /* * */
    C_ABDF, /* ABDF */
    C_E,    /* E */
    C_G,    /* G */
    C_T,    /* T */
    C_P,    /* P */
    C_H,    /* H */
    C_O,    /* O */
    C_S,    /* S */
    C_C,    /* C */
    C_N,    /* N */
    C_L,    /* L */
    C_USCORE,/* _ */
    NR_CLASSES
} parser_class;


static int ascii_class[128] = {
/*
    This array maps the 128 ASCII characters into character classes.
    The remaining Unicode characters should be mapped to C_ETC.
    Non-whitespace control characters are errors.
*/
    C_NUL,    C_NUL,   C_NUL,    C_NUL,    C_NUL,   C_NUL,   C_NUL,   C_NUL,
    C_NUL,    C_NUL,   C_LF,     C_NUL,    C_NUL,   C_CR,    C_NUL,   C_NUL,
    C_NUL,    C_NUL,   C_NUL,    C_NUL,    C_NUL,   C_NUL,   C_NUL,   C_NUL,
    C_NUL,    C_NUL,   C_NUL,    C_NUL,    C_NUL,   C_NUL,   C_NUL,   C_NUL,

    C_SPACE,  C_ETC,   C_QUOTE,  C_ETC,    C_ETC,   C_PCT,   C_ETC,   C_ETC,
    C_ETC,    C_ETC,   C_STAR,   C_PLUS,   C_COMMA, C_MINUS, C_POINT, C_SLASH,
    C_DIGIT,  C_DIGIT, C_DIGIT,  C_DIGIT,  C_DIGIT, C_DIGIT, C_DIGIT, C_DIGIT,
    C_DIGIT,  C_DIGIT, C_COLON,  C_ETC,    C_ETC,   C_ETC,   C_ETC,   C_MARK,

    C_ETC,   C_ABDF,  C_ABDF,  C_C,     C_ABDF,  C_E,     C_ABDF,  C_G,
    C_H,     C_ETC,   C_ETC,   C_ETC,   C_L,     C_ETC,   C_N,     C_O,
    C_P,     C_ETC,   C_ETC,   C_S,     C_T,     C_ETC,   C_ETC,   C_ETC,
    C_ETC,   C_ETC,   C_ETC,   C_ETC,   C_BACKS, C_ETC,   C_ETC,   C_USCORE,

    C_ETC,   C_ABDF,  C_ABDF,  C_C,     C_ABDF,  C_E,     C_ABDF,  C_G,
    C_H,     C_ETC,   C_ETC,   C_ETC,   C_L,     C_ETC,   C_N,     C_O,
    C_P,     C_ETC,   C_ETC,   C_S,     C_T,     C_ETC,   C_ETC,   C_ETC,
    C_ETC,   C_ETC,   C_ETC,   C_ETC,   C_ETC,   C_ETC,   C_ETC,   C_NUL
};

typedef enum actions {
    MG = -2, /* Method get */
    MP = -3, /* Method post */
    PE = -4, /* Path end */
    HA = -5, /* HTTP MAJOR */
    HB = -6, /* HTTP MINOR  */
    KH = -7, /* header key */
    VH = -8, /* header value */
    KC = -9, /* Content length */
    VC = -10, /* Content-length digit */
    VE = -11, /* content length finish */
    EA = -12, /* first hex in % path */
    EB = -13, /* second digit in % path */
    EH = -14, /* end of headers */
    BC = -15, /* body char */
    PC = -16, /* path char */
    QS = -17, /* query string */
    BH = -18,  /* Begin unhex */
    HK = -19,
    HV = -20,
    RA = -21,
    RB = -22,
    RC = -23,
    RH = -24
} parser_actions;


static int state_transition_table[NR_STATES][NR_CLASSES] = {
/*
                       nul   white                                      etc   ABDF
                       | space |  \r\n  :  ,  "  \  /  +  -  .  ?  % 09  |  *  | E  G  T  P  H  O  S  C  N  L  _ */
/*start           GO*/ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,G1,__,P1,R1,__,__,__,__,__,__},
/*GE              G1*/ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,G2,__,__,__,__,__,__,__,__,__,__},
/*GET             G2*/ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,G3,__,__,__,__,__,__,__,__},
/*GET             G3*/ {__,MG,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
/*PO              P1*/ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,P2,__,__,__,__,__},
/*POS             P2*/ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,P3,__,__,__,__},
/*POST            P3*/ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,P4,__,__,__,__,__,__,__,__},
/*POST            P4*/ {__,MP,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
/* path           PT*/ {__,PE,__,__,__,PC,PC,__,__,PC,PC,PC,PC,QS,E1,PC,PC,__,PC,PC,PC,PC,PC,PC,PC,PC,PC,PC,PC,PC},
/*H               H1*/ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,H2,__,__,__,__,__,__},
/*HT              H2*/ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,H3,__,__,__,__,__,__,__,__},
/*HTT             H3*/ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,H4,__,__,__,__,__,__,__,__},
/*HTTP            H4*/ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,H5,__,__,__,__,__,__,__},
/*HTTP/           H5*/ {__,__,__,__,__,__,__,__,__,H6,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
/*HTTP/[0-9]      H6*/ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,HA,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
/*HTTP/[0-9]/     H7*/ {__,__,__,__,__,__,__,__,__,__,__,__,H8,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
/*HTTP/[0-9]/[0-9]H8*/ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,HB,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
/* new line       EL*/ {__,__,__,ER,C1,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
/* \r expect \n   ER*/ {__,__,__,__,C1,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
/* header key     HH*/ {__,__,__,__,__,KH,__,__,__,__,__,HK,__,__,__,HK,HK,__,HK,HK,HK,HK,HK,HK,HK,HK,HK,HK,HK,__},
/* header value   HI*/ {__,HV,__,VH,VH,HV,HV,HV,HV,HV,HV,HV,HV,HV,HV,HV,HV,HV,HV,HV,HV,HV,HV,HV,HV,HV,HV,HV,HV,HV},
#if 1
/*C               C1*/ {__,__,__,FI,EH,KH,__,__,__,__,__,HK,__,__,__,__,HK,__,HK,HK,HK,HK,HK,HK,HK,HK,C2,HK,HK,__},
/*Co              C2*/ {__,__,__,__,__,KH,__,__,__,__,__,HK,__,__,__,__,HK,__,HK,HK,HK,HK,HK,HK,C3,HK,HK,HK,HK,__},
/*Con             C3*/ {__,__,__,__,__,KH,__,__,__,__,__,HK,__,__,__,__,HK,__,HK,HK,HK,HK,HK,HK,HK,HK,HK,C4,HK,__},
/*Cont            C4*/ {__,__,__,__,__,KH,__,__,__,__,__,HK,__,__,__,__,HK,__,HK,HK,HK,C5,HK,HK,HK,HK,HK,HK,HK,__},
/*Conte           C5*/ {__,__,__,__,__,KH,__,__,__,__,__,HK,__,__,__,__,HK,__,HK,C6,HK,HK,HK,HK,HK,HK,HK,HK,HK,__},
/*Conten          C6*/ {__,__,__,__,__,KH,__,__,__,__,__,HK,__,__,__,__,HK,__,HK,HK,HK,HK,HK,HK,HK,HK,HK,C7,HK,__},
/*Content         C7*/ {__,__,__,__,__,KH,__,__,__,__,__,HK,__,__,__,__,HK,__,HK,HK,HK,C8,HK,HK,HK,HK,HK,HK,HK,__},
/*Content-        C8*/ {__,__,__,__,__,KH,__,__,__,__,__,C9,__,__,__,__,HK,__,HK,HK,HK,HK,HK,HK,HK,HK,HK,HK,HK,__},
/*Content-l       C9*/ {__,__,__,__,__,KH,__,__,__,__,__,HK,__,__,__,__,HK,__,HK,HK,HK,HK,HK,HK,HK,HK,HK,HK,CA,__},
/*Content-le      CA*/ {__,__,__,__,__,KH,__,__,__,__,__,HK,__,__,__,__,HK,__,HK,CB,HK,HK,HK,HK,HK,HK,HK,HK,HK,__},
/*Content-len     CB*/ {__,__,__,__,__,KH,__,__,__,__,__,HK,__,__,__,__,HK,__,HK,HK,HK,HK,HK,HK,HK,HK,HK,CC,HK,__},
/*Content-leng    CC*/ {__,__,__,__,__,KH,__,__,__,__,__,HK,__,__,__,__,HK,__,HK,HK,CD,HK,HK,HK,HK,HK,HK,HK,HK,__},
/*Content-lengt   CD*/ {__,__,__,__,__,KH,__,__,__,__,__,HK,__,__,__,__,HK,__,HK,HK,HK,CE,HK,HK,HK,HK,HK,HK,HK,__},
/*Content-length  CE*/ {__,__,__,__,__,KH,__,__,__,__,__,HK,__,__,__,__,HK,__,HK,HK,HK,HK,HK,CF,HK,HK,HK,HK,HK,__},
/*Content-length: CF*/ {__,__,__,__,__,CG,__,__,__,__,__,HK,__,__,__,__,HK,__,HK,HK,HK,HK,HK,HK,HK,HK,HK,HK,HK,__},
/*Content-length: CG*/ {__,KC,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
/* CL value       CV*/ {__,__,__,VE,VE,__,__,__,__,__,__,__,__,__,__,VC,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
#endif
/*                E1*/ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,EA,__,__,EA,EA,__,__,__,__,__,__,EA,__,__,__},
/*                E2*/ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,EB,__,__,EB,EB,__,__,__,__,__,__,EB,__,__,__},
/*                FI*/ {__,__,__,__,EH,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
/* body           BT*/ {__,BC,BC,BC,BC,__,__,__,__,BC,BC,BC,BC,__,BH,BC,BC,__,BC,BC,BC,BC,BC,BC,BC,BC,BC,BC,BC,BC},
/*HT              R1*/ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,R2,__,__,__,__,__,__,__,__},
/*HTT             R2*/ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,R3,__,__,__,__,__,__,__,__},
/*HTTP            R3*/ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,R5,__,__,__,__,__,__,__},
/*HTTP/           R5*/ {__,__,__,__,__,__,__,__,__,R6,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
/*HTTP/[0-9]      R6*/ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,RA,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
/*HTTP/[0-9]/     R7*/ {__,__,__,__,__,__,__,__,__,__,__,__,R8,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
/*HTTP/[0-9]/[0-9]R8*/ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,RB,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
/*HTTP/[0-9]/[0-9]R9*/ {__,RN,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
/* response code  RN*/ {__,RD,__,__,__,__,__,__,__,__,__,__,__,__,__,RC,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
/* response desc  RD*/ {__,RD,__,RH,RH,RD,RD,RD,RD,RD,RD,RD,RD,RD,RD,RD,RD,RD,RD,RD,RD,RD,RD,RD,RD,RD,RD,RD,RD,RD},
};

/* compiled as jump table by gcc */
inline int parse_http_char(struct _http_parser *parser, const unsigned char c)
{
#define HTTP_FLG_POST (1 << 31)
#define HTTP_FLG_QS (1 << 30)
#define HTTP_FLG_BODYCONTENT (1 << 29)
#define HTTP_FLG_READY (1 << 28)

#define HTTP_ISQS() (parser->rx & HTTP_FLG_QS)
#define HTTP_ISPOST() (parser->rx & HTTP_FLG_POST)
#define HTTP_ISBODYCONTENT() (parser->rx & HTTP_FLG_BODYCONTENT)
#define HTTP_ISREADY() (parser->rx & HTTP_FLG_READY)

#define HTTP_PATHORQS (HTTP_ISQS() ? HTTP_QS_CHAR : HTTP_PATH_CHAR)

#define HTTP_CONSUME_BODY() \
    if (--parser->cl == 0) { \
        parser->rx |= HTTP_FLG_READY; \
        parser->callback(parser->ctx, HTTP_READY, 0, parser->step); \
    }


#define HTTP_BODY_AS_ENDED() (HTTP_ISBODYCONTENT() && --parser->cl == 0 && (parser->rx |= HTTP_FLG_READY, 1))

    parser_class c_classe;
    int8_t state;
    unsigned char ch;

    if (c >= 128) {
        c_classe = C_ETC;
    } else {
        c_classe = ascii_class[c];
    }

    parser->step++;

    if ((parser->state != AA && c_classe == C_NUL) || HTTP_ISREADY()) {
        return 0;
    }

    if (parser->state == AA) {
        parser->callback(parser->ctx, HTTP_BODY_CHAR, c, parser->step);
        HTTP_CONSUME_BODY();
        return 1;  
    }

    state = state_transition_table[parser->state][c_classe]; /* state > 0, action < 0 */

    if (state >= C2 && state <= CG) {
        parser->callback(parser->ctx, HTTP_HEADER_KEYC, c, parser->step);
    }

    if (state >= 0) {
        parser->state = state;
    } else {

        switch(state) {
            case MG: /* GET detected */
                parser->callback(parser->ctx, HTTP_METHOD, HTTP_GET, parser->step);
                parser->state = PT;
                break;
            case MP:
                parser->callback(parser->ctx, HTTP_METHOD, HTTP_POST, parser->step);
                parser->rx |= HTTP_FLG_POST;
                parser->state = PT;
                break;
            case PE:
                if (!HTTP_ISQS()) {
                    parser->callback(parser->ctx, HTTP_PATH_END, 0, parser->step);
                }
                parser->state = H1;

                break;
            case HA: /* HTTP Major */
                parser->callback(parser->ctx, HTTP_VERSION_MAJOR, c-'0', parser->step);
                parser->state = H7;
                break;
            case HB: /* HTTP Minor */
                parser->callback(parser->ctx, HTTP_VERSION_MINOR, c-'0', parser->step);
                parser->state = EL;
                break;
            case KH: /* Header key */
                parser->callback(parser->ctx, HTTP_HEADER_KEY, 0, parser->step);
                parser->state = HI;
                break;
            case VH: /* Header value */
                parser->callback(parser->ctx, HTTP_HEADER_VAL, 0, parser->step);
                parser->state = (c_classe == C_CR ? ER : C1); /* \r\n or \n */
                break;
            case KC: /* Content length */
                parser->callback(parser->ctx, HTTP_HEADER_KEY, 0, parser->step);
                parser->state = CV;
                break;
            case VC: /* Content length digit */
                if ((parser->cl = (parser->cl*10) + (c - '0')) > MAX_CL) {
                    return 0;
                }
                parser->callback(parser->ctx, HTTP_HEADER_VALC, c, parser->step);
                parser->state = CV;
                break;
            case VE:
                parser->callback(parser->ctx, HTTP_CL_VAL, parser->cl, parser->step);
                parser->callback(parser->ctx, HTTP_HEADER_VAL, 0, parser->step);
                parser->state = (c_classe == C_CR ? ER : C1); /* \r\n or \n */
                break;

            case EA: /* first char from %x */
                if (HTTP_BODY_AS_ENDED()) {
                    return 0;
                }
                ch = (unsigned char) (c | 0x20); /* tolower */

                parser->rx |= (unsigned char)(10 + ch -
                        ((ch >= '0' && ch <= '9') ? '0'+10 : 'a'))
                            | (c << 8);

                parser->state = E2;

                break;
            case EB: /* second char from %xx */
                ch = (unsigned char) (c | 0x20); /* tolower */

                ch = (unsigned char) (((parser->rx & 0x000000ff) << 4) + 10 + ch -
                        ((ch >= '0' && ch <= '9') ? '0'+10 : 'a'));

                parser->callback(parser->ctx, HTTP_ISBODYCONTENT() ?
                                                HTTP_BODY_CHAR :
                                                HTTP_PATHORQS,
                                            ch, parser->step); /* return the decoded char */
                parser->state = HTTP_ISBODYCONTENT() ? BT : PT;
                parser->rx &= 0xF0000000;
                if (HTTP_ISBODYCONTENT()) {
                    HTTP_CONSUME_BODY();
                }
                break;
            case EH:
                parser->callback(parser->ctx, HTTP_HEADER_END, 0, parser->step);
                if (HTTP_ISPOST()) {
                    parser->state = BT;
                    parser->rx = HTTP_FLG_POST | HTTP_FLG_QS | HTTP_FLG_BODYCONTENT;
                } else if (parser->rcode) {
                    parser->rx = HTTP_FLG_BODYCONTENT;
                    parser->state = AA;
                }
                if (parser->cl) break;  /* assume ready if 0/no content-length */
                parser->rx |= HTTP_FLG_READY;
                parser->callback(parser->ctx, HTTP_READY, 0, parser->step);
                break;
            case BC:
                parser->callback(parser->ctx, HTTP_BODY_CHAR, c, parser->step);
                HTTP_CONSUME_BODY();
                break;
            case PC:
            case QS:

                parser->state = PT;

                if (state == QS && !HTTP_ISQS()) {
                    parser->rx |= HTTP_FLG_QS;
                    parser->callback(parser->ctx, HTTP_PATH_END, 0, parser->step);
                    break;
                }

                parser->callback(parser->ctx, HTTP_PATHORQS, c, parser->step);

                break;
            case BH:
                if (HTTP_BODY_AS_ENDED()) {
                    return 0;
                }
                parser->state = E1;
                break;
            case HK:
                parser->callback(parser->ctx, HTTP_HEADER_KEYC, c, parser->step);
                parser->state = HH;
                break;
            case HV:
                parser->callback(parser->ctx, HTTP_HEADER_VALC, c, parser->step);
                parser->state = HI;
                break;
            case RA: /* HTTP Major */
                parser->callback(parser->ctx, HTTP_VERSION_MAJOR, c-'0', parser->step);
                parser->state = R7;
                break;
            case RB: /* HTTP Minor */
                parser->callback(parser->ctx, HTTP_VERSION_MINOR, c-'0', parser->step);
                parser->state = R9;
                break;
            case RC: /* HTTP response code */
                if ((parser->rcode = (parser->rcode*10) + (c - '0')) > MAX_RCODE) {
                    return 0;
                }
                parser->state = RN;
                break;
            case RH: /* Header value */
                parser->state = (c_classe == C_CR ? ER : C1); /* \r\n or \n */
                break;
            default:
                return 0;
        }
    }

    return 1;
}


#ifdef HTTP_TEST
/* also compiled as jump table */

static int parse_callback(void **ctx, callback_type type, int value, uint32_t step)
{
    switch(type) {
        case HTTP_METHOD:
            switch(value) {
                case HTTP_GET:
                    printf("GET method detected\n");
                    break;
                case HTTP_POST:
                    printf("POST method detected\n");
                    break;
            }
            break;
        case HTTP_PATH_END:
            printf("\n", step);
            break;
        case HTTP_PATH_CHAR:
            printf("%c", (unsigned char)value);
            break;
        case HTTP_VERSION_MAJOR:
        case HTTP_VERSION_MINOR:
            printf("Version detected %i\n", value);
            break;
        case HTTP_HEADER_KEY:
            printf("Header key\n");
            break;
        case HTTP_HEADER_VAL:
            printf("Header value\n");
            break;
        case HTTP_CL_VAL:
            printf("CL value : %i\n", value);
            break;
        case HTTP_HEADER_END:
            printf("--------- HEADERS END ---------\n");
            break;
        case HTTP_READY:
            printf("--------- HTTP END ---------\n");
            break;
        case HTTP_BODY_CHAR:
            printf("char %c\n", value);
            break;
        default:
            break;
    }
    return 1;
}

/* TEST */
int main()
{
    int length = 0, i;
    struct _http_parser p;

    /* Process BYTE_GET/POST opti check before running the parser */

    HTTP_PARSER_RESET(&p);

    p.ctx[0] = &p;
    p.callback = parse_callback;

    char chaine[] = "HTTP/1.1 200 OK\nfoo: bar\ncontent-length: 10\n\n1234567890";
    //char chaine[] = "POST / HTTP/1.1\ncontent-length: 8\n\n%40a%40b";
    /* TODO implement a "duff device" here */
    for (i = 0, length = strlen(chaine); i < length; i++) {
        if (parse_http_char(&p, chaine[i]) == 0) {
            printf("fail at %i\n", i);
            break;
        }
    }
    printf("done with status %d\n", p.rcode);
}
#endif

// vim: ts=4 sts=4 sw=4 et