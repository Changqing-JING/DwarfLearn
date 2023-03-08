#include "DebugAbbrev.hpp"
#include <cassert>
#include <iostream>
#include <stdexcept>
#include "ByteReader.hpp"

const std::unordered_map<uint64_t, DebugAbbrev::AbbrevEntry> DebugAbbrev::parseDebugAbbrev(std::vector<uint8_t> const &elfFile, Elf32_Shdr const *const debugAbbrevSection) {
  uint8_t const *const debugAbbrevData = elfFile.data() + debugAbbrevSection->sh_offset;
  ByteReader debugAbbrevReader(debugAbbrevData, debugAbbrevSection->sh_size);

  std::unordered_map<uint64_t, AbbrevEntry> abbrevTable;

  while (true) {
    uint64_t const index = debugAbbrevReader.readLEB128(false);

    if (index != 0) {
      assert(abbrevTable.count(index) == 0 && "duplicate Abbrev index");

      AbbrevEntry &abbrevEntry = abbrevTable[index];

      uint64_t const entryTag = debugAbbrevReader.readLEB128(false);
      abbrevEntry.tag = static_cast<Tag>(entryTag);

      uint8_t const hasChild = debugAbbrevReader.getNumber<uint8_t>();

      if (hasChild == DW_CHILDREN_yes) {
        abbrevEntry.hasChildren = true;
      } else if (hasChild == DW_CHILDREN_no) {
        abbrevEntry.hasChildren = false;
      } else {
        throw std::runtime_error("unknown hasChild");
      }

      while (true) {
        uint64_t const attributeName = debugAbbrevReader.readLEB128(false);
        uint64_t const form = debugAbbrevReader.readLEB128(false);

        if (!((attributeName == 0) && (form == 0))) {
          abbrevEntry.attributeSpecifications.emplace_back(AttributeSpecification{static_cast<AttributeName>(attributeName), static_cast<Form>(form)});
        } else {
          break;
        }
      }
    } else {
      assert(debugAbbrevReader.reachedEnd());
      break;
    }
  }
  return abbrevTable;
}

