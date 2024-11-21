/**
 * @brief main expression processor header
 */

#ifndef EP_H_
#define EP_H_

#include <stdio.h>

/// @brief maximal length of node variable name
#define EP_NODE_VAR_MAX ((size_t)16)

/// @brief node type (union tag, actually) representation enumeration
typedef enum __EpNodeType {
    EP_NODE_VARIABLE,        ///< variable
    EP_NODE_CONSTANT,        ///< number (a.k.a. constant)
    EP_NODE_BINARY_OPERATOR, ///< binary operator
    EP_NODE_UNARY_OPERATOR,  ///< unary operator
} EpNodeType;

typedef enum __EpBinaryOperator {
    EP_BINARY_OPERATOR_ADD,
    EP_BINARY_OPERATOR_SUB,
    EP_BINARY_OPERATOR_MUL,
    EP_BINARY_OPERATOR_DIV,
    EP_BINARY_OPERATOR_POW,
} EpBinaryOperator;

typedef enum __EpUnaryOperator {
    EP_UNARY_OPERATOR_NEG,
    EP_UNARY_OPERATOR_SIN,
    EP_UNARY_OPERATOR_COS,
} EpUnaryOperator;

/// @brief node type forward declaration
typedef struct __EpNode EpNode;

/// @brief node tagged union representation structure
struct __EpNode {
    EpNodeType type; ///< node type (union 'tag')

    union {
        char variable[EP_NODE_VAR_MAX]; ///< variable node name
        double constant;                ///< constnat node

        struct {
            EpBinaryOperator   op;  ///< binary operator
            EpNode           * lhs; ///< left hand side
            EpNode           * rhs; ///< right hand side
        } binaryOperator;

        struct {
            EpUnaryOperator   op;      ///< unary operator
            EpNode          * operand; ///< operand
        } unaryOperator;
    };
}; // struct __EpNode

/**
 * @brief node copying function
 * 
 * @param[in] node node to copy (non-null)
 * 
 * @return node copy
 */
EpNode * epNodeCopy( const EpNode *node );

/**
 * @brief node destructor
 * 
 * @param[in] node node to destroy (nullable)
 */
void epNodeDtor( EpNode *node );

/**
 * @brief node from number constructor
 * 
 * @param[in] value value to construct node from
 * 
 * @return created node pointer (null if allocation failed)
 */
EpNode * epNodeConstant( double constant );

/**
 * @brief variable node constructor
 * 
 * @param[in] varName variable name
 * 
 * @return created node pointer (null if allocation failed or name is longer than EP_NODE_VAR_MAX - 1)
 */
EpNode * epNodeVariable( const char *varName );

/**
 * @brief binary operator node constructor
 * 
 * @param[in] op  operator
 * @param[in] lhs left  hand side (created by node constructor)
 * @param[in] rhs right hand side (created by node constructor)
 * 
 * @return created node pointer (NULL in case if allocation failed or either of lhs and rhs is NULL. In this case destructors for lhs and rhs are called.)
 */
EpNode * epNodeBinaryOperator( EpBinaryOperator op, EpNode *lhs, EpNode *rhs );

/**
 * @brief unary operator node constructor
 * 
 * @param[in] op      unary operator
 * @param[in] operand operand expression
 * 
 * @return created node (null if allocation failed or operand is null)
 */
EpNode * epNodeUnaryOperator( EpUnaryOperator op, EpNode *operand );

typedef enum __EpParseExpressionStatus {
    EP_PARSE_EXPRESSION_OK,                      ///< 
    EP_PARSE_EXPRESSION_INTERNAL_ERROR,          ///< 
    EP_PARSE_EXPRESSION_UNKNOWN_BINARY_OPERATOR, ///< 
    EP_PARSE_EXPRESSION_NO_CLOSING_BRACKET,      ///< 
    EP_PARSE_EXPRESSION_TOO_LONG_VAR_NAME,       ///< 
    EP_PARSE_EXPRESSION_UNKNOWN_EXPRESSION,      ///< 
} EpParseExpressionStatus;

/// @brief expression parsing result representaiton structure (tagged union)
typedef struct __EpParseExpressionResult {
    EpParseExpressionStatus status; ///< parsing status

    union {
        struct {
            const char * rest;   ///< text remains
            EpNode     * result; ///< resulting node
        } ok; ///< operation succeeded

        char unknownBinaryOperator; ///< unknown binary operator occured

        struct {
            const char *begin; ///< name begin
            const char *end;   ///< name end (exclusive)
        } tooLongVarName; ///< too long variable name occured
    };
} EpParseExpressionResult;

/**
 * @brief infix expression parsing function
 * 
 * @param[in] str text to parse
 */
EpParseExpressionResult epParseExpression( const char *str );

/**
 * @brief node as infix expression displaying function
 * 
 * @param[in,out] out  file to print expression to
 * @param[in]     node node to display (non-null)
 */
void epPrintExpression( FILE *out, const EpNode *node );

#endif // !defined(EP_H_)
