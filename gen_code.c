#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <limits.h>
#include "ast.h"
#include "bof.h"
#include "instruction.h"
#include "code.h"
#include "gen_code.h"
#include "regname.h"
#include "literal_table.h"
#include "id_use.h"
#include "utilities.h"
#include "pl0.tab.h"

#define STACK_SPACE 4096

static void gen_code_output_seq(BOFFILE bf, code_seq cs)
{
    while (!code_seq_is_empty(cs))
    {
        bin_instr_t inst = code_seq_first(cs)->instr;
        instruction_write_bin_instr(bf, inst);
        cs = code_seq_rest(cs);
    }
}

static BOFHeader gen_code_program_header(code_seq main_cs)
{
    BOFHeader ret;
    strncpy(ret.magic, "FBF", 4);
    ret.text_start_address = 0;
    ret.text_length = code_seq_size(main_cs) * BYTES_PER_WORD;
    int dsa = MAX(ret.text_length, 1024);
    ret.data_start_address = dsa;
    ret.data_length = literal_table_size() * BYTES_PER_WORD;
    int sba = dsa + ret.text_length + ret.data_start_address + ret.data_length + STACK_SPACE;
    ret.stack_bottom_addr = sba;

    return ret;
}

static void gen_code_output_literals(BOFFILE bf)
{
    literal_table_start_iteration();
    while (literal_table_iteration_has_next())
    {
        word_type w = literal_table_iteration_next();
        // debug_print("Writing literal %f to BOF file\n", w);
        bof_write_word(bf, w);
    }
    literal_table_end_iteration(); // not necessary
}

// Requires: bf is open for writing in binary
// Write the program's BOFFILE to bf
static void gen_code_output_program(BOFFILE bf, code_seq main_cs)
{
    BOFHeader bfh = gen_code_program_header(main_cs);
    bof_write_header(bf, bfh);
    gen_code_output_seq(bf, main_cs);
    gen_code_output_literals(bf);
    bof_close(bf);
}

// Initialize the code generator
void gen_code_initialize()
{
    literal_table_initialize();
}

// Requires: bf if open for writing in binary
// Generate code for prog into bf
void gen_code_program(BOFFILE bf, block_t prog)
{
    code_seq main_cs;

    main_cs = gen_code_const_decls(prog.const_decls);
    main_cs = gen_code_var_decls(prog.var_decls);
    int var_len = (code_seq_size(main_cs) / 2) * BYTES_PER_WORD;
    int const_len = literal_table_size() * BYTES_PER_WORD;
    main_cs = code_seq_concat(main_cs, code_allocate_stack_space(var_len + const_len));
    main_cs = code_seq_concat(main_cs, code_save_registers_for_AR());
    main_cs = code_seq_concat(main_cs, gen_code_stmt(prog.stmt));
    main_cs = code_seq_concat(main_cs, code_restore_registers_from_AR());
    main_cs = code_seq_concat(main_cs, code_deallocate_stack_space(var_len));

    main_cs = code_seq_add_to_end(main_cs, code_exit());
    gen_code_output_program(bf, main_cs);
}

// Requires: bf if open for writing in binary
// Generate code for the given AST
code_seq gen_code_block(block_t blk)
{

    code_seq seq = code_seq_empty();

    // Generate code for constant declarations
    seq = code_seq_concat(seq, gen_code_const_decls(blk.const_decls));
    // Generate code for variable declarations
    seq = code_seq_concat(seq, gen_code_var_decls(blk.var_decls));

    // Generate code for procedure declarations
    seq = code_seq_concat(seq, gen_code_proc_decls(blk.proc_decls));

    // Generate code for statements
    seq = code_seq_concat(seq, gen_code_stmt(blk.stmt));

    return seq;
}


