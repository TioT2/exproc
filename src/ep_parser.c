/**
 * @brief expression parsers implementation file
 */

#include <assert.h>
#include <ctype.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#include "ep.h"

/// @brief token type
typedef enum __EpParserTokenType {
    EP_PARSER_TOKEN_NUMBER,   ///< floating point number
    EP_PARSER_TOKEN_IDENT,    ///< ident
    EP_PARSER_TOKEN_LEFT_BR,  ///< (
    EP_PARSER_TOKEN_RIGHT_BR, ///< )
    EP_PARSER_TOKEN_PLUS,     ///< +
    EP_PARSER_TOKEN_MINUS,    ///< -
    EP_PARSER_TOKEN_SLASH,    ///< /
    EP_PARSER_TOKEN_ASTERISK, ///< *
    EP_PARSER_TOKEN_CARET,    ///< ^
    EP_PARSER_TOKEN_END,      ///< \0 (trailing token)
} EpParserTokenType;

/// @brief token representation structure
typedef struct __EpParserToken {
    EpParserTokenType type; ///< union 'tag'

    union {
        char ident[EP_NODE_VAR_MAX]; ///< ident token
        double number;               ///< number
    };
} EpParserToken;

/// @brief parsing context representation structure
typedef struct __EpParser {
    const char    *         str;     ///< string slice
    EpParserToken           current; ///< current token
    EpParseExpressionResult result;  ///< expression result
} EpParser;


/**
 * @brief next token parsing function
 * 
 * @param[in] self parser pointer
 * 
 * @return true if parsed successfully, false if smth went wrong
 */
static bool epParserNext( EpParser *const self ) {
    while (isspace(*self->str))
        self->str++;

    const char current = *self->str;

    // quite bad fix, actually
    if (current == '\0') {
        self->current.type = EP_PARSER_TOKEN_END;
        return true;
    }

    bool found = true;

    switch (current) {
    case '+' : self->current.type = EP_PARSER_TOKEN_PLUS     ; break;
    case '-' : self->current.type = EP_PARSER_TOKEN_MINUS    ; break;
    case '*' : self->current.type = EP_PARSER_TOKEN_ASTERISK ; break;
    case '/' : self->current.type = EP_PARSER_TOKEN_SLASH    ; break;
    case '^' : self->current.type = EP_PARSER_TOKEN_CARET    ; break;
    case '(' : self->current.type = EP_PARSER_TOKEN_LEFT_BR  ; break;
    case ')' : self->current.type = EP_PARSER_TOKEN_RIGHT_BR ; break;
    default : found = false;
    }

    if (found) {
        self->str++;
        return true;
    }

    if (isdigit(current) || current == '-' || current == '.') {
        char *end = NULL;

        self->current.type = EP_PARSER_TOKEN_NUMBER;
        self->current.number = strtod(self->str, &end);

        self->str = end;
        return true;
    }

    if (isalnum(current) || current == '_') {
        const char *end = self->str;

        while (isalnum(*end) || *end == '_')
            end++;

        if (end - self->str >= EP_NODE_VAR_MAX - 1) {
            self->result.status = EP_PARSE_EXPRESSION_TOO_LONG_VAR_NAME;
            self->result.tooLongVarName.begin = self->str;
            self->result.tooLongVarName.end = end;

            return false;
        }

        self->current.type = EP_PARSER_TOKEN_IDENT;
        memset(self->current.ident, 0, EP_NODE_VAR_MAX);
        memcpy(self->current.ident, self->str, end - self->str);

        self->str = end;
        return true;
    }

    self->result.status = EP_PARSE_EXPRESSION_UNKNOWN_TOKEN;
    return false;
} // epParserNext

/**
 * @brief parser setting up function
 * 
 * @param[in]  str string to parse pointer
 * @param[out] dst parser pointer (non-null)
 * 
 * @return true if parser initialization succeeded, false if not.
 * @note in case if start failed, dst->result field contains error that happened during parser initialization
 */
