# PODIO on minimum - this script generates some boilerplate ROOT IO from simply defined structures
from collections import namedtuple
from typing import Callable
from dataclasses import dataclass

@dataclass
class FieldInfo:
    """Class for keeping track of an item in inventory."""
    type: str
    name: str
    comment: str = ""

    def gen_struct_definition(self):
        return f'{self.type} {self.name};'

class FieldVectorInfo(FieldInfo):
    pass

@dataclass
class ClassInfo:
    name: str             # Name. E.g. SrsRecord
    root_name: str        # Part of root file name: e.g. 'srs' => fileds will be srs_id, srs_value, etc.
    fields: list          # List of variable fields



io_classes = [

    ClassInfo(
        name="SrsRawRecord",
        root_name="srs_raw",
        fields=[
            FieldInfo('uint32_t', 'roc'),
            FieldInfo('uint32_t', 'slot'),
            FieldInfo('uint32_t', 'channel'),
            FieldInfo('uint32_t', 'apv_id'),
            FieldInfo('uint32_t', 'channel_apv'),
            FieldInfo('uint16_t', 'best_sample'),
            FieldInfo('std::vector<uint16_t>', 'samples'),
        ]),
    ClassInfo(
        name="SrsDecodedRecord",
        root_name="srs_decoded",
        fields=[
            FieldInfo('uint32_t', 'apv_id'),
            FieldInfo('std::string', 'plane_name'),
            FieldInfo('std::string', 'detector'),
            FieldInfo('std::vector<uint16_t>', 'samples'),
        ]),
    ClassInfo(
        name="SrsPlaneRecord",
        root_name="srs_plane",
        fields=[
            FieldInfo('uint32_t', 'apv_id'),
            FieldInfo('std::string', 'name'),
            FieldInfo('std::string', 'detector'),
            FieldInfo('std::vector<uint16_t>', 'samples'),
        ]),

    ClassInfo(
        name="F125WindowRawRecord",
        root_name="f125_wraw",
        fields=[
            FieldInfo('uint32_t', 'roc'),
            FieldInfo('uint32_t', 'slot'),
            FieldInfo('uint32_t', 'channel'),
            FieldInfo('bool',     'invalid_samples'),
            FieldInfo('bool',     'overflow'),
            FieldInfo('uint32_t', 'itrigger'),
            FieldInfo('std::vector<uint16_t>', 'samples'),
        ]),

    ClassInfo(
        name="F250WindowRawRecord",
        root_name="f250_wraw",
        fields=[
            FieldInfo('uint32_t', 'roc'),
            FieldInfo('uint32_t', 'slot'),
            FieldInfo('uint32_t', 'channel'),
            FieldInfo('bool',     'invalid_samples'),
            FieldInfo('bool',     'overflow'),
            FieldInfo('uint32_t', 'itrigger'),
            FieldInfo('std::vector<uint16_t>', 'samples'),
        ]),

    ClassInfo(
        name="F125FDCPulseRecord",
        root_name="f125_pulse",
        fields=[
            FieldInfo('uint32_t', 'roc'),
            FieldInfo('uint32_t', 'slot'),
            FieldInfo('uint32_t', 'channel'),
            FieldInfo('uint32_t', 'npk',                       'from first word'),
            FieldInfo('uint32_t', 'le_time',                   'from first word'),
            FieldInfo('uint32_t', 'time_quality_bit',          'from first word'),
            FieldInfo('uint32_t', 'overflow_count',            'from first word'),
            FieldInfo('uint32_t', 'pedestal',                  'from second word'),
            FieldInfo('uint32_t', 'integral',                  'from second word (type 6)'),
            FieldInfo('uint32_t', 'peak_amp',                  'from second word (type 9)'),
            FieldInfo('uint32_t', 'peak_time',                 'from second word'),
            FieldInfo('uint32_t', 'word1',                     'first word'),
            FieldInfo('uint32_t', 'word2',                     'second word'),
            FieldInfo('uint32_t', 'nsamples_pedestal',         'number of samples used in integral'),
            FieldInfo('uint32_t', 'nsamples_integral',         'number of samples used in pedestal'),
            FieldInfo('bool',     'emulated',                  'true if emulated values are copied to the main input'),
            FieldInfo('uint32_t', 'le_time_emulated',          'emulated from raw data when available'),
            FieldInfo('uint32_t', 'time_quality_bit_emulated', 'emulated from raw data when available'),
            FieldInfo('uint32_t', 'overflow_count_emulated',   'emulated from raw data when available'),
            FieldInfo('uint32_t', 'pedestal_emulated',         'emulated from raw data when available'),
            FieldInfo('uint32_t', 'integral_emulated',         'emulated from raw data when available'),
            FieldInfo('uint32_t', 'peak_amp_emulated',         'emulated from raw data when available'),
            FieldInfo('uint32_t', 'peak_time_emulated',        'emulated from raw data when available'),
        ]),
    ClassInfo(
        name="F250FDCPulseRecord",
        root_name="f250_pulse",
        fields=[
		   FieldInfo('uint32_t' ,'event_within_block'),
		   FieldInfo('bool'     ,'qf_pedestal'),
		   FieldInfo('uint32_t' ,'pedestal'),
		   FieldInfo('uint32_t' ,'integral'),
		   FieldInfo('bool'     ,'qf_nsa_beyond_ptw'),
		   FieldInfo('bool'     ,'qf_overflow'),
		   FieldInfo('bool'     ,'qf_underflow'),
		   FieldInfo('uint32_t' ,'nsamples_over_threshold'),
		   FieldInfo('uint32_t' ,'course_time'),               # < 4 ns/count
		   FieldInfo('uint32_t' ,'fine_time'),                 # < 0.0625 ns/count
		   FieldInfo('uint32_t' ,'pulse_peak'),
		   FieldInfo('bool'     ,'qf_vpeak_beyond_nsa'),
		   FieldInfo('bool'     ,'qf_vpeak_not_found'),
		   FieldInfo('bool'     ,'qf_bad_pedestal'),
		   FieldInfo('uint32_t' ,'pulse_number'),         # pulse number for this channel, this event starting from 0
		   FieldInfo('uint32_t' ,'nsamples_integral'),    # number of samples used in integral
		   FieldInfo('uint32_t' ,'nsamples_pedestal'),    # number of samples used in pedestal
		   FieldInfo('bool'     ,'emulated'),             # true if made from Window Raw Data
           FieldInfo('uint32_t' ,'integral_emulated'),    # Value calculated from raw data (if available)
           FieldInfo('uint32_t' ,'pedestal_emulated'),    # Value calculated from raw data (if available)
           FieldInfo('uint32_t' ,'time_emulated'),        # Value calculated from raw data (if available)
           FieldInfo('uint32_t' ,'course_time_emulated'), # Value calculated from raw data (if available) - debug
           FieldInfo('uint32_t' ,'fine_time_emulated'),   # Value calculated from raw data (if available) - debug
           FieldInfo('uint32_t' ,'pulse_peak_emulated'),  # Value calculated from raw data (if available)
           FieldInfo('uint32_t' ,'qf_emulated'),
        ]
    ),

    ClassInfo(
        name="GemSimpleCluster",
        root_name="gem_scluster",
        fields=[
            FieldInfo('double', 'x'),
            FieldInfo('double', 'y'),
            FieldInfo('double', 'energy'),
            FieldInfo('double', 'adc'),
        ]),
]