const std::string DebugAbbrev::attributeNameToString(AttributeName const attributeName) {
  switch (attributeName) {
  case (AttributeName::DW_AT_sibling): {
    return "DW_AT_sibling";
  }
  case (AttributeName::DW_AT_location): {
    return "DW_AT_location";
  }
  case (AttributeName::DW_AT_name): {
    return "DW_AT_name";
  }
  case (AttributeName::DW_AT_ordering): {
    return "DW_AT_ordering";
  }
  case (AttributeName::DW_AT_byte_size): {
    return "DW_AT_byte_size";
  }
  case (AttributeName::DW_AT_bit_offset): {
    return "DW_AT_bit_offset";
  }
  case (AttributeName::DW_AT_bit_size): {
    return "DW_AT_bit_size";
  }
  case (AttributeName::DW_AT_stmt_list): {
    return "DW_AT_stmt_list";
  }
  case (AttributeName::DW_AT_low_pc): {
    return "DW_AT_low_pc";
  }
  case (AttributeName::DW_AT_high_pc): {
    return "DW_AT_high_pc";
  }
  case (AttributeName::DW_AT_language): {
    return "DW_AT_language";
  }
  case (AttributeName::DW_AT_discr): {
    return "DW_AT_discr";
  }
  case (AttributeName::DW_AT_discr_value): {
    return "DW_AT_discr_value";
  }
  case (AttributeName::DW_AT_visibility): {
    return "DW_AT_visibility";
  }
  case (AttributeName::DW_AT_import): {
    return "DW_AT_import";
  }
  case (AttributeName::DW_AT_string_length): {
    return "DW_AT_string_length";
  }
  case (AttributeName::DW_AT_common_reference): {
    return "DW_AT_common_reference";
  }
  case (AttributeName::DW_AT_comp_dir): {
    return "DW_AT_comp_dir";
  }
  case (AttributeName::DW_AT_const_value): {
    return "DW_AT_const_value";
  }
  case (AttributeName::DW_AT_containing_type): {
    return "DW_AT_containing_type";
  }
  case (AttributeName::DW_AT_default_value): {
    return "DW_AT_default_value";
  }
  case (AttributeName::DW_AT_inline): {
    return "DW_AT_inline";
  }
  case (AttributeName::DW_AT_is_optional): {
    return "DW_AT_is_optional";
  }
  case (AttributeName::DW_AT_lower_bound): {
    return "DW_AT_lower_bound";
  }
  case (AttributeName::DW_AT_producer): {
    return "DW_AT_producer";
  }
  case (AttributeName::DW_AT_prototyped): {
    return "DW_AT_prototyped";
  }
  case (AttributeName::DW_AT_return_addr): {
    return "DW_AT_return_addr";
  }
  case (AttributeName::DW_AT_start_scope): {
    return "DW_AT_start_scope";
  }
  case (AttributeName::DW_AT_bit_stride): {
    return "DW_AT_bit_stride";
  }
  case (AttributeName::DW_AT_upper_bound): {
    return "DW_AT_upper_bound";
  }
  case (AttributeName::DW_AT_abstract_origin): {
    return "DW_AT_abstract_origin";
  }
  case (AttributeName::DW_AT_accessibility): {
    return "DW_AT_accessibility";
  }
  case (AttributeName::DW_AT_address_class): {
    return "DW_AT_address_class";
  }
  case (AttributeName::DW_AT_artificial): {
    return "DW_AT_artificial";
  }
  case (AttributeName::DW_AT_base_types): {
    return "DW_AT_base_types";
  }
  case (AttributeName::DW_AT_calling_convention): {
    return "DW_AT_calling_convention";
  }
  case (AttributeName::DW_AT_count): {
    return "DW_AT_count";
  }
  case (AttributeName::DW_AT_data_member_location): {
    return "DW_AT_data_member_location";
  }
  case (AttributeName::DW_AT_decl_column): {
    return "DW_AT_decl_column";
  }
  case (AttributeName::DW_AT_decl_file): {
    return "DW_AT_decl_file";
  }
  case (AttributeName::DW_AT_decl_line): {
    return "DW_AT_decl_line";
  }
  case (AttributeName::DW_AT_declaration): {
    return "DW_AT_declaration";
  }
  case (AttributeName::DW_AT_discr_list): {
    return "DW_AT_discr_list";
  }
  case (AttributeName::DW_AT_encoding): {
    return "DW_AT_encoding";
  }
  case (AttributeName::DW_AT_external): {
    return "DW_AT_external";
  }
  case (AttributeName::DW_AT_frame_base): {
    return "DW_AT_frame_base";
  }
  case (AttributeName::DW_AT_friend): {
    return "DW_AT_friend";
  }
  case (AttributeName::DW_AT_identifier_case): {
    return "DW_AT_identifier_case";
  }
  case (AttributeName::DW_AT_macro_info): {
    return "DW_AT_macro_info";
  }
  case (AttributeName::DW_AT_namelist_item): {
    return "DW_AT_namelist_item";
  }
  case (AttributeName::DW_AT_priority): {
    return "DW_AT_priority";
  }
  case (AttributeName::DW_AT_segment): {
    return "DW_AT_segment";
  }
  case (AttributeName::DW_AT_specification): {
    return "DW_AT_specification";
  }
  case (AttributeName::DW_AT_static_link): {
    return "DW_AT_static_link";
  }
  case (AttributeName::DW_AT_type): {
    return "DW_AT_type";
  }
  case (AttributeName::DW_AT_use_location): {
    return "DW_AT_use_location";
  }
  case (AttributeName::DW_AT_variable_parameter): {
    return "DW_AT_variable_parameter";
  }
  case (AttributeName::DW_AT_virtuality): {
    return "DW_AT_virtuality";
  }
  case (AttributeName::DW_AT_vtable_elem_location): {
    return "DW_AT_vtable_elem_location";
  }
  case (AttributeName::DW_AT_allocated): {
    return "DW_AT_allocated ";
  }
  case (AttributeName::DW_AT_associated): {
    return "DW_AT_associated ";
  }
  case (AttributeName::DW_AT_data_location): {
    return "DW_AT_data_location ";
  }
  case (AttributeName::DW_AT_byte_stride): {
    return "DW_AT_byte_stride ";
  }
  case (AttributeName::DW_AT_entry_pc): {
    return "DW_AT_entry_pc ";
  }
  case (AttributeName::DW_AT_use_UTF8): {
    return "DW_AT_use_UTF8 ";
  }
  case (AttributeName::DW_AT_extension): {
    return "DW_AT_extension ";
  }
  case (AttributeName::DW_AT_ranges): {
    return "DW_AT_ranges ";
  }
  case (AttributeName::DW_AT_recursive): {
    return "DW_AT_recursive ";
  }
  case (AttributeName::DW_AT_lo_user): {
    return "DW_AT_lo_user";
  }
  case (AttributeName::DW_AT_MIPS_linkage_name): {
    return "DW_AT_MIPS_linkage_name";
  }
  case (AttributeName::DW_AT_GNU_all_call_sites): {
    return "DW_AT_GNU_all_call_sites";
  }
  case (AttributeName::DW_AT_hi_user): {
    return "DW_AT_hi_user";
  }

  default: {
    throw std::runtime_error("unknown attribute name");
  }
  }
}