bool epParserStart( const char *str, EpParser *dst ) {
    *dst = (EpParser) {
        .str = str,
        .current = {},
        .result = {},
    };

    return epParserNext(dst);
} // epParserStart

/**
 * @brief '+' and '-' operators parsing function
 * 
 * @param[in]  self parser pointer
 * @param[out] dst  node parsing destination (non-null)
 * 
 * @return true if parsed successfully, false if not.
 */
static bool epParseSum( EpParser *const self, EpNode **dst );

/**
 * @brief '*' and '/' operators parsing function
 * 
 * @param[in]  self parser pointer
 * @param[out] dst  node parsing destination (non-null)
 * 
 * @return true if parsed successfully, false if not.
 */
static bool epParseProduct( EpParser *const self, EpNode **dst );

/**
 * @brief '^' operator parsing function
 * 
 * @param[in]  self parser pointer
 * @param[out] dst  node parsing destination (non-null)
 * 
 * @return true if parsed successfully, false if not.
 */
static bool epParsePower( EpParser *const self, EpNode **dst );

/**
 * @brief 'basic' expression parsing function (unary operator and number | variable | expression in brackets)
 * 
 * @param[in]  self parser pointer
 * @param[out] dst  node parsing destination (non-null)
 * 
 * @return true if parsed successfully, false if not.
 */
static bool epParseExpressionImpl( EpParser *const self, EpNode **dst );


bool epParseSum( EpParser *const self, EpNode **dst ) {
    EpNode *lhs = NULL;

    if (!epParseProduct(self, &lhs))
        return false;

    for (;;) {
        EpBinaryOperator op;

        switch (self->current.type) {
        case EP_PARSER_TOKEN_PLUS  : op = EP_BINARY_OPERATOR_ADD; break;
        case EP_PARSER_TOKEN_MINUS : op = EP_BINARY_OPERATOR_SUB; break;

        default:
            *dst = lhs;
            return true;
        }

        if (!epParserNext(self)) {
            epNodeDtor(lhs);
            return false;
        }

        // parse rhs
        EpNode *rhs = NULL;
        if (!epParseProduct(self, &rhs)) {
            epNodeDtor(lhs);
            return false;
        }

        // ok, because epNodeBinaryOperator gathers lhs and rhs ownership.
        lhs = epNodeBinaryOperator(op, lhs, rhs);

        if (lhs == NULL) {
            self->result.status = EP_PARSE_EXPRESSION_INTERNAL_ERROR;
            return false;
        }
    }
} // epParseSum

bool epParseProduct( EpParser *const self, EpNode **dst ) {
    EpNode *lhs = NULL;

    if (!epParsePower(self, &lhs))
        return false;

    for (;;) {
        EpBinaryOperator op;

        switch (self->current.type) {
        case EP_PARSER_TOKEN_ASTERISK : op = EP_BINARY_OPERATOR_MUL; break;
        case EP_PARSER_TOKEN_SLASH    : op = EP_BINARY_OPERATOR_DIV; break;

        default:
            *dst = lhs;
            return true;
        }

        if (!epParserNext(self)) {
            epNodeDtor(lhs);
            return false;
        }

        // parse rhs
        EpNode *rhs = NULL;
        if (!epParsePower(self, &rhs)) {
            epNodeDtor(lhs);
            return false;
        }

        // ok, because epNodeBinaryOperator gathers lhs and rhs ownership.
        lhs = epNodeBinaryOperator(op, lhs, rhs);

        if (lhs == NULL) {
            self->result.status = EP_PARSE_EXPRESSION_INTERNAL_ERROR;
            return false;
        }
    }
} // epParseProduct

bool epParsePower( EpParser *const self, EpNode **dst ) {
    EpNode *lhs = NULL;

    if (!epParseExpressionImpl(self, &lhs))
        return false;

    for (;;) {
        if (self->current.type != EP_PARSER_TOKEN_CARET) {
            *dst = lhs;
            return true;
        }

        if (!epParserNext(self)) {
            epNodeDtor(lhs);
            return false;
        }

        // parse rhs
        EpNode *rhs = NULL;
        if (!epParseExpressionImpl(self, &rhs)) {
            epNodeDtor(lhs);
            return false;
        }

        // ok, because epNodeBinaryOperator gathers lhs and rhs ownership.
        lhs = epNodeBinaryOperator(EP_BINARY_OPERATOR_POW, lhs, rhs);

        if (lhs == NULL) {
            self->result.status = EP_PARSE_EXPRESSION_INTERNAL_ERROR;
            return false;
        }
    }
} // epParsePower

