#ifndef _TQL_PARSER_H_
#define _TQL_PARSER_H_

#include <string>
#include <vector>
#include <string.h>
#include <inttypes.h>

namespace tql
{
    const int MAX_SIMPLE_STR_BUF_SIZE = 128;
    struct simple_str_buf_t
    {
        char buf_[MAX_SIMPLE_STR_BUF_SIZE];
    };

    class token_t
    {
    public:
        int tid_;
        std::string str_;
    };

    class variant_t
    {
    public:
        enum e_variant_type
        {
            // 按照类型提升排序，向取值小的提升
            evt_string = 1,
            evt_double = 2,
            evt_float = 3,
            evt_int64 = 4,
            evt_uint64 = 5,
            evt_int32 = 6,
            evt_uint32 = 7,
            evt_field_desc = 8,
        };

        variant_t() { type_ = -1; }
        variant_t(const variant_t& var) { *this = var; }
        variant_t& operator = (const variant_t& var)
        {
            type_ = var.type_;
            str_ = var.str_;
            memcpy(&numeral_, &var.numeral_, sizeof(numeral_));
            return *this;
        }

        const std::string& to_string();
        int cast_type(int type);

        int type_;
        union
        {
            float float_val_;
            double double_val_;
            int int32_val_;
            unsigned int uint32_val_;
            int64_t int64_val_;
            uint64_t uint64_val_;
        } numeral_;

        std::string str_;
    };

    class expr2_t
    {
    public:
        // type
        enum e_expr2_type
        {
            eet_var = 0,
            eet_logic = 1,
            eet_math = 2,
        };

        // functions
        expr2_t()
        {
            type_ = -1;
            op_ = -1;
            left_ = -1;
            right_ = -1;
        }

        expr2_t(const variant_t& var)
        {
            type_ = eet_var;
            var_ = var;
            op_ = -1;
            left_ = -1;
            right_ = -1;
        }

        short type_;
        short op_;
        variant_t var_;
        int left_;  // idx in array
        int right_; // idx in array
    };

    class assign_t
    {
    public:
        int op_;
        std::string left_;  // field
        int right_; // math expr index
    };

    class parser_context_t
    {
    public:
        // types
        enum e_stmt_type
        {
            est_select = 1,
            est_insert = 2,
            est_update = 3,
            est_delete = 4,
        };

        enum e_parser_errno
        {
            epe_no_error = 0,
            epe_syntax_error = 1,
            epe_stackoverflow = 2,
            epe_invalid_variant = 3,
        };

        // functions
        parser_context_t();
        void append_field(const std::string& fld);
        const std::string* get_field(int idx) const;
        int append_condition(const expr2_t& condition);
        const expr2_t* get_condition(int idx) const;
        const expr2_t* get_last_condition() const;
        int append_assignment(const assign_t& assign);
        const assign_t* get_assignment(int idx) const;
        int append_math(const expr2_t& assign);
        const expr2_t* get_math(int idx) const;
        int append_variant(const variant_t& var);
        const variant_t* get_variant(int idx) const;
        void clear();

        void set_table(const std::string& tbl) { table_ = tbl; }
        const std::string& get_table() const { return table_; }
        void set_key(const std::string& key) { key_ = key; }
        const std::string& get_key() const { return key_; }

        void set_stmt_type(int type) { stmt_type_ = type; }
        int get_stmt_type() const { return stmt_type_; }

        void set_errno(int err) { errno_ = err; }
        int get_errno() const { return errno_; }
        void clear_errno() { errno_ = epe_no_error; }
        const std::vector<std::string>& get_fields() const { return fields_; }

    private:
        parser_context_t(const parser_context_t&) { /*forbidden*/ }
        // data member
        int errno_;
        int stmt_type_;
        std::string key_;   // where
        std::string table_;
        std::vector<expr2_t> conditions_;    // where, logic expr
        std::vector<std::string> fields_;    // select
        std::vector<assign_t> assigns_;   // update, insert
        std::vector<variant_t> var_pool_;   // pool, for scanner
        std::vector<expr2_t> math_; // 数学表达式, math expr
    };

    // utility function for scanner
    int var_to_expr2(parser_context_t &ctx, int var);
}

#endif