// Generate code for the const-decls, cds
// There are 3 instructions generated for each identifier declared
// (one to allocate space and two to initialize that space)
code_seq gen_code_const_decls(const_decls_t cds)
{

    code_seq ret = code_seq_empty();

    // Iterate over the constant declarations
    const_decl_t *current_decl = cds.const_decls;
    while (current_decl != NULL)
    {
        ret = code_seq_concat(ret, gen_code_const_decl(*current_decl));

        // Move to the next declaration
        current_decl = current_decl->next;
    }

    return ret;
}

// Generate code for the const-decl, cd
code_seq gen_code_const_decl(const_decl_t cd)
{
    return gen_code_const_defs(cd.const_defs);
}

// Generate code for the const-defs, cdfs
code_seq gen_code_const_defs(const_defs_t cdfs)
{
    code_seq ret = code_seq_empty();

    // Iterate over the constant definitions
    const_def_t *current_def = cdfs.const_defs;
    while (current_def != NULL)
    {
        ret = code_seq_concat(ret, gen_code_const_def(*current_def));

        // Move to the next definition
        current_def = current_def->next;
    }

    return ret;
}


// Generate code for the const-def, cdf
code_seq gen_code_const_def(const_def_t cdf)

{

    code_seq ret = code_seq_singleton(code_addi(SP, SP, -BYTES_PER_WORD));
    ret = code_seq_add_to_end(ret, code_sw(0, SP, 0));
    ret = code_seq_concat(ret, gen_code_number(cdf.number));
    ret = code_seq_concat(ret, code_pop_stack_into_reg(V0));
    ret = code_seq_add_to_end(ret, code_sw(GP, V0, 0));
    return ret;
}


// Generate code for the var_decls_t vds to out
// There are 2 instructions generated for each identifier declared
// (one to allocate space and another to initialize that space)
code_seq gen_code_var_decls(var_decls_t vds)
{
    code_seq ret = code_seq_empty();

    var_decl_t *vdp = vds.var_decls;
    while (vdp != NULL)
    {
        ret = code_seq_concat(gen_code_var_decl(*vdp), ret);
        vdp = vdp->next;
    }
    return ret;
}

// Generate code for a single <var-decl>, vd,
// There are 2 instructions generated for each identifier declared
// (one to allocate space and another to initialize that space)
code_seq gen_code_var_decl(var_decl_t vd)
{
    code_seq ret = code_seq_empty();

    ret = code_seq_concat(ret, gen_code_idents(vd.idents));

    return ret;
}

// Generate code for the identififers in idents
// in reverse order (so the first declared are allocated last).
// There are 2 instructions generated for each identifier declared
// (one to allocate space and another to initialize that space)
code_seq gen_code_idents(idents_t idents)
{
    code_seq ret = code_seq_empty();
    ident_t *idp = idents.idents;
    while (idp != NULL)
    {
        ret = code_seq_singleton(code_addi(SP, SP, -BYTES_PER_WORD));
        ret = code_seq_add_to_end(ret, code_sw(0, SP, 0));

        idp = idp->next;
    }
    return ret;
}

// (Stub for:) Generate code for the procedure declarations
code_seq gen_code_proc_decls(proc_decls_t pds)
{
    code_seq ret = code_seq_empty();

    proc_decl_t *pdp = pds.proc_decls;
    while (pdp != NULL)
    {
        ret = code_seq_concat(ret, gen_code_proc_decl(*pdp));
        pdp = pdp->next;
    }
    return ret;
}

// (Stub for:) Generate code for a procedure declaration
code_seq gen_code_proc_decl(proc_decl_t pd)

{
    bail_with_error("no implementation of gen_code_proc_decl");
    return code_seq_empty();
}

