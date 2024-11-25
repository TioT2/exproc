/**
 * @brief expression parsers implementation file
 */

#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#include "ep.h"

/**
 * @brief space skipping function
 * 
 * @param[in] str to skip heading space in
 * 
 * @return str without heading spaces pointer
 */
static const char * epParseSkipSpaces( const char *str ) {
    while (isspace(*str))
        str++;
    return str;
} // epParseSkipSpaces

EpParseExpressionResult epParseExpression( const char *str ) {
    assert(str != NULL);

    str = epParseSkipSpaces(str);

    // start binary operator parsing
    if (*str == '(') {
        // parse binary operator
        // parse left subexpression
        EpParseExpressionResult lhs = epParseExpression(str + 1);

        if (lhs.status != EP_PARSE_EXPRESSION_OK)
            return lhs;
        str = epParseSkipSpaces(lhs.ok.rest);

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

        EpBinaryOperator op = EP_BINARY_OPERATOR_ADD;
        bool opFound = false;

        for (uint32_t i = 0; i < sizeof(binaryOperators) / sizeof(binaryOperators[0]); i++) {
            if (*str == binaryOperators[i].name) {
                str++;

                opFound = true;
                op = binaryOperators[i].op;
                break;
            }
        }

        if (!opFound) {
            epNodeDtor(lhs.ok.result);
            return (EpParseExpressionResult) {
                .status = EP_PARSE_EXPRESSION_UNKNOWN_BINARY_OPERATOR,
                .unknownBinaryOperator = *str,
            };
        }

        EpParseExpressionResult rhs = epParseExpression(str);
        if (rhs.status != EP_PARSE_EXPRESSION_OK) {
            epNodeDtor(lhs.ok.result);
            return rhs;
        }
        str = epParseSkipSpaces(rhs.ok.rest);

        if (*str != ')') {
            epNodeDtor(lhs.ok.result);
            epNodeDtor(rhs.ok.result);
            return (EpParseExpressionResult) { .status = EP_PARSE_EXPRESSION_NO_CLOSING_BRACKET };
        }
        str++;

        EpNode *result = epNodeBinaryOperator(op, lhs.ok.result, rhs.ok.result);

        if (result == NULL)
            return (EpParseExpressionResult) { .status = EP_PARSE_EXPRESSION_INTERNAL_ERROR };

        return (EpParseExpressionResult) {
            .status = EP_PARSE_EXPRESSION_OK,
            .ok = {
                .rest = str,
                .result = result,
            },
        };
    }

    if (*str >= '0' && *str <= '9') {
        char *end = NULL;
        EpNode *result = epNodeConstant(strtod(str, &end));

        return result == NULL
            ? (EpParseExpressionResult) { .status = EP_PARSE_EXPRESSION_INTERNAL_ERROR }
            : (EpParseExpressionResult) {
                .status = EP_PARSE_EXPRESSION_OK,
                .ok = {
                    .rest = end,
                    .result = result,
                },
            };
    }

    static const struct {
        const char *name;
        EpUnaryOperator op;
    } unaryOperators[] = {
        {"-",   EP_UNARY_OPERATOR_NEG},
        {"sin", EP_UNARY_OPERATOR_SIN},
        {"cos", EP_UNARY_OPERATOR_COS},
        {"ln",  EP_UNARY_OPERATOR_LN },
    };

    for (uint32_t i = 0; i < sizeof(unaryOperators) / sizeof(unaryOperators[0]); i++) {
        const size_t nameLength = strlen(unaryOperators[i].name);

        if (strncmp(str, unaryOperators[i].name, nameLength) == 0) {
            str += nameLength;

            EpParseExpressionResult operandParsingResult = epParseExpression(str);

            if (operandParsingResult.status != EP_PARSE_EXPRESSION_OK)
                return operandParsingResult;
            str = operandParsingResult.ok.rest;

            EpNode *result = epNodeUnaryOperator(unaryOperators[i].op, operandParsingResult.ok.result);

            if (result == NULL)
                return (EpParseExpressionResult) { .status = EP_PARSE_EXPRESSION_INTERNAL_ERROR };

            return (EpParseExpressionResult) {
                .status = EP_PARSE_EXPRESSION_OK,
                .ok = {
                    .rest = str,
                    .result = result,
                },
            };
        }
    }

    if (isalpha(*str)) {
        const char *first = str;

        while (isalnum(*str) || *str == '_')
            str++;

        const char *last = str;

        const size_t length = last - first;

        if (length > EP_NODE_VAR_MAX - 1)
            return (EpParseExpressionResult) {
                .status = EP_PARSE_EXPRESSION_TOO_LONG_VAR_NAME,
                .tooLongVarName = {
                    .begin = first,
                    .end = last,
                }
            };

        char nameBuffer[EP_NODE_VAR_MAX] = {0};

        memcpy(nameBuffer, first, last - first);

        EpNode *node = epNodeVariable(nameBuffer);
        if (node == NULL)
            return (EpParseExpressionResult) { .status = EP_PARSE_EXPRESSION_INTERNAL_ERROR };
        return (EpParseExpressionResult) {
            .status = EP_PARSE_EXPRESSION_OK,
            .ok = {
                .rest = last,
                .result = node,
            },
        };
    }

    return (EpParseExpressionResult) {
        .status = EP_PARSE_EXPRESSION_UNKNOWN_EXPRESSION,
    };
} // epParseExpressionImpl

// ep_parser.c
