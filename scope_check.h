#ifndef _SCOPE_CHECK_H
#define _SCOPE_CHECK_H
#include "ast.h"
#include "id_use.h"

// Check the variable declarations, constant declarations, and procedure declarations
// in the given program block and the statements within it.
extern void scope_check_block(block_t prog);

// Check the entire program, including the program block, and create a new scope
// for the main program.
extern void scope_check_program(block_t prog);

// Check the condition expression for variable references.
extern void scope_check_condition(condition_t condition);

// Placeholder function to check a call statement, implement if needed.
extern void scope_check_callStmt(call_stmt_t stmt);

// Check the variable declarations within a block.
extern void scope_check_varDecls(var_decls_t vds);

// Check a single variable declaration, including the identifiers in it.
extern void scope_check_varDecl(var_decl_t vd);

// Check a list of identifiers in a variable declaration.
extern void scope_check_idents(idents_t ids, var_decl_t vt);

// Declare an identifier within the current scope and check for duplicates.
extern void scope_check_declare_ident(ident_t id, var_decl_t vt);

// Check a statement, including assignment, begin, if, read, write, call, skip, or while statements.
extern void scope_check_stmt(stmt_t stmt);

// Check an assignment statement for variable references and scope.
extern void scope_check_assignStmt(assign_stmt_t stmt);

// Check a begin statement, entering a new scope for its body.
extern void scope_check_beginStmt(begin_stmt_t stmt);

// Check a list of statements within a begin statement.
extern void scope_check_stmts(stmts_t stmts);

// Check an if statement, including its condition and branches.
extern void scope_check_ifStmt(if_stmt_t stmt);

// Check a read statement for variable references.
extern void scope_check_readStmt(read_stmt_t stmt);

// Check a write statement for variable references.
extern void scope_check_writeStmt(write_stmt_t stmt);

// Check an expression, including binary operations, identifiers, and numbers.
extern void scope_check_expr(expr_t exp);

// Check a binary operation expression.
extern void scope_check_binary_op_expr(binary_op_expr_t exp);

// Check a list of constant definitions.
extern void scope_check_constDefs(const_defs_t cdf);

// Check a single constant definition.
extern void scope_check_constDef(const_def_t cdf);

// Declare a constant identifier within the current scope and check for duplicates.
extern void scope_check_const(ident_t ids, const_def_t cdf);

// Check an identifier within an expression.
extern void scope_check_ident_expr(ident_t id);

// Check a list of constant declarations.
extern void scope_check_constDecls(const_decls_t cds);

// Check a list of procedure declarations.
extern void scope_check_proDecls(proc_decls_t pds);

// Check a single constant declaration.
extern void scope_check_constDecl(const_decl_t cdp);

// Check a while statement, including its condition and body.
extern void scope_check_whileStmt(while_stmt_t stmt);

// Check if an identifier has been declared, and produce an error if not.
extern id_use *scope_check_ident_declared(file_location floc, const char *name);

// Declare a constant identifier within the current scope and check for duplicates.
extern void scope_check_const_ident(ident_t id, const_def_t cdf);

#endif