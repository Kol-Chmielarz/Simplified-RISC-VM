/* $Id: scope_check.c,v 1.9 2023/11/01 13:20:24 leavens Exp $ */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scope_check.h"
#include "id_attrs.h"
#include "file_location.h"
#include "ast.h"
#include "utilities.h"
#include "symtab.h"
#include "scope_check.h"

// Build the symbol table for the given program AST
// and check the given program AST for duplicate declarations
// or uses of identifiers that were not declared
void scope_check_program(block_t prog)
{
    symtab_enter_scope();
    scope_check_block(prog);
    symtab_leave_scope();
}


// Build the symbol table for the given block AST
void scope_check_block(block_t prog)
{
    scope_check_constDecls(prog.const_decls);
    scope_check_varDecls(prog.var_decls);
    scope_check_proDecls(prog.proc_decls);
    scope_check_stmt(prog.stmt);
}

// Check the list of constant declarations in the AST
void scope_check_constDecls(const_decls_t cd)
{
    const_decl_t *cdp = cd.const_decls;
    while (cdp != NULL)
    {
        scope_check_constDecl(*cdp);
        cdp = cdp->next;
    }
}

// Check a single constant declaration
void scope_check_constDecl(const_decl_t cd)
{
    scope_check_constDefs(cd.const_defs);
}

// Check the list of constant definitions in the AST
void scope_check_constDefs(const_defs_t cdf)
{
    const_def_t *cdp = cdf.const_defs;
    while (cdp != NULL)
    {
        scope_check_constDef(*cdp);
        cdp = cdp->next;
    }
}

// Check a single constant definition
void scope_check_constDef(const_def_t cdf)
{
    scope_check_const(cdf.ident, cdf);
}

// Check the identifiers that make up a constant definition
void scope_check_const(ident_t ids, const_def_t cdf)
{
    scope_check_const_ident(ids, cdf);
}

// Check the list of procedure declarations in the AST
void scope_check_proDecls(proc_decls_t pds)
{
    proc_decl_t *pdp = pds.proc_decls;

    while (pdp != NULL)
    {
        pdp = pdp->next;
    }
}

// build the symbol table and check the declarations in vds
void scope_check_varDecls(var_decls_t vds)
{
    var_decl_t *vdp = vds.var_decls;
    while (vdp != NULL)
    {
        scope_check_varDecl(*vdp);
        vdp = vdp->next;
    }
}

// Add declarations to the current scope's symbol table for vd,
// producing errors for duplicate declarations
void scope_check_varDecl(var_decl_t vd)
{
    scope_check_idents(vd.idents, vd);
}

// Add declarations for the names in ids to the current scope's symbol table,
// for variables of the type vt,
// producing errors for any duplicate declarations
void scope_check_idents(idents_t ids, var_decl_t vt)
{
    ident_t *idp = ids.idents;
    while (idp != NULL)
    {
        scope_check_declare_ident(*idp, vt);
        idp = idp->next;
    }
}

// Add a declaration of the name id.name with the type vt
// to the current scope's symbol table,
// producing an error if this would be a duplicate declaration
void scope_check_declare_ident(ident_t id, var_decl_t vt)
{
    if (symtab_defined_in_current_scope(id.name))
    {
        if (symtab_lookup(id.name)->attrs->kind == constant_idk)
            bail_with_prog_error(*(id.file_loc), // Use the file location of the constant declaration
                                 "variable \"%s\" is already declared as a constant", id.name);
        else
        {
            bail_with_prog_error(*(id.file_loc), // Use the file location of the constant declaration
                                 "variable \"%s\" is already declared as a variable", id.name);
        }
    }

    else
    {
        int ofst_cnt = symtab_scope_loc_count();
        id_attrs *attrs = create_id_attrs(*(id.file_loc), variable_idk, ofst_cnt);
        symtab_insert(id.name, attrs);
    }
}

// Check for constant identifier declaration
void scope_check_const_ident(ident_t id, const_def_t cd)
{
    if (symtab_defined_in_current_scope(id.name))
    {
        id_use *prev = symtab_lookup(id.name);
        if (prev->attrs->kind == variable_idk)
        {

            bail_with_prog_error(*(cd.file_loc),
                                 "variable \"%s\" is already declared as a constant", id.name);
        }
        else
        {

            bail_with_prog_error(*(cd.file_loc),
                                 "constant \"%s\" is already declared as a constant", id.name);
        }
    }
    else
    {
        int ofst_cnt = symtab_scope_loc_count();
        id_attrs *attrs = create_id_attrs(*(id.file_loc), constant_idk, ofst_cnt);
        symtab_insert(id.name, attrs);
    }
}