bool epParseExpressionImpl( EpParser *const self, EpNode **dst ) {
    EpNode *result = NULL;

    EpUnaryOperator unaryOperator = EP_UNARY_OPERATOR_ACOS;
    bool unaryParsed = false;

    static struct {
        const char *name;
        EpUnaryOperator op;
    } unaryOperators[] = {
        {"ln"     , EP_UNARY_OPERATOR_LN   },
        {"sin"    , EP_UNARY_OPERATOR_SIN  },
        {"cos"    , EP_UNARY_OPERATOR_COS  },
        {"tan"    , EP_UNARY_OPERATOR_TAN  },
        {"cot"    , EP_UNARY_OPERATOR_COT  },
        {"arcsin" , EP_UNARY_OPERATOR_ASIN },
        {"arccos" , EP_UNARY_OPERATOR_ACOS },
        {"arctan" , EP_UNARY_OPERATOR_ATAN },
        {"arccot" , EP_UNARY_OPERATOR_ACOT },
    };

    // try to parse prefix unary operator
    if (self->current.type == EP_PARSER_TOKEN_IDENT)
        for (size_t i = 0; i < sizeof(unaryOperators) / sizeof(unaryOperators[0]); i++)
            if (strcmp(unaryOperators[i].name, self->current.ident) == 0) {
                unaryParsed = true;
                unaryOperator = unaryOperators[i].op;

                if (!epParserNext(self))
                    return false;

                break;
            }

    switch (self->current.type) {
    case EP_PARSER_TOKEN_LEFT_BR:
        if (!epParserNext(self))
            return false;

        if (!epParseSum(self, &result))
            return false;

        if (self->current.type != EP_PARSER_TOKEN_RIGHT_BR) {
            epNodeDtor(result);
            self->result.status = EP_PARSE_EXPRESSION_NO_CLOSING_BRACKET;
            return false;
        }
        break;

    case EP_PARSER_TOKEN_IDENT:
        result = epNodeVariable(self->current.ident);
        break;

    case EP_PARSER_TOKEN_NUMBER:
        result = epNodeConstant(self->current.number);
        break;

    default:
        self->result.status = EP_PARSE_EXPRESSION_NUMBER_IDENT_OR_BRACKET_EXPECTED;
        return false;
    }

    if (!epParserNext(self)) {
        epNodeDtor(result);
        return false;
    }

    if (unaryParsed)
        result = epNodeUnaryOperator(unaryOperator, result);

    if (result == NULL) {
        self->result.status = EP_PARSE_EXPRESSION_INTERNAL_ERROR;
        return false;
    }

    *dst = result;
    return true;
} // epParseExpression

bool epParseGrammar( EpParser *const self, EpNode **dst ) {
    EpNode *sumDst = NULL;

    if (!epParseSum(self, &sumDst))
        return false;

    if (self->current.type != EP_PARSER_TOKEN_END) {
        epNodeDtor(sumDst);
        self->result.status = EP_PARSE_EXPRESSION_NO_END;
        return false;
    }

    *dst = sumDst;
    return true;
} // epParseGrammar

EpParseExpressionResult epParseExpression( const char *str ) {
    assert(str != NULL);

    EpParser parser = {};
    EpNode *dst = NULL;

    return epParserStart(str, &parser) && epParseGrammar(&parser, &dst)
        ? (EpParseExpressionResult) {
            .status = EP_PARSE_EXPRESSION_OK,
            .ok = {
                .rest = parser.str,
                .result = dst,
            },
        }
        : parser.result
    ;
} // epParseExpression

// ep_parser.c