struct_template = """
#pragma once

#include <cstdint>
#include <vector>
#include <TTree.h>
#include "AlignedArraysIO.h"

namespace flatio {
    struct {{name}}
    {
        {{struct_code}}
    };
    
    class {{name}}IO: public AlignedArraysIO
    {
    public:
        void bindToTree(TTree *tree) override {
            m_is_bound = true;
            tree->Branch("{{root_name}}_count", &m_count, "{{root_name}}_count/l");
            {{fields_bind_code}}
        }

        void clear() override {
            m_count = 0;
            {{fields_clear_code}}
        }
        
        void add(const {{name}}& data) {
            if(!m_is_bound) {
                throw std::logic_error("Can't add {{name}} data because {{name}}IO is not bound to tree");
            }
            m_count++;
            {{fields_add_code}}
        }
        
        bool isBoundToTree() const { return m_is_bound; }
        
    private:
        bool m_is_bound = false;
        uint64_t m_count;
        {{fields_declaration_code}}
    };
}
"""

def gen_fields_code(template: str, class_info: ClassInfo, spaces: int):
    code = ""
    for i, field_info in enumerate(class_info.fields):
        assert isinstance(field_info, FieldInfo)
        code += template\
            .replace("{{class_root_name}}", class_info.root_name)\
            .replace("{{field_name}}", field_info.name)\
            .replace("{{field_type}}", field_info.type)\
            .replace("{{field_comment}}", field_info.comment)
        code += '\n' + spaces*' ' if i < len(class_info.fields) - 1 else ''
    return code


def generate_header(class_info: ClassInfo):

    # Structure declaration
    struct_code = gen_fields_code('{{field_type}} {{field_name}};', class_info, 8)

    # Generate code for bindToTree function
    fields_bind_code = gen_fields_code('tree->Branch("{{class_root_name}}_{{field_name}}", &m_vect_{{field_name}});', class_info, 12)

    # Code for clear function
    fields_clear_code = gen_fields_code('m_vect_{{field_name}}.clear();', class_info, 12)

    # Code for adding data from single struct to vectors
    fields_add_code = gen_fields_code('m_vect_{{field_name}}.push_back(data.{{field_name}});', class_info, 12)

    # Fields declaration
    fields_declaration_code = gen_fields_code('std::vector<{{field_type}}> m_vect_{{field_name}};', class_info, 8)

    # Process the main template
    result = struct_template\
        .replace('{{name}}', class_info.name) \
        .replace('{{root_name}}', class_info.root_name) \
        .replace('{{struct_code}}', struct_code) \
        .replace('{{fields_bind_code}}', fields_bind_code) \
        .replace('{{fields_clear_code}}', fields_clear_code) \
        .replace('{{fields_declaration_code}}', fields_declaration_code)\
        .replace('{{fields_add_code}}', fields_add_code)

    return result


def generate():
    for class_info in io_classes:
        code = generate_header(class_info)
        print("="*30)
        print(code)
        # with open(f"{class_info.name}.h", "w") as text_file:
        #     text_file.write(code)


if __name__ == "__main__":
    generate()