// check the statement to make sure that
// all idenfifiers referenced in it have been declared
// (if not, then produce an error)
void scope_check_stmt(stmt_t stmt)
{
    switch (stmt.stmt_kind)
    {
    case assign_stmt:
        scope_check_assignStmt(stmt.data.assign_stmt);
        break;
    case begin_stmt:
        scope_check_beginStmt(stmt.data.begin_stmt);
        break;
    case if_stmt:
        scope_check_ifStmt(stmt.data.if_stmt);
        break;
    case read_stmt:
        scope_check_readStmt(stmt.data.read_stmt);
        break;
    case write_stmt:
        scope_check_writeStmt(stmt.data.write_stmt);
        break;
    case call_stmt:
        scope_check_callStmt(stmt.data.call_stmt);
        break;
    case while_stmt:
        scope_check_whileStmt(stmt.data.while_stmt);
        break;
    case skip_stmt:
        //do nothing
        break;

    default:
        break;
    }
}


// Check a while statement for its condition and body
void scope_check_whileStmt(while_stmt_t stmt)
{
    scope_check_condition(stmt.condition);

    symtab_enter_scope();

    scope_check_stmt(*(stmt.body));

    symtab_leave_scope();
}

// Check a call statement for its arguments
void scope_check_callStmt(call_stmt_t stmt)
{
    return;
}

// check the statement to make sure that
// all idenfifiers referenced in it have been declared
// (if not, then produce an error)
void scope_check_assignStmt(assign_stmt_t stmt)
{
    const char *name = stmt.name;
    id_use *idu = scope_check_ident_declared(*(stmt.file_loc), name);
    assert(idu != NULL);
    scope_check_expr(*(stmt.expr));
}

// check the statement to make sure that
// there are no duplicate declarations and that
// all idenfifiers referenced in it have been declared
// (if not, then produce an error)
void scope_check_beginStmt(begin_stmt_t stmt)
{
    symtab_enter_scope();
    scope_check_stmts(stmt.stmts);
    symtab_leave_scope();
}

// check the statements to make sure that
// all idenfifiers referenced in them have been declared
// (if not, then produce an error)
void scope_check_stmts(stmts_t stmts)
{
    stmt_t *sp = stmts.stmts;
    while (sp != NULL)
    {
        scope_check_stmt(*sp);
        sp = sp->next;
    }
}

// check the statement to make sure that
// all idenfifiers referenced in it have been declared
// (if not, then produce an error)
void scope_check_ifStmt(if_stmt_t stmt)
{

    scope_check_condition(stmt.condition);
    symtab_enter_scope();
    scope_check_stmt(*(stmt.then_stmt));
    scope_check_stmt(*(stmt.else_stmt));
    symtab_leave_scope();
}

// Check the condition expression for variable references
void scope_check_condition(condition_t condition)
{
    switch (condition.cond_kind)
    {
    case ck_odd:
        // Handle odd condition
        scope_check_expr(condition.data.odd_cond.expr);
        break;
    case ck_rel:
        // Handle relational condition
        scope_check_expr(condition.data.rel_op_cond.expr1);
        scope_check_expr(condition.data.rel_op_cond.expr2);
        break;
    default:
        // Handle unexpected condition kind (error)
        bail_with_error("Unexpected condition kind (%d) in scope_check_condition", condition.cond_kind);
        break;
    }
}

// check the statement to make sure that
// all idenfifiers referenced in it have been declared
// (if not, then produce an error)
void scope_check_readStmt(read_stmt_t stmt)
{
    scope_check_ident_declared(*(stmt.file_loc), stmt.name);
}

// check the statement to make sure that
// all idenfifiers referenced in it have been declared
// (if not, then produce an error)
void scope_check_writeStmt(write_stmt_t stmt)
{
    scope_check_expr(stmt.expr);
}

// check the expresion to make sure that
// all idenfifiers referenced in it have been declared
// (if not, then produce an error)
void scope_check_expr(expr_t exp)
{
    switch (exp.expr_kind)
    {
    case expr_bin:
        scope_check_binary_op_expr(exp.data.binary);
        break;
    case expr_ident:
        scope_check_ident_expr(exp.data.ident);
        break;
    case expr_number:
        // nothing to do
        break;
    default:
        bail_with_error("Unexpected expr_kind_e (%d) in scope_check_expr",
                        exp.expr_kind);
        break;
    }
}

// check the expression (exp) to make sure that
// all idenfifiers referenced in it have been declared
// (if not, then produce an error)
void scope_check_binary_op_expr(binary_op_expr_t exp)
{
    scope_check_expr(*(exp.expr1));
    // (note: no identifiers can occur in the operator)
    scope_check_expr(*(exp.expr2));
}

// check the identifier (id) to make sure that
// all it has been declared (if not, then produce an error)
void scope_check_ident_expr(ident_t id)
{
    scope_check_ident_declared(*(id.file_loc), id.name);
}

// check that the given name has been declared,
// if so, then return an id_use (containing the attributes) for that name,
// otherwise, produce an error using the file_location (floc) given.
id_use *scope_check_ident_declared(file_location floc, const char *name)
{
    id_use *ret = symtab_lookup(name);
    if (ret == NULL)
    {
        bail_with_prog_error(floc,
                             "identifer \"%s\" is not declared!",
                             name);
    }
    return ret;
}
