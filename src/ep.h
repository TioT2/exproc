/**
 * @brief main expression processor header
 */

#ifndef EP_H_
#define EP_H_

#include <stdio.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Short info about used macro definitions.
 * 
 * _EP_NODE_SHORT_OPERATORS - enables short wrappers around operator create functions
 */

/// shortened version of operations on nodes
#ifdef _EP_NODE_SHORT_OPERATORS
    #define EP_ADD(lhs, rhs) (epNodeBinaryOperator(EP_BINARY_OPERATOR_ADD, (lhs), (rhs)))
    #define EP_SUB(lhs, rhs) (epNodeBinaryOperator(EP_BINARY_OPERATOR_SUB, (lhs), (rhs)))
    #define EP_MUL(lhs, rhs) (epNodeBinaryOperator(EP_BINARY_OPERATOR_MUL, (lhs), (rhs)))
    #define EP_DIV(lhs, rhs) (epNodeBinaryOperator(EP_BINARY_OPERATOR_DIV, (lhs), (rhs)))
    #define EP_POW(lhs, rhs) (epNodeBinaryOperator(EP_BINARY_OPERATOR_POW, (lhs), (rhs)))

    #define EP_NEG(op) (epNodeUnaryOperator(EP_UNARY_OPERATOR_NEG, (op)))
    #define EP_LN(op)  (epNodeUnaryOperator(EP_UNARY_OPERATOR_LN , (op)))

    #define EP_SIN(op) (epNodeUnaryOperator(EP_UNARY_OPERATOR_SIN, (op)))
    #define EP_COS(op) (epNodeUnaryOperator(EP_UNARY_OPERATOR_COS, (op)))
    #define EP_TAN(op) (epNodeUnaryOperator(EP_UNARY_OPERATOR_TAN, (op)))
    #define EP_COT(op) (epNodeUnaryOperator(EP_UNARY_OPERATOR_COT, (op)))

    #define EP_ASIN(op) (epNodeUnaryOperator(EP_UNARY_OPERATOR_ASIN, (op)))
    #define EP_ACOS(op) (epNodeUnaryOperator(EP_UNARY_OPERATOR_ACOS, (op)))
    #define EP_ATAN(op) (epNodeUnaryOperator(EP_UNARY_OPERATOR_ATAN, (op)))
    #define EP_ACOT(op) (epNodeUnaryOperator(EP_UNARY_OPERATOR_ACOT, (op)))

    #define EP_CONST(value) (epNodeConstant((value)))
    #define EP_VARIABLE(name) (epNodeVariable((name)))
#endif // defined(_EP_NODE_SHORT_OPERATORS)

/// @brief double comparison epsilon
#define EP_DOUBLE_EPSILON ((double)0.0000001)

/**
 * @brief double numbers equality checking function
 * 
 * @param[in] lhs left hand side
 * @param[in] rhs right hand side
 * 
 * @return true if lhs and rhs are same in threshold, false otherwise
 */
bool epDoubleIsSame( double lhs, double rhs );

/// @brief maximal length of node variable name
#define EP_NODE_VAR_MAX ((size_t)16)

/// @brief node type (union tag, actually) representation enumeration
typedef enum __EpNodeType {
    EP_NODE_VARIABLE,        ///< variable
    EP_NODE_CONSTANT,        ///< number (a.k.a. constant)
    EP_NODE_BINARY_OPERATOR, ///< binary operator
    EP_NODE_UNARY_OPERATOR,  ///< unary operator
} EpNodeType;

/**
 * @brief node type corresponding string getting function
 * 
 * @param[in] ty node type
 * 
 * @return corresponding string
 */
const char * epNodeTypeStr( EpNodeType ty );

/// @brief binary operator representation enumeration
typedef enum __EpBinaryOperator {
    EP_BINARY_OPERATOR_ADD, ///< addition
    EP_BINARY_OPERATOR_SUB, ///< substraction
    EP_BINARY_OPERATOR_MUL, ///< multiplication
    EP_BINARY_OPERATOR_DIV, ///< division
    EP_BINARY_OPERATOR_POW, ///< raising to a power
} EpBinaryOperator;

/**
 * @brief binary operator corresponding string getting function
 * 
 * @param[in] op binary operator
 * 
 * @return corresponding string
 */
const char * epBinaryOperatorStr( EpBinaryOperator op );

/**
 * @brief binary operator priority getting function
 * 
 * @param[in] op binary operator to get priority of
 * 
 * @return operator priority
 */
int epBinaryOperatorGetPriority( EpBinaryOperator op );

/**
 * @brief binary operator apply function
 * 
 * @param[in] op  binary operator
 * @param[in] lhs left hand side
 * @param[in] rhs right hand side
 * 
 * @return result of applying op on lhs and rhs.
 */
double epBinaryOperatorApply( EpBinaryOperator op, double lhs, double rhs );

/// @brief unary operator representation enumeration
typedef enum __EpUnaryOperator {
    EP_UNARY_OPERATOR_NEG, ///< negation
    EP_UNARY_OPERATOR_LN,  ///< natural logarithm

    EP_UNARY_OPERATOR_SIN, ///< sine
    EP_UNARY_OPERATOR_COS, ///< cosine
    EP_UNARY_OPERATOR_TAN, ///< tangent
    EP_UNARY_OPERATOR_COT, ///< cotangent

    EP_UNARY_OPERATOR_ASIN, ///< arcsine
    EP_UNARY_OPERATOR_ACOS, ///< arccosine
    EP_UNARY_OPERATOR_ATAN, ///< arctangent
    EP_UNARY_OPERATOR_ACOT, ///< arccotangent
} EpUnaryOperator;