// Generate code for stmt
code_seq gen_code_stmt(stmt_t stmt)
{
    switch (stmt.stmt_kind)
    {
    case assign_stmt:
        return gen_code_assign_stmt(stmt.data.assign_stmt);
        break;
    case call_stmt:
        return gen_code_call_stmt(stmt.data.call_stmt);
        break;
    case begin_stmt:
        return gen_code_begin_stmt(stmt.data.begin_stmt);
        break;
    case if_stmt:
        return gen_code_if_stmt(stmt.data.if_stmt);
        break;
    case while_stmt:
        return gen_code_while_stmt(stmt.data.while_stmt);
        break;
    case read_stmt:
        return gen_code_read_stmt(stmt.data.read_stmt);
        break;
    case write_stmt:
        return gen_code_write_stmt(stmt.data.write_stmt);
        break;
    case skip_stmt:
        return gen_code_skip_stmt(stmt.data.skip_stmt);
        break;
    default:
        bail_with_error("Call to gen_code_stmt with an AST that is not a statement!");
        break;
    }
    return code_seq_empty();
}

// Generate code for stmt
code_seq gen_code_assign_stmt(assign_stmt_t stmt)
{
    code_seq ret;
    ret = gen_code_expr(*(stmt.expr));
    assert(stmt.idu != NULL);
    assert(id_use_get_attrs(stmt.idu) != NULL);
    ret = code_seq_concat(ret, code_pop_stack_into_reg(V0));
    ret = code_seq_concat(ret, code_compute_fp(T9, stmt.idu->levelsOutward));
    unsigned int offset_count = id_use_get_attrs(stmt.idu)->offset_count;
    assert(offset_count <= USHRT_MAX);
    ret = code_seq_add_to_end(ret, code_sw(GP, V0, offset_count));
    return ret;
}

// Generate code for stmt
code_seq gen_code_call_stmt(call_stmt_t stmt)
{
    code_seq ret = code_seq_empty();
    ret = code_seq_concat(ret, code_push_reg_on_stack(FP));
    ret = code_seq_concat(ret, code_push_reg_on_stack(GP));
    ret = code_seq_concat(ret, code_push_reg_on_stack(T9));
    ret = code_seq_concat(ret, code_compute_fp(FP, stmt.idu->levelsOutward));
    ret = code_seq_concat(ret, code_restore_registers_from_AR());
    ret = code_seq_concat(ret, code_pop_stack_into_reg(T9));
    ret = code_seq_concat(ret, code_pop_stack_into_reg(GP));
    ret = code_seq_concat(ret, code_pop_stack_into_reg(FP));
    return ret;
}

// Generate code for stmt
code_seq gen_code_begin_stmt(begin_stmt_t stmt)
{

    return gen_code_stmts(stmt.stmts);
}

// Generate code for stmt
code_seq gen_code_stmts(stmts_t stmts)
{
    code_seq ret = code_seq_empty();
    stmt_t *sp = stmts.stmts;
    while (sp != NULL)
    {
        ret = code_seq_concat(ret, gen_code_stmt(*sp));
        sp = sp->next;
    }
    return ret;
}

// Generate code for the if-statment given by stmt
code_seq gen_code_if_stmt(if_stmt_t stmt)
{
    code_seq ret = gen_code_condition(stmt.condition);
    ret = code_seq_concat(ret, code_pop_stack_into_reg(V0));
    ret = code_seq_concat(ret, code_seq_singleton(code_beq(V0, 0, 0)));
    ret = code_seq_concat(ret, gen_code_stmt(*(stmt.then_stmt)));
    ret = code_seq_concat(ret, code_seq_singleton(code_beq(AT, 0, 1)));

    // ret = code_seq_concat(ret, gen_code_stmt(*(stmt.else_stmt)));
    return ret;
}

// Generate code for the if-statment given by stmt
code_seq gen_code_while_stmt(while_stmt_t stmt)
{
    code_seq ret = gen_code_condition(stmt.condition);
    ret = code_seq_concat(ret, code_pop_stack_into_reg(V0));
    ret = code_seq_concat(ret, code_seq_singleton(code_beq(V0, 0, 0)));
    ret = code_seq_concat(ret, gen_code_stmt(*(stmt.body)));
    ret = code_seq_concat(ret, code_seq_singleton(code_beq(AT, 0, 1)));
    return ret;
}

