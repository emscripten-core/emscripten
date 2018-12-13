# Copyright 2015 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import os, shutil, logging, subprocess, sys, stat

TAG = 'version_1'

def get(ports, settings, shared):
    if settings.USE_REGAL == 1:
        ports.fetch_project('regal', 'https://github.com/gabrielcuvillier/regal/archive/' + TAG + '.zip', 'Regal-' + TAG)
        def create():
            logging.info('building port: regal')
            ports.clear_project_build('regal')

            # copy sources
            # only what is needed is copied: regal, md5, jsonsl, boost, lookup3, glslopt
            source_path_src = os.path.join(ports.get_dir(), 'regal', 'regal-' + TAG, 'src')
            dest_path_src = os.path.join(ports.get_build_dir(), 'regal', 'src')

            source_path_regal = os.path.join(source_path_src, 'regal')
            source_path_md5 = os.path.join(source_path_src, 'md5')
            source_path_jsonsl = os.path.join(source_path_src, 'jsonsl')
            source_path_boost = os.path.join(source_path_src, 'boost')
            source_path_lookup3 = os.path.join(source_path_src, 'lookup3')
            source_path_glslopt = os.path.join(source_path_src, 'glsl')
            dest_path_regal = os.path.join(dest_path_src, 'regal')
            dest_path_md5 = os.path.join(dest_path_src, 'md5')
            dest_path_jsonsl = os.path.join(dest_path_src, 'jsonsl')
            dest_path_boost = os.path.join(dest_path_src, 'boost')
            dest_path_lookup3 = os.path.join(dest_path_src, 'lookup3')
            dest_path_glslopt = os.path.join(dest_path_src, 'glsl')

            shutil.rmtree(dest_path_src, ignore_errors=True)
            shutil.copytree(source_path_regal, dest_path_regal)
            shutil.copytree(source_path_md5, dest_path_md5)
            shutil.copytree(source_path_jsonsl, dest_path_jsonsl)
            shutil.copytree(source_path_boost, dest_path_boost)
            shutil.copytree(source_path_lookup3, dest_path_lookup3)
            shutil.copytree(source_path_glslopt, dest_path_glslopt)

            # includes
            source_path_include = os.path.join(ports.get_dir(), 'regal', 'regal-' + TAG, 'include')
            dest_path_include = os.path.join(ports.get_build_dir(), 'regal', 'include')
            shutil.copytree(source_path_include, dest_path_include)

            # build
            # there is two separate libraries, with different compilation options: regal and glslopt
            srcs_regal = ['regal/RegalShaderInstance.cpp',
                          'regal/RegalIff.cpp',
                          'regal/RegalQuads.cpp',
                          'regal/Regal.cpp',
                          'regal/RegalLog.cpp',
                          'regal/RegalInit.cpp',
                          'regal/RegalBreak.cpp',
                          'regal/RegalUtil.cpp',
                          'regal/RegalEmu.cpp',
                          'regal/RegalEmuInfo.cpp',
                          'regal/RegalFrame.cpp',
                          'regal/RegalHelper.cpp',
                          'regal/RegalMarker.cpp',
                          'regal/RegalTexC.cpp',
                          'regal/RegalCacheShader.cpp',
                          'regal/RegalCacheTexture.cpp',
                          'regal/RegalConfig.cpp',
                          'regal/RegalContext.cpp',
                          'regal/RegalContextInfo.cpp',
                          'regal/RegalDispatch.cpp',
                          'regal/RegalStatistics.cpp',
                          'regal/RegalLookup.cpp',
                          'regal/RegalPlugin.cpp',
                          'regal/RegalShader.cpp',
                          'regal/RegalToken.cpp',
                          'regal/RegalDispatchGlobal.cpp',
                          'regal/RegalDispatcher.cpp',
                          'regal/RegalDispatcherGL.cpp',
                          'regal/RegalDispatcherGlobal.cpp',
                          'regal/RegalDispatchEmu.cpp',
                          'regal/RegalDispatchGLX.cpp',
                          'regal/RegalDispatchLog.cpp',
                          'regal/RegalDispatchCode.cpp',
                          'regal/RegalDispatchCache.cpp',
                          'regal/RegalDispatchError.cpp',
                          'regal/RegalDispatchLoader.cpp',
                          'regal/RegalDispatchDebug.cpp',
                          'regal/RegalDispatchPpapi.cpp',
                          'regal/RegalDispatchStatistics.cpp',
                          'regal/RegalDispatchStaticES2.cpp',
                          'regal/RegalDispatchStaticEGL.cpp',
                          'regal/RegalDispatchTrace.cpp',
                          'regal/RegalDispatchMissing.cpp',
                          'regal/RegalPixelConversions.cpp',
                          'regal/RegalHttp.cpp',
                          'regal/RegalDispatchHttp.cpp',
                          'regal/RegalJson.cpp',
                          'regal/RegalFavicon.cpp',
                          'regal/RegalMac.cpp',
                          'regal/RegalSo.cpp',
                          'regal/RegalFilt.cpp',
                          'regal/RegalXfer.cpp',
                          'regal/RegalX11.cpp',
                          'regal/RegalDllMain.cpp',
                          'md5/src/md5.c',
                          'jsonsl/jsonsl.c']

            srcs_glslopt = [ 'glsl/src/glsl/ast_array_index.cpp',
                             'glsl/src/glsl/ast_expr.cpp',
                             'glsl/src/glsl/ast_function.cpp',
                             'glsl/src/glsl/ast_to_hir.cpp',
                             'glsl/src/glsl/ast_type.cpp',
                             'glsl/src/glsl/builtin_function.cpp',
                             'glsl/src/glsl/builtin_variables.cpp',
                             'glsl/src/glsl/glsl_lexer.cpp',
                             'glsl/src/glsl/glsl_optimizer.cpp',
                             'glsl/src/glsl/glsl_parser.cpp',
                             'glsl/src/glsl/glsl_parser_extras.cpp',
                             'glsl/src/glsl/glsl_symbol_table.cpp',
                             'glsl/src/glsl/glsl_types.cpp',
                             'glsl/src/glsl/hir_field_selection.cpp',
                             'glsl/src/glsl/ir.cpp',
                             'glsl/src/glsl/ir_basic_block.cpp',
                             'glsl/src/glsl/ir_builder.cpp',
                             'glsl/src/glsl/ir_clone.cpp',
                             'glsl/src/glsl/ir_constant_expression.cpp',
                             'glsl/src/glsl/ir_expression_flattening.cpp',
                             'glsl/src/glsl/ir_function.cpp',
                             'glsl/src/glsl/ir_function_can_inline.cpp',
                             'glsl/src/glsl/ir_function_detect_recursion.cpp',
                             'glsl/src/glsl/ir_hierarchical_visitor.cpp',
                             'glsl/src/glsl/ir_hv_accept.cpp',
                             'glsl/src/glsl/ir_import_prototypes.cpp',
                             'glsl/src/glsl/ir_print_glsl_visitor.cpp',
                             'glsl/src/glsl/ir_print_visitor.cpp',
                             'glsl/src/glsl/ir_reader.cpp',
                             'glsl/src/glsl/ir_rvalue_visitor.cpp',
                             'glsl/src/glsl/ir_unused_structs.cpp',
                             'glsl/src/glsl/ir_validate.cpp',
                             'glsl/src/glsl/ir_variable_refcount.cpp',
                             'glsl/src/glsl/link_functions.cpp',
                             'glsl/src/glsl/link_uniform_block_active_visitor.cpp',
                             'glsl/src/glsl/link_uniform_blocks.cpp',
                             'glsl/src/glsl/link_uniform_initializers.cpp',
                             'glsl/src/glsl/link_uniforms.cpp',
                             'glsl/src/glsl/link_varyings.cpp',
                             'glsl/src/glsl/linker.cpp',
                             'glsl/src/glsl/loop_analysis.cpp',
                             'glsl/src/glsl/loop_controls.cpp',
                             'glsl/src/glsl/loop_unroll.cpp',
                             'glsl/src/glsl/lower_clip_distance.cpp',
                             'glsl/src/glsl/lower_discard.cpp',
                             'glsl/src/glsl/lower_discard_flow.cpp',
                             'glsl/src/glsl/lower_if_to_cond_assign.cpp',
                             'glsl/src/glsl/lower_instructions.cpp',
                             'glsl/src/glsl/lower_jumps.cpp',
                             'glsl/src/glsl/lower_mat_op_to_vec.cpp',
                             'glsl/src/glsl/lower_noise.cpp',
                             'glsl/src/glsl/lower_packed_varyings.cpp',
                             'glsl/src/glsl/lower_variable_index_to_cond_assign.cpp',
                             'glsl/src/glsl/lower_vec_index_to_cond_assign.cpp',
                             'glsl/src/glsl/lower_vec_index_to_swizzle.cpp',
                             'glsl/src/glsl/lower_vector.cpp',
                             'glsl/src/glsl/opt_algebraic.cpp',
                             'glsl/src/glsl/opt_array_splitting.cpp',
                             'glsl/src/glsl/opt_constant_folding.cpp',
                             'glsl/src/glsl/opt_constant_propagation.cpp',
                             'glsl/src/glsl/opt_constant_variable.cpp',
                             'glsl/src/glsl/opt_copy_propagation.cpp',
                             'glsl/src/glsl/opt_copy_propagation_elements.cpp',
                             'glsl/src/glsl/opt_dead_code.cpp',
                             'glsl/src/glsl/opt_dead_code_local.cpp',
                             'glsl/src/glsl/opt_dead_functions.cpp',
                             'glsl/src/glsl/opt_flatten_nested_if_blocks.cpp',
                             'glsl/src/glsl/opt_function_inlining.cpp',
                             'glsl/src/glsl/opt_if_simplification.cpp',
                             'glsl/src/glsl/opt_noop_swizzle.cpp',
                             'glsl/src/glsl/opt_redundant_jumps.cpp',
                             'glsl/src/glsl/opt_structure_splitting.cpp',
                             'glsl/src/glsl/opt_swizzle_swizzle.cpp',
                             'glsl/src/glsl/opt_tree_grafting.cpp',
                             'glsl/src/glsl/ralloc.c',
                             'glsl/src/glsl/s_expression.cpp',
                             'glsl/src/glsl/standalone_scaffolding.cpp',
                             'glsl/src/glsl/strtod.c',
                             'glsl/src/glsl/glcpp/glcpp-lex.c',
                             'glsl/src/glsl/glcpp/glcpp-parse.c',
                             'glsl/src/glsl/glcpp/pp.c',
                             'glsl/src/mesa/main/imports.c',
                             'glsl/src/mesa/main/hash_table.c',
                             'glsl/src/mesa/program/symbol_table.c',
                             'glsl/src/mesa/program/prog_hash_table.c']

            commands = []
            o_s = []

            # commands for glslopt
            for src in srcs_glslopt:
                c = os.path.join(dest_path_src, src)
                o = os.path.join(dest_path_src, src + '.o')
                shared.safe_ensure_dirs(os.path.dirname(o))
                commands.append([shared.PYTHON, shared.EMCC, c,
                                 '-DNDEBUG',
                                 '-fvisibility=hidden',
                                 '-Wno-sign-compare',
                                 '-O2',
                                 '-I' + os.path.join(dest_path_glslopt, 'include'),
                                 '-I' + os.path.join(dest_path_glslopt,'src','glsl'),
                                 '-I' + os.path.join(dest_path_glslopt,'src','mesa'),
                                 '-o', o,
                                 '-w'])
                o_s.append(o)

            # commands for regal
            for src in srcs_regal:
                c = os.path.join(dest_path_src, src)
                o = os.path.join(dest_path_src, src + '.o')
                shared.safe_ensure_dirs(os.path.dirname(o))
                commands.append([shared.PYTHON, shared.EMCC, c,
                                 # specify the defined symbols as the Regal Makefiles does for Emscripten+Release
                                 # the define logic for other symbols will be handled automatically by Regal headers (SYS_EMSCRIPTEN, SYS_EGL, SYS_ES2, etc.)
                                 '-DNDEBUG',
                                 '-DREGAL_NO_PNG=1',
                                 '-DREGAL_LOG=0',
                                 '-DREGAL_NO_TLS=1',
                                 '-DREGAL_THREAD_LOCKING=0',
                                 '-fomit-frame-pointer',
                                 '-Wno-constant-logical-operand',
                                 '-fvisibility=hidden',
                                 '-O2',
                                 '-o', o,
                                 '-I' + dest_path_include,
                                 '-I' + dest_path_regal,
                                 '-I' + os.path.join(dest_path_md5, 'include'),
                                 '-I' + dest_path_lookup3,
                                 '-I' + dest_path_jsonsl,
                                 '-I' + dest_path_boost,
                                 '-I' + os.path.join(dest_path_glslopt, 'include'),
                                 '-I' + os.path.join(dest_path_glslopt,'src','glsl'),
                                 '-I' + os.path.join(dest_path_glslopt,'src','mesa'),
                                 '-w'])
                o_s.append(o)

            ports.run_commands(commands)
            final = os.path.join(ports.get_build_dir(), 'regal', 'libregal.bc')
            shared.try_delete(final)
            shared.Building.link(o_s, final)
            assert os.path.exists(final)
            return final
        return [shared.Cache.get('regal', create, what='port')]
    else:
        return []

def process_args(ports, args, settings, shared):
    if settings.USE_REGAL == 1:
        get(ports, settings, shared)
        args += ['-Xclang', '-isystem' + os.path.join(ports.get_build_dir(), 'regal', 'include')]
    return args

def show():
    return 'regal (USE_REGAL=1; Regal license)'
