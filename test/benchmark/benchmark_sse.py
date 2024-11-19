#!/usr/bin/env python3
# Copyright 2020 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import json
import os
import re
import shutil
import sys
import tempfile
from subprocess import Popen

__rootpath__ = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, __rootpath__)

from tools.shared import WINDOWS, CLANG_CXX, EMCC, PIPE
from tools.shared import run_process
from tools.config import V8_ENGINE
from common import EMRUN, test_file
import clang_native

temp_dir = tempfile.mkdtemp()

# System info
system_info = Popen([EMRUN, '--system_info'], stdout=PIPE, stderr=PIPE).communicate()

# Native info
native_info = Popen(['clang', '-v'], stdout=PIPE, stderr=PIPE).communicate()

# Emscripten info
emscripten_info = Popen([EMCC, '-v'], stdout=PIPE, stderr=PIPE).communicate()


def run_benchmark(benchmark_file, results_file, build_args):
    # Run native build
    out_file = os.path.join(temp_dir, 'benchmark_sse_native')
    if WINDOWS:
        out_file += '.exe'
    cmd = [CLANG_CXX] + clang_native.get_clang_native_args() + [benchmark_file, '-O3', '-o', out_file]
    print('Building native version of the benchmark:')
    print(' '.join(cmd))
    run_process(cmd, env=clang_native.get_clang_native_env())

    native_results = Popen([out_file], stdout=PIPE, stderr=PIPE).communicate()
    print(native_results[0])

    # Run emscripten build
    out_file = os.path.join(temp_dir, 'benchmark_sse_html.js')
    cmd = [EMCC, benchmark_file, '-O3', '-sTOTAL_MEMORY=536870912', '-o', out_file] + build_args
    print('Building Emscripten version of the benchmark:')
    print(' '.join(cmd))
    run_process(cmd)

    cmd = V8_ENGINE + ['--experimental-wasm-simd', os.path.basename(out_file)]
    print(' '.join(cmd))
    old_dir = os.getcwd()
    os.chdir(os.path.dirname(out_file))
    wasm_results = Popen(cmd, stdout=PIPE, stderr=PIPE).communicate()
    os.chdir(old_dir)

    if not wasm_results:
        raise Exception('Unable to run benchmark in V8!')

    if not wasm_results[0].strip():
        print(wasm_results[1])
        sys.exit(1)

    print(wasm_results[0])

    def strip_comments(text):
        return re.sub('//.*?\n|/\*.*?\*/', '', text, re.S) # noqa

    benchmark_results = strip_comments(wasm_results[0])

    # Strip out unwanted print output.
    benchmark_results = benchmark_results[benchmark_results.find('{'):].strip()
    if '*************************' in benchmark_results:
        benchmark_results = benchmark_results[:benchmark_results.find('*************************')].strip()

    print(benchmark_results)

    shutil.rmtree(temp_dir)

    native_results = json.loads(native_results[0])
    benchmark_results = benchmark_results[benchmark_results.index('{'):benchmark_results.rindex('}') + 1]
    wasm_results = json.loads(benchmark_results)

    # native_workload = native_results['workload']
    # html_workload = wasm_results['workload']

    html = '''<html><head></head><body><h1>SSE JavaScript Benchmark</h1>
    <script src="https://ajax.googleapis.com/ajax/libs/jquery/1.8.2/jquery.min.js"></script>
    <script src="https://code.highcharts.com/highcharts.js"></script>
    <script src="https://code.highcharts.com/modules/exporting.js"></script><b>System Info:</b><br/>
    ''' + system_info[0].replace('\n', '<br/>') + '''
    <b>Native Clang Compiler:</b><br/>
    ''' + native_info[1].replace('\n', '<br/>') + '''
    <b>Emscripten Compiler:</b><br/>
    ''' + emscripten_info[0].replace('\n', '<br/>')

    charts_native = {}
    charts_html = {}
    for result in native_results['results']:
        ch = result['chart']
        if ch not in charts_native:
            charts_native[ch] = []
        charts_native[ch] += [result]
    for result in wasm_results['results']:
        ch = result['chart']
        if ch not in charts_html:
            charts_html[ch] = []
        charts_html[ch] += [result]

    def find_result_in_category(results, category):
        for result in results:
            if result['category'] == category:
                return result
        return None

    def format_comparison(a, b):
        if a < b and a != 0:
            return "<span style='color:green;font-weight:bold;'> {:10.2f}".format(b / a) + 'x FASTER</span>'
        elif b != 0:
            return "<span style='color:red;font-weight:bold;'> {:10.2f}".format(a / b) + 'x SLOWER</span>'
        else:
            return "<span style='color:red;font-weight:bold;'> NaN </span>"

    chartNumber = 0

    total_time_native_scalar = 0
    total_time_native_simd = 0
    total_time_html_scalar = 0
    total_time_html_simd = 0

    for chart_name in charts_native.keys():
        # Extract data for each chart.
        categories = []
        nativeScalarResults = []
        nativeSimdResults = []
        htmlScalarResults = []
        htmlSimdResults = []
        native_results = charts_native[chart_name]
        wasm_results = charts_html[chart_name]
        textual_results_native = '<p>'
        textual_results_html = '<p>'
        textual_results_html2 = '<p>'
        textual_results_html3 = '<p>'
        for result in native_results:
            categories += ["'" + result['category'] + "'"]
            nsc = result['scalar']
            nsi = result['simd']
            nativeScalarResults += [str(nsc)]
            nativeSimdResults += [str(nsi)]
            html_result = find_result_in_category(wasm_results, result['category'])
            textual_results_native += 'Native ' + result['category'] + ': ' + "{:10.4f}".format(nsc) + 'ns -> ' + "{:10.4f}".format(nsi) + 'ns. '
            textual_results_native += 'Native SSE is ' + format_comparison(nsi, nsc) + ' than native scalar. &nbsp; &nbsp; &nbsp; &nbsp; <br />'

            if html_result is not None:
                hsc = html_result['scalar']
                htmlScalarResults += [str(hsc)]
                hsi = html_result['simd']
                htmlSimdResults += [str(hsi)]
                textual_results_html += 'JS ' + result['category'] + ': ' + "{:10.4f}".format(hsc) + 'ns -> ' + "{:10.4f}".format(hsi) + 'ns. '
                textual_results_html += 'JS SSE is ' + format_comparison(hsi, hsc) + ' than JS scalar. &nbsp; &nbsp; &nbsp; &nbsp; <br />'
                textual_results_html2 += 'JS ' + result['category'] + ': JS scalar is ' + format_comparison(hsc, nsc) + ' than native scalar. &nbsp; &nbsp; &nbsp; &nbsp; <br />'
                textual_results_html3 += 'JS ' + result['category'] + ': JS SSE is ' + format_comparison(hsi, nsi) + ' than native SSE. &nbsp; &nbsp; &nbsp; &nbsp; <br />'
                total_time_native_scalar += nsc
                total_time_native_simd += nsi
                total_time_html_scalar += hsc
                total_time_html_simd += hsi
            else:
                htmlScalarResults += [str(-1)]
                htmlSimdResults += [str(-1)]

        chartNumber += 1
        html += '<div id="chart' + str(chartNumber) + '" style="width:100%; height:400px; margin-top: 100px;"></div>'
        html += '''<script>$(function () {
        $('#chart''' + str(chartNumber) + '''').highcharts({
            chart: {
                type: 'column'
            },
            title: {
                text: "''' + chart_name + '''"
            },
            subtitle: {
                text: 'Time per operation in nanoseconds'
            },
            xAxis: {
                categories: [''' + ','.join(categories) + '''
                ]
            },
            yAxis: {
                min: 0,
                title: {
                    text: 'Time (nanoseconds)'
                }
            },
            tooltip: {
                headerFormat: '<span style="font-size:10px">{point.key}</span><table>',
                pointFormat: '<tr><td style="color:{series.color};padding:0">{series.name}: </td>' +
                    '<td style="padding:0"><b>{point.y:.3f} ns</b></td></tr>',
                footerFormat: '</table>',
                shared: true,
                useHTML: true
            },
            plotOptions: {
                column: {
                    pointPadding: 0.2,
                    borderWidth: 0
                }
            },
            series: [{
                name: 'Native scalar',
                data: [''' + ','.join(nativeScalarResults) + ''']

            }, {
                name: 'Native SSE',
                data: [''' + ','.join(nativeSimdResults) + ''']

            }, {
                name: 'JS scalar',
                data: [''' + ','.join(htmlScalarResults) + ''']

            }, {
                name: 'JS SSE',
                data: [''' + ','.join(htmlSimdResults) + ''']

            }]
        });
    });</script>''' + '<table><tr><td>' + textual_results_native + '</td><td>' + textual_results_html + '</td></tr><tr><td>' + textual_results_html2 + '</td><td>' + textual_results_html3 + '</td></tr></table>'

    # Final overall score

    html += '<div id="overallscore" style="width:100%; height:400px; margin-top: 100px;"></div>'
    html += '''<script>$(function () {
        $('#overallscore').highcharts({
            chart: {
                type: 'column'
            },
            title: {
                text: "Overall Execution Time"
            },
            xAxis: {
                categories: ['Total time normalized to native']
            },
            yAxis: {
                min: 0,
                title: {
                    text: 'Relative time'
                }
            },
            tooltip: {
                headerFormat: '<span style="font-size:10px">{point.key}</span><table>',
                pointFormat: '<tr><td style="color:{series.color};padding:0">{series.name}: </td>' +
                    '<td style="padding:0"><b>{point.y:.3f}x</b></td></tr>',
                footerFormat: '</table>',
                shared: true,
                useHTML: true
            },
            plotOptions: {
                column: {
                    pointPadding: 0.2,
                    borderWidth: 0
                }
            },
            series: [{
                name: 'Native scalar',
                data: [''' + str(1.0) + ''']

            }, {
                name: 'Native SSE',
                data: [''' + (str(total_time_native_simd / total_time_native_scalar) if total_time_native_scalar != 0 else 'N/A') + ''']

            }, {
                name: 'JS scalar',
                data: [''' + (str(total_time_html_scalar / total_time_native_scalar) if total_time_native_scalar != 0 else 'N/A') + ''']

            }, {
                name: 'JS SSE',
                data: [''' + (str(total_time_html_simd / total_time_native_scalar) if total_time_native_scalar != 0 else 'N/A') + ''']

            }]
        });
    });</script>'''

    html += '</body></html>'

    open(results_file, 'w').write(html)
    print('Wrote ' + str(len(html)) + ' bytes to file ' + results_file + '.')


if __name__ == '__main__':
    suite = sys.argv[1].lower() if len(sys.argv) == 2 else None
    if suite in ['sse', 'sse1']:
        run_benchmark(test_file('sse/benchmark_sse1.cpp'), 'results_sse1.html', ['-msse'])
    elif suite == 'sse2':
        run_benchmark(test_file('sse/benchmark_sse2.cpp'), 'results_sse2.html', ['-msse2'])
    elif suite == 'sse3':
        run_benchmark(test_file('sse/benchmark_sse3.cpp'), 'results_sse3.html', ['-msse3'])
    elif suite == 'ssse3':
        run_benchmark(test_file('sse/benchmark_ssse3.cpp'), 'results_ssse3.html', ['-mssse3'])
    else:
        raise Exception('Usage: python test/benchmark_sse.py sse1|sse2|sse3')