// Generate code for the read statment given by stmt
code_seq gen_code_read_stmt(read_stmt_t stmt)
{
    code_seq ret = code_seq_singleton(code_rch());
    assert(stmt.idu != NULL);
    ret = code_seq_concat(ret, code_compute_fp(T9, stmt.idu->levelsOutward));
    unsigned int offset_count = id_use_get_attrs(stmt.idu)->offset_count;
    assert(offset_count <= USHRT_MAX);
    ret = code_seq_add_to_end(ret, code_sw(GP, A0, offset_count));
    return ret;
}

// Generate code for the write statment given by stmt.
code_seq gen_code_write_stmt(write_stmt_t stmt)
{
    code_seq ret = gen_code_expr(stmt.expr);
    ret = code_seq_concat(ret, code_pop_stack_into_reg(A0));
    ret = code_seq_add_to_end(ret, code_pint());
    return ret;
}

// Generate code for the skip statment 
code_seq gen_code_skip_stmt(skip_stmt_t stmt)
{
    return code_seq_empty();
}

// Requires: reg != T9
// Generate code for cond, putting its truth value
// on top of the runtime stack
// and using V0 and AT as temporary registers
// May modify HI,LO when executed
code_seq gen_code_condition(condition_t cond)
{
    switch (cond.cond_kind)
    {
    case ck_odd:
        return gen_code_odd_condition(cond.data.odd_cond);
        break;
    case ck_rel:
        return gen_code_rel_op_condition(cond.data.rel_op_cond);
        break;
    default:
        bail_with_error("Call to gen_code_condition with an AST that is not a condition!");
    }
    return code_seq_empty();
}


// Generate code for cond, putting its truth value
// on top of the runtime stack
// and using V0 and AT as temporary registers
// Modifies SP, HI,LO when executed
code_seq gen_code_odd_condition(odd_condition_t cond)
{
    code_seq ret = gen_code_expr(cond.expr);
    ret = code_seq_concat(ret, code_pop_stack_into_reg(V0));
    ret = code_seq_concat(ret, code_seq_singleton(code_andi(V0, V0, 1)));
    ret = code_seq_concat(ret, code_seq_singleton(code_bgez(V0, 0)));
    return ret;
}


// Generate code for cond, putting its truth value
// on top of the runtime stack
// and using V0 and AT as temporary registers
// May also modify SP, HI,LO when executed
code_seq gen_code_rel_op_condition(rel_op_condition_t cond)
{
    code_seq ret = gen_code_expr(cond.expr1);
    ret = code_seq_concat(ret, gen_code_expr(cond.expr2));
    ret = code_seq_concat(ret, gen_code_rel_op(cond.rel_op));
    return ret;
}

// Generate code for the rel_op
// applied to 2nd from top and top of the stack,
// putting the result on top of the stack in their place,
// and using V0 and AT as temporary registers
// May also modify SP, HI,LO when executed
code_seq gen_code_rel_op(token_t rel_op)
{

    code_seq ret = code_pop_stack_into_reg(AT);

    ret = code_seq_concat(ret, code_pop_stack_into_reg(V0));
    code_seq do_op = code_seq_empty();

    switch (rel_op.code)
    {
    case eqsym:
        do_op = code_seq_singleton(code_beq(V0, AT, 2));
        break;
    case neqsym:
        do_op = code_seq_singleton(code_bne(V0, AT, 2));
        break;
    case ltsym:
        do_op = code_seq_singleton(code_bltz(V0, 2));
        break;
    case leqsym:
        do_op = code_seq_singleton(code_blez(V0, 2));
        break;
    case gtsym:
        do_op = code_seq_singleton(code_bgtz(V0, 2));
        break;
    case geqsym:
        do_op = code_seq_singleton(code_bgez(V0, 2));
        break;
    case becomessym:
        do_op = code_seq_singleton(code_beq(V0, AT, 2));
        break;

    case plussym:
    case minussym:
    case multsym:
    case divsym:
        return gen_code_arith_op(rel_op);
    default:
        bail_with_error("Unknown token code (%d) in gen_code_rel_op",
                        rel_op.code);
        break;
    }
    ret = code_seq_concat(ret, do_op);
    ret = code_seq_add_to_end(ret, code_add(0, 0, AT));
    ret = code_seq_add_to_end(ret, code_beq(0, 0, 1));
    ret = code_seq_add_to_end(ret, code_addi(0, AT, 1));
    ret = code_seq_add_to_end(ret, code_push_reg_on_stack(AT));
    return ret;
}

