/**
 * @brief expression parsers implementation file
 */

#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "ep.h"

typedef struct __EpParseExpressionImplResult {
    EpExpressionParsingStatus   status;

    const char                * rest;
    EpNode                    * result;
} EpParseExpressionImplResult;

const char * epParseSkipSpaces( const char *str ) {
    while (isspace(*str))
        str++;
    return str;
} // epParseSkipSpaces

EpParseExpressionImplResult epParseExpressionImpl( const char *str ) {
    assert(str != NULL);

    str = epParseSkipSpaces(str);

    // start binary operator parsing
    if (*str == '(') {
        // parse binary operator
        // parse left subexpression
        EpParseExpressionImplResult lhs = epParseExpressionImpl(str + 1);

        if (lhs.status != EP_EXPRESSION_PARSING_OK)
            return (EpParseExpressionImplResult) { .status = lhs.status };
        str = epParseSkipSpaces(lhs.rest);

        static const struct {
            char name;
            EpBinaryOperator op;
        } binaryOperators[] = {
            {'+', EP_BINARY_OPERATOR_ADD},
            {'-', EP_BINARY_OPERATOR_SUB},
            {'*', EP_BINARY_OPERATOR_MUL},
            {'/', EP_BINARY_OPERATOR_DIV},
            {'^', EP_BINARY_OPERATOR_POW},
        };
    }

    if (*str >= '0' && *str <= '9') {
        // parse constant
    }

    static const struct {
        const char *name;
        EpUnaryOperator op;
    } unaryOperators[] = {
        {"-",   EP_UNARY_OPERATOR_NEG},
        {"sin", EP_UNARY_OPERATOR_SIN},
        {"cos", EP_UNARY_OPERATOR_COS},
    };

    for (uint32_t i = 0; i < sizeof(unaryOperators) / sizeof(unaryOperators[0]); i++) {
        if (strncmp(str, unaryOperators[i].name, strlen(unaryOperators[i].name)) == 0) {
            // parse corresponding value
        }
    }

    assert(false && "unimplemented");
    return {}; // TODO FINISH IMPLEMENTATION
} // epParseExpressionImpl

EpExpressionParsingStatus epParseExpression( const char *str, EpNode **dst ) {
    assert(false && "unimplemented");
    return EP_EXPRESSION_PARSING_INTERNAL_ERROR;
} // epParseExpression

// ep_parser.c
