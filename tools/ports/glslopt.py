# Copyright 2015 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import os, shutil, logging, subprocess, sys, stat

TAG = 'version_1'

def get(ports, settings, shared):
    if settings.USE_REGAL_GLSLOPT == 1:
        ports.fetch_project('glslopt', 'https://github.com/gabrielcuvillier/regal/archive/' + TAG + '.zip', 'Regal-' + TAG)
        def create():
            logging.info('building port: glslopt')
            ports.clear_project_build('glslopt')

            source_path = os.path.join(ports.get_dir(), 'regal', 'Regal-' + TAG)
            dest_path = os.path.join(shared.Cache.get_path('ports-builds'), 'glslopt')

            shutil.rmtree(dest_path, ignore_errors=True)
            shutil.copytree(source_path, dest_path)

            # build
            srcs = [
                'src/glsl/src/glsl/ast_array_index.cpp',
                'src/glsl/src/glsl/ast_expr.cpp',
                'src/glsl/src/glsl/ast_function.cpp',
                'src/glsl/src/glsl/ast_to_hir.cpp',
                'src/glsl/src/glsl/ast_type.cpp',
                'src/glsl/src/glsl/builtin_function.cpp',
                'src/glsl/src/glsl/builtin_variables.cpp',
                'src/glsl/src/glsl/glsl_lexer.cpp',
                'src/glsl/src/glsl/glsl_optimizer.cpp',
                'src/glsl/src/glsl/glsl_parser.cpp',
                'src/glsl/src/glsl/glsl_parser_extras.cpp',
                'src/glsl/src/glsl/glsl_symbol_table.cpp',
                'src/glsl/src/glsl/glsl_types.cpp',
                'src/glsl/src/glsl/hir_field_selection.cpp',
                'src/glsl/src/glsl/ir.cpp',
                'src/glsl/src/glsl/ir_basic_block.cpp',
                'src/glsl/src/glsl/ir_builder.cpp',
                'src/glsl/src/glsl/ir_clone.cpp',
                'src/glsl/src/glsl/ir_constant_expression.cpp',
                'src/glsl/src/glsl/ir_expression_flattening.cpp',
                'src/glsl/src/glsl/ir_function.cpp',
                'src/glsl/src/glsl/ir_function_can_inline.cpp',
                'src/glsl/src/glsl/ir_function_detect_recursion.cpp',
                'src/glsl/src/glsl/ir_hierarchical_visitor.cpp',
                'src/glsl/src/glsl/ir_hv_accept.cpp',
                'src/glsl/src/glsl/ir_import_prototypes.cpp',
                'src/glsl/src/glsl/ir_print_glsl_visitor.cpp',
                'src/glsl/src/glsl/ir_print_visitor.cpp',
                'src/glsl/src/glsl/ir_reader.cpp',
                'src/glsl/src/glsl/ir_rvalue_visitor.cpp',
                'src/glsl/src/glsl/ir_unused_structs.cpp',
                'src/glsl/src/glsl/ir_validate.cpp',
                'src/glsl/src/glsl/ir_variable_refcount.cpp',
                'src/glsl/src/glsl/link_functions.cpp',
                'src/glsl/src/glsl/link_uniform_block_active_visitor.cpp',
                'src/glsl/src/glsl/link_uniform_blocks.cpp',
                'src/glsl/src/glsl/link_uniform_initializers.cpp',
                'src/glsl/src/glsl/link_uniforms.cpp',
                'src/glsl/src/glsl/link_varyings.cpp',
                'src/glsl/src/glsl/linker.cpp',
                'src/glsl/src/glsl/loop_analysis.cpp',
                'src/glsl/src/glsl/loop_controls.cpp',
                'src/glsl/src/glsl/loop_unroll.cpp',
                'src/glsl/src/glsl/lower_clip_distance.cpp',
                'src/glsl/src/glsl/lower_discard.cpp',
                'src/glsl/src/glsl/lower_discard_flow.cpp',
                'src/glsl/src/glsl/lower_if_to_cond_assign.cpp',
                'src/glsl/src/glsl/lower_instructions.cpp',
                'src/glsl/src/glsl/lower_jumps.cpp',
                'src/glsl/src/glsl/lower_mat_op_to_vec.cpp',
                'src/glsl/src/glsl/lower_noise.cpp',
                'src/glsl/src/glsl/lower_packed_varyings.cpp',
                'src/glsl/src/glsl/lower_variable_index_to_cond_assign.cpp',
                'src/glsl/src/glsl/lower_vec_index_to_cond_assign.cpp',
                'src/glsl/src/glsl/lower_vec_index_to_swizzle.cpp',
                'src/glsl/src/glsl/lower_vector.cpp',
                'src/glsl/src/glsl/opt_algebraic.cpp',
                'src/glsl/src/glsl/opt_array_splitting.cpp',
                'src/glsl/src/glsl/opt_constant_folding.cpp',
                'src/glsl/src/glsl/opt_constant_propagation.cpp',
                'src/glsl/src/glsl/opt_constant_variable.cpp',
                'src/glsl/src/glsl/opt_copy_propagation.cpp',
                'src/glsl/src/glsl/opt_copy_propagation_elements.cpp',
                'src/glsl/src/glsl/opt_dead_code.cpp',
                'src/glsl/src/glsl/opt_dead_code_local.cpp',
                'src/glsl/src/glsl/opt_dead_functions.cpp',
                'src/glsl/src/glsl/opt_flatten_nested_if_blocks.cpp',
                'src/glsl/src/glsl/opt_function_inlining.cpp',
                'src/glsl/src/glsl/opt_if_simplification.cpp',
                'src/glsl/src/glsl/opt_noop_swizzle.cpp',
                'src/glsl/src/glsl/opt_redundant_jumps.cpp',
                'src/glsl/src/glsl/opt_structure_splitting.cpp',
                'src/glsl/src/glsl/opt_swizzle_swizzle.cpp',
                'src/glsl/src/glsl/opt_tree_grafting.cpp',
                'src/glsl/src/glsl/ralloc.c',
                'src/glsl/src/glsl/s_expression.cpp',
                'src/glsl/src/glsl/standalone_scaffolding.cpp',
                'src/glsl/src/glsl/strtod.c',
                'src/glsl/src/glsl/glcpp/glcpp-lex.c',
                'src/glsl/src/glsl/glcpp/glcpp-parse.c',
                'src/glsl/src/glsl/glcpp/pp.c',
                'src/glsl/src/glsl/../mesa/main/imports.c',
                'src/glsl/src/glsl/../mesa/main/hash_table.c',
                'src/glsl/src/glsl/../mesa/program/symbol_table.c',
                'src/glsl/src/glsl/../mesa/program/prog_hash_table.c']
            commands = []
            o_s = []
            for src in srcs:
                o = os.path.join(ports.get_build_dir(), 'glslopt', src + '.o')
                shared.safe_ensure_dirs(os.path.dirname(o))
                commands.append([shared.PYTHON, shared.EMCC, os.path.join(dest_path, src),
                                 '-DNDEBUG',
                                 '-DREGAL_NO_ASSERT=1',
                                 '-fvisibility=hidden',
                                 '-Wno-sign-compare',
                                 '-O2',
                                 '-o', o,
                                 '-I' + dest_path + '/include',
                                 '-I' + dest_path + '/src/glsl/include',
                                 '-I' + dest_path + '/src/glsl/src/glsl',
                                 '-I' + dest_path + '/src/glsl/src/mesa',
                                 '-w',])
                o_s.append(o)

            ports.run_commands(commands)
            final = os.path.join(ports.get_build_dir(), 'glslopt', 'libglslopt.a')
            shared.try_delete(final)
            Popen([shared.LLVM_AR, 'rc', final] + o_s).communicate()
            assert os.path.exists(final)
            return final
        return [shared.Cache.get('glslopt', create, what='port')]
    else:
        return []

def process_args(ports, args, settings, shared):
    if settings.USE_REGAL_GLSLOPT == 1:
        get(ports, settings, shared)
        args += ['-Xclang', '-isystem' + os.path.join(shared.Cache.get_path('ports-builds'), 'glslopt', 'include')]
    return args

def show():
    return 'regal/glslopt (USE_REGAL_GLSLOPT=1; Regal license)'