// Generate code for the expression exp
// putting the result on top of the stack,
// and using V0 and AT as temporary registers
// May also modify SP, HI,LO when executed
code_seq gen_code_expr(expr_t exp)
{
    switch (exp.expr_kind)
    {
    case expr_bin:
        return gen_code_binary_op_expr(exp.data.binary);
        break;
    case expr_ident:
        return gen_code_ident(exp.data.ident);
        break;
    case expr_number:
        return gen_code_number(exp.data.number);
        break;
    default:
        bail_with_error("Call to gen_code_expr with an AST that is not an expression!");
        break;
    }
    return code_seq_empty();
}

// Generate code for the expression exp
// putting the result on top of the stack,
// and using V0 and AT as temporary registers
// May also modify SP, HI,LO when executed
code_seq gen_code_binary_op_expr(binary_op_expr_t exp)
{

    code_seq ret = gen_code_expr(*(exp.expr1));
    ret = code_seq_concat(ret, gen_code_expr(*(exp.expr2)));
    ret = code_seq_concat(ret, gen_code_arith_op(exp.arith_op));

    return ret;
}

// Generate code to apply arith_op to the
// 2nd from top and top of the stack,
// putting the result on top of the stack in their place,
// and using V0 and AT as temporary registers
// May also modify SP, HI,LO when executed
code_seq gen_code_arith_op(token_t arith_op)
{
    code_seq ret = code_pop_stack_into_reg(V0);
    ret = code_seq_concat(ret, code_pop_stack_into_reg(AT));

    code_seq op = code_seq_empty();

    switch (arith_op.code)
    {
    case plussym:
        op = code_seq_add_to_end(op, code_add(AT, V0, AT));
        break;
    case minussym:
        op = code_seq_add_to_end(op, code_sub(AT, V0, AT));
        break;
    case multsym:
        op = code_seq_add_to_end(op, code_mul(AT, V0));
        break;
    case divsym:
        op = code_seq_add_to_end(op, code_div(AT, V0));

        break;
    default:
        bail_with_error("no implementation of gen_code_arith_op");
        break;
    }

    op = code_seq_concat(op, code_push_reg_on_stack(AT));
    return code_seq_concat(ret, op);
}

// Generate code to put the value of the given identifier
// on top of the stack
// Modifies T9, V0, and SP when executed
code_seq gen_code_ident(ident_t id)
{

    assert(id.idu != NULL);
    code_seq ret = code_compute_fp(T9, id.idu->levelsOutward);
    assert(id_use_get_attrs(id.idu) != NULL);

    unsigned int offset_count = id_use_get_attrs(id.idu)->offset_count;
    assert(offset_count <= USHRT_MAX);

    ret = code_seq_add_to_end(ret, code_lw(GP, V0, offset_count));
    ret = code_seq_concat(ret, code_push_reg_on_stack(V0));
    return ret;
}

// Generate code to put the given number on top of the stack
code_seq gen_code_number(number_t num)
{
    unsigned int offset = literal_table_lookup(num.text, num.value);
    return code_seq_concat(code_seq_singleton(code_lw(GP, V0, offset)), code_push_reg_on_stack(V0));
}