/**
 * @brief unary operator corresponding string getting function
 * 
 * @param[in] op unary operator
 * 
 * @return corresponding string
 */
const char * epUnaryOperatorStr( EpUnaryOperator op );

/**
 * @brief unary operator apply function
 * 
 * @param[in] op      unary operator
 * @param[in] operand operand
 * 
 * @return result of applying 'op' operator on operand.
 */
double epUnaryOperatorApply( EpUnaryOperator op, double operand );

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
 * @brief node comparison function
 * 
 * @param[in] lhs left hand side (non-null)
 * @param[in] rhs right hand side (non-null)
 * 
 * @return true if nodes same, false if not.
 */
bool epNodeIsSame( const EpNode *lhs, const EpNode *rhs );

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
 * @param[in] varName variable name (non-null)
 * 
 * @return created node pointer (null if allocation failed or name is longer than EP_NODE_VAR_MAX - 1)
 */
EpNode * epNodeVariable( const char *varName );

/**
 * @brief binary operator node constructor
 * 
 * @param[in] op  operator
 * @param[in] lhs left  hand side (nullable)
 * @param[in] rhs right hand side (nullable)
 * 
 * @return created node pointer (NULL in case if allocation failed or either of lhs and rhs is NULL. In this case destructors for lhs and rhs are called.)
 */
EpNode * epNodeBinaryOperator( EpBinaryOperator op, EpNode *lhs, EpNode *rhs );

/**
 * @brief unary operator node constructor
 * 
 * @param[in] op      unary operator
 * @param[in] operand operand expression (nullable)
 * 
 * @return created node (null if allocation failed or operand is null)
 */
EpNode * epNodeUnaryOperator( EpUnaryOperator op, EpNode *operand );

/**
 * @brief node derivative calculation function
 * 
 * @param[in] node node to get derivative of (nullable)
 * @param[in] var  variable to calculate derivative by (non-null)
 * 
 * @return derivative node pointer. (null if node is null or internal error occured)
 */
EpNode * epNodeDerivative( const EpNode *node, const char *var );

/**
 * @brief node by taylor series approximation getting function
 * 
 * @param[in] node  node to unfold
 * @param[in] var   variable
 * @param[in] point point to unfold in taylor series around
 * @param[in] count count of sum participants
 * 
 * @return approximation function
 */
EpNode * epNodeTaylor(
    const EpNode * node,
    const char   * var,
    const EpNode * point,
    unsigned int   count
);

/// @brief substitution representation structure
typedef struct __EpSubstitution {
    const char   * name; ///< substituted variable name
    const EpNode * node; ///< node to substitute
} EpSubstitution;

/**
 * @brief node variable substitution function
 * 
 * @param[in] node              node to substitute
 * @param[in] substitutions     expression to substitute array
 * @param[in] substitutionCount substitution array size
 * 
 * @note this functions performs single substitution step
 * 
 * @return node with substituted variables
 */
EpNode * epNodeSubstitute(
    const EpNode         * node,
    const EpSubstitution * substitutions,
    size_t                 substitutionCount
);

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

/**
 * @brief node optimization function
 * 
 * @param[in] node to optimize (nullable)
 * 
 * @return optimized node (may be null)
 */
EpNode * epNodeOptimize( const EpNode *node );

/// @brief expression parsing status
typedef enum __EpParseExpressionStatus {
    EP_PARSE_EXPRESSION_OK,                               ///< parsing succeeded
    EP_PARSE_EXPRESSION_INTERNAL_ERROR,                   ///< internal error occured
    EP_PARSE_EXPRESSION_NO_CLOSING_BRACKET,               ///< no closing bracket in binary operator
    EP_PARSE_EXPRESSION_TOO_LONG_VAR_NAME,                ///< variable name exceeds EP_NODE_VAR_MAX
    EP_PARSE_EXPRESSION_UNKNOWN_TOKEN,                    ///< given text is not an expression
    EP_PARSE_EXPRESSION_NO_END,                           ///< expression end expected
    EP_PARSE_EXPRESSION_UNEXPECTED_EXPRESSION_END,        ///< unexpected expression end
    EP_PARSE_EXPRESSION_NUMBER_IDENT_OR_BRACKET_EXPECTED, ///< number or ident expected
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

/// @brief dumping representation structure
typedef enum __EpDumpFormat {
    EP_DUMP_INFIX_EXPRESSION,  ///< more 'general' expression format
    EP_DUMP_TEX,               ///< TeX expression
} EpDumpFormat;

/**
 * @brief node dumping function
 * 
 * @param[in] out    output file
 * @param[in] node   node to dump
 * @param[in] format dumping format
 */
void epNodeDump( FILE *out, const EpNode *node, EpDumpFormat format );

/**
 * @brief TeX graph from node generation function
 * 
 * @param[in] out  text file to generate grpah text to
 * @param[in] node node to generate TeX graph for
 */
void epNodeGenNodeFunctionInfo( FILE *out, const EpNode *node );

/**
 * @brief node in dot format dumping function
 * 
 * @param[in] out  output file
 * @param[in] node node to dump
 */
void epDbgNodeDumpDot( FILE *out, const EpNode *node );

#ifdef __cplusplus
}
#endif

#endif // !defined(EP_H_)
