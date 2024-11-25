/**
 * @brief main expression processor header
 */

#ifndef EP_H_
#define EP_H_

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _EP_NODE_SHORT_OPERATORS
    #define EP_ADD(lhs, rhs) (epNodeBinaryOperator(EP_BINARY_OPERATOR_ADD, (lhs), (rhs)))
    #define EP_SUB(lhs, rhs) (epNodeBinaryOperator(EP_BINARY_OPERATOR_SUB, (lhs), (rhs)))
    #define EP_MUL(lhs, rhs) (epNodeBinaryOperator(EP_BINARY_OPERATOR_MUL, (lhs), (rhs)))
    #define EP_DIV(lhs, rhs) (epNodeBinaryOperator(EP_BINARY_OPERATOR_DIV, (lhs), (rhs)))
    #define EP_POW(lhs, rhs) (epNodeBinaryOperator(EP_BINARY_OPERATOR_POW, (lhs), (rhs)))

    #define EP_SIN(op) (epNodeUnaryOperator(EP_UNARY_OPERATOR_SIN, (op)))
    #define EP_COS(op) (epNodeUnaryOperator(EP_UNARY_OPERATOR_COS, (op)))
    #define EP_NEG(op) (epNodeUnaryOperator(EP_UNARY_OPERATOR_NEG, (op)))
    #define EP_LN(op)  (epNodeUnaryOperator(EP_UNARY_OPERATOR_LN , (op)))

    #define EP_CONST(value) (epNodeConstant((value)))
    #define EP_VARIABLE(name) (epNodeVariable((name)))
#endif

/// @brief maximal length of node variable name
#define EP_NODE_VAR_MAX ((size_t)16)

/// @brief node type (union tag, actually) representation enumeration
typedef enum __EpNodeType {
    EP_NODE_VARIABLE,        ///< variable
    EP_NODE_CONSTANT,        ///< number (a.k.a. constant)
    EP_NODE_BINARY_OPERATOR, ///< binary operator
    EP_NODE_UNARY_OPERATOR,  ///< unary operator
} EpNodeType;

/// @brief binary operator representation enumeration
typedef enum __EpBinaryOperator {
    EP_BINARY_OPERATOR_ADD, ///< addition
    EP_BINARY_OPERATOR_SUB, ///< substraction
    EP_BINARY_OPERATOR_MUL, ///< multiplication
    EP_BINARY_OPERATOR_DIV, ///< division
    EP_BINARY_OPERATOR_POW, ///< raising to a power
} EpBinaryOperator;

/// @brief unary operator representation enumeration
typedef enum __EpUnaryOperator {
    EP_UNARY_OPERATOR_NEG, ///< negation
    EP_UNARY_OPERATOR_SIN, ///< sine
    EP_UNARY_OPERATOR_COS, ///< cosine
    EP_UNARY_OPERATOR_LN,  ///< natural logarithm
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

/**
 * @brief node derivative getting function
 * 
 * @param[in] node node to get derivative of (nullable)
 * @param[in] var  variable to calculate derivative by (non-null)
 * 
 * @return derivative node pointer. (null if node is null)
 */
EpNode * epNodeDerivative( const EpNode *node, const char *var );

/// @brief computation status
typedef enum __EpNodeComputeStatus {
    EP_NODE_COMPUTE_OK,               ///< computation succeeded
    EP_NODE_COMPUTE_UNKNOWN_VARIABLE, ///< unknown variable reference occured
} EpNodeComputeStatus;

/// @brief node computation result (tagged union)
typedef struct __EpNodeComputeResult {
    EpNodeComputeStatus status; ///< compute status

    union {
        double      ok;              ///< computation result
        const char *unknownVariable; ///< unknown variable
    };
} EpNodeComputeResult;

/// @brief variable representation structure
typedef struct __EpVariable {
    const char * name;  ///< variable name
    double       value; ///< variable current value
} EpVariable;

/**
 * @brief node value computation function
 * 
 * @param[in] node          node to compute (non-null)
 * @param[in] variables     variables used in computation array (non-null if variableCount != 0)
 * @param[in] variableCount count of variables used in computation
 */
EpNodeComputeResult epNodeCompute(
    const EpNode     * node,
    const EpVariable * variables,
    size_t             variableCount
);

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
 * 
 * @return expression parsing result
 */
EpParseExpressionResult epParseExpression( const char *str );

/**
 * @brief node as infix (that can be parsed by parser) expression displaying function
 * 
 * @param[in,out] out  file to print expression to
 * @param[in]     node node to display (non-null)
 */
void epPrintExpression( FILE *out, const EpNode *node );

#ifdef __cplusplus
}
#endif

#endif // !defined(EP_H_)
