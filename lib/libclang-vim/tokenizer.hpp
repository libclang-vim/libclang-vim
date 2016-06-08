#if !defined LIBCLANG_VIM_TOKENIZER_HPP_INCLUDED
#define      LIBCLANG_VIM_TOKENIZER_HPP_INCLUDED

#include <string>
#include <vector>

#include <clang-c/Index.h>

#include "helpers.hpp"

namespace libclang_vim
{

class tokenizer
{
    std::string file_name;

    CXSourceRange get_range_whole_file(const cxtranslation_unit_ptr& translation_unit) const;
    const char* get_kind_spelling(const CXTokenKind kind) const;
    std::string make_vimson_from_tokens(const cxtranslation_unit_ptr& translation_unit, std::vector<CXToken> tokens) const;

public:
    tokenizer(std::string file_name);
    std::string tokenize_as_vimson(char const* const* args, size_t const argc);
};

} // namespace libclang_vim

#endif    // LIBCLANG_VIM_TOKENIZER_HPP_INCLUDED

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
