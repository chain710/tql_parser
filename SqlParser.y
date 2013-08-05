%include {
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "tql_parser.h"
#define NOUSED(x) (x)
}

/*Parse第三个参数类型*/
%token_type { const tql::token_t * }
/*Parse第四个参数*/
%extra_argument { tql::parser_context_t *ctx }
%token_prefix TK_

%syntax_error {
    ctx->set_errno(tql::parser_context_t::epe_syntax_error);
}

%stack_overflow {
    ctx->set_errno(tql::parser_context_t::epe_stackoverflow);
}

%type logic_expr_m {int}
%type logic_expr {int}
%type math_expr_m {int}
%type math_expr {int}
%type inst_val {int}
%type field {int}
%type assignments_m {int}
%type table {tql::simple_str_buf_t}

%left AND.
%left OR.
%left EQ UNEQ GT GE LT LE COMMA.
%left PLUS MINUS.
%left BIN_AND BIN_OR.
%left MULTI DIV.

stmt ::= select. {
}

stmt ::= update. {
}

stmt ::= insert. {
}

stmt ::= delete. {
}

stmt ::= replace. {
}

select ::= SELECT cols from where. {
    ctx->set_stmt_type(tql::parser_context_t::est_select);
}

update ::= UPDATE table(T) sets where. {
    ctx->set_stmt_type(tql::parser_context_t::est_update);
    ctx->set_table(T.buf_);
}

replace ::= REPLACE table(T) sets where_key. {
    ctx->set_stmt_type(tql::parser_context_t::est_replace);
    ctx->set_table(T.buf_);
}

delete ::= DELETE from where. {
    ctx->set_stmt_type(tql::parser_context_t::est_delete);
}

insert ::= INSERT table(T) sets where_key. {
    ctx->set_stmt_type(tql::parser_context_t::est_insert);
    ctx->set_table(T.buf_);
}

cols ::= cols COMMA field(B). {
    const tql::variant_t* tmp = ctx->get_variant(B);
    if (NULL == tmp)
    {
        ctx->set_errno(tql::parser_context_t::epe_invalid_variant);
    }
    else
    {
        ctx->append_field(tmp->str_);
    }
}

cols ::= field(B). {
    const tql::variant_t* tmp = ctx->get_variant(B);
    if (NULL == tmp)
    {
        ctx->set_errno(tql::parser_context_t::epe_invalid_variant);
    }
    else
    {
        ctx->append_field(tmp->str_);
    }
}

cols ::= MULTI. {
    // simply do nothing
}

from ::= FROM table(B). {
    ctx->set_table(B.buf_);
}

where ::= where_key. {
}

where ::= where_key AND logic_expr. {
}

where_key ::= WHERE key. {
}

sets ::= SET assignments. {
}

assignments ::= assignments COMMA assignments_m. {
}

assignments ::= assignments_m. {
}

assignments_m(A) ::= field(B) EQ(OP) math_expr(C). {
    const tql::variant_t* fld = ctx->get_variant(B);
    
    if (NULL == fld)
    {
        ctx->set_errno(tql::parser_context_t::epe_invalid_variant);
    }
    else
    {
        tql::assign_t tmp;
        tmp.op_ = OP->tid_;
        tmp.left_ = fld->str_;
        tmp.right_ = C;
        A = ctx->append_assignment(tmp);
    }
}

logic_expr(A) ::= logic_expr_m(B). {
    A = B;
}

logic_expr(A) ::= LB logic_expr(B) RB. {
    A = B;
}

logic_expr(A) ::= logic_expr(B) AND|OR(OP) logic_expr(C). {
    tql::expr2_t tmp;
    tmp.type_ = tql::expr2_t::eet_logic;
    tmp.op_ = OP->tid_;
    tmp.left_ = B;
    tmp.right_ = C;
    A = ctx->append_condition(tmp);
}

logic_expr_m(A) ::= math_expr(B) EQ|UNEQ|GT|GE|LT|LE(OP) math_expr(C). {
    tql::expr2_t tmp;
    tmp.type_ = tql::expr2_t::eet_logic;
    tmp.op_ = OP->tid_;
    tmp.left_ = B;
    tmp.right_ = C;
    A = ctx->append_condition(tmp);
}

math_expr(A) ::= math_expr(B) PLUS|MINUS|MULTI|DIV|BIN_AND|BIN_OR(OP) math_expr(C). {
    // append math
    tql::expr2_t tmp;
    tmp.type_ = tql::expr2_t::eet_math;
    tmp.op_ = OP->tid_;
    tmp.left_ = B;
    tmp.right_ = C;
    A = ctx->append_math(tmp);
}

math_expr(A) ::= math_expr_m(B). {
    A = B;
}

math_expr(A) ::= LB math_expr(B) RB. {
    A = B;
}

math_expr_m(A) ::= field(B). {
    A = var_to_expr2(*ctx, B);
}

math_expr_m(A) ::= inst_val(B). {
    A = var_to_expr2(*ctx, B);
}

key ::= KEY LB key_m RB. {
}

key_m ::= ID|INTEGER(A). {
    ctx->append_key(A->str_);
}

key_m ::= key_m COMMA ID|INTEGER(A). {
    ctx->append_key(A->str_);
}

table(A) ::= ID(B). {
    snprintf(A.buf_, sizeof(A.buf_), "%s", B->str_.c_str());
}

inst_val(A) ::= MINUS INTEGER(B). {
    tql::variant_t tmp;
    tmp.type_ = tql::variant_t::evt_int64;
    tmp.numeral_.int64_val_ = -strtoll(B->str_.c_str(), NULL, 10);
    A = ctx->append_variant(tmp);
}

inst_val(A) ::= INTEGER(B). {
    tql::variant_t tmp;
    cast_int(tmp, strtoll(B->str_.c_str(), NULL, 10));
    A = ctx->append_variant(tmp);
}

inst_val(A) ::= QUOTE STRING(B) QUOTE. {
    tql::variant_t tmp;
    tmp.type_ = tql::variant_t::evt_string;
    tmp.str_ = B->str_;
    A = ctx->append_variant(tmp);
}

inst_val(A) ::= QUOTE QUOTE. {
    tql::variant_t tmp;
    tmp.type_ = tql::variant_t::evt_string;
    tmp.str_.clear();
    A = ctx->append_variant(tmp);
}

field(A) ::= ID(B). {
    tql::variant_t tmp;
    tmp.type_ = tql::variant_t::evt_field_desc;
    tmp.str_ = B->str_;
    A = ctx->append_variant(tmp);
}