const std::string DebugAbbrev::tagToString(Tag const tag) {
  switch (tag) {
  case (Tag::DW_TAG_array_type): {
    return "DW_TAG_array_type";
  }
  case (Tag::DW_TAG_class_type): {
    return "DW_TAG_class_type";
  }
  case (Tag::DW_TAG_entry_point): {
    return "DW_TAG_entry_point";
  }
  case (Tag::DW_TAG_enumeration_type): {
    return "DW_TAG_enumeration_type";
  }
  case (Tag::DW_TAG_formal_parameter): {
    return "DW_TAG_formal_parameter";
  }
  case (Tag::DW_TAG_imported_declaration): {
    return "DW_TAG_imported_declaration";
  }
  case (Tag::DW_TAG_label): {
    return "DW_TAG_label";
  }
  case (Tag::DW_TAG_lexical_block): {
    return "DW_TAG_lexical_block";
  }
  case (Tag::DW_TAG_member): {
    return "DW_TAG_member";
  }
  case (Tag::DW_TAG_pointer_type): {
    return "DW_TAG_pointer_type";
  }
  case (Tag::DW_TAG_reference_type): {
    return "DW_TAG_reference_type";
  }
  case (Tag::DW_TAG_compile_unit): {
    return "DW_TAG_compile_unit";
  }
  case (Tag::DW_TAG_string_type): {
    return "DW_TAG_string_type";
  }
  case (Tag::DW_TAG_structure_type): {
    return "DW_TAG_structure_type";
  }
  case (Tag::DW_TAG_subroutine_type): {
    return "DW_TAG_subroutine_type";
  }
  case (Tag::DW_TAG_typedef): {
    return "DW_TAG_typedef";
  }
  case (Tag::DW_TAG_union_type): {
    return "DW_TAG_union_type";
  }
  case (Tag::DW_TAG_unspecified_parameters): {
    return "DW_TAG_unspecified_parameters";
  }
  case (Tag::DW_TAG_variant): {
    return "DW_TAG_variant";
  }
  case (Tag::DW_TAG_common_block): {
    return "DW_TAG_common_block";
  }
  case (Tag::DW_TAG_common_inclusion): {
    return "DW_TAG_common_inclusion";
  }
  case (Tag::DW_TAG_inheritance): {
    return "DW_TAG_inheritance";
  }
  case (Tag::DW_TAG_inlined_subroutine): {
    return "DW_TAG_inlined_subroutine";
  }
  case (Tag::DW_TAG_module): {
    return "DW_TAG_module";
  }
  case (Tag::DW_TAG_ptr_to_member_type): {
    return "DW_TAG_ptr_to_member_type";
  }
  case (Tag::DW_TAG_set_type): {
    return "DW_TAG_set_type";
  }
  case (Tag::DW_TAG_subrange_type): {
    return "DW_TAG_subrange_type";
  }
  case (Tag::DW_TAG_with_stmt): {
    return "DW_TAG_with_stmt";
  }
  case (Tag::DW_TAG_access_declaration): {
    return "DW_TAG_access_declaration";
  }
  case (Tag::DW_TAG_base_type): {
    return "DW_TAG_base_type";
  }
  case (Tag::DW_TAG_catch_block): {
    return "DW_TAG_catch_block";
  }
  case (Tag::DW_TAG_const_type): {
    return "DW_TAG_const_type";
  }
  case (Tag::DW_TAG_constant): {
    return "DW_TAG_constant";
  }
  case (Tag::DW_TAG_enumerator): {
    return "DW_TAG_enumerator";
  }
  case (Tag::DW_TAG_file_type): {
    return "DW_TAG_file_type";
  }
  case (Tag::DW_TAG_friend): {
    return "DW_TAG_friend";
  }
  case (Tag::DW_TAG_namelist): {
    return "DW_TAG_namelist";
  }
  case (Tag::DW_TAG_namelist_item): {
    return "DW_TAG_namelist_item";
  }
  case (Tag::DW_TAG_packed_type): {
    return "DW_TAG_packed_type";
  }
  case (Tag::DW_TAG_subprogram): {
    return "DW_TAG_subprogram";
  }
  case (Tag::DW_TAG_template_type_parameter): {
    return "DW_TAG_template_type_parameter";
  }
  case (Tag::DW_TAG_template_value_parameter): {
    return "DW_TAG_template_value_parameter";
  }
  case (Tag::DW_TAG_thrown_type): {
    return "DW_TAG_thrown_type";
  }
  case (Tag::DW_TAG_try_block): {
    return "DW_TAG_try_block";
  }
  case (Tag::DW_TAG_variant_part): {
    return "DW_TAG_variant_part";
  }
  case (Tag::DW_TAG_variable): {
    return "DW_TAG_variable";
  }
  case (Tag::DW_TAG_volatile_type): {
    return "DW_TAG_volatile_type";
  }
  case (Tag::DW_TAG_dwarf_procedure): {
    return "DW_TAG_dwarf_procedure";
  }
  case (Tag::DW_TAG_restrict_type): {
    return "DW_TAG_restrict_type";
  }
  case (Tag::DW_TAG_interface_type): {
    return "DW_TAG_interface_type";
  }
  case (Tag::DW_TAG_namespace): {
    return "DW_TAG_namespace";
  }
  case (Tag::DW_TAG_imported_module): {
    return "DW_TAG_imported_module";
  }
  case (Tag::DW_TAG_unspecified_type): {
    return "DW_TAG_unspecified_type";
  }
  case (Tag::DW_TAG_partial_unit): {
    return "DW_TAG_partial_unit";
  }
  case (Tag::DW_TAG_imported_unit): {
    return "DW_TAG_imported_unit";
  }
  case (Tag::DW_TAG_condition): {
    return "DW_TAG_condition";
  }
  case (Tag::DW_TAG_shared_type): {
    return "DW_TAG_shared_type";
  }
  case (Tag::DW_TAG_lo_user): {
    return "DW_TAG_lo_user";
  }
  case (Tag::DW_TAG_hi_user): {
    return "DW_TAG_hi_user";
  }
  default: {
    throw std::runtime_error("unknown tag");
  }
  }
